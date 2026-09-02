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

namespace lrender
{

class Dx11Renderer;

class EditorLayer final
{
  public:
    void Draw(Scene& scene, CommandHistory& history, Camera& camera, Dx11Renderer& renderer);
    void RequestExit() noexcept;
    [[nodiscard]] bool ConsumeExitConfirmed() noexcept;
    [[nodiscard]] std::uint32_t SelectedEntityId() const noexcept;

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
    void DrawMaterialEditor(Scene& scene, CommandHistory& history, Dx11Renderer& renderer, Entity& entity);
    void DrawViewport(Scene& scene, CommandHistory& history, Camera& camera, Dx11Renderer& renderer);
    void BeginSolidCreation(PrimitiveType primitive, const Scene& scene);
    void CreateSolid(Scene& scene, CommandHistory& history, SolidGeometry geometry, std::string name);
    void ImportModel(Scene& scene, CommandHistory& history, Dx11Renderer& renderer);
    void TrackPropertyEdit(Scene& scene, CommandHistory& history, Entity& entity, const Transform& beforeControl);
    void TrackMaterialEdit(Scene& scene, CommandHistory& history, Entity& entity, const EntityMaterial& beforeControl);
    void TrackSolidEdit(Scene& scene, CommandHistory& history, Entity& entity, const SolidGeometry& beforeControl);
    bool SaveScene(const Scene& scene, CommandHistory& history, Dx11Renderer& renderer, bool saveAs);
    void OpenScene(Scene& scene, CommandHistory& history, Dx11Renderer& renderer, const std::filesystem::path& path);
    void QueueSceneAction(int action, Scene& scene, CommandHistory& history, Dx11Renderer& renderer,
                          std::filesystem::path path = {});
    void ExecuteSceneAction(Scene& scene, CommandHistory& history, Dx11Renderer& renderer);
    void ValidateSelection(const Scene& scene);

    std::uint32_t m_selectedEntityId = 0;
    ModelId m_selectedModelId = 0;
    ImGuizmo::OPERATION m_gizmoOperation = ImGuizmo::TRANSLATE;
    bool m_wasUsingGizmo = false;
    Transform m_gizmoStart;
    std::optional<Transform> m_propertyEditStart;
    std::uint32_t m_propertyEditEntityId = 0;
    std::optional<EntityMaterial> m_materialEditStart;
    std::uint32_t m_materialEditEntityId = 0;
    std::string m_importError;
    bool m_openImportErrorPopup = false;
    std::string m_materialError;
    bool m_openMaterialErrorPopup = false;
    std::optional<SolidGeometry> m_solidEditStart;
    std::uint32_t m_solidEditEntityId = 0;
    PrimitiveType m_pendingSolidType = PrimitiveType::Cube;
    bool m_openSolidCreatePopup = false;
    std::array<char, 128> m_solidCreateName = {};
    CubeParameters m_cubeCreateParameters;
    SphereParameters m_sphereCreateParameters;
    PlaneParameters m_planeCreateParameters;
    std::string m_solidCreateError;
    std::filesystem::path m_currentScenePath;
    std::filesystem::path m_pendingScenePath;
    int m_pendingSceneAction = 0;
    bool m_openUnsavedPopup = false;
    bool m_exitRequested = false;
    bool m_exitConfirmed = false;
    std::string m_sceneFileError;
    bool m_openSceneFileErrorPopup = false;
    bool m_autoRotate = false;
    bool m_autoRotateClockwise = false;
    float m_autoRotateSpeedDegrees = 20.0F;
};

} // namespace lrender
