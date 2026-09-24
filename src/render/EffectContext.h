/**
 * @file Validated frame and draw snapshots consumed by mesh effects.
 * @author Codex
 * @created 2026-08-26
 * @depends core/Camera.h, core/Scene.h, render/MaterialManager.h, D3D11
 */
#pragma once

#include "render/MaterialManager.h"

#include <SimpleMath.h>
#include <array>
#include <cstdint>
#include <d3d11.h>
#include <wrl/client.h>

namespace lrender
{

class Camera;
class CommonConstantBuffers;
class Mesh;
struct Entity;
enum class RenderMode
{
    DirectRendering,
    DelayedRendering,
};

class EffectFrameContext final
{
  public:
    EffectFrameContext(ID3D11DeviceContext* deviceContext, CommonConstantBuffers& constantBuffers,
                       const Camera& camera, float aspectRatio);

    [[nodiscard]] ID3D11DeviceContext* DeviceContext() const noexcept;
    [[nodiscard]] CommonConstantBuffers& ConstantBuffers() const noexcept;
    [[nodiscard]] const DirectX::SimpleMath::Matrix& View() const noexcept;
    [[nodiscard]] const DirectX::SimpleMath::Matrix& Projection() const noexcept;
    [[nodiscard]] const DirectX::SimpleMath::Matrix& InverseViewProjection() const noexcept;
    [[nodiscard]] const DirectX::SimpleMath::Vector3& CameraPosition() const noexcept;
    [[nodiscard]] float AspectRatio() const noexcept;
    [[nodiscard]] RenderMode GetRenderMode() const noexcept;
    void SetRenderMode(RenderMode mode) noexcept;
    void BeginFrame() const;
    void CapturePipelineState() const;
    void ResetPipelineState() const;

  private:
    struct PipelineStateSnapshot
    {
        Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
        Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;
        D3D11_PRIMITIVE_TOPOLOGY primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
        std::array<float, 4> blendFactor = {};
        UINT sampleMask = 0xffffffffU;
        UINT stencilReference = 0;
        bool captured = false;
    };

    ID3D11DeviceContext* m_deviceContext = nullptr;
    CommonConstantBuffers* m_constantBuffers = nullptr;
    DirectX::SimpleMath::Matrix m_view;
    DirectX::SimpleMath::Matrix m_projection;
    DirectX::SimpleMath::Matrix m_inverseViewProjection;
    DirectX::SimpleMath::Vector3 m_cameraPosition;
    RenderMode m_renderMode = RenderMode::DirectRendering;
    float m_aspectRatio = 0.0F;
    mutable PipelineStateSnapshot m_pipelineState;
};

class EffectDrawContext final
{
  public:
    EffectDrawContext(const Entity& entity, MaterialDrawData material, std::uint32_t selectedEntityId,
                      const Mesh* mesh = nullptr);

    [[nodiscard]] const DirectX::SimpleMath::Matrix& World() const noexcept;
    [[nodiscard]] const MaterialDrawData& PreparedMaterial() const noexcept;
    [[nodiscard]] const DirectX::SimpleMath::Color& Tint() const noexcept;
    [[nodiscard]] bool IsSelected() const noexcept;
    [[nodiscard]] const Mesh* MeshGeometry() const noexcept;

  private:
    DirectX::SimpleMath::Matrix m_world;
    MaterialDrawData m_material;
    DirectX::SimpleMath::Color m_tint;
    bool m_isSelected = false;
    const Mesh* m_mesh = nullptr;
};

} // namespace lrender
