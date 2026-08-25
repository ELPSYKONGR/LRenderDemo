/**
 * @file Lightweight behavioral tests for scene and command history.
 * @author Codex
 * @created 2026-08-20
 * @depends core/Scene.h, commands/CommandHistory.h
 */
#include "commands/CommandHistory.h"
#include "commands/CreateEntityCommand.h"
#include "commands/MaterialCommand.h"
#include "commands/TransformCommand.h"
#include "core/Camera.h"
#include "core/Scene.h"

#include <cmath>
#include <iostream>
#include <filesystem>
#include <memory>
#include <stdexcept>

namespace {

void Require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void RequireNear(float actual, float expected, const char* message) {
    Require(std::abs(actual - expected) < 0.001F, message);
}

void RequireFinite(const DirectX::SimpleMath::Matrix& matrix, const char* message) {
    const float values[]{
        matrix._11, matrix._12, matrix._13, matrix._14,
        matrix._21, matrix._22, matrix._23, matrix._24,
        matrix._31, matrix._32, matrix._33, matrix._34,
        matrix._41, matrix._42, matrix._43, matrix._44};
    for (const float value : values) {
        Require(std::isfinite(value), message);
    }
}

void TestCameraViewPresets() {
    lrender::Camera camera;
    constexpr struct ExpectedView {
        lrender::CameraViewPreset preset;
        DirectX::SimpleMath::Vector3 direction;
    } views[]{
        {lrender::CameraViewPreset::Front, {0.0F, 0.0F, 1.0F}},
        {lrender::CameraViewPreset::Back, {0.0F, 0.0F, -1.0F}},
        {lrender::CameraViewPreset::Left, {-1.0F, 0.0F, 0.0F}},
        {lrender::CameraViewPreset::Right, {1.0F, 0.0F, 0.0F}},
        {lrender::CameraViewPreset::Top, {0.0F, 1.0F, 0.0F}},
        {lrender::CameraViewPreset::Bottom, {0.0F, -1.0F, 0.0F}}};

    for (const ExpectedView& view : views) {
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

void TestSceneLifecycle() {
    lrender::Scene scene;
    const auto id = scene.CreateEntity(lrender::PrimitiveType::Cube, "Cube").id;
    Require(scene.FindEntity(id) != nullptr, "Created entity should be findable");
    Require(scene.RemoveEntity(id).has_value(), "Remove should return a snapshot");
    Require(scene.FindEntity(id) == nullptr, "Removed entity should be absent");
}

void TestTransformUndoRedo() {
    lrender::Scene scene;
    lrender::CommandHistory history;
    auto& entity = scene.CreateEntity(lrender::PrimitiveType::Sphere, "Sphere");
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

void TestCreateUndoRedo() {
    lrender::Scene scene;
    lrender::CommandHistory history;
    const auto entity = scene.CreateEntity(lrender::PrimitiveType::Plane, "Plane");
    history.PushApplied(std::make_unique<lrender::CreateEntityCommand>(scene, entity));
    Require(history.Undo(), "Creation should be undoable");
    Require(scene.FindEntity(entity.id) == nullptr, "Undo should remove created entity");
    Require(history.Redo(), "Creation should be redoable");
    const auto* restored = scene.FindEntity(entity.id);
    Require(restored != nullptr, "Redo should restore created entity");
    Require(
        restored->primitive == lrender::PrimitiveType::Plane,
        "Redo should preserve the plane primitive type");
}

void TestModelCreateUndoRedo() {
    lrender::Scene scene;
    lrender::CommandHistory history;
    const std::filesystem::path modelPath = "assets/sample.glb";
    const auto entity = scene.CreateModelEntity(modelPath, "Sample model");
    history.PushApplied(std::make_unique<lrender::CreateEntityCommand>(scene, entity));

    Require(entity.IsModel(), "Model entity should report its resource type");
    Require(history.Undo(), "Model creation should be undoable");
    Require(history.Redo(), "Model creation should be redoable");
    const auto* restored = scene.FindEntity(entity.id);
    Require(restored != nullptr, "Redo should restore the model entity");
    Require(restored->modelPath == modelPath, "Redo should preserve the model path");
}

void TestMaterialUndoRedo() {
    lrender::Scene scene;
    lrender::CommandHistory history;
    auto& entity = scene.CreateEntity(lrender::PrimitiveType::Cube, "Material cube");
    const lrender::EntityMaterial before = entity.material;
    lrender::EntityMaterial after = before;
    after.baseColor = {0.2F, 0.4F, 0.8F, 1.0F};
    after.specularStrength = 0.8F;
    after.baseColorTexturePath = "assets/textures/test.png";
    after.useSourceTexture = false;

    history.Execute(std::make_unique<lrender::MaterialCommand>(
        scene, entity.id, before, after));
    Require(
        scene.FindEntity(entity.id)->material.NearlyEquals(after),
        "Material command should apply all material properties");
    Require(history.Undo(), "Material edit should be undoable");
    Require(
        scene.FindEntity(entity.id)->material.NearlyEquals(before),
        "Undo should restore the complete material snapshot");
    Require(history.Redo(), "Material edit should be redoable");
    Require(
        scene.FindEntity(entity.id)->material.NearlyEquals(after),
        "Redo should restore the edited material snapshot");
}

} // namespace

int main() {
    try {
        TestSceneLifecycle();
        TestCameraViewPresets();
        TestTransformUndoRedo();
        TestCreateUndoRedo();
        TestModelCreateUndoRedo();
        TestMaterialUndoRedo();
        std::cout << "LRenderCoreTests: all tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "LRenderCoreTests failed: " << error.what() << '\n';
        return 1;
    }
}
