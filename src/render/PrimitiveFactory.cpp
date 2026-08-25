/**
 * @file Procedural cube, UV sphere, and plane generation.
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

using Vertex = MeshVertex;

void AddFace(
    std::vector<Vertex>& vertices,
    std::vector<std::uint32_t>& indices,
    const std::array<DirectX::XMFLOAT3, 4>& positions,
    const DirectX::XMFLOAT3& normal) {
    const auto base = static_cast<std::uint32_t>(vertices.size());
    constexpr std::array<DirectX::XMFLOAT2, 4> textureCoordinates{
        DirectX::XMFLOAT2{0.0F, 1.0F}, DirectX::XMFLOAT2{1.0F, 1.0F},
        DirectX::XMFLOAT2{1.0F, 0.0F}, DirectX::XMFLOAT2{0.0F, 0.0F}};
    for (std::size_t index = 0; index < positions.size(); ++index) {
        vertices.push_back({positions[index], normal, textureCoordinates[index]});
    }
    const std::array<std::uint32_t, 6> faceIndices{
        base, base + 1, base + 2, base, base + 2, base + 3};
    indices.insert(indices.end(), faceIndices.begin(), faceIndices.end());
}

} // namespace

std::unique_ptr<Mesh> PrimitiveFactory::CreateCube(ID3D11Device* device) {
    constexpr float half = 0.5F;
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
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
    const std::size_t vertexCount =
        (static_cast<std::size_t>(slices) + 1) * (static_cast<std::size_t>(stacks) + 1);
    if (vertexCount > std::numeric_limits<std::uint32_t>::max()) {
        throw std::overflow_error("Sphere exceeds the 32-bit vertex limit");
    }
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    vertices.reserve(vertexCount);
    indices.reserve(static_cast<std::size_t>(slices) * stacks * 6);

    for (std::uint32_t stack = 0; stack <= stacks; ++stack) {
        const float latitude = DirectX::XM_PI * static_cast<float>(stack) / stacks;
        const float y = std::cos(latitude);
        const float radius = std::sin(latitude);
        for (std::uint32_t slice = 0; slice <= slices; ++slice) {
            const float longitude = DirectX::XM_2PI * static_cast<float>(slice) / slices;
            const DirectX::XMFLOAT3 normal{
                radius * std::sin(longitude), y, radius * std::cos(longitude)};
            const DirectX::XMFLOAT3 position{normal.x * 0.5F, normal.y * 0.5F, normal.z * 0.5F};
            const DirectX::XMFLOAT2 textureCoordinate{
                static_cast<float>(slice) / slices,
                static_cast<float>(stack) / stacks};
            vertices.push_back({position, normal, textureCoordinate});
        }
    }

    const std::uint32_t row = static_cast<std::uint32_t>(slices + 1);
    for (std::uint32_t stack = 0; stack < stacks; ++stack) {
        for (std::uint32_t slice = 0; slice < slices; ++slice) {
            const auto first = static_cast<std::uint32_t>(stack) * row + slice;
            const auto second = first + row;
            indices.insert(indices.end(), {first, second, first + 1, first + 1, second, second + 1});
        }
    }
    return std::make_unique<Mesh>(device, vertices, indices);
}

std::unique_ptr<Mesh> PrimitiveFactory::CreatePlane(ID3D11Device* device) {
    constexpr float halfExtent = 5.0F;
    constexpr float uvRepeat = 5.0F;
    constexpr std::array<Vertex, 4> vertices{{
        {{-halfExtent, 0.0F, -halfExtent}, {0.0F, 1.0F, 0.0F}, {0.0F, uvRepeat}},
        {{-halfExtent, 0.0F, halfExtent}, {0.0F, 1.0F, 0.0F}, {0.0F, 0.0F}},
        {{halfExtent, 0.0F, halfExtent}, {0.0F, 1.0F, 0.0F}, {uvRepeat, 0.0F}},
        {{halfExtent, 0.0F, -halfExtent}, {0.0F, 1.0F, 0.0F}, {uvRepeat, uvRepeat}},
    }};
    constexpr std::array<std::uint32_t, 6> indices{0, 1, 2, 0, 2, 3};
    return std::make_unique<Mesh>(device, vertices, indices);
}

} // namespace lrender
