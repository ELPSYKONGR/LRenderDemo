/**
 * @file Editable transform matrix and comparison implementation.
 */
#include "core/Transform.h"

#include <cmath>

namespace lrender
{

DirectX::SimpleMath::Matrix Transform::ToMatrix() const
{
    using DirectX::SimpleMath::Matrix;
    return Matrix::CreateScale(scale) *
           Matrix::CreateFromYawPitchRoll(DirectX::XMConvertToRadians(rotationDegrees.y),
                                          DirectX::XMConvertToRadians(rotationDegrees.x),
                                          DirectX::XMConvertToRadians(rotationDegrees.z)) *
           Matrix::CreateTranslation(position);
}

bool Transform::NearlyEquals(const Transform& other, float epsilon) const
{
    const auto close = [epsilon](float left, float right)
    {
        return std::abs(left - right) <= epsilon;
    };
    return close(position.x, other.position.x) && close(position.y, other.position.y) &&
           close(position.z, other.position.z) && close(rotationDegrees.x, other.rotationDegrees.x) &&
           close(rotationDegrees.y, other.rotationDegrees.y) && close(rotationDegrees.z, other.rotationDegrees.z) &&
           close(scale.x, other.scale.x) && close(scale.y, other.scale.y) && close(scale.z, other.scale.z);
}

} // namespace lrender
