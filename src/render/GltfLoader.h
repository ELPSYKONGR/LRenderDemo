/**
 * @file glTF 2.0 and GLB model importer boundary.
 * @author Codex
 * @created 2026-08-21
 * @depends cgltf, render/Model.h, render/ResourceCache.h
 */
#pragma once

#include "render/Model.h"

#include <filesystem>
#include <memory>

namespace lrender {

class ResourceCache;

class GltfLoader final {
public:
    [[nodiscard]] static std::shared_ptr<Model> Load(
        const std::filesystem::path& path, ResourceCache& resources);
};

} // namespace lrender
