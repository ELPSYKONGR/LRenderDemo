/**
 * @file Editable HLSL effect for lit colored meshes.
 * @author Codex
 * @created 2026-08-20
 * @depends render/IRenderEffect.h, compiled BasicMesh shaders, DirectXTK CommonStates
 */
#pragma once

#include "render/IRenderEffect.h"

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
        const DirectX::SimpleMath::Color& color,
        bool isSelected) override;

    [[nodiscard]] std::string_view Name() const noexcept override { return "Basic Lit"; }
    void SetWireframe(bool isWireframe) noexcept { isWireframe_ = isWireframe; }
    [[nodiscard]] bool IsWireframe() const noexcept { return isWireframe_; }

private:
    struct alignas(16) Constants {
        DirectX::SimpleMath::Matrix worldViewProjection;
        DirectX::SimpleMath::Matrix worldInverseTranspose;
        DirectX::SimpleMath::Vector4 diffuseColor;
        DirectX::SimpleMath::Vector4 lightDirection;
        DirectX::SimpleMath::Vector4 lightColor;
        DirectX::SimpleMath::Vector4 ambientColor;
    };

    std::unique_ptr<DirectX::CommonStates> states_;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer_;
    bool isWireframe_{false};
};

} // namespace lrender
