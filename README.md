# LRenderDemo

LRenderDemo 是一个用于学习 Direct3D 11 的小型 Windows 原生渲染实验平台。项目有意保持平台层、
编辑器、场景、命令、渲染器和 Effect 之间的边界清晰，使学习者无需引入完整游戏引擎，便可独立
实现、验证和比较不同渲染技术。

## 当前里程碑

- Win32 原生窗口，以及 D3D11 设备和交换链
- 基于 Dear ImGui Docking 的界面，包含视口、层级、检查器和工具面板
- 支持环绕、平移和缩放的编辑器相机
- 创建立方体和 UV 球体
- 使用 ImGuizmo 进行平移、旋转和缩放
- 对实体创建和变换编辑执行撤销/重做
- 程序化生成 D3D11 顶点缓冲区和索引缓冲区
- 独立的 `IRenderEffect` 边界，以及基于 DirectXTK `BasicEffect` 的实现
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

在构建代理上，或仅需执行验证时，请使用
`scripts/bootstrap-and-verify.ps1 -SkipLaunch`。

## 编辑器操作

| 功能 | 输入方式 |
|---|---|
| 环绕相机 | 在视口中按住鼠标右键并拖动 |
| 平移相机 | 在视口中按住鼠标中键并拖动 |
| 缩放相机 | 鼠标指向视口时滚动滚轮 |
| 平移/旋转/缩放 | 按 `W` / `E` / `R`，或使用工具面板按钮 |
| 撤销/重做 | 按 `Ctrl+Z` / `Ctrl+Y` |
| 创建基础几何体 | 使用 `Create` 菜单 |

## 建议学习路线

建议按以下顺序阅读：

1. `src/platform/Window.cpp`：Win32 消息和可见窗口的生命周期。
2. `src/render/Dx11Renderer.cpp`：设备、交换链、帧目标和绘制遍历。
3. `src/render/Mesh.cpp`：不可变顶点/索引缓冲区和索引绘制调用。
4. `src/render/BasicMeshEffect.cpp`：着色器常量、输入布局和渲染状态。
5. `src/editor/EditorLayer.cpp`：编辑器面板、相机输入、Gizmo 和命令创建。
6. `src/commands/`：独立于界面的可逆操作。

添加新的渲染技术前，请先阅读 `docs/architecture.md` 和 `docs/adding-an-effect.md`。

## 仓库约定

日常开发在 `dev` 或 `feat/*` 分支进行。`main` 仅用于经用户批准的稳定里程碑。
生成文件、本地环境值、日志和凭据均由 `.gitignore` 排除。
