# Effect 资源与 View 管理

本文记录当前 DX11 框架中多 Pass Effect 的资源边界、绘制入口和视图状态保护方式。

## 1. 设计目标

- 保留现有 `Texture2D` 和 `RenderTarget` 的职责与生命周期。
- 让每个 Effect 自己持有本 Effect 创建的纹理和离屏目标，避免 Renderer 保存大量临时资源。
- 将“绑定管线”和“发出绘制”区分开，为以后增加多个 Pass 留出位置。
- 在一个原生 D3D11 Context 上安全切换视图，Effect 结束后恢复调用者状态。

## 2. EffectResource

`EffectResource` 位于 `src/render/IRenderEffect.h/.cpp`，由 `IRenderEffect` 持有。它不复制 DX11 设备或 Context，只保存非拥有指针；设备和 Context 的生命周期由 Renderer 保证长于 Effect。

资源按字符串命名：

以下代码放在具体 Effect 的成员函数中（`Resources()` 是基类保护接口）：

```cpp
auto& resources = Resources();
auto& target = resources.CreateRenderTarget("blur.result", width, height);
auto texture = resources.CreateTexture("lut", "assets/lut.png");
auto* input = resources.GetShaderResource("blur.result");
```

`CreateTexture` 和 `CreateRenderTarget` 遇到重复名称会抛出异常，调用方应在应用边界记录错误。`Resize` 只调整 Effect 管理的 RenderTarget，不会修改外部共享纹理；`Clear` 释放 Effect 的全部资源。

## 3. Draw 与 Pass

`IRenderEffect::Bind(frame, draw)` 是低层管线绑定接口，便于学习和调试。新增的 `IRenderEffect::Draw(frame, draw)` 是高层入口，默认调用 `Bind`，派生 Effect 可以覆盖它并在内部完成多个 Pass。

当前 `BasicMeshEffect::Draw` 的流程是：

1. 调用 `Bind` 更新 CBuffer、Shader、材质、Sampler 和光栅化状态。
2. 从 `EffectDrawContext::MeshGeometry()` 取得 Mesh。
3. 调用 Mesh 的 `Draw` 发出索引绘制。

`ColorProcessorEffect` 使用同名的重载：

```cpp
colorProcessor.Draw(context, sceneTarget.ShaderResourceView());
```

它绑定全屏管线、设置输入 SRV，然后绘制三角形。后续增加 Bloom、ToneMapping 等效果时，可以在一个 Effect 的 `Draw` 内依次执行：绑定 Pass A 的 RenderTarget、绘制；解绑 SRV/RTV；绑定 Pass B；最后输出到目标。Pass 间资源的命名和所有权由 `EffectResource` 管理。

## 4. ViewManager 与状态恢复

`ViewManager` 保存逻辑 View 的尺寸、Camera、原生窗口句柄和离屏 RenderTarget。它采用进程内单例实例，先由 Renderer 调用 `ViewManager::Initialize(device, context)`，之后通过 `ViewManager::Instance()` 获取实例；Renderer 关闭时调用 `ViewManager::Shutdown()`。窗口句柄不归 ViewManager 所有，窗口销毁仍由 `platform::Window` 负责。一个 View 可以附加一个 HWND；多个 View 可以同时存在，Renderer 可以按 View ID 选择当前目标。

`CaptureState()` 返回 RAII 的 `ViewStateGuard`：构造时快照 D3D11 Context，析构时恢复。当前快照包括：

- VS、HS、DS、GS、PS、CS Shader；
- InputLayout、PrimitiveTopology；
- Rasterizer、Depth/Stencil、Blend 状态及参数；
- RenderTarget/DepthStencilView；
- Viewport 数组。

典型用法：

```cpp
{
    auto guard = views.CaptureState();
    views.SetDepthMode(DepthMode::ReadOnly);
    views.SetStencil(stencilDescription);
    effect.Draw(frame, draw);
} // guard 析构，Context 恢复到进入前状态
```

`SetDepthMode` 支持 Disabled、ReadOnly、ReadWrite；`SetStencil` 创建并绑定对应的模板状态。状态对象由 ComPtr 临时持有，绑定到 Context 后由 DX11 引用计数保证有效。

## 5. 当前边界与下一步

本次 ViewManager 已独立接入并可编译，但 `Dx11Renderer` 仍使用现有单窗口交换链和 `m_sceneTarget`/`m_viewportTarget`。这是刻意保留的渐进迁移边界：先学习资源与状态封装，再把每个 View 的交换链、ImGui 显示和多窗口消息路由接入 Renderer。

建议后续按以下顺序扩展：

1. 为 Effect 增加显式 `Pass` 描述（输入、输出、状态和绘制回调）。
2. 将 `ViewManager::ActiveView()` 接入 Renderer 的相机和 RenderTarget 选择。
3. 每个原生窗口拥有独立交换链时，再增加 SwapChainView 派生实现，不改变 EffectResource API。
