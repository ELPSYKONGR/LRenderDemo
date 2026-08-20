/**
 * @file DirectXTK BasicEffect adapter for lit colored meshes.
 * @author Codex
 * @created 2026-08-20
 * @depends render/IRenderEffect.h, DirectXTK Effects/CommonStates
 */
#pragma once

#include "render/IRenderEffect.h"

#include <CommonStates.h>
#include <Effects.h>
#include <wrl/client.h>

namespace lrender {

class BasicMeshEffect final : public IRenderEffect {
public:
    explicit BasicMeshEffect(ID3D11Device* device);

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
    std::unique_ptr<DirectX::BasicEffect> effect_;
    std::unique_ptr<DirectX::CommonStates> states_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
    bool isWireframe_{false};
};

} // namespace lrender
