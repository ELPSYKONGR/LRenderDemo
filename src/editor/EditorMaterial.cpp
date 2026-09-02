/**
 * @file Entity material inspector and native texture selection workflow.
 * @author Codex
 * @created 2026-08-25
 * @depends editor/EditorLayer.h, commands/MaterialCommand.h, Win32 common dialogs
 */
#include "editor/EditorLayer.h"

#include "commands/MaterialCommand.h"
#include "render/Dx11Renderer.h"
#include "utils/Logger.h"

#include <array>
#include <commdlg.h>
#include <filesystem>
#include <format>
#include <memory>
#include <stdexcept>
#include <string>

namespace lrender
{
namespace
{

std::string PathUtf8(const std::filesystem::path& path)
{
    const auto text = path.u8string();
    return {reinterpret_cast<const char*>(text.data()), text.size()};
}

std::filesystem::path SelectTextureFile(HWND owner)
{
    std::array<wchar_t, 32768> pathBuffer{};
    const std::filesystem::path textureDirectory = std::filesystem::current_path() / "assets" / "textures";
    const std::filesystem::path initialDirectory =
        std::filesystem::is_directory(textureDirectory) ? textureDirectory : std::filesystem::current_path() / "assets";

    OPENFILENAMEW dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = owner;
    dialog.lpstrFilter = L"Texture files (*.dds;*.png;*.jpg;*.jpeg;*.bmp;*.tif;*.tiff;*.gif)\0"
                         L"*.dds;*.png;*.jpg;*.jpeg;*.bmp;*.tif;*.tiff;*.gif\0"
                         L"All files (*.*)\0*.*\0";
    dialog.lpstrFile = pathBuffer.data();
    dialog.nMaxFile = static_cast<DWORD>(pathBuffer.size());
    const std::wstring initialDirectoryText = initialDirectory.wstring();
    dialog.lpstrInitialDir = initialDirectoryText.c_str();
    dialog.lpstrDefExt = L"png";
    dialog.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameW(&dialog) != FALSE)
    {
        return pathBuffer.data();
    }
    const DWORD error = CommDlgExtendedError();
    if (error != 0)
    {
        throw std::runtime_error("Native texture file dialog failed with code " + std::to_string(error));
    }
    return {};
}

void PushDiscreteEdit(Scene& scene, CommandHistory& history, Entity& entity, const EntityMaterial& before, bool changed)
{
    const EntityMaterial after = entity.EditableMaterial();
    if (changed && !before.NearlyEquals(after))
    {
        entity.SetOverrideMaterial(after);
        history.PushApplied(std::make_unique<MaterialCommand>(scene, entity.id, before, after));
    }
}

} // namespace

