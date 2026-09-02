/**
 * @file glTF 2.0 and GLB model importer boundary.
 * @author Codex
 * @created 2026-08-21
 * @depends cgltf, render/IModelImporter.h, render/ResourceCache.h
 */
#pragma once

#include "render/IModelImporter.h"

#include <filesystem>
#include <memory>

namespace lrender
{

class ResourceCache;

class GltfLoader final : public IModelImporter
{
  public:
    [[nodiscard]] bool SupportsExtension(std::wstring_view extension) const noexcept override;
    [[nodiscard]] std::shared_ptr<MeshAsset> Import(const std::filesystem::path& path,
                                                    ResourceCache& resources) const override;
};

} // namespace lrender
