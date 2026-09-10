/**
 * @file Renderer-owned common D3D11 constant buffers.
 * @author Codex
 * @created 2026-09-08
 * @depends render/Dx11ConstantBuffer.h
 */
#pragma once

#include "render/Dx11ConstantBuffer.h"

#include <SimpleMath.h>

#include <array>
#include <cstddef>
#include <cstdint>

namespace lrender
{
	struct alignas(16) FrameConstants
	{
		DirectX::SimpleMath::Matrix view;
		DirectX::SimpleMath::Matrix projection;
		DirectX::SimpleMath::Matrix inverseViewProjection;
		DirectX::SimpleMath::Vector4 cameraPosition;   // xyz:相机世界坐标 w:保留
		DirectX::SimpleMath::Vector4 viewport;         // x:宽高比 y:高度缩放 z/w:保留
		DirectX::SimpleMath::Vector4 frameParameters;  // x:渲染模式 yzw:保留
	};

	struct alignas(16) ObjectConstants
	{
		DirectX::SimpleMath::Matrix worldViewProjection;
		DirectX::SimpleMath::Matrix world;
		DirectX::SimpleMath::Matrix worldInverseTranspose;
	};

	struct alignas(16) MaterialConstants
	{
		DirectX::SimpleMath::Vector4 baseColor;           // rgba:基础颜色
		DirectX::SimpleMath::Vector4 specularColor;       // rgb:高光颜色 a:保留
		DirectX::SimpleMath::Vector4 materialParameters;   // x:高光强度 y:光泽度 z:漫反射强度 w:显示模式
	};

	struct alignas(16) LightConstants
	{
		DirectX::SimpleMath::Vector4 ambientColor;                    // rgb:环境光颜色 a:保留
		DirectX::SimpleMath::Vector4 directionalDirectionAndIntensity; // xyz:方向 w:强度
		DirectX::SimpleMath::Vector4 directionalColorAndEnabled;       // rgb:颜色 w:启用标志
		std::array<DirectX::SimpleMath::Vector4, 8> pointLightData;    // 偶数项 xyz:位置 w:范围；奇数项 rgb:颜色 w:强度
		std::uint32_t pointLightCount = 0;                             // 当前有效点光源槽数量
		std::array<std::uint32_t, 3> lightPadding = {};                // 对齐到 16 字节
	};


	static_assert(sizeof(FrameConstants) == 240);
	static_assert(sizeof(ObjectConstants) == 192);
	static_assert(sizeof(MaterialConstants) == 48);
	static_assert(sizeof(LightConstants) == 192);
	static_assert(offsetof(FrameConstants, inverseViewProjection) == 128);
	static_assert(offsetof(FrameConstants, cameraPosition) == 192);
	static_assert(offsetof(ObjectConstants, world) == 64);
	static_assert(offsetof(LightConstants, pointLightData) == 48);
	static_assert(offsetof(LightConstants, pointLightCount) == 176);

	class CommonConstantBuffers final
	{
	  public:
		CommonConstantBuffers(ID3D11Device* device, ID3D11DeviceContext* context);

		CommonConstantBuffers(const CommonConstantBuffers&) = delete;
		CommonConstantBuffers& operator=(const CommonConstantBuffers&) = delete;

		void UpdateFrameBuffer(const FrameConstants& data) const;
		void UpdateObjectBuffer(const ObjectConstants& data) const;
		void UpdateMaterialBuffer(const MaterialConstants& data) const;
		void UpdateLightBuffer(const LightConstants& data) const;

		void BindFrameBuffer() const;
		void BindObjectBuffer() const;
		void BindMaterialBuffer() const;
		void BindLightBuffer() const;

	  private:
		ID3D11DeviceContext* m_context = nullptr;
		Dx11ConstantBuffer<FrameConstants> m_frameBuffer;
		Dx11ConstantBuffer<ObjectConstants> m_objectBuffer;
		Dx11ConstantBuffer<MaterialConstants> m_materialBuffer;
		Dx11ConstantBuffer<LightConstants> m_lightBuffer;
	};

} // namespace lrender
