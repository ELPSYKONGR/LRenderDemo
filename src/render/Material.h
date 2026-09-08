/**
 * @file GPU-ready material shared by procedural and imported meshes.
 * @author Codex
 * @created 2026-08-21
 * @depends render/Texture2D.h, render/SamplerState.h, DirectXTK SimpleMath
 */
#pragma once

#include "core/EntityMaterial.h"
#include "render/SamplerState.h"
#include "render/Texture2D.h"

#include <SimpleMath.h>
#include <memory>
#include <string>

namespace lrender
{

class Material final
{
  public:
    Material();
    explicit Material(std::string name);

    [[nodiscard]] const std::string& GetName() const noexcept;
    void SetName(std::string name);

    [[nodiscard]] const DirectX::SimpleMath::Color& GetBaseColorFactor() const noexcept;
    void SetBaseColorFactor(const DirectX::SimpleMath::Color& value) noexcept;

    [[nodiscard]] float GetDiffuseStrength() const noexcept;
    void SetDiffuseStrength(float value) noexcept;

    [[nodiscard]] const DirectX::SimpleMath::Color& GetSpecularColor() const noexcept;
    void SetSpecularColor(const DirectX::SimpleMath::Color& value) noexcept;

    [[nodiscard]] float GetSpecularStrength() const noexcept;
    void SetSpecularStrength(float value) noexcept;

    [[nodiscard]] float GetShininess() const noexcept;
    void SetShininess(float value) noexcept;

    [[nodiscard]] bool IsDoubleSided() const noexcept;
    void SetDoubleSided(bool value) noexcept;

    [[nodiscard]] SurfaceDisplayMode GetDisplayMode() const noexcept;
    void SetDisplayMode(SurfaceDisplayMode value) noexcept;

    [[nodiscard]] const std::shared_ptr<Texture2D>& GetBaseColorTexture() const noexcept;
    void SetBaseColorTexture(std::shared_ptr<Texture2D> value) noexcept;

    [[nodiscard]] bool UsesBaseColorTexture() const noexcept;

    [[nodiscard]] const std::shared_ptr<SamplerState>& GetSampler() const noexcept;
    void SetSampler(std::shared_ptr<SamplerState> value) noexcept;

  private:
    std::string m_name = "Default";
    DirectX::SimpleMath::Color m_baseColorFactor = {1.0F, 1.0F, 1.0F, 1.0F};
    float m_diffuseStrength = 1.0F;
    DirectX::SimpleMath::Color m_specularColor = {1.0F, 1.0F, 1.0F, 1.0F};
    float m_specularStrength = 0.25F;
    float m_shininess = 32.0F;
    bool m_doubleSided = false;
    SurfaceDisplayMode m_displayMode = SurfaceDisplayMode::LitTextured;
    std::shared_ptr<Texture2D> m_baseColorTexture;
    std::shared_ptr<SamplerState> m_sampler;
};

} // namespace lrender
