# cgltf 依赖说明

本目录内置 `cgltf` 的 C 源码，用于解析静态 glTF 2.0 与 GLB 模型。

- 上游仓库：`https://github.com/jkuhlmann/cgltf`
- 固定提交：`360db1a95480fe102ae9c69b27c5d101167ff5ba`
- `cgltf.h` SHA-256：`F958CA31C945A216A59A499B7E787A1AA7321A1E50A0ED884DEA5CE76261E818`
- `LICENSE` SHA-256：`F619925F80EF862497AAF8E8155EF218FA6A2190055129523CA3DF9119A9BA95`
- 许可：MIT，原始条款见 `LICENSE`

项目仅移除了上游头文件中的一处空白行尾，源码语义未改变；通过 `cgltf.c` 定义实现宏并生成独立
静态库。更新版本时必须同时更新固定提交、规范化后的哈希、许可文件，并重新验证 Suzanne `.gltf`
与 Damaged Helmet `.glb`。
