/**
 * @file Editable entity material comparison implementation.
 */
#include "core/EntityMaterial.h"

#include <cmath>

namespace lrender
{

bool EntityMaterial::NearlyEquals(const EntityMaterial& other, float epsilon) const
{
    const auto colorNear =
        [epsilon](const DirectX::SimpleMath::Color& left, const DirectX::SimpleMath::Color& right)
    {
        return std::abs(left.x - right.x) <= epsilon && std::abs(left.y - right.y) <= epsilon &&
               std::abs(left.z - right.z) <= epsilon && std::abs(left.w - right.w) <= epsilon;
    };
    return colorNear(baseColor, other.baseColor) && std::abs(diffuseStrength - other.diffuseStrength) <= epsilon &&
           colorNear(specularColor, other.specularColor) &&
           std::abs(specularStrength - other.specularStrength) <= epsilon &&
           std::abs(shininess - other.shininess) <= epsilon && doubleSided == other.doubleSided &&
           displayMode == other.displayMode && useSourceTexture == other.useSourceTexture &&
           baseColorTexturePath == other.baseColorTexturePath && filter == other.filter &&
           addressMode == other.addressMode;
}

} // namespace lrender
