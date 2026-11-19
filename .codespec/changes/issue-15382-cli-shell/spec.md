# Spec

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | cli SA 沙箱 shell 命令执行与会话管理 |
| 特性编号 | FEAT-003-1 |
| 优先级 | P1 |
| 复杂度 | 标准（引用 proposal.md 初始分级） |
| 目标版本 | 7.0 |

## 用户故事或场景

### US-1: 应用以前台模式执行一组 shell 命令

**作为** 应用开发者，**我想要** 在同一沙箱内顺序执行一组 shell 命令（如 `cd workpath; git log`）并直接获取结果，**以便** 多条命令共享上下文且用完即销毁。

**验收标准：**

- **AC-1.1:** Given 应用已具备调用 cli SA shell 接口的合法身份 When 调用 ExecCmd(background=false, cmd="cd workpath; git log") Then 回调返回 CliSessionInfo(status=completed)，其 ExecResult.exitCode 为命令真实退出码、outputText 含 git log 输出
- **AC-1.2:** Given 前台命令执行中 When 命令执行完毕 Then shell 进程自动销毁，无需应用显式清理

### US-2: 应用以后台模式执行并与长驻 shell 交互

**作为** 应用开发者，**我想要** 以后台模式发起命令立即获得 sessionId，随后注入交互输入、查询结果并清理会话，**以便** 支持交互式长会话。

**验收标准：**

- **AC-2.1:** Given 应用具备调用身份 When 调用 ExecCmd(background=true) Then 接口立即返回 sessionId（不等命令执行结束）
- **AC-2.2:** Given 已有后台 sessionId When 应用 SubscribeSession(sessionId) 并 SendMessage(sessionId, inputText) Then 经回调收到 CliSessionInfo（含执行结果/交互响应），交互结果正确
- **AC-2.3:** Given 后台会话运行中 When 应用调用 ClearSession(sessionId) Then shell 进程被销毁，后续 QuerySession 返回 failed/不存在

## 业务规则

| 规则 ID | 规则描述 | 约束条件 | 关联 AC |
|---------|----------|----------|---------|
| BR-1 | 前台/后台由 ExecOptions.background 布尔区分 | 单一 ExecCmd 入口 | AC-1.1/AC-2.1 |
| BR-2 | policy 透传不解释语义 | ExecCmdParam.policy → GenerateCmdSandboxConfig，接口层不解析 | AC-1.1/AC-2.2 |
| BR-3 | tokenId 设置顺序固定 | fork 后先 ATM VerifyAccessToken 再 SetParentHapTokenId | AC-1.1/AC-2.2 |
| BR-4 | 并发清理唯一 owner | TryClaimCleanup 原子认领 | AC-2.3 |
| BR-5 | 仅动态接口 | 静态接口 7.1 另立 | 全部 |

## 异常与边界规则

| 编号 | 场景 | 触发条件 | 系统行为 | 关联 AC |
|------|------|----------|----------|---------|
| EX-1 | 命令超时 | options.timeout>0 且执行超时 | ExecResult.timeout=true，终止进程 | AC-1.1 |
| EX-2 | 输出超 64KB | stdoutText/stderrText 超 MAX_BUFFERED_OUTPUT_BYTES(65536) | TrimBufferedOutput 截断，不保证完整 | AC-1.1/AC-2.2 |
| EX-3 | 重复清理 | 多路径同时 ClearSession 或进程退出与 Clear 并发 | TryClaimCleanup 仅首个 owner 执行 kill | AC-2.3 |
| EX-4 | sessionId 不存在 | Query/Send/Clear 不存在 sessionId | 返回错误码（见 EC-2） | AC-2.3 |
| EX-5 | 非法调用方 | 调用方不具备合法身份/tokenId 校验失败 | 返回权限错误码（见 EC-3） | AC-1.1/AC-2.1 |

## 错误码定义

