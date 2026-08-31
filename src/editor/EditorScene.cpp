/**
 * @file Native scene file dialogs, save/load, and unsaved-change workflow.
 * @author Codex
 * @created 2026-08-26
 * @depends editor/EditorLayer.h, persistence/SceneSerializer.h, Win32 common dialogs
 */
#include "editor/EditorLayer.h"

#include "persistence/SceneSerializer.h"
#include "render/Dx11Renderer.h"
#include "utils/Logger.h"

#include <array>
#include <commdlg.h>
#include <cwchar>
#include <format>
#include <stdexcept>
#include <string>
#include <utility>

namespace lrender {
namespace {

constexpr int kNoSceneAction = 0;
constexpr int kNewSceneAction = 1;
constexpr int kOpenSceneAction = 2;
constexpr int kExitAction = 3;

std::string PathUtf8(const std::filesystem::path& path) {
    const auto text = path.u8string();
    return {reinterpret_cast<const char*>(text.data()), text.size()};
}

std::filesystem::path SelectSceneFile(
    HWND owner, bool save, const std::filesystem::path& currentPath) {
    std::array<wchar_t, 32768> pathBuffer{};
    if (save && !currentPath.empty()) {
        const std::wstring current = currentPath.wstring();
        wcsncpy_s(pathBuffer.data(), pathBuffer.size(), current.c_str(), _TRUNCATE);
    }
    const std::filesystem::path initialDirectory = currentPath.empty()
        ? std::filesystem::current_path()
        : currentPath.parent_path();

    OPENFILENAMEW dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = owner;
    dialog.lpstrFilter = L"LRender scenes (*.lscene)\0*.lscene\0All files (*.*)\0*.*\0";
    dialog.lpstrFile = pathBuffer.data();
    dialog.nMaxFile = static_cast<DWORD>(pathBuffer.size());
    const std::wstring initialDirectoryText = initialDirectory.wstring();
    dialog.lpstrInitialDir = initialDirectoryText.c_str();
    dialog.lpstrDefExt = L"lscene";
    dialog.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR |
                   (save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);

    const BOOL result = save ? GetSaveFileNameW(&dialog) : GetOpenFileNameW(&dialog);
    if (result != FALSE) {
        return pathBuffer.data();
    }
    const DWORD error = CommDlgExtendedError();
    if (error != 0) {
        throw std::runtime_error(
            "Native scene file dialog failed with code " + std::to_string(error));
    }
    return {};
}

void PreloadSceneResources(const Scene& scene, Dx11Renderer& renderer) {
    for (const Model& model : scene.Models()) {
        for (const Entity& entity : model.entities) {
            if (const MeshGeometry* mesh = entity.Mesh()) {
                const auto asset = renderer.PreloadModel(mesh->assetPath);
                if (mesh->assetEntityIndex >= asset->entities.size()) {
                    throw std::runtime_error(
                        "Scene mesh entity index is outside its referenced asset: " +
                        PathUtf8(mesh->assetPath));
                }
            }
            if (!entity.material.useSourceTexture &&
                !entity.material.baseColorTexturePath.empty()) {
                renderer.PreloadTexture(entity.material.baseColorTexturePath);
            }
        }
    }
}

} // namespace

void EditorLayer::DrawSceneFileMenu(
    Scene& scene, CommandHistory& history, Dx11Renderer& renderer) {
    if (m_exitRequested) {
        m_exitRequested = false;
        QueueSceneAction(kExitAction, scene, history, renderer);
    }

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New", "Ctrl+N")) {
            QueueSceneAction(kNewSceneAction, scene, history, renderer);
        }
        if (ImGui::MenuItem("Open...", "Ctrl+O")) {
            try {
                const auto path = SelectSceneFile(
                    renderer.WindowHandle(), false, m_currentScenePath);
                if (!path.empty()) {
                    QueueSceneAction(kOpenSceneAction, scene, history, renderer, path);
                }
            } catch (const std::exception& error) {
                m_sceneFileError = error.what();
                m_openSceneFileErrorPopup = true;
            }
        }
        if (ImGui::MenuItem("Save", "Ctrl+S")) {
            SaveScene(scene, history, renderer, false);
        }
        if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
            SaveScene(scene, history, renderer, true);
        }
        ImGui::EndMenu();
    }

    const ImGuiIO& input = ImGui::GetIO();
    if (input.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N, false)) {
        QueueSceneAction(kNewSceneAction, scene, history, renderer);
    }
    if (input.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O, false)) {
        try {
            const auto path = SelectSceneFile(renderer.WindowHandle(), false, m_currentScenePath);
            if (!path.empty()) {
                QueueSceneAction(kOpenSceneAction, scene, history, renderer, path);
            }
        } catch (const std::exception& error) {
            m_sceneFileError = error.what();
            m_openSceneFileErrorPopup = true;
        }
    }
    if (input.KeyCtrl && input.KeyShift && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
        SaveScene(scene, history, renderer, true);
    } else if (input.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
        SaveScene(scene, history, renderer, false);
    }
}

