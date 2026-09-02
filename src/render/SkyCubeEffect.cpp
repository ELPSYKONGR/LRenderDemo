/**
 * @file Skybox effect boundary implementation.
 */
#include "render/SkyCubeEffect.h"

#include <stdexcept>
#include <utility>

namespace lrender {

SkyCubeEffect::SkyCubeEffect(
    ID3D11Device* device, ID3D11DeviceContext* context,
    std::filesystem::path shaderDirectory)
    : IRenderEffect(device, context), m_shaderDirectory(std::move(shaderDirectory)) {}

void SkyCubeEffect::Bind(
    const EffectFrameContext&, const EffectDrawContext&) {
    throw std::logic_error(
        "SkyCubeEffect requires a cubemap resource and skybox shaders before use");
}

} // namespace lrender
