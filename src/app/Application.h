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
    explicit Application(HINSTANCE instance) : m_instance(instance) {}
    ~Application() { Shutdown(); }
    int Run();

private:
    void Initialize();
    void Shutdown() noexcept;

    HINSTANCE m_instance = nullptr;
    Window m_window;
    Dx11Renderer m_renderer;
    Scene m_scene;
    CommandHistory m_history;
    Camera m_camera;
    EditorLayer m_editor;
    bool m_isImGuiInitialized = false;
    bool m_isComInitialized = false;
};

} // namespace lrender
