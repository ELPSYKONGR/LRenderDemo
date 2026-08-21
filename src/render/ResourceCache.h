/**
 * @file D3D11 model, texture, and sampler resource cache.
 * @author Codex
 * @created 2026-08-21
 * @depends render/Model.h, render/Texture2D.h, render/SamplerState.h
 */
#pragma once

#include "render/Model.h"

#include <cstddef>
#include <d3d11.h>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>

namespace lrender {

class ResourceCache final {
public:
    ResourceCache(ID3D11Device* device, ID3D11DeviceContext* context);

    [[nodiscard]] std::shared_ptr<Model> LoadModel(const std::filesystem::path& path);
    [[nodiscard]] std::shared_ptr<Texture2D> LoadTexture(const std::filesystem::path& path);
    [[nodiscard]] std::shared_ptr<Texture2D> LoadEmbeddedTexture(
        std::string key, std::span<const std::byte> bytes);
    [[nodiscard]] std::shared_ptr<SamplerState> GetSampler(
        const SamplerDescription& description = {});
    [[nodiscard]] Material DefaultMaterial() const;
    [[nodiscard]] Material CheckerMaterial() const;

    [[nodiscard]] std::size_t ModelCount() const noexcept { return models_.size(); }
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
    std::unordered_map<std::wstring, std::shared_ptr<Model>> models_;
    std::unordered_map<std::wstring, std::shared_ptr<Texture2D>> textures_;
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> embeddedTextures_;
    std::unordered_map<std::uint64_t, std::shared_ptr<SamplerState>> samplers_;
    std::shared_ptr<Texture2D> whiteTexture_;
    std::shared_ptr<Texture2D> checkerTexture_;
    std::shared_ptr<SamplerState> defaultSampler_;
};

} // namespace lrender
