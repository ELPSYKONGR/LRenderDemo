/**
 * @file Application initialization, frame loop, and orderly shutdown.
 * @author Codex
 * @created 2026-08-20
 * @depends app/Application.h, ImGui Win32/DX11 backends
 */
#include "app/Application.h"

#include "utils/Logger.h"

#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include <imgui.h>

#include <cstdint>
#include <format>
#include <stdexcept>

namespace lrender {

int Application::Run() {
    Initialize();
    Logger::Instance().Info("app", "Application frame loop started");

    while (window_.PumpMessages()) {
        renderer_.ResizeSwapChain(window_.ClientWidth(), window_.ClientHeight());

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        editor_.Draw(scene_, history_, camera_, renderer_);
        renderer_.RenderScene(scene_, camera_, editor_.SelectedEntityId());

        ImGui::Render();
        renderer_.RenderEditor(ImGui::GetDrawData());
        renderer_.Present();
    }

    Shutdown();
    return 0;
}

void Application::Initialize() {
    window_.Create(instance_, L"LRenderDemo - DX11 Render Lab", 1440, 900);
    Logger::Instance().Info(
        "platform",
        std::format(
            "Window created {{handle: {}, visible: {}}}",
            reinterpret_cast<std::uintptr_t>(window_.Handle()),
            IsWindowVisible(window_.Handle()) != FALSE));
    renderer_.Initialize(window_.Handle(), window_.ClientWidth(), window_.ClientHeight());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& input = ImGui::GetIO();
    input.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 3.0F;
    style.FrameRounding = 2.0F;

    if (!ImGui_ImplWin32_Init(window_.Handle()) ||
        !ImGui_ImplDX11_Init(renderer_.Device(), renderer_.Context())) {
        throw std::runtime_error("Failed to initialize Dear ImGui backends");
    }
    isImGuiInitialized_ = true;

    auto& cube = scene_.CreateEntity(PrimitiveType::Cube, "Cube 1");
    cube.transform.position.x = -0.8F;
    cube.color = {0.25F, 0.55F, 0.92F, 1.0F};
    auto& sphere = scene_.CreateEntity(PrimitiveType::Sphere, "Sphere 2");
    sphere.transform.position.x = 0.8F;
    sphere.color = {0.92F, 0.42F, 0.22F, 1.0F};
}

void Application::Shutdown() noexcept {
    if (isImGuiInitialized_) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        isImGuiInitialized_ = false;
    }
    renderer_.Shutdown();
}

} // namespace lrender
