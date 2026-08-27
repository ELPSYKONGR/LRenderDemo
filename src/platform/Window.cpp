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
    if (handle_ != nullptr) {
        DestroyWindow(handle_);
    }
    if (instance_ != nullptr) {
        UnregisterClassW(kWindowClassName, instance_);
    }
}

void Window::Create(
    HINSTANCE instance, std::wstring_view title, std::uint32_t width, std::uint32_t height) {
    if (instance == nullptr || title.empty() || width == 0 || height == 0) {
        throw std::invalid_argument("Window creation arguments are invalid");
    }
    instance_ = instance;

    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProcedure;
    windowClass.hInstance = instance_;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    windowClass.lpszClassName = kWindowClassName;
    if (RegisterClassExW(&windowClass) == 0) {
        throw std::runtime_error("RegisterClassExW failed");
    }

    RECT rectangle{0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    if (!AdjustWindowRect(&rectangle, WS_OVERLAPPEDWINDOW, FALSE)) {
        throw std::runtime_error("AdjustWindowRect failed");
    }
    const std::wstring ownedTitle(title);
    handle_ = CreateWindowExW(
        0,
        kWindowClassName,
        ownedTitle.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rectangle.right - rectangle.left,
        rectangle.bottom - rectangle.top,
        nullptr,
        nullptr,
        instance_,
        this);
    if (handle_ == nullptr) {
        throw std::runtime_error("CreateWindowExW failed");
    }
    clientWidth_ = width;
    clientHeight_ = height;
    // Use SW_SHOW so automation or IDE startup flags cannot accidentally hide the learning window.
    ShowWindow(handle_, SW_SHOW);
    UpdateWindow(handle_);
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
    if (handle_ != nullptr) {
        DestroyWindow(handle_);
    }
}

LRESULT CALLBACK Window::WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    Window* self = reinterpret_cast<Window*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        const auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = static_cast<Window*>(create->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->handle_ = window;
    }
    return self != nullptr ? self->HandleMessage(message, wParam, lParam)
                           : DefWindowProcW(window, message, wParam, lParam);
}

LRESULT Window::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_CLOSE) {
        closeRequested_ = true;
        return 0;
    }
    if (message == WM_DESTROY) {
        handle_ = nullptr;
        PostQuitMessage(0);
        return 0;
    }
    if (ImGui::GetCurrentContext() != nullptr &&
        ImGui_ImplWin32_WndProcHandler(handle_, message, wParam, lParam)) {
        return 1;
    }

    switch (message) {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED) {
            clientWidth_ = std::max<std::uint32_t>(LOWORD(lParam), 1U);
            clientHeight_ = std::max<std::uint32_t>(HIWORD(lParam), 1U);
        }
        return 0;
    case WM_ERASEBKGND:
        return 1;
    default:
        return DefWindowProcW(handle_, message, wParam, lParam);
    }
}

} // namespace lrender
