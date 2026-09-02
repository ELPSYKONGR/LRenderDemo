# CHANGELOG - LRenderDemo

## 2026-09-02 统一成员默认初始化格式

- 遍历 `src/` 和 `tests/` 头文件，将类与结构体成员的默认初始化统一为 `= ...` 形式。
- 空值初始化使用 `= {}`，指针初始化使用 `= nullptr`，不改变运行时行为。
- 保留局部变量、聚合构造和着色器资源布局中的必要花括号。

## 2026-09-01 物体法线 RenderTarget 输出

- 场景物体绘制启用颜色和法线双 RenderTarget，法线与场景颜色共用深度缓冲。
- `BasicMeshPS.hlsl` 新增 `SV_TARGET1`，输出编码到 `[0,1]` 的世界空间法线。
- `Dx11Renderer::NormalTarget()` 提供法线纹理的只读访问，便于后续调试视图和后处理使用。
- `RenderTarget::BindAndClear` 支持可选附加颜色目标，并检查尺寸兼容性。

## 2026-09-01 Entity 材质基础值与覆盖值分离

- `Entity` 新增 `m_entityMaterial`、`m_overrideEntityMaterial` 和覆盖状态，提供
  `EntityMaterialData()`、`EditableMaterial()`、`EffectiveMaterial()` 等访问接口。
- Solid 创建时使用默认基础材质，Mesh 创建时默认使用导入源材质；Renderer 保留底层
  `ResolveMaterial(source, settings)`，并按 MeshPart 或默认资源选择 GPU 材质源。
- 材质编辑与撤销命令写入覆盖材质；覆盖值恢复到基础值时自动清除覆盖状态。
- 统一新增材质默认初始化为 `=` 形式，Effect Context 构造调用使用圆括号。
- Inspector 材质面板新增 `Reset` 控件，可通过撤销命令恢复 Entity 的原始材质。

## 2026-08-31 统一直接启动 exe 的资源布局

- 保留源码中的 `assets/...` 相对资源引用，不再在程序入口切换当前目录。
- CMake 构建后将 `assets` 拷贝到 exe 同级目录，修复双击 Debug 目录下 exe 时 Suzanne 等导入模型无法加载的问题。

## 2026-08-31 统一 VS 调试与直接启动的 ImGui 布局

- ImGui 配置文件改为固定写入 `LRenderDemo.exe` 所在目录，不再依赖当前工作目录。
- 修复 VS2022 调试使用工程根目录 `imgui.ini`、直接启动使用 Debug 输出目录 `imgui.ini` 导致布局不一致的问题。

## 2026-08-31 修复用户改名后的 RenderTarget API，并将 ViewManager 改为单例实例

- 统一 `RenderTarget` 的 `GetWidth`、`GetHeight`、`GetShaderResourceView` 调用点。
- `ViewManager` 新增静态 `Initialize`、`Shutdown`、`Instance`，由唯一实例保存 DX11 设备、Context 和 View 状态。

## 2026-08-31 修复平面覆盖立方体和球

- `BasicMeshEffect::Bind` 每次绘制明确绑定 `DepthDefault`。
- 修复全屏后处理使用 `DepthNone` 后，深度状态泄漏到下一帧场景绘制，导致最后绘制的平面覆盖其他物体的问题。

## 2026-08-31 Effect 资源管理与 View 状态保护

- 新增 `EffectResource`，由 `IRenderEffect` 管理命名 Texture2D、RenderTarget 和 ShaderResourceView。
- 新增 `IRenderEffect::Draw` 高层入口；BasicMeshEffect 在 Draw 内完成绑定和 Mesh 绘制，ColorProcessorEffect 将 Apply 改名为 Draw。
- 新增 `ViewManager` 与 `ViewStateGuard`，支持逻辑多 View、深度写入模式、模板状态以及 DX11 各 Shader 阶段和管线状态的 RAII 恢复。
- 当前 Renderer 保持单交换链，后续可以逐步接入 ActiveView 和多窗口交换链。

## 2026-08-31 公共 Shader 契约与 Effect 基类

