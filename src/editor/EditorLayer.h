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
#include <cstdint>
#include <optional>
#include <string>

namespace lrender {

class Dx11Renderer;

class EditorLayer final {
public:
    void Draw(Scene& scene, CommandHistory& history, Camera& camera, Dx11Renderer& renderer);
    [[nodiscard]] std::uint32_t SelectedEntityId() const noexcept { return selectedEntityId_; }

private:
    void DrawMainMenu(Scene& scene, CommandHistory& history, Dx11Renderer& renderer);
    void DrawToolbar(CommandHistory& history, Dx11Renderer& renderer);
    void DrawCameraControls(Camera& camera);
    void DrawLighting(Dx11Renderer& renderer);
    void DrawResources(Dx11Renderer& renderer);
    void DrawHierarchy(Scene& scene);
    void DrawInspector(Scene& scene, CommandHistory& history, Dx11Renderer& renderer);
    void DrawMaterialEditor(
        Scene& scene, CommandHistory& history, Dx11Renderer& renderer, Entity& entity);
    void DrawViewport(Scene& scene, CommandHistory& history, Camera& camera, Dx11Renderer& renderer);
    void CreatePrimitive(Scene& scene, CommandHistory& history, PrimitiveType primitive);
    void ImportModel(Scene& scene, CommandHistory& history, Dx11Renderer& renderer);
    void TrackPropertyEdit(
        Scene& scene, CommandHistory& history, Entity& entity, const Transform& beforeControl);
    void TrackMaterialEdit(
        Scene& scene, CommandHistory& history, Entity& entity,
        const EntityMaterial& beforeControl);
    void ValidateSelection(const Scene& scene);

    std::uint32_t selectedEntityId_{};
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
    bool autoRotate_{false};
    bool autoRotateClockwise_{false};
    float autoRotateSpeedDegrees_{20.0F};
};

} // namespace lrender
