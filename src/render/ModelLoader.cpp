/**
 * @file Model importer registration and extension dispatch.
 * @author Codex
 * @created 2026-08-25
 * @depends render/ModelLoader.h, render/GltfLoader.h, render/ObjLoader.h
 */
#include "render/ModelLoader.h"

#include "render/GltfLoader.h"
#include "render/ObjLoader.h"

#include <algorithm>
#include <cwctype>
#include <stdexcept>

namespace lrender {

ModelLoader::ModelLoader() {
    importers_.push_back(std::make_unique<GltfLoader>());
    importers_.push_back(std::make_unique<ObjLoader>());
}

std::shared_ptr<MeshAsset> ModelLoader::Load(
    const std::filesystem::path& path, ResourceCache& resources) const {
    std::wstring extension = path.extension().wstring();
    std::ranges::transform(extension, extension.begin(), ::towlower);
    for (const auto& importer : importers_) {
        if (importer->SupportsExtension(extension)) {
            return importer->Import(path, resources);
        }
    }
    throw std::invalid_argument("Unsupported mesh asset extension: " + path.extension().string());
}

} // namespace lrender