| 错误码 ID | 错误码值 | 含义 | 关联 AC |
|-----------|----------|------|---------|
| EC-1 | ERR_OK (0) | 成功 | 全部 |
| EC-2 | ERR_SESSION_NOT_FOUND | sessionId 不存在或已清理 | AC-2.3 |
| EC-3 | ERR_PERMISSION_DENIED | 调用方身份/tokenId 校验失败 | AC-1.1/AC-2.1 |
| EC-4 | ERR_NOT_SA_CALLER | BatchQueryPermissionBySubCommand 非 SA 调用 | - |
| EC-5 | ERR_CLI_TOOL_MGR_SERVICE_UNAVAILABLE | cli SA 未就绪/获取失败 | 全部 |

## 接口变更分析

### 新增接口

| 接口名称 | 开放级别 | 参数概要 | 返回值 | 错误码 | 关联 AC |
|----------|----------|----------|--------|--------|---------|
| ExecCmd | Inner | ExecCmdParam(cmd/workDir/env/policy/options), ExecToolReplyCallback, SessionEventCallback | ErrCode | EC-1/EC-3/EC-5 | AC-1.1/AC-2.1 |
| SubscribeSession | Inner | sessionId, SessionEventCallback, out subscriptionId | ErrCode | EC-1/EC-2 | AC-2.2 |
| UnsubscribeSession | Inner | sessionId, subscriptionId | ErrCode | EC-1/EC-2 | AC-2.2 |
| ClearSession | Inner | sessionId | ErrCode | EC-1/EC-2 | AC-2.3 |
| QuerySession | Inner | sessionId, out CliSessionInfo | ErrCode | EC-1/EC-2 | AC-2.3 |
| SendMessage | Inner | sessionId, inputText, EventReplyCallback | ErrCode | EC-1/EC-2 | AC-2.2 |
| BatchQueryPermissionBySubCommand | Inner | vector<Command> cmds, out vector<CommandPermission> | int32_t | EC-1/EC-4 | - |

### 变更/废弃接口

无变更/废弃接口（全新能力）。

## 兼容性声明

- **已有 API 行为变更:** 否
- **配置文件格式变更:** 否
- **数据存储格式变更:** 否

## 验证映射

| AC | 关联规则 | 验证方式 | 证据 |
|----|----------|----------|------|
| AC-1.1 | BR-1/BR-2/BR-3 | demo ExecCmd(background=false) | CliSessionInfo.status=completed, ExecResult 正确 |
| AC-1.2 | BR-1 | 前台命令后查询进程 | shell 进程已销毁 |
| AC-2.1 | BR-1 | demo ExecCmd(background=true) | 立即返回 sessionId |
| AC-2.2 | BR-2/BR-3 | Subscribe+SendMessage | 回调 CliSessionInfo 正确 |
| AC-2.3 | BR-4 | ClearSession 后 QuerySession | 进程销毁/Query 返回 failed |

## 测试设计提示

| AC | 测试类型 | 测试文件 | 测试名称 | 输入/触发 | 期望输出/错误 | Red 条件 |
|----|----------|----------|----------|-----------|---------------|----------|
| AC-1.1 | 集成 | demo | ExecCmdForeground | background=false, cmd="echo hi" | status=completed, outputText 含 hi | ExecCmd 未实现前接口不可用 |
| AC-2.1 | 集成 | demo | ExecCmdBackgroundImmediate | background=true | 立即返回 sessionId | 未实现前无 sessionId 返回 |
| AC-2.2 | 集成 | demo | SubscribeAndSend | sessionId+input | 回调 CliSessionInfo 正确 | SendMessage 未实现前回调不触发 |
| AC-2.3 | 集成 | demo | ClearSessionKill | ClearSession | 进程销毁 | ClearSession 未实现前进程残留 |
| - | 单元 | `cli_tool_framework/test/unittest/tool_util_test/tool_util_test.cpp` | GenerateCliSessionId_0100 | name, nullptr | sessionId 非空且以 name 开头 | 未实现返回空 |
| - | 单元 | 同上 | GenerateCmdSandboxConfig_0100 | 非 hap token | 返回 false, sandboxConfig 空 | 未实现返回 true 或崩溃 |

> AC → 实现文件 + Task + 验证状态的映射见 `execution-plan.md`「AC 到 Task 追溯」+「代码范围映射」。
