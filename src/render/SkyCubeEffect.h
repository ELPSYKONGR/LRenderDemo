/**
 * @file Skybox effect boundary reserved for the cubemap pass.
 */
#pragma once

#include "render/IRenderEffect.h"

#include <filesystem>
#include <string_view>

namespace lrender
{

class SkyCubeEffect final : public IRenderEffect
{
  public:
    SkyCubeEffect(ID3D11Device* device, ID3D11DeviceContext* context, std::filesystem::path shaderDirectory);

    void Bind(const EffectFrameContext& frame, const EffectDrawContext& draw) override;
    [[nodiscard]] std::string_view Name() const noexcept override;

  private:
    std::filesystem::path m_shaderDirectory;
};

} // namespace lrender
