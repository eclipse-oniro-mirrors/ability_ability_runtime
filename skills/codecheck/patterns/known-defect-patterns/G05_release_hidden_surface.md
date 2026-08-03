# G5 user 版本隐藏面

> 来源：known-defect-patterns.md G5。

## 特征信号 / grep 线索

- CLI 工具的命令/参数解析表中存在文档未登记的命令或 flag；`--help` 输出与代码支持集不一致。
- user/RELEASE 版本保留调试入口、测试 hook、`-D` 类调试参数。
- "扫描仪驱动/debug 签名"类限制判断可被构造条件绕过。

## 历史案例

- `aa` 隐藏参数 `-s`（crash）、`-C`（冷启动）、`-c`（持续启动）、`stop-service` 隐藏参数、`force-stop` 杀非 debug 签名进程、`pre-start` 未文档化。
- `ability_tool -D` 对 release 签名应用进入调试模式；`bm` 存在未公开子命令。
- 双空间禁装 debug 签名驱动的判断被绕过。

## 检查点

- 导出代码支持的命令/参数全集，与对外文档 diff，未登记项全部上报。
- 调试/测试入口在 user 版本是否有编译期或运行期硬隔离？
