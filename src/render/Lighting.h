/**
 * @file Editable scene-light settings for the basic mesh effect.
 * @author Codex
 * @created 2026-08-21
 * @depends DirectXTK SimpleMath
 */
#pragma once

#include <SimpleMath.h>
#include <array>

namespace lrender {

struct DirectionalLight {
    bool enabled{true};
    DirectX::SimpleMath::Vector3 direction{-0.45F, -0.8F, 0.35F};
    DirectX::SimpleMath::Color color{1.0F, 0.96F, 0.88F, 1.0F};
    float intensity{0.85F};
};

struct PointLight {
    bool enabled{true};
    DirectX::SimpleMath::Vector3 position{};
    DirectX::SimpleMath::Color color{1.0F, 1.0F, 1.0F, 1.0F};
    float intensity{1.5F};
    float range{5.0F};
};

struct LightingSettings {
    DirectX::SimpleMath::Color ambient{0.12F, 0.14F, 0.18F, 1.0F};
    DirectionalLight directional;
    std::array<PointLight, 4> points{
        PointLight{true, {-2.5F, 2.0F, -1.0F}, {1.0F, 0.32F, 0.20F, 1.0F}, 1.8F, 5.5F},
        PointLight{true, {2.5F, 1.5F, 0.5F}, {0.20F, 0.48F, 1.0F, 1.0F}, 1.7F, 5.5F},
        PointLight{true, {0.0F, 3.5F, 2.0F}, {0.42F, 1.0F, 0.55F, 1.0F}, 1.4F, 6.0F},
        PointLight{false, {0.0F, 1.0F, -3.0F}, {1.0F, 1.0F, 1.0F, 1.0F}, 1.0F, 5.0F}};
};

} // namespace lrender
