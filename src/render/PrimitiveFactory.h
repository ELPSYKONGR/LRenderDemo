/**
 * @file Procedural learning geometry factory.
 * @author Codex
 * @created 2026-08-20
 * @depends render/Mesh.h
 */
#pragma once

#include "core/SolidGeometry.h"
#include "render/Mesh.h"

#include <memory>

namespace lrender
{

class PrimitiveFactory final
{
  public:
    [[nodiscard]] static std::unique_ptr<Mesh> Create(ID3D11Device* device, const SolidGeometry& geometry);

  private:
    [[nodiscard]] static std::unique_ptr<Mesh> CreateCube(ID3D11Device* device, const CubeParameters& parameters);
    [[nodiscard]] static std::unique_ptr<Mesh> CreateSphere(ID3D11Device* device, const SphereParameters& parameters);
    [[nodiscard]] static std::unique_ptr<Mesh> CreatePlane(ID3D11Device* device, const PlaneParameters& parameters);
};

} // namespace lrender
