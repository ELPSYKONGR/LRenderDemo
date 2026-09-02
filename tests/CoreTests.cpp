/**
 * @file Lightweight behavioral tests for scene and command history.
 * @author Codex
 * @created 2026-08-20
 * @depends core/Scene.h, commands/CommandHistory.h
 */
#include "commands/CommandHistory.h"
#include "commands/CreateEntityCommand.h"
#include "commands/CreateModelCommand.h"
#include "commands/MaterialCommand.h"
#include "commands/SolidGeometryCommand.h"
#include "commands/TransformCommand.h"
#include "core/Camera.h"
#include "core/Scene.h"

#include <cmath>
#include <iostream>
#include <filesystem>
#include <memory>
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

template <typename Function> void RequireThrows(Function&& function, const char* message)
{
    try
    {
        function();
    }
    catch (const std::exception&)
    {
        return;
    }
    throw std::runtime_error(message);
}

void RequireFinite(const DirectX::SimpleMath::Matrix& matrix, const char* message)
{
    const float values[]{matrix._11, matrix._12, matrix._13, matrix._14, matrix._21, matrix._22,
                         matrix._23, matrix._24, matrix._31, matrix._32, matrix._33, matrix._34,
                         matrix._41, matrix._42, matrix._43, matrix._44};
    for (const float value : values)
    {
        Require(std::isfinite(value), message);
    }
}

void TestCameraViewPresets()
{
    lrender::Camera camera;
    constexpr struct ExpectedView
    {
        lrender::CameraViewPreset preset;
        DirectX::SimpleMath::Vector3 direction;
    } views[]{{lrender::CameraViewPreset::Front, {0.0F, 0.0F, 1.0F}},
              {lrender::CameraViewPreset::Back, {0.0F, 0.0F, -1.0F}},
              {lrender::CameraViewPreset::Left, {-1.0F, 0.0F, 0.0F}},
              {lrender::CameraViewPreset::Right, {1.0F, 0.0F, 0.0F}},
              {lrender::CameraViewPreset::Top, {0.0F, 1.0F, 0.0F}},
              {lrender::CameraViewPreset::Bottom, {0.0F, -1.0F, 0.0F}}};

    for (const ExpectedView& view : views)
    {
        camera.SetView(view.preset);
        const auto position = camera.Position();
        RequireNear(position.x / 8.0F, view.direction.x, "Camera preset X is incorrect");
        RequireNear(position.y / 8.0F, view.direction.y, "Camera preset Y is incorrect");
        RequireNear(position.z / 8.0F, view.direction.z, "Camera preset Z is incorrect");
        RequireFinite(camera.ViewMatrix(), "Camera preset produced an invalid view matrix");
    }

    camera.SetView(lrender::CameraViewPreset::RightIsometric);
    Require(camera.Position().x > 0.0F && camera.Position().y > 0.0F,
            "Right isometric view should be above and right of the target");
    camera.SetView(lrender::CameraViewPreset::LeftIsometric);
    Require(camera.Position().x < 0.0F && camera.Position().y > 0.0F,
            "Left isometric view should be above and left of the target");

    camera.SetView(lrender::CameraViewPreset::Front);
    camera.RotateAroundTarget(90.0F);
    RequireNear(camera.Position().x, 8.0F, "Automatic rotation should advance camera yaw");
}

void TestSceneLifecycle()
{
    lrender::Scene scene;
    const auto modelId = scene.CreateModel("Model").id;
    const auto id = scene.CreateEntity(modelId, lrender::PrimitiveType::Cube, "Cube").id;
    Require(scene.FindEntityModel(id)->id == modelId, "Entity should belong to its model");
    Require(scene.FindEntity(id) != nullptr, "Created entity should be findable");
    Require(scene.RemoveEntity(id).has_value(), "Remove should return a snapshot");
    Require(scene.FindEntity(id) == nullptr, "Removed entity should be absent");
}

