# 经典图形学测试模型

本目录集中保存用于学习和验证渲染效果的第三方测试资产。实际模型下载到 `downloads/`，该目录
默认不进入 Git；仓库只保留可复现下载脚本和本说明。

## 下载方式

在仓库根目录运行：

```powershell
powershell -ExecutionPolicy Bypass -File scripts/download-test-scenes.ps1
```

已有且非空的文件默认跳过。需要重新下载时添加 `-Force`。脚本完成后会在
`downloads/SHA256SUMS.txt` 生成本机文件校验清单。

## 资产清单

| 资产 | 格式 | 主要用途 | 来源与许可说明 |
|---|---|---|---|
| Stanford Bunny | OFF | 网格加载、法线、曲率、细分和简化 | 原始数据来自 Stanford 3D Scanning Repository，本项目通过 libigl 教程数据镜像下载；使用时应注明 Stanford 来源，其他用途需复核双方说明 |
| Suzanne | glTF | 基础拓扑、材质、法线和快速回归 | Khronos glTF Sample Assets；CC0 1.0，具体署名见 `SOURCE_README.md` |
| Sponza | glTF | 大型室内场景、阴影、SSAO、SSR 和遮挡剔除 | Khronos glTF Sample Assets；采用 CryEngine Limited License，使用前必须阅读 `SOURCE_README.md` |
| Metal/Roughness Spheres | GLB | PBR 金属度/粗糙度组合和 BRDF 回归 | Khronos glTF Sample Assets；CC BY 4.0，使用时需保留署名，详情见 `SOURCE_README.md` |
| Damaged Helmet | GLB | PBR 金属度/粗糙度、法线、AO 和 IBL | Khronos glTF Sample Assets；包含 CC BY 4.0 和 CC BY-NC 4.0，禁止未经许可的商业使用 |

## 目录结构

```text
assets/test-scenes/
├── README.md
└── downloads/
    ├── SHA256SUMS.txt
    ├── stanford-bunny/
    │   ├── bunny.off
    │   └── MIRROR_README.md
    ├── suzanne/
    │   ├── Suzanne.gltf
    │   ├── Suzanne.bin
    │   ├── textures/
    │   └── SOURCE_README.md
    ├── sponza/
    │   ├── Sponza.gltf
    │   ├── Sponza.bin
    │   ├── textures/
    │   └── SOURCE_README.md
    ├── metal-rough-spheres/
    │   ├── MetalRoughSpheres.glb
    │   └── SOURCE_README.md
    └── damaged-helmet/
        ├── DamagedHelmet.glb
        └── SOURCE_README.md
```

## 使用注意事项

- 第三方资产的版权和许可不因下载到本项目而改变；公开发布截图、模型修改版或产品前，应再次
  阅读对应来源说明。
- GLB 是包含网格、材质和纹理的单文件 glTF；Sponza 和 Suzanne 则保留多文件 glTF 结构，
  两者适合一起验证导入器的 URI 和资源路径处理。
- Stanford Bunny 原始扫描存在孔洞和多个重建版本；这里采用 libigl 教程中的 OFF 转换版本，
  便于先实现轻量网格导入器。
- 模型的单位、坐标系和正面方向并不统一。导入器应显式记录轴向转换和缩放，而不是修改原始资产。
