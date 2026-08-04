# G2 鉴权遗漏分支

> 来源：known-defect-patterns.md G2。

## 特征信号 / grep 线索

- Stub 的 `OnRemoteRequest`/`OnRemoteRequestEx` 中部分 code 分支无权限校验直接进业务。
- 白名单判断存在可绕过的类型/前缀分支（如以 AGENT 类型扩展绕过 SA 白名单）。
- 跨用户（multi-user / 跨空间）场景只校验调用者身份，未校验目标资源所属 userId。
- 组件 `exported=false`、调试开关（开发者模式/USB 调试/框架调试开关）关闭时仍可被调用。

## 历史案例

- `GetApplicationInfo` 等查询接口缺跨用户校验 → 跨用户应用数据泄露。
- `CliToolManagerService::ExecTool` 存在未鉴权路径 → 任意 Hap 以 CLI-SA 身份执行任意 Skill。
- `StartUIExtensionAbility`/`ConnectUIExtensionAbility` 未校验调用者 → 跨用户连接任意 App 组件。
- 拉起 invisible 组件时对 SA 放通 → 任意 SA 拉起非导出 Ability。
- 意图调试 hap 不校验开发者模式/USB 调试/调试开关 → 三方应用直接调用。

## 检查点

- 逐 code/逐接口核对：每个对外 IPC 入口是否都有权限声明？默认是否拒绝？
- 白名单/豁免分支能否被构造输入绕过？
- userId 是否始终取自服务端上下文而非客户端参数？
