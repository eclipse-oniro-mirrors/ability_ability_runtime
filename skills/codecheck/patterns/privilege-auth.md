# 权限与鉴权模式 (Privilege & Auth)

> 所有 scanner 共享的权限/鉴权/敏感数据缺陷模式词典。

---

## AUTH-001 凭据管理缺陷

**信号特征**
- 公网地址、认证凭据（Token/Key）明文存储或打印。
- 用普通字符串保存密码密钥。
- 使用不安全加密算法。
- 会话凭据（challenge/sessionId/token）出现在命令行参数与日志中。

**grep 线索**
```bash
rg -n "password|secret|apiKey|token\s*=" --type cpp -i
rg -n "challenge|sessionId|cmdLine" --type cpp
rg -n "http://|https://" --type cpp
```

**典型后果**：凭据泄露，P0。凭据视同已泄露，需评估轮换。

**历史案例**：G7（`process_manager.cpp` 打印完整 cmdLine 含会话 challenge）。

---

## AUTH-002 日志红线关键字（逐条 grep 必查）

**信号特征**
- 日志打印以下关键字：`udid`、`networkid`、`SN`、`challenge`、`token`、`password`、完整 `cmdLine`、文件路径/名称、已安装应用包名。
- 敏感信息未从内存缓冲区清除（Memory Sanitization）。
- 接口/查询函数返回调用者无权知晓的信息。

**grep 线索**
```bash
rg -n "udid|networkid|SN|challenge|token|password|cmdLine" --type cpp
rg -n "HILOG_INFO|HILOG_WARN|HILOG_ERROR|TAG_LOG" --type cpp
rg -n "%\{public\}s" --type cpp
```

**检查点**
- 对红线关键字做全库 grep，逐处确认是否脱敏。
- 凭据类内容出现在日志中：除报代码问题外，提示"凭据视同已泄露，需评估轮换"。

**典型后果**：敏感信息泄露，P0。

**历史案例**：G7（`distributed_data_storage.cpp` 打印 udid、`ability_record.cpp`/`ui_ability_lifecycle_manager.cpp` 打印 networkid、aa/AMS 打印 SN 多版本复发、Context 打印文件名）。

---

## AUTH-003 权限管控不足

**信号特征**
- 敏感场景未充分校验访问权限。
- 仅使用 BundleName（包名）作为白名单校验。
- 未对 DataAbility/DataShareExtension 接口设置合理读写权限。
- **升级/更新路径上的权限保护回归必查**（历史案例：应用市场升级导致 DataShare read/writePermission 保护丢失）。
- 一个权限映射多个系统权限导致权限扩大化。

**grep 线索**
```bash
rg -n "bundleName|BundleName" --type cpp
rg -n "readPermission|writePermission" --type cpp
rg -n "CheckPermission|VerifyAccessToken|PermissionVerification" --type cpp
```

**典型后果**：越权访问，P0/P1。

**历史案例**：G8（`ohos.permission.cli.START_ABILITY` 含 `START_INVISIBLE_ABILITY` → 调起任意应用非暴露组件）。

---

## AUTH-004 环境残留与隐藏面（历史高发，重点）

**信号特征**
- RELEASE/user 版本二进制包含调试工具/接口、隐藏命令、未文档化参数。
- `--help` 输出与代码支持集不一致。
- 调试/测试 hook、`-D` 类调试参数在 user 版本保留。
- "扫描仪驱动/debug 签名"类限制判断可被构造条件绕过。

**审查方法**
- 导出代码支持的命令/参数全集，与对外文档 diff，所有未登记项一律上报。
- 对比 CLI 工具命令解析表与文档清单。

**grep 线索**
```bash
rg -n "getopt|argc|argv" --type cpp
rg -n "\-\-help|Usage:" --type cpp
rg -n "DEBUG|debug|TEST|test" --type cpp --glob '!*test*'
```

