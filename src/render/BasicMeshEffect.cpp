/**
 * @file DirectXTK BasicEffect adapter implementation.
 * @author Codex
 * @created 2026-08-20
 * @depends render/BasicMeshEffect.h
 */
#include "render/BasicMeshEffect.h"

#include <VertexTypes.h>
#include <stdexcept>

namespace lrender {

BasicMeshEffect::BasicMeshEffect(ID3D11Device* device) {
    if (device == nullptr) {
        throw std::invalid_argument("BasicMeshEffect requires a D3D11 device");
    }

    effect_ = std::make_unique<DirectX::BasicEffect>(device);
    states_ = std::make_unique<DirectX::CommonStates>(device);
    effect_->SetLightingEnabled(true);
    effect_->EnableDefaultLighting();
    effect_->SetVertexColorEnabled(true);

    const void* shaderByteCode = nullptr;
    std::size_t shaderByteCodeLength = 0;
    effect_->GetVertexShaderBytecode(&shaderByteCode, &shaderByteCodeLength);
    const HRESULT result = device->CreateInputLayout(
        DirectX::VertexPositionNormalColor::InputElements,
        DirectX::VertexPositionNormalColor::InputElementCount,
        shaderByteCode,
        shaderByteCodeLength,
        inputLayout_.ReleaseAndGetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to create BasicEffect input layout");
    }
}

void BasicMeshEffect::Bind(
    ID3D11DeviceContext* context,
    const DirectX::SimpleMath::Matrix& world,
    const DirectX::SimpleMath::Matrix& view,
    const DirectX::SimpleMath::Matrix& projection,
    const DirectX::SimpleMath::Color& color,
    bool isSelected) {
    if (context == nullptr) {
        throw std::invalid_argument("BasicMeshEffect requires a D3D11 context");
    }

    constexpr DirectX::SimpleMath::Color selectionColor{1.0F, 0.84F, 0.0F, 1.0F};
    const DirectX::SimpleMath::Color finalColor = isSelected
        ? DirectX::SimpleMath::Color::Lerp(color, selectionColor, 0.28F)
        : color;
    effect_->SetWorld(world);
    effect_->SetView(view);
    effect_->SetProjection(projection);
    effect_->SetDiffuseColor(finalColor.ToVector3());
    effect_->SetAlpha(finalColor.w);

    context->IASetInputLayout(inputLayout_.Get());
    context->RSSetState(isWireframe_ ? states_->Wireframe() : states_->CullNone());
    effect_->Apply(context);
}

} // namespace lrender
