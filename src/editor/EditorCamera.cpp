/**
 * @file Camera view presets and automatic orbit controls.
 * @author Codex
 * @created 2026-08-25
 * @depends editor/EditorLayer.h, core/Camera.h, ImGui
 */
#include "editor/EditorLayer.h"

#include <cfloat>

namespace lrender {

void EditorLayer::DrawCameraControls(Camera& camera) {
    ImGui::SetNextWindowSize(ImVec2(215.0F, 235.0F), ImGuiCond_Once);
    ImGui::SetNextWindowSizeConstraints(
        ImVec2(215.0F, 235.0F), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::Begin("Camera")) {
        if (ImGui::BeginTable("CameraViews", 2, ImGuiTableFlags_SizingStretchSame)) {
            constexpr struct ViewButton {
                const char* label;
                CameraViewPreset preset;
            } buttons[]{
                {"Front", CameraViewPreset::Front},
                {"Back", CameraViewPreset::Back},
                {"Left", CameraViewPreset::Left},
                {"Right", CameraViewPreset::Right},
                {"Top", CameraViewPreset::Top},
                {"Bottom", CameraViewPreset::Bottom},
                {"Right Iso", CameraViewPreset::RightIsometric},
                {"Left Iso", CameraViewPreset::LeftIsometric}};

            for (const ViewButton& button : buttons) {
                ImGui::TableNextColumn();
                if (ImGui::Button(button.label, ImVec2(-FLT_MIN, 0.0F))) {
                    camera.SetView(button.preset);
                }
            }
            ImGui::EndTable();
        }

        ImGui::Separator();
        ImGui::Checkbox("Auto rotate", &autoRotate_);
        ImGui::SameLine();
        ImGui::Checkbox("Clockwise", &autoRotateClockwise_);
        ImGui::TextUnformatted("Speed (deg/s)");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::SliderFloat(
            "##AutoRotateSpeed", &autoRotateSpeedDegrees_, 1.0F, 180.0F, "%.0f");
    }
    ImGui::End();

    if (autoRotate_) {
        const float direction = autoRotateClockwise_ ? -1.0F : 1.0F;
        camera.RotateAroundTarget(
            direction * autoRotateSpeedDegrees_ * ImGui::GetIO().DeltaTime);
    }
}

} // namespace lrender
