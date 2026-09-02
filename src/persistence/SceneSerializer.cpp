/**
 * @file Versioned .lscene JSON reader and atomic writer.
 * @author Codex
 * @created 2026-08-26
 * @depends persistence/SceneSerializer.h, nlohmann/json, Win32
 */
#include "persistence/SceneSerializer.h"

#include <nlohmann/json.hpp>
#include <windows.h>

#include <array>
#include <fstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>

namespace lrender
{
namespace
{

using Json = nlohmann::json;
using DirectX::SimpleMath::Color;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;

std::string PathUtf8(const std::filesystem::path& path)
{
    const auto text = path.generic_u8string();
    return {reinterpret_cast<const char*>(text.data()), text.size()};
}

std::filesystem::path Utf8Path(const std::string& text)
{
    return std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(text.data()), text.size()));
}

std::filesystem::path ResolveResourcePath(const std::filesystem::path& value,
                                          const std::filesystem::path& sceneDirectory)
{
    return value.is_absolute() ? value.lexically_normal() : (sceneDirectory / value).lexically_normal();
}

std::string StoreResourcePath(const std::filesystem::path& value, const std::filesystem::path& sceneDirectory)
{
    if (value.empty())
    {
        return {};
    }
    const std::filesystem::path absolute =
        value.is_absolute() ? value.lexically_normal() : std::filesystem::absolute(value).lexically_normal();
    const std::filesystem::path relative = absolute.lexically_relative(sceneDirectory);
    return PathUtf8(relative.empty() ? absolute : relative);
}

Json Vector(const Vector2& value)
{
    return Json::array({value.x, value.y});
}
Json Vector(const Vector3& value)
{
    return Json::array({value.x, value.y, value.z});
}
Json Vector(const Color& value)
{
    return Json::array({value.x, value.y, value.z, value.w});
}

template <std::size_t Size> std::array<float, Size> ReadVector(const Json& value, const char* field)
{
    if (!value.is_array() || value.size() != Size)
    {
        throw std::runtime_error(std::string(field) + " must be a numeric array");
    }
    std::array<float, Size> result{};
    for (std::size_t index = 0; index < Size; ++index)
    {
        result[index] = value.at(index).get<float>();
    }
    return result;
}

const char* DisplayModeName(SurfaceDisplayMode value)
{
    switch (value)
    {
    case SurfaceDisplayMode::LitTextured:
        return "lit-textured";
    case SurfaceDisplayMode::TextureOnly:
        return "texture-only";
    case SurfaceDisplayMode::LitUntextured:
        return "lit-untextured";
    }
    throw std::invalid_argument("Unknown surface display mode");
}

const char* FilterName(MaterialFilter value)
{
    switch (value)
    {
    case MaterialFilter::Point:
        return "point";
    case MaterialFilter::Linear:
        return "linear";
    case MaterialFilter::Anisotropic:
        return "anisotropic";
    }
    throw std::invalid_argument("Unknown material filter");
}

const char* AddressName(MaterialAddressMode value)
{
    switch (value)
    {
    case MaterialAddressMode::Wrap:
        return "wrap";
    case MaterialAddressMode::Clamp:
        return "clamp";
    case MaterialAddressMode::Mirror:
        return "mirror";
    }
    throw std::invalid_argument("Unknown material address mode");
}

template <typename Enum> Enum ReadEnum(const std::string& value);

template <> SurfaceDisplayMode ReadEnum(const std::string& value)
{
    if (value == "lit-textured")
        return SurfaceDisplayMode::LitTextured;
    if (value == "texture-only")
        return SurfaceDisplayMode::TextureOnly;
    if (value == "lit-untextured")
        return SurfaceDisplayMode::LitUntextured;
    throw std::runtime_error("Unknown surface display mode: " + value);
}

template <> MaterialFilter ReadEnum(const std::string& value)
{
    if (value == "point")
        return MaterialFilter::Point;
    if (value == "linear")
        return MaterialFilter::Linear;
    if (value == "anisotropic")
        return MaterialFilter::Anisotropic;
    throw std::runtime_error("Unknown material filter: " + value);
}

template <> MaterialAddressMode ReadEnum(const std::string& value)
{
    if (value == "wrap")
        return MaterialAddressMode::Wrap;
    if (value == "clamp")
        return MaterialAddressMode::Clamp;
    if (value == "mirror")
        return MaterialAddressMode::Mirror;
    throw std::runtime_error("Unknown material address mode: " + value);
}

