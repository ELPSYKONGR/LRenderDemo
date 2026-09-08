/**
 * @file Fullscreen TextureCube sky effect.
 */
#pragma once


namespace lrender
{


#define LRENDER_SKYCUBE_DIR  "assets/skyEffectRecources/desertcube1024.dds"

class SkyCubeEffect final : public IRenderEffect
{
  public:
    SkyCubeEffect(ID3D11Device* device, ID3D11DeviceContext* context, const std::filesystem::path& shaderDirectory,
        const std::filesystem::path& cubeMapPath = LRENDER_SKYCUBE_DIR);

    void Bind(const EffectFrameContext& frame);
    void Bind(const EffectFrameContext& frame, const EffectDrawContext& draw) override;
    void Draw(const EffectFrameContext& frame);
    void Draw(const EffectFrameContext& frame, const EffectDrawContext& draw) override;
    [[nodiscard]] std::string_view Name() const noexcept override;

    [[nodiscard]] const EffectCubeMapResource& CubeMapResource() const noexcept;

  private:
    std::unique_ptr<DirectX::CommonStates> m_states;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    EffectCubeMapResource m_cubeMapResource;
};

} // namespace lrender
