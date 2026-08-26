# 添加渲染效果

每个渲染实验都应保持隔离，确保禁用某项技术时不会破坏编辑器或场景模型的稳定性。

## 网格 Effect

当某项技术主要改变每个网格的着色器、常量和管线状态时，使用 `IRenderEffect`。
`BasicMeshEffect` 是参考实现。

1. 添加 `src/render/effects/MyEffect.h` 和 `.cpp`。
2. 在 `src/shaders/` 添加 VS/PS，并参考 `src/CMakeLists.txt` 登记 `FXCompile` 类型、入口点、
   Shader Model 和按配置隔离的 CSO 输出路径。
3. 实现 `IRenderEffect::Bind`。
4. 定义该 Effect 专用的 C++ 常量结构和共享 `.hlsli` 布局；在类内部持有
   `Dx11ConstantBuffer<T>`，由 Effect 决定参数语义、寄存器槽位和使用它的 Shader 阶段。
5. 在类内部持有着色器、输入布局和固定状态；从构建目录加载 CSO，不直接加载源码目录。
6. 验证所有设备/上下文输入；创建失败时抛出包含上下文信息的异常。
7. 在 `Dx11Renderer` 中注册 Effect；只向 `EditorLayer` 暴露用于学习的参数。
8. 条件允许时，为参数验证添加 CPU 侧测试。
9. 如果边界发生变化，更新 `FILE_INDEX.md`、`CHANGELOG.md` 和本文档。

不要因为存在通用 `Dx11ConstantBuffer<T>` 就共享不同 Effect 的常量类型。C++ 结构、HLSL 布局、
寄存器槽位和更新频率共同组成一个 Effect 契约；只有多个实际消费者需要同一份数据时才上移所有权。

## 场景级 Pass

天空盒、阴影图等功能每帧按场景执行一次，不属于单个实体。此类功能应继承 `IRenderPass`，由 Pass
负责绘制时机和资源依赖，并在内部组合专用 Effect。不要为了复用接口而伪造
`IRenderEffect::Bind` 所需的 world、color 或 selected 参数。

完整示例见 `docs/examples/skybox-pass.md`，其中说明了 `SkyboxPass`、`SkyboxEffect`、cubemap
DDS、深度状态和 `Dx11Renderer` 接入方式。

## 屏幕空间效果

SSAO、SSR、Bloom 和色调映射操作的是帧资源，而非单个网格。实现第一项此类技术时，应新增独立的
`IRenderPass` 边界。Pass 应接收显式上下文，其中包含输入 SRV、输出 RTV、深度、相机矩阵和
视口尺寸。不要将屏幕空间工作强行放入 `IRenderEffect::Bind`。

## 建议学习顺序

1. 无光照颜色 Effect
2. Blinn-Phong 光照控制
3. 法线贴图
4. 阴影贴图 Pass
5. HDR 目标和色调映射
6. SSAO
7. SSR

实现每个效果前后各捕获一次 PIX 帧，用于检查资源绑定和 GPU 开销。
