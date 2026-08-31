/**
 * @file Native model import and editable resource/lighting panels.
 * @author Codex
 * @created 2026-08-21
 * @depends editor/EditorLayer.h, render/Dx11Renderer.h, Win32 common dialogs
 */
#include "editor/EditorLayer.h"

#include "commands/CreateModelCommand.h"
#include "render/Dx11Renderer.h"
#include "utils/Logger.h"

#include <array>
#include <commdlg.h>
#include <filesystem>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>

namespace lrender {
namespace {

std::string PathUtf8(const std::filesystem::path& path) {
    const auto text = path.u8string();
    return {reinterpret_cast<const char*>(text.data()), text.size()};
}

std::filesystem::path SelectModelFile(HWND owner) {
    std::array<wchar_t, 32768> pathBuffer{};
    const std::filesystem::path assetDirectory =
        std::filesystem::current_path() / "assets" / "test-scenes" / "downloads";
    const std::wstring initialDirectory = std::filesystem::is_directory(assetDirectory)
        ? assetDirectory.wstring() : std::filesystem::current_path().wstring();

    OPENFILENAMEW dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = owner;
    dialog.lpstrFilter =
        L"Mesh assets (*.gltf;*.glb;*.obj)\0*.gltf;*.glb;*.obj\0"
        L"glTF models (*.gltf;*.glb)\0*.gltf;*.glb\0"
        L"Wavefront OBJ (*.obj)\0*.obj\0All files (*.*)\0*.*\0";
    dialog.lpstrFile = pathBuffer.data();
    dialog.nMaxFile = static_cast<DWORD>(pathBuffer.size());
    dialog.lpstrInitialDir = initialDirectory.c_str();
    dialog.lpstrDefExt = L"gltf";
    dialog.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameW(&dialog) != FALSE) {
        return pathBuffer.data();
    }
    const DWORD error = CommDlgExtendedError();
    if (error != 0) {
        throw std::runtime_error(
            "Native model file dialog failed with code " + std::to_string(error));
    }
    return {};
}

} // namespace

void EditorLayer::ImportModel(
    Scene& scene, CommandHistory& history, Dx11Renderer& renderer) {
    try {
        const std::filesystem::path path = SelectModelFile(renderer.WindowHandle());
        if (path.empty()) {
            return;
        }
        const auto asset = renderer.PreloadModel(path);
        std::vector<std::string> entityNames;
        entityNames.reserve(asset->entities.size());
        for (const MeshAssetEntity& entity : asset->entities) {
            entityNames.push_back(entity.name);
        }
        for (const std::string& warning : asset->warnings) {
            Logger::Instance().Info("assets", "Mesh import warning: " + warning);
        }
        Model& model = scene.CreateMeshModel(path, PathUtf8(path.stem()), entityNames);
        m_selectedModelId = model.id;
        m_selectedEntityId = model.entities.front().id;
        history.PushApplied(std::make_unique<CreateModelCommand>(scene, model));
    } catch (const std::exception& error) {
        m_importError = error.what();
        m_openImportErrorPopup = true;
        try {
            Logger::Instance().Error(
                "assets", std::format("Model import failed: {}", error.what()));
        } catch (const std::exception& logError) {
            m_importError += std::format("\nLogging also failed: {}", logError.what());
        }
    }
}

void EditorLayer::DrawLighting(Dx11Renderer& renderer) {
    ImGui::Begin("Lighting");
    LightingSettings& settings = renderer.Effect().Lights();
    ImGui::ColorEdit3("Ambient", &settings.ambient.x);

    if (ImGui::CollapsingHeader("Directional light", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Enabled##directional", &settings.directional.enabled);
        ImGui::DragFloat3("Direction", &settings.directional.direction.x, 0.01F, -1.0F, 1.0F);
        ImGui::ColorEdit3("Color##directional", &settings.directional.color.x);
        ImGui::DragFloat(
            "Intensity##directional", &settings.directional.intensity, 0.02F, 0.0F, 20.0F);
    }

    for (std::size_t index = 0; index < settings.points.size(); ++index) {
        ImGui::PushID(static_cast<int>(index));
        const std::string label = "Point light " + std::to_string(index + 1);
        if (ImGui::CollapsingHeader(label.c_str())) {
            PointLight& light = settings.points[index];
            ImGui::Checkbox("Enabled", &light.enabled);
            ImGui::DragFloat3("Position", &light.position.x, 0.05F);
            ImGui::ColorEdit3("Color", &light.color.x);
            ImGui::DragFloat("Intensity", &light.intensity, 0.02F, 0.0F, 50.0F);
            ImGui::DragFloat("Range", &light.range, 0.05F, 0.01F, 1000.0F);
        }
        ImGui::PopID();
    }
    ImGui::End();
}

void EditorLayer::DrawResources(Dx11Renderer& renderer) {
    ImGui::Begin("Resources");
    ImGui::Text("Cached mesh assets: %zu", renderer.CachedMeshAssetCount());
    ImGui::Text("Cached textures: %zu", renderer.CachedTextureCount());
    ImGui::TextDisabled("Create > Import Mesh...");
    ImGui::End();
}

} // namespace lrender