**典型后果**：隐藏攻击面、权限提升，P0/P1。

**历史案例**：G5（`aa -s` crash、`-C`/`-c` 冷启动/持续启动、`stop-service`/`force-stop` 隐藏参数、`ability_tool -D` 拉起 release 签名应用、`bm` 未公开子命令、`aa pre-start` 未文档化、双空间禁装 debug 签名驱动判断被绕过）。

---

## AUTH-005 SELinux 与文件权限

**信号特征**
- 数据库/服务目录 SELinux 标签与 DAC 权限过大（攻破低权限服务即可篡改开机自启/保活）。
- CLI/bin 工具无独立 SELinux 标签（shell 无权限直接调用）。
- 新增 SELinux 权限/豁免项未上报评审。

**典型后果**：提权、篡改系统配置，P0/P1。

**历史案例**：G8（`auto_startup_service.db`/`ability_manager_service.db`/`bmsdb.db` 标签过大、小艺 Claw CLI 无独立标签）。

---

## AUTH-006 系统公共事件与隐私指示器

**信号特征**
- 订阅非系统公共事件未充分管控（被三方仿冒风险）。
- 系统公共事件定义后未加入系统公共事件列表（历史多次重犯）。
- 使用隐私指示器未判断返回值。

**grep 线索**
```bash
rg -n "COMMON_EVENT|CommonEvent" --type cpp
rg -n "PrivacyIndicator|privacyIndicator" --type cpp
```

**典型后果**：仿冒、隐私泄露，P1。

---

## AUTH-007 硬编码白名单（机制设计缺陷）

**信号特征**
- 代码中硬编码包名/uid/服务名白名单（如 `"com.ohos.camera"`、`"com.ohos.launcher"`），未走配置化或动态鉴权。
- `startAbility`/`ConnectAbility` 入口对 `want` 中 `resv` 字段、`callerNativeName`、`debug` 等参数未在服务端 reset/校验，直接透传。
- 多个鉴权维度由单一服务集中管控形成单点失效。
- 权限框架与业务代码耦合过深。

**grep 线索**
```bash
rg -n "\"com\.ohos\.\w+\"" --type cpp
rg -n "resv|callerNativeName|debug" --type cpp
```

**检查点**
- 白名单是否全部走配置化并支持动态审计？硬编码即上报。
- `startAbility` 入口是否对 want 中所有非业务必要字段做 reset/过滤？
- 鉴权 token 的生成/分发/校验是否分散到不同信任域？

**典型后果**：绕过鉴权、身份伪造，P0/P1。

**历史案例**：G13（`UriPermissionManagerService` 硬编码 19 个系统应用包名白名单 → 无需授权访问任意应用私有目录；15 个 `resv` 字段未 reset；`callerNativeName` 未 reset → 跨调用链伪造 caller；`want.debug` 字段被篡改绕过启动超时）。

---

## AUTH-008 签名/安装管控绕过

**信号特征**
- 代码签名/验签流程存在"豁免"配置（ownerid 豁免等）。
- 解析可执行文件（abc/zip）时未按签名保护信息定位代码段（offset 外部可控）。
- 调试/工具链可加载未签名或可执行内容。
- 证书/profile 解析缺少根 CA 信任链校验。
- 降级安装路径无权限校验。

**检查点**
- 信任链每个环节（验签→解析→加载→执行）是否都有不可绕过的校验？
- 所有"豁免/特批"配置逐项上报评审。

**典型后果**：任意代码执行、签名绕过，P0。

**历史案例**：G10（`app_bin_file` ownerid 豁免 + `LD_PRELOAD` 提权、ArkTS abc offset 外部可控绕过 MAP_XPM + 云端 Verifier、`ohos-arkTSScript` CLI 加载未签名 abc、installd profile 缺 pkcs7 根 CA 校验 → 伪造自签名 profile、`allowPatchDowngrade` 无校验 → hdc shell 降级安装）。
