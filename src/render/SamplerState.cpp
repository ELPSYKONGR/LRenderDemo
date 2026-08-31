/**
 * @file D3D11 sampler creation implementation.
 * @author Codex
 * @created 2026-08-21
 * @depends render/SamplerState.h
 */
#include "render/SamplerState.h"

#include <stdexcept>

namespace lrender {

SamplerState::SamplerState(ID3D11Device* device, const SamplerDescription& description) {
    if (device == nullptr) {
        throw std::invalid_argument("Sampler creation requires a D3D11 device");
    }
    D3D11_SAMPLER_DESC native{};
    native.Filter = description.filter;
    native.AddressU = description.addressU;
    native.AddressV = description.addressV;
    native.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    native.MaxAnisotropy = description.filter == D3D11_FILTER_ANISOTROPIC ? 8U : 1U;
    native.ComparisonFunc = D3D11_COMPARISON_NEVER;
    native.MinLOD = 0.0F;
    native.MaxLOD = D3D11_FLOAT32_MAX;
    if (FAILED(device->CreateSamplerState(&native, m_state.GetAddressOf()))) {
        throw std::runtime_error("Failed to create D3D11 sampler state");
    }
}

} // namespace lrender