void TestTransformUndoRedo()
{
    lrender::Scene scene;
    lrender::CommandHistory history;
    const auto modelId = scene.CreateModel("Model").id;
    auto& entity = scene.CreateEntity(modelId, lrender::PrimitiveType::Sphere, "Sphere");
    const lrender::Transform before = entity.transform;
    lrender::Transform after = before;
    after.position.x = 4.0F;

    history.Execute(std::make_unique<lrender::TransformCommand>(scene, entity.id, before, after));
    Require(scene.FindEntity(entity.id)->transform.position.x == 4.0F, "Execute should move entity");
    Require(history.Undo(), "Undo should be available");
    Require(scene.FindEntity(entity.id)->transform.position.x == 0.0F, "Undo should restore transform");
    Require(history.Redo(), "Redo should be available");
    Require(scene.FindEntity(entity.id)->transform.position.x == 4.0F, "Redo should restore edit");
}

void TestCreateUndoRedo()
{
    lrender::Scene scene;
    lrender::CommandHistory history;
    const auto modelId = scene.CreateModel("Model").id;
    const auto entity = scene.CreateEntity(modelId, lrender::PrimitiveType::Plane, "Plane");
    history.PushApplied(std::make_unique<lrender::CreateEntityCommand>(scene, modelId, entity));
    Require(history.Undo(), "Creation should be undoable");
    Require(scene.FindEntity(entity.id) == nullptr, "Undo should remove created entity");
    Require(history.Redo(), "Creation should be redoable");
    const auto* restored = scene.FindEntity(entity.id);
    Require(restored != nullptr, "Redo should restore created entity");
    Require(restored->GetPrimitiveType() == lrender::PrimitiveType::Plane,
            "Redo should preserve the plane primitive type");
}

void TestMixedModelCreateUndoRedo()
{
    lrender::Scene scene;
    lrender::CommandHistory history;
    const auto modelId = scene.CreateModel("Mixed model").id;
    const auto solidId = scene.CreateEntity(modelId, lrender::PrimitiveType::Cube, "Solid").id;
    const std::filesystem::path assetPath = "assets/sample.obj";
    const auto meshId = scene.CreateMeshEntity(modelId, assetPath, 2, "Mesh").id;
    const lrender::Model snapshot = *scene.FindModel(modelId);
    history.PushApplied(std::make_unique<lrender::CreateModelCommand>(scene, snapshot));

    Require(scene.FindEntity(solidId)->IsSolid(), "Model should contain a solid entity");
    Require(scene.FindEntity(meshId)->IsMesh(), "Model should contain a mesh entity");
    Require(history.Undo(), "Mixed model creation should be undoable");
    Require(scene.FindModel(modelId) == nullptr, "Undo should remove the complete model");
    Require(scene.FindEntity(meshId) == nullptr, "Undo should remove nested mesh entities");
    Require(history.Redo(), "Mixed model creation should be redoable");
    const auto* restored = scene.FindEntity(meshId);
    Require(restored != nullptr, "Redo should restore nested mesh entities");
    Require(restored->Mesh()->assetPath == assetPath, "Redo should preserve the mesh asset path");
    Require(restored->Mesh()->assetEntityIndex == 2, "Redo should preserve the mesh asset entity index");
}

