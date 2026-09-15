/**
 * @file Screen-space reflection effect skeleton.
 */
#pragma once

#include "render/IRenderEffect.h"

#include <filesystem>
#include <string_view>

namespace lrender
{

class SSREffect final : public IRenderEffect
{
  public:
    SSREffect(ID3D11Device* device, ID3D11DeviceContext* context,
              const std::filesystem::path& shaderDirectory);

    void Bind(const EffectFrameContext& frame);
    void Bind(const EffectFrameContext& frame, const EffectDrawContext& draw) override;
    void Draw(const EffectFrameContext& frame);
    void Draw(const EffectFrameContext& frame, const EffectDrawContext& draw) override;
    [[nodiscard]] std::string_view Name() const noexcept override;
};

} // namespace lrender
