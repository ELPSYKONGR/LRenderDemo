/**
 * @file Parameterized solid mesh cache implementation.
 * @author Codex
 * @created 2026-08-26
 * @depends render/SolidMeshCache.h, render/PrimitiveFactory.h
 */
#include "render/SolidMeshCache.h"

#include "render/PrimitiveFactory.h"

#include <stdexcept>

namespace lrender {

SolidMeshCache::SolidMeshCache(ID3D11Device* device) : m_device(device) {
    if (device == nullptr) {
        throw std::invalid_argument("Solid mesh cache requires a D3D11 device");
    }
}

const Mesh& SolidMeshCache::Resolve(EntityId entityId, const SolidGeometry& geometry) {
    auto found = m_entries.find(entityId);
    if (found != m_entries.end() &&
        found->second.generatedFrom.NearlyEquals(geometry, 0.0F)) {
        return *found->second.mesh;
    }

    Entry entry{geometry, PrimitiveFactory::Create(m_device.Get(), geometry)};
    if (found == m_entries.end()) {
        found = m_entries.emplace(entityId, std::move(entry)).first;
    } else {
        found->second = std::move(entry);
    }
    return *found->second.mesh;
}

void SolidMeshCache::Prune(const std::unordered_set<EntityId>& activeEntities) {
    std::erase_if(m_entries, [&activeEntities](const auto& item) {
        return !activeEntities.contains(item.first);
    });
}

} // namespace lrender
