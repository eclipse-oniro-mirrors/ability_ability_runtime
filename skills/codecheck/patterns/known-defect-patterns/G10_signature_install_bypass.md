# G10 签名/安装管控绕过

> 来源：known-defect-patterns.md G10。

## 特征信号 / grep 线索

- 代码签名/验签流程存在"豁免"配置（ownerid 豁免等）。
- 解析可执行文件（abc/zip）时未按签名保护的信息定位代码段（offset 外部可控）。
- 调试/工具链可加载未签名或可执行内容。
- 证书/profile 解析缺少对根 CA 信任链的校验。
- 降级安装路径（allowPatchDowngrade 类）无权限校验。

## 历史案例

- `app_bin_file` 配置 ownerid 豁免 → 签名保护整体失效，叠加 `LD_PRELOAD` 提权。
- ArkTS 未按签名保护信息解析 abc 在 zip 中的 offset → MAP_XPM 签名机制与云端 Verifier 双绕过。
- `ohos-arkTSScript` CLI 无权限保护，直接加载运行未签名 abc → 代码签名绕过。
- installd 调 key_enable 解析 profile 缺 pkcs7 根 CA 信任校验 → 伪造自签名 profile 注册进内核，任意 App"合法"签名构造。
- `allowPatchDowngrade` 无权限校验 → hdc shell 降级安装任意应用。

## 检查点

- 信任链每个环节（验签→解析→加载→执行）是否都有不可绕过的校验？
- 所有"豁免/特批"配置逐项上报评审。
