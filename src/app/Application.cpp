/**
 * @file Application initialization, frame loop, and orderly shutdown.
 * @author Codex
 * @created 2026-08-20
 * @depends app/Application.h, ImGui Win32/DX11 backends
 */
#include "app/Application.h"

#include "platform/RuntimePaths.h"
#include "render/ViewManager.h"
#include "utils/Logger.h"

#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>
#include <imgui.h>

#include <cstdint>
#include <filesystem>
#include <format>
#include <stdexcept>

namespace lrender
{
namespace
{

std::string ImGuiIniPath()
{
    return (RuntimePaths::ExecutableDirectory() / "imgui.ini").string();
}

} // namespace

Application::Application(HINSTANCE instance) : m_instance(instance)
{
}

Application::~Application()
{
    Shutdown();
}

int Application::Run()
{
    Initialize();
    Logger::Instance().Info("app", "Application frame loop started");

    while (m_window.PumpMessages())
    {
        if (m_window.CloseRequested())
        {
            m_window.ClearCloseRequest();
            m_editor.RequestExit();
        }
        m_renderer.ResizeSwapChain(m_window.ClientWidth(), m_window.ClientHeight());

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        m_editor.Draw(m_scene, m_history, m_camera, m_renderer);
        if (m_editor.ConsumeExitConfirmed())
        {
            m_window.Close();
            continue;
        }
        m_renderer.RenderScene(m_scene, m_camera, m_editor.SelectedEntityId());

        ImGui::Render();
        m_renderer.RenderEditor(ImGui::GetDrawData());
        m_renderer.Present();
    }

    Shutdown();
    return 0;
}

void Application::Initialize()
{
    const HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (SUCCEEDED(comResult))
    {
        m_isComInitialized = true;
    }
    else if (comResult != RPC_E_CHANGED_MODE)
    {
        throw std::runtime_error("Failed to initialize COM for texture loading");
    }
    m_window.Create(m_instance, L"LRenderDemo - DX11 Render Lab", 1440, 900);
    Logger::Instance().Info("platform", std::format("Window created {{handle: {}, visible: {}}}",
                                                    reinterpret_cast<std::uintptr_t>(m_window.Handle()),
                                                    IsWindowVisible(m_window.Handle()) != FALSE));
    m_renderer.Initialize(m_window.Handle(), m_window.ClientWidth(), m_window.ClientHeight());

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& input = ImGui::GetIO();
    static const std::string imguiIniPath = ImGuiIniPath();
    input.IniFilename = imguiIniPath.c_str();
    input.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 3.0F;
    style.FrameRounding = 2.0F;

    if (!ImGui_ImplWin32_Init(m_window.Handle()) || !ImGui_ImplDX11_Init(m_renderer.Device(), m_renderer.Context()))
    {
        throw std::runtime_error("Failed to initialize Dear ImGui backends");
    }
    m_isImGuiInitialized = true;

    Model& defaultModel = m_scene.CreateModel("Default Model");
    const ModelId defaultModelId = defaultModel.id;
    ViewManager::Instance().CreateSphereTestEntity(m_scene, defaultModelId);

    //auto& cube = m_scene.CreateEntity(defaultModelId, PrimitiveType::Cube, "Cube 1");
    //cube.transform.position.x = -0.8F;
    //cube.EntityMaterialData().baseColor = {0.25F, 0.55F, 0.92F, 1.0F};
    //auto& sphere = m_scene.CreateEntity(defaultModelId, PrimitiveType::Sphere, "Sphere 2");
    //sphere.transform.position.x = 0.8F;
    //sphere.EntityMaterialData().baseColor = {0.92F, 0.42F, 0.22F, 1.0F};
    //auto& plane = m_scene.CreateEntity(defaultModelId, PrimitiveType::Plane, "Plane 3");
    //plane.transform.position.y = -0.5F;
    //plane.EntityMaterialData().baseColor = {0.55F, 0.58F, 0.62F, 1.0F};

    //const std::filesystem::path sampleModel = "assets/test-scenes/downloads/suzanne/Suzanne.gltf";
    //"assets/test-scenes/downloads/sponza/Sponza.gltf";
    //if (std::filesystem::is_regular_file(sampleModel))
    //{
    //    try
    //    {
    //        const auto asset = m_renderer.PreloadModel(sampleModel);
    //        std::vector<std::string> entityNames;
    //        for (const MeshAssetEntity& entity : asset->entities)
    //        {
    //            entityNames.push_back(entity.name);
    //        }
    //        auto& model = m_scene.CreateMeshModel(sampleModel, "Suzanne (glTF)", entityNames);
    //        for (Entity& entity : model.entities)
    //        {
    //            entity.transform.position.y = 1.8F;
    //        }
    //        Logger::Instance().Info("assets", "Loaded optional Suzanne glTF sample");
    //    }
    //    catch (const std::exception& error)
    //    {
    //        Logger::Instance().Error("assets", std::format("Optional Suzanne sample failed: {}", error.what()));
    //    }
    //}

    //const std::filesystem::path glbValidationModel = "assets/test-scenes/downloads/damaged-helmet/DamagedHelmet.glb";
    //if (std::filesystem::is_regular_file(glbValidationModel))
    //{
    //    try
    //    {
    //        static_cast<void>(m_renderer.PreloadModel(glbValidationModel));
    //        Logger::Instance().Info("assets", "Validated optional Damaged Helmet GLB and embedded texture");
    //    }
    //    catch (const std::exception& error)
    //    {
    //        Logger::Instance().Error("assets", std::format("Optional GLB validation failed: {}", error.what()));
    //    }
    //}
}

void Application::Shutdown() noexcept
{
    if (m_isImGuiInitialized)
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        m_isImGuiInitialized = false;
    }
    m_renderer.Shutdown();
    if (m_isComInitialized)
    {
        CoUninitialize();
        m_isComInitialized = false;
    }
}

} // namespace lrender