bool EditorLayer::SaveScene(
    const Scene& scene, CommandHistory& history, Dx11Renderer& renderer, bool saveAs) {
    try {
        std::filesystem::path path = m_currentScenePath;
        if (saveAs || path.empty()) {
            path = SelectSceneFile(renderer.WindowHandle(), true, m_currentScenePath);
            if (path.empty()) {
                return false;
            }
        }
        SceneSerializer::Save(scene, path);
        m_currentScenePath = std::filesystem::absolute(path).lexically_normal();
        history.MarkSaved();
        Logger::Instance().Info("scene", "Saved scene: " + PathUtf8(m_currentScenePath));
        return true;
    } catch (const std::exception& error) {
        m_sceneFileError = error.what();
        m_openSceneFileErrorPopup = true;
        try {
            Logger::Instance().Error("scene", std::format("Scene save failed: {}", error.what()));
        } catch (const std::exception& logError) {
            m_sceneFileError += std::format("\nLogging also failed: {}", logError.what());
        }
        return false;
    }
}

void EditorLayer::OpenScene(
    Scene& scene, CommandHistory& history, Dx11Renderer& renderer,
    const std::filesystem::path& path) {
    Scene loaded = SceneSerializer::Load(path);
    PreloadSceneResources(loaded, renderer);
    scene = std::move(loaded);
    renderer.ClearRuntimeCaches();
    history.Clear();
    m_currentScenePath = std::filesystem::absolute(path).lexically_normal();
    m_selectedEntityId = 0;
    m_selectedModelId = 0;
    Logger::Instance().Info("scene", "Opened scene: " + PathUtf8(m_currentScenePath));
}

void EditorLayer::QueueSceneAction(
    int action, Scene& scene, CommandHistory& history, Dx11Renderer& renderer,
    std::filesystem::path path) {
    m_pendingSceneAction = action;
    m_pendingScenePath = std::move(path);
    if (history.IsModified()) {
        m_openUnsavedPopup = true;
    } else {
        ExecuteSceneAction(scene, history, renderer);
    }
}

void EditorLayer::ExecuteSceneAction(
    Scene& scene, CommandHistory& history, Dx11Renderer& renderer) {
    const int action = m_pendingSceneAction;
    const std::filesystem::path path = std::move(m_pendingScenePath);
    m_pendingSceneAction = kNoSceneAction;
    m_pendingScenePath.clear();
    try {
        if (action == kNewSceneAction) {
            scene = Scene{};
            history.Clear();
            renderer.ClearRuntimeCaches();
            m_currentScenePath.clear();
            m_selectedEntityId = 0;
            m_selectedModelId = 0;
        } else if (action == kOpenSceneAction) {
            OpenScene(scene, history, renderer, path);
        } else if (action == kExitAction) {
            m_exitConfirmed = true;
        }
    } catch (const std::exception& error) {
        m_sceneFileError = error.what();
        m_openSceneFileErrorPopup = true;
        try {
            Logger::Instance().Error("scene", std::format("Scene action failed: {}", error.what()));
        } catch (const std::exception& logError) {
            m_sceneFileError += std::format("\nLogging also failed: {}", logError.what());
        }
    }
}

void EditorLayer::DrawSceneFilePopups(
    Scene& scene, CommandHistory& history, Dx11Renderer& renderer) {
    if (m_openUnsavedPopup) {
        ImGui::OpenPopup("Unsaved changes");
        m_openUnsavedPopup = false;
    }
    if (ImGui::BeginPopupModal("Unsaved changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Save changes before continuing?");
        if (ImGui::Button("Save")) {
            if (SaveScene(scene, history, renderer, false)) {
                ImGui::CloseCurrentPopup();
                ExecuteSceneAction(scene, history, renderer);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Don't save")) {
            ImGui::CloseCurrentPopup();
            ExecuteSceneAction(scene, history, renderer);
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            m_pendingSceneAction = kNoSceneAction;
            m_pendingScenePath.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (m_openSceneFileErrorPopup) {
        ImGui::OpenPopup("Scene file error");
        m_openSceneFileErrorPopup = false;
    }
    if (ImGui::BeginPopupModal("Scene file error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("%s", m_sceneFileError.c_str());
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

} // namespace lrender
