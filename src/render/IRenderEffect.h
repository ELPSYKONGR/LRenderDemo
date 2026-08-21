/**
 * @file Render-effect boundary used by scene drawing code.
 * @author Codex
 * @created 2026-08-20
 * @depends D3D11, DirectXTK SimpleMath, render/Material.h
 */
#pragma once

#include "render/Material.h"

#include <SimpleMath.h>
#include <d3d11.h>
#include <string_view>

namespace lrender {

class IRenderEffect {
public:
    virtual ~IRenderEffect() = default;

    /** Configures shaders, constants, input layout, and rasterizer state for one mesh. */
    virtual void Bind(
        ID3D11DeviceContext* context,
        const DirectX::SimpleMath::Matrix& world,
        const DirectX::SimpleMath::Matrix& view,
        const DirectX::SimpleMath::Matrix& projection,
        const DirectX::SimpleMath::Vector3& cameraPosition,
        const Material& material,
        const DirectX::SimpleMath::Color& tint,
        bool isSelected) = 0;

    [[nodiscard]] virtual std::string_view Name() const noexcept = 0;
};

} // namespace lrender
