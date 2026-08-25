/**
 * @file Format strategy boundary for imported mesh assets.
 * @author Codex
 * @created 2026-08-25
 * @depends render/MeshAsset.h
 */
#pragma once

#include "render/MeshAsset.h"

#include <filesystem>
#include <memory>
#include <string_view>

namespace lrender {

class ResourceCache;

class IModelImporter {
public:
    virtual ~IModelImporter() = default;
    [[nodiscard]] virtual bool SupportsExtension(std::wstring_view extension) const noexcept = 0;
    [[nodiscard]] virtual std::shared_ptr<MeshAsset> Import(
        const std::filesystem::path& path, ResourceCache& resources) const = 0;
};

} // namespace lrender
