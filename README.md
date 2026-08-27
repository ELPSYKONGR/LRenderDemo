# LRenderDemo

LRenderDemo 是一个用于学习 Direct3D 11 的小型 Windows 原生渲染实验平台。项目有意保持平台层、
编辑器、场景、命令、渲染器和 Effect 之间的边界清晰，使学习者无需引入完整游戏引擎，便可独立
实现、验证和比较不同渲染技术。

## 当前里程碑

- Win32 原生窗口，以及 D3D11 设备和交换链
- 基于 Dear ImGui Docking 的界面，包含视口、层级、检查器和工具面板
- 支持环绕、平移、缩放、八个标准视角和自动旋转的编辑器相机
- 按尺寸、半径和细分参数创建并实时编辑立方体、UV 球体和水平平面
- 实体级 Blinn-Phong 材质编辑，可调基础色、漫反射、高光、双面状态和贴图采样方式
- BaseColor 贴图缩略图、原生文件选择、源贴图恢复和三种贴图/光照显示模式
- 导入静态 glTF/GLB 和 OBJ/MTL 模型，支持多网格实体、基础材质、贴图与资源缓存
- `Scene -> Model -> Entity` 两级场景层级；同一 Model 可混合绘制 Solid Entity 和 Mesh Entity
- 一盏方向光和最多四盏点光，支持 Lambert 漫反射与 Blinn-Phong 高光实时调节
- 使用 ImGuizmo 进行平移、旋转和缩放
- 对实体创建、变换和材质编辑执行撤销/重做
- 使用版本化 `.lscene` JSON 保存/打开场景，外部资源使用相对路径引用
- 程序化生成 D3D11 顶点缓冲区和索引缓冲区
- 独立的 `IRenderEffect` 边界，以及可直接编辑、构建和加载的项目自有 HLSL
- 面向 Visual Studio 2022 的 CMake Presets 和轻量级 CTest 测试

## 环境要求

- Windows 10 或 Windows 11
- Visual Studio 2022，并安装 **使用 C++ 的桌面开发** 工作负载
- Windows 10 SDK 10.0.19041 或更高版本
- CMake 3.24 或更高版本
- Git 2.30 或更高版本

## 快速开始

首次克隆时请使用递归选项，以准确还原依赖项的固定提交：

```powershell
git clone --recurse-submodules https://github.com/ELPSYKONGR/LRenderDemo.git
Set-Location LRenderDemo
powershell -ExecutionPolicy Bypass -File scripts/bootstrap-and-verify.ps1
```

引导脚本会检查工具链、初始化缺失的子模块、生成 VS2022 解决方案、构建 Debug 版本、运行 CTest，
最后启动应用程序。执行日志保存在 `logs/` 目录下。

## 使用 Visual Studio 2022 调试

1. 至少运行一次引导脚本。
2. 打开 `build/vs2022/LRenderDemo.sln`。
3. 选择 `LRenderDemo` 和 `Debug | x64`。
4. 按 `F5` 开始调试。

生成的项目已将仓库根目录设置为调试器工作目录，因此在不同机器上，日志和后续相对路径资源都能
以一致方式解析。

## 日常命令

```powershell
cmake --preset vs2022
cmake --build --preset vs2022-debug
ctest --preset vs2022-debug
```

## 修改 Shader

基础网格 Shader 位于 `src/shaders/BasicMeshVS.hlsl` 和 `src/shaders/BasicMeshPS.hlsl`，在
Visual Studio 的 `LRenderDemo > Shaders` 筛选器中可直接打开。修改后执行“生成”或按 `F5`，VS
会通过 FXC 重新生成当前配置的 `.cso`；`BasicMeshEffect` 启动时加载本次构建对应的文件，因此
无需手工复制 Shader。Debug 产物位于 `build/vs2022/src/shaders/Debug/`，不要直接编辑该目录。

在构建代理上，或仅需执行验证时，请使用
`scripts/bootstrap-and-verify.ps1 -SkipLaunch`。

## 经典测试模型

项目提供 Stanford Bunny、Suzanne、Sponza、Metal/Roughness Spheres 和 Damaged Helmet 的
可复现下载脚本：

```powershell
powershell -ExecutionPolicy Bypass -File scripts/download-test-scenes.ps1
```

模型保存在 `assets/test-scenes/downloads/`，来源、许可限制和建议用途见
`assets/test-scenes/README.md`。第三方模型大文件不会提交到 Git。

