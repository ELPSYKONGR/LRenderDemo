/**
 * @file Native Win32 window and message-pump wrapper.
 * @author Codex
 * @created 2026-08-20
 * @depends Win32
 */
#pragma once

#include <cstdint>
#include <string_view>
#include <windows.h>

namespace lrender {

class Window final {
public:
    Window() = default;
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void Create(HINSTANCE instance, std::wstring_view title, std::uint32_t width, std::uint32_t height);
    [[nodiscard]] bool PumpMessages();
    [[nodiscard]] HWND Handle() const noexcept { return handle_; }
    [[nodiscard]] std::uint32_t ClientWidth() const noexcept { return clientWidth_; }
    [[nodiscard]] std::uint32_t ClientHeight() const noexcept { return clientHeight_; }

private:
    static LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

    HINSTANCE instance_{};
    HWND handle_{};
    std::uint32_t clientWidth_{1};
    std::uint32_t clientHeight_{1};
    static constexpr wchar_t kWindowClassName[] = L"LRenderDemoWindow";
};

} // namespace lrender
