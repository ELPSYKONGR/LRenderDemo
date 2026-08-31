/**
 * @file Registry and dispatcher for model-import format strategies.
 * @author Codex
 * @created 2026-08-25
 * @depends render/IModelImporter.h
 */
#pragma once

#include "render/IModelImporter.h"

#include <memory>
#include <vector>

namespace lrender {

class ModelLoader final {
public:
    ModelLoader();
    [[nodiscard]] std::shared_ptr<MeshAsset> Load(
        const std::filesystem::path& path, ResourceCache& resources) const;

private:
    std::vector<std::unique_ptr<IModelImporter>> m_importers;
};

} // namespace lrender
