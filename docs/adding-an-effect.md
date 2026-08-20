# 添加渲染效果

每个渲染实验都应保持隔离，确保禁用某项技术时不会破坏编辑器或场景模型的稳定性。

## 网格 Effect

当某项技术主要改变每个网格的着色器、常量和管线状态时，使用 `IRenderEffect`。
`BasicMeshEffect` 是参考实现。

1. 添加 `src/render/effects/MyEffect.h` 和 `.cpp`。
2. 实现 `IRenderEffect::Bind`。
3. 在类内部持有着色器、输入布局、状态和常量缓冲区。
4. 验证所有设备/上下文输入；创建失败时抛出包含上下文信息的异常。
5. 在 `Dx11Renderer` 中注册 Effect；只向 `EditorLayer` 暴露用于学习的参数。
6. 条件允许时，为参数验证添加 CPU 侧测试。
7. 如果边界发生变化，更新 `FILE_INDEX.md`、`CHANGELOG.md` 和本文档。

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
