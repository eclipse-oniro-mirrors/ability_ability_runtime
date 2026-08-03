# G1 身份信任错误

> 用途：安全审计时对历史高发模式做"同类排查"。发现一处，全库横扫。
> 来源：known-defect-patterns.md G1。

## 特征信号 / grep 线索

- `getpid` / `GetCallingPid` / `pid` 作为身份或权限判断依据；未校验 pid 所属 uid/token 是否变化。
- 从客户端传入的 `tokenId` / `fullTokenId` / `specifiedTokenId` 直接采信，未经服务端映射校验。
- 依赖 `binder death recipient` / fd 状态推断对端存活或身份。
- 序列化端与反序列化端对同一参数的处理逻辑不一致（走私攻击面）。

## 历史案例

- `UninstallAppInner` 用 pid 校验，pid 回绕 → 卸载任意应用。
- `ExecuteIntentForDistributed` 的 `specifiedFullTokenId` 外部可控 → 任意 token 身份伪造。
- AMS tokenId 注入 → Launch-Any-Page；任意进程 tokenId 可提取 → 仿冒系统应用下发指令。
- appmgr 完全信任 binder death notification → close fd 实现进程保活。
- CLI-SA 与沙箱间序列化/反序列化参数处理不一致 → 命令行参数走私。

## 检查点

- 所有身份判断是否最终落到 uid/token（服务端可验证），而非 pid/自报字段？
- 跨进程传递的 tokenId 是否经 `AccessTokenKit` 类机制验真？
- 进程存活/会话有效性是否依赖可被对端操纵的信号？
