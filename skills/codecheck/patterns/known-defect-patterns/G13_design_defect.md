# G13 机制设计缺陷（硬编码 / 字段生命周期 / 耦合）

> 来源：known-defect-patterns.md G13。

## 特征信号 / grep 线索

- 代码中存在硬编码的包名/uid/服务名白名单（如 `"com.ohos.camera"`、`"com.ohos.launcher"` 等），未走配置化或动态鉴权。
- `startAbility` / `ConnectAbility` 等入口对 `want` 中的 `resv` 字段、`callerNativeName`、`debug` 等参数未在服务端重置/校验，直接透传至下游。
- 多个鉴权维度（`AccessToken`、`SELinux`、xpm 标签、uid）由单一服务（如 BMS）集中管控，形成单点失效。
- 权限框架与业务代码耦合过深：修改 BMS 即可控制所有新启动应用的鉴权 token 和标志。

## 历史案例

- `UriPermissionManagerService` 硬编码 19 个系统应用包名白名单（相机、桌面、应用市场等）→ 这些 app 无需三方授权即可访问任意应用私有目录，现网已被恶意应用利用为跳板。
- 元能力 15 个 `resv` 字段在 `startAbility` 中未被 reset，与文档不符 → 应用开发者误信任这些字段，造成大批量生态应用无效鉴权。
- `callerNativeName` 字段在 `startAbility` 中未被 reset → 可跨调用链伪造 caller 身份。
- `want` 的 `debug` 字段可被外部篡改 → 绕过启动超时机制，攻击者可延长恶意行为窗口。
- 应用权限标志（AccessToken、SELinux 身份、xpm 标签、uid）均由 BMS 控制 → 控制 BMS 即可控制新启动应用的所有鉴权 token，不符合用户态鉴权模型。

## 检查点

- 白名单是否全部走配置化（如 json/xml）并支持动态审计？硬编码即上报。
- `startAbility` 等入口是否对 want 中所有非业务必要字段做 reset/过滤？
- 鉴权 token 的生成、分发、校验是否分散到不同信任域，避免单点服务被攻破后全盘失控？
- 设计文档与实际代码对字段生命周期的描述是否一致？
