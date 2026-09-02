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

namespace lrender
{

class Window final
{
  public:
    Window() = default;
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void Create(HINSTANCE instance, std::wstring_view title, std::uint32_t width, std::uint32_t height);
    [[nodiscard]] bool PumpMessages();
    void Close();
    [[nodiscard]] bool CloseRequested() const noexcept;
    void ClearCloseRequest() noexcept;
    [[nodiscard]] HWND Handle() const noexcept;
    [[nodiscard]] std::uint32_t ClientWidth() const noexcept;
    [[nodiscard]] std::uint32_t ClientHeight() const noexcept;

  private:
    static LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

    HINSTANCE m_instance = nullptr;
    HWND m_handle = nullptr;
    std::uint32_t m_clientWidth = 1;
    std::uint32_t m_clientHeight = 1;
    bool m_closeRequested = false;
    static constexpr wchar_t m_windowClassName[] = L"LRenderDemoWindow";
};

} // namespace lrender
