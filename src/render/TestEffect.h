/**
 * @file Test effect skeleton.
 */
#pragma once

#include "render/IRenderEffect.h"

#include <filesystem>
#include <string_view>

namespace lrender
{

class TestEffect final : public IRenderEffect
{
  public:
    TestEffect(ID3D11Device* device, ID3D11DeviceContext* context,
               const std::filesystem::path& shaderDirectory);
    void Bind(const EffectFrameContext& frame, const EffectDrawContext& draw) override;
    void Draw(const EffectFrameContext& frame, const EffectDrawContext& draw) override;
    [[nodiscard]] std::string_view Name() const noexcept override;
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
};

} // namespace lrender
