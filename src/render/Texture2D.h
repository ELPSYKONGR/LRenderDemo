/**
 * @file D3D11 shader-resource texture with file, memory, and generated sources.
 * @author Codex
 * @created 2026-08-21
 * @depends D3D11, DirectXTK WIC/DDS texture loaders
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <d3d11.h>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <wrl/client.h>

namespace lrender
{

class Texture2D final
{
  public:
    [[nodiscard]] static std::shared_ptr<Texture2D> LoadFile(ID3D11Device* device, ID3D11DeviceContext* context,
                                                             const std::filesystem::path& path, bool forceSrgb = true);
    [[nodiscard]] static std::shared_ptr<Texture2D> LoadMemory(ID3D11Device* device, ID3D11DeviceContext* context,
                                                               std::span<const std::byte> bytes, std::string sourceName,
                                                               bool forceSrgb = true);
    [[nodiscard]] static std::shared_ptr<Texture2D> CreateChecker(ID3D11Device* device);
    [[nodiscard]] static std::shared_ptr<Texture2D> CreateSolidWhite(ID3D11Device* device);

    [[nodiscard]] ID3D11ShaderResourceView* ShaderResourceView() const noexcept;
    [[nodiscard]] const std::string& SourceName() const noexcept;

  private:
    Texture2D(Microsoft::WRL::ComPtr<ID3D11Resource> resource,
              Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView, std::string sourceName);

    Microsoft::WRL::ComPtr<ID3D11Resource> m_resource;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;
    std::string m_sourceName;
};

} // namespace lrender
