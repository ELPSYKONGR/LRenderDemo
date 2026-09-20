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

    void BindPipeline(const EffectFrameContext& frame) override;
    void RenderEffect(const EffectFrameContext& frame) override;
    [[nodiscard]] std::string_view Name() const noexcept override;
};

} // namespace lrender
