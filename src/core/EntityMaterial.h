/**
 * @file Graphics-API-independent editable entity material settings.
 * @author Codex
 * @created 2026-08-25
 * @depends DirectXTK SimpleMath
 */
#pragma once

#include <SimpleMath.h>
#include <cmath>
#include <filesystem>

namespace lrender {

enum class SurfaceDisplayMode { LitTextured, TextureOnly, LitUntextured };
enum class MaterialFilter { Point, Linear, Anisotropic };
enum class MaterialAddressMode { Wrap, Clamp, Mirror };

struct EntityMaterial {
    DirectX::SimpleMath::Color baseColor = {1.0F, 1.0F, 1.0F, 1.0F};
    float diffuseStrength = 1.0F;
    DirectX::SimpleMath::Color specularColor = {1.0F, 1.0F, 1.0F, 1.0F};
    float specularStrength = 0.25F;
    float shininess = 32.0F;
    bool doubleSided = false;
    SurfaceDisplayMode displayMode = SurfaceDisplayMode::LitTextured;
    bool useSourceTexture = false;
    std::filesystem::path baseColorTexturePath;
    MaterialFilter filter = MaterialFilter::Linear;
    MaterialAddressMode addressMode = MaterialAddressMode::Wrap;

    [[nodiscard]] bool NearlyEquals(const EntityMaterial& other, float epsilon = 0.0001F) const {
        const auto colorNear = [epsilon](
                                   const DirectX::SimpleMath::Color& left,
                                   const DirectX::SimpleMath::Color& right) {
            return std::abs(left.x - right.x) <= epsilon &&
                   std::abs(left.y - right.y) <= epsilon &&
                   std::abs(left.z - right.z) <= epsilon &&
                   std::abs(left.w - right.w) <= epsilon;
        };
        return colorNear(baseColor, other.baseColor) &&
               std::abs(diffuseStrength - other.diffuseStrength) <= epsilon &&
               colorNear(specularColor, other.specularColor) &&
               std::abs(specularStrength - other.specularStrength) <= epsilon &&
               std::abs(shininess - other.shininess) <= epsilon &&
               doubleSided == other.doubleSided && displayMode == other.displayMode &&
               useSourceTexture == other.useSourceTexture &&
               baseColorTexturePath == other.baseColorTexturePath && filter == other.filter &&
               addressMode == other.addressMode;
    }
};

} // namespace lrender