- 将 `BasicMeshConstants.hlsli` 迁移为 `common.hlsli`，拆分 `FrameInfo`、`ObjectInfo`、`MaterialInfo`、`LightInfo` 四类 CBuffer。
- 新增 `CommonConstants.h`，让 C++ 与 HLSL 布局使用 `static_assert` 校验。
- 修正 `IRenderEffect` 构造/析构边界，统一类成员使用 `m_` 前缀。
- 修正 `ViewPort` 逻辑对象，并将全屏 Quad 与颜色处理 Shader 纳入 VS2022/CMake 构建。

## 近期变更

| 时间 | 类型 | 摘要 | 模块 | 提交 |
|---|---|---|---|---|
| 08-26 | 功能 | 增加参数化 Solid 创建/实时编辑和版本化场景保存 | core、commands、render、editor、persistence、tests | 本次提交 |
| 08-26 | 重构 | 用 Frame/Draw Context 收敛 Effect Bind 参数并集中生成快照 | render、docs | 本次提交 |
| 08-26 | 重构 | 抽离类型化 DX11 常量缓冲并统一 BasicMesh VS/PS 常量布局 | render、shaders、tests、docs | 本次提交 |
| 08-25 | 功能 | 增加 Scene/Model/Entity 层级、混合 Solid/Mesh 绘制和 OBJ/MTL 导入 | core、commands、editor、render、tests | 本次提交 |
| 08-25 | 功能 | 增加实体材质编辑、贴图选择和显示控制 | core、commands、editor、render、shaders | 本次提交 |
| 08-25 | 功能 | 增加八个相机视角和自动旋转控制 | core、editor | 本次提交 |
| 08-25 | 功能 | 增加可创建的水平 Plane 基础几何 | core、render、editor、app | 本次提交 |
| 08-25 | 工程 | 在 VS2022 工程中显示全部项目头文件 | cmake | 本次提交 |
| 08-21 | 功能 | 增加纹理材质、glTF/GLB 缓存导入与可编辑多光源 | core、render、editor、shaders | 工作区 |
| 08-20 | 文档 | 增加 DX11 教程差距分析、实施路线和素材映射 | docs、assets、scripts | 工作区 |

---

## [2026-08-26] 参数化 Solid 与场景保存

- **文件**：`src/core/SolidGeometry.*`、`src/render/PrimitiveFactory.*`、
  `src/render/SolidMeshCache.*`、`src/commands/SolidGeometryCommand.*`、
  `src/persistence/SceneSerializer.*`、`src/editor/EditorSolid.cpp`、`EditorScene.cpp`。
- **功能**：Cube 支持三轴尺寸，Sphere 支持半径/经纬分段，Plane 支持尺寸/XZ 细分；创建弹窗和
  Inspector 可编辑参数，连续拖动只记录一次撤销。`File` 菜单支持新建、打开、保存、另存为和关闭前
  未保存确认。
- **运行时边界**：Scene 只保存 API 无关参数；`SolidMeshCache` 按 Entity 替换当前 CPU 生成的 DX11
  Mesh，不保存历史参数 Mesh。未来改为 GPU 生成时无需修改 Scene 和文件格式。
- **文件边界**：`.lscene` v1 使用 JSON 保存层级、几何、Transform、材质和相对资源引用；先写临时
  文件再替换，加载和资源预加载成功后才替换当前 Scene。相机、灯光、GPU 缓存和资源打包不在本期。
- **依赖**：Git 子模块锁定官方 `nlohmann/json` v3.11.3（提交 `9cca280`）。
- **验证**：VS2022 Debug 构建和四个 CTest 目标通过；包含 Core 参数/保存点、WARP Mesh 更新和
  `.lscene` 往返/版本拒绝测试。
- **提交**：本次提交

## [2026-08-26] 用 Frame/Draw Context 收敛 Effect Bind 参数

- **文件**：`src/render/EffectContext.*`、`src/render/IRenderEffect.h`、
  `src/render/BasicMeshEffect.*`、`src/render/Dx11Renderer.cpp`
