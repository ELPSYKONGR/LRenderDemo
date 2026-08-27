/**
 * @file Dear ImGui editor shell and scene manipulation workflow.
 * @author Codex
 * @created 2026-08-20
 * @depends core/Scene.h, commands/CommandHistory.h, render/Dx11Renderer.h, ImGuizmo
 */
#pragma once

#include "commands/CommandHistory.h"
#include "core/Camera.h"
#include "core/Scene.h"

#include <imgui.h>
#include <ImGuizmo.h>
#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>

namespace lrender {

class Dx11Renderer;

class EditorLayer final {
public:
    void Draw(Scene& scene, CommandHistory& history, Camera& camera, Dx11Renderer& renderer);
    void RequestExit() noexcept { exitRequested_ = true; }
    [[nodiscard]] bool ConsumeExitConfirmed() noexcept {
        const bool result = exitConfirmed_;
        exitConfirmed_ = false;
        return result;
    }
    [[nodiscard]] std::uint32_t SelectedEntityId() const noexcept { return selectedEntityId_; }

private:
    void DrawMainMenu(Scene& scene, CommandHistory& history, Dx11Renderer& renderer);
    void DrawSceneFileMenu(Scene& scene, CommandHistory& history, Dx11Renderer& renderer);
    void DrawSceneFilePopups(Scene& scene, CommandHistory& history, Dx11Renderer& renderer);
    void DrawToolbar(CommandHistory& history, Dx11Renderer& renderer);
    void DrawCameraControls(Camera& camera);
    void DrawLighting(Dx11Renderer& renderer);
    void DrawResources(Dx11Renderer& renderer);
    void DrawHierarchy(Scene& scene);
    void DrawInspector(Scene& scene, CommandHistory& history, Dx11Renderer& renderer);
    void DrawSolidGeometryEditor(Scene& scene, CommandHistory& history, Entity& entity);
    void DrawSolidCreationPopup(Scene& scene, CommandHistory& history);
    void DrawMaterialEditor(
        Scene& scene, CommandHistory& history, Dx11Renderer& renderer, Entity& entity);
    void DrawViewport(Scene& scene, CommandHistory& history, Camera& camera, Dx11Renderer& renderer);
    void BeginSolidCreation(PrimitiveType primitive, const Scene& scene);
    void CreateSolid(
        Scene& scene, CommandHistory& history,
        SolidGeometry geometry, std::string name);
    void ImportModel(Scene& scene, CommandHistory& history, Dx11Renderer& renderer);
    void TrackPropertyEdit(
        Scene& scene, CommandHistory& history, Entity& entity, const Transform& beforeControl);
    void TrackMaterialEdit(
        Scene& scene, CommandHistory& history, Entity& entity,
        const EntityMaterial& beforeControl);
    void TrackSolidEdit(
        Scene& scene, CommandHistory& history, Entity& entity,
        const SolidGeometry& beforeControl);
    bool SaveScene(
        const Scene& scene, CommandHistory& history, Dx11Renderer& renderer, bool saveAs);
    void OpenScene(
        Scene& scene, CommandHistory& history, Dx11Renderer& renderer,
        const std::filesystem::path& path);
    void QueueSceneAction(
        int action, Scene& scene, CommandHistory& history, Dx11Renderer& renderer,
        std::filesystem::path path = {});
    void ExecuteSceneAction(
        Scene& scene, CommandHistory& history, Dx11Renderer& renderer);
    void ValidateSelection(const Scene& scene);

    std::uint32_t selectedEntityId_{};
    ModelId selectedModelId_{};
    ImGuizmo::OPERATION gizmoOperation_{ImGuizmo::TRANSLATE};
    bool wasUsingGizmo_{false};
    Transform gizmoStart_;
    std::optional<Transform> propertyEditStart_;
    std::uint32_t propertyEditEntityId_{};
    std::optional<EntityMaterial> materialEditStart_;
    std::uint32_t materialEditEntityId_{};
    std::string importError_;
    bool openImportErrorPopup_{false};
    std::string materialError_;
    bool openMaterialErrorPopup_{false};
    std::optional<SolidGeometry> solidEditStart_;
    std::uint32_t solidEditEntityId_{};
    PrimitiveType pendingSolidType_{PrimitiveType::Cube};
    bool openSolidCreatePopup_{false};
    std::array<char, 128> solidCreateName_{};
    CubeParameters cubeCreateParameters_;
    SphereParameters sphereCreateParameters_;
    PlaneParameters planeCreateParameters_;
    std::string solidCreateError_;
    std::filesystem::path currentScenePath_;
    std::filesystem::path pendingScenePath_;
    int pendingSceneAction_{};
    bool openUnsavedPopup_{false};
    bool exitRequested_{false};
    bool exitConfirmed_{false};
    std::string sceneFileError_;
    bool openSceneFileErrorPopup_{false};
    bool autoRotate_{false};
    bool autoRotateClockwise_{false};
    float autoRotateSpeedDegrees_{20.0F};
};

} // namespace lrender
