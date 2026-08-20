/**
 * @file Procedural learning geometry factory.
 * @author Codex
 * @created 2026-08-20
 * @depends render/Mesh.h
 */
#pragma once

#include "render/Mesh.h"

#include <memory>

namespace lrender {

class PrimitiveFactory final {
public:
    [[nodiscard]] static std::unique_ptr<Mesh> CreateCube(ID3D11Device* device);
    [[nodiscard]] static std::unique_ptr<Mesh> CreateSphere(
        ID3D11Device* device, std::uint16_t slices = 32, std::uint16_t stacks = 20);
};

} // namespace lrender
