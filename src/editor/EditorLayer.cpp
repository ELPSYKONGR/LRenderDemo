/**
 * @file Docking UI, scene panels, camera controls, and transform Gizmo implementation.
 * @author Codex
 * @created 2026-08-20
 * @depends editor/EditorLayer.h, render/Dx11Renderer.h, ImGui, ImGuizmo
 */
#include "editor/EditorLayer.h"

#include "commands/TransformCommand.h"
#include "render/Dx11Renderer.h"

#include <imgui.h>
#include <algorithm>
#include <memory>
#include <string>

namespace lrender {
namespace {

void ItemTooltip(const char* text) {
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
        ImGui::SetTooltip("%s", text);
    }
}

} // namespace

void EditorLayer::Draw(
    Scene& scene, CommandHistory& history, Camera& camera, Dx11Renderer& renderer) {
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
    DrawMainMenu(scene, history, renderer);
    DrawToolbar(history, renderer);
    DrawCameraControls(camera);
    DrawLighting(renderer);
    DrawResources(renderer);
    DrawHierarchy(scene);
    ValidateSelection(scene);
    DrawInspector(scene, history, renderer);
    DrawViewport(scene, history, camera, renderer);
}

void EditorLayer::DrawMainMenu(
    Scene& scene, CommandHistory& history, Dx11Renderer& renderer) {
    if (!ImGui::BeginMainMenuBar()) {
        return;
    }
    DrawSceneFileMenu(scene, history, renderer);
    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Undo", "Ctrl+Z", false, history.CanUndo())) {
            history.Undo();
            ValidateSelection(scene);
        }
        if (ImGui::MenuItem("Redo", "Ctrl+Y", false, history.CanRedo())) {
            history.Redo();
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Create")) {
        if (ImGui::MenuItem("Cube")) {
            BeginSolidCreation(PrimitiveType::Cube, scene);
        }
        if (ImGui::MenuItem("Sphere")) {
            BeginSolidCreation(PrimitiveType::Sphere, scene);
        }
        if (ImGui::MenuItem("Plane")) {
            BeginSolidCreation(PrimitiveType::Plane, scene);
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Import Mesh...")) {
            ImportModel(scene, history, renderer);
        }
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
    DrawSolidCreationPopup(scene, history);
    DrawSceneFilePopups(scene, history, renderer);

    const ImGuiIO& input = ImGui::GetIO();
    if (input.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z, false) && history.CanUndo()) {
        history.Undo();
        ValidateSelection(scene);
    }
    if (input.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y, false) && history.CanRedo()) {
        history.Redo();
    }

    if (openImportErrorPopup_) {
        ImGui::OpenPopup("Model import failed");
        openImportErrorPopup_ = false;
    }
    if (ImGui::BeginPopupModal("Model import failed", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("%s", importError_.c_str());
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void EditorLayer::DrawToolbar(CommandHistory& history, Dx11Renderer& renderer) {
    ImGui::Begin("Tools", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::BeginDisabled(!history.CanUndo());
    if (ImGui::Button("<-")) {
        history.Undo();
    }
    ImGui::EndDisabled();
    ItemTooltip("Undo");
    ImGui::SameLine();
    ImGui::BeginDisabled(!history.CanRedo());
    if (ImGui::Button("->")) {
        history.Redo();
    }
    ImGui::EndDisabled();
    ItemTooltip("Redo");
    ImGui::SameLine();
    if (ImGui::Button("T")) {
        gizmoOperation_ = ImGuizmo::TRANSLATE;
    }
    ItemTooltip("Translate");
    ImGui::SameLine();
    if (ImGui::Button("R")) {
        gizmoOperation_ = ImGuizmo::ROTATE;
    }
    ItemTooltip("Rotate");
    ImGui::SameLine();
    if (ImGui::Button("S")) {
        gizmoOperation_ = ImGuizmo::SCALE;
    }
    ItemTooltip("Scale");
    ImGui::SameLine();
    bool isWireframe = renderer.Effect().IsWireframe();
    if (ImGui::Checkbox("Wireframe", &isWireframe)) {
        renderer.Effect().SetWireframe(isWireframe);
    }
    ImGui::End();
}

void EditorLayer::DrawInspector(
    Scene& scene, CommandHistory& history, Dx11Renderer& renderer) {
    ImGui::Begin("Inspector");
    Entity* entity = scene.FindEntity(selectedEntityId_);
    if (entity == nullptr) {
        ImGui::TextDisabled("No entity selected");
        ImGui::End();
        return;
    }

    ImGui::TextUnformatted(entity->name.c_str());
    ImGui::Separator();

    Transform before = entity->transform;
    ImGui::DragFloat3("Position", &entity->transform.position.x, 0.05F);
    TrackPropertyEdit(scene, history, *entity, before);
    before = entity->transform;
    ImGui::DragFloat3("Rotation", &entity->transform.rotationDegrees.x, 0.5F);
    TrackPropertyEdit(scene, history, *entity, before);
    before = entity->transform;
    ImGui::DragFloat3("Scale", &entity->transform.scale.x, 0.02F, 0.01F, 100.0F);
    TrackPropertyEdit(scene, history, *entity, before);
    DrawSolidGeometryEditor(scene, history, *entity);
    DrawMaterialEditor(scene, history, renderer, *entity);
    ImGui::End();
}

void EditorLayer::DrawViewport(
    Scene& scene, CommandHistory& history, Camera& camera, Dx11Renderer& renderer) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0F, 0.0F));
    ImGui::Begin("Viewport");
    const ImVec2 available = ImGui::GetContentRegionAvail();
    const auto width = static_cast<std::uint32_t>(std::max(available.x, 1.0F));
    const auto height = static_cast<std::uint32_t>(std::max(available.y, 1.0F));
    renderer.ResizeViewport(width, height);

    const ImVec2 viewportPosition = ImGui::GetCursorScreenPos();
    ImGui::Image(
        reinterpret_cast<ImTextureID>(renderer.ViewportTarget().ShaderResourceView()), available);
    const bool isHovered = ImGui::IsItemHovered();
    const ImGuiIO& input = ImGui::GetIO();
    if (isHovered && !ImGuizmo::IsUsing()) {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
            autoRotate_ = false;
            camera.Orbit(input.MouseDelta.x, input.MouseDelta.y);
        }
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
            camera.Pan(input.MouseDelta.x, input.MouseDelta.y);
        }
        if (input.MouseWheel != 0.0F) {
            camera.Zoom(input.MouseWheel);
        }
        if (!input.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_W, false)) {
            gizmoOperation_ = ImGuizmo::TRANSLATE;
        }
        if (!input.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_E, false)) {
            gizmoOperation_ = ImGuizmo::ROTATE;
        }
        if (!input.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_R, false)) {
            gizmoOperation_ = ImGuizmo::SCALE;
        }
    }

    Entity* entity = scene.FindEntity(selectedEntityId_);
    if (entity != nullptr) {
        auto world = entity->transform.ToMatrix();
        auto view = camera.ViewMatrix();
        auto projection = camera.ProjectionMatrix(static_cast<float>(width) / height);
        const Transform beforeManipulate = entity->transform;

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
        ImGuizmo::SetRect(viewportPosition.x, viewportPosition.y, available.x, available.y);
        ImGuizmo::Manipulate(
            &view._11, &projection._11, gizmoOperation_, ImGuizmo::LOCAL, &world._11);

        const bool isUsing = ImGuizmo::IsUsing();
        if (isUsing) {
            float translation[3]{};
            float rotation[3]{};
            float scale[3]{};
            ImGuizmo::DecomposeMatrixToComponents(&world._11, translation, rotation, scale);
            entity->transform.position = {translation[0], translation[1], translation[2]};
            entity->transform.rotationDegrees = {rotation[0], rotation[1], rotation[2]};
            entity->transform.scale = {
                std::max(scale[0], 0.01F), std::max(scale[1], 0.01F), std::max(scale[2], 0.01F)};
        }
        if (isUsing && !wasUsingGizmo_) {
            gizmoStart_ = beforeManipulate;
        }
        if (!isUsing && wasUsingGizmo_ && !gizmoStart_.NearlyEquals(entity->transform)) {
            history.PushApplied(std::make_unique<TransformCommand>(
                scene, entity->id, gizmoStart_, entity->transform));
        }
        wasUsingGizmo_ = isUsing;
    } else {
        wasUsingGizmo_ = false;
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void EditorLayer::TrackPropertyEdit(
    Scene& scene, CommandHistory& history, Entity& entity, const Transform& beforeControl) {
    if (ImGui::IsItemActivated()) {
        propertyEditStart_ = beforeControl;
        propertyEditEntityId_ = entity.id;
    }
    if (ImGui::IsItemDeactivatedAfterEdit() && propertyEditStart_ &&
        propertyEditEntityId_ == entity.id) {
        if (!propertyEditStart_->NearlyEquals(entity.transform)) {
            history.PushApplied(std::make_unique<TransformCommand>(
                scene, entity.id, *propertyEditStart_, entity.transform));
        }
        propertyEditStart_.reset();
        propertyEditEntityId_ = 0;
    }
}

void EditorLayer::ValidateSelection(const Scene& scene) {
    if (selectedEntityId_ != 0 && scene.FindEntity(selectedEntityId_) == nullptr) {
        selectedEntityId_ = 0;
    }
    if (selectedModelId_ != 0 && scene.FindModel(selectedModelId_) == nullptr) {
        selectedModelId_ = 0;
    }
    if (selectedEntityId_ != 0) {
        if (const Model* model = scene.FindEntityModel(selectedEntityId_)) {
            selectedModelId_ = model->id;
        }
    }
}

} // namespace lrender
