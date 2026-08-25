/**
 * @file OBJ and MTL mesh importer strategy.
 * @author Codex
 * @created 2026-08-25
 * @depends tinyobjloader, render/IModelImporter.h
 */
#pragma once

#include "render/IModelImporter.h"

namespace lrender {

class ObjLoader final : public IModelImporter {
public:
    [[nodiscard]] bool SupportsExtension(std::wstring_view extension) const noexcept override;
    [[nodiscard]] std::shared_ptr<MeshAsset> Import(
        const std::filesystem::path& path, ResourceCache& resources) const override;
};

} // namespace lrender
