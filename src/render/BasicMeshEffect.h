/**
 * @file Editable HLSL effect for lit colored meshes.
 * @author Codex
 * @created 2026-08-20
 * @depends render/IRenderEffect.h, compiled BasicMesh shaders, DirectXTK CommonStates
 */
#pragma once

#include "render/IRenderEffect.h"
#include "render/Lighting.h"

#include <CommonStates.h>
#include <array>
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
    struct alignas(16) PointLightConstants {
        DirectX::SimpleMath::Vector4 positionAndRange;
        DirectX::SimpleMath::Vector4 colorAndIntensity;
    };

    struct alignas(16) Constants {
        DirectX::SimpleMath::Matrix worldViewProjection;
        DirectX::SimpleMath::Matrix world;
        DirectX::SimpleMath::Matrix worldInverseTranspose;
        DirectX::SimpleMath::Vector4 baseColor;
        DirectX::SimpleMath::Vector4 cameraPosition;
        DirectX::SimpleMath::Vector4 ambientColor;
        DirectX::SimpleMath::Vector4 directionalDirectionAndIntensity;
        DirectX::SimpleMath::Vector4 directionalColorAndEnabled;
        std::array<PointLightConstants, 4> pointLights;
        DirectX::SimpleMath::Vector4 materialParameters;
    };

    std::unique_ptr<DirectX::CommonStates> states_;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer_;
    LightingSettings lights_;
    bool isWireframe_{false};
};

} // namespace lrender
