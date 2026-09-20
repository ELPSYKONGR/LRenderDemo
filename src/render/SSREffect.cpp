/**
 * @file Screen-space reflection effect skeleton implementation.
 */

#include "render/SSREffect.h"

namespace lrender
{

SSREffect::SSREffect(ID3D11Device* device, ID3D11DeviceContext* context,
                     const std::filesystem::path&)
    : IRenderEffect(device, context)
{
}

void SSREffect::BindPipeline(const EffectFrameContext&)
{
}

void SSREffect::RenderEffect(const EffectFrameContext& frame)
{
    BindPipeline(frame);
}

std::string_view SSREffect::Name() const noexcept
{
    return "SSR";
}

} // namespace lrender
