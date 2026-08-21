/**
 * @file D3D11 mesh buffer implementation.
 * @author Codex
 * @created 2026-08-20
 * @depends render/Mesh.h
 */
#include "render/Mesh.h"

#include <limits>
#include <stdexcept>

namespace lrender {

Mesh::Mesh(
    ID3D11Device* device,
    std::span<const DirectX::VertexPositionNormalColor> vertices,
    std::span<const std::uint16_t> indices) {
    if (device == nullptr || vertices.empty() || indices.empty()) {
        throw std::invalid_argument("Mesh requires a device and non-empty geometry");
    }
    if (vertices.size_bytes() > std::numeric_limits<UINT>::max() ||
        indices.size_bytes() > std::numeric_limits<UINT>::max()) {
        throw std::overflow_error("Mesh data exceeds D3D11 buffer limits");
    }

    D3D11_BUFFER_DESC vertexDescription{};
    vertexDescription.ByteWidth = static_cast<UINT>(vertices.size_bytes());
    vertexDescription.Usage = D3D11_USAGE_IMMUTABLE;
    vertexDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA vertexData{vertices.data(), 0, 0};
    if (FAILED(device->CreateBuffer(
            &vertexDescription, &vertexData, vertexBuffer_.ReleaseAndGetAddressOf()))) {
        throw std::runtime_error("Failed to create mesh vertex buffer");
    }

    D3D11_BUFFER_DESC indexDescription{};
    indexDescription.ByteWidth = static_cast<UINT>(indices.size_bytes());
    indexDescription.Usage = D3D11_USAGE_IMMUTABLE;
    indexDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
    D3D11_SUBRESOURCE_DATA indexData{indices.data(), 0, 0};
    if (FAILED(device->CreateBuffer(
            &indexDescription, &indexData, indexBuffer_.ReleaseAndGetAddressOf()))) {
        throw std::runtime_error("Failed to create mesh index buffer");
    }
    indexCount_ = static_cast<std::uint32_t>(indices.size());
}

void Mesh::Draw(ID3D11DeviceContext* context) const {
    if (context == nullptr) {
        throw std::invalid_argument("Mesh draw requires a D3D11 context");
    }
    constexpr UINT stride = sizeof(DirectX::VertexPositionNormalColor);
    constexpr UINT offset = 0;
    ID3D11Buffer* vertexBuffer = vertexBuffer_.Get();
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R16_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->DrawIndexed(indexCount_, 0, 0);
}

} // namespace lrender
