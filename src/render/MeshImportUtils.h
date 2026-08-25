/**
 * @file Shared mesh-import conversion helpers.
 * @author Codex
 * @created 2026-08-25
 * @depends render/Mesh.h
 */
#pragma once

#include "render/Mesh.h"

#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace lrender::mesh_import {

[[nodiscard]] std::string PathUtf8(const std::filesystem::path& path);
void ComputeNormals(
    std::vector<MeshVertex>& vertices, std::span<const std::uint32_t> indices);

} // namespace lrender::mesh_import
