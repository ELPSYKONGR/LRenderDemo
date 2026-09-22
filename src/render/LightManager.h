/**
 * @file Global light settings and D3D11 light constant-buffer ownership.
 */
#pragma once

#include "render/Dx11ConstantBuffer.h"
#include "render/Lighting.h"

#include <SimpleMath.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>

namespace lrender
{

struct alignas(16) LightConstants
{
    DirectX::SimpleMath::Vector4 ambientColor;
    DirectX::SimpleMath::Vector4 directionalDirectionAndIntensity;
    DirectX::SimpleMath::Vector4 directionalColorAndEnabled;
    std::array<DirectX::SimpleMath::Vector4, 8> pointLightData;
    std::uint32_t pointLightCount = 0;
    std::array<std::uint32_t, 3> lightPadding = {};
};

static_assert(sizeof(LightConstants) == 192);
static_assert(offsetof(LightConstants, pointLightData) == 48);
static_assert(offsetof(LightConstants, pointLightCount) == 176);

class LightManager final
{
  public:
    static constexpr std::size_t MaxDirectionalLights = 1;
    static constexpr std::size_t MaxPointLights = 4;

    static void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    static void Shutdown() noexcept;
    [[nodiscard]] static LightManager& Instance();

    LightManager(const LightManager&) = delete;
    LightManager& operator=(const LightManager&) = delete;

    [[nodiscard]] DirectX::SimpleMath::Color& Ambient() noexcept;
    [[nodiscard]] const DirectX::SimpleMath::Color& Ambient() const noexcept;
    [[nodiscard]] DirectionalLight* Directional() noexcept;
    [[nodiscard]] const DirectionalLight* Directional() const noexcept;
    [[nodiscard]] std::span<PointLight> PointLights() noexcept;
    [[nodiscard]] std::span<const PointLight> PointLights() const noexcept;

    [[nodiscard]] std::optional<LightId> AddDirectionalLight(DirectionalLight light = {});
    [[nodiscard]] std::optional<LightId> AddPointLight(PointLight light = {});
    [[nodiscard]] bool RemoveLight(LightId id) noexcept;
    [[nodiscard]] DirectionalLight* FindDirectionalLight(LightId id) noexcept;
    [[nodiscard]] PointLight* FindPointLight(LightId id) noexcept;

    void ClearLights() noexcept;
    void ResetDefaults();
    void UpdateBuffer() const;
    void BindBuffer() const;

  private:
    LightManager(ID3D11Device* device, ID3D11DeviceContext* context);
    [[nodiscard]] LightId NextId();
    [[nodiscard]] LightConstants BuildConstants() const;

    static std::unique_ptr<LightManager> m_instance;
    ID3D11DeviceContext* m_context = nullptr;
    LightingSettings m_lights;
    Dx11ConstantBuffer<LightConstants> m_lightBuffer;
    LightId m_nextLightId = 1;
};

} // namespace lrender
