/**
 * @file Test effect skeleton implementation.
 */
#include "render/TestEffect.h"

#include <stdexcept>

namespace lrender
{

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
}


void TestEffect::Bind(const EffectFrameContext& frame, const EffectDrawContext& )
{
	ID3D11DeviceContext* context = frame.DeviceContext();
	frame.ConstantBuffers().BindFrameBuffer();
	context->IASetInputLayout(m_inputLayout.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
}

void TestEffect::Draw(const EffectFrameContext& frame, const EffectDrawContext& draw)
{
	Bind(frame, draw);
}

std::string_view TestEffect::Name() const noexcept
{
    return "Test";
}

} // namespace lrender
