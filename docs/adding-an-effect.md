# Adding A Rendering Effect

Rendering experiments should be isolated so disabling one technique does not destabilize the
editor or scene model.

## Mesh effect

Use `IRenderEffect` when a technique primarily changes shaders, constants, and pipeline state for
each mesh. `BasicMeshEffect` is the reference implementation.

1. Add `src/render/effects/MyEffect.h` and `.cpp`.
2. Implement `IRenderEffect::Bind`.
3. Own shaders, input layouts, states, and constant buffers inside the class.
4. Validate all device/context inputs and throw a contextual exception on creation failure.
5. Register the Effect in `Dx11Renderer`; expose only its learning parameters to `EditorLayer`.
6. Add a CPU-side test for parameter validation when possible.
7. Update `FILE_INDEX.md`, `CHANGELOG.md`, and this document if the boundary changes.

## Screen-space effect

SSAO, SSR, bloom, and tone mapping operate on frame resources rather than one mesh. Add a separate
`IRenderPass` boundary when the first such technique is implemented. A pass should receive an
explicit context containing input SRVs, output RTV, depth, camera matrices, and viewport size.
Do not force screen-space work into `IRenderEffect::Bind`.

## Suggested learning order

1. Unlit color Effect
2. Blinn-Phong lighting controls
3. Normal mapping
4. Shadow-map pass
5. HDR target and tone mapping
6. SSAO
7. SSR

Capture a PIX frame before and after each effect to verify resource bindings and GPU cost.
