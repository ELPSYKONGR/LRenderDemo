/**
 * @file Renderer-owned common D3D11 constant buffers.
 * @author Codex
 * @created 2026-09-08
 * @depends render/CommonConstants.h, render/Dx11ConstantBuffer.h
 */
#pragma once

#include "render/CommonConstants.h"
#include "render/Dx11ConstantBuffer.h"

namespace lrender
{

class CommonConstantBuffers final
{
  public:
    CommonConstantBuffers(ID3D11Device* device, ID3D11DeviceContext* context);

    CommonConstantBuffers(const CommonConstantBuffers&) = delete;
    CommonConstantBuffers& operator=(const CommonConstantBuffers&) = delete;

    void UpdateFrame(const FrameConstants& data) const;
    void UpdateObject(const ObjectConstants& data) const;
    void UpdateMaterial(const MaterialConstants& data) const;
    void UpdateLight(const LightConstants& data) const;

    void BindFrame() const;
    void BindObject() const;
    void BindMaterial() const;
    void BindLight() const;

  private:
    ID3D11DeviceContext* m_context = nullptr;
    Dx11ConstantBuffer<FrameConstants> m_frameBuffer;
    Dx11ConstantBuffer<ObjectConstants> m_objectBuffer;
    Dx11ConstantBuffer<MaterialConstants> m_materialBuffer;
    Dx11ConstantBuffer<LightConstants> m_lightBuffer;
};

} // namespace lrender
