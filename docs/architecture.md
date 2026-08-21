# 架构说明

## 设计目标

本应用有意保持在比游戏引擎更小的规模。DX11 概念在代码中保持可见，同时编辑器和场景代码具有
足够的独立性，以便将来迁移到 RHI 时继续复用。

## 帧执行序列

```mermaid
sequenceDiagram
    participant Win32 as Win32 消息泵
    participant Editor as EditorLayer
    participant Scene as Scene 和 Commands
    participant DX11 as Dx11Renderer 和 ResourceCache
    participant UI as ImGui DX11 后端
    Win32->>Editor: 开始 ImGui 帧
    Editor->>Scene: 应用相机和变换编辑
    Editor->>DX11: 请求视口尺寸
    DX11->>DX11: 将场景渲染到离屏目标
    DX11->>UI: 使用视口 SRV 渲染面板
    DX11->>Win32: 呈现交换链
```

## 所有权关系

- `Application` 负责子系统生命周期和帧循环。
- `Window` 只负责 Win32 `HWND` 和消息状态。
- `Dx11Renderer` 负责 GPU 对象、程序化网格、资源缓存和当前启用的 Effects。
- `ResourceCache` 按规范化路径复用模型和纹理，并按描述复用 Sampler；缓存与 D3D 设备同生命周期。
- `Scene` 负责可编辑的实体数据，只保存模型路径，不持有 GPU 资源。
- `EditorLayer` 将用户交互转换为场景编辑和命令。
- `CommandHistory` 负责可逆操作，且不依赖界面。
- 每项渲染技术派生自 `IRenderEffect`，或实现为后续的渲染 Pass 类。

## RHI 迁移边界

仅学习 DX11 时，不应引入通用 RHI。将原生对象限制在 `src/render/` 中，避免它们泄漏到
`core/`、`commands/` 或 `editor/`。实现第二个后端时，再从已经验证的 DX11 操作中提取接口，
并新增 `render/rhi/` 和对应后端模块。场景和命令代码应无需改动。

## 资源生命周期

COM 资源使用 `Microsoft::WRL::ComPtr`。CPU 对象通过值语义、`std::unique_ptr` 或缓存共享所需的
`std::shared_ptr` 管理。关闭顺序依次为：UI 后端、Effect、模型/纹理缓存、程序化网格/目标、
D3D 上下文、交换链、设备、COM apartment 和窗口。
