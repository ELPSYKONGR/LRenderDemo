/**
 * @file D3D11 texture loading implementation.
 * @author Codex
 * @created 2026-08-21
 * @depends render/Texture2D.h, DirectXTK WIC/DDS texture loaders
 */
#include "render/Texture2D.h"

#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include <algorithm>
#include <array>
#include <stdexcept>

namespace lrender {
namespace {

void ValidateDevice(ID3D11Device* device, ID3D11DeviceContext* context) {
    if (device == nullptr || context == nullptr) {
        throw std::invalid_argument("Texture loading requires a D3D11 device and context");
    }
}

std::string PathForMessage(const std::filesystem::path& path) {
    const auto text = path.u8string();
    return {reinterpret_cast<const char*>(text.data()), text.size()};
}

bool IsDds(std::span<const std::byte> bytes) {
    constexpr std::array<std::byte, 4> magic{
        std::byte{'D'}, std::byte{'D'}, std::byte{'S'}, std::byte{' '}};
    return bytes.size() >= magic.size() && std::equal(magic.begin(), magic.end(), bytes.begin());
}

} // namespace

Texture2D::Texture2D(
    Microsoft::WRL::ComPtr<ID3D11Resource> resource,
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView,
    std::string sourceName)
    : resource_(std::move(resource)), shaderResourceView_(std::move(shaderResourceView)),
      sourceName_(std::move(sourceName)) {}

std::shared_ptr<Texture2D> Texture2D::LoadFile(
    ID3D11Device* device, ID3D11DeviceContext* context,
    const std::filesystem::path& path, bool forceSrgb) {
    ValidateDevice(device, context);
    if (!std::filesystem::is_regular_file(path)) {
        throw std::runtime_error("Texture file does not exist: " + PathForMessage(path));
    }

    Microsoft::WRL::ComPtr<ID3D11Resource> resource;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> view;
    std::wstring extension = path.extension().wstring();
    std::ranges::transform(extension, extension.begin(), ::towlower);
    HRESULT result{};
    if (extension == L".dds") {
        const auto flags = forceSrgb ? DirectX::DDS_LOADER_FORCE_SRGB : DirectX::DDS_LOADER_DEFAULT;
        result = DirectX::CreateDDSTextureFromFileEx(
            device, context, path.c_str(), 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE,
            0, 0, flags, resource.GetAddressOf(), view.GetAddressOf());
    } else {
        const auto flags = forceSrgb ? DirectX::WIC_LOADER_FORCE_SRGB : DirectX::WIC_LOADER_DEFAULT;
        result = DirectX::CreateWICTextureFromFileEx(
            device, context, path.c_str(), 0, D3D11_USAGE_DEFAULT,
            D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 0,
            D3D11_RESOURCE_MISC_GENERATE_MIPS, flags, resource.GetAddressOf(), view.GetAddressOf());
    }
    if (FAILED(result)) {
        throw std::runtime_error("Failed to load texture: " + PathForMessage(path));
    }
    return std::shared_ptr<Texture2D>(
        new Texture2D(std::move(resource), std::move(view), PathForMessage(path)));
}

std::shared_ptr<Texture2D> Texture2D::LoadMemory(
    ID3D11Device* device, ID3D11DeviceContext* context,
    std::span<const std::byte> bytes, std::string sourceName, bool forceSrgb) {
    ValidateDevice(device, context);
    if (bytes.empty()) {
        throw std::invalid_argument("Texture memory is empty: " + sourceName);
    }

    Microsoft::WRL::ComPtr<ID3D11Resource> resource;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> view;
    HRESULT result{};
    if (IsDds(bytes)) {
        const auto flags = forceSrgb ? DirectX::DDS_LOADER_FORCE_SRGB : DirectX::DDS_LOADER_DEFAULT;
        result = DirectX::CreateDDSTextureFromMemoryEx(
            device, context, reinterpret_cast<const std::uint8_t*>(bytes.data()), bytes.size(),
            0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, flags,
            resource.GetAddressOf(), view.GetAddressOf());
    } else {
        const auto flags = forceSrgb ? DirectX::WIC_LOADER_FORCE_SRGB : DirectX::WIC_LOADER_DEFAULT;
        result = DirectX::CreateWICTextureFromMemoryEx(
            device, context, reinterpret_cast<const std::uint8_t*>(bytes.data()), bytes.size(),
            0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
            0, D3D11_RESOURCE_MISC_GENERATE_MIPS, flags,
            resource.GetAddressOf(), view.GetAddressOf());
    }
    if (FAILED(result)) {
        throw std::runtime_error("Failed to decode embedded texture: " + sourceName);
    }
    return std::shared_ptr<Texture2D>(
        new Texture2D(std::move(resource), std::move(view), std::move(sourceName)));
}

std::shared_ptr<Texture2D> Texture2D::CreateChecker(ID3D11Device* device) {
    if (device == nullptr) {
        throw std::invalid_argument("Checker texture requires a D3D11 device");
    }
    constexpr std::uint32_t textureSize = 64;
    constexpr std::uint32_t cellSize = 8;
    std::array<std::uint32_t, textureSize * textureSize> pixels{};
    for (std::uint32_t y = 0; y < textureSize; ++y) {
        for (std::uint32_t x = 0; x < textureSize; ++x) {
            const bool isLight = ((x / cellSize) + (y / cellSize)) % 2 == 0;
            pixels[y * textureSize + x] = isLight ? 0xffe8e8e8U : 0xff303030U;
        }
    }
    D3D11_TEXTURE2D_DESC description{};
    description.Width = textureSize;
    description.Height = textureSize;
    description.MipLevels = 1;
    description.ArraySize = 1;
    description.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    description.SampleDesc.Count = 1;
    description.Usage = D3D11_USAGE_IMMUTABLE;
    description.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA initialData{
        pixels.data(), textureSize * sizeof(std::uint32_t), 0};

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    if (FAILED(device->CreateTexture2D(&description, &initialData, texture.GetAddressOf()))) {
        throw std::runtime_error("Failed to create generated checker texture");
    }
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> view;
    if (FAILED(device->CreateShaderResourceView(texture.Get(), nullptr, view.GetAddressOf()))) {
        throw std::runtime_error("Failed to create checker texture view");
    }
    Microsoft::WRL::ComPtr<ID3D11Resource> resource;
    if (FAILED(texture.As(&resource))) {
        throw std::runtime_error("Failed to retain checker texture resource");
    }
    return std::shared_ptr<Texture2D>(
        new Texture2D(std::move(resource), std::move(view), "generated://checker"));
}

std::shared_ptr<Texture2D> Texture2D::CreateSolidWhite(ID3D11Device* device) {
    if (device == nullptr) {
        throw std::invalid_argument("Solid texture requires a D3D11 device");
    }
    constexpr std::uint32_t pixel = 0xffffffffU;
    D3D11_TEXTURE2D_DESC description{};
    description.Width = 1;
    description.Height = 1;
    description.MipLevels = 1;
    description.ArraySize = 1;
    description.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    description.SampleDesc.Count = 1;
    description.Usage = D3D11_USAGE_IMMUTABLE;
    description.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA initialData{&pixel, sizeof(pixel), 0};

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    if (FAILED(device->CreateTexture2D(&description, &initialData, texture.GetAddressOf()))) {
        throw std::runtime_error("Failed to create generated white texture");
    }
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> view;
    if (FAILED(device->CreateShaderResourceView(texture.Get(), nullptr, view.GetAddressOf()))) {
        throw std::runtime_error("Failed to create white texture view");
    }
    Microsoft::WRL::ComPtr<ID3D11Resource> resource;
    if (FAILED(texture.As(&resource))) {
        throw std::runtime_error("Failed to retain white texture resource");
    }
    return std::shared_ptr<Texture2D>(
        new Texture2D(std::move(resource), std::move(view), "generated://white"));
}

} // namespace lrender