void EditorLayer::DrawMaterialEditor(Scene& scene, CommandHistory& history, Dx11Renderer& renderer, Entity& entity)
{
    EntityMaterial& material = entity.EditableMaterial();
    if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
    {
        EntityMaterial before = material;
        ImGui::ColorEdit3("Base color", &material.baseColor.x);
        TrackMaterialEdit(scene, history, entity, before);

        before = material;
        ImGui::DragFloat("Diffuse strength", &material.diffuseStrength, 0.01F, 0.0F, 2.0F, "%.2f");
        TrackMaterialEdit(scene, history, entity, before);

        before = material;
        ImGui::ColorEdit3("Specular color", &material.specularColor.x);
        TrackMaterialEdit(scene, history, entity, before);

        before = material;
        ImGui::DragFloat("Specular strength", &material.specularStrength, 0.01F, 0.0F, 2.0F, "%.2f");
        TrackMaterialEdit(scene, history, entity, before);

        before = material;
        ImGui::DragFloat("Shininess", &material.shininess, 1.0F, 1.0F, 256.0F, "%.0f");
        TrackMaterialEdit(scene, history, entity, before);

        before = material;
        PushDiscreteEdit(scene, history, entity, before, ImGui::Checkbox("Double sided", &material.doubleSided));

        constexpr const char* displayModes[]{"Lit textured", "Texture only", "Lit untextured"};
        int displayMode = static_cast<int>(material.displayMode);
        before = material;
        if (ImGui::Combo("Display mode", &displayMode, displayModes, std::size(displayModes)))
        {
            material.displayMode = static_cast<SurfaceDisplayMode>(displayMode);
            PushDiscreteEdit(scene, history, entity, before, true);
        }

        ImGui::SeparatorText("Base color texture");
        if (ID3D11ShaderResourceView* preview = renderer.MaterialPreview(entity))
        {
            ImGui::Image(reinterpret_cast<ImTextureID>(preview), ImVec2(96.0F, 96.0F));
        }
        else
        {
            ImGui::TextDisabled("No texture preview");
        }
        if (material.useSourceTexture)
        {
            ImGui::TextUnformatted(entity.IsMesh() ? "Mesh texture (first material)" : "Generated checker texture");
        }
        else
        {
            const std::string fileName = PathUtf8(material.baseColorTexturePath.filename());
            ImGui::TextWrapped("%s", fileName.c_str());
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            {
                const std::string fullPath = PathUtf8(material.baseColorTexturePath);
                ImGui::SetTooltip("%s", fullPath.c_str());
            }
        }

        if (ImGui::Button("Choose..."))
        {
            try
            {
                const std::filesystem::path path = SelectTextureFile(renderer.WindowHandle());
                if (!path.empty())
                {
                    renderer.PreloadTexture(path);
                    EntityMaterial after = material;
                    after.baseColorTexturePath = path;
                    after.useSourceTexture = false;
                    history.Execute(std::make_unique<MaterialCommand>(scene, entity.id, material, std::move(after)));
                }
            }
            catch (const std::exception& error)
            {
                m_materialError = error.what();
                m_openMaterialErrorPopup = true;
                try
                {
                    Logger::Instance().Error("material", std::format("Texture selection failed: {}", error.what()));
                }
                catch (const std::exception& logError)
                {
                    m_materialError += std::format("\nLogging also failed: {}", logError.what());
                }
            }
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(material.useSourceTexture);
        if (ImGui::Button("Use source"))
        {
            EntityMaterial after = material;
            after.useSourceTexture = true;
            after.baseColorTexturePath.clear();
            history.Execute(std::make_unique<MaterialCommand>(scene, entity.id, material, std::move(after)));
        }
        ImGui::EndDisabled();

        constexpr const char* filters[]{"Point", "Linear", "Anisotropic"};
        int filter = static_cast<int>(material.filter);
        before = material;
        if (ImGui::Combo("Filter", &filter, filters, std::size(filters)))
        {
            material.filter = static_cast<MaterialFilter>(filter);
            PushDiscreteEdit(scene, history, entity, before, true);
        }

        constexpr const char* addressModes[]{"Wrap", "Clamp", "Mirror"};
        int addressMode = static_cast<int>(material.addressMode);
        before = material;
        if (ImGui::Combo("Address mode", &addressMode, addressModes, std::size(addressModes)))
        {
            material.addressMode = static_cast<MaterialAddressMode>(addressMode);
            PushDiscreteEdit(scene, history, entity, before, true);
        }

        ImGui::Separator();
        ImGui::BeginDisabled(!entity.HasMaterialOverride());
        if (ImGui::Button("Reset"))
        {
            const EntityMaterial beforeReset = entity.EffectiveMaterial();
            const EntityMaterial afterReset = entity.EntityMaterialData();
            history.Execute(std::make_unique<MaterialCommand>(scene, entity.id, beforeReset, afterReset));
        }
        ImGui::EndDisabled();
    }

    if (m_openMaterialErrorPopup)
    {
        ImGui::OpenPopup("Texture load failed");
        m_openMaterialErrorPopup = false;
    }
    if (ImGui::BeginPopupModal("Texture load failed", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextWrapped("%s", m_materialError.c_str());
        if (ImGui::Button("Close"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void EditorLayer::TrackMaterialEdit(Scene& scene, CommandHistory& history, Entity& entity,
                                    const EntityMaterial& beforeControl)
{
    if (ImGui::IsItemActivated())
    {
        m_materialEditStart = beforeControl;
        m_materialEditEntityId = entity.id;
    }
    if (ImGui::IsItemDeactivatedAfterEdit() && m_materialEditStart && m_materialEditEntityId == entity.id)
    {
        const EntityMaterial after = entity.EditableMaterial();
        if (!m_materialEditStart->NearlyEquals(after))
        {
            entity.SetOverrideMaterial(after);
            history.PushApplied(std::make_unique<MaterialCommand>(scene, entity.id, *m_materialEditStart, after));
        }
        m_materialEditStart.reset();
        m_materialEditEntityId = 0;
    }
}

} // namespace lrender
