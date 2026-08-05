# 特性规格

> 本规格仅覆盖 `execTool` 接口行为。范围限定：`execTool` 的用户可见行为、参数契约、错误码、会话生命周期与超时/让出/后台语义。`execCmd`、`SubscribeSession`、`SendMessage`、`RegisterFunction` 等同模块其他接口不在本规格范围内。
>
> **输入文档说明：** 工作区未检索到 `REQ-010` 需求条目与已批准的 `proposal.md`。本规格依据 `cli_tool_framework` 中 `execTool` 的既有实现（`frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp`、`services/climgr/src/cli_tool_manager_service.cpp`、`interfaces/cli_tool/`）反推固化。待 `proposal.md` 基线化后，以下「优先级/目标版本/SIG 归属」字段应回填 proposal 值。

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | 执行 CLI 工具（execTool） |
| 特性编号 | FEAT-CLI-001 |
| 所属 Epic | REQ-010 |
| 优先级 | P1 |
| 目标版本 | 参见 proposal.md（待基线） |
| SIG 彾属 | Sig-Ability |
| 状态 | Draft |
| 复杂度 | 标准 |

## 本次变更范围（Delta）

本特性为新增能力（lineage: new）。`execTool` 为 `cli_tool_framework` 对外暴露的 System API，此前不存在对外契约。

| 类型 | 内容 | 说明 |
|------|------|------|
| ADDED | `execTool` System API（JS） | 系统应用按名称执行已注册 CLI 工具，返回会话信息 |
| ADDED | 错误码 35700000–35700008、35700020 | 见 cli_error_code.h，本规格仅声明 execTool 链路涉及项 |
| ADDED | 权限 `ohos.permission.EXEC_CLI_TOOL` | execTool 调用前置权限 |
| ADDED | 数据结构 `ExecOptions`/`CliSessionInfo`/`ExecResult` | 对外行为契约，非内部实现 |

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Requirement | `REQ-010`（未在工作区检索到） | Pending |
| Proposal | `proposal.md`（未提供） | Pending |

> 需求基线、不涉及项、受影响子系统与仓库详见 proposal.md，本文档不重复摘录。design.md 与本文档并行产出，互不依赖。

## 用户故事

### US-1: 系统应用执行已注册的 CLI 工具

**作为** 系统应用开发者,
**我想要** 通过 `execTool` 接口按名称执行已注册的 CLI 工具并获取执行会话信息,
**以便** 在系统应用内集成受沙箱约束的命令行工具能力。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-1.1 | WHEN 系统应用持 `ohos.permission.EXEC_CLI_TOOL` 调用 `execTool`，传入已注册的 `toolName`、工具支持的 `subcommand`、符合工具 `inputSchema` 的 `args`、非空 `challenge` THEN 接口返回 Promise，工具执行结束后 resolve 为 `CliSessionInfo`（`status="completed"`，`result.exitCode` 等于子进程退出码） | 正常 |
| AC-1.2 | WHEN `options.background=false` 且 `options.yieldMs>0` THEN 在 `yieldMs` 毫秒后 resolve 为 `status="running"` 的 `CliSessionInfo`，会话转入后台继续运行 | 行为 |
| AC-1.3 | WHEN `options.background=true` THEN 接口立即 resolve 为 `status="running"` 的 `CliSessionInfo`，不等待执行结束 | 行为 |
| AC-1.4 | WHEN 进程持续运行至 `options.timeout` 秒 THEN 进程被终止、最终会话 `result.timeout=true`，并向会话订阅者派发超时错误事件 | 边界 |
| AC-1.5 | WHEN 非系统应用调用 `execTool` THEN Promise reject 错误码 `35700008`（ERR_NOT_SYSTEM_APP） | 异常 |
| AC-1.6 | WHEN 系统应用未持有 `ohos.permission.EXEC_CLI_TOOL` THEN Promise reject 错误码 `35700007`（ERR_PERMISSION_DENIED） | 异常 |
| AC-1.7 | WHEN `toolName` 未在系统中注册 THEN Promise reject 错误码 `35700005`（ERR_TOOL_NOT_EXIST） | 异常 |
| AC-1.8 | WHEN 传入非空 `subcommand` 但工具未声明子命令，或 `subcommand` 不在工具子命令列表中 THEN Promise reject 错误码 `35700005`（ERR_TOOL_NOT_EXIST） | 异常 |
| AC-1.9 | WHEN `args` 含工具 `inputSchema` 未定义的键，或某键值类型与 schema 声明不符 THEN Promise reject 错误码 `35700002`（ERR_INVALID_PARAM） | 异常 |
| AC-1.10 | WHEN `options.timeout<0` 或 `options.timeout>1800` THEN Promise reject 错误码 `35700002`（ERR_INVALID_PARAM） | 边界 |
| AC-1.11 | WHEN `options.background=false` 且 `options.yieldMs>options.timeout*1000` THEN Promise reject 错误码 `35700002`（ERR_INVALID_PARAM） | 边界 |
| AC-1.12 | WHEN `toolName` 为空字符串/非字符串、`challenge` 为空字符串/非字符串、`args` 非对象，或实参个数小于 4 THEN 同步抛出 invalid parameter 异常，不进入异步链路 | 边界 |
| AC-1.13 | WHEN 调用时系统并发会话数已达上限 THEN Promise reject 错误码 `35700001`（ERR_SESSION_LIMIT_EXCEEDED） | 边界 |
| AC-1.14 | WHEN 调用方进程非 HAP THEN Promise reject 错误码 `35700003`（ERR_NOT_HAP） | 异常 |
| AC-1.15 | WHEN CliToolManager 系统能力未就绪或代理获取失败 THEN Promise reject 错误码 `35700000`（GET_CLI_TOOL_MGR_SERVICE_FAILED） | 恢复 |

