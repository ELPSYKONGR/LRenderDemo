/**
 * @file Viewport state implementation.
 */
#include "core/ViewPort.h"

#include <algorithm>

namespace lrender
{

ViewPort::ViewPort(std::uint32_t width, std::uint32_t height, std::uint32_t viewportNumber) noexcept
    : m_width(std::max(width, 1U)), m_height(std::max(height, 1U)), m_viewportNumber(viewportNumber)
{
}

std::uint32_t ViewPort::Width() const noexcept
{
    return m_width;
}

std::uint32_t ViewPort::Height() const noexcept
{
    return m_height;
}

std::uint32_t ViewPort::Number() const noexcept
{
    return m_viewportNumber;
}

Camera& ViewPort::GetCamera() noexcept
{
    return m_camera;
}

const Camera& ViewPort::GetCamera() const noexcept
{
    return m_camera;
}

void ViewPort::SetWidth(std::uint32_t width) noexcept
{
    m_width = std::max(width, 1U);
}

void ViewPort::SetHeight(std::uint32_t height) noexcept
{
    m_height = std::max(height, 1U);
}

void ViewPort::SetSize(std::uint32_t width, std::uint32_t height) noexcept
{
    m_width = std::max(width, 1U);
    m_height = std::max(height, 1U);
}

} // namespace lrender
