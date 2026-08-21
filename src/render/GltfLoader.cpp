/**
 * @file cgltf-based static mesh and base-color material importer.
 * @author Codex
 * @created 2026-08-21
 * @depends render/GltfLoader.h, render/ResourceCache.h, cgltf
 */
#include "render/GltfLoader.h"

#include "render/ResourceCache.h"

#include <SimpleMath.h>
#include <cgltf.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <cwctype>
#include <memory>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace lrender {
namespace {

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

struct CgltfDeleter {
    void operator()(cgltf_data* data) const noexcept { cgltf_free(data); }
};

std::string Utf8Path(const std::filesystem::path& path) {
    const auto text = path.u8string();
    return {reinterpret_cast<const char*>(text.data()), text.size()};
}

const char* ResultName(cgltf_result result) {
    switch (result) {
    case cgltf_result_success: return "success";
    case cgltf_result_data_too_short: return "data too short";
    case cgltf_result_unknown_format: return "unknown format";
    case cgltf_result_invalid_json: return "invalid JSON";
    case cgltf_result_invalid_gltf: return "invalid glTF";
    case cgltf_result_invalid_options: return "invalid options";
    case cgltf_result_file_not_found: return "file not found";
    case cgltf_result_io_error: return "I/O error";
    case cgltf_result_out_of_memory: return "out of memory";
    case cgltf_result_legacy_gltf: return "legacy glTF";
    default: return "unknown error";
    }
}

const cgltf_accessor* FindAttribute(
    const cgltf_primitive& primitive, cgltf_attribute_type type, cgltf_int index = 0) {
    for (cgltf_size attributeIndex = 0;
         attributeIndex < primitive.attributes_count; ++attributeIndex) {
        const cgltf_attribute& attribute = primitive.attributes[attributeIndex];
        if (attribute.type == type && attribute.index == index) {
            return attribute.data;
        }
    }
    return nullptr;
}

Matrix NodeWorldMatrix(const cgltf_node& node) {
    cgltf_float values[16]{};
    cgltf_node_transform_world(&node, values);
    return {
        values[0], values[1], values[2], values[3],
        values[4], values[5], values[6], values[7],
        values[8], values[9], values[10], values[11],
        values[12], values[13], values[14], values[15]};
}

void ComputeMissingNormals(
    std::vector<MeshVertex>& vertices, const std::vector<std::uint32_t>& indices) {
    for (MeshVertex& vertex : vertices) {
        vertex.normal = {};
    }
    for (std::size_t index = 0; index + 2 < indices.size(); index += 3) {
        const std::uint32_t first = indices[index];
        const std::uint32_t second = indices[index + 1];
        const std::uint32_t third = indices[index + 2];
        const Vector3 a{vertices[first].position};
        const Vector3 b{vertices[second].position};
        const Vector3 c{vertices[third].position};
        const Vector3 face = (b - a).Cross(c - a);
        for (const std::uint32_t vertexIndex : {first, second, third}) {
            Vector3 normal{vertices[vertexIndex].normal};
            normal += face;
            vertices[vertexIndex].normal = normal;
        }
    }
    for (MeshVertex& vertex : vertices) {
        Vector3 normal{vertex.normal};
        if (normal.LengthSquared() > 0.0F) {
            normal.Normalize();
        } else {
            normal = Vector3::UnitY;
        }
        vertex.normal = normal;
    }
}

D3D11_TEXTURE_ADDRESS_MODE AddressMode(cgltf_wrap_mode mode) {
    switch (mode) {
    case cgltf_wrap_mode_clamp_to_edge: return D3D11_TEXTURE_ADDRESS_CLAMP;
    case cgltf_wrap_mode_mirrored_repeat: return D3D11_TEXTURE_ADDRESS_MIRROR;
    default: return D3D11_TEXTURE_ADDRESS_WRAP;
    }
}

D3D11_FILTER FilterMode(const cgltf_sampler* sampler) {
    if (sampler == nullptr) {
        return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    }
    const bool nearestMag = sampler->mag_filter == cgltf_filter_type_nearest;
    const bool nearestMin = sampler->min_filter == cgltf_filter_type_nearest ||
        sampler->min_filter == cgltf_filter_type_nearest_mipmap_nearest ||
        sampler->min_filter == cgltf_filter_type_nearest_mipmap_linear;
    return nearestMag && nearestMin
        ? D3D11_FILTER_MIN_MAG_MIP_POINT
        : D3D11_FILTER_MIN_MAG_MIP_LINEAR;
}

std::shared_ptr<Texture2D> LoadImage(
    const cgltf_data& data, const cgltf_image& image,
    const std::filesystem::path& modelPath, ResourceCache& resources) {
    if (image.uri != nullptr && std::strncmp(image.uri, "data:", 5) != 0) {
        std::string decodedUri = image.uri;
        cgltf_decode_uri(decodedUri.data());
        decodedUri.resize(std::strlen(decodedUri.c_str()));
        const auto uri = std::u8string(
            reinterpret_cast<const char8_t*>(decodedUri.data()), decodedUri.size());
        return resources.LoadTexture(modelPath.parent_path() / std::filesystem::path(uri));
    }
    if (image.buffer_view == nullptr || image.buffer_view->buffer == nullptr ||
        image.buffer_view->buffer->data == nullptr) {
        throw std::runtime_error("glTF image has no supported URI or buffer view");
    }
    const auto imageIndex = static_cast<std::size_t>(&image - data.images);
    const auto* begin = static_cast<const std::byte*>(image.buffer_view->buffer->data) +
                        image.buffer_view->offset;
    const std::span bytes{begin, image.buffer_view->size};
    return resources.LoadEmbeddedTexture(
        Utf8Path(modelPath) + "#image-" + std::to_string(imageIndex), bytes);
}

Material LoadMaterial(
    const cgltf_data& data, const cgltf_material* source,
    const std::filesystem::path& modelPath, ResourceCache& resources) {
    Material material = resources.DefaultMaterial();
    material.baseColorTexture.reset();
    material.name = source != nullptr && source->name != nullptr ? source->name : "glTF material";
    if (source == nullptr || !source->has_pbr_metallic_roughness) {
        material.baseColorTexture = resources.DefaultMaterial().baseColorTexture;
        return material;
    }

    const auto& pbr = source->pbr_metallic_roughness;
    material.baseColorFactor = {
        pbr.base_color_factor[0], pbr.base_color_factor[1],
        pbr.base_color_factor[2], pbr.base_color_factor[3]};
    material.specularStrength = 0.04F + pbr.metallic_factor * 0.46F;
    material.shininess = 8.0F + (1.0F - pbr.roughness_factor) * 120.0F;
    material.doubleSided = source->double_sided != 0;

    const cgltf_texture* texture = pbr.base_color_texture.texture;
    if (texture != nullptr && texture->image != nullptr) {
        material.baseColorTexture = LoadImage(data, *texture->image, modelPath, resources);
        SamplerDescription samplerDescription;
        samplerDescription.filter = FilterMode(texture->sampler);
        if (texture->sampler != nullptr) {
            samplerDescription.addressU = AddressMode(texture->sampler->wrap_s);
            samplerDescription.addressV = AddressMode(texture->sampler->wrap_t);
        }
        material.sampler = resources.GetSampler(samplerDescription);
    } else {
        material.baseColorTexture = resources.DefaultMaterial().baseColorTexture;
    }
    return material;
}

ModelPart LoadPrimitive(
    const cgltf_data& data, const cgltf_node& node, const cgltf_primitive& primitive,
    const std::filesystem::path& modelPath, ResourceCache& resources) {
    if (primitive.type != cgltf_primitive_type_triangles) {
        throw std::runtime_error("Only triangle-list glTF primitives are supported");
    }
    const cgltf_accessor* positions = FindAttribute(primitive, cgltf_attribute_type_position);
    const cgltf_accessor* normals = FindAttribute(primitive, cgltf_attribute_type_normal);
    const cgltf_int textureCoordinateSet = primitive.material != nullptr &&
        primitive.material->has_pbr_metallic_roughness
        ? primitive.material->pbr_metallic_roughness.base_color_texture.texcoord : 0;
    const cgltf_accessor* textureCoordinates =
        FindAttribute(primitive, cgltf_attribute_type_texcoord, textureCoordinateSet);
    if (positions == nullptr || positions->count == 0) {
        throw std::runtime_error("glTF primitive has no POSITION attribute");
    }
    if (positions->count > std::numeric_limits<std::uint32_t>::max()) {
        throw std::runtime_error("glTF primitive exceeds the 32-bit vertex limit");
    }

    const Matrix world = NodeWorldMatrix(node);
    const Matrix normalMatrix = world.Invert().Transpose();
    std::vector<MeshVertex> vertices(positions->count);
    for (cgltf_size index = 0; index < positions->count; ++index) {
        float positionValues[3]{};
        float normalValues[3]{0.0F, 1.0F, 0.0F};
        float textureValues[2]{};
        if (!cgltf_accessor_read_float(positions, index, positionValues, 3)) {
            throw std::runtime_error("Failed to read glTF POSITION accessor");
        }
        if (normals != nullptr) {
            if (!cgltf_accessor_read_float(normals, index, normalValues, 3)) {
                throw std::runtime_error("Failed to read glTF NORMAL accessor");
            }
        }
        if (textureCoordinates != nullptr) {
            if (!cgltf_accessor_read_float(textureCoordinates, index, textureValues, 2)) {
                throw std::runtime_error("Failed to read glTF TEXCOORD accessor");
            }
        }
        Vector3 position = Vector3::Transform(
            Vector3{positionValues[0], positionValues[1], positionValues[2]}, world);
        Vector3 normal = Vector3::TransformNormal(
            Vector3{normalValues[0], normalValues[1], normalValues[2]}, normalMatrix);
        normal.Normalize();
        position.z = -position.z;
        normal.z = -normal.z;
        vertices[index] = {
            position, normal, Vector2{textureValues[0], textureValues[1]}};
    }

    const std::size_t indexCount = primitive.indices != nullptr
        ? primitive.indices->count : positions->count;
    if (indexCount % 3 != 0 || indexCount > std::numeric_limits<std::uint32_t>::max()) {
        throw std::runtime_error("glTF triangle index count is invalid or exceeds 32-bit limits");
    }
    std::vector<std::uint32_t> indices(indexCount);
    for (std::size_t index = 0; index < indexCount; ++index) {
        const std::size_t value = primitive.indices != nullptr
            ? cgltf_accessor_read_index(primitive.indices, index) : index;
        if (value >= vertices.size()) {
            throw std::runtime_error("glTF index is outside the vertex buffer");
        }
        indices[index] = static_cast<std::uint32_t>(value);
    }
    for (std::size_t index = 0; index + 2 < indices.size(); index += 3) {
        std::swap(indices[index + 1], indices[index + 2]);
    }
    if (normals == nullptr) {
        ComputeMissingNormals(vertices, indices);
    }
    return {
        std::make_unique<Mesh>(resources.Device(), vertices, indices),
        LoadMaterial(data, primitive.material, modelPath, resources)};
}

} // namespace

