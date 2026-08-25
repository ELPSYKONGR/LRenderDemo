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
#include <filesystem>
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
    const HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (SUCCEEDED(comResult)) {
        isComInitialized_ = true;
    } else if (comResult != RPC_E_CHANGED_MODE) {
        throw std::runtime_error("Failed to initialize COM for texture loading");
    }
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
    cube.material.baseColor = {0.25F, 0.55F, 0.92F, 1.0F};
    auto& sphere = scene_.CreateEntity(PrimitiveType::Sphere, "Sphere 2");
    sphere.transform.position.x = 0.8F;
    sphere.material.baseColor = {0.92F, 0.42F, 0.22F, 1.0F};
    auto& plane = scene_.CreateEntity(PrimitiveType::Plane, "Plane 3");
    plane.transform.position.y = -0.5F;
    plane.material.baseColor = {0.55F, 0.58F, 0.62F, 1.0F};

    const std::filesystem::path sampleModel =
        "assets/test-scenes/downloads/suzanne/Suzanne.gltf";
    if (std::filesystem::is_regular_file(sampleModel)) {
        try {
            renderer_.PreloadModel(sampleModel);
            auto& model = scene_.CreateModelEntity(sampleModel, "Suzanne (glTF)");
            model.transform.position.y = 1.8F;
            Logger::Instance().Info("assets", "Loaded optional Suzanne glTF sample");
        } catch (const std::exception& error) {
            Logger::Instance().Error(
                "assets", std::format("Optional Suzanne sample failed: {}", error.what()));
        }
    }

    const std::filesystem::path glbValidationModel =
        "assets/test-scenes/downloads/damaged-helmet/DamagedHelmet.glb";
    if (std::filesystem::is_regular_file(glbValidationModel)) {
        try {
            renderer_.PreloadModel(glbValidationModel);
            Logger::Instance().Info(
                "assets", "Validated optional Damaged Helmet GLB and embedded texture");
        } catch (const std::exception& error) {
            Logger::Instance().Error(
                "assets", std::format("Optional GLB validation failed: {}", error.what()));
        }
    }
}

void Application::Shutdown() noexcept {
    if (isImGuiInitialized_) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        isImGuiInitialized_ = false;
    }
    renderer_.Shutdown();
    if (isComInitialized_) {
        CoUninitialize();
        isComInitialized_ = false;
    }
}

} // namespace lrender
