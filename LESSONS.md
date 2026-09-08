# LESSONS - LRenderDemo

### ADR-016：Effect 通过显式成员拥有具体 GPU 资源

- **决策**：`EffectResource` 表示单个二维颜色/深度目标，`EffectCubeMapResource` 表示单个 Cubemap；具体 Effect 直接持有需要的资源成员，不使用字符串资源注册表。
- **原因**：当前资源数量在编译期明确，显式成员更容易追踪所有权、调试和调整不同分辨率，也避免资源容器与资源实例职责重叠。
- **边界**：跨 Effect 共享的 Mesh、Texture 和 Sampler 继续由 `ResourceCache` 管理；资源数量真正动态变化后才使用容器。
- **工程约定**：`stdfx.h` 作为 CMake 预编译头集中维护常用依赖，但公共头仍必须自包含。
- **状态**：已接受，取代 ADR-012 中“IRenderEffect 持有资源注册表”的部分。

### ADR-017：由 Renderer 统一拥有公共 CBuffer

- **决策**：`Dx11Renderer` 持有 `CommonConstantBuffers`，Frame/Object/Material/Light 四种 Buffer 每种只创建一份；`EffectFrameContext` 只借用该集合。
- **原因**：公共 CBuffer 的生命周期与 D3D11 Device/Context 一致，避免每个 Effect 重复创建同一 ABI 的 Buffer；Object/Material 仍可按绘制项复用更新。
- **边界**：这是每个 Device/ImmediateContext 一份，不是进程级静态全局；DeferredContext 或多设备场景需要各自创建一组。
- **状态**：已接受。

### ADR-015：普通函数声明与实现分离

- **决策**：普通类在头文件中只保留声明，函数体放入对应的 `.cpp`；模板、接口和 GPU 布局继续保留在头文件。
- **原因**：缩短头文件编译依赖，降低修改实现时的重编译范围，同时保持模板实例化和二进制布局契约有效。
- **边界**：纯数据结构可以只包含默认成员初始化；`Dx11ConstantBuffer<T>` 不拆分，避免模板链接问题。

### ADR-014：资源路径使用相对布局并在构建时打包
- **问题**：VS 调试工作目录是仓库根目录，直接启动 exe 的工作目录是 Debug 输出目录，导致相对模型和贴图路径失效。
- **决策**：保留 `assets/...` 相对路径，CMake 构建后将 `assets` 拷贝到 exe 同级目录；不在运行时修改当前目录。ImGui 配置仍独立保存在 exe 目录。
- **结论**：源码和部署包都采用同一目录布局，不包含机器相关绝对路径。

### ADR-013：每个 Effect 必须显式恢复其依赖的关键状态
- **问题**：全屏后处理将深度测试设为 Disabled，下一帧场景绘制沿用该状态，导致平面覆盖立方体和球。
- **决策**：`BasicMeshEffect::Bind` 在每次 Mesh 绘制前绑定 `DepthDefault`，不依赖前一个 Effect 的状态。
- **结论**：DX11 immediate context 是有状态机器；Effect 必须显式设置自己依赖的状态，不能假设上一调用留下正确值。

### ADR-012：Effect 资源和视图状态由渲染层集中管理
- **决策**：`IRenderEffect` 持有 `EffectResource`；高层 `Draw` 负责实际绘制，`Bind` 保留为可调试的管线绑定步骤；`ViewStateGuard` 用 RAII 保存并恢复 DX11 Context 状态。
- **原因**：多 Pass 效果需要多个离屏目标和 SRV，且 Effect 不应把资源生命周期泄漏到 Renderer；即时 Context 的状态泄漏会影响后续 Effect 和 ImGui。
- **边界**：当前 ViewManager 管理逻辑 View 和 HWND 记录，Renderer 仍使用单交换链；后续接入多交换链无需改变 EffectResource 接口。
- **状态**：部分被 ADR-016 取代；`Draw/Bind` 和 `ViewStateGuard` 结论继续有效。

### ADR-011：公共 Shader 契约按更新频率拆分