Json SerializeGeometry(const Entity& entity, const std::filesystem::path& sceneDirectory)
{
    if (const MeshGeometry* mesh = entity.Mesh())
    {
        return {{"kind", "mesh"},
                {"assetPath", StoreResourcePath(mesh->assetPath, sceneDirectory)},
                {"assetEntityIndex", mesh->assetEntityIndex}};
    }
    const SolidGeometry& solid = *entity.Solid();
    Json parameters;
    std::string type;
    std::visit(
        [&](const auto& value)
        {
            using Parameters = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<Parameters, CubeParameters>)
            {
                type = "cube";
                parameters = {{"size", Vector(value.size)}};
            }
            else if constexpr (std::is_same_v<Parameters, SphereParameters>)
            {
                type = "sphere";
                parameters = {{"radius", value.radius}, {"slices", value.slices}, {"stacks", value.stacks}};
            }
            else
            {
                type = "plane";
                parameters = {{"size", Vector(value.size)},
                              {"subdivisionsX", value.subdivisionsX},
                              {"subdivisionsZ", value.subdivisionsZ}};
            }
        },
        solid.Parameters());
    return {{"kind", "solid"}, {"type", type}, {"parameters", std::move(parameters)}};
}

Json SerializeMaterial(const EntityMaterial& material, const std::filesystem::path& sceneDirectory)
{
    return {{"baseColor", Vector(material.baseColor)},
            {"diffuseStrength", material.diffuseStrength},
            {"specularColor", Vector(material.specularColor)},
            {"specularStrength", material.specularStrength},
            {"shininess", material.shininess},
            {"doubleSided", material.doubleSided},
            {"displayMode", DisplayModeName(material.displayMode)},
            {"useSourceTexture", material.useSourceTexture},
            {"baseColorTexturePath", StoreResourcePath(material.baseColorTexturePath, sceneDirectory)},
            {"filter", FilterName(material.filter)},
            {"addressMode", AddressName(material.addressMode)}};
}

SolidGeometry DeserializeSolid(const Json& geometry)
{
    const std::string type = geometry.at("type").get<std::string>();
    const Json& parameters = geometry.at("parameters");
    if (type == "cube")
    {
        const auto size = ReadVector<3>(parameters.at("size"), "cube.size");
        return SolidGeometry::Cube({{size[0], size[1], size[2]}});
    }
    if (type == "sphere")
    {
        return SolidGeometry::Sphere({parameters.at("radius").get<float>(),
                                      parameters.at("slices").get<std::uint16_t>(),
                                      parameters.at("stacks").get<std::uint16_t>()});
    }
    if (type == "plane")
    {
        const auto size = ReadVector<2>(parameters.at("size"), "plane.size");
        return SolidGeometry::Plane({{size[0], size[1]},
                                     parameters.at("subdivisionsX").get<std::uint16_t>(),
                                     parameters.at("subdivisionsZ").get<std::uint16_t>()});
    }
    throw std::runtime_error("Unknown solid type: " + type);
}

EntityMaterial DeserializeMaterial(const Json& value, const std::filesystem::path& sceneDirectory)
{
    EntityMaterial material;
    const auto base = ReadVector<4>(value.at("baseColor"), "material.baseColor");
    const auto specular = ReadVector<4>(value.at("specularColor"), "material.specularColor");
    material.baseColor = {base[0], base[1], base[2], base[3]};
    material.diffuseStrength = value.at("diffuseStrength").get<float>();
    material.specularColor = {specular[0], specular[1], specular[2], specular[3]};
    material.specularStrength = value.at("specularStrength").get<float>();
    material.shininess = value.at("shininess").get<float>();
    material.doubleSided = value.at("doubleSided").get<bool>();
    material.displayMode = ReadEnum<SurfaceDisplayMode>(value.at("displayMode").get<std::string>());
    material.useSourceTexture = value.at("useSourceTexture").get<bool>();
    const auto texture = Utf8Path(value.at("baseColorTexturePath").get<std::string>());
    if (!texture.empty())
    {
        material.baseColorTexturePath = ResolveResourcePath(texture, sceneDirectory);
    }
    material.filter = ReadEnum<MaterialFilter>(value.at("filter").get<std::string>());
    material.addressMode = ReadEnum<MaterialAddressMode>(value.at("addressMode").get<std::string>());
    return material;
}

