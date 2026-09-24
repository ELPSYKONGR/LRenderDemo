/**
 * @file Material resolution and DX11 resource management implementation.
 */
#include "render/MaterialManager.h"

#include <algorithm>
#include <cwctype>
#include <stdexcept>
#include <utility>

namespace lrender
{
namespace
{

D3D11_FILTER NativeFilter(MaterialFilter filter)
{
    switch (filter)
    {
    case MaterialFilter::Point:
        return D3D11_FILTER_MIN_MAG_MIP_POINT;
    case MaterialFilter::Linear:
        return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    case MaterialFilter::Anisotropic:
        return D3D11_FILTER_ANISOTROPIC;
    }
    throw std::invalid_argument("Unsupported material texture filter");
}

D3D11_TEXTURE_ADDRESS_MODE NativeAddressMode(MaterialAddressMode mode)
{
    switch (mode)
    {
    case MaterialAddressMode::Wrap:
        return D3D11_TEXTURE_ADDRESS_WRAP;
    case MaterialAddressMode::Clamp:
        return D3D11_TEXTURE_ADDRESS_CLAMP;
    case MaterialAddressMode::Mirror:
        return D3D11_TEXTURE_ADDRESS_MIRROR;
    }
    throw std::invalid_argument("Unsupported material texture address mode");
}

} // namespace

std::unique_ptr<MaterialManager> MaterialManager::m_instance;

void MaterialManager::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    if (m_instance != nullptr)
    {
        throw std::logic_error("MaterialManager is already initialized");
    }
    m_instance = std::unique_ptr<MaterialManager>(new MaterialManager(device, context));
}

void MaterialManager::Shutdown() noexcept
{
    m_instance.reset();
}

MaterialManager& MaterialManager::Instance()
{
    if (m_instance == nullptr)
    {
        throw std::logic_error("MaterialManager is not initialized");
    }
    return *m_instance;
}

MaterialManager::MaterialManager(ID3D11Device* device, ID3D11DeviceContext* context)
    : m_device(device), m_context(context)
{
    if (m_device == nullptr || m_context == nullptr)
    {
        throw std::invalid_argument("MaterialManager requires a D3D11 device and context");
    }
    m_whiteTexture = Texture2D::CreateSolidWhite(m_device);
    m_checkerTexture = Texture2D::CreateChecker(m_device);
}

MaterialDrawData MaterialManager::PrepareMaterial(const Material& source, const Material* overrideMaterial)
{
    Material resolved = overrideMaterial != nullptr ? *overrideMaterial : source;
    const bool useSourceTexture = overrideMaterial != nullptr && overrideMaterial->GetTextureSource() == MaterialTextureSource::Source;
    const Material& textureMaterial = useSourceTexture ? source : resolved;
    if (&textureMaterial != &resolved)
    {
        if (!textureMaterial.GetBaseColorTexturePath().empty())
        {
            resolved.SetBaseColorTexturePath(textureMaterial.GetBaseColorTexturePath());
        }
        else if (!textureMaterial.GetEmbeddedBaseColorTextureKey().empty())
        {
            resolved.SetEmbeddedBaseColorTextureKey(textureMaterial.GetEmbeddedBaseColorTextureKey());
        }
        else
        {
            resolved.ClearBaseColorTexture();
        }
    }
    std::shared_ptr<Texture2D> texture = m_whiteTexture;
    if (resolved.GetDisplayMode() != SurfaceDisplayMode::LitUntextured && textureMaterial.HasBaseColorTexture())
    {
        texture = ResolveTexture(textureMaterial);
    }
    else
    {
        resolved.SetDisplayMode(SurfaceDisplayMode::LitUntextured);
    }
    std::shared_ptr<SamplerState> sampler = ResolveSampler(overrideMaterial != nullptr ? *overrideMaterial : source);
    return {std::move(resolved), std::move(texture), std::move(sampler)};
}

std::shared_ptr<Texture2D> MaterialManager::LoadTexture(const std::filesystem::path& path)
{
    const std::wstring key = NormalizePath(path);
    if (const auto found = m_textures.find(key); found != m_textures.end())
    {
        return found->second;
    }
    auto texture = Texture2D::LoadFile(m_device, m_context, path);
    m_textures.emplace(key, texture);
    return texture;
}

void MaterialManager::RegisterEmbeddedTexture(std::string key, std::span<const std::byte> bytes)
{
    if (key.empty())
    {
        throw std::invalid_argument("Embedded texture key must not be empty");
    }
    if (m_embeddedTextures.contains(key))
    {
        return;
    }
    auto texture = Texture2D::LoadMemory(m_device, m_context, bytes, key);
    m_embeddedTextures.emplace(std::move(key), std::move(texture));
}

Material MaterialManager::DefaultMaterial() const
{
    Material material("Default");
    material.SetDisplayMode(SurfaceDisplayMode::LitUntextured);
    return material;
}

Material MaterialManager::CheckerMaterial() const
{
    Material material("Generated checker");
    material.SetDisplayMode(SurfaceDisplayMode::LitTextured);
    material.SetEmbeddedBaseColorTextureKey("LRender.Generated.Checker");
    return material;
}

std::size_t MaterialManager::TextureCount() const noexcept
{
    return m_textures.size() + m_embeddedTextures.size();
}

std::shared_ptr<Texture2D> MaterialManager::ResolveTexture(const Material& material)
{
    if (!material.GetBaseColorTexturePath().empty())
    {
        return LoadTexture(material.GetBaseColorTexturePath());
    }
    const std::string& key = material.GetEmbeddedBaseColorTextureKey();
    if (key == "LRender.Generated.Checker")
    {
        return m_checkerTexture;
    }
    if (const auto found = m_embeddedTextures.find(key); found != m_embeddedTextures.end())
    {
        return found->second;
    }
    throw std::runtime_error("Embedded material texture is not registered: " + key);
}

std::shared_ptr<SamplerState> MaterialManager::ResolveSampler(const Material& material)
{
    SamplerDescription description;
    description.filter = NativeFilter(material.GetFilter());
    description.addressU = NativeAddressMode(material.GetAddressModeU());
    description.addressV = NativeAddressMode(material.GetAddressModeV());
    const std::uint64_t key = SamplerKey(description);
    if (const auto found = m_samplers.find(key); found != m_samplers.end())
    {
        return found->second;
    }
    auto sampler = std::make_shared<SamplerState>(m_device, description);
    m_samplers.emplace(key, sampler);
    return sampler;
}

std::wstring MaterialManager::NormalizePath(const std::filesystem::path& path)
{
    if (path.empty())
    {
        throw std::invalid_argument("Resource path must not be empty");
    }
    std::error_code error;
    auto normalized = std::filesystem::weakly_canonical(path, error);
    if (error)
    {
        normalized = std::filesystem::absolute(path, error).lexically_normal();
    }
    if (error)
    {
        throw std::runtime_error("Unable to normalize resource path");
    }
    std::wstring key = normalized.wstring();
    std::ranges::transform(key, key.begin(), ::towlower);
    return key;
}

std::uint64_t MaterialManager::SamplerKey(const SamplerDescription& description) noexcept
{
    return static_cast<std::uint64_t>(description.filter) | (static_cast<std::uint64_t>(description.addressU) << 16U) |
           (static_cast<std::uint64_t>(description.addressV) << 24U);
}

} // namespace lrender
