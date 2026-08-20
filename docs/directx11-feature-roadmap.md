# DirectX11 教程功能差距与实现路线

本文将 LRenderDemo 与本机的 `DirectX11-With-Windows-SDK-master` 教程工程逐项对照，并给出适合
编辑器型学习平台的实现顺序。对照基于实际源码和构建目标，不把“已有设计文档或素材”记为已实现。

## 1. 结论

LRenderDemo 不是教程章节的线性复刻。它的平台能力已经超过早期示例：具备原生窗口、ImGui
编辑器、离屏视口、场景实体、相机、Gizmo、撤销/重做和可编辑 HLSL；但渲染效果仍处于基础阶段。

- 38 个教程示例中，能力等价或被更高层实现覆盖的约 6 项。
- 已有基础设施但未完成对应效果的约 6 项。
- 尚未实现的约 26 项。
- 按渲染能力看，当前大致处于第 10～14 章的骨架阶段。
- 第 9 章纹理映射和第 19 章模型/材质导入是当前最重要的断点。
- `RenderTarget` 虽涉及第 24 章所需技术，但目前只服务编辑器视口，不能视为完整的 Render To Texture 效果。

因此下一步不应直接跳到 SSAO、延迟渲染或 RHI。先建立纹理、材质、模型和 Pass 资源边界，后续
效果才能共享稳定的数据结构，而不是每个示例各写一套资源加载代码。

## 2. 当前已经具备的基础

| 能力 | LRenderDemo 现状 | 主要代码 |
|---|---|---|
| Win32 与 DX11 初始化 | 已完成，包含交换链 resize 和 Debug Layer 回退 | `platform/Window.*`、`render/Dx11Renderer.*` |
| 可见编辑器界面 | 已完成，包含停靠、视口、层级、检查器和工具栏 | `editor/EditorLayer.*` |
| 场景与相机 | 已完成基础实体、环绕/平移/缩放相机 | `core/Scene.*`、`core/Camera.*` |
| 基础几何 | 已完成立方体和 UV 球体程序化生成 | `render/PrimitiveFactory.*` |
| 基础光照 | 已完成单方向光 Lambert 漫反射，无材质纹理和高光 | `render/BasicMeshEffect.*` |
| Shader 工作流 | 已完成 VS 工程显示、FXC 增量构建和 CSO 加载 | `src/shaders/`、`src/CMakeLists.txt` |
| 深度缓冲 | 已有视口 DSV 和默认深度测试 | `render/RenderTarget.*` |
| 离屏渲染 | 场景渲染到纹理，再由 ImGui 采样显示 | `RenderTarget`、`RenderEditor()` |
| 编辑操作 | 已有 Gizmo、创建实体、撤销和重做 | `editor/`、`commands/` |

## 3. 教程逐项对照

“完成”表示核心能力已存在；“部分”表示只有底层资源或简化版本；“未实现”表示产品代码中没有
对应管线。教程没有 04、05、18 目录，下面按实际 38 个示例列出。

