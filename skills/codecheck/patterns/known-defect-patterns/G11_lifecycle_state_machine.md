# G11 生命周期状态机与后台拉起管控

> 来源：known-defect-patterns.md G11。

## 特征信号 / grep 线索

- `onPageHide` / `onDestroy` / `onBackground` 等生命周期回调中存在 `startAbility` / `wantAgent` 拉起链路。
- 拉起链路（`StartAbilityInner`、`ConnectAbility` 等）无递归深度计数/循环检测，可被构造闭环互相拉起。
- 应用退至后台时，状态切换非原子（窗口状态与 Ability 状态不同步），可"窗口已隐藏但 Ability 仍在前台"的瞬时态被利用。
- 杀进程/清理后台逻辑依赖应用主动上报（如 `APP_APPLICATION_TERMINATED`）或仅发 `SIGTERM`，无强制兜底。
- `RestartApp`、`force-stop` 等接口未校验应用是否真正处于可被杀死的终端状态。

## 历史案例

- 毒王霸屏：恶意应用利用 `onPageHide` 回调中 `wantAgent` 拉起自身，或两个 Ability 死循环互相拉起 → 进程杀不死、无限弹框、桌面卡死冻屏，只能重启。
- `RestartApp` 在校验应用是否处于后台状态时不严格，应用可在被清理瞬间调用该接口重新拉起自身。
- 攻击者劫持 `APP_APPLICATION_TERMINATED` 上报接口 → 清空后台时目标 app 免杀；当该 app 处于最后一个后台时，用户手工清理需强制杀两次。
- 恶意应用借助伪造的长时任务（如数据传输任务）在后台任意时间弹广告窗，元能力 ability 后台管控策略弱于 Android 前台服务限制。
- `want` 的 `debug` 字段可被外部篡改 → 绕过应用启动超时机制。

## 检查点

- 所有生命周期回调中拉起 Ability 的行为是否被后台管控策略拦截？
- 拉起链路是否有递归深度上限（如 >3 层即拒绝）或闭环检测？
- 杀进程逻辑是否不依赖应用主动上报？是否有 `SIGKILL` 强制兜底 + 内核级 cgroup 清理？
- `RestartApp`、`force-stop` 等管理接口是否校验应用真实状态（非正在清理/非前台）？
