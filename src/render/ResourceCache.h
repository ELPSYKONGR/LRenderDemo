/**
 * @file D3D11 mesh asset cache.
 * @author Codex
 * @created 2026-08-21
 * @depends render/MeshAsset.h
 */
#pragma once

#include "render/MeshAsset.h"

#include <cstddef>
#include <d3d11.h>
#include <filesystem>
#include <memory>
#include <unordered_map>

namespace lrender
{

class ModelLoader;

class ResourceCache final
{
  public:
    explicit ResourceCache(ID3D11Device* device);
    ~ResourceCache();

    [[nodiscard]] std::shared_ptr<MeshAsset> LoadMeshAsset(const std::filesystem::path& path);
    [[nodiscard]] std::size_t MeshAssetCount() const noexcept;
    [[nodiscard]] ID3D11Device* Device() const noexcept;

  private:
    static std::wstring NormalizePath(const std::filesystem::path& path);

    ID3D11Device* m_device = nullptr;
    std::unique_ptr<ModelLoader> m_modelLoader;
    std::unordered_map<std::wstring, std::shared_ptr<MeshAsset>> m_meshAssets;
};

} // namespace lrender