- **范围**：逐网格 Effect 的调用协议和参数生命周期。
- **改动**：新增并列的 `EffectFrameContext final` 与 `EffectDrawContext final`；前者每帧从 Camera
  生成 View/Projection/Position 快照并校验 D3D11 Context，后者从 Entity 生成 World/Tint/Selected
  快照并按值持有当前 MeshPart 已解析的 GPU Material。`Bind()` 从八个参数缩减为 `frame/draw` 两个。
- **边界**：两个 Context 不继承共同父类，因为它们不能互相替换；Camera/Entity 只在 Context 构造
  边界出现，Effect 仍消费稳定的渲染快照。Shader、cbuffer、Scene 和资源缓存行为不变。
- **验证**：VS2022 Debug 构建、CTest 和可见窗口运行验证。
- **提交**：本次提交

## [2026-08-26] 抽离 DX11 常量缓冲与 BasicMesh 布局

- **文件**：`src/render/Dx11ConstantBuffer.h`、`src/render/BasicMeshConstants.h`、
  `src/render/BasicMeshEffect.*`、`src/shaders/BasicMeshConstants.hlsli`、`tests/ImportTests.cpp`
- **范围**：Effect 内的常量缓冲资源操作，以及 C++/HLSL 常量布局边界。
- **改动**：新增 `Dx11ConstantBuffer<T>`，统一执行 16 字节/容量编译期检查、`ComPtr` 所有权、
  `UpdateSubresource` 和 VS/PS 槽位绑定；`BasicMeshEffect` 继续负责常量语义和数据组装，只把 GPU
  缓冲机械操作委托给封装；VS/PS 通过同一个 `.hlsli` 使用原有 `b0` 布局。
- **边界**：没有拆分 Frame/Object/Material 缓冲，没有修改 `IRenderEffect::Bind`、渲染器调用顺序
  或场景/材质/资源接口。等第二个正式网格 Effect 或多 Pass 共享数据出现后再评估按更新频率拆分。
- **验证**：VS2022 Debug 构建、HLSL include 依赖构建和 WARP 常量缓冲绑定测试。
- **提交**：本次提交

## [2026-08-25] 增加模型实体层级与 OBJ/MTL 导入

- **文件**：`src/core/Scene.*`、`src/commands/CreateModelCommand.*`、`src/render/MeshAsset.h`、
  `src/render/IModelImporter.h`、`src/render/ModelLoader.*`、`src/render/ObjLoader.*`、
  `src/render/MeshImportUtils.*`、`src/editor/EditorHierarchy.cpp`、`tests/ImportTests.cpp`
- **范围**：场景所有权、混合实体绘制、格式扩展边界、OBJ/MTL 导入和资源缓存。
- **改动**：`Scene` 管理 `Model`，`Model` 管理多个 `Entity`；同一个 Model 可同时容纳程序化
  Solid Entity 与导入的 Mesh Entity。新增 `PrimitiveType::Mesh` 和独立几何描述；把渲染资源类型
  从易混淆的 `Model` 改为 `MeshAsset`；通过 `IModelImporter` 和 `ModelLoader` 注册 glTF 与 OBJ 导入器。
- **OBJ 边界**：支持多 Shape、多材质、Position/Normal/UV、缺失法线重建，以及 MTL 的 `Kd`、
  `Ks`、`Ns`、透明度和 `map_Kd`。导入时集中完成手性、绕序和 UV V 方向转换。
- **扩展性**：后续接入 Assimp 时新增 `AssimpImporter` 并在 `ModelLoader` 注册目标扩展名即可，
  `Scene`、编辑器、渲染遍历和资源缓存接口无需随格式改变。
- **验证**：VS2022 Debug 构建通过；`LRenderCoreTests` 和 WARP 支持的 `LRenderImportTests` 通过。
- **依赖**：tinyobjloader 固定到 v2.0.0rc13（提交 `2945a96`）。
- **提交**：本次提交

## [2026-08-25] 增加实体材质编辑与贴图控制

- **文件**：`src/core/EntityMaterial.h`、`src/commands/MaterialCommand.*`、
  `src/editor/EditorMaterial.cpp`、`src/render/Dx11Renderer.*`、`src/render/BasicMeshEffect.*`、
  `src/shaders/BasicMesh*.hlsl`