- **决策**：使用 `common.hlsli` 统一声明 Frame、Object、Material、Light CBuffer；C++ 使用 `CommonConstants.h` 对齐布局。
- **原因**：多个 Effect 需要共享数据，但世界矩阵、材质和相机的更新频率不同，不能继续塞进一个 BasicMesh 专属缓冲。
- **边界**：纹理类型和采样器语义由具体 Effect 声明，CPU 侧通过 `ResourceCache` 复用资源；Core 层不依赖 DX11。
- **状态**：已接受。

本文档记录长期有效的架构决策和项目专用工程知识。

## 架构决策记录

### ADR-001：使用原生 DX11 和职责集中的辅助库

- **日期**：2026-08-20
- **背景**：项目既要用于学习 DX11，也要提供可用的编辑器平台。
- **备选方案**：rbfx、Diligent Engine、LLGL，或 Win32 + DirectXTK + ImGui + ImGuizmo。
- **决策**：使用 Win32 + DirectXTK + Dear ImGui + ImGuizmo。
- **原因**：DX11 对象和绘制流程仍然可见；辅助库则减少重复的数学、界面和 Gizmo 工作。
- **结果**：场景和撤销系统需要由项目维护，但学习界面更加清晰。
- **状态**：已接受。

### ADR-002：在出现第二个后端前暂缓设计通用 RHI

- **日期**：2026-08-20
- **背景**：未来可能引入 RHI，但当前仅学习 DX11。
- **备选方案**：立即设计 RHI，或先通过模块边界隔离原生 DX11。
- **决策**：将原生 DX11 保持在 `src/render/` 内，今后再根据经过验证的需求提取 RHI。
- **原因**：避免推测性的抽象，同时保护非渲染器模块不受影响。
- **结果**：未来新增后端时需要有计划地重构渲染器，但无需重写整个编辑器。
- **状态**：已接受。

### ADR-003：使用 Git 子模块锁定第三方源码

- **日期**：2026-08-20
- **背景**：构建必须能在其他 VS2022 机器上准确复现。
- **备选方案**：CMake FetchContent 分支、包管理器、源码内置或子模块。
- **决策**：使用 Git 子模块记录依赖项的精确提交。
- **原因**：CMake 保持简单，并且每次克隆都能解析到相同的源码版本。
- **结果**：克隆时应使用 `--recurse-submodules`；引导脚本会修复遗漏的子模块。
- **状态**：已接受。

### ADR-004：项目 Shader 在构建期编译为配置隔离的 CSO

- **日期**：2026-08-20
- **背景**：学习者需要在 VS 中直接编辑 HLSL，并让 Shader 改动通过正常构建稳定反映到运行效果。
- **备选方案**：继续使用 DirectXTK 内置字节码、运行时编译源码，或在构建期用 FXC 生成 CSO。
- **决策**：将项目 HLSL 纳入 `LRenderDemo` 目标，使用 VS 的 `FXCompile` 按配置生成 CSO，
  Effect 只加载当前构建目录中的产物。
- **原因**：编译错误能在构建阶段暴露，VS 工程中可见源码，Debug/Release 不会互相覆盖，且无需
  手工复制生成文件。
- **结果**：新增 Effect 时必须同时登记 HLSL、入口点、Shader Model、输出目录和运行时加载路径。
- **状态**：已接受。

### ADR-005：使用 cgltf 建立静态 glTF 资源边界

- **日期**：2026-08-21
- **背景**：后续纹理、法线贴图、PBR 和场景测试都需要统一的模型与材质入口。
- **备选方案**：Assimp、手写 JSON/glTF 解析，或固定版本的 `cgltf`。
- **决策**：内置固定提交的 `cgltf`，项目负责坐标系转换、GPU 资源创建与缓存。
- **原因**：保留 glTF 数据流的学习可见性，同时避免自行维护格式解析器；C 接口也能验证工程的 C/C++
  混合编译边界。
- **结果**：`core/Scene` 只保存资产路径和资产实体索引，DX11 对象集中在 `render/`；网格资产、
  纹理与 Sampler 由 `ResourceCache` 复用。当前只承诺静态三角网格和 BaseColor，不把近似高光
  称为完整 PBR。
