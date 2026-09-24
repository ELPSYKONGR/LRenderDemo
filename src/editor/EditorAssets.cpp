/**
 * @file Native model import and editable resource/lighting panels.
 * @author Codex
 * @created 2026-08-21
 * @depends editor/EditorLayer.h, render/Dx11Renderer.h, Win32 common dialogs
 */
#include "editor/EditorLayer.h"

#include "commands/CreateModelCommand.h"
#include "render/Dx11Renderer.h"
#include "render/LightManager.h"
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

std::filesystem::path SelectModelFile(HWND owner)
{
    std::array<wchar_t, 32768> pathBuffer{};
    const std::filesystem::path assetDirectory = std::filesystem::current_path() / "assets" / "test-scenes" / "downloads";
    const std::wstring initialDirectory = std::filesystem::is_directory(assetDirectory)
                                              ? assetDirectory.wstring()
                                              : std::filesystem::current_path().wstring();

    OPENFILENAMEW dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = owner;
    dialog.lpstrFilter = L"Mesh assets (*.gltf;*.glb;*.obj)\0*.gltf;*.glb;*.obj\0"
                         L"glTF models (*.gltf;*.glb)\0*.gltf;*.glb\0"
                         L"Wavefront OBJ (*.obj)\0*.obj\0All files (*.*)\0*.*\0";
    dialog.lpstrFile = pathBuffer.data();
    dialog.nMaxFile = static_cast<DWORD>(pathBuffer.size());
    dialog.lpstrInitialDir = initialDirectory.c_str();
    dialog.lpstrDefExt = L"gltf";
    dialog.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameW(&dialog) != FALSE)
    {
        return pathBuffer.data();
    }
    const DWORD error = CommDlgExtendedError();
    if (error != 0)
    {
        throw std::runtime_error("Native model file dialog failed with code " + std::to_string(error));
    }
    return {};
}

} // namespace

void EditorLayer::ImportModel(Scene& scene, CommandHistory& history, Dx11Renderer& renderer)
{
    try
    {
        const std::filesystem::path path = SelectModelFile(renderer.WindowHandle());
        if (path.empty())
        {
            return;
        }
        const auto asset = renderer.PreloadModel(path);
        std::vector<std::string> entityNames;
        std::vector<BoundingBox> entityBounds;
        entityNames.reserve(asset->entities.size());
        entityBounds.reserve(asset->entities.size());
        for (const MeshAssetEntity& entity : asset->entities)
        {
            entityNames.push_back(entity.name);
            entityBounds.push_back(entity.localBoundingBox);
        }
        for (const std::string& warning : asset->warnings)
        {
            Logger::Instance().Info("assets", "Mesh import warning: " + warning);
        }
        Model& model = scene.CreateMeshModel(path, PathUtf8(path.stem()), entityNames, entityBounds);
        for (std::size_t entityIndex = 0; entityIndex < model.entities.size(); ++entityIndex)
        {
            if (!asset->entities[entityIndex].parts.empty())
            {
                Material sourceMaterial = asset->entities[entityIndex].parts.front().material;
                sourceMaterial.SetTextureSource(MaterialTextureSource::Source);
                model.entities[entityIndex].EntityMaterialData() = std::move(sourceMaterial);
            }
        }
        m_selectedModelId = model.id;
        m_selectedEntityId = model.entities.front().id;
        history.PushApplied(std::make_unique<CreateModelCommand>(scene, model));
    }
    catch (const std::exception& error)
    {
        m_importError = error.what();
        m_openImportErrorPopup = true;
        try
        {
            Logger::Instance().Error("assets", std::format("Model import failed: {}", error.what()));
        }
        catch (const std::exception& logError)
        {
            m_importError += std::format("\nLogging also failed: {}", logError.what());
        }
    }
}

void EditorLayer::DrawLighting()
{
    ImGui::Begin("Lighting");
    LightManager& lights = LightManager::Instance();
    ImGui::ColorEdit3("Ambient", &lights.Ambient().x);
    ImGui::DragFloat("Ambient Intensity", &lights.AmbientIntensity(), 0.01F, 0.0F, 20.0F);

    LightId pendingRemoval = 0;

    if (DirectionalLight* light = lights.Directional())
    {
        if (ImGui::CollapsingHeader("Directional light", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("Enabled##directional", &light->enabled);
            ImGui::DragFloat3("Direction", &light->direction.x, 0.01F, -1.0F, 1.0F);
            ImGui::ColorEdit3("Color##directional", &light->color.x);
            ImGui::DragFloat("Intensity##directional", &light->intensity, 0.02F, 0.0F, 20.0F);
            if (ImGui::Button("Delete directional light"))
            {
                pendingRemoval = light->id;
            }
        }
    }
    else if (ImGui::Button("Add directional light"))
    {
        static_cast<void>(lights.AddDirectionalLight());
    }

    std::size_t index = 0;
    for (PointLight& light : lights.PointLights())
    {
        ImGui::PushID(static_cast<int>(light.id));
        const std::string label = "Point light " + std::to_string(index + 1);
        if (ImGui::CollapsingHeader(label.c_str()))
        {
            ImGui::Checkbox("Enabled", &light.enabled);
            ImGui::DragFloat3("Position", &light.position.x, 0.05F);
            ImGui::ColorEdit3("Color", &light.color.x);
            ImGui::DragFloat("Intensity", &light.intensity, 0.02F, 0.0F, 50.0F);
            ImGui::DragFloat("Range", &light.range, 0.05F, 0.01F, 1000.0F);
            if (ImGui::Button("Delete"))
            {
                pendingRemoval = light.id;
            }
        }
        ImGui::PopID();
        ++index;
    }

    if (lights.PointLights().size() < LightManager::MaxPointLights)
    {
        if (ImGui::Button("Add point light"))
        {
            static_cast<void>(lights.AddPointLight());
        }
    }
    else
    {
        ImGui::TextDisabled("Maximum point lights reached (%zu)", LightManager::MaxPointLights);
    }

    if (pendingRemoval != 0)
    {
        static_cast<void>(lights.RemoveLight(pendingRemoval));
    }
    ImGui::End();
}

void EditorLayer::DrawResources(Dx11Renderer& renderer)
{
    ImGui::Begin("Resources");
    ImGui::Text("Cached mesh assets: %zu", renderer.CachedMeshAssetCount());
    ImGui::Text("Cached textures: %zu", renderer.CachedTextureCount());
    ImGui::TextDisabled("Create > Import Mesh...");
    ImGui::End();
}

} // namespace lrender
