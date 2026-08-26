/**
 * @file Editable HLSL effect for lit colored meshes.
 * @author Codex
 * @created 2026-08-20
 * @depends render/IRenderEffect.h, compiled BasicMesh shaders, DirectXTK CommonStates
 */
#pragma once

#include "render/BasicMeshConstants.h"
#include "render/Dx11ConstantBuffer.h"
#include "render/IRenderEffect.h"
#include "render/Lighting.h"

#include <CommonStates.h>
#include <filesystem>
#include <memory>
#include <wrl/client.h>

namespace lrender {

class BasicMeshEffect final : public IRenderEffect {
public:
    BasicMeshEffect(ID3D11Device* device, const std::filesystem::path& shaderDirectory);

    void Bind(
        ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& world,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection,
        const DirectX::SimpleMath::Vector3& cameraPosition,
        const Material& material,
        const DirectX::SimpleMath::Color& tint,
        bool isSelected) override;

    [[nodiscard]] std::string_view Name() const noexcept override { return "Basic Lit"; }
    void SetWireframe(bool isWireframe) noexcept { isWireframe_ = isWireframe; }
    [[nodiscard]] bool IsWireframe() const noexcept { return isWireframe_; }
    [[nodiscard]] LightingSettings& Lights() noexcept { return lights_; }
    [[nodiscard]] const LightingSettings& Lights() const noexcept { return lights_; }

private:
    std::unique_ptr<DirectX::CommonStates> states_;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
    Dx11ConstantBuffer<BasicMeshConstants> constantBuffer_;
    LightingSettings lights_;
    bool isWireframe_{false};
};

} // namespace lrender
