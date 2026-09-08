# 公共 Shader 与 Effect 框架

## 目标

本版本使用 `common.hlsli` 作为所有 DX11 Effect 的公共 Shader 契约。C++ 侧的 `CommonConstants.h` 必须与该文件保持字段顺序、矩阵类型和 16 字节对齐一致。

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

渲染层使用 `src/render/Material.h` 中的 `Material` 类。它保存已经解析完成、可以直接提交给 GPU 的颜色、光照参数、纹理和采样器，并通过 `Get...`/`Set...` 接口访问。`Material` 使用值语义，纹理和采样器由 `shared_ptr` 共享所有权。

编辑器和 `.lscene` 文件使用 `src/core/EntityMaterial.h` 中的 `EntityMaterial`。它只保存与图形 API 无关的可编辑设置，例如贴图路径、过滤模式和显示模式。`Dx11Renderer::ResolveMaterial` 在每次绘制前把实体设置合并到导入材质或默认材质，生成本次绘制使用的 `Material`。

BaseColor 的来源是互斥的：当 `Material::UsesBaseColorTexture()` 为真时，像素着色器直接使用贴图采样颜色；否则直接使用 `EntityMaterial.baseColor`。该查询完全由 `SurfaceDisplayMode` 推导：`LitTextured` 和 `TextureOnly` 表示使用贴图，`LitUntextured` 表示使用实体颜色。渲染器在没有实际贴图时会把模式规范化为 `LitUntextured`，避免白色占位纹理被误认为真实贴图。导入材质的 BaseColor 因子不会再与实体颜色或贴图颜色相乘。

## 公共光照函数

`common.hlsli` 中的 `EvaluateLight` 和 `CalcBlinnPhongLightColor` 不依赖具体 Pass 的局部变量。像世界坐标、表面颜色和透明度这样的输入必须通过函数参数传入，这样其他 Effect 或额外 Pass 可以复用同一套光照逻辑。`BasicMeshPS.hlsl` 负责采样纹理、计算法线和视线方向，再根据 `IfDelayedRenderMode()` 选择延迟输出或完整 Blinn-Phong 光照。

## Effect 边界

`IRenderEffect` 只持有非拥有的 D3D11 Device/Context，并提供编译产物加载辅助函数。每个派生 Effect 自己管理 Shader、InputLayout、纹理、采样器和渲染状态。二维颜色/深度目标使用 `EffectResource`，TextureCube 使用 `EffectCubeMapResource`；基类不提供字符串资源注册表。`EffectFrameContext` 与 `EffectDrawContext` 是 CPU 侧只读快照，FrameContext 只借用 Renderer 的 `CommonConstantBuffers`，不拥有 GPU Buffer。

普通材质的 `Texture2D`、天空盒的 `TextureCube` 和后处理的场景颜色纹理由各自的 Shader 声明；CPU 侧的 `ResourceCache` 负责复用纹理和 SamplerState。

## 当前后处理入口

`QuadViewVS.hlsl` 使用全屏三角形，不需要 VertexBuffer。`ColorProcessorPS.hlsl` 当前只做场景颜色直通采样，用于验证 `EffectResource` 到全屏 Pass 的连接。`SkyVS/SkyPS` 使用位于最远深度的全屏三角形，根据逆视图投影矩阵重建世界空间观察方向并采样 TextureCube。后续可在同一个 Pass 边界中加入灰度、曝光、Gamma、Tonemap 和 Bloom。

## 扩展顺序

1. 将 Frame/Light 缓冲更新扩展为真正的 Renderer BeginFrame 阶段，并增加 Dirty 标记。
2. 将天空盒开关、旋转和资源选择暴露给 Editor。
3. 扩展 ColorProcessorEffect，将场景 `EffectResource` 送入多级后处理目标。
4. 将 `core::ViewPort` 与 `render::EffectResource` 组合成多视口调度。
5. 在第二个图形后端出现后，再从 DX11 实现中抽取 RHI。
