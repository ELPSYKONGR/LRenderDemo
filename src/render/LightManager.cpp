/**
 * @file Global light manager implementation.
 */
#include "render/LightManager.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

namespace lrender
{
namespace
{

DirectX::SimpleMath::Vector4 ToVector4(const DirectX::SimpleMath::Color& color)
{
    return {color.x, color.y, color.z, color.w};
}

} // namespace

std::unique_ptr<LightManager> LightManager::m_instance;

void LightManager::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    if (m_instance != nullptr)
    {
        throw std::logic_error("LightManager is already initialized");
    }
    m_instance = std::unique_ptr<LightManager>(new LightManager(device, context));
}

void LightManager::Shutdown() noexcept
{
    m_instance.reset();
}

LightManager& LightManager::Instance()
{
    if (m_instance == nullptr)
    {
        throw std::logic_error("LightManager is not initialized");
    }
    return *m_instance;
}

LightManager::LightManager(ID3D11Device* device, ID3D11DeviceContext* context)
    : m_context(context), m_lightBuffer(device)
{
    if (m_context == nullptr)
    {
        throw std::invalid_argument("LightManager requires a D3D11 context");
    }
    ResetDefaults();
}

DirectX::SimpleMath::Color& LightManager::Ambient() noexcept
{
    return m_lights.ambient;
}

const DirectX::SimpleMath::Color& LightManager::Ambient() const noexcept
{
    return m_lights.ambient;
}

DirectionalLight* LightManager::Directional() noexcept
{
    return m_lights.directional ? &*m_lights.directional : nullptr;
}

const DirectionalLight* LightManager::Directional() const noexcept
{
    return m_lights.directional ? &*m_lights.directional : nullptr;
}

std::span<PointLight> LightManager::PointLights() noexcept
{
    return m_lights.points;
}

std::span<const PointLight> LightManager::PointLights() const noexcept
{
    return m_lights.points;
}

std::optional<LightId> LightManager::AddDirectionalLight(DirectionalLight light)
{
    if (m_lights.directional)
    {
        return std::nullopt;
    }
    light.id = NextId();
    const LightId id = light.id;
    m_lights.directional = std::move(light);
    return id;
}

std::optional<LightId> LightManager::AddPointLight(PointLight light)
{
    if (m_lights.points.size() >= MaxPointLights)
    {
        return std::nullopt;
    }
    light.id = NextId();
    const LightId id = light.id;
    m_lights.points.push_back(std::move(light));
    return id;
}

bool LightManager::RemoveLight(LightId id) noexcept
{
    if (id == 0)
    {
        return false;
    }
    if (m_lights.directional && m_lights.directional->id == id)
    {
        m_lights.directional.reset();
        return true;
    }
    const auto iterator = std::find_if(m_lights.points.begin(), m_lights.points.end(), [id](const PointLight& light)
    {
        return light.id == id;
    });
    if (iterator == m_lights.points.end())
    {
        return false;
    }
    m_lights.points.erase(iterator);
    return true;
}

DirectionalLight* LightManager::FindDirectionalLight(LightId id) noexcept
{
    DirectionalLight* light = Directional();
    return light != nullptr && light->id == id ? light : nullptr;
}

PointLight* LightManager::FindPointLight(LightId id) noexcept
{
    const auto iterator = std::find_if(m_lights.points.begin(), m_lights.points.end(), [id](const PointLight& light)
    {
        return light.id == id;
    });
    return iterator == m_lights.points.end() ? nullptr : &*iterator;
}

void LightManager::ClearLights() noexcept
{
    m_lights.directional.reset();
    m_lights.points.clear();
    m_nextLightId = 1;
}

void LightManager::ResetDefaults()
{
    m_lights = {};
    m_nextLightId = 1;
    static_cast<void>(AddDirectionalLight());
    static_cast<void>(AddPointLight(PointLight{0, true, {-2.5F, 2.0F, -1.0F},
                                                {1.0F, 0.32F, 0.20F, 1.0F}, 1.8F, 5.5F}));
    static_cast<void>(AddPointLight(PointLight{0, true, {2.5F, 1.5F, 0.5F},
                                                {0.20F, 0.48F, 1.0F, 1.0F}, 1.7F, 5.5F}));
    static_cast<void>(AddPointLight(PointLight{0, true, {0.0F, 3.5F, 2.0F},
                                                {0.42F, 1.0F, 0.55F, 1.0F}, 1.4F, 6.0F}));
}

void LightManager::UpdateBuffer() const
{
    m_lightBuffer.Update(m_context, BuildConstants());
}

void LightManager::BindBuffer() const
{
    m_lightBuffer.BindPS(m_context, 3);
}

LightId LightManager::NextId()
{
    if (m_nextLightId == 0 || m_nextLightId == std::numeric_limits<LightId>::max())
    {
        throw std::overflow_error("Light id space is exhausted");
    }
    return m_nextLightId++;
}

LightConstants LightManager::BuildConstants() const
{
    LightConstants data{};
    data.ambientColor = ToVector4(m_lights.ambient);
    if (const DirectionalLight* light = Directional())
    {
        auto direction = light->direction;
        if (direction.LengthSquared() < 0.000001F)
        {
            direction = {0.0F, -1.0F, 0.0F};
        }
        else
        {
            direction.Normalize();
        }
        data.directionalDirectionAndIntensity = {direction.x, direction.y, direction.z, light->intensity};
        data.directionalColorAndEnabled = {light->color.x, light->color.y, light->color.z,
                                           light->enabled ? 1.0F : 0.0F};
    }

    std::size_t outputIndex = 0;
    for (const PointLight& light : m_lights.points)
    {
        if (!light.enabled)
        {
            continue;
        }
        data.pointLightData[outputIndex * 2] = {light.position.x, light.position.y, light.position.z,
                                                std::max(light.range, 0.0001F)};
        data.pointLightData[outputIndex * 2 + 1] = {light.color.x, light.color.y, light.color.z, light.intensity};
        ++outputIndex;
    }
    data.pointLightCount = static_cast<std::uint32_t>(outputIndex);
    return data;
}

} // namespace lrender
