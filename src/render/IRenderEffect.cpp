/**
 * @file Common DX11 render effect helpers.
 */
#include "render/IRenderEffect.h"

#include <d3dcompiler.h>
#include <filesystem>
#include <stdexcept>
#include <utility>

namespace lrender {

EffectResource::EffectResource(ID3D11Device* device, ID3D11DeviceContext* context)
    : m_device(device), m_context(context) {
    if (m_device == nullptr || m_context == nullptr) {
        throw std::invalid_argument("EffectResource requires a D3D11 device and context");
    }
}

std::shared_ptr<Texture2D> EffectResource::CreateTexture(
    std::string name, const std::filesystem::path& path, bool forceSrgb) {
    if (name.empty()) {
        throw std::invalid_argument("Effect texture name must not be empty");
    }
    if (m_textures.contains(name) || m_renderTargets.contains(name)) {
        throw std::invalid_argument("Effect resource name already exists: " + name);
    }
    auto texture = Texture2D::LoadFile(m_device, m_context, path, forceSrgb);
    m_textures.emplace(name, texture);
    return texture;
}

std::shared_ptr<Texture2D> EffectResource::AddTexture(
    std::string name, std::shared_ptr<Texture2D> texture) {
    if (name.empty() || texture == nullptr) {
        throw std::invalid_argument("Effect texture name or value is invalid");
    }
    if (m_textures.contains(name) || m_renderTargets.contains(name)) {
        throw std::invalid_argument("Effect resource name already exists: " + name);
    }
    m_textures.emplace(name, texture);
    return texture;
}

RenderTarget& EffectResource::CreateRenderTarget(
    std::string name, std::uint32_t width, std::uint32_t height) {
    if (name.empty()) {
        throw std::invalid_argument("Effect render target name must not be empty");
    }
    if (m_textures.contains(name) || m_renderTargets.contains(name)) {
        throw std::invalid_argument("Effect resource name already exists: " + name);
    }
    auto target = std::make_unique<RenderTarget>();
    target->Resize(m_device, width, height);
    RenderTarget& result = *target;
    m_renderTargets.emplace(std::move(name), std::move(target));
    return result;
}

Texture2D* EffectResource::GetTexture(std::string_view name) noexcept {
    const auto iterator = m_textures.find(std::string(name));
    return iterator == m_textures.end() ? nullptr : iterator->second.get();
}

RenderTarget* EffectResource::GetRenderTarget(std::string_view name) noexcept {
    const auto iterator = m_renderTargets.find(std::string(name));
    return iterator == m_renderTargets.end() ? nullptr : iterator->second.get();
}

ID3D11ShaderResourceView* EffectResource::GetShaderResource(
    std::string_view name) const noexcept {
    const auto texture = m_textures.find(std::string(name));
    if (texture != m_textures.end()) {
        return texture->second->ShaderResourceView();
    }
    const auto target = m_renderTargets.find(std::string(name));
    return target == m_renderTargets.end() ? nullptr : target->second->GetShaderResourceView();
}

void EffectResource::Resize(std::uint32_t width, std::uint32_t height) {
    for (auto& [name, target] : m_renderTargets) {
        target->Resize(m_device, width, height);
    }
}

void EffectResource::Clear() noexcept {
    m_renderTargets.clear();
    m_textures.clear();
}

IRenderEffect::IRenderEffect(ID3D11Device* device, ID3D11DeviceContext* context)
    : m_device(device), m_context(context), m_resources(device, context) {
    if (m_device == nullptr || m_context == nullptr) {
        throw std::invalid_argument("Render effect requires a D3D11 device and context");
    }
}

Microsoft::WRL::ComPtr<ID3DBlob> IRenderEffect::LoadShader(
    const std::filesystem::path& path) {
    Microsoft::WRL::ComPtr<ID3DBlob> shader;
    const HRESULT result = D3DReadFileToBlob(path.c_str(), shader.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to load compiled shader: " + path.string());
    }
    return shader;
}

} // namespace lrender
