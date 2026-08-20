# LRenderDemo 代理开发规则

以下项目规则将用户提供的《工程开发通用标准（增强版）v2.1》应用于 C++/CMake 工程。

1. 修改代码前，先阅读 `FILE_INDEX.md` 和 `CHANGELOG.md` 顶部。
2. 根据依赖关系图识别受影响的模块。
3. 生产源码文件尽量控制在 300 行左右，测试文件不超过 400 行。
4. 临时诊断文件放入 `scratch/`；运行时诊断日志放入 `logs/`。
5. 不得硬编码绝对路径、凭据、API 密钥或与特定机器绑定的 SDK 路径。
6. COM 对象使用 `ComPtr` 管理所有权，其他对象使用 RAII 或值语义管理。
7. 每个失败都必须补充上下文、在应用边界记录日志，或被有意设计为可恢复错误。除已记录的
   致命日志回退路径外，禁止使用空的 `catch` 块。
8. 提交前运行 `cmake --build --preset vs2022-debug` 和 `ctest --preset vs2022-debug`。
9. 适用时更新 `FILE_INDEX.md`、`CHANGELOG.md`、`LESSONS.md` 和 `SKILLS_USED.md`。
10. 在 `dev` 或 `feat/*` 分支工作；未经用户明确批准，不得合并或推送到 `main`。
