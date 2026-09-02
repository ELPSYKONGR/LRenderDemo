/**
 * @file D3D11 texture sampler value object.
 * @author Codex
 * @created 2026-08-21
 * @depends D3D11
 */
#pragma once

#include <d3d11.h>
#include <wrl/client.h>

namespace lrender {

struct SamplerDescription {
    D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    D3D11_TEXTURE_ADDRESS_MODE addressU = D3D11_TEXTURE_ADDRESS_WRAP;
    D3D11_TEXTURE_ADDRESS_MODE addressV = D3D11_TEXTURE_ADDRESS_WRAP;
};

class SamplerState final {
public:
    SamplerState(ID3D11Device* device, const SamplerDescription& description);
    [[nodiscard]] ID3D11SamplerState* Get() const noexcept { return m_state.Get(); }

private:
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_state;
};

} // namespace lrender
