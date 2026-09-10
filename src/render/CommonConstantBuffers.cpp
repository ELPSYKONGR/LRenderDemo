/**
 * @file Renderer-owned common D3D11 constant buffers implementation.
 * @author Codex
 * @created 2026-09-08
 * @depends render/CommonConstantBuffers.h
 */
#include "render/CommonConstantBuffers.h"

#include <stdexcept>

namespace lrender
{

CommonConstantBuffers::CommonConstantBuffers(ID3D11Device* device, ID3D11DeviceContext* context)
    : m_context(context), m_frameBuffer(device), m_objectBuffer(device), m_materialBuffer(device), m_lightBuffer(device)
{
    if (m_context == nullptr)
    {
        throw std::invalid_argument("CommonConstantBuffers requires a D3D11 context");
    }
}

void CommonConstantBuffers::UpdateFrameBuffer(const FrameConstants& data) const
{
    m_frameBuffer.Update(m_context, data);
}

void CommonConstantBuffers::UpdateObjectBuffer(const ObjectConstants& data) const
{
    m_objectBuffer.Update(m_context, data);
}

void CommonConstantBuffers::UpdateMaterialBuffer(const MaterialConstants& data) const
{
    m_materialBuffer.Update(m_context, data);
}

void CommonConstantBuffers::UpdateLightBuffer(const LightConstants& data) const
{
    m_lightBuffer.Update(m_context, data);
}

void CommonConstantBuffers::BindFrameBuffer() const
{
    m_frameBuffer.BindVS(m_context, 0);
    m_frameBuffer.BindPS(m_context, 0);
}

void CommonConstantBuffers::BindObjectBuffer() const
{
    m_objectBuffer.BindVS(m_context, 1);
    m_objectBuffer.BindPS(m_context, 1);
}

void CommonConstantBuffers::BindMaterialBuffer() const
{
    m_materialBuffer.BindPS(m_context, 2);
}

void CommonConstantBuffers::BindLightBuffer() const
{
    m_lightBuffer.BindPS(m_context, 3);
}

} // namespace lrender
