/**
 * @file Graphics-API-independent material implementation.
 */
#include "core/Material.h"

#include <cmath>
#include <utility>

namespace lrender
{

Material::Material(std::string name) : m_name(std::move(name))
{
}

const std::string& Material::GetName() const noexcept
{
    return m_name;
}

void Material::SetName(std::string name)
{
    m_name = std::move(name);
}

const DirectX::SimpleMath::Color& Material::GetBaseColor() const noexcept
{
    return m_baseColor;
}

void Material::SetBaseColor(const DirectX::SimpleMath::Color& value) noexcept
{
    m_baseColor = value;
}

float Material::GetDiffuseStrength() const noexcept
{
    return m_diffuseStrength;
}

void Material::SetDiffuseStrength(float value) noexcept
{
    m_diffuseStrength = value;
}

const DirectX::SimpleMath::Color& Material::GetSpecularColor() const noexcept
{
    return m_specularColor;
}

void Material::SetSpecularColor(const DirectX::SimpleMath::Color& value) noexcept
{
    m_specularColor = value;
}

float Material::GetSpecularStrength() const noexcept
{
    return m_specularStrength;
}

void Material::SetSpecularStrength(float value) noexcept
{
    m_specularStrength = value;
}

float Material::GetShininess() const noexcept
{
    return m_shininess;
}

void Material::SetShininess(float value) noexcept
{
    m_shininess = value;
}

bool Material::IsDoubleSided() const noexcept
{
    return m_doubleSided;
}

void Material::SetDoubleSided(bool value) noexcept
{
    m_doubleSided = value;
}

SurfaceDisplayMode Material::GetDisplayMode() const noexcept
{
    return m_displayMode;
}

void Material::SetDisplayMode(SurfaceDisplayMode value) noexcept
{
    m_displayMode = value;
}

MaterialTextureSource Material::GetTextureSource() const noexcept
{
    return m_textureSource;
}

void Material::SetTextureSource(MaterialTextureSource value) noexcept
{
    m_textureSource = value;
}

const std::filesystem::path& Material::GetBaseColorTexturePath() const noexcept
{
    return m_baseColorTexturePath;
}

void Material::SetBaseColorTexturePath(std::filesystem::path path)
{
    m_baseColorTexturePath = std::move(path);
    m_embeddedBaseColorTextureKey.clear();
}

const std::string& Material::GetEmbeddedBaseColorTextureKey() const noexcept
{
    return m_embeddedBaseColorTextureKey;
}

void Material::SetEmbeddedBaseColorTextureKey(std::string key)
{
    m_embeddedBaseColorTextureKey = std::move(key);
    m_baseColorTexturePath.clear();
}

void Material::ClearBaseColorTexture() noexcept
{
    m_baseColorTexturePath.clear();
    m_embeddedBaseColorTextureKey.clear();
}

bool Material::HasBaseColorTexture() const noexcept
{
    return !m_baseColorTexturePath.empty() || !m_embeddedBaseColorTextureKey.empty();
}

bool Material::UsesBaseColorTexture() const noexcept
{
    return m_displayMode != SurfaceDisplayMode::LitUntextured && HasBaseColorTexture();
}

MaterialFilter Material::GetFilter() const noexcept
{
    return m_filter;
}

void Material::SetFilter(MaterialFilter value) noexcept
{
    m_filter = value;
}

MaterialAddressMode Material::GetAddressMode() const noexcept
{
    return m_addressModeU;
}

MaterialAddressMode Material::GetAddressModeU() const noexcept
{
    return m_addressModeU;
}

MaterialAddressMode Material::GetAddressModeV() const noexcept
{
    return m_addressModeV;
}

void Material::SetAddressMode(MaterialAddressMode value) noexcept
{
    SetAddressModes(value, value);
}

void Material::SetAddressModes(MaterialAddressMode addressU, MaterialAddressMode addressV) noexcept
{
    m_addressModeU = addressU;
    m_addressModeV = addressV;
}

bool Material::NearlyEquals(const Material& other, float epsilon) const
{
    const auto colorNear = [epsilon](const DirectX::SimpleMath::Color& left,
                                     const DirectX::SimpleMath::Color& right)
    {
        return std::abs(left.x - right.x) <= epsilon && std::abs(left.y - right.y) <= epsilon &&
               std::abs(left.z - right.z) <= epsilon && std::abs(left.w - right.w) <= epsilon;
    };
    return m_name == other.m_name && colorNear(m_baseColor, other.m_baseColor) &&
           std::abs(m_diffuseStrength - other.m_diffuseStrength) <= epsilon &&
           colorNear(m_specularColor, other.m_specularColor) &&
           std::abs(m_specularStrength - other.m_specularStrength) <= epsilon &&
           std::abs(m_shininess - other.m_shininess) <= epsilon && m_doubleSided == other.m_doubleSided &&
           m_displayMode == other.m_displayMode && m_textureSource == other.m_textureSource &&
           m_baseColorTexturePath == other.m_baseColorTexturePath &&
           m_embeddedBaseColorTextureKey == other.m_embeddedBaseColorTextureKey && m_filter == other.m_filter &&
           m_addressModeU == other.m_addressModeU && m_addressModeV == other.m_addressModeV;
}

} // namespace lrender
