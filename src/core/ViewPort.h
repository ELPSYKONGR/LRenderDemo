/**
 * @file API-independent viewport state used by the editor and renderer.
 */
#pragma once

#include "core/Camera.h"

#include <cstdint>

namespace lrender {

class ViewPort final {
public:
    explicit ViewPort(
        std::uint32_t width = 1, std::uint32_t height = 1,
        std::uint32_t viewportNumber = 0) noexcept;

    [[nodiscard]] std::uint32_t Width() const noexcept { return m_width; }
    [[nodiscard]] std::uint32_t Height() const noexcept { return m_height; }
    [[nodiscard]] std::uint32_t Number() const noexcept { return m_viewportNumber; }
    [[nodiscard]] Camera& GetCamera() noexcept { return m_camera; }
    [[nodiscard]] const Camera& GetCamera() const noexcept { return m_camera; }

    void SetWidth(std::uint32_t width) noexcept;
    void SetHeight(std::uint32_t height) noexcept;
    void SetSize(std::uint32_t width, std::uint32_t height) noexcept;

private:
    std::uint32_t m_width;
    std::uint32_t m_height;
    std::uint32_t m_viewportNumber;
    Camera m_camera;
};

} // namespace lrender