- **状态**：已接受。

### ADR-006：实体保存材质参数，渲染层解析 GPU 材质

- **日期**：2026-08-25
- **背景**：每个实体需要独立修改材质和贴图，同时 glTF 模型可能包含多个共享的子材质。
- **备选方案**：让 `Entity` 直接持有 SRV/Sampler，修改缓存中的 `MeshPart::material`，或保存与 API
  无关的实体材质参数并在绘制时合并。
- **决策**：`Entity::material` 保存颜色、光照系数、显示模式、Sampler 枚举和贴图路径；
  `Dx11Renderer` 复制源 `Material` 后应用这些参数，并从 `ResourceCache` 取得 GPU 资源。
- **原因**：实体材质可撤销、可序列化且不依赖 DX11；同一缓存模型的多个实例可以独立编辑，
  不会互相污染。
- **结果**：后续增加材质通道时，先扩展 API 无关的数据，再由后端解析成纹理槽和渲染状态。
- **状态**：已接受。

### ADR-007：分离场景 Model 与渲染 MeshAsset，并用导入器策略扩展格式

- **日期**：2026-08-25
- **背景**：场景需要由 Model 管理可混合的 Solid/Mesh Entity，同时新增 OBJ，未来还要尽量低成本
  接入 Assimp 支持 FBX 等格式。
- **备选方案**：每种格式直接改编辑器和渲染器；让场景 Entity 持有 GPU Mesh；或统一通过
  `IModelImporter` 生成渲染层 `MeshAsset`。
- **决策**：`Scene -> Model -> Entity` 只表达可编辑所有权；`MeshGeometry` 保存资产路径和实体索引；
  `ResourceCache -> ModelLoader -> IModelImporter` 负责缓存、格式分发和 GPU 资产创建。
- **原因**：同一 Model 可自然混合基础实体和导入网格；新增格式只增加导入器，不让格式细节进入
  core、commands、editor 或渲染遍历；同时避免场景 `Model` 与缓存资源类型同名。
- **结果**：原渲染 `Model/ModelPart` 更名为 `MeshAsset/MeshPart`；glTF 和 OBJ 共用相同输出结构；
  导入整个 Model 可以一次撤销/重做。未来 Assimp 导入器应优先承接 FBX 等复杂格式，不替换现有
  glTF/OBJ 教学路径。
- **状态**：已接受。

### ADR-008：只抽离常量缓冲资源操作，保留 Effect 的参数语义

- **日期**：2026-08-26
- **背景**：`BasicMeshEffect` 内重复包含 DX11 缓冲创建、更新和阶段绑定代码，VS/PS 也分别声明
  同一份 `cbuffer`；但当前只有一个正式网格 Effect，尚无共享 Frame/Object/Material 数据的需求。
- **备选方案**：保持全部内置；只抽离类型化缓冲封装和布局文件；或立即让 Renderer 管理并拆分
  所有常量缓冲。
- **决策**：新增 `Dx11ConstantBuffer<T>` 处理 GPU 资源操作，新增独立 C++ 常量结构和共享 `.hlsli`；
  `BasicMeshEffect` 仍决定数据含义、组装方式、`b0` 槽位和 Shader 阶段。
- **原因**：消除稳定且真实的机械重复，同时让 DX11 调用保持可见；避免在第二个 Effect 出现前
  推测跨 Effect 的参数系统和更新频率。
- **结果**：新增 Effect 可以复用缓冲 RAII 封装，但应拥有自己的常量类型与 HLSL 契约。出现跨
  Effect 共享的每帧数据后，再评估 `b0/b1/b2` 和 BeginFrame/BindObject/BindMaterial 调用协议。
- **状态**：已接受。

### ADR-009：用并列 Context 类表达 Effect 参数生命周期

- **日期**：2026-08-26
- **背景**：`IRenderEffect::Bind` 的八个位置参数混合了 D3D11 Context、每帧 Camera 数据和逐 Draw
  的 Entity/Material/选择状态，调用点易错且无法直接看出更新频率。
