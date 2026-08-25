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

namespace lrender {

class ModelLoader;

class ResourceCache final {
public:
    ResourceCache(ID3D11Device* device, ID3D11DeviceContext* context);
    ~ResourceCache();

    [[nodiscard]] std::shared_ptr<MeshAsset> LoadMeshAsset(const std::filesystem::path& path);
    [[nodiscard]] std::shared_ptr<Texture2D> LoadTexture(const std::filesystem::path& path);
    [[nodiscard]] std::shared_ptr<Texture2D> LoadEmbeddedTexture(
        std::string key, std::span<const std::byte> bytes);
    [[nodiscard]] std::shared_ptr<SamplerState> GetSampler(
        const SamplerDescription& description = {});
    [[nodiscard]] Material DefaultMaterial() const;
    [[nodiscard]] Material CheckerMaterial() const;

    [[nodiscard]] std::size_t MeshAssetCount() const noexcept { return meshAssets_.size(); }
    [[nodiscard]] std::size_t TextureCount() const noexcept {
        return textures_.size() + embeddedTextures_.size();
    }

    [[nodiscard]] ID3D11Device* Device() const noexcept { return device_; }
    [[nodiscard]] ID3D11DeviceContext* Context() const noexcept { return context_; }

private:
    static std::wstring NormalizePath(const std::filesystem::path& path);
    static std::uint64_t SamplerKey(const SamplerDescription& description) noexcept;

    ID3D11Device* device_{};
    ID3D11DeviceContext* context_{};
    std::unique_ptr<ModelLoader> modelLoader_;
    std::unordered_map<std::wstring, std::shared_ptr<MeshAsset>> meshAssets_;
    std::unordered_map<std::wstring, std::shared_ptr<Texture2D>> textures_;
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> embeddedTextures_;
    std::unordered_map<std::uint64_t, std::shared_ptr<SamplerState>> samplers_;
    std::shared_ptr<Texture2D> whiteTexture_;
    std::shared_ptr<Texture2D> checkerTexture_;
    std::shared_ptr<SamplerState> defaultSampler_;
};

} // namespace lrender
