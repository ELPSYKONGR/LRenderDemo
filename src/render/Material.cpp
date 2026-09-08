/**
 * @file GPU material accessors and construction.
 */
#include "render/Material.h"

#include <utility>

namespace lrender
{

Material::Material() = default;

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

const DirectX::SimpleMath::Color& Material::GetBaseColorFactor() const noexcept
{
    return m_baseColorFactor;
}

void Material::SetBaseColorFactor(const DirectX::SimpleMath::Color& value) noexcept
{
    m_baseColorFactor = value;
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

const std::shared_ptr<Texture2D>& Material::GetBaseColorTexture() const noexcept
{
    return m_baseColorTexture;
}

void Material::SetBaseColorTexture(std::shared_ptr<Texture2D> value) noexcept
{
    m_baseColorTexture = std::move(value);
}

bool Material::UsesBaseColorTexture() const noexcept
{
    return m_displayMode != SurfaceDisplayMode::LitUntextured;
}

const std::shared_ptr<SamplerState>& Material::GetSampler() const noexcept
{
    return m_sampler;
}

void Material::SetSampler(std::shared_ptr<SamplerState> value) noexcept
{
    m_sampler = std::move(value);
}

} // namespace lrender
