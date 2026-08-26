/**
 * @file Typed RAII wrapper for a D3D11 constant buffer.
 * @author Codex
 * @created 2026-08-26
 * @depends D3D11, WRL ComPtr
 */
#pragma once

#include <d3d11.h>
#include <stdexcept>
#include <type_traits>
#include <wrl/client.h>

namespace lrender {

template <typename T>
class Dx11ConstantBuffer final {
    static_assert(std::is_trivially_copyable_v<T>);
    static_assert(sizeof(T) > 0 && sizeof(T) % 16 == 0);
    static_assert(sizeof(T) <= D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16U);

public:
    explicit Dx11ConstantBuffer(ID3D11Device* device) {
        if (device == nullptr) {
            throw std::invalid_argument("Dx11ConstantBuffer requires a D3D11 device");
        }
        D3D11_BUFFER_DESC description{};
        description.ByteWidth = static_cast<UINT>(sizeof(T));
        description.Usage = D3D11_USAGE_DEFAULT;
        description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        if (FAILED(device->CreateBuffer(&description, nullptr, buffer_.GetAddressOf()))) {
            throw std::runtime_error("Failed to create D3D11 constant buffer");
        }
    }

    Dx11ConstantBuffer(const Dx11ConstantBuffer&) = delete;
    Dx11ConstantBuffer& operator=(const Dx11ConstantBuffer&) = delete;
    Dx11ConstantBuffer(Dx11ConstantBuffer&&) noexcept = default;
    Dx11ConstantBuffer& operator=(Dx11ConstantBuffer&&) noexcept = default;

    void Update(ID3D11DeviceContext* context, const T& value) const {
        ValidateContext(context);
        context->UpdateSubresource(buffer_.Get(), 0, nullptr, &value, 0, 0);
    }

    void BindVS(ID3D11DeviceContext* context, UINT slot) const {
        ValidateContext(context);
        ID3D11Buffer* buffer = buffer_.Get();
        context->VSSetConstantBuffers(slot, 1, &buffer);
    }

    void BindPS(ID3D11DeviceContext* context, UINT slot) const {
        ValidateContext(context);
        ID3D11Buffer* buffer = buffer_.Get();
        context->PSSetConstantBuffers(slot, 1, &buffer);
    }

private:
    static void ValidateContext(ID3D11DeviceContext* context) {
        if (context == nullptr) {
            throw std::invalid_argument("Dx11ConstantBuffer requires a D3D11 context");
        }
    }

    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer_;
};

} // namespace lrender
