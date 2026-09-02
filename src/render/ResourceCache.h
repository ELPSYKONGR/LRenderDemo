/**
 * @file D3D11 model, texture, and sampler resource cache.
 * @author Codex
 * @created 2026-08-21
 * @depends render/MeshAsset.h, render/Texture2D.h, render/SamplerState.h
 */
#pragma once

#include "render/MeshAsset.h"

#include <cstddef>
#include <d3d11.h>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>

namespace lrender
{

class ModelLoader;

class ResourceCache final
{
  public:
    ResourceCache(ID3D11Device* device, ID3D11DeviceContext* context);
    ~ResourceCache();

    [[nodiscard]] std::shared_ptr<MeshAsset> LoadMeshAsset(const std::filesystem::path& path);
    [[nodiscard]] std::shared_ptr<Texture2D> LoadTexture(const std::filesystem::path& path);
    [[nodiscard]] std::shared_ptr<Texture2D> LoadEmbeddedTexture(std::string key, std::span<const std::byte> bytes);
    [[nodiscard]] std::shared_ptr<SamplerState> GetSampler(const SamplerDescription& description = {});
    [[nodiscard]] Material DefaultMaterial() const;
    [[nodiscard]] Material CheckerMaterial() const;

    [[nodiscard]] std::size_t MeshAssetCount() const noexcept;
    [[nodiscard]] std::size_t TextureCount() const noexcept;
    [[nodiscard]] ID3D11Device* Device() const noexcept;
    [[nodiscard]] ID3D11DeviceContext* Context() const noexcept;

  private:
    static std::wstring NormalizePath(const std::filesystem::path& path);
    static std::uint64_t SamplerKey(const SamplerDescription& description) noexcept;

    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;
    std::unique_ptr<ModelLoader> m_modelLoader;
    std::unordered_map<std::wstring, std::shared_ptr<MeshAsset>> m_meshAssets;
    std::unordered_map<std::wstring, std::shared_ptr<Texture2D>> m_textures;
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_embeddedTextures;
    std::unordered_map<std::uint64_t, std::shared_ptr<SamplerState>> m_samplers;
    std::shared_ptr<Texture2D> m_whiteTexture;
    std::shared_ptr<Texture2D> m_checkerTexture;
    std::shared_ptr<SamplerState> m_defaultSampler;
};

} // namespace lrender