- **范围**：实体材质属性、撤销/重做、BaseColor 贴图选择、Sampler 和显示模式
- **改动**：把可序列化材质参数作为 `Entity::material` 保存；支持基础色、漫反射强度、高光颜色/
  强度/指数、双面状态、Point/Linear/Anisotropic 过滤及 Wrap/Clamp/Mirror 寻址；支持贴图缩略图、
  Windows 原生选择器、恢复模型/程序化几何源贴图，以及带光照贴图、仅贴图、带光照无贴图三种模式。
- **边界**：导入资产各 `MeshPart` 的源材质和 GPU 资源仍由渲染层缓存；绘制时复制并应用实体参数，
  不修改共享缓存。当前仅编辑 Blinn-Phong 与 BaseColor，不包含法线、金属粗糙度或透明混合。
- **验证**：VS2022 Debug 构建和 CTest 通过；材质完整快照支持撤销/重做。
- **提交**：本次提交

## [2026-08-25] 增加相机视角预设和自动旋转

- **文件**：`src/core/Camera.*`、`src/editor/EditorCamera.cpp`、`src/editor/EditorLayer.*`、
  `tests/CoreTests.cpp`
- **区块**：相机朝向、View Matrix、Camera 控制面板和相机回归测试
- **改动**：增加正、背、左、右、顶、底、右轴测和左轴测八个透视视角预设；增加围绕观察目标
  的自动旋转开关、顺/逆时针方向与每秒角速度；顶/底视图使用非共线 up 向量保持矩阵有效。
- **原因**：让学习者快速对比模型各方向结构，并连续观察材质和多光源效果。
- **提交**：本次提交

## [2026-08-25] 增加 Plane 基础几何

- **文件**：`src/core/Scene.h`、`src/render/PrimitiveFactory.*`、`src/render/Dx11Renderer.*`、
  `src/editor/EditorLayer.cpp`、`src/app/Application.cpp`、`tests/CoreTests.cpp`
- **区块**：`PrimitiveType`、基础网格创建、场景绘制选择与 `Create` 菜单
- **改动**：增加带向上法线和平铺 UV 的 10x10 水平 Plane；支持菜单创建、撤销/重做，并在默认
  场景的 `Y=-0.5` 位置创建平面，使现有对象位于其上方。
- **原因**：提供观察纹理平铺、光照和后续阴影效果所需的基础承接面。
- **提交**：本次提交

## [2026-08-25] 在 VS2022 工程中显示项目头文件

- **文件**：`src/CMakeLists.txt`
- **区块**：`LRenderCore`、`LRenderDemo` 源文件清单与 `source_group`
- **改动**：将 `src/` 下 24 个项目头文件显式加入所属 CMake target，并按
  `Header Files/commands`、`core`、`render` 等模块生成 VS 筛选器。
- **原因**：此前 CMake 仅登记 `.cpp` 和 HLSL，VS2022 解决方案中无法稳定浏览项目头文件。
- **提交**：本次提交

## [2026-08-21] 增加纹理材质、模型导入和多光源

- **文件**：`src/render/Texture2D.*`、`SamplerState.*`、`Material.h`、`Model.h`、
  `GltfLoader.*`、`ResourceCache.*`、`Lighting.h`、`BasicMeshEffect.*`、`src/shaders/`、
  `src/editor/EditorAssets.cpp`、`src/core/Scene.*`
- **范围**：基础资源系统、静态模型管线、场景实体和编辑器控制
- **变更**：为顶点增加 UV 并切换到 32 位索引；程序化几何使用棋盘纹理；固定版本 `cgltf`
  导入 glTF/GLB、外部/内嵌 BaseColor 图片、节点变换和子网格；按路径缓存模型/纹理并缓存 Sampler；
  增加方向光、四盏点光、Lambert 与 Blinn-Phong；通过原生文件对话框导入模型并显示缓存统计。