## 验收追溯

| AC | 关联规则 | 关联 Task | 验证方式 | 证据 |
|----|----------|-----------|----------|------|
| AC-1.1 | R-1 | TASK-CLI-001 | 集成 + 单测 | `cli_tool_framework/test/unittest/process_manager_test/` |
| AC-1.2 | R-2 | TASK-CLI-002 | 集成 | `cli_tool_framework/test/` |
| AC-1.3 | R-3 | TASK-CLI-002 | 集成 | `cli_tool_framework/test/` |
| AC-1.4 | R-4 | TASK-CLI-003 | 集成 | `cli_tool_framework/test/` |
| AC-1.5 | R-5 | TASK-CLI-004 | 单测 | `cli_tool_mgr_client_test/` |
| AC-1.6 | R-5 | TASK-CLI-004 | 单测 | `cli_tool_mgr_client_test/` |
| AC-1.7 | R-6 | TASK-CLI-005 | 单测 | `cli_tool_mgr_client_test/` |
| AC-1.8 | R-6 | TASK-CLI-005 | 单测 | `cli_tool_mgr_client_test/` |
| AC-1.9 | R-7 | TASK-CLI-005 | 单测 | `exec_tool_param_test/` |
| AC-1.10 | R-8 | TASK-CLI-005 | 单测 | `exec_options_test/` |
| AC-1.11 | R-8 | TASK-CLI-005 | 单测 | `exec_options_test/` |
| AC-1.12 | R-9 | TASK-CLI-006 | 单测 | `js_cli_manager` NAPI 单测 |
| AC-1.13 | R-10 | TASK-CLI-007 | 集成 | `cli_tool_framework/test/` |
| AC-1.14 | R-11 | TASK-CLI-004 | 集成 | `cli_tool_framework/test/` |
| AC-1.15 | R-12 | TASK-CLI-008 | 集成 | `cli_tool_mgr_client_test/` |

## 规则定义

