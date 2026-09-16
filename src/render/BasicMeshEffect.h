/**
 * @file Editable HLSL effect for lit colored meshes.
 */
#pragma once

#include "render/IRenderEffect.h"

#include <filesystem>
#include <wrl/client.h>

namespace lrender
{

class BasicMeshEffect final : public IRenderEffect
{
  public:
    BasicMeshEffect(ID3D11Device* device, ID3D11DeviceContext* context, const std::filesystem::path& shaderDirectory);

    void Bind(const EffectFrameContext& frame, const EffectDrawContext& draw) override;
    void Draw(const EffectFrameContext& frame, const EffectDrawContext& draw) override;

    [[nodiscard]] std::string_view Name() const noexcept override;
    void SetWireframe(bool isWireframe) noexcept;
    [[nodiscard]] bool IsWireframe() const noexcept;

  private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    bool m_isWireframe = false;
};

} // namespace lrender