std::shared_ptr<Model> GltfLoader::Load(
    const std::filesystem::path& path, ResourceCache& resources) {
    std::wstring extension = path.extension().wstring();
    std::ranges::transform(extension, extension.begin(), ::towlower);
    if (extension != L".gltf" && extension != L".glb") {
        throw std::invalid_argument("Model importer supports only .gltf and .glb files");
    }
    if (!std::filesystem::is_regular_file(path)) {
        throw std::runtime_error("Model file does not exist: " + Utf8Path(path));
    }

    cgltf_options options{};
    cgltf_data* rawData{};
    const std::string pathText = Utf8Path(path);
    cgltf_result result = cgltf_parse_file(&options, pathText.c_str(), &rawData);
    if (result != cgltf_result_success) {
        throw std::runtime_error(
            "Failed to parse glTF " + pathText + ": " + ResultName(result));
    }
    std::unique_ptr<cgltf_data, CgltfDeleter> data(rawData);
    result = cgltf_load_buffers(&options, data.get(), pathText.c_str());
    if (result != cgltf_result_success) {
        throw std::runtime_error(
            "Failed to load glTF buffers " + pathText + ": " + ResultName(result));
    }
    result = cgltf_validate(data.get());
    if (result != cgltf_result_success) {
        throw std::runtime_error(
            "glTF validation failed " + pathText + ": " + ResultName(result));
    }

    auto model = std::make_shared<Model>();
    model->name = Utf8Path(path.filename());
    for (cgltf_size nodeIndex = 0; nodeIndex < data->nodes_count; ++nodeIndex) {
        const cgltf_node& node = data->nodes[nodeIndex];
        if (node.mesh == nullptr) {
            continue;
        }
        for (cgltf_size primitiveIndex = 0;
             primitiveIndex < node.mesh->primitives_count; ++primitiveIndex) {
            model->parts.push_back(LoadPrimitive(
                *data, node, node.mesh->primitives[primitiveIndex], path, resources));
        }
    }
    if (model->parts.empty()) {
        throw std::runtime_error("glTF contains no supported triangle mesh: " + pathText);
    }
    return model;
}

} // namespace lrender
