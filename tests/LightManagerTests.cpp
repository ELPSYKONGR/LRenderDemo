/**
 * @file WARP-backed LightManager regression tests.
 */
#include "render/LightManager.h"

#include <d3d11.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <wrl/client.h>

namespace
{

void Require(bool condition, const char* message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

void TestLightLifecycleAndBuffer()
{
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    D3D_FEATURE_LEVEL featureLevel{};
    const HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, nullptr, 0,
                                             D3D11_SDK_VERSION, device.GetAddressOf(), &featureLevel,
                                             context.GetAddressOf());
    Require(SUCCEEDED(result), "WARP D3D11 device creation failed");

    lrender::LightManager::Initialize(device.Get(), context.Get());
    lrender::LightManager& lights = lrender::LightManager::Instance();
    Require(lights.Directional() != nullptr, "Default directional light is missing");
    Require(lights.PointLights().size() == 3, "Default point light count is incorrect");

    const lrender::LightId directionalId = lights.Directional()->id;
    Require(lights.RemoveLight(directionalId), "Directional light removal failed");
    Require(lights.Directional() == nullptr, "Directional light should be absent after removal");
    const auto replacementDirectional = lights.AddDirectionalLight();
    Require(replacementDirectional.has_value(), "Directional light recreation failed");
    Require(*replacementDirectional != directionalId, "Recreated directional light should receive a new id");
    Require(!lights.AddDirectionalLight().has_value(), "A second directional light should be rejected");

    const auto fourthPoint = lights.AddPointLight();
    Require(fourthPoint.has_value(), "Fourth point light creation failed");
    Require(!lights.AddPointLight().has_value(), "A fifth point light should be rejected");
    Require(lights.RemoveLight(*fourthPoint), "Point light removal failed");
    const auto replacementPoint = lights.AddPointLight();
    Require(replacementPoint.has_value(), "Point light recreation failed");
    Require(*replacementPoint != *fourthPoint, "Recreated point light should receive a new id");
    lights.FindPointLight(*replacementPoint)->enabled = false;

    lights.UpdateBuffer();
    lights.BindBuffer();
    Microsoft::WRL::ComPtr<ID3D11Buffer> lightBuffer;
    context->PSGetConstantBuffers(3, 1, lightBuffer.GetAddressOf());
    Require(lightBuffer != nullptr, "Light buffer should be bound to pixel-shader slot b3");
    D3D11_BUFFER_DESC description{};
    lightBuffer->GetDesc(&description);
    Require(description.ByteWidth == sizeof(lrender::LightConstants), "Light buffer size is incorrect");

    D3D11_BUFFER_DESC stagingDescription = description;
    stagingDescription.Usage = D3D11_USAGE_STAGING;
    stagingDescription.BindFlags = 0;
    stagingDescription.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    Microsoft::WRL::ComPtr<ID3D11Buffer> stagingBuffer;
    Require(SUCCEEDED(device->CreateBuffer(&stagingDescription, nullptr, stagingBuffer.GetAddressOf())),
            "Light staging buffer creation failed");
    context->CopyResource(stagingBuffer.Get(), lightBuffer.Get());
    D3D11_MAPPED_SUBRESOURCE mapped{};
    Require(SUCCEEDED(context->Map(stagingBuffer.Get(), 0, D3D11_MAP_READ, 0, &mapped)),
            "Light staging buffer mapping failed");
    lrender::LightConstants uploaded{};
    std::memcpy(&uploaded, mapped.pData, sizeof(uploaded));
    context->Unmap(stagingBuffer.Get(), 0);
    Require(uploaded.pointLightCount == 3, "Disabled point lights should not occupy GPU light slots");

    lrender::LightManager::Shutdown();
}

} // namespace

int main()
{
    try
    {
        TestLightLifecycleAndBuffer();
        std::cout << "LRenderLightManagerTests: all tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        lrender::LightManager::Shutdown();
        std::cerr << "LRenderLightManagerTests failed: " << error.what() << '\n';
        return 1;
    }
}
