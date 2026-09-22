/**
 * @file API-independent axis-aligned bounding box implementation.
 */
#include "core/BoundingBox.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace lrender
{

BoundingBox::BoundingBox(const DirectX::SimpleMath::Vector3& minimum,
                         const DirectX::SimpleMath::Vector3& maximum) noexcept
    : m_min(minimum), m_max(maximum), m_isValid(true)
{
}

void BoundingBox::Reset() noexcept
{
    m_min = {};
    m_max = {};
    m_isValid = false;
}

void BoundingBox::ExtendPoint(const DirectX::SimpleMath::Vector3& point) noexcept
{
    if (!m_isValid)
    {
        m_min = point;
        m_max = point;
        m_isValid = true;
        return;
    }
    m_min.x = std::min(m_min.x, point.x);
    m_min.y = std::min(m_min.y, point.y);
    m_min.z = std::min(m_min.z, point.z);
    m_max.x = std::max(m_max.x, point.x);
    m_max.y = std::max(m_max.y, point.y);
    m_max.z = std::max(m_max.z, point.z);
}

void BoundingBox::ExtendBox(const BoundingBox& box) noexcept
{
    if (!box.IsValid())
    {
        return;
    }
    ExtendPoint(box.Min());
    ExtendPoint(box.Max());
}

void BoundingBox::ExtendBox(const BoundingBox& box, const DirectX::SimpleMath::Matrix& transform) noexcept
{
    if (!box.IsValid())
    {
        return;
    }

    const DirectX::SimpleMath::Vector3 center = box.Center();
    const DirectX::SimpleMath::Vector3 extents = box.Size() * 0.5F;
    const std::array<DirectX::SimpleMath::Vector3, 8> corners{
        center + DirectX::SimpleMath::Vector3{-extents.x, -extents.y, -extents.z},
        center + DirectX::SimpleMath::Vector3{-extents.x, -extents.y, extents.z},
        center + DirectX::SimpleMath::Vector3{-extents.x, extents.y, -extents.z},
        center + DirectX::SimpleMath::Vector3{-extents.x, extents.y, extents.z},
        center + DirectX::SimpleMath::Vector3{extents.x, -extents.y, -extents.z},
        center + DirectX::SimpleMath::Vector3{extents.x, -extents.y, extents.z},
        center + DirectX::SimpleMath::Vector3{extents.x, extents.y, -extents.z},
        center + DirectX::SimpleMath::Vector3{extents.x, extents.y, extents.z}};
    for (const DirectX::SimpleMath::Vector3& corner : corners)
    {
        ExtendPoint(DirectX::SimpleMath::Vector3::Transform(corner, transform));
    }
}

bool BoundingBox::IsValid() const noexcept
{
    return m_isValid;
}

const DirectX::SimpleMath::Vector3& BoundingBox::Min() const noexcept
{
    return m_min;
}

const DirectX::SimpleMath::Vector3& BoundingBox::Max() const noexcept
{
    return m_max;
}

DirectX::SimpleMath::Vector3 BoundingBox::Center() const noexcept
{
    return m_isValid ? (m_min + m_max) * 0.5F : DirectX::SimpleMath::Vector3{};
}

DirectX::SimpleMath::Vector3 BoundingBox::Size() const noexcept
{
    return m_isValid ? m_max - m_min : DirectX::SimpleMath::Vector3{};
}

float BoundingBox::Radius() const noexcept
{
    return Size().Length() * 0.5F;
}

} // namespace lrender
