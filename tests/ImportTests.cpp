/**
 * @file WARP-backed mesh asset importer regression tests.
 * @author Codex
 * @created 2026-08-25
 * @depends render/ResourceCache.h, D3D11 WARP
 */
#include "render/Dx11ConstantBuffer.h"
#include "render/ResourceCache.h"

#include <cmath>
#include <d3d11.h>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <wrl/client.h>

namespace {

struct alignas(16) TestConstants {
    float values[4];
};

void Require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void RequireNear(float actual, float expected, const char* message) {
    Require(std::abs(actual - expected) < 0.001F, message);
}

void TestConstantBufferBinding() {
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    D3D_FEATURE_LEVEL featureLevel{};
    const HRESULT result = D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, nullptr, 0,
        D3D11_SDK_VERSION, device.GetAddressOf(), &featureLevel, context.GetAddressOf());
    Require(SUCCEEDED(result), "WARP D3D11 device creation failed");

    lrender::Dx11ConstantBuffer<TestConstants> buffer(device.Get());
    buffer.Update(context.Get(), TestConstants{{1.0F, 2.0F, 3.0F, 4.0F}});
    buffer.BindVS(context.Get(), 2);
    buffer.BindPS(context.Get(), 3);

    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> pixelBuffer;
    context->VSGetConstantBuffers(2, 1, vertexBuffer.GetAddressOf());
    context->PSGetConstantBuffers(3, 1, pixelBuffer.GetAddressOf());
    Require(vertexBuffer != nullptr, "Constant buffer should be bound to the vertex shader");
    Require(pixelBuffer != nullptr, "Constant buffer should be bound to the pixel shader");
    Require(vertexBuffer.Get() == pixelBuffer.Get(), "Shader stages should share the same buffer");

    D3D11_BUFFER_DESC description{};
    vertexBuffer->GetDesc(&description);
    Require(
        description.ByteWidth == static_cast<UINT>(sizeof(TestConstants)),
        "Constant buffer size is incorrect");
    Require(
        (description.BindFlags & D3D11_BIND_CONSTANT_BUFFER) != 0,
        "Constant buffer bind flag is missing");
}

void TestObjImport() {
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    D3D_FEATURE_LEVEL featureLevel{};
    const HRESULT result = D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, nullptr, 0,
        D3D11_SDK_VERSION, device.GetAddressOf(), &featureLevel, context.GetAddressOf());
    Require(SUCCEEDED(result), "WARP D3D11 device creation failed");

    lrender::ResourceCache resources(device.Get(), context.Get());
    const std::filesystem::path path =
        std::filesystem::path(LRENDER_TEST_ASSET_DIR) / "obj" / "mixed.obj";
    const auto asset = resources.LoadMeshAsset(path);
    Require(asset->entities.size() == 2, "OBJ shapes should become separate asset entities");
    Require(asset->entities[0].name == "RedTriangle", "First OBJ shape name is incorrect");
    Require(asset->entities[1].name == "BlueTriangle", "Second OBJ shape name is incorrect");
    Require(asset->entities[0].parts.size() == 1, "First OBJ shape should have one part");
    Require(asset->entities[1].parts.size() == 1, "Second OBJ shape should have one part");
    Require(asset->entities[0].parts[0].mesh->IndexCount() == 3, "OBJ triangle index count is wrong");
    RequireNear(
        asset->entities[0].parts[0].material.baseColorFactor.x, 0.8F,
        "OBJ MTL diffuse color was not imported");
    RequireNear(
        asset->entities[1].parts[0].material.shininess, 16.0F,
        "OBJ MTL shininess was not imported");

    const auto cached = resources.LoadMeshAsset(path);
    Require(asset == cached, "Repeated OBJ loads should return the cached mesh asset");
    Require(resources.MeshAssetCount() == 1, "OBJ cache should contain one mesh asset");
}

} // namespace

int main() {
    try {
        TestConstantBufferBinding();
        TestObjImport();
        std::cout << "LRenderImportTests: all tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "LRenderImportTests failed: " << error.what() << '\n';
        return 1;
    }
}
