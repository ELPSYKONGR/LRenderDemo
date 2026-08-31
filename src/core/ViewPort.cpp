/**
 * @file Viewport state implementation.
 */
#include "core/ViewPort.h"

#include <algorithm>

namespace lrender {

ViewPort::ViewPort(
    std::uint32_t width, std::uint32_t height, std::uint32_t viewportNumber) noexcept
    : m_width(std::max(width, 1U)),
      m_height(std::max(height, 1U)),
      m_viewportNumber(viewportNumber) {}

void ViewPort::SetWidth(std::uint32_t width) noexcept {
    m_width = std::max(width, 1U);
}

void ViewPort::SetHeight(std::uint32_t height) noexcept {
    m_height = std::max(height, 1U);
}

void ViewPort::SetSize(std::uint32_t width, std::uint32_t height) noexcept {
    m_width = std::max(width, 1U);
    m_height = std::max(height, 1U);
}

} // namespace lrender
