# 天空盒学习素材

本目录保存天空盒与环境光照实验使用的第三方素材。实际二进制文件下载到 `downloads/`，默认不进入
Git；仓库只保留可复现下载脚本和中文说明。

## 当前素材

| 名称 | 格式 | 用途 | 来源 | 许可 |
|---|---|---|---|---|
| DirectXTKTest Cubemap | DDS TextureCube | DX11 天空盒第一阶段示例 | `walbourn/directxtktest` 的 `EffectsTest/cubemap.dds` | MIT，原始许可见下载目录的 `SOURCE_LICENSE.txt` |

固定上游提交：`fabb928cf620381dfc188d52040a5a8e32bd1aec`

## 下载方式

在仓库根目录运行：

```powershell
powershell -ExecutionPolicy Bypass -File scripts/download-skybox-assets.ps1
```

下载结果：

```text
assets/skyboxes/downloads/directxtk-cubemap/
├── cubemap.dds
├── SOURCE_LICENSE.txt
└── SHA256SUMS.txt
```

脚本默认跳过已有的非空文件；使用 `-Force` 可重新下载。天空盒实现示例见
`docs/examples/skybox-pass.md`。

## 为什么先选 DDS cubemap

- DirectXTK 已提供 `CreateDDSTextureFromFile`，无需先引入图像解码和全景图转换流程。
- DDS 能直接保存 `TextureCube`、Mip 链和 GPU 像素格式，适合先学习 Pass、shader 和深度状态。
- 后续学习 HDR/IBL 时，再增加等距柱状 HDR 到 cubemap 的离线转换、辐照度卷积和预过滤环境图，
  不会干扰第一阶段的天空盒结构。

第三方素材的版权不会因下载到本项目而改变。再发布素材或派生内容前，应重新阅读上游许可。
