/**
 * @file Common DX11 render effect helpers.
 */
#include "render/IRenderEffect.h"

#include <d3dcompiler.h>
#include <stdexcept>

namespace lrender {

IRenderEffect::IRenderEffect(ID3D11Device* device) : m_device(device) {
    if (m_device == nullptr) {
        throw std::invalid_argument("Render effect requires a D3D11 device");
    }
}

Microsoft::WRL::ComPtr<ID3DBlob> IRenderEffect::LoadShader(
    const std::filesystem::path& path) {
    Microsoft::WRL::ComPtr<ID3DBlob> shader;
    const HRESULT result = D3DReadFileToBlob(path.c_str(), shader.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to load compiled shader: " + path.string());
    }
    return shader;
}

} // namespace lrender
