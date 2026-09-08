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

void CommonConstantBuffers::UpdateFrame(const FrameConstants& data) const
{
    m_frameBuffer.Update(m_context, data);
}

void CommonConstantBuffers::UpdateObject(const ObjectConstants& data) const
{
    m_objectBuffer.Update(m_context, data);
}

void CommonConstantBuffers::UpdateMaterial(const MaterialConstants& data) const
{
    m_materialBuffer.Update(m_context, data);
}

void CommonConstantBuffers::UpdateLight(const LightConstants& data) const
{
    m_lightBuffer.Update(m_context, data);
}

void CommonConstantBuffers::BindFrame() const
{
    m_frameBuffer.BindVS(m_context, 0);
    m_frameBuffer.BindPS(m_context, 0);
}

void CommonConstantBuffers::BindObject() const
{
    m_objectBuffer.BindVS(m_context, 1);
    m_objectBuffer.BindPS(m_context, 1);
}

void CommonConstantBuffers::BindMaterial() const
{
    m_materialBuffer.BindPS(m_context, 2);
}

void CommonConstantBuffers::BindLight() const
{
    m_lightBuffer.BindPS(m_context, 3);
}

} // namespace lrender
