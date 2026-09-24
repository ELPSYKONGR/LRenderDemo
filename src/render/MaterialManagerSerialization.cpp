/**
 * @file Material JSON conversion implementation.
 */
#include "render/MaterialManager.h"

#include <nlohmann/json.hpp>

#include <array>
#include <stdexcept>

namespace lrender
{
namespace
{

using Json = nlohmann::json;

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
    const std::filesystem::path absolute = value.is_absolute()
                                               ? value.lexically_normal()
                                               : std::filesystem::absolute(value).lexically_normal();
    const std::filesystem::path relative = absolute.lexically_relative(sceneDirectory);
    return PathUtf8(relative.empty() ? absolute : relative);
}

Json ColorValue(const DirectX::SimpleMath::Color& value)
{
    return Json::array({value.x, value.y, value.z, value.w});
}

std::array<float, 4> ReadColor(const Json& value, const char* field)
{
    if (!value.is_array() || value.size() != 4)
    {
        throw std::runtime_error(std::string(field) + " must be a numeric array");
    }
    return {value.at(0).get<float>(), value.at(1).get<float>(), value.at(2).get<float>(), value.at(3).get<float>()};
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

SurfaceDisplayMode ReadDisplayMode(const std::string& value)
{
    if (value == "lit-textured")
    {
        return SurfaceDisplayMode::LitTextured;
    }
    if (value == "texture-only")
    {
        return SurfaceDisplayMode::TextureOnly;
    }
    if (value == "lit-untextured")
    {
        return SurfaceDisplayMode::LitUntextured;
    }
    throw std::runtime_error("Unknown surface display mode: " + value);
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

MaterialFilter ReadFilter(const std::string& value)
{
    if (value == "point")
    {
        return MaterialFilter::Point;
    }
    if (value == "linear")
    {
        return MaterialFilter::Linear;
    }
    if (value == "anisotropic")
    {
        return MaterialFilter::Anisotropic;
    }
    throw std::runtime_error("Unknown material filter: " + value);
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

MaterialAddressMode ReadAddressMode(const std::string& value)
{
    if (value == "wrap")
    {
        return MaterialAddressMode::Wrap;
    }
    if (value == "clamp")
    {
        return MaterialAddressMode::Clamp;
    }
    if (value == "mirror")
    {
        return MaterialAddressMode::Mirror;
    }
    throw std::runtime_error("Unknown material address mode: " + value);
}

} // namespace

std::string MaterialManager::SerializeMaterial(const Material& material,
                                               const std::filesystem::path& sceneDirectory)
{
    const Json value = {{"name", material.GetName()},
                        {"baseColor", ColorValue(material.GetBaseColor())},
                        {"diffuseStrength", material.GetDiffuseStrength()},
                        {"specularColor", ColorValue(material.GetSpecularColor())},
                        {"specularStrength", material.GetSpecularStrength()},
                        {"shininess", material.GetShininess()},
                        {"doubleSided", material.IsDoubleSided()},
                        {"displayMode", DisplayModeName(material.GetDisplayMode())},
                        {"useSourceTexture", material.GetTextureSource() == MaterialTextureSource::Source},
                        {"baseColorTexturePath", StoreResourcePath(material.GetBaseColorTexturePath(), sceneDirectory)},
                        {"embeddedBaseColorTextureKey", material.GetEmbeddedBaseColorTextureKey()},
                        {"filter", FilterName(material.GetFilter())},
                        {"addressMode", AddressName(material.GetAddressMode())},
                        {"addressModeU", AddressName(material.GetAddressModeU())},
                        {"addressModeV", AddressName(material.GetAddressModeV())}};
    return value.dump();
}

Material MaterialManager::DeserializeMaterial(std::string_view text, const std::filesystem::path& sceneDirectory)
{
    const Json value = Json::parse(text);
    Material material;
    const std::array<float, 4> base = ReadColor(value.at("baseColor"), "material.baseColor");
    const std::array<float, 4> specular = ReadColor(value.at("specularColor"), "material.specularColor");
    material.SetName(value.value("name", "Default"));
    material.SetBaseColor({base[0], base[1], base[2], base[3]});
    material.SetDiffuseStrength(value.at("diffuseStrength").get<float>());
    material.SetSpecularColor({specular[0], specular[1], specular[2], specular[3]});
    material.SetSpecularStrength(value.at("specularStrength").get<float>());
    material.SetShininess(value.at("shininess").get<float>());
    material.SetDoubleSided(value.at("doubleSided").get<bool>());
    material.SetDisplayMode(ReadDisplayMode(value.at("displayMode").get<std::string>()));
    material.SetTextureSource(value.at("useSourceTexture").get<bool>() ? MaterialTextureSource::Source
                                                                       : MaterialTextureSource::Custom);
    const std::filesystem::path texture = Utf8Path(value.at("baseColorTexturePath").get<std::string>());
    if (!texture.empty())
    {
        material.SetBaseColorTexturePath(ResolveResourcePath(texture, sceneDirectory));
    }
    else if (const std::string embeddedKey = value.value("embeddedBaseColorTextureKey", ""); !embeddedKey.empty())
    {
        material.SetEmbeddedBaseColorTextureKey(embeddedKey);
    }
    material.SetFilter(ReadFilter(value.at("filter").get<std::string>()));
    const std::string addressMode = value.at("addressMode").get<std::string>();
    material.SetAddressModes(ReadAddressMode(value.value("addressModeU", addressMode)),
                             ReadAddressMode(value.value("addressModeV", addressMode)));
    return material;
}

} // namespace lrender
