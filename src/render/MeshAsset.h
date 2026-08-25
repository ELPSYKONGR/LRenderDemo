/**
 * @file Cached imported mesh asset with entity and material-part boundaries.
 * @author Codex
 * @created 2026-08-25
 * @depends render/Mesh.h, render/Material.h
 */
#pragma once

#include "render/Material.h"
#include "render/Mesh.h"

#include <memory>
#include <string>
#include <vector>

namespace lrender {

struct MeshPart {
    std::unique_ptr<Mesh> mesh;
    Material material;
};

struct MeshAssetEntity {
    std::string name;
    std::vector<MeshPart> parts;
};

class MeshAsset final {
public:
    std::string name;
    std::vector<MeshAssetEntity> entities;
    std::vector<std::string> warnings;
};

} // namespace lrender
