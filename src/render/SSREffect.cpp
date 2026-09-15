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

void SSREffect::Bind(const EffectFrameContext&)
{
}

void SSREffect::Bind(const EffectFrameContext& frame, const EffectDrawContext&)
{
    Bind(frame);
}

void SSREffect::Draw(const EffectFrameContext&)
{
}

void SSREffect::Draw(const EffectFrameContext& frame, const EffectDrawContext&)
{
    Draw(frame);
}

std::string_view SSREffect::Name() const noexcept
{
    return "SSR";
}

} // namespace lrender
