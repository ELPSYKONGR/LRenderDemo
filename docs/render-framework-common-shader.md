# 公共 Shader 与 Effect 框架

## 目标

本版本使用 `common.hlsli` 作为所有 DX11 Effect 的公共 Shader 契约。C++ 侧的 `CommonConstantBuffers.h` 同时定义常量布局和 GPU Buffer 管理接口，必须与该文件保持字段顺序、矩阵类型和 16 字节对齐一致。

## CBuffer 约定

| 槽位 | 名称 | 更新频率 | 内容 |
|---|---|---|---|
| `b0` | `FrameInfo` | 每帧或每视口 | `C_View`、`C_Projection`、`C_CameraPosition`、`C_Viewport` |
| `b1` | `ObjectInfo` | 每个实体绘制 | `C_World`、`C_WorldInverseTranspose`、`C_WorldViewProjection` |
| `b2` | `MaterialInfo` | 每个材质或绘制 | `C_BaseColor`、`C_SpecularColor`、`C_MaterialParameters` |
| `b3` | `LightInfo` | 每帧或灯光变化 | `C_AmbientColor`、`C_Directional...`、`C_PointLightData` |

四个公共 Buffer 由 `Dx11Renderer` 的 `CommonConstantBuffers` 统一创建和拥有，每种类型各一份。
`EffectFrameContext` 只保存对该集合的非拥有引用；Frame/Light 在帧开始更新，Object/Material 在
每个网格绘制前更新。这样不同 Effect 共享同一套 `b0-b3` ABI，同时不会重复创建 GPU Buffer。

`World` 不属于 View 数据，必须放在 Object 缓冲中。这样新增天空盒、后处理或其他 Effect 时，可以复用帧数据而不绑定错误的更新频率。

## 材质数据边界

场景、编辑器、命令和导入 MeshPart 统一使用 `src/core/Material.*` 中的 `Material` 类。它只保存颜色、光照参数、显示模式、采样枚举和纹理引用，不持有 `Texture2D`、SRV 或 SamplerState，因此可以直接复制、撤销和序列化。

`src/render/MaterialManager*` 按 D3D11 Device 生命周期初始化，拥有纹理与 Sampler 缓存。绘制前由 `PrepareMaterial()` 合并 MeshPart 源材质和可选 Entity Override，生成临时 `MaterialDrawData`；其中包含统一 `Material` 快照及本次 Draw 使用的 Texture2D/SamplerState。`Dx11Renderer` 不再实现材质解析规则。

BaseColor 的来源互斥：存在有效贴图且模式为 `LitTextured` 或 `TextureOnly` 时直接使用采样颜色，否则使用 `Material::GetBaseColor()`。Entity Override 选择 `MaterialTextureSource::Source` 时，各 MeshPart 保留自己的源贴图；选择 `Custom` 时使用 Entity 指定路径。没有有效贴图时 Manager 把显示模式规范化为 `LitUntextured`，白色占位纹理不会被当作真实材质贴图。

## 公共光照函数

`common.hlsli` 中的 `EvaluateLight` 和 `CalcBlinnPhongLightColor` 不依赖具体 Pass 的局部变量。像世界坐标、表面颜色和透明度这样的输入必须通过函数参数传入，这样其他 Effect 或额外 Pass 可以复用同一套光照逻辑。`BasicMeshPS.hlsl` 负责采样纹理、计算法线和视线方向，再根据 `IfDelayedRenderMode()` 选择延迟输出或完整 Blinn-Phong 光照。

## Effect 边界

`IRenderEffect` 只持有非拥有的 D3D11 Device/Context，并提供编译产物加载辅助函数。每个派生 Effect 自己管理 Shader、InputLayout、纹理、采样器和渲染状态。二维颜色/深度目标使用 `EffectResource`，TextureCube 使用 `EffectCubeMapResource`；基类不提供字符串资源注册表。`EffectFrameContext` 与 `EffectDrawContext` 是 CPU 侧只读快照，FrameContext 只借用 Renderer 的 `CommonConstantBuffers`，不拥有 GPU Buffer。

普通材质的 `Texture2D` 和 SamplerState 由 `MaterialManager` 复用；`ResourceCache` 只缓存 MeshAsset。天空盒 TextureCube 和后处理场景颜色仍由各自 Effect 管理。

## 当前后处理入口

`QuadViewVS.hlsl` 使用全屏三角形，不需要 VertexBuffer。`ColorProcessorPS.hlsl` 当前只做场景颜色直通采样，用于验证 `EffectResource` 到全屏 Pass 的连接。`SkyVS/SkyPS` 使用位于最远深度的全屏三角形，根据逆视图投影矩阵重建世界空间观察方向并采样 TextureCube。后续可在同一个 Pass 边界中加入灰度、曝光、Gamma、Tonemap 和 Bloom。

## 扩展顺序

1. 将 Frame/Light 缓冲更新扩展为真正的 Renderer BeginFrame 阶段，并增加 Dirty 标记。
2. 将天空盒开关、旋转和资源选择暴露给 Editor。
3. 扩展 ColorProcessorEffect，将场景 `EffectResource` 送入多级后处理目标。
4. 将 `core::ViewPort` 与 `render::EffectResource` 组合成多视口调度。
5. 在第二个图形后端出现后，再从 DX11 实现中抽取 RHI。
