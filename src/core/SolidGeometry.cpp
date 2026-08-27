/**
 * @file Procedural solid parameter validation and type queries.
 * @author Codex
 * @created 2026-08-26
 * @depends core/SolidGeometry.h
 */
#include "core/SolidGeometry.h"

#include <cmath>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace lrender {
namespace {

bool IsPositiveFinite(float value) noexcept {
    return std::isfinite(value) && value >= 0.001F;
}

bool Near(float left, float right, float epsilon) noexcept {
    return std::abs(left - right) <= epsilon;
}

} // namespace

SolidGeometry::SolidGeometry(SolidParameters parameters) : parameters_(std::move(parameters)) {
    Validate(parameters_);
}

SolidGeometry SolidGeometry::Cube(CubeParameters parameters) {
    return SolidGeometry{parameters};
}

SolidGeometry SolidGeometry::Sphere(SphereParameters parameters) {
    return SolidGeometry{parameters};
}

SolidGeometry SolidGeometry::Plane(PlaneParameters parameters) {
    return SolidGeometry{parameters};
}

PrimitiveType SolidGeometry::Type() const noexcept {
    return std::visit(
        [](const auto& parameters) {
            using Parameters = std::decay_t<decltype(parameters)>;
            if constexpr (std::is_same_v<Parameters, CubeParameters>) {
                return PrimitiveType::Cube;
            } else if constexpr (std::is_same_v<Parameters, SphereParameters>) {
                return PrimitiveType::Sphere;
            } else {
                return PrimitiveType::Plane;
            }
        },
        parameters_);
}

bool SolidGeometry::NearlyEquals(const SolidGeometry& other, float epsilon) const noexcept {
    if (parameters_.index() != other.parameters_.index()) {
        return false;
    }
    return std::visit(
        [epsilon](const auto& left, const auto& right) {
            using Left = std::decay_t<decltype(left)>;
            using Right = std::decay_t<decltype(right)>;
            if constexpr (!std::is_same_v<Left, Right>) {
                return false;
            } else if constexpr (std::is_same_v<Left, CubeParameters>) {
                return Near(left.size.x, right.size.x, epsilon) &&
                       Near(left.size.y, right.size.y, epsilon) &&
                       Near(left.size.z, right.size.z, epsilon);
            } else if constexpr (std::is_same_v<Left, SphereParameters>) {
                return Near(left.radius, right.radius, epsilon) &&
                       left.slices == right.slices && left.stacks == right.stacks;
            } else {
                return Near(left.size.x, right.size.x, epsilon) &&
                       Near(left.size.y, right.size.y, epsilon) &&
                       left.subdivisionsX == right.subdivisionsX &&
                       left.subdivisionsZ == right.subdivisionsZ;
            }
        },
        parameters_, other.parameters_);
}

void SolidGeometry::Validate(const SolidParameters& parameters) {
    std::visit(
        [](const auto& value) {
            using Parameters = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<Parameters, CubeParameters>) {
                if (!IsPositiveFinite(value.size.x) || !IsPositiveFinite(value.size.y) ||
                    !IsPositiveFinite(value.size.z)) {
                    throw std::invalid_argument("Cube dimensions must be finite and positive");
                }
            } else if constexpr (std::is_same_v<Parameters, SphereParameters>) {
                if (!IsPositiveFinite(value.radius) || value.slices < 3 || value.stacks < 2) {
                    throw std::invalid_argument(
                        "Sphere requires a positive radius, 3 slices, and 2 stacks");
                }
            } else {
                if (!IsPositiveFinite(value.size.x) || !IsPositiveFinite(value.size.y) ||
                    value.subdivisionsX == 0 || value.subdivisionsZ == 0) {
                    throw std::invalid_argument(
                        "Plane requires positive dimensions and subdivisions");
                }
            }
        },
        parameters);
}

} // namespace lrender
