# FryHalftone 产品交付 v0.3.0

**发给产品同事**：[FryHalftone-v0.3.0.zip](FryHalftone-v0.3.0.zip) 和 [ZIP 校验值](FryHalftone-v0.3.0.zip.sha256)。

本目录同时提供 [已展开的产品包](FryHalftone-v0.3.0/README.md)。库与 CLI 按操作系统和处理器架构存放在 [platforms](FryHalftone-v0.3.0/platforms/) 中。

- 平台开发人员请读 [CLI 与动态库接入](FryHalftone-v0.3.0/docs/integration.md) 和 [27 项参数](FryHalftone-v0.3.0/docs/parameters.md)。
- 自动化部署工具读取 [PRODUCT.json](FryHalftone-v0.3.0/PRODUCT.json)，以 product_version=0.3.0 选择交付包，以 runtime_version=0.2.0 检查库的 GetVersion()。
- 原生五机结果、真实模型和角度差异见 [验收说明](FryHalftone-v0.3.0/docs/acceptance.md)；最终 ZIP 的本机验证结果见 [验收回执](acceptance_receipt.json)。

历史二进制取自 2026-11-09 00:32 PST 的 mac 修正批。随包保留 SDK 许可与原文件摘要。
