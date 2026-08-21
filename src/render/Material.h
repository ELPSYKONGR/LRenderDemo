/**
 * @file GPU-ready material shared by procedural and imported meshes.
 * @author Codex
 * @created 2026-08-21
 * @depends render/Texture2D.h, render/SamplerState.h, DirectXTK SimpleMath
 */
#pragma once

#include "render/SamplerState.h"
#include "render/Texture2D.h"

#include <SimpleMath.h>
#include <memory>
#include <string>

namespace lrender {

struct Material {
    std::string name{"Default"};
    DirectX::SimpleMath::Color baseColorFactor{1.0F, 1.0F, 1.0F, 1.0F};
    float specularStrength{0.25F};
    float shininess{32.0F};
    bool doubleSided{false};
    std::shared_ptr<Texture2D> baseColorTexture;
    std::shared_ptr<SamplerState> sampler;
};

} // namespace lrender
