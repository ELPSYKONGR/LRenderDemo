/**
 * @file Graphics-API-independent editable material definition.
 */
#pragma once

#include <SimpleMath.h>

#include <filesystem>
#include <string>

namespace lrender
{

enum class SurfaceDisplayMode
{
    LitTextured,
    TextureOnly,
    LitUntextured
};

enum class MaterialFilter
{
    Point,
    Linear,
    Anisotropic
};

enum class MaterialAddressMode
{
    Wrap,
    Clamp,
    Mirror
};

enum class MaterialTextureSource
{
    Source,
    Custom
};

class Material final
{
  public:
    Material() = default;
    explicit Material(std::string name);

    [[nodiscard]] const std::string& GetName() const noexcept;
    void SetName(std::string name);
    [[nodiscard]] const DirectX::SimpleMath::Color& GetBaseColor() const noexcept;
    void SetBaseColor(const DirectX::SimpleMath::Color& value) noexcept;
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
    [[nodiscard]] MaterialTextureSource GetTextureSource() const noexcept;
    void SetTextureSource(MaterialTextureSource value) noexcept;
    [[nodiscard]] const std::filesystem::path& GetBaseColorTexturePath() const noexcept;
    void SetBaseColorTexturePath(std::filesystem::path path);
    [[nodiscard]] const std::string& GetEmbeddedBaseColorTextureKey() const noexcept;
    void SetEmbeddedBaseColorTextureKey(std::string key);
    void ClearBaseColorTexture() noexcept;
    [[nodiscard]] bool HasBaseColorTexture() const noexcept;
    [[nodiscard]] bool UsesBaseColorTexture() const noexcept;
    [[nodiscard]] MaterialFilter GetFilter() const noexcept;
    void SetFilter(MaterialFilter value) noexcept;
    [[nodiscard]] MaterialAddressMode GetAddressMode() const noexcept;
    [[nodiscard]] MaterialAddressMode GetAddressModeU() const noexcept;
    [[nodiscard]] MaterialAddressMode GetAddressModeV() const noexcept;
    void SetAddressMode(MaterialAddressMode value) noexcept;
    void SetAddressModes(MaterialAddressMode addressU, MaterialAddressMode addressV) noexcept;
    [[nodiscard]] bool NearlyEquals(const Material& other, float epsilon = 0.0001F) const;

  private:
    std::string m_name = "Default";
    DirectX::SimpleMath::Color m_baseColor = {1.0F, 1.0F, 1.0F, 1.0F};
    float m_diffuseStrength = 1.0F;
    DirectX::SimpleMath::Color m_specularColor = {1.0F, 1.0F, 1.0F, 1.0F};
    float m_specularStrength = 0.25F;
    float m_shininess = 32.0F;
    bool m_doubleSided = false;
    SurfaceDisplayMode m_displayMode = SurfaceDisplayMode::LitUntextured;
    MaterialTextureSource m_textureSource = MaterialTextureSource::Source;
    std::filesystem::path m_baseColorTexturePath;
    std::string m_embeddedBaseColorTextureKey;
    MaterialFilter m_filter = MaterialFilter::Linear;
    MaterialAddressMode m_addressModeU = MaterialAddressMode::Wrap;
    MaterialAddressMode m_addressModeV = MaterialAddressMode::Wrap;
};

} // namespace lrender
