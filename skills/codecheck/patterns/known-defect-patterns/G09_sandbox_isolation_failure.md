# G9 沙箱隔离失效

> 来源：known-defect-patterns.md G9。

## 特征信号 / grep 线索

- 创建子进程/沙箱未配置 pid 等 namespace 隔离。
- 终止沙箱进程仅发 `SIGTERM`（可被忽略/捕获），无 `SIGKILL` 兜底。
- fork/exec 前未关闭继承的 fd（pipefd 遗留）。
- 会话（session）标识可被猜测或未绑定创建者身份。

## 历史案例

- CLI-SA 拉起 claw_sandbox 未隔离 pid namespace → killpg 超时机制失效，沙箱进程长期驻留。
- killpg 用 SIGTERM → 恶意进程忽略信号不被杀。
- 父进程遗留 pipefd → 沙箱内进程接管其他 Session 输入输出。
- sessionId 泄露 → 跨应用 CLI 会话劫持；全局变量竞争 → 跨应用泄露 Skill 执行结果。
- nativespawn 孵化进程沙箱问题。

## 检查点

- 沙箱创建五件套：namespace 隔离、fd 关闭清单、SIGKILL 兜底、会话所有权绑定 uid、最小权限。
- 会话 id 是否不可猜测、是否校验所有权？
