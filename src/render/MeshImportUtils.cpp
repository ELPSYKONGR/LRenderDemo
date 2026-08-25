/**
 * @file Shared mesh-import conversion helper implementation.
 * @author Codex
 * @created 2026-08-25
 * @depends render/MeshImportUtils.h, DirectXTK SimpleMath
 */
#include "render/MeshImportUtils.h"

#include <SimpleMath.h>

namespace lrender::mesh_import {

std::string PathUtf8(const std::filesystem::path& path) {
    const auto text = path.u8string();
    return {reinterpret_cast<const char*>(text.data()), text.size()};
}

void ComputeNormals(
    std::vector<MeshVertex>& vertices, std::span<const std::uint32_t> indices) {
    using DirectX::SimpleMath::Vector3;
    for (MeshVertex& vertex : vertices) {
        vertex.normal = {};
    }
    for (std::size_t index = 0; index + 2 < indices.size(); index += 3) {
        const std::uint32_t first = indices[index];
        const std::uint32_t second = indices[index + 1];
        const std::uint32_t third = indices[index + 2];
        const Vector3 a{vertices[first].position};
        const Vector3 b{vertices[second].position};
        const Vector3 c{vertices[third].position};
        const Vector3 face = (b - a).Cross(c - a);
        for (const std::uint32_t vertexIndex : {first, second, third}) {
            Vector3 normal{vertices[vertexIndex].normal};
            normal += face;
            vertices[vertexIndex].normal = normal;
        }
    }
    for (MeshVertex& vertex : vertices) {
        Vector3 normal{vertex.normal};
        if (normal.LengthSquared() > 0.0F) {
            normal.Normalize();
        } else {
            normal = Vector3::UnitY;
        }
        vertex.normal = normal;
    }
}

} // namespace lrender::mesh_import
