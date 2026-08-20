# LRenderDemo

LRenderDemo is a small native Windows render lab for learning Direct3D 11. It keeps the platform,
editor, scene, command, renderer, and Effect boundaries visible so individual rendering techniques
can be implemented and compared without adopting a full game engine.

## Current milestone

- Native Win32 window and D3D11 device/swap chain
- Dear ImGui docking UI with Viewport, Hierarchy, Inspector, and Tools panels
- Orbit, pan, and zoom editor camera
- Cube and UV-sphere creation
- ImGuizmo translate, rotate, and scale controls
- Undo/redo for creation and transform edits
- Procedural D3D11 vertex/index buffers
- Independent `IRenderEffect` boundary with a DirectXTK `BasicEffect` implementation
- CMake Presets for Visual Studio 2022 and lightweight CTest coverage

## Prerequisites

- Windows 10 or Windows 11
- Visual Studio 2022 with **Desktop development with C++**
- Windows 10 SDK 10.0.19041 or newer
- CMake 3.24 or newer
- Git 2.30 or newer

## Quick start

For a fresh clone, use the recursive option so dependency commits are reproduced exactly:

```powershell
git clone --recurse-submodules https://github.com/ELPSYKONGR/LRenderDemo.git
Set-Location LRenderDemo
powershell -ExecutionPolicy Bypass -File scripts/bootstrap-and-verify.ps1
```

The bootstrap script checks the toolchain, initializes missing submodules, generates the VS2022
solution, builds Debug, runs CTest, and launches the application. Its log is written under `logs/`.

## Visual Studio 2022 debugging

1. Run the bootstrap script once.
2. Open `build/vs2022/LRenderDemo.sln`.
3. Select `LRenderDemo` and `Debug | x64`.
4. Press `F5`.

The generated project sets the repository root as its debugger working directory, so logs and
future relative assets resolve consistently on every machine.

## Daily commands

```powershell
cmake --preset vs2022
cmake --build --preset vs2022-debug
ctest --preset vs2022-debug
```

Use `scripts/bootstrap-and-verify.ps1 -SkipLaunch` on build agents or when only verification is
required.

## Editor controls

| Action | Input |
|---|---|
| Orbit camera | Hold right mouse button in Viewport and drag |
| Pan camera | Hold middle mouse button in Viewport and drag |
| Zoom camera | Mouse wheel over Viewport |
| Translate / rotate / scale | `W` / `E` / `R`, or the Tools buttons |
| Undo / redo | `Ctrl+Z` / `Ctrl+Y` |
| Create primitive | `Create` menu |

## Learning route

Start in this order:

1. `src/platform/Window.cpp`: Win32 messages and visible-window lifetime.
2. `src/render/Dx11Renderer.cpp`: device, swap chain, frame targets, and draw traversal.
3. `src/render/Mesh.cpp`: immutable vertex/index buffers and indexed draw calls.
4. `src/render/BasicMeshEffect.cpp`: shader constants, input layout, and render states.
5. `src/editor/EditorLayer.cpp`: editor panels, camera input, Gizmo, and command creation.
6. `src/commands/`: reversible operations independent from the UI.

See `docs/architecture.md` and `docs/adding-an-effect.md` before adding a rendering technique.

## Repository policy

Development occurs on `dev` or `feat/*`. `main` is reserved for user-approved stable milestones.
Generated files, local environment values, logs, and credentials are excluded by `.gitignore`.
