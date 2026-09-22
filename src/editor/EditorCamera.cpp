/**
 * @file Camera view presets and automatic orbit controls.
 * @author Codex
 * @created 2026-08-25
 * @depends editor/EditorLayer.h, core/Camera.h, ImGui
 */
#include "editor/EditorLayer.h"

#include "render/Dx11Renderer.h"
#include "render/ViewManager.h"

#include <cfloat>

namespace lrender
{

namespace
{

void CameraItemTooltip(const char* text)
{
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
    {
        ImGui::SetTooltip("%s", text);
    }
}

} // namespace

void EditorLayer::DrawCameraControls(Scene& scene, Camera& camera, Dx11Renderer& renderer)
{
    ImGui::SetNextWindowSize(ImVec2(215.0F, 235.0F), ImGuiCond_Once);
    ImGui::SetNextWindowSizeConstraints(ImVec2(215.0F, 235.0F), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::Begin("Camera"))
    {
        if (ImGui::BeginTable("CameraViews", 2, ImGuiTableFlags_SizingStretchSame))
        {
            constexpr struct ViewButton
            {
                const char* label;
                CameraViewPreset preset;
            } buttons[]{{"Front", CameraViewPreset::Front},
                        {"Back", CameraViewPreset::Back},
                        {"Left", CameraViewPreset::Left},
                        {"Right", CameraViewPreset::Right},
                        {"Top", CameraViewPreset::Top},
                        {"Bottom", CameraViewPreset::Bottom},
                        {"Right Iso", CameraViewPreset::RightIsometric},
                        {"Left Iso", CameraViewPreset::LeftIsometric}};

            for (const ViewButton& button : buttons)
            {
                ImGui::TableNextColumn();
                if (ImGui::Button(button.label, ImVec2(-FLT_MIN, 0.0F)))
                {
                    camera.SetView(button.preset);
                }
            }
            ImGui::EndTable();
        }

        ImGui::Separator();
        const float aspectRatio = static_cast<float>(renderer.ViewportResource().GetWidth()) /
                                  static_cast<float>(renderer.ViewportResource().GetHeight());
        if (ImGui::Button("Frame Selected"))
        {
            if (const Entity* entity = scene.FindEntity(m_selectedEntityId))
            {
                scene.CalculateBoundingBoxes();
                static_cast<void>(ViewManager::Instance().FitView(
                    camera, entity->BoundingBoxData(), aspectRatio));
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Frame Scene"))
        {
            scene.CalculateBoundingBoxes();
            static_cast<void>(ViewManager::Instance().FitView(
                camera, scene.BoundingBoxData(), aspectRatio));
        }
        CameraItemTooltip("Frame Selected: F. Frame Scene: Home.");
        if (ImGui::Button("Center View", ImVec2(-FLT_MIN, 0.0F)))
        {
            scene.CalculateBoundingBoxes();
            if (const Entity* entity = scene.FindEntity(m_selectedEntityId))
            {
                static_cast<void>(ViewManager::Instance().FitView(
                    camera, entity->BoundingBoxData(), aspectRatio));
            }
            else
            {
                static_cast<void>(ViewManager::Instance().FitView(
                    camera, scene.BoundingBoxData(), aspectRatio));
            }
        }
        CameraItemTooltip("Center View: selected entity when available, otherwise the whole scene.");

        ImGui::Separator();
        ImGui::Checkbox("Auto rotate", &m_autoRotate);
        ImGui::SameLine();
        ImGui::Checkbox("Clockwise", &m_autoRotateClockwise);
        ImGui::TextUnformatted("Speed (deg/s)");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::SliderFloat("##AutoRotateSpeed", &m_autoRotateSpeedDegrees, 1.0F, 180.0F, "%.0f");
    }
    ImGui::End();

    const float aspectRatio = static_cast<float>(renderer.ViewportResource().GetWidth()) /
                              static_cast<float>(renderer.ViewportResource().GetHeight());
    if (ImGui::IsKeyPressed(ImGuiKey_F, false))
    {
        if (const Entity* entity = scene.FindEntity(m_selectedEntityId))
        {
            scene.CalculateBoundingBoxes();
            static_cast<void>(ViewManager::Instance().FitView(
                camera, entity->BoundingBoxData(), aspectRatio));
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Home, false))
    {
        scene.CalculateBoundingBoxes();
        static_cast<void>(ViewManager::Instance().FitView(
            camera, scene.BoundingBoxData(), aspectRatio));
    }

    if (m_autoRotate)
    {
        const float direction = m_autoRotateClockwise ? -1.0F : 1.0F;
        camera.RotateAroundTarget(direction * m_autoRotateSpeedDegrees * ImGui::GetIO().DeltaTime);
    }
}

} // namespace lrender