void TestMaterialUndoRedo()
{
    lrender::Scene scene;
    lrender::CommandHistory history;
    const auto modelId = scene.CreateModel("Model").id;
    auto& entity = scene.CreateEntity(modelId, lrender::PrimitiveType::Cube, "Material cube");
    const lrender::EntityMaterial before = entity.EffectiveMaterial();
    lrender::EntityMaterial after = before;
    after.baseColor = {0.2F, 0.4F, 0.8F, 1.0F};
    after.specularStrength = 0.8F;
    after.baseColorTexturePath = "assets/textures/test.png";
    after.useSourceTexture = false;

    history.Execute(std::make_unique<lrender::MaterialCommand>(scene, entity.id, before, after));
    Require(scene.FindEntity(entity.id)->EffectiveMaterial().NearlyEquals(after),
            "Material command should apply all material properties");
    Require(history.Undo(), "Material edit should be undoable");
    Require(scene.FindEntity(entity.id)->EffectiveMaterial().NearlyEquals(before),
            "Undo should restore the complete material snapshot");
    Require(history.Redo(), "Material edit should be redoable");
    Require(scene.FindEntity(entity.id)->EffectiveMaterial().NearlyEquals(after),
            "Redo should restore the edited material snapshot");

    history.Execute(std::make_unique<lrender::MaterialCommand>(scene, entity.id, after, entity.EntityMaterialData()));
    Require(!scene.FindEntity(entity.id)->HasMaterialOverride(),
            "Reset material command should clear the entity override");
    Require(scene.FindEntity(entity.id)->EffectiveMaterial().NearlyEquals(before),
            "Reset material command should restore the original material");
    Require(history.Undo(), "Reset material should be undoable");
    Require(scene.FindEntity(entity.id)->HasMaterialOverride() &&
                scene.FindEntity(entity.id)->EffectiveMaterial().NearlyEquals(after),
            "Undo should restore the material override");
    Require(history.Redo(), "Reset material should be redoable");
    Require(!scene.FindEntity(entity.id)->HasMaterialOverride(), "Redo should clear the material override again");
}

void TestParameterizedSolidUndoAndSavedState()
{
    lrender::Scene scene;
    lrender::CommandHistory history;
    const auto modelId = scene.CreateModel("Model").id;
    auto& entity =
        scene.CreateSolidEntity(modelId, lrender::SolidGeometry::Sphere({1.25F, 48, 24}), "Parameterized sphere");
    const auto* parameters = std::get_if<lrender::SphereParameters>(&entity.Solid()->Parameters());
    Require(parameters != nullptr, "Solid entity should retain sphere parameters");
    RequireNear(parameters->radius, 1.25F, "Sphere radius should be retained");
    Require(parameters->slices == 48 && parameters->stacks == 24, "Sphere topology parameters should be retained");

    const lrender::SolidGeometry before = *entity.Solid();
    const lrender::SolidGeometry after = lrender::SolidGeometry::Sphere({2.0F, 64, 32});
    history.Execute(std::make_unique<lrender::SolidGeometryCommand>(scene, entity.id, before, after));
    Require(history.IsModified(), "Solid edit should mark command history as modified");
    history.MarkSaved();
    Require(!history.IsModified(), "MarkSaved should establish a clean revision");

    const lrender::SolidGeometry third = lrender::SolidGeometry::Sphere({3.0F, 64, 32});
    history.Execute(std::make_unique<lrender::SolidGeometryCommand>(scene, entity.id, after, third));
    Require(history.IsModified(), "Edit after save should mark history as modified");
    Require(history.Undo(), "Solid parameter edit should be undoable");
    Require(!history.IsModified(), "Undo to saved revision should restore clean state");
    Require(entity.Solid()->NearlyEquals(after), "Undo should restore saved solid parameters");
    Require(history.Redo(), "Solid parameter edit should be redoable");
    Require(entity.Solid()->NearlyEquals(third), "Redo should restore edited solid parameters");

    RequireThrows([] { static_cast<void>(lrender::SolidGeometry::Sphere({0.0F, 32, 20})); },
                  "Zero sphere radius should be rejected");
    RequireThrows([] { static_cast<void>(lrender::SolidGeometry::Plane({{10.0F, 10.0F}, 0, 1})); },
                  "Zero plane subdivisions should be rejected");
}

} // namespace

int main()
{
    try
    {
        TestSceneLifecycle();
        TestCameraViewPresets();
        TestTransformUndoRedo();
        TestCreateUndoRedo();
        TestMixedModelCreateUndoRedo();
        TestMaterialUndoRedo();
        TestParameterizedSolidUndoAndSavedState();
        std::cout << "LRenderCoreTests: all tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "LRenderCoreTests failed: " << error.what() << '\n';
        return 1;
    }
}
