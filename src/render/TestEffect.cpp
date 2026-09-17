/**
 * @file Test effect skeleton implementation.
 */
#include "render/TestEffect.h"

#include "render/EffectManager.h"

#include <SimpleMath.h>
#include <array>
#include <stdexcept>

namespace lrender
{
namespace
{

struct TestVertex
{
    DirectX::SimpleMath::Vector3 position;
    DirectX::SimpleMath::Color color;
};

Microsoft::WRL::ComPtr<ID3D11Buffer> CreateBuffer(ID3D11Device* device, const void* data,
                                                   std::size_t dataSize, UINT bindFlags)
{
    D3D11_BUFFER_DESC description{};
    description.ByteWidth = static_cast<UINT>(dataSize);
    description.Usage = D3D11_USAGE_DEFAULT;
    description.BindFlags = bindFlags;

    D3D11_SUBRESOURCE_DATA initialData{};
    initialData.pSysMem = data;

    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
    if (FAILED(device->CreateBuffer(&description, &initialData, buffer.GetAddressOf())))
    {
        throw std::runtime_error("Failed to create test geometry buffer");
    }
    return buffer;
}

} // namespace

TestEffect::TestEffect(ID3D11Device* device, ID3D11DeviceContext* context,
                       const std::filesystem::path& shaderDirectory)
    : IRenderEffect(device, context)
{
	const auto vertexShader = LoadShader(shaderDirectory / L"TestEffectVS.cso");
	const auto pixelShader = LoadShader(shaderDirectory / L"TestEffectPS.cso");
	if (FAILED(Device()->CreateVertexShader(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), nullptr,
	                                         m_vertexShader.GetAddressOf())))
	{
		throw std::runtime_error("Failed to create test vertex shader");
	}
	if (FAILED(Device()->CreatePixelShader(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize(), nullptr,
	                                        m_pixelShader.GetAddressOf())))
	{
		throw std::runtime_error("Failed to create test pixel shader");
	}

    constexpr D3D11_INPUT_ELEMENT_DESC inputElements[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    if (FAILED(Device()->CreateInputLayout(inputElements, static_cast<UINT>(std::size(inputElements)),
                                           vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(),
                                           m_inputLayout.GetAddressOf())))
    {
        throw std::runtime_error("Failed to create test input layout");
    }

    // 测试几何使用裁剪空间坐标，便于独立验证输入布局、深度和混合状态。
    constexpr std::array<TestVertex, 3> singleTriangleVertices =
    {
        TestVertex{{-0.65F, -0.55F, 0.20F}, {1.0F, 0.1F, 0.1F, 1.0F}},
        TestVertex{{0.0F, 0.65F, 0.20F}, {0.1F, 1.0F, 0.1F, 1.0F}},
        TestVertex{{0.65F, -0.55F, 0.20F}, {0.1F, 0.3F, 1.0F, 1.0F}}
    };
    constexpr std::array<std::uint32_t, 3> singleTriangleIndices = {0, 1, 2};

    constexpr std::array<TestVertex, 9> transparentVertices =
    {
        TestVertex{{-0.62F, -0.50F, 0.20F}, {1.0F, 0.1F, 0.1F, 0.35F}},
        TestVertex{{0.0F, 0.62F, 0.20F}, {1.0F, 0.1F, 0.1F, 0.35F}},
        TestVertex{{0.62F, -0.50F, 0.20F}, {1.0F, 0.1F, 0.1F, 0.35F}},
        TestVertex{{-0.54F, -0.48F, 0.10F}, {0.1F, 1.0F, 0.1F, 0.35F}},
        TestVertex{{0.08F, 0.64F, 0.10F}, {0.1F, 1.0F, 0.1F, 0.35F}},
        TestVertex{{0.70F, -0.48F, 0.10F}, {0.1F, 1.0F, 0.1F, 0.35F}},
        TestVertex{{-0.62F, -0.56F, 0.0F}, {0.1F, 0.2F, 1.0F, 0.35F}},
        TestVertex{{0.0F, 0.56F, 0.0F}, {0.1F, 0.2F, 1.0F, 0.35F}},
        TestVertex{{0.62F, -0.56F, 0.0F}, {0.1F, 0.2F, 1.0F, 0.35F}}
    };
    constexpr std::array<std::uint32_t, 9> transparentIndices = {0, 1, 2, 3, 4, 5, 6, 7, 8};

    m_singleTriangleVertexBuffer = CreateBuffer(Device(), singleTriangleVertices.data(),
                                                sizeof(singleTriangleVertices), D3D11_BIND_VERTEX_BUFFER);
    m_singleTriangleIndexBuffer = CreateBuffer(Device(), singleTriangleIndices.data(),
                                                sizeof(singleTriangleIndices), D3D11_BIND_INDEX_BUFFER);
    m_transparentVertexBuffer = CreateBuffer(Device(), transparentVertices.data(),
                                              sizeof(transparentVertices), D3D11_BIND_VERTEX_BUFFER);
    m_transparentIndexBuffer = CreateBuffer(Device(), transparentIndices.data(),
                                             sizeof(transparentIndices), D3D11_BIND_INDEX_BUFFER);
    m_singleTriangleIndexCount = static_cast<std::uint32_t>(singleTriangleIndices.size());
    m_transparentIndexCount = static_cast<std::uint32_t>(transparentIndices.size());
}

void TestEffect::SetGeometryMode(TestGeometryMode mode) noexcept
{
    m_geometryMode = mode;
}

TestGeometryMode TestEffect::GeometryMode() const noexcept
{
    return m_geometryMode;
}

void TestEffect::Bind(const EffectFrameContext& frame, const EffectDrawContext&)
{
	ID3D11DeviceContext* context = frame.DeviceContext();
	EffectManager& effectManager = EffectManager::Instance();
	frame.ConstantBuffers().BindFrameBuffer();
	context->IASetInputLayout(m_inputLayout.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	effectManager.SetRasterizerMode(RasterizerMode::SolidCullNone);
	if (m_geometryMode == TestGeometryMode::TransparentTriangles)
	{
		effectManager.SetDepthMode(DepthMode::ReadOnly);
		effectManager.SetBlendMode(BlendMode::AlphaBlend);
	}
	else
	{
		effectManager.SetDepthMode(DepthMode::ReadWrite);
		effectManager.SetBlendMode(BlendMode::Opaque);
	}
}

void TestEffect::Draw(const EffectFrameContext& frame, const EffectDrawContext& draw)
{
	Bind(frame, draw);
	ID3D11DeviceContext* context = frame.DeviceContext();
	const bool transparent = m_geometryMode == TestGeometryMode::TransparentTriangles;
	ID3D11Buffer* vertexBuffer = transparent ? m_transparentVertexBuffer.Get() : m_singleTriangleVertexBuffer.Get();
	ID3D11Buffer* indexBuffer = transparent ? m_transparentIndexBuffer.Get() : m_singleTriangleIndexBuffer.Get();
	const UINT indexCount = transparent ? m_transparentIndexCount : m_singleTriangleIndexCount;
	constexpr UINT stride = sizeof(TestVertex);
	constexpr UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	context->DrawIndexed(indexCount, 0, 0);
	ID3D11Buffer* nullBuffer = nullptr;
	context->IASetVertexBuffers(0, 1, &nullBuffer, &stride, &offset);
	context->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
}

std::string_view TestEffect::Name() const noexcept
{
    return "TestEffect";
}

} // namespace lrender
