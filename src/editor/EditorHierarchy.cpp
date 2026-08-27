/**
 * @file Model/entity hierarchy and solid creation workflow.
 * @author Codex
 * @created 2026-08-25
 * @depends editor/EditorLayer.h, commands/CreateEntityCommand.h
 */
#include "editor/EditorLayer.h"

#include "commands/CreateEntityCommand.h"
#include "commands/CreateModelCommand.h"

#include <imgui.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace lrender {

void EditorLayer::DrawHierarchy(Scene& scene) {
    ImGui::Begin("Hierarchy");
    for (const Model& model : scene.Models()) {
        ImGui::PushID(static_cast<int>(model.id));
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen |
                                   ImGuiTreeNodeFlags_OpenOnArrow |
                                   ImGuiTreeNodeFlags_SpanAvailWidth;
        if (model.id == selectedModelId_ && selectedEntityId_ == 0) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        const bool isOpen = ImGui::TreeNodeEx("##Model", flags, "%s", model.name.c_str());
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            selectedModelId_ = model.id;
            selectedEntityId_ = 0;
        }
        if (isOpen) {
            for (const Entity& entity : model.entities) {
                if (ImGui::Selectable(
                        entity.name.c_str(), entity.id == selectedEntityId_)) {
                    selectedModelId_ = model.id;
                    selectedEntityId_ = entity.id;
                }
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    ImGui::End();
}

void EditorLayer::CreateSolid(
    Scene& scene, CommandHistory& history,
    SolidGeometry geometry, std::string name) {
    Model* model = scene.FindModel(selectedModelId_);
    const bool createdModel = model == nullptr;
    if (createdModel) {
        model = &scene.CreateModel("Model " + std::to_string(scene.Models().size() + 1));
    }
    const PrimitiveType primitive = geometry.Type();
    Entity& entity = scene.CreateSolidEntity(model->id, std::move(geometry), std::move(name));
    if (primitive == PrimitiveType::Plane) {
        entity.transform.position.y = -0.5F;
        entity.material.baseColor = {0.55F, 0.58F, 0.62F, 1.0F};
    }
    selectedModelId_ = model->id;
    selectedEntityId_ = entity.id;
    if (createdModel) {
        history.PushApplied(std::make_unique<CreateModelCommand>(scene, *model));
    } else {
        history.PushApplied(std::make_unique<CreateEntityCommand>(scene, model->id, entity));
    }
}

} // namespace lrender
