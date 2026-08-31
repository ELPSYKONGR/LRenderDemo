# 公共 Shader 与 Effect 框架

## 目标

本版本使用 `common.hlsli` 作为所有 DX11 Effect 的公共 Shader 契约。C++ 侧的 `CommonConstants.h` 必须与该文件保持字段顺序、矩阵类型和 16 字节对齐一致。

## CBuffer 约定

| 槽位 | 名称 | 更新频率 | 内容 |
|---|---|---|---|
| `b0` | `FrameInfo` | 每帧或每视口 | View、Projection、相机位置、视口信息 |
| `b1` | `ObjectInfo` | 每个实体绘制 | World、法线矩阵、WVP |
| `b2` | `MaterialInfo` | 每个材质或绘制 | 基础色、高光色、材质参数 |
| `b3` | `LightInfo` | 每帧或灯光变化 | 环境光、方向光、点光源 |

`World` 不属于 View 数据，必须放在 Object 缓冲中。这样新增天空盒、后处理或其他 Effect 时，可以复用帧数据而不绑定错误的更新频率。

## Effect 边界

`IRenderEffect` 只持有非拥有的 D3D11 Device，并提供编译产物加载辅助函数。每个派生 Effect 自己管理 Shader、InputLayout、纹理、采样器和渲染状态。`EffectFrameContext` 与 `EffectDrawContext` 是 CPU 侧只读快照，不直接拥有 GPU 资源。

普通材质的 `Texture2D`、天空盒的 `TextureCube` 和后处理的场景颜色纹理由各自的 Shader 声明；CPU 侧的 `ResourceCache` 负责复用纹理和 SamplerState。

## 当前后处理入口

`QuadViewVS.hlsl` 使用全屏三角形，不需要 VertexBuffer。`ColorProcessorPS.hlsl` 当前只做场景颜色直通采样，用于验证 RenderTarget 到全屏 Pass 的连接。后续可在同一个 Pass 边界中加入灰度、曝光、Gamma、Tonemap 和 Bloom。

## 扩展顺序

1. 将 Frame/Light 缓冲更新移动到 Renderer 的 BeginFrame。
2. 增加真正的 SkyCube Shader、TextureCube 资源和深度状态。
3. 增加 FullscreenEffect，并将场景 RenderTarget 送入后处理目标。
4. 将 `core::ViewPort` 与 `render::RenderTarget` 组合成多视口调度。
5. 在第二个图形后端出现后，再从 DX11 实现中抽取 RHI。
