/**
 * @file Parameterized solid creation and inspector editing UI.
 * @author Codex
 * @created 2026-08-26
 * @depends editor/EditorLayer.h, commands/SolidGeometryCommand.h, ImGui
 */
#include "editor/EditorLayer.h"

#include "commands/SolidGeometryCommand.h"

#include <algorithm>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace lrender
{
namespace
{

constexpr int kMaximumSubdivisions = 256;

std::uint16_t ClampSubdivision(int value, int minimum)
{
    return static_cast<std::uint16_t>(std::clamp(value, minimum, kMaximumSubdivisions));
}

} // namespace

void EditorLayer::BeginSolidCreation(PrimitiveType primitive, const Scene& scene)
{
    if (primitive == PrimitiveType::Mesh)
    {
        throw std::invalid_argument("Mesh entities must be imported");
    }
    m_pendingSolidType = primitive;
    m_cubeCreateParameters = {};
    m_sphereCreateParameters = {};
    m_planeCreateParameters = {};
    const char* baseName = primitive == PrimitiveType::Cube     ? "Cube"
                           : primitive == PrimitiveType::Sphere ? "Sphere"
                                                                : "Plane";
    std::snprintf(m_solidCreateName.data(), m_solidCreateName.size(), "%s %zu", baseName, scene.EntityCount() + 1U);
    m_solidCreateError.clear();
    m_openSolidCreatePopup = true;
}

void EditorLayer::DrawSolidCreationPopup(Scene& scene, CommandHistory& history)
{
    if (m_openSolidCreatePopup)
    {
        ImGui::OpenPopup("Create parameterized solid");
        m_openSolidCreatePopup = false;
    }
    if (!ImGui::BeginPopupModal("Create parameterized solid", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }

    ImGui::InputText("Name", m_solidCreateName.data(), m_solidCreateName.size());
    switch (m_pendingSolidType)
    {
    case PrimitiveType::Cube:
        ImGui::DragFloat3("Size", &m_cubeCreateParameters.size.x, 0.05F, 0.001F, 10000.0F, "%.3f");
        break;
    case PrimitiveType::Sphere:
    {
        ImGui::DragFloat("Radius", &m_sphereCreateParameters.radius, 0.02F, 0.001F, 10000.0F, "%.3f");
        int slices = m_sphereCreateParameters.slices;
        int stacks = m_sphereCreateParameters.stacks;
        if (ImGui::DragInt("Slices", &slices, 1.0F, 3, kMaximumSubdivisions))
        {
            m_sphereCreateParameters.slices = ClampSubdivision(slices, 3);
        }
        if (ImGui::DragInt("Stacks", &stacks, 1.0F, 2, kMaximumSubdivisions))
        {
            m_sphereCreateParameters.stacks = ClampSubdivision(stacks, 2);
        }
        break;
    }
    case PrimitiveType::Plane:
    {
        ImGui::DragFloat2("Size", &m_planeCreateParameters.size.x, 0.05F, 0.001F, 10000.0F, "%.3f");
        int subdivisionsX = m_planeCreateParameters.subdivisionsX;
        int subdivisionsZ = m_planeCreateParameters.subdivisionsZ;
        if (ImGui::DragInt("X subdivisions", &subdivisionsX, 1.0F, 1, kMaximumSubdivisions))
        {
            m_planeCreateParameters.subdivisionsX = ClampSubdivision(subdivisionsX, 1);
        }
        if (ImGui::DragInt("Z subdivisions", &subdivisionsZ, 1.0F, 1, kMaximumSubdivisions))
        {
            m_planeCreateParameters.subdivisionsZ = ClampSubdivision(subdivisionsZ, 1);
        }
        break;
    }
    case PrimitiveType::Mesh:
        break;
    }

    if (!m_solidCreateError.empty())
    {
        ImGui::TextWrapped("%s", m_solidCreateError.c_str());
    }
    ImGui::BeginDisabled(m_solidCreateName[0] == '\0');
    if (ImGui::Button("Create"))
    {
        try
        {
            SolidGeometry geometry;
            switch (m_pendingSolidType)
            {
            case PrimitiveType::Cube:
                geometry = SolidGeometry::Cube(m_cubeCreateParameters);
                break;
            case PrimitiveType::Sphere:
                geometry = SolidGeometry::Sphere(m_sphereCreateParameters);
                break;
            case PrimitiveType::Plane:
                geometry = SolidGeometry::Plane(m_planeCreateParameters);
                break;
            case PrimitiveType::Mesh:
                throw std::invalid_argument("Mesh is not a parameterized solid");
            }
            CreateSolid(scene, history, std::move(geometry), m_solidCreateName.data());
            ImGui::CloseCurrentPopup();
        }
        catch (const std::exception& error)
        {
            m_solidCreateError = error.what();
        }
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void EditorLayer::DrawSolidGeometryEditor(Scene& scene, CommandHistory& history, Entity& entity)
{
    SolidGeometry* solid = entity.Solid();
    if (solid == nullptr || !ImGui::CollapsingHeader("Geometry", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }

    if (const auto* cube = std::get_if<CubeParameters>(&solid->Parameters()))
    {
        CubeParameters parameters = *cube;
        const SolidGeometry before = *solid;
        if (ImGui::DragFloat3("Dimensions", &parameters.size.x, 0.02F, 0.001F, 10000.0F, "%.3f"))
        {
            parameters.size.x = std::max(parameters.size.x, 0.001F);
            parameters.size.y = std::max(parameters.size.y, 0.001F);
            parameters.size.z = std::max(parameters.size.z, 0.001F);
            entity.geometry = SolidGeometry::Cube(parameters);
        }
        TrackSolidEdit(scene, history, entity, before);
    }
    else if (const auto* sphere = std::get_if<SphereParameters>(&solid->Parameters()))
    {
        SphereParameters parameters = *sphere;
        SolidGeometry before = *solid;
        if (ImGui::DragFloat("Radius", &parameters.radius, 0.01F, 0.001F, 10000.0F, "%.3f"))
        {
            parameters.radius = std::max(parameters.radius, 0.001F);
            entity.geometry = SolidGeometry::Sphere(parameters);
        }
        TrackSolidEdit(scene, history, entity, before);

        parameters = std::get<SphereParameters>(entity.Solid()->Parameters());
        int slices = parameters.slices;
        before = *entity.Solid();
        if (ImGui::DragInt("Slices", &slices, 1.0F, 3, kMaximumSubdivisions))
        {
            parameters.slices = ClampSubdivision(slices, 3);
            entity.geometry = SolidGeometry::Sphere(parameters);
        }
        TrackSolidEdit(scene, history, entity, before);

        parameters = std::get<SphereParameters>(entity.Solid()->Parameters());
        int stacks = parameters.stacks;
        before = *entity.Solid();
        if (ImGui::DragInt("Stacks", &stacks, 1.0F, 2, kMaximumSubdivisions))
        {
            parameters.stacks = ClampSubdivision(stacks, 2);
            entity.geometry = SolidGeometry::Sphere(parameters);
        }
        TrackSolidEdit(scene, history, entity, before);
    }
    else
    {
        PlaneParameters parameters = std::get<PlaneParameters>(solid->Parameters());
        SolidGeometry before = *solid;
        if (ImGui::DragFloat2("Dimensions", &parameters.size.x, 0.02F, 0.001F, 10000.0F, "%.3f"))
        {
            parameters.size.x = std::max(parameters.size.x, 0.001F);
            parameters.size.y = std::max(parameters.size.y, 0.001F);
            entity.geometry = SolidGeometry::Plane(parameters);
        }
        TrackSolidEdit(scene, history, entity, before);

        parameters = std::get<PlaneParameters>(entity.Solid()->Parameters());
        int subdivisionsX = parameters.subdivisionsX;
        before = *entity.Solid();
        if (ImGui::DragInt("X subdivisions", &subdivisionsX, 1.0F, 1, kMaximumSubdivisions))
        {
            parameters.subdivisionsX = ClampSubdivision(subdivisionsX, 1);
            entity.geometry = SolidGeometry::Plane(parameters);
        }
        TrackSolidEdit(scene, history, entity, before);

        parameters = std::get<PlaneParameters>(entity.Solid()->Parameters());
        int subdivisionsZ = parameters.subdivisionsZ;
        before = *entity.Solid();
        if (ImGui::DragInt("Z subdivisions", &subdivisionsZ, 1.0F, 1, kMaximumSubdivisions))
        {
            parameters.subdivisionsZ = ClampSubdivision(subdivisionsZ, 1);
            entity.geometry = SolidGeometry::Plane(parameters);
        }
        TrackSolidEdit(scene, history, entity, before);
    }
}

void EditorLayer::TrackSolidEdit(Scene& scene, CommandHistory& history, Entity& entity,
                                 const SolidGeometry& beforeControl)
{
    if (ImGui::IsItemActivated())
    {
        m_solidEditStart = beforeControl;
        m_solidEditEntityId = entity.id;
    }
    if (ImGui::IsItemDeactivatedAfterEdit() && m_solidEditStart && m_solidEditEntityId == entity.id)
    {
        const SolidGeometry& after = *entity.Solid();
        if (!m_solidEditStart->NearlyEquals(after, 0.0F))
        {
            history.PushApplied(std::make_unique<SolidGeometryCommand>(scene, entity.id, *m_solidEditStart, after));
        }
        m_solidEditStart.reset();
        m_solidEditEntityId = 0;
    }
}

} // namespace lrender