| 教程 | 状态 | 对照说明 |
|---|---|---|
| 01 DirectX11 Initialization | 完成 | 已创建 Device、Context、SwapChain、RTV，并处理窗口 resize。 |
| 02 Rendering a Triangle | 完成 | 未保留独立三角形示例，但当前索引网格绘制已覆盖其 IA/VS/PS/Draw 能力。 |
| 03 Rendering a Cube | 完成 | 程序化立方体、顶点/索引缓冲和场景绘制均已存在。 |
| 06 Use ImGui | 完成 | 已形成多面板编辑器，而非单一 ImGui 示例。 |
| 07 Lighting | 部分 | 只有单方向 Lambert 光；缺少多光源、镜面反射、材质参数和可视化灯光。 |
| 08 Direct2D and Direct3D Interoperability | 未实现 | 未接入 Direct2D/DirectWrite；当前 UI 文本由 ImGui 负责。 |
| 09 Texture Mapping | 未实现 | 顶点没有 UV，Effect 没有 SRV/Sampler，Scene 没有材质。 |
| 10 Camera | 完成 | 已有透视相机和编辑器环绕、平移、缩放输入。 |
| 11 Blending | 未实现 | 产品渲染路径没有自有 BlendState 和透明物排序。 |
| 12 Depth and Stenciling | 部分 | 有 DSV 和默认深度测试；没有模板写入、遮罩、镜面或轮廓实验。 |
| 13 Living Without FX11 | 完成 | 项目使用自有 HLSL、常量缓冲和原生 DX11 Shader 对象，不依赖 FX11。 |
| 14 Depth Test | 部分 | 基础深度测试有效；没有可切换比较函数、深度可视化和反向 Z。 |
| 15 Geometry Shader Beginning | 未实现 | 没有 GS 阶段和相关 Shader 管理。 |
| 16 Stream Output | 未实现 | 没有 Stream Output buffer、声明和多阶段更新。 |
| 17 Tree Billboard | 未实现 | 没有 Billboard、纹理数组和 Alpha-to-Coverage。 |
| 19 Meshes | 部分 | 有 `Mesh` 和程序化几何；没有模型导入、子网格、材质或 GPU 资源缓存。 |
| 20 Instancing and Frustum Culling | 未实现 | 没有实例缓冲、包围体、视锥测试和 `DrawIndexedInstanced`。 |
| 21 Picking | 未实现 | 只能在层级面板选实体；没有视口射线、包围体/三角形求交。 |
| 22 Static Cube Mapping | 未实现 | 已有详细设计文档和 cubemap 素材，但没有 `SkyboxPass/SkyboxEffect` 产品代码。 |
| 23 Dynamic Cube Mapping | 未实现 | 没有 cubemap 六面 RTV、逐面相机和反射探针更新。 |
| 24 Render To Texture | 部分 | 已有离屏视口 RTV/SRV；缺少显式 Pass 输入输出和将结果用于场景材质的示例。 |
| 25 Normal Mapping | 未实现 | 顶点无 tangent，材质无 normal texture，Shader 无 TBN。 |
| 26 Compute Shader Beginning | 未实现 | 没有 CS、UAV、Dispatch 和资源屏障式解绑约定。 |
| 27 Bitonic Sort | 未实现 | 没有 GPU 排序数据结构或 Compute Pass。 |
| 28 Waves | 未实现 | 没有波面模拟、动态顶点或 Compute 更新。 |
| 29 OIT | 未实现 | 没有透明链表、UAV counter 或顺序无关透明合成。 |
| 30 Blur and Sobel | 未实现 | 没有全屏三角形、Ping-Pong 纹理和后处理 Pass。 |
| 31 Shadow Mapping | 未实现 | 没有光源相机、Shadow DSV/SRV、深度偏移和 PCF。 |
| 32 SSAO | 未实现 | 没有可采样深度/法线缓冲、随机核、AO 与模糊 Pass。 |
| 33 Tessellation | 未实现 | 没有 HS/DS、Patch topology 和细分参数。 |
| 34 Displacement Mapping | 未实现 | 没有高度纹理、切线空间或 Tessellation 位移。 |
| 35 Particle System | 未实现 | 没有粒子池、GPU 更新、Billboard 和混合管线。 |
| 36 Deferred Rendering | 未实现 | 没有 GBuffer、多渲染目标和光照合成。 |
| 37 Tile-Based Deferred Rendering | 未实现 | 没有屏幕分块、光源列表和 Compute 光照剔除。 |
| 38 Cascaded Shadow Mapping | 未实现 | 没有级联划分、稳定投影和级联选择。 |
| 39 VSM and ESM | 未实现 | 没有阴影矩、指数阴影和阴影滤波链。 |
| 40 FXAA | 未实现 | 没有 Luma 输入和全屏 FXAA Pass。 |
| Archive Mouse and Keyboard | 部分 | 已有 Win32/ImGui 输入、相机和快捷键，但没有独立输入映射层。 |

## 4. 推荐实现顺序

教程编号适合逐章学习，却不完全适合持续扩展的编辑器。建议按依赖关系分成以下里程碑。