- **原因**：建立后续法线贴图、PBR、阴影和场景级 Pass 能共享的最小资源边界。
- **验证**：VS2022 Debug 构建和 CTest 通过；运行时验证 Suzanne glTF 外部纹理、Damaged Helmet
  GLB 内嵌纹理，以及程序化几何的 UV/多光源输出。
- **提交**：工作区

## [2026-08-20] 增加 DX11 功能差距路线与素材映射

- **文件**：`docs/directx11-feature-roadmap.md`、`assets/learning-roadmap/README.md`、
  `scripts/verify-learning-assets.ps1`
- **范围**：学习路线、测试素材和验证工具
- **变更**：逐项对照 `DirectX11-With-Windows-SDK-master` 的 38 个示例，记录已完成、部分完成和
  未实现能力；按资源依赖给出 12 个实现里程碑，并把 Bunny、Suzanne、Sponza、Damaged Helmet、
  Metal-Rough Spheres 和 cubemap 映射到对应阶段。
- **原因**：避免按教程编号盲目移植效果，并确保后续开发使用已经下载、可校验且用途明确的素材。
- **验证**：85 个素材文件通过 SHA-256 校验，0 个缺失或损坏。
- **提交**：工作区

## [2026-08-20] 接入项目自有 Shader 构建管线

- **文件**：`src/shaders/BasicMeshVS.hlsl`、`src/shaders/BasicMeshPS.hlsl`、
  `src/render/BasicMeshEffect.*`、`src/CMakeLists.txt`
- **范围**：基础网格 Effect 和 VS2022 构建流程
- **变更**：用项目自有的 VS/PS 替代 DirectXTK 内置预编译 Shader；HLSL 显示在 VS 的
  `Shaders` 筛选器中，由 FXC 按 Debug/Release 分目录生成 CSO，Effect 加载当前构建产物。
- **原因**：保证学习者修改 HLSL 后通过正常构建即可观察最终效果，无需修改第三方库或手工复制文件。
- **提交**：工作区

## [2026-08-20] 增加天空盒 Pass 教学示例与素材

- **文件**：`docs/examples/skybox-pass.md`、`assets/skyboxes/`、
  `scripts/download-skybox-assets.ps1`
- **范围**：渲染架构学习示例与第三方素材
- **变更**：增加固定版本的 DirectXTKTest cubemap DDS 下载流程，以及覆盖 `IRenderPass`、
  `SkyboxPass`、`SkyboxEffect`、HLSL、DX11 状态、编辑器接入和验证方法的详细中文文档。
- **原因**：用一个完整例子说明如何在不滥用逐网格 `IRenderEffect` 的前提下扩展场景级渲染功能。
- **提交**：本次提交

## [2026-08-20] 增加经典图形学测试模型下载集

- **文件**：`assets/test-scenes/`、`scripts/download-test-scenes.ps1`、`.gitignore`
- **范围**：学习资产和可复现下载流程
- **变更**：增加 Stanford Bunny、Suzanne、Sponza、Metal/Roughness Spheres 和 Damaged Helmet 的集中下载、
  来源说明、许可入口及 SHA-256 清单。
- **原因**：提供覆盖几何、光照、大型场景和 PBR 材质的经典测试数据，同时避免将第三方大文件
  直接提交到 Git 历史。
- **提交**：本次提交

## [2026-08-20] 项目文档中文化

- **文件**：`README.md`、项目管理文档、`docs/`
- **范围**：面向开发者和学习者的项目文档
- **变更**：将仓库内的 Markdown 文档统一改为中文，同时保留代码标识符、命令和路径原文。
- **原因**：降低项目学习和后续维护的语言门槛，并满足项目文档使用中文的要求。
- **提交**：本次提交

## [2026-08-20] 搭建 DX11 渲染实验平台

- **文件**：根目录 CMake/文档、`src/`、`tests/`、`scripts/`
- **范围**：项目初始实现
- **变更**：新增可移植的 VS2022 DX11 编辑器框架，包含基础几何体、相机、Gizmo、撤销/重做、
  Effect 隔离、日志、测试和学习文档。
- **原因**：建立一个可复用的平台，用于验证学习到的渲染技术。
- **提交**：`3aafc78`
