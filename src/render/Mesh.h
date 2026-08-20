/**
 * @file Immutable D3D11 vertex and index buffer pair.
 * @author Codex
 * @created 2026-08-20
 * @depends D3D11, DirectXTK VertexTypes
 */
#pragma once

#include <VertexTypes.h>
#include <cstdint>
#include <d3d11.h>
#include <span>
#include <wrl/client.h>

namespace lrender {

class Mesh final {
public:
    Mesh(
        ID3D11Device* device,
        std::span<const DirectX::VertexPositionNormalColor> vertices,
        std::span<const std::uint16_t> indices);

    void Draw(ID3D11DeviceContext* context) const;
    [[nodiscard]] std::uint32_t IndexCount() const noexcept { return indexCount_; }

private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer_;
    std::uint32_t indexCount_{};
};

} // namespace lrender
