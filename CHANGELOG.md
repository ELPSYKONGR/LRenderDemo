# CHANGELOG - LRenderDemo

## 近期变更

| 时间 | 类型 | 摘要 | 模块 | 提交 |
|---|---|---|---|---|
| 08-20 | 文档 | 增加 DX11 教程差距分析、实施路线和素材映射 | docs、assets、scripts | 工作区 |
| 08-20 | 功能 | 接入可编辑且自动增量编译的项目 HLSL | render、shaders、cmake | 工作区 |
| 08-20 | 文档 | 增加天空盒 Pass 教学示例与素材 | docs、assets、scripts | 本次提交 |
| 08-20 | 资产 | 增加经典图形学测试模型下载集 | assets、scripts | 本次提交 |
| 08-20 | 文档 | 项目 Markdown 文档中文化 | docs | 本次提交 |
| 08-20 | 功能 | 搭建 DX11 渲染实验平台 | 全部 | `3aafc78` |

---

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