- **备选方案**：单个大参数包；共同父类加两个派生 Context；`BeginFrame` 有状态协议；或并列的
  Frame/Draw Context 快照。
- **决策**：使用无共同父类的两个 `final class`。Frame Context 每帧从 Camera 构造一次；Draw
  Context 每 MeshPart 从 Entity、解析后 Material 和 selectedEntityId 构造；Effect 只接收两者。
- **原因**：Frame 与 Draw 必须同时存在且不可相互替换，继承不符合 is-a 关系；构造快照既减少
  重复相机计算，也不让 Effect 依赖完整 Camera/Entity 行为或保存跨调用的临时状态。
- **结果**：`Bind(frame, draw)` 具有明确生命周期；未来真正拆分 Frame/Object/Material cbuffer 时，
  可以在此基础上评估 `BeginFrame`，当前不提前引入时间耦合。
- **状态**：已接受。

### ADR-010：Solid 保存参数语义，GPU Mesh 作为可重建缓存

- **日期**：2026-08-26
- **背景**：Solid 需要按尺寸和细分创建、实时修改并保存，未来还可能从 CPU Mesh 迁移到 GPU 生成。
- **备选方案**：继续只保存类型和 Transform Scale；把生成顶点保存进 Entity；或保存参数并由渲染层
  维护可重建 Mesh。
- **决策**：`SolidGeometry` 使用强类型参数作为唯一场景语义；`SolidMeshCache` 按 Entity 保存当前
  参数生成的 DX11 Mesh；`.lscene` 只序列化参数和资源引用。
- **原因**：编辑、撤销和保存共享同一份 API 无关数据；实时拖动不会留下无界参数缓存；未来替换
  GPU Generator 时不改变 Scene 和文件格式。
- **结果**：相同参数的多个 Entity 当前不共享 Mesh；高细分实时修改仍有 Buffer 重建成本。等实际
  场景规模证明需要时，再引入有界的共享缓存，不提前设计浮点哈希。
- **状态**：已接受。

## 已知问题与经验

### PIT-001：子模块初始化中断会留下受保护的 Git 元数据

- **日期**：2026-08-20
- **现象**：超时的 `git submodule add` 留下了工作树指针，但未取得对应的远端分支。
- **根因**：获取所需分支之前，完整历史克隆过程被终止。
- **解决方案**：引导脚本检测不完整状态，通过 Git 执行浅获取，检出 `FETCH_HEAD`，并在不直接
  编辑受保护 `.git` 文件的情况下登记 gitlink。
- **预防措施**：初次添加和更新都使用 `--depth 1`；记录的 gitlink 仍然固定到精确的依赖提交。

### PIT-002：DirectXTK 工具会要求无关的 C# 工作负载

- **日期**：2026-08-20
- **现象**：在仅安装 C++ 的 VS2022 环境中，CMake 因 `MakeSpriteFont` 请求 C# 编译器。
- **根因**：DirectXTK 默认启用命令行工具。
- **解决方案**：添加 DirectXTK 前设置 `BUILD_TOOLS=OFF`。
- **预防措施**：只启用渲染实验平台实际使用的依赖组件。

## 最佳实践

### PRACTICE-001：区分网格 Effect 和屏幕空间 Pass

逐对象状态实现 `IRenderEffect`。使用帧纹理的效果应采用未来的 `IRenderPass` 接口，使资源依赖
保持显式可见。

### PRACTICE-002：导入时烘焙节点变换并集中处理手性

glTF 节点矩阵为列主序，当前 DirectX `SimpleMath` 代码采用行向量约定。`cgltf` 输出的 16 个值按
相同顺序写入 `SimpleMath::Matrix` 时，语义上已经完成列向量矩阵到行向量矩阵的转置，不能再交换
行列。随后将顶点烘焙到模型空间；右手系转左手系时翻转 Z 并反转三角形绕序。

## 项目约定

### CONVENTION-001：矩阵和变换的表示方式

场景变换保存位移、欧拉角（度）和缩放值，以便直观编辑。矩阵组合使用 DirectXTK
`SimpleMath`；ImGuizmo 将交互矩阵重新分解为这些字段。
