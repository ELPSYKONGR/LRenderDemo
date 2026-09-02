/**
 * @file Render-effect boundary used by scene drawing code.
 * @author Codex
 * @created 2026-08-20
 * @depends render/EffectContext.h
 */
#pragma once

#include "render/EffectContext.h"
#include "render/RenderTarget.h"
#include "render/Texture2D.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <wrl/client.h>

namespace lrender
{

class EffectResource final
{
  public:
    EffectResource(ID3D11Device* device, ID3D11DeviceContext* context);

    [[nodiscard]] std::shared_ptr<Texture2D> CreateTexture(std::string name, const std::filesystem::path& path,
                                                           bool forceSrgb = true);
    [[nodiscard]] std::shared_ptr<Texture2D> AddTexture(std::string name, std::shared_ptr<Texture2D> texture);
    [[nodiscard]] RenderTarget& CreateRenderTarget(std::string name, std::uint32_t width, std::uint32_t height);

    [[nodiscard]] Texture2D* GetTexture(std::string_view name) noexcept;
    [[nodiscard]] RenderTarget* GetRenderTarget(std::string_view name) noexcept;
    [[nodiscard]] ID3D11ShaderResourceView* GetShaderResource(std::string_view name) const noexcept;

    void Resize(std::uint32_t width, std::uint32_t height);
    void Clear() noexcept;

  private:
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_textures;
    std::unordered_map<std::string, std::unique_ptr<RenderTarget>> m_renderTargets;
};

class IRenderEffect
{
  public:
    IRenderEffect(ID3D11Device* device, ID3D11DeviceContext* context);
    virtual ~IRenderEffect() = default;

    IRenderEffect(const IRenderEffect&) = delete;
    IRenderEffect& operator=(const IRenderEffect&) = delete;

    virtual void Bind(const EffectFrameContext& frame, const EffectDrawContext& draw) = 0;

    // High-level draw entry point. Effects with multiple passes can override this
    // method and keep pass ordering and resource transitions private.
    virtual void Draw(const EffectFrameContext& frame, const EffectDrawContext& draw);

    [[nodiscard]] virtual std::string_view Name() const noexcept = 0;

  protected:
    [[nodiscard]] ID3D11Device* Device() const noexcept;
    [[nodiscard]] ID3D11DeviceContext* DeviceContext() const noexcept;
    [[nodiscard]] EffectResource& Resources() noexcept;
    [[nodiscard]] const EffectResource& Resources() const noexcept;
    [[nodiscard]] static Microsoft::WRL::ComPtr<ID3DBlob> LoadShader(const std::filesystem::path& path);

  private:
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;
    EffectResource m_resources;
};

} // namespace lrender
