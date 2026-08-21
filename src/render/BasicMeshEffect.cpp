/**
 * @file Editable HLSL basic mesh effect implementation.
 * @author Codex
 * @created 2026-08-20
 * @depends render/BasicMeshEffect.h
 */
#include "render/BasicMeshEffect.h"

#include <d3dcompiler.h>
#include <iterator>
#include <stdexcept>
#include <string>

namespace lrender {
namespace {

Microsoft::WRL::ComPtr<ID3DBlob> LoadShader(const std::filesystem::path& path) {
    Microsoft::WRL::ComPtr<ID3DBlob> shader;
    const HRESULT result = D3DReadFileToBlob(path.c_str(), shader.GetAddressOf());
    if (FAILED(result)) {
        throw std::runtime_error("Failed to load compiled shader: " + path.string());
    }
    return shader;
}

void ThrowIfFailed(HRESULT result, const char* message) {
    if (FAILED(result)) {
        throw std::runtime_error(message);
    }
}

} // namespace

BasicMeshEffect::BasicMeshEffect(
    ID3D11Device* device, const std::filesystem::path& shaderDirectory) {
    if (device == nullptr) {
        throw std::invalid_argument("BasicMeshEffect requires a D3D11 device");
    }

    states_ = std::make_unique<DirectX::CommonStates>(device);
    const auto vertexShader = LoadShader(shaderDirectory / L"BasicMeshVS.cso");
    const auto pixelShader = LoadShader(shaderDirectory / L"BasicMeshPS.cso");

    ThrowIfFailed(
        device->CreateVertexShader(
            vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), nullptr,
            vertexShader_.GetAddressOf()),
        "Failed to create basic mesh vertex shader");
    ThrowIfFailed(
        device->CreatePixelShader(
            pixelShader->GetBufferPointer(), pixelShader->GetBufferSize(), nullptr,
            pixelShader_.GetAddressOf()),
        "Failed to create basic mesh pixel shader");
    constexpr D3D11_INPUT_ELEMENT_DESC inputElements[]{
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
         D3D11_INPUT_PER_VERTEX_DATA, 0}};
    ThrowIfFailed(device->CreateInputLayout(
        inputElements,
        static_cast<UINT>(std::size(inputElements)),
        vertexShader->GetBufferPointer(),
        vertexShader->GetBufferSize(),
        inputLayout_.GetAddressOf()),
        "Failed to create basic mesh input layout");

    static_assert(sizeof(Constants) % 16 == 0);
    D3D11_BUFFER_DESC bufferDescription{};
    bufferDescription.ByteWidth = sizeof(Constants);
    bufferDescription.Usage = D3D11_USAGE_DEFAULT;
    bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    ThrowIfFailed(
        device->CreateBuffer(&bufferDescription, nullptr, constantBuffer_.GetAddressOf()),
        "Failed to create basic mesh constant buffer");
}

void BasicMeshEffect::Bind(
    ID3D11DeviceContext* context,
    const DirectX::SimpleMath::Matrix& world,
    const DirectX::SimpleMath::Matrix& view,
    const DirectX::SimpleMath::Matrix& projection,
    const DirectX::SimpleMath::Vector3& cameraPosition,
    const Material& material,
    const DirectX::SimpleMath::Color& tint,
    bool isSelected) {
    if (context == nullptr) {
        throw std::invalid_argument("BasicMeshEffect requires a D3D11 context");
    }
    if (material.baseColorTexture == nullptr || material.sampler == nullptr) {
        throw std::invalid_argument("BasicMeshEffect requires a texture and sampler material");
    }

    constexpr DirectX::SimpleMath::Color selectionColor{1.0F, 0.84F, 0.0F, 1.0F};
    const DirectX::SimpleMath::Color selectedTint = isSelected
        ? DirectX::SimpleMath::Color::Lerp(tint, selectionColor, 0.28F)
        : tint;
    const DirectX::SimpleMath::Color finalColor{
        selectedTint.x * material.baseColorFactor.x,
        selectedTint.y * material.baseColorFactor.y,
        selectedTint.z * material.baseColorFactor.z,
        selectedTint.w * material.baseColorFactor.w};

    Constants constants{};
    constants.worldViewProjection = world * view * projection;
    constants.world = world;
    constants.worldInverseTranspose = world.Invert().Transpose();
    constants.baseColor = finalColor;
    constants.cameraPosition = {cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0F};
    constants.ambientColor = lights_.ambient;
    auto directionalDirection = lights_.directional.direction;
    if (directionalDirection.LengthSquared() < 0.000001F) {
        directionalDirection = {0.0F, -1.0F, 0.0F};
    } else {
        directionalDirection.Normalize();
    }
    constants.directionalDirectionAndIntensity = {
        directionalDirection.x, directionalDirection.y,
        directionalDirection.z, lights_.directional.intensity};
    constants.directionalColorAndEnabled = {
        lights_.directional.color.x, lights_.directional.color.y,
        lights_.directional.color.z, lights_.directional.enabled ? 1.0F : 0.0F};
    for (std::size_t index = 0; index < lights_.points.size(); ++index) {
        const PointLight& light = lights_.points[index];
        constants.pointLights[index].positionAndRange = {
            light.position.x, light.position.y, light.position.z,
            light.range > 0.0001F ? light.range : 0.0001F};
        constants.pointLights[index].colorAndIntensity = {
            light.color.x, light.color.y, light.color.z,
            light.enabled ? light.intensity : 0.0F};
    }
    constants.materialParameters = {
        material.specularStrength, material.shininess, 0.0F, 0.0F};
    context->UpdateSubresource(constantBuffer_.Get(), 0, nullptr, &constants, 0, 0);

    context->IASetInputLayout(inputLayout_.Get());
    context->RSSetState(isWireframe_ ? states_->Wireframe() : states_->CullNone());
    context->VSSetShader(vertexShader_.Get(), nullptr, 0);
    context->PSSetShader(pixelShader_.Get(), nullptr, 0);
    ID3D11Buffer* constantBuffer = constantBuffer_.Get();
    context->VSSetConstantBuffers(0, 1, &constantBuffer);
    context->PSSetConstantBuffers(0, 1, &constantBuffer);
    ID3D11ShaderResourceView* texture = material.baseColorTexture->ShaderResourceView();
    ID3D11SamplerState* sampler = material.sampler->Get();
    context->PSSetShaderResources(0, 1, &texture);
    context->PSSetSamplers(0, 1, &sampler);
}

} // namespace lrender
