/**
 * @file Test effect skeleton.
 */
#pragma once

#include "render/IRenderEffect.h"

#include <filesystem>
#include <cstdint>
#include <string_view>

namespace lrender
{

enum class TestGeometryMode
{
    SingleTriangle,
    TransparentTriangles
};

class TestEffect final : public IRenderEffect
{
  public:
    TestEffect(ID3D11Device* device, ID3D11DeviceContext* context,
               const std::filesystem::path& shaderDirectory);
    void SetGeometryMode(TestGeometryMode mode) noexcept;
    [[nodiscard]] TestGeometryMode GeometryMode() const noexcept;
    void Bind(const EffectFrameContext& frame, const EffectDrawContext& draw) override;
    void Draw(const EffectFrameContext& frame, const EffectDrawContext& draw) override;
    [[nodiscard]] std::string_view Name() const noexcept override;
  private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_singleTriangleVertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_singleTriangleIndexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_transparentVertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_transparentIndexBuffer;
    std::uint32_t m_singleTriangleIndexCount = 0;
    std::uint32_t m_transparentIndexCount = 0;
    TestGeometryMode m_geometryMode = TestGeometryMode::SingleTriangle;
};

} // namespace lrender
