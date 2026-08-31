/**
 * @file Per-entity runtime cache for parameterized solid meshes.
 * @author Codex
 * @created 2026-08-26
 * @depends core/Scene.h, render/Mesh.h
 */
#pragma once

#include "core/Scene.h"
#include "render/Mesh.h"

#include <d3d11.h>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <wrl/client.h>

namespace lrender {

class SolidMeshCache final {
public:
    explicit SolidMeshCache(ID3D11Device* device);

    [[nodiscard]] const Mesh& Resolve(EntityId entityId, const SolidGeometry& geometry);
    void Prune(const std::unordered_set<EntityId>& activeEntities);
    void Clear() noexcept { m_entries.clear(); }

private:
    struct Entry {
        SolidGeometry generatedFrom;
        std::unique_ptr<Mesh> mesh;
    };

    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    std::unordered_map<EntityId, Entry> m_entries;
};

} // namespace lrender
