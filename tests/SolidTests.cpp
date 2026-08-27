/**
 * @file WARP-backed parameterized solid mesh generation tests.
 * @author Codex
 * @created 2026-08-26
 * @depends render/PrimitiveFactory.h, render/SolidMeshCache.h, D3D11 WARP
 */
#include "render/SolidMeshCache.h"

#include <d3d11.h>
#include <iostream>
#include <stdexcept>
#include <unordered_set>
#include <wrl/client.h>

namespace {

void Require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void TestParameterizedMeshCache() {
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    D3D_FEATURE_LEVEL featureLevel{};
    const HRESULT result = D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, nullptr, 0,
        D3D11_SDK_VERSION, device.GetAddressOf(), &featureLevel, context.GetAddressOf());
    Require(SUCCEEDED(result), "WARP D3D11 device creation failed");

    lrender::SolidMeshCache cache(device.Get());
    const auto sphere = lrender::SolidGeometry::Sphere({1.0F, 8, 4});
    const lrender::Mesh& first = cache.Resolve(7, sphere);
    Require(first.IndexCount() == 8U * 4U * 6U, "Sphere index count is incorrect");
    const lrender::Mesh& reused = cache.Resolve(7, sphere);
    Require(&first == &reused, "Unchanged solid parameters should reuse the runtime mesh");

    const auto edited = lrender::SolidGeometry::Sphere({2.0F, 12, 6});
    const lrender::Mesh& rebuilt = cache.Resolve(7, edited);
    Require(rebuilt.IndexCount() == 12U * 6U * 6U,
            "Edited topology should rebuild the runtime mesh");

    const auto plane = lrender::SolidGeometry::Plane({{4.0F, 6.0F}, 2, 3});
    Require(cache.Resolve(9, plane).IndexCount() == 2U * 3U * 6U,
            "Subdivided plane index count is incorrect");
    cache.Prune(std::unordered_set<lrender::EntityId>{9});
}

} // namespace

int main() {
    try {
        TestParameterizedMeshCache();
        std::cout << "LRenderSolidTests: all tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "LRenderSolidTests failed: " << error.what() << '\n';
        return 1;
    }
}
