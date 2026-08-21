/**
 * @file Imported model made of cached mesh/material parts.
 * @author Codex
 * @created 2026-08-21
 * @depends render/Mesh.h, render/Material.h
 */
#pragma once

#include "render/Material.h"
#include "render/Mesh.h"

#include <memory>
#include <string>
#include <vector>

namespace lrender {

struct ModelPart {
    std::unique_ptr<Mesh> mesh;
    Material material;
};

class Model final {
public:
    std::string name;
    std::vector<ModelPart> parts;
};

} // namespace lrender
