/**
 * @file API-independent axis-aligned bounding box.
 */
#pragma once

#include <SimpleMath.h>

namespace lrender
{

class BoundingBox final
{
  public:
    BoundingBox() = default;
    BoundingBox(const DirectX::SimpleMath::Vector3& minimum,
                const DirectX::SimpleMath::Vector3& maximum) noexcept;

    void Reset() noexcept;
    void ExtendPoint(const DirectX::SimpleMath::Vector3& point) noexcept;
    void ExtendBox(const BoundingBox& box) noexcept;
    void ExtendBox(const BoundingBox& box, const DirectX::SimpleMath::Matrix& transform) noexcept;

    [[nodiscard]] bool IsValid() const noexcept;
    [[nodiscard]] const DirectX::SimpleMath::Vector3& Min() const noexcept;
    [[nodiscard]] const DirectX::SimpleMath::Vector3& Max() const noexcept;
    [[nodiscard]] DirectX::SimpleMath::Vector3 Center() const noexcept;
    [[nodiscard]] DirectX::SimpleMath::Vector3 Size() const noexcept;
    [[nodiscard]] float Radius() const noexcept;

  private:
    DirectX::SimpleMath::Vector3 m_min = {};
    DirectX::SimpleMath::Vector3 m_max = {};
    bool m_isValid = false;
};

} // namespace lrender
