/**
 * @file CPU layouts shared by the DX11 effects and common.hlsli.
 */
#pragma once

#include <SimpleMath.h>

#include <array>
#include <cstddef>

namespace lrender {

struct alignas(16) FrameConstants {
    DirectX::SimpleMath::Matrix view;
    DirectX::SimpleMath::Matrix projection;
    DirectX::SimpleMath::Vector4 mode;
    DirectX::SimpleMath::Vector4 cameraPosition;
    DirectX::SimpleMath::Vector4 viewport;
};

struct alignas(16) ObjectConstants {
    DirectX::SimpleMath::Matrix worldViewProjection;
    DirectX::SimpleMath::Matrix world;
    DirectX::SimpleMath::Matrix worldInverseTranspose;
};

struct alignas(16) MaterialConstants {
    DirectX::SimpleMath::Vector4 baseColor;
    DirectX::SimpleMath::Vector4 specularColor;
    DirectX::SimpleMath::Vector4 materialParameters;
};

struct alignas(16) LightConstants {
    DirectX::SimpleMath::Vector4 ambientColor;
    DirectX::SimpleMath::Vector4 directionalDirectionAndIntensity;
    DirectX::SimpleMath::Vector4 directionalColorAndEnabled;
    std::array<DirectX::SimpleMath::Vector4, 8> pointLightData;
};

static_assert(sizeof(FrameConstants) == 176);
static_assert(sizeof(ObjectConstants) == 192);
static_assert(sizeof(MaterialConstants) == 48);
static_assert(sizeof(LightConstants) == 176);
static_assert(offsetof(FrameConstants, cameraPosition) == 144);
static_assert(offsetof(ObjectConstants, world) == 64);
static_assert(offsetof(LightConstants, pointLightData) == 48);

} // namespace lrender
