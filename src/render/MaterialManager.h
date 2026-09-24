/**
 * @file Material resolution and DX11 texture/sampler resource management.
 */
#pragma once

#include "core/Material.h"
#include "render/SamplerState.h"
#include "render/Texture2D.h"

#include <cstddef>
#include <d3d11.h>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>

namespace lrender
{

struct MaterialDrawData
{
    Material material;
    std::shared_ptr<Texture2D> baseColorTexture;
    std::shared_ptr<SamplerState> sampler;
};

class MaterialManager final
{
  public:
    static void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    static void Shutdown() noexcept;
    [[nodiscard]] static MaterialManager& Instance();

    MaterialManager(const MaterialManager&) = delete;
    MaterialManager& operator=(const MaterialManager&) = delete;

    [[nodiscard]] MaterialDrawData PrepareMaterial(const Material& source, const Material* overrideMaterial = nullptr);
    [[nodiscard]] static std::string SerializeMaterial(const Material& material,
                                                       const std::filesystem::path& sceneDirectory);
    [[nodiscard]] static Material DeserializeMaterial(std::string_view value,
                                                      const std::filesystem::path& sceneDirectory);
    [[nodiscard]] std::shared_ptr<Texture2D> LoadTexture(const std::filesystem::path& path);
    void RegisterEmbeddedTexture(std::string key, std::span<const std::byte> bytes);
    [[nodiscard]] Material DefaultMaterial() const;
    [[nodiscard]] Material CheckerMaterial() const;
    [[nodiscard]] std::size_t TextureCount() const noexcept;

  private:
    MaterialManager(ID3D11Device* device, ID3D11DeviceContext* context);

    [[nodiscard]] std::shared_ptr<Texture2D> ResolveTexture(const Material& material);
    [[nodiscard]] std::shared_ptr<SamplerState> ResolveSampler(const Material& material);
    [[nodiscard]] static std::wstring NormalizePath(const std::filesystem::path& path);
    [[nodiscard]] static std::uint64_t SamplerKey(const SamplerDescription& description) noexcept;

    static std::unique_ptr<MaterialManager> m_instance;
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;
    std::unordered_map<std::wstring, std::shared_ptr<Texture2D>> m_textures;
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_embeddedTextures;
    std::unordered_map<std::uint64_t, std::shared_ptr<SamplerState>> m_samplers;
    std::shared_ptr<Texture2D> m_whiteTexture;
    std::shared_ptr<Texture2D> m_checkerTexture;
};

} // namespace lrender
