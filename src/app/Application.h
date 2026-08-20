/**
 * @file Application lifetime and frame-loop coordinator.
 * @author Codex
 * @created 2026-08-20
 * @depends platform/Window.h, render/Dx11Renderer.h, editor/EditorLayer.h
 */
#pragma once

#include "commands/CommandHistory.h"
#include "core/Camera.h"
#include "core/Scene.h"
#include "editor/EditorLayer.h"
#include "platform/Window.h"
#include "render/Dx11Renderer.h"

#include <windows.h>

namespace lrender {

class Application final {
public:
    explicit Application(HINSTANCE instance) : instance_(instance) {}
    ~Application() { Shutdown(); }
    int Run();

private:
    void Initialize();
    void Shutdown() noexcept;

    HINSTANCE instance_{};
    Window window_;
    Dx11Renderer renderer_;
    Scene scene_;
    CommandHistory history_;
    Camera camera_;
    EditorLayer editor_;
    bool isImGuiInitialized_{false};
};

} // namespace lrender