| 规则ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联AC |
|--------|------|----------|----------|-----------|--------|
| R-1 | 行为 | 系统应用 + 持 EXEC_CLI_TOOL + toolName 已注册 + subcommand 合法 + args 匹配 inputSchema + challenge 非空 + options 合法 | 创建会话与受沙箱子进程执行工具，结束后 resolve `CliSessionInfo{status="completed", result.exitCode=进程退出码, outputText, errorText, executionTime}` | 退出码为 int32；executionTime 为 int64 毫秒 | AC-1.1 |
| R-2 | 行为 | foreground(yieldMs>0) 模式 | 会话在 `yieldMs` 毫秒处派发 reply，resolve 为 `status="running"` 的会话；会话切后台继续运行；后续结束通过会话事件（订阅）派发 | yieldMs 单位毫秒，int64 | AC-1.2 |
| R-3 | 行为 | background=true 模式 | 立即 resolve 为 `status="running"` 的会话；不调度 yield 任务；结束时向订阅者派发退出事件 | — | AC-1.3 |
| R-4 | 边界 | 进程运行时间 = options.timeout 秒（timeout>0） | 终止进程、`result.timeout=true`、向订阅者派发 "session timed out" 错误事件；若此前未后台则派发终态 reply | timeout 单位秒，上限 1800（=30 分钟）；=0 表示不启用超时 | AC-1.4 |
| R-5 | 异常 | 调用方 Token 非系统应用，或未持 EXEC_CLI_TOOL 权限 | reject：非系统应用→35700008；缺权限→35700007 | 校验在入口执行 | AC-1.5, AC-1.6 |
| R-6 | 异常 | toolName 未注册；或 subcommand 非空但工具无子命令/不在子命令表 | reject 35700005（ERR_TOOL_NOT_EXIST） | subcommand 为空串时使用工具顶层 inputSchema | AC-1.7, AC-1.8 |
| R-7 | 异常 | args 含 inputSchema 未声明的键，或键值类型与 schema.type 不符 | reject 35700002（ERR_INVALID_PARAM）；特殊键 `help` 单独存在时放行 | args 为空对象视为合法 | AC-1.9 |
| R-8 | 边界 | options.timeout<0 或 >1800；或非后台且 yieldMs>timeout*1000；或 yieldMs<0 | reject 35700002（ERR_INVALID_PARAM） | timeout∈[0,1800] 秒；yieldMs∈[0, timeout*1000] 毫秒（非后台） | AC-1.10, AC-1.11 |
| R-9 | 边界 | 实参个数<4；或 toolName/challenge 为空串或非字符串；或 args 非对象 | 同步抛出 invalid parameter 异常（不进入异步 Promise） | NAPI 层校验先于服务端 | AC-1.12 |
| R-10 | 边界 | 调用时活动会话数 ≥ 系统并发上限 | reject 35700001（ERR_SESSION_LIMIT_EXCEEDED） | 上限由系统配置 CcmUtil.GetCliConcurrencyLimit 决定 | AC-1.13 |
| R-11 | 异常 | 调用方 Token 非 HAP（无法生成沙箱配置） | reject 35700003（ERR_NOT_HAP） | 沙箱配置生成失败前置 | AC-1.14 |
| R-12 | 恢复 | CliToolManager SA 未加载或代理为空 | reject 35700000（GET_CLI_TOOL_MGR_SERVICE_FAILED） | 客户端按需拉起 SA；拉起失败即此码 | AC-1.15 |

## 验证映射

| 编号 | 对应规格项 | 验证方式 | 验证重点 |
|------|------------|----------|----------|
| VM-1 | R-1 / AC-1.1 | 集成 + 单测 | 正常执行返回 completed 会话与退出码 |
| VM-2 | R-2,R-3 / AC-1.2,AC-1.3 | 集成 | yield/background 模式 reply 时机与 status |
| VM-3 | R-4 / AC-1.4 | 集成 | 超时终止、result.timeout、错误事件派发 |
| VM-4 | R-5 / AC-1.5,AC-1.6 | 单测 | 系统应用判定与权限校验路径 |
| VM-5 | R-6,R-7,R-8 / AC-1.7–AC-1.11 | 单测 | 工具/子命令/schema/超时参数校验 |
| VM-6 | R-9 / AC-1.12 | 单测 | NAPI 参数校验同步抛错 |
| VM-7 | R-10,R-11,R-12 / AC-1.13–AC-1.15 | 集成 + 单测 | 会话上限、非 HAP、SA 不可用 |

## API 变更分析

### 新增 API

| API 名称 | 开放范围 | 入参概要 | 返回值 | 错误码范围 | 功能描述 | 关联 AC |
|----------|----------|----------|--------|------------|----------|---------|
| `cliTool.execTool` | System | `(toolName: string, subcommand: string, args: object, challenge: string, options?: ExecOptions)` | `Promise<CliSessionInfo>` | 35700000–35700008, 35700020 | 按名称执行已注册 CLI 工具，返回会话信息 | AC-1.1–1.15 |

> API 签名、d.ts 位置、权限要求等实现细节见 design.md。ExecOptions/CliSessionInfo/ExecResult 的字段语义见下「接口规格」。

### 变更/废弃 API

无变更或废弃项。

## 接口规格

