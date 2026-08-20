# LESSONS - LRenderDemo

This file records durable architecture decisions and project-specific engineering knowledge.

## Architecture decision records

### ADR-001: Use native DX11 plus focused helper libraries

- **Date**: 2026-08-20
- **Context**: The project must teach DX11 while providing a usable editor platform.
- **Options**: rbfx, Diligent Engine, LLGL, or Win32 + DirectXTK + ImGui + ImGuizmo.
- **Decision**: Use Win32 + DirectXTK + Dear ImGui + ImGuizmo.
- **Reason**: DX11 objects and draw flow remain visible; helpers remove repetitive math, UI, and Gizmo work.
- **Consequence**: Scene and undo systems are maintained locally, but the learning surface is clearer.
- **Status**: Accepted.

### ADR-002: Delay a generic RHI until a second backend exists

- **Date**: 2026-08-20
- **Context**: A future RHI is possible, but only DX11 is currently being studied.
- **Options**: Design an RHI now or isolate native DX11 behind module boundaries.
- **Decision**: Keep native DX11 in `src/render/` and extract an RHI from proven needs later.
- **Reason**: Avoids speculative abstractions while preserving non-renderer modules.
- **Consequence**: A future backend requires a deliberate renderer refactor, not a whole-editor rewrite.
- **Status**: Accepted.

### ADR-003: Lock third-party sources as Git submodules

- **Date**: 2026-08-20
- **Context**: Builds must reproduce on other VS2022 machines.
- **Options**: CMake FetchContent branches, package manager, vendored source, or submodules.
- **Decision**: Record exact dependency commits as Git submodules.
- **Reason**: CMake stays simple and every clone resolves the same source revisions.
- **Consequence**: Clones should use `--recurse-submodules`; bootstrap repairs omitted submodules.
- **Status**: Accepted.

## Pitfalls

No resolved project-specific pitfalls yet.

## Best practices

### PRACTICE-001: Separate mesh Effects from screen-space passes

Per-object state implements `IRenderEffect`. Effects that consume frame textures should use a
future `IRenderPass` interface so resource dependencies remain explicit.

## Project conventions

### CONVENTION-001: Matrix and transform representation

Scene transforms store translation, Euler degrees, and scale for transparent editing. Matrix
composition uses DirectXTK `SimpleMath`; ImGuizmo decomposes interactive matrices back to fields.
