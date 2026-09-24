/**
 * @file Versioned .lscene save/load regression tests.
 * @author Codex
 * @created 2026-08-26
 * @depends persistence/SceneSerializer.h
 */
#include "persistence/SceneSerializer.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace
{

void Require(bool condition, const char* message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

void RequireNear(float actual, float expected, const char* message)
{
    Require(std::abs(actual - expected) < 0.001F, message);
}

void TestSceneRoundTrip()
{
    const std::filesystem::path directory = std::filesystem::current_path() / "scratch" / "tests";
    std::filesystem::create_directories(directory);
    const std::filesystem::path scenePath = directory / "persistence-roundtrip.lscene";

    lrender::Scene source;
    const auto modelId = source.CreateModel("Mixed model").id;
    auto& sphere = source.CreateSolidEntity(modelId, lrender::SolidGeometry::Sphere({1.75F, 40, 22}), "Sphere");
    sphere.transform.position = {2.0F, 3.0F, 4.0F};
    sphere.EntityMaterialData().SetBaseColor({0.2F, 0.4F, 0.8F, 1.0F});
    sphere.EntityMaterialData().SetBaseColorTexturePath("assets/textures/custom.png");
    sphere.EntityMaterialData().SetTextureSource(lrender::MaterialTextureSource::Custom);
    const lrender::EntityId sphereId = sphere.id;
    lrender::Material expectedMaterial = sphere.EffectiveMaterial();
    expectedMaterial.SetBaseColorTexturePath(
        std::filesystem::absolute(expectedMaterial.GetBaseColorTexturePath()).lexically_normal());
    const auto& mesh = source.CreateMeshEntity(modelId, "assets/test-scenes/sample.obj", 3, "Imported");
    const lrender::EntityId meshId = mesh.id;
    lrender::Material meshOverride = mesh.EntityMaterialData();
    meshOverride.SetBaseColor({0.7F, 0.3F, 0.2F, 1.0F});
    meshOverride.SetDisplayMode(lrender::SurfaceDisplayMode::LitTextured);
    meshOverride.SetTextureSource(lrender::MaterialTextureSource::Custom);
    meshOverride.SetEmbeddedBaseColorTextureKey("Persistence.Test.Embedded");
    source.FindEntity(meshId)->SetOverrideMaterial(meshOverride);
    const std::filesystem::path expectedMeshPath = std::filesystem::absolute(mesh.Mesh()->assetPath).lexically_normal();

    lrender::LightingSettings sourceLighting;
    sourceLighting.ambient = {0.1F, 0.2F, 0.3F, 1.0F};
    sourceLighting.ambientIntensity = 0.65F;
    sourceLighting.directional = lrender::DirectionalLight{0, false, {-0.25F, -0.9F, 0.1F}, {0.8F, 0.7F, 0.6F, 1.0F}, 2.25F};
    sourceLighting.points.push_back(
        lrender::PointLight{0, true, {1.0F, 2.0F, 3.0F}, {0.3F, 0.5F, 0.7F, 1.0F}, 4.0F, 9.0F});

    lrender::SceneSerializer::Save(source, sourceLighting, scenePath);
    const lrender::SceneDocument document = lrender::SceneSerializer::LoadDocument(scenePath);
    const lrender::Scene& loaded = document.scene;
    Require(loaded.Models().size() == 1, "Scene model count should survive round trip");
    Require(loaded.Models()[0].entities.size() == 2, "Scene entity count should survive round trip");
    const lrender::Entity* loadedSphere = loaded.FindEntity(sphereId);
    Require(loadedSphere != nullptr && loadedSphere->IsSolid(), "Parameterized solid should survive round trip");
    const auto& parameters = std::get<lrender::SphereParameters>(loadedSphere->Solid()->Parameters());
    RequireNear(parameters.radius, 1.75F, "Sphere radius should survive round trip");
    Require(parameters.slices == 40 && parameters.stacks == 22, "Sphere topology should survive round trip");
    RequireNear(loadedSphere->transform.position.y, 3.0F, "Transform should survive round trip");
    Require(loadedSphere->EffectiveMaterial().NearlyEquals(expectedMaterial), "Material should survive round trip");

    const lrender::Entity* loadedMesh = loaded.FindEntity(meshId);
    Require(loadedMesh != nullptr && loadedMesh->IsMesh(), "Mesh reference should survive round trip");
    Require(loadedMesh->Mesh()->assetPath == expectedMeshPath, "Mesh path should resolve relative to the scene file");
    Require(loadedMesh->HasMaterialOverride(), "Mesh material override state should survive round trip");
    Require(loadedMesh->EffectiveMaterial().NearlyEquals(meshOverride),
            "Mesh material override should survive round trip");

    Require(document.lighting.has_value(), "Lighting settings should survive scene round trip");
    const lrender::LightingSettings& loadedLighting = *document.lighting;
    RequireNear(loadedLighting.ambient.x, 0.1F, "Ambient color should survive round trip");
    RequireNear(loadedLighting.ambientIntensity, 0.65F, "Ambient intensity should survive round trip");
    Require(loadedLighting.directional.has_value(), "Directional light should survive round trip");
    Require(!loadedLighting.directional->enabled, "Directional enabled state should survive round trip");
    RequireNear(loadedLighting.directional->intensity, 2.25F,
                "Directional intensity should survive round trip");
    Require(loadedLighting.points.size() == 1, "Point-light count should survive round trip");
    RequireNear(loadedLighting.points[0].range, 9.0F, "Point-light range should survive round trip");

    lrender::SceneSerializer::Save(source, scenePath);
    Require(!lrender::SceneSerializer::LoadDocument(scenePath).lighting.has_value(),
            "Legacy scene save should remain valid without lighting settings");

    std::error_code ignored;
    std::filesystem::remove(scenePath, ignored);
}

void TestUnsupportedVersionIsRejected()
{
    const std::filesystem::path directory = std::filesystem::current_path() / "scratch" / "tests";
    std::filesystem::create_directories(directory);
    const std::filesystem::path path = directory / "unsupported-version.lscene";
    {
        std::ofstream output(path, std::ios::binary | std::ios::trunc);
        output << R"({"format":"LRenderScene","version":99,"models":[]})";
    }
    bool rejected = false;
    try
    {
        static_cast<void>(lrender::SceneSerializer::Load(path));
    }
    catch (const std::exception&)
    {
        rejected = true;
    }
    Require(rejected, "Unsupported scene version should be rejected");
    std::error_code ignored;
    std::filesystem::remove(path, ignored);
}

void TestLegacyMeshMaterialBecomesOverride()
{
    const std::filesystem::path directory = std::filesystem::current_path() / "scratch" / "tests";
    std::filesystem::create_directories(directory);
    const std::filesystem::path path = directory / "legacy-mesh-material.lscene";
    {
        std::ofstream output(path, std::ios::binary | std::ios::trunc);
        output << R"({
  "format": "LRenderScene",
  "version": 1,
  "models": [{
    "id": 1,
    "name": "Legacy model",
    "entities": [{
      "id": 2,
      "name": "Legacy mesh",
      "geometry": {"kind": "mesh", "assetPath": "legacy.obj", "assetEntityIndex": 0},
      "transform": {
        "position": [0.0, 0.0, 0.0],
        "rotationDegrees": [0.0, 0.0, 0.0],
        "scale": [1.0, 1.0, 1.0]
      },
      "material": {
        "baseColor": [0.2, 0.4, 0.8, 1.0],
        "diffuseStrength": 0.75,
        "specularColor": [1.0, 1.0, 1.0, 1.0],
        "specularStrength": 0.25,
        "shininess": 32.0,
        "doubleSided": false,
        "displayMode": "lit-untextured",
        "useSourceTexture": true,
        "baseColorTexturePath": "",
        "filter": "linear",
        "addressMode": "wrap"
      }
    }]
  }]
})";
    }

    const lrender::Scene scene = lrender::SceneSerializer::Load(path);
    const lrender::Entity* entity = scene.FindEntity(2);
    Require(entity != nullptr && entity->IsMesh(), "Legacy mesh should load");
    Require(entity->HasMaterialOverride(), "Legacy mesh material should be restored as an override");
    RequireNear(entity->EffectiveMaterial().GetBaseColor().z, 0.8F, "Legacy mesh override color should survive load");
    RequireNear(entity->EffectiveMaterial().GetDiffuseStrength(), 0.75F,
                "Legacy mesh override properties should survive load");

    std::error_code ignored;
    std::filesystem::remove(path, ignored);
}

} // namespace

int main()
{
    try
    {
        TestSceneRoundTrip();
        TestUnsupportedVersionIsRejected();
        TestLegacyMeshMaterialBecomesOverride();
        std::cout << "LRenderPersistenceTests: all tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "LRenderPersistenceTests failed: " << error.what() << '\n';
        return 1;
    }
}
