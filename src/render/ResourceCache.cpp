/**
 * @file D3D11 resource cache implementation.
 * @author Codex
 * @created 2026-08-21
 * @depends render/ResourceCache.h, render/GltfLoader.h
 */
#include "render/ResourceCache.h"

#include "render/GltfLoader.h"

#include <algorithm>
#include <cwctype>
#include <stdexcept>

namespace lrender {

ResourceCache::ResourceCache(ID3D11Device* device, ID3D11DeviceContext* context)
    : device_(device), context_(context) {
    if (device_ == nullptr || context_ == nullptr) {
        throw std::invalid_argument("ResourceCache requires a D3D11 device and context");
    }
    whiteTexture_ = Texture2D::CreateSolidWhite(device_);
    checkerTexture_ = Texture2D::CreateChecker(device_);
    defaultSampler_ = GetSampler();
}

std::shared_ptr<Model> ResourceCache::LoadModel(const std::filesystem::path& path) {
    const std::wstring key = NormalizePath(path);
    if (const auto found = models_.find(key); found != models_.end()) {
        return found->second;
    }
    auto model = GltfLoader::Load(path, *this);
    models_.emplace(key, model);
    return model;
}

std::shared_ptr<Texture2D> ResourceCache::LoadTexture(const std::filesystem::path& path) {
    const std::wstring key = NormalizePath(path);
    if (const auto found = textures_.find(key); found != textures_.end()) {
        return found->second;
    }
    auto texture = Texture2D::LoadFile(device_, context_, path);
    textures_.emplace(key, texture);
    return texture;
}

std::shared_ptr<Texture2D> ResourceCache::LoadEmbeddedTexture(
    std::string key, std::span<const std::byte> bytes) {
    if (const auto found = embeddedTextures_.find(key); found != embeddedTextures_.end()) {
        return found->second;
    }
    auto texture = Texture2D::LoadMemory(device_, context_, bytes, key);
    embeddedTextures_.emplace(std::move(key), texture);
    return texture;
}

std::shared_ptr<SamplerState> ResourceCache::GetSampler(
    const SamplerDescription& description) {
    const std::uint64_t key = SamplerKey(description);
    if (const auto found = samplers_.find(key); found != samplers_.end()) {
        return found->second;
    }
    auto sampler = std::make_shared<SamplerState>(device_, description);
    samplers_.emplace(key, sampler);
    return sampler;
}

Material ResourceCache::DefaultMaterial() const {
    Material material;
    material.name = "Default white";
    material.baseColorTexture = whiteTexture_;
    material.sampler = defaultSampler_;
    return material;
}

Material ResourceCache::CheckerMaterial() const {
    Material material = DefaultMaterial();
    material.name = "Generated checker";
    material.baseColorTexture = checkerTexture_;
    return material;
}

std::wstring ResourceCache::NormalizePath(const std::filesystem::path& path) {
    if (path.empty()) {
        throw std::invalid_argument("Resource path must not be empty");
    }
    std::error_code error;
    auto normalized = std::filesystem::weakly_canonical(path, error);
    if (error) {
        normalized = std::filesystem::absolute(path, error).lexically_normal();
    }
    if (error) {
        throw std::runtime_error("Unable to normalize resource path");
    }
    std::wstring key = normalized.wstring();
    std::ranges::transform(key, key.begin(), ::towlower);
    return key;
}

std::uint64_t ResourceCache::SamplerKey(const SamplerDescription& description) noexcept {
    return static_cast<std::uint64_t>(description.filter) |
           (static_cast<std::uint64_t>(description.addressU) << 16U) |
           (static_cast<std::uint64_t>(description.addressV) << 24U);
}

} // namespace lrender