| 顺序 | 里程碑 | 对应教程 | 为什么现在做 | 完成标准 | 推荐资产 |
|---:|---|---|---|---|---|
| 1 | Texture2D、Sampler、UV 和 Material | 09 | 所有后续材质效果的共同入口 | 立方体和 Suzanne 可显示 BaseColor；sRGB 正确 | Suzanne |
| 2 | glTF 模型导入与资源缓存 | 19 | 当前只能画程序化几何，测试场景无法进入平台 | 支持 glTF/GLB、子网格、索引、节点变换和基础材质 | Suzanne、Bunny、Damaged Helmet |
| 3 | 完整基础光照与渲染状态 | 07、11、12、14 | 建立材质、光源、混合、深度和模板的可控基线 | 方向/点/聚光、Blinn-Phong、透明排序、Stencil 示例 | Suzanne、Sponza |
| 4 | 编辑器拾取 | 21 | 对工具平台的收益高于视觉特效 | 鼠标射线先测包围体，再测三角形，并与层级选择同步 | Bunny、Suzanne |
| 5 | 实例化、包围体与视锥剔除 | 20 | 为复杂场景和后续性能比较建立统计基线 | 可切换剔除，显示提交/剔除数量，支持实例绘制 | 大量 Bunny/Suzanne |
| 6 | `IRenderPass` 与静态天空盒 | 22 | 第一个场景级 Pass，用于验证资源依赖和状态恢复 | 天空盒正确处理相机平移、深度和 cubemap | `cubemap.dds` |
| 7 | Tangent 与法线贴图 | 25 | 在进入复杂光照前验证顶点格式和材质扩展性 | TBN 正确，可切换 normal map，并有错误纹理回退 | Damaged Helmet |
| 8 | Shadow Mapping | 31 | 最重要的多 Pass 基础，能暴露状态和资源生命周期问题 | 深度 Pass、PCF、Bias UI、ShadowMap 可视化 | Sponza、Suzanne |
| 9 | 全屏后处理框架 | 24、30、40 | 统一全屏三角形、Ping-Pong 资源和 Pass 调度 | 完成灰度、Blur、Sobel、FXAA，可独立启停和排序 | Sponza |
| 10 | SSAO | 32 | 复用深度/法线和后处理框架，验证屏幕空间技术 | AO、双边模糊、半分辨率选项和调试视图 | Sponza |
| 11 | Deferred、Tile Lighting 与高级阴影 | 36～39 | 需要前面所有资源/Pass 能力，不宜提前实现 | GBuffer 可视化、光源压力测试、CSM/VSM/ESM 对比 | Sponza、Metal-Rough Spheres |
| 12 | 实验性专题 | 15～17、23、26～29、33～35 | 相互依赖较少，适合作为独立学习分支 | 每项独立 Pass/Effect，可禁用且不污染主路径 | 按专题选择或程序化生成 |

## 5. 关键架构建议

### 5.1 模型导入

优先支持 glTF 2.0，而不是围绕旧 OBJ 建立长期材质格式。可选择固定版本的 `cgltf`（C、单头文件）
解析 glTF/GLB，使用 DirectXTK WIC/DDS loader 创建纹理。这样既保留数据处理的学习价值，又避免
Assimp 将场景转换细节全部隐藏。OBJ/OFF 可作为简单几何导入练习，不应成为主资产格式。

CPU 侧建议拆分为 `MeshData`、`MaterialData`、`ModelAsset`，GPU 侧继续由 `render/` 创建
VertexBuffer、IndexBuffer、SRV 和 Sampler。不要让 `core/Scene` 持有 `ID3D11*`。

### 5.2 Effect 与 Pass

- 单个网格如何着色：实现 `IRenderEffect`，例如 Unlit、BlinnPhong、NormalMapped。
- 一帧在何时渲染什么资源：实现 `IRenderPass`，例如 Skybox、Shadow、SSAO、PostProcess。
- `Dx11Renderer` 负责调度，Pass 显式声明输入 SRV、输出 RTV/DSV 和视口。
- 在真正添加第二个图形后端前，不设计通用 RHI；先让 DX11 Pass API 经受多个效果验证。

### 5.3 每个里程碑都应提供的学习开关

每个效果至少应提供启用/禁用、关键参数、输入/中间结果调试视图和 GPU/CPU 统计。只有最终画面
而没有中间资源可视化，不利于定位矩阵、格式、色彩空间和状态泄漏问题。

## 6. 可以后置或跳过的教程内容

- Direct2D 互操作：除非要学习 DirectWrite 或必须在 DX11 纹理上绘制矢量 UI，否则 ImGui 已满足平台需求。
- Stream Output：现代实现更常用 Compute Shader；可为理解历史 DX11 管线单独学习，但不应成为粒子系统前置。
- Bitonic Sort、OIT、Tile Deferred：属于高级专题，应在资源解绑、UAV 和 GPU 调试流程稳定后实现。
- Tessellation/Displacement：先完成普通法线贴图和阴影，否则很难判断位移后的法线与阴影错误来自哪里。
- Dynamic Cube Mapping：更新成本高，应在静态天空盒和标准反射材质稳定后再实现。

## 7. 下一项建议

下一次实现建议选择“Texture2D + Material + UV”，先让程序化立方体显示一张 sRGB 纹理，再加载
Suzanne 的 BaseColor。不要一开始就加载 Sponza；它的节点、材质和纹理数量会让导入器问题与
渲染问题混在一起。Suzanne 完成后，再用 Damaged Helmet 验证 GLB 内嵌资源，最后进入 Sponza。
