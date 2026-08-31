/**
 * @file Skybox effect boundary implementation.
 */
#include "render/SkyCubeEffect.h"

#include <stdexcept>
#include <utility>

namespace lrender {

SkyCubeEffect::SkyCubeEffect(
    ID3D11Device* device, std::filesystem::path shaderDirectory)
    : IRenderEffect(device), m_shaderDirectory(std::move(shaderDirectory)) {}

void SkyCubeEffect::Bind(
    const EffectFrameContext&, const EffectDrawContext&) {
    throw std::logic_error(
        "SkyCubeEffect requires a cubemap resource and skybox shaders before use");
}

} // namespace lrender
