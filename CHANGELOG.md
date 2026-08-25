# CHANGELOG - LRenderDemo

## 近期变更

| 时间 | 类型 | 摘要 | 模块 | 提交 |
|---|---|---|---|---|
| 08-25 | 工程 | 在 VS2022 工程中显示全部项目头文件 | cmake | 本次提交 |
| 08-21 | 功能 | 增加纹理材质、glTF/GLB 缓存导入与可编辑多光源 | core、render、editor、shaders | 工作区 |
| 08-20 | 文档 | 增加 DX11 教程差距分析、实施路线和素材映射 | docs、assets、scripts | 工作区 |
| 08-20 | 功能 | 接入可编辑且自动增量编译的项目 HLSL | render、shaders、cmake | 工作区 |
| 08-20 | 文档 | 增加天空盒 Pass 教学示例与素材 | docs、assets、scripts | 本次提交 |

---

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
