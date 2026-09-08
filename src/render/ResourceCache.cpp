/**
 * @file D3D11 resource cache implementation.
 * @author Codex
 * @created 2026-08-21
 * @depends render/ResourceCache.h, render/ModelLoader.h
 */
#include "render/ResourceCache.h"

#include "render/ModelLoader.h"

#include <algorithm>
#include <cwctype>
#include <stdexcept>

namespace lrender
{

ResourceCache::ResourceCache(ID3D11Device* device, ID3D11DeviceContext* context) : m_device(device), m_context(context)
{
    if (m_device == nullptr || m_context == nullptr)
    {
        throw std::invalid_argument("ResourceCache requires a D3D11 device and context");
    }
    m_whiteTexture = Texture2D::CreateSolidWhite(m_device);
    m_checkerTexture = Texture2D::CreateChecker(m_device);
    m_defaultSampler = GetSampler();
    m_modelLoader = std::make_unique<ModelLoader>();
}

ResourceCache::~ResourceCache() = default;

std::shared_ptr<MeshAsset> ResourceCache::LoadMeshAsset(const std::filesystem::path& path)
{
    const std::wstring key = NormalizePath(path);
    if (const auto found = m_meshAssets.find(key); found != m_meshAssets.end())
    {
        return found->second;
    }
    auto asset = m_modelLoader->Load(path, *this);
    m_meshAssets.emplace(key, asset);
    return asset;
}

std::shared_ptr<Texture2D> ResourceCache::LoadTexture(const std::filesystem::path& path)
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

std::shared_ptr<Texture2D> ResourceCache::LoadEmbeddedTexture(std::string key, std::span<const std::byte> bytes)
{
    if (const auto found = m_embeddedTextures.find(key); found != m_embeddedTextures.end())
    {
        return found->second;
    }
    auto texture = Texture2D::LoadMemory(m_device, m_context, bytes, key);
    m_embeddedTextures.emplace(std::move(key), texture);
    return texture;
}

std::shared_ptr<SamplerState> ResourceCache::GetSampler(const SamplerDescription& description)
{
    const std::uint64_t key = SamplerKey(description);
    if (const auto found = m_samplers.find(key); found != m_samplers.end())
    {
        return found->second;
    }
    auto sampler = std::make_shared<SamplerState>(m_device, description);
    m_samplers.emplace(key, sampler);
    return sampler;
}

Material ResourceCache::DefaultMaterial() const
{
    Material material;
    material.SetName("Default white");
    material.SetBaseColorTexture(m_whiteTexture);
    material.SetSampler(m_defaultSampler);
    return material;
}

Material ResourceCache::CheckerMaterial() const
{
    Material material = DefaultMaterial();
    material.SetName("Generated checker");
    material.SetBaseColorTexture(m_checkerTexture);
    return material;
}

std::wstring ResourceCache::NormalizePath(const std::filesystem::path& path)
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

std::uint64_t ResourceCache::SamplerKey(const SamplerDescription& description) noexcept
{
    return static_cast<std::uint64_t>(description.filter) | (static_cast<std::uint64_t>(description.addressU) << 16U) |
           (static_cast<std::uint64_t>(description.addressV) << 24U);
}

std::size_t ResourceCache::MeshAssetCount() const noexcept
{
    return m_meshAssets.size();
}

std::size_t ResourceCache::TextureCount() const noexcept
{
    return m_textures.size() + m_embeddedTextures.size();
}

ID3D11Device* ResourceCache::Device() const noexcept
{
    return m_device;
}

ID3D11DeviceContext* ResourceCache::Context() const noexcept
{
    return m_context;
}

} // namespace lrender
