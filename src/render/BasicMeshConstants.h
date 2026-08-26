/**
 * @file CPU layout matching the BasicMesh HLSL constant buffer.
 * @author Codex
 * @created 2026-08-26
 * @depends DirectXTK SimpleMath
 */
#pragma once

#include <SimpleMath.h>

#include <array>
#include <cstddef>

namespace lrender {

struct alignas(16) BasicMeshPointLightConstants {
    DirectX::SimpleMath::Vector4 positionAndRange;
    DirectX::SimpleMath::Vector4 colorAndIntensity;
};

struct alignas(16) BasicMeshConstants {
    DirectX::SimpleMath::Matrix worldViewProjection;
    DirectX::SimpleMath::Matrix world;
    DirectX::SimpleMath::Matrix worldInverseTranspose;
    DirectX::SimpleMath::Vector4 baseColor;
    DirectX::SimpleMath::Vector4 cameraPosition;
    DirectX::SimpleMath::Vector4 ambientColor;
    DirectX::SimpleMath::Vector4 directionalDirectionAndIntensity;
    DirectX::SimpleMath::Vector4 directionalColorAndEnabled;
    std::array<BasicMeshPointLightConstants, 4> pointLights;
    DirectX::SimpleMath::Vector4 specularColor;
    DirectX::SimpleMath::Vector4 materialParameters;
};

static_assert(sizeof(BasicMeshConstants) == 432);
static_assert(sizeof(BasicMeshConstants) % 16 == 0);
static_assert(offsetof(BasicMeshConstants, baseColor) == 192);
static_assert(offsetof(BasicMeshConstants, pointLights) == 272);
static_assert(offsetof(BasicMeshConstants, specularColor) == 400);
static_assert(offsetof(BasicMeshConstants, materialParameters) == 416);

} // namespace lrender