### 接口定义

**execTool**

| 属性 | 值 |
|------|-----|
| 函数签名 | `execTool(toolName: string, subcommand: string, args: object, challenge: string, options?: ExecOptions): Promise<CliSessionInfo>` |
| 返回值 | `Promise<CliSessionInfo>` — resolve 为会话信息；reject 为错误码 |
| 开放范围 | System |
| 权限 | `ohos.permission.EXEC_CLI_TOOL`，且调用方须为系统应用 |
| 错误码 | 35700000, 35700001, 35700002, 35700003, 35700004, 35700005, 35700007, 35700008 |
| 关联 AC | AC-1.1–1.15 |

**参数约束**

| 参数 | 类型 | 必填 | 默认值 | 约束条件 |
|------|------|------|--------|---------|
| toolName | string | 是 | — | 非空；须为已注册工具名，否则 35700005 |
| subcommand | string | 是 | — | 可为空串（工具无子命令时）；非空时须在工具子命令表内，否则 35700005 |
| args | object（WantParams 键值对） | 是 | — | 键须在工具 inputSchema.properties 内且类型匹配；空对象合法；特殊键 `help` 须单独存在 |
| challenge | string | 是 | — | 非空 |
| options | ExecOptions | 否 | `{background:false, yieldMs:0, timeout:0}` | 见 ExecOptions 约束 |

**ExecOptions 约束**

| 字段 | 类型 | 默认 | 约束 |
|------|------|------|------|
| background | boolean | false | true=后台立即返回；false=前台，可由 yieldMs 触发让出 |
| yieldMs | number(int64) | 0 | ≥0，单位毫秒；仅 foreground 生效；foreground 下须 ≤ timeout*1000 |
| timeout | number(int64) | 0 | ∈[0,1800]，单位秒；0=不启用超时；timeoutMs=timeout*1000 |

**CliSessionInfo（resolve 值）**

| 字段 | 类型 | 说明 |
|------|------|------|
| sessionId | string | 会话唯一标识 |
| toolName | string | 工具名 |
| status | string | `"running"` / `"completed"` / `"failed"` |
| result | ExecResult \| undefined | 仅 completed/failed 时存在 |

**ExecResult**

| 字段 | 类型 | 说明 |
|------|------|------|
| exitCode | number(int32) | 子进程退出码，默认 1 |
| outputText | string | 标准输出累计 |
| errorText | string | 标准错误累计 |
| signalNumber | number(int32) | 终止信号，0 表示正常 |
| timeout | boolean | 是否因超时终止 |
| executionTime | number(int64) | 执行耗时（毫秒） |

**行为场景**

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | 前台 + timeout=0 + yieldMs=0 + 进程正常退出 | 进程退出且输出排空后 resolve `status="completed"`，含 exitCode/outputText | AC-1.1 |
| 2 | 前台 + yieldMs>0 | yieldMs 毫秒处 resolve `status="running"`，会话转后台；后续结束经订阅事件通知 | AC-1.2 |
| 3 | background=true | 立即 resolve `status="running"`；结束时向订阅者派发退出事件，不再二次 resolve | AC-1.3 |
| 4 | timeout>0 且进程未在 timeout 秒内退出 | 终止进程、`result.timeout=true`、派发 "session timed out" 事件；未后台则派发终态 reply | AC-1.4 |
| 5 | 非系统应用调用 | reject 35700008 | AC-1.5 |
| 6 | 系统应用缺权限 | reject 35700007 | AC-1.6 |
| 7 | toolName 未注册 / subcommand 非法 | reject 35700005 | AC-1.7, AC-1.8 |
| 8 | args 不符 inputSchema | reject 35700002 | AC-1.9 |
| 9 | options.timeout/yieldMs 越界 | reject 35700002 | AC-1.10, AC-1.11 |
| 10 | 必填参数缺失/类型错/实参<4 | 同步抛 invalid parameter 异常 | AC-1.12 |
| 11 | 会话数达上限 | reject 35700001 | AC-1.13 |
| 12 | 非 HAP 调用 | reject 35700003 | AC-1.14 |
| 13 | SA 不可用 | reject 35700000 | AC-1.15 |

## 兼容性声明