下载后可在编辑器中选择 `Create > Import Mesh...`，导入成功的模型会进入层级面板，并与
基础几何体一样支持 Gizmo、检查器变换和创建操作的撤销/重做。`Resources` 面板显示网格资产/纹理
缓存数量；重复导入同一路径不会重复创建 GPU 资源。`Lighting` 面板可编辑环境光、方向光和四盏
点光。当前导入器的完整支持边界和阅读顺序见 `docs/model-import-and-material.md`。

天空盒学习素材可通过 `scripts/download-skybox-assets.ps1` 下载。完整的类设计、Pass 顺序、HLSL、
DX11 状态和逐文件改动示例见 `docs/examples/skybox-pass.md`。

与 `DirectX11-With-Windows-SDK-master` 的逐项功能差距、推荐实现顺序和验收标准见
`docs/directx11-feature-roadmap.md`。已经下载的模型如何对应纹理、拾取、法线贴图、阴影、SSAO 和
延迟渲染阶段，见 `assets/learning-roadmap/README.md`。

## 编辑器操作

| 功能 | 输入方式 |
|---|---|
| 环绕相机 | 在视口中按住鼠标右键并拖动 |
| 平移相机 | 在视口中按住鼠标中键并拖动 |
| 缩放相机 | 鼠标指向视口时滚动滚轮 |
| 标准/轴测视角 | 使用 `Camera` 面板的八个视角按钮 |
| 自动旋转 | 在 `Camera` 面板启用并设置方向、速度 |
| 平移/旋转/缩放 | 按 `W` / `E` / `R`，或使用工具面板按钮 |
| 撤销/重做 | 按 `Ctrl+Z` / `Ctrl+Y` |
| 创建基础几何体 | 使用 `Create` 菜单 |
| 修改 Solid 参数 | 在 `Inspector > Geometry` 调节尺寸、半径和细分数 |
| 导入模型 | `Create > Import Mesh...`，支持 `.gltf`、`.glb`、`.obj` |
| 新建/打开/保存场景 | `Ctrl+N` / `Ctrl+O` / `Ctrl+S` |
| 修改材质 | 在 `Inspector > Material` 调节颜色、光照参数和双面状态 |
| 选择/恢复贴图 | 在材质区域使用 `Choose...` / `Use source` |
| 切换贴图显示 | 选择 `Lit textured`、`Texture only` 或 `Lit untextured` |
| 调节多光源 | 使用 `Lighting` 面板 |

## 建议学习路线

建议按以下顺序阅读：

1. `src/platform/Window.cpp`：Win32 消息和可见窗口的生命周期。
2. `src/render/Dx11Renderer.cpp`：设备、交换链、帧目标和绘制遍历。
3. `src/render/Mesh.cpp`：不可变顶点/索引缓冲区和索引绘制调用。
4. `src/render/EffectContext.cpp`、`src/render/Dx11ConstantBuffer.h`、
   `src/render/BasicMeshConstants.h` 与 `src/render/BasicMeshEffect.cpp`：Frame/Draw 快照、常量缓冲
   RAII、C++/HLSL 布局契约、参数组装和渲染状态。
5. `src/render/ModelLoader.cpp`、`src/render/GltfLoader.cpp`、`src/render/ObjLoader.cpp` 与
   `src/render/ResourceCache.cpp`：格式分发、网格资产导入及纹理/Sampler 缓存。
6. `src/editor/EditorLayer.cpp`、`src/editor/EditorAssets.cpp`、`src/editor/EditorMaterial.cpp`：
   编辑器交互、导入、材质和光照控制。
7. `src/commands/`：独立于界面的可逆操作。
8. `src/core/SolidGeometry.cpp`、`src/render/SolidMeshCache.cpp`、
   `src/persistence/SceneSerializer.cpp`：参数校验、运行时 Mesh 更新和场景持久化。

添加新的渲染技术前，请先阅读 `docs/architecture.md` 和 `docs/adding-an-effect.md`。
参数化 Solid 与 `.lscene` 字段和调用流程见 `docs/parameterized-solid-and-scene-save.md`。

## 仓库约定

日常开发和提交直接在 `main` 分支进行；只有需要隔离的实验性工作才创建 `feat/*` 分支。
生成文件、本地环境值、日志和凭据均由 `.gitignore` 排除。
