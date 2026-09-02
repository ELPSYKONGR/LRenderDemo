/**
 * @file tinyobjloader-based OBJ geometry and MTL material import.
 * @author Codex
 * @created 2026-08-25
 * @depends render/ObjLoader.h, render/ResourceCache.h, render/MeshImportUtils.h
 */
#include "render/ObjLoader.h"

#include "render/MeshImportUtils.h"
#include "render/ResourceCache.h"

#include <tiny_obj_loader.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace lrender
{
namespace
{

struct VertexKey
{
    int position = -1;
    int normal = -1;
    int textureCoordinate = -1;

    bool operator==(const VertexKey&) const = default;
};

struct VertexKeyHash
{
    std::size_t operator()(const VertexKey& key) const noexcept
    {
        std::size_t value = std::hash<int>{}(key.position);
        value ^= std::hash<int>{}(key.normal) + 0x9e3779b9U + (value << 6U) + (value >> 2U);
        value ^= std::hash<int>{}(key.textureCoordinate) + 0x9e3779b9U + (value << 6U) + (value >> 2U);
        return value;
    }
};

struct PartBuilder
{
    int materialId = -1;
    bool hasMissingNormals = false;
    std::vector<MeshVertex> vertices;
    std::vector<std::uint32_t> indices;
    std::unordered_map<VertexKey, std::uint32_t, VertexKeyHash> vertexLookup;
};

std::filesystem::path Utf8RelativePath(const std::string& value)
{
    const std::u8string text(reinterpret_cast<const char8_t*>(value.data()), value.size());
    return std::filesystem::path(text);
}

void ValidateIndex(int index, std::size_t valueCount, const char* attribute)
{
    if (index < 0 || static_cast<std::size_t>(index) >= valueCount)
    {
        throw std::runtime_error(std::string("OBJ ") + attribute + " index is out of range");
    }
}

std::uint32_t AddVertex(PartBuilder& builder, const tinyobj::index_t& source, const tinyobj::attrib_t& attributes)
{
    const VertexKey key{source.vertex_index, source.normal_index, source.texcoord_index};
    if (const auto found = builder.vertexLookup.find(key); found != builder.vertexLookup.end())
    {
        return found->second;
    }
    if (builder.vertices.size() >= std::numeric_limits<std::uint32_t>::max())
    {
        throw std::runtime_error("OBJ mesh exceeds the 32-bit vertex limit");
    }

    ValidateIndex(source.vertex_index, attributes.vertices.size() / 3U, "position");
    const std::size_t positionOffset = static_cast<std::size_t>(source.vertex_index) * 3U;
    MeshVertex vertex{};
    vertex.position = {attributes.vertices[positionOffset], attributes.vertices[positionOffset + 1U],
                       -attributes.vertices[positionOffset + 2U]};

    if (source.normal_index >= 0)
    {
        ValidateIndex(source.normal_index, attributes.normals.size() / 3U, "normal");
        const std::size_t normalOffset = static_cast<std::size_t>(source.normal_index) * 3U;
        vertex.normal = {attributes.normals[normalOffset], attributes.normals[normalOffset + 1U],
                         -attributes.normals[normalOffset + 2U]};
    }
    else
    {
        builder.hasMissingNormals = true;
    }

    if (source.texcoord_index >= 0)
    {
        ValidateIndex(source.texcoord_index, attributes.texcoords.size() / 2U, "texture coordinate");
        const std::size_t textureOffset = static_cast<std::size_t>(source.texcoord_index) * 2U;
        vertex.textureCoordinate = {attributes.texcoords[textureOffset],
                                    1.0F - attributes.texcoords[textureOffset + 1U]};
    }

    const auto index = static_cast<std::uint32_t>(builder.vertices.size());
    builder.vertices.push_back(vertex);
    builder.vertexLookup.emplace(key, index);
    return index;
}

Material LoadMaterial(int materialId, const std::vector<tinyobj::material_t>& sources,
                      const std::filesystem::path& modelPath, ResourceCache& resources)
{
    Material material = resources.DefaultMaterial();
    if (materialId < 0)
    {
        material.SetName("OBJ default material");
        return material;
    }
    if (static_cast<std::size_t>(materialId) >= sources.size())
    {
        throw std::runtime_error("OBJ face references an invalid material");
    }

    const tinyobj::material_t& source = sources[static_cast<std::size_t>(materialId)];
    material.SetName(source.name.empty() ? "OBJ material" : source.name);
    material.SetBaseColorFactor({source.diffuse[0], source.diffuse[1], source.diffuse[2], source.dissolve});
    material.SetSpecularColor({source.specular[0], source.specular[1], source.specular[2], 1.0F});
    material.SetSpecularStrength(1.0F);
    material.SetShininess(std::clamp(source.shininess, 1.0F, 256.0F));
    if (!source.diffuse_texname.empty())
    {
        material.SetBaseColorTexture(
            resources.LoadTexture(modelPath.parent_path() / Utf8RelativePath(source.diffuse_texname)));
        material.SetUsesBaseColorTexture(true);
    }
    return material;
}

MeshAssetEntity LoadShape(const tinyobj::shape_t& shape, std::size_t shapeIndex, const tinyobj::attrib_t& attributes,
                          const std::vector<tinyobj::material_t>& materials, const std::filesystem::path& modelPath,
                          ResourceCache& resources)
{
    std::vector<PartBuilder> builders;
    std::unordered_map<int, std::size_t> materialBuilders;
    std::size_t sourceIndex = 0;
    for (std::size_t faceIndex = 0; faceIndex < shape.mesh.num_face_vertices.size(); ++faceIndex)
    {
        const std::size_t vertexCount = shape.mesh.num_face_vertices[faceIndex];
        if (vertexCount != 3 || sourceIndex + vertexCount > shape.mesh.indices.size())
        {
            throw std::runtime_error("OBJ triangulation produced an invalid face");
        }
        const int materialId = faceIndex < shape.mesh.material_ids.size() ? shape.mesh.material_ids[faceIndex] : -1;
        auto [position, inserted] = materialBuilders.emplace(materialId, builders.size());
        if (inserted)
        {
            builders.push_back(PartBuilder{materialId});
        }
        PartBuilder& builder = builders[position->second];
        const std::array<std::uint32_t, 3> face{AddVertex(builder, shape.mesh.indices[sourceIndex], attributes),
                                                AddVertex(builder, shape.mesh.indices[sourceIndex + 1U], attributes),
                                                AddVertex(builder, shape.mesh.indices[sourceIndex + 2U], attributes)};
        builder.indices.insert(builder.indices.end(), {face[0], face[2], face[1]});
        sourceIndex += vertexCount;
    }

    MeshAssetEntity entity;
    entity.name = shape.name.empty() ? "OBJ Shape " + std::to_string(shapeIndex + 1U) : shape.name;
    for (PartBuilder& builder : builders)
    {
        if (builder.hasMissingNormals)
        {
            mesh_import::ComputeNormals(builder.vertices, builder.indices);
        }
        entity.parts.push_back({std::make_unique<Mesh>(resources.Device(), builder.vertices, builder.indices),
                                LoadMaterial(builder.materialId, materials, modelPath, resources)});
    }
    return entity;
}

} // namespace

bool ObjLoader::SupportsExtension(std::wstring_view extension) const noexcept
{
    return extension == L".obj";
}

std::shared_ptr<MeshAsset> ObjLoader::Import(const std::filesystem::path& path, ResourceCache& resources) const
{
    if (!std::filesystem::is_regular_file(path))
    {
        throw std::runtime_error("OBJ file does not exist: " + mesh_import::PathUtf8(path));
    }

    tinyobj::ObjReaderConfig configuration;
    configuration.triangulate = true;
    configuration.mtl_search_path = mesh_import::PathUtf8(path.parent_path());
    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(mesh_import::PathUtf8(path), configuration))
    {
        throw std::runtime_error("Failed to parse OBJ " + mesh_import::PathUtf8(path) + ": " + reader.Error());
    }

    auto asset = std::make_shared<MeshAsset>();
    asset->name = mesh_import::PathUtf8(path.filename());
    if (!reader.Warning().empty())
    {
        asset->warnings.push_back(reader.Warning());
    }
    const auto& shapes = reader.GetShapes();
    for (std::size_t index = 0; index < shapes.size(); ++index)
    {
        MeshAssetEntity entity =
            LoadShape(shapes[index], index, reader.GetAttrib(), reader.GetMaterials(), path, resources);
        if (!entity.parts.empty())
        {
            asset->entities.push_back(std::move(entity));
        }
    }
    if (asset->entities.empty())
    {
        throw std::runtime_error("OBJ contains no supported triangle mesh: " + mesh_import::PathUtf8(path));
    }
    return asset;
}

} // namespace lrender