- **已有 API 行为变更:** 否。本特性为新增 System API，不改变既有公共 API 行为。
- **配置文件格式变更:** 否（沙箱配置为内部生成，非对外配置文件）。
- **数据存储格式变更:** 否。会话为运行态内存对象，不持久化。
- **最低支持版本:** 参见 proposal.md（待基线）。
- **API 版本号策略:** 新增 API 自首个引入版本起标注 `@since`；错误码 357000xx 为本特性专属区间，不可被其他特性复用数值。

## 架构约束

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| 仅系统应用 + `ohos.permission.EXEC_CLI_TOOL` 可调用 | 入口须做系统应用判定与权限校验 | AC-1.5, AC-1.6 |
| 执行须在受限沙箱子进程中进行 | 调用方须为 HAP 以生成沙箱配置 | AC-1.14 |
| 回复经 IPC Scheduler 异步派发 | 回调不在调用线程同步执行 | AC-1.1–1.4 |
| 并发会话受系统上限约束 | 超限拒绝，不排队 | AC-1.13 |

> 架构规则适用性及设计方案（SA 注册、IPC Stub/Proxy、沙箱生成、IOMonitor 调度等）见 design.md。

## 非功能性需求

| 类型 | 指标/阈值 | 验证方式 | 证据 |
|------|-----------|----------|------|
| 可靠性 | 单次 execTool 调用不应导致 CliToolManager SA 崩溃 | 压力测试 | `cli_tool_framework/test/` |
| 安全 | 非系统应用/无权限/非 HAP 调用均被拒绝 | 单测 | `cli_tool_mgr_client_test/` |
| 可测试性 | execTool 行为可经 mock 服务端校验错误码与回复时机 | 单测 | `cli_tool_mgr_client_test/`、`exec_options_test/` |
| 定界定位 | 失败路径经 HiSysEvent 上报（bundleName、toolName、failureReason/duration） | hisysevent | `hisysevent.yaml` |

## 多设备适配声明

| 设备类型 | 行为差异 | 规格/约束 | 验证方式 | 证据 |
|----------|----------|-----------|----------|------|
| 手机 | 无差异 | — | 集成 | `cli_tool_framework/test/` |
| 平板 | 无差异 | — | 集成 | `cli_tool_framework/test/` |
| 折叠屏 | 无差异 | — | 集成 | `cli_tool_framework/test/` |

> execTool 行为与设备形态无关；并发上限等可随产品配置调整，不影响行为语义。

## 全局特性影响

| 特性 | 适用？ | 结论 | 关联场景 |
|------|--------|------|----------|
| 无障碍 | 否 | execTool 为无界面 System API | — |
| 大字体 | N/A | — | — |
| 深色模式 | N/A | — | — |
| 多窗口/分屏 | 否 | 与窗口无关 | — |
| 多用户 | 否 | 权限与沙箱按调用方 Token 校验，不依赖用户切换 | — |
| 版本升级 | 否 | 无持久化数据需迁移 | — |
| 生态兼容 | 否 | 仅系统应用可用 | — |

## Spec 自审清单

在提交审查前逐项自检：

- [x] 无"待定""TBD""TODO"等占位符（"待基线"为上游 proposal 缺失的如实标注，非规格占位）
- [x] 所有 AC 使用 WHEN/THEN 格式，可独立测试
- [x] 范围边界明确（仅 execTool；execCmd 等不涉及）
- [x] 无语义模糊表述
- [x] AC 与规则表交叉一致（每个 AC 至少关联一条规则，每条规则至少关联一个 AC）
- [x] 规则表每条通过 5 项质量检查（可复现/可观测/边界值/关联AC/无冲突）

## context-references

```yaml
context-queries:
  - repo: "openharmony/ability_runtime"
    query: "cli_tool_framework execTool 的 SA 注册、IPC Stub/Proxy 序列化与沙箱生成实现"
  - repo: "openharmony/ability_runtime"
    query: "ExecToolParam/ExecOptions/CliSessionInfo 的 Parcelable 编解码分支"
```

**关键文档：**
- `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp`（NAPI 入口 `execTool`/`OnExecTool`）
- `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp`（服务端 `ExecTool`、超时/让出处理）
- `cli_tool_framework/services/climgr/src/tool_util.cpp`（参数与 inputSchema 校验）
- `cli_tool_framework/interfaces/cli_tool/include/cli_error_code.h`（错误码定义）
- `cli_tool_framework/interfaces/cli_tool/ICliToolManager.idl`（IPC 接口）
