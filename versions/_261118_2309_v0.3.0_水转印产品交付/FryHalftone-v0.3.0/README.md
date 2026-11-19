# FryHalftone 水转印加网 · 产品交付 v0.3.0

把一个彩色 GLB 转成打印机使用的逐层 4-bit BMP。每个目标目录都含可直接运行的命令行和可链接的动态库。产品交付编号为 **v0.3.0**，随附的 C++ SDK 和 CLI 运行时版本为 **0.2.0**。在库里调用 `GetVersion()`，预期得到 `0.2.0`。产品号从 [PRODUCT.json](PRODUCT.json) 读取。

**English.** Delivery v0.3.0 contains the verified SDK runtime v0.2.0. Select the directory for the worker's OS and CPU. Run the CLI with a GLB model, a fresh output directory and one target size.

## 选哪个文件夹

|部署环境|应取的完整目录|库|命令行|
|---|---|---|---|
|Windows 10+ x64|[win32-x64](platforms/win32-x64/)|bin/fry_halftone.dll|bin/fry-halftone-cli.exe|
|Linux x86-64|[linux-x64](platforms/linux-x64/)|bin/libfry_halftone.so|bin/fry-halftone-cli|
|Linux AArch64|[linux-arm64](platforms/linux-arm64/)|bin/libfry_halftone.so|bin/fry-halftone-cli|
|macOS Apple Silicon|[darwin-arm64](platforms/darwin-arm64/)|bin/libfry_halftone.dylib|bin/fry-halftone-cli|
|macOS Intel|[darwin-x64](platforms/darwin-x64/)|bin/libfry_halftone.dylib|bin/fry-halftone-cli|

按**运行进程架构**选择。macOS 包记录的最低版本为 13.0，Linux 库报告的最高 glibc 符号版本为 2.17。实际受测系统见[验收记录](docs/acceptance.md)。

## 五分钟首次出片

将产品 ZIP 解压到可写目录，进入内层含本 README 的目录。以下命令使用附带的顶点色小盒。每次运行请选择一个新的输出目录。

**Windows PowerShell**

```powershell
$cli = '.\platforms\win32-x64\bin\fry-halftone-cli.exe'
& $cli info --json
& $cli slice .\examples\vertex_gray_box.glb --target-longest-edge 4 --out .\first-slices --json
```

**Linux 或 macOS 终端**（示例为 Apple Silicon，其他目标按表修改变量）

```sh
target=darwin-arm64
cli="./platforms/$target/bin/fry-halftone-cli"
"$cli" info --json
"$cli" slice ./examples/vertex_gray_box.glb --target-longest-edge 4 --out ./first-slices --json
```

如果解压工具丢弃了 Unix 执行权限，运行 `chmod +x "$cli"`。成功时最后一条 JSON 事件中有 `result.success=true`。打开 `first-slices` 可见从 `000000.bmp` 开始的连续层和 `slice_metadata.json`。此快速命令使用默认 HE500。要同时核对预设、内容和一个错误输入，可在产品根运行 `python -X utf8 check_install.py`（Linux/macOS 可用 `python3`）。此步骤需要 Python 3.9+ 标准库，成功的 HE520 对照件为 109 层。

## 接入资料

|读者|入口|用途|
|---|---|---|
|服务端、桌面端|[接入手册](docs/integration.md)|CLI 与 C ABI、JSON 事件、错误码、并发和取消|
|产品、工艺和表单开发|[27 项参数](docs/parameters.md)|范围、单位、默认值、预设联动|
|AI 与自动化服务|[产品清单](PRODUCT.json)、[运行时参数合同](platforms/win32-x64/params_schema.json)、[示例请求](examples/request.json)|精确字段和资源相对路径。五套参数合同相同|
|测试和运维|[验收记录](docs/acceptance.md)、[更新与回退](docs/operations.md)|目标环境、模型结果、已知差异与回退|
|交付负责人|[变更说明](CHANGELOG.md)、[许可证](LICENSE.txt)|确认产品边界与授权|

产品根 `SHA256SUMS.txt` 覆盖包内文件（校验表自身除外），每个平台也保留原 SDK 的独立交付清单和校验工具。外层 ZIP 摘要在旁边的 `.sha256` 文件中。产品接收方可先核对 ZIP，部署方再核对所选目录和运行时身份。
