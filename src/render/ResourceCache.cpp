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

ResourceCache::ResourceCache(ID3D11Device* device) : m_device(device)
{
    if (m_device == nullptr)
    {
        throw std::invalid_argument("ResourceCache requires a D3D11 device");
    }
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

std::size_t ResourceCache::MeshAssetCount() const noexcept
{
    return m_meshAssets.size();
}

ID3D11Device* ResourceCache::Device() const noexcept
{
    return m_device;
}

} // namespace lrender
