/**
 * @file Lightweight behavioral tests for scene and command history.
 * @author Codex
 * @created 2026-08-20
 * @depends core/Scene.h, commands/CommandHistory.h
 */
#include "commands/CommandHistory.h"
#include "commands/CreateEntityCommand.h"
#include "commands/TransformCommand.h"
#include "core/Scene.h"

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

} // namespace

int main() {
    try {
        TestSceneLifecycle();
        TestTransformUndoRedo();
        TestCreateUndoRedo();
        TestModelCreateUndoRedo();
        std::cout << "LRenderCoreTests: all tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "LRenderCoreTests failed: " << error.what() << '\n';
        return 1;
    }
}
