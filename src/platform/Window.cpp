/**
 * @file Native Win32 window implementation and ImGui message forwarding.
 * @author Codex
 * @created 2026-08-20
 * @depends platform/Window.h, ImGui Win32 backend
 */
#include "platform/Window.h"

#include <backends/imgui_impl_win32.h>
#include <imgui.h>
#include <algorithm>
#include <stdexcept>
#include <string>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND window, UINT message, WPARAM wParam, LPARAM lParam);

namespace lrender {

Window::~Window() {
    if (m_handle != nullptr) {
        DestroyWindow(m_handle);
    }
    if (m_instance != nullptr) {
        UnregisterClassW(m_windowClassName, m_instance);
    }
}

void Window::Create(
    HINSTANCE instance, std::wstring_view title, std::uint32_t width, std::uint32_t height) {
    if (instance == nullptr || title.empty() || width == 0 || height == 0) {
        throw std::invalid_argument("Window creation arguments are invalid");
    }
    m_instance = instance;

    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProcedure;
    windowClass.hInstance = m_instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    windowClass.lpszClassName = m_windowClassName;
    if (RegisterClassExW(&windowClass) == 0) {
        throw std::runtime_error("RegisterClassExW failed");
    }

    RECT rectangle{0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    if (!AdjustWindowRect(&rectangle, WS_OVERLAPPEDWINDOW, FALSE)) {
        throw std::runtime_error("AdjustWindowRect failed");
    }
    const std::wstring ownedTitle(title);
    m_handle = CreateWindowExW(
        0,
        m_windowClassName,
        ownedTitle.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rectangle.right - rectangle.left,
        rectangle.bottom - rectangle.top,
        nullptr,
        nullptr,
        m_instance,
        this);
    if (m_handle == nullptr) {
        throw std::runtime_error("CreateWindowExW failed");
    }
    m_clientWidth = width;
    m_clientHeight = height;
    // Use SW_SHOW so automation or IDE startup flags cannot accidentally hide the learning window.
    ShowWindow(m_handle, SW_SHOW);
    UpdateWindow(m_handle);
}

bool Window::PumpMessages() {
    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
        if (message.message == WM_QUIT) {
            return false;
        }
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return true;
}

void Window::Close() {
    if (m_handle != nullptr) {
        DestroyWindow(m_handle);
    }
}

LRESULT CALLBACK Window::WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    Window* self = reinterpret_cast<Window*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        const auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = static_cast<Window*>(create->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->m_handle = window;
    }
    return self != nullptr ? self->HandleMessage(message, wParam, lParam)
                           : DefWindowProcW(window, message, wParam, lParam);
}

LRESULT Window::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_CLOSE) {
        m_closeRequested = true;
        return 0;
    }
    if (message == WM_DESTROY) {
        m_handle = nullptr;
        PostQuitMessage(0);
        return 0;
    }
    if (ImGui::GetCurrentContext() != nullptr &&
        ImGui_ImplWin32_WndProcHandler(m_handle, message, wParam, lParam)) {
        return 1;
    }

    switch (message) {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED) {
            m_clientWidth = std::max<std::uint32_t>(LOWORD(lParam), 1U);
            m_clientHeight = std::max<std::uint32_t>(HIWORD(lParam), 1U);
        }
        return 0;
    case WM_ERASEBKGND:
        return 1;
    default:
        return DefWindowProcW(m_handle, message, wParam, lParam);
    }
}

} // namespace lrender