Entity DeserializeEntity(const Json& value, const std::filesystem::path& sceneDirectory)
{
    Entity entity;
    entity.id = value.at("id").get<EntityId>();
    entity.name = value.at("name").get<std::string>();
    const Json& transform = value.at("transform");
    const auto position = ReadVector<3>(transform.at("position"), "transform.position");
    const auto rotation = ReadVector<3>(transform.at("rotationDegrees"), "transform.rotationDegrees");
    const auto scale = ReadVector<3>(transform.at("scale"), "transform.scale");
    entity.transform.position = {position[0], position[1], position[2]};
    entity.transform.rotationDegrees = {rotation[0], rotation[1], rotation[2]};
    entity.transform.scale = {scale[0], scale[1], scale[2]};
    entity.EntityMaterialData() = DeserializeMaterial(value.at("material"), sceneDirectory);

    const Json& geometry = value.at("geometry");
    const std::string kind = geometry.at("kind").get<std::string>();
    if (kind == "solid")
    {
        entity.geometry = DeserializeSolid(geometry);
    }
    else if (kind == "mesh")
    {
        entity.geometry =
            MeshGeometry{ResolveResourcePath(Utf8Path(geometry.at("assetPath").get<std::string>()), sceneDirectory),
                         geometry.at("assetEntityIndex").get<std::uint32_t>()};
    }
    else
    {
        throw std::runtime_error("Unknown entity geometry kind: " + kind);
    }
    return entity;
}

Json SerializeScene(const Scene& scene, const std::filesystem::path& sceneDirectory)
{
    Json models = Json::array();
    for (const Model& model : scene.Models())
    {
        Json entities = Json::array();
        for (const Entity& entity : model.entities)
        {
            entities.push_back({{"id", entity.id},
                                {"name", entity.name},
                                {"geometry", SerializeGeometry(entity, sceneDirectory)},
                                {"transform",
                                 {{"position", Vector(entity.transform.position)},
                                  {"rotationDegrees", Vector(entity.transform.rotationDegrees)},
                                  {"scale", Vector(entity.transform.scale)}}},
                                {"material", SerializeMaterial(entity.EffectiveMaterial(), sceneDirectory)}});
        }
        models.push_back({{"id", model.id}, {"name", model.name}, {"entities", entities}});
    }
    return {{"format", "LRenderScene"}, {"version", 1}, {"models", models}};
}

} // namespace

void SceneSerializer::Save(const Scene& scene, const std::filesystem::path& path)
{
    if (path.empty())
    {
        throw std::invalid_argument("Scene save path must not be empty");
    }
    const std::filesystem::path absolutePath = std::filesystem::absolute(path).lexically_normal();
    const std::filesystem::path directory = absolutePath.parent_path();
    if (!std::filesystem::is_directory(directory))
    {
        throw std::runtime_error("Scene save directory does not exist: " + PathUtf8(directory));
    }
    std::filesystem::path temporary = absolutePath;
    temporary += L".tmp";
    try
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        if (!output)
        {
            throw std::runtime_error("Failed to open temporary scene file for writing");
        }
        output << SerializeScene(scene, directory).dump(2) << '\n';
        output.close();
        if (!output)
        {
            throw std::runtime_error("Failed while writing temporary scene file");
        }
        if (MoveFileExW(temporary.c_str(), absolutePath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) ==
            FALSE)
        {
            throw std::system_error(static_cast<int>(GetLastError()), std::system_category(),
                                    "Failed to replace scene file");
        }
    }
    catch (...)
    {
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        throw;
    }
}

Scene SceneSerializer::Load(const std::filesystem::path& path)
{
    if (!std::filesystem::is_regular_file(path))
    {
        throw std::runtime_error("Scene file does not exist: " + PathUtf8(path));
    }
    try
    {
        std::ifstream input(path, std::ios::binary);
        if (!input)
        {
            throw std::runtime_error("Failed to open scene file");
        }
        const Json root = Json::parse(input);
        if (root.at("format").get<std::string>() != "LRenderScene")
        {
            throw std::runtime_error("File is not an LRender scene");
        }
        const int version = root.at("version").get<int>();
        if (version != 1)
        {
            throw std::runtime_error("Unsupported LRender scene version: " + std::to_string(version));
        }

        Scene scene;
        const std::filesystem::path directory = std::filesystem::absolute(path).lexically_normal().parent_path();
        for (const Json& modelValue : root.at("models"))
        {
            Model model;
            model.id = modelValue.at("id").get<ModelId>();
            model.name = modelValue.at("name").get<std::string>();
            for (const Json& entityValue : modelValue.at("entities"))
            {
                model.entities.push_back(DeserializeEntity(entityValue, directory));
            }
            scene.AddModel(std::move(model));
        }
        return scene;
    }
    catch (const std::exception& error)
    {
        throw std::runtime_error("Failed to load scene " + PathUtf8(path) + ": " + error.what());
    }
}

} // namespace lrender
