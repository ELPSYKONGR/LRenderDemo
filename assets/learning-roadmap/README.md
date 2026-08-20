# 渲染学习路线素材索引

本目录不复制第三方大文件，而是把已经下载到 `assets/test-scenes/downloads/` 和
`assets/skyboxes/downloads/` 的素材映射到实现里程碑。下载目录由 `.gitignore` 排除；本索引、
下载脚本和来源说明进入版本控制。

## 准备状态

- 核验日期：2026-08-20
- 已核验文件：85
- SHA-256 失败：0
- 模型资产：约 66.59 MB
- 天空盒资产：约 1.50 MB

在当前机器上，下列素材均已下载，可直接供后续模型加载器和 Effect 使用。

## 素材与用途

| 素材 | 本地入口 | 适用阶段 | 选择原因 | 压力级别 |
|---|---|---|---|---|
| Stanford Bunny | `../test-scenes/downloads/stanford-bunny/bunny.off` | 几何导入、拾取、实例化、剔除 | 无复杂材质，适合先验证拓扑和射线求交 | 低 |
| Suzanne | `../test-scenes/downloads/suzanne/Suzanne.gltf` | 首个 glTF、UV、BaseColor、材质 | 外部 BIN/PNG 依赖清晰，便于逐步调试加载流程 | 低 |
| Damaged Helmet | `../test-scenes/downloads/damaged-helmet/DamagedHelmet.glb` | GLB、法线贴图、PBR、IBL | 单文件内嵌资源，包含完整材质验证价值 | 中 |
| Metal-Rough Spheres | `../test-scenes/downloads/metal-rough-spheres/MetalRoughSpheres.glb` | 金属度/粗糙度、曝光、色调映射 | 规则参数梯度便于识别 BRDF 和色彩空间错误 | 中 |
| Sponza | `../test-scenes/downloads/sponza/Sponza.gltf` | 阴影、SSAO、延迟渲染、剔除和性能 | 大型多材质室内场景，能暴露资源和深度问题 | 高 |
| DirectXTK Cubemap | `../skyboxes/downloads/directxtk-cubemap/cubemap.dds` | 静态天空盒、反射、IBL 前置实验 | DX11 可直接读取的 cubemap DDS，来源提交已固定 | 低 |

## 推荐使用顺序

1. Bunny：验证最小模型数据、包围体和拾取。
2. Suzanne：验证 glTF 外部 Buffer、PNG、UV 和基础材质。
3. Damaged Helmet：验证 GLB、Normal、Metallic/Roughness 和内嵌资源。
4. Metal-Rough Spheres：标定 PBR、曝光与色调映射。
5. Cubemap：实现静态天空盒，再用于环境反射。
6. Sponza：最后用于阴影、SSAO、Deferred 和性能回归。

## 下载与校验

首次克隆或下载目录缺失时运行：

```powershell
powershell -ExecutionPolicy Bypass -File scripts/download-test-scenes.ps1
powershell -ExecutionPolicy Bypass -File scripts/download-skybox-assets.ps1
```

下载完成后运行：

```powershell
powershell -ExecutionPolicy Bypass -File scripts/verify-learning-assets.ps1
```

每个 Khronos 模型的具体许可和署名要求以同目录 `SOURCE_README.md` 为准；Bunny 的镜像说明见
`MIRROR_README.md`，cubemap 许可见 `SOURCE_LICENSE.txt`。在分发应用或素材包前必须重新检查这些文件。
