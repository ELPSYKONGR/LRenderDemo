/**
 * @file Procedural cube and UV sphere generation.
 * @author Codex
 * @created 2026-08-20
 * @depends render/PrimitiveFactory.h
 */
#include "render/PrimitiveFactory.h"

#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

namespace lrender {
namespace {

using Vertex = DirectX::VertexPositionNormalColor;
const DirectX::XMFLOAT4 kWhite{1.0F, 1.0F, 1.0F, 1.0F};

void AddFace(
    std::vector<Vertex>& vertices,
    std::vector<std::uint16_t>& indices,
    const std::array<DirectX::XMFLOAT3, 4>& positions,
    const DirectX::XMFLOAT3& normal) {
    const auto base = static_cast<std::uint16_t>(vertices.size());
    for (const auto& position : positions) {
        vertices.emplace_back(position, normal, kWhite);
    }
    const std::array<std::uint16_t, 6> faceIndices{
        base, static_cast<std::uint16_t>(base + 1), static_cast<std::uint16_t>(base + 2),
        base, static_cast<std::uint16_t>(base + 2), static_cast<std::uint16_t>(base + 3)};
    indices.insert(indices.end(), faceIndices.begin(), faceIndices.end());
}

} // namespace

std::unique_ptr<Mesh> PrimitiveFactory::CreateCube(ID3D11Device* device) {
    constexpr float half = 0.5F;
    std::vector<Vertex> vertices;
    std::vector<std::uint16_t> indices;
    vertices.reserve(24);
    indices.reserve(36);

    AddFace(vertices, indices, {{{-half, -half, half}, {half, -half, half},
                                 {half, half, half}, {-half, half, half}}}, {0, 0, 1});
    AddFace(vertices, indices, {{{half, -half, -half}, {-half, -half, -half},
                                 {-half, half, -half}, {half, half, -half}}}, {0, 0, -1});
    AddFace(vertices, indices, {{{half, -half, half}, {half, -half, -half},
                                 {half, half, -half}, {half, half, half}}}, {1, 0, 0});
    AddFace(vertices, indices, {{{-half, -half, -half}, {-half, -half, half},
                                 {-half, half, half}, {-half, half, -half}}}, {-1, 0, 0});
    AddFace(vertices, indices, {{{-half, half, half}, {half, half, half},
                                 {half, half, -half}, {-half, half, -half}}}, {0, 1, 0});
    AddFace(vertices, indices, {{{-half, -half, -half}, {half, -half, -half},
                                 {half, -half, half}, {-half, -half, half}}}, {0, -1, 0});
    return std::make_unique<Mesh>(device, vertices, indices);
}

std::unique_ptr<Mesh> PrimitiveFactory::CreateSphere(
    ID3D11Device* device, std::uint16_t slices, std::uint16_t stacks) {
    if (slices < 3 || stacks < 2) {
        throw std::invalid_argument("Sphere requires at least 3 slices and 2 stacks");
    }
    const std::uint32_t vertexCount =
        static_cast<std::uint32_t>(slices + 1) * static_cast<std::uint32_t>(stacks + 1);
    if (vertexCount > std::numeric_limits<std::uint16_t>::max()) {
        throw std::overflow_error("Sphere exceeds 16-bit mesh index capacity");
    }

    std::vector<Vertex> vertices;
    std::vector<std::uint16_t> indices;
    vertices.reserve(vertexCount);
    indices.reserve(static_cast<std::size_t>(slices) * stacks * 6);

    for (std::uint16_t stack = 0; stack <= stacks; ++stack) {
        const float latitude = DirectX::XM_PI * static_cast<float>(stack) / stacks;
        const float y = std::cos(latitude);
        const float radius = std::sin(latitude);
        for (std::uint16_t slice = 0; slice <= slices; ++slice) {
            const float longitude = DirectX::XM_2PI * static_cast<float>(slice) / slices;
            const DirectX::XMFLOAT3 normal{
                radius * std::sin(longitude), y, radius * std::cos(longitude)};
            const DirectX::XMFLOAT3 position{normal.x * 0.5F, normal.y * 0.5F, normal.z * 0.5F};
            vertices.emplace_back(position, normal, kWhite);
        }
    }

    const std::uint16_t row = static_cast<std::uint16_t>(slices + 1);
    for (std::uint16_t stack = 0; stack < stacks; ++stack) {
        for (std::uint16_t slice = 0; slice < slices; ++slice) {
            const auto first = static_cast<std::uint16_t>(stack * row + slice);
            const auto second = static_cast<std::uint16_t>(first + row);
            indices.insert(indices.end(), {first, second, static_cast<std::uint16_t>(first + 1),
                                           static_cast<std::uint16_t>(first + 1), second,
                                           static_cast<std::uint16_t>(second + 1)});
        }
    }
    return std::make_unique<Mesh>(device, vertices, indices);
}

} // namespace lrender
