/**
 * @file Versioned JSON persistence for editable scenes.
 * @author Codex
 * @created 2026-08-26
 * @depends core/Scene.h, nlohmann/json
 */
#pragma once

#include "core/Scene.h"
#include "render/Lighting.h"

#include <filesystem>
#include <optional>

namespace lrender
{

struct SceneDocument
{
    Scene scene;
    std::optional<LightingSettings> lighting;
};

class SceneSerializer final
{
  public:
    static void Save(const Scene& scene, const std::filesystem::path& path);
    static void Save(const Scene& scene, const LightingSettings& lighting, const std::filesystem::path& path);
    [[nodiscard]] static Scene Load(const std::filesystem::path& path);
    [[nodiscard]] static SceneDocument LoadDocument(const std::filesystem::path& path);
};

} // namespace lrender
