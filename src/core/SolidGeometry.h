/**
 * @file API-independent parameter descriptions for procedural solid geometry.
 * @author Codex
 * @created 2026-08-26
 * @depends DirectXTK SimpleMath
 */
#pragma once

#include <SimpleMath.h>

#include <cstdint>
#include <variant>

namespace lrender
{

enum class PrimitiveType
{
    Cube,
    Sphere,
    Plane,
    Mesh
};

struct CubeParameters
{
    DirectX::SimpleMath::Vector3 size = {1.0F, 1.0F, 1.0F};
};

struct SphereParameters
{
    float radius = 0.5F;
    std::uint16_t slices = 32;
    std::uint16_t stacks = 20;
};

struct PlaneParameters
{
    DirectX::SimpleMath::Vector2 size = {10.0F, 10.0F};
    std::uint16_t subdivisionsX = 1;
    std::uint16_t subdivisionsZ = 1;
};

using SolidParameters = std::variant<CubeParameters, SphereParameters, PlaneParameters>;

class SolidGeometry final
{
  public:
    SolidGeometry() = default;

    [[nodiscard]] static SolidGeometry Cube(CubeParameters parameters = {});
    [[nodiscard]] static SolidGeometry Sphere(SphereParameters parameters = {});
    [[nodiscard]] static SolidGeometry Plane(PlaneParameters parameters = {});

    [[nodiscard]] PrimitiveType Type() const noexcept;
    [[nodiscard]] const SolidParameters& Parameters() const noexcept;
    [[nodiscard]] bool NearlyEquals(const SolidGeometry& other, float epsilon = 0.0001F) const noexcept;

  private:
    explicit SolidGeometry(SolidParameters parameters);
    static void Validate(const SolidParameters& parameters);

    SolidParameters m_parameters = CubeParameters();
};

} // namespace lrender
