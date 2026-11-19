---
target_release:
  id: OpenHarmony-6.0-Release
  status: proposed
---

# 需求文档

> 一份文档，从原始需求到基线结论。本需求 `REQ-010` 原始条目未在工作区检索到，以下内容依据 `cli_tool_framework` 中 `execTool` 既有实现反推并固化，待需求方/Owner 确认后基线。

## 一、原始需求

### 基本信息

| 字段 | 内容 |
|------|------|
| 需求ID | REQ-010 |
| 需求名称 | 执行 CLI 工具（execTool）System API |
| 来源 | 待确认（实现已存在于 cli_tool_framework，2026 版权） |
| 提出人 | 待确认 |
| 目标发行版本 | OpenHarmony-6.0-Release（待确认） |
| 候选 Profile | none |
| 优先级 | P1 |
| 状态 | Clarifying |

### 原始描述

**原始问题：** 系统应用需要一种受控、受沙箱约束的方式按名称执行系统中已注册的命令行工具（CLI Tool），并获取执行会话的状态与结果，而不应直接 fork 任意命令或绕过权限审计。

**痛点：**

| 用户类型 | 当前痛点 | 影响 |
|----------|----------|------|
| 系统应用开发者 | 无统一受控入口执行已注册 CLI 工具，需自建子进程与权限处理 | 集成成本高、安全风险大 |
| 系统安全/审计方 | 缺少统一的权限校验、沙箱隔离与失败上报 | 故障归因与合规审计困难 |
| 平台方 | 工具执行无并发上限与超时控制 | 易被滥用导致资源耗尽 |

**期望结果：** 系统应用通过一个 System API（`execTool`）执行已注册 CLI 工具，系统统一完成权限校验、沙箱子进程创建、超时/让出/后台会话管理与结果回传，并在失败路径经 HiSysEvent 上报。

### 背景证据

| 证据类型 | 链接/路径 | 说明 |
|----------|-----------|------|
| 源码实现 | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp` | NAPI 入口 `execTool`/`OnExecTool` 已实现 |
| 源码实现 | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | 服务端 `ExecTool`、超时/让出/会话管理已实现 |
| SA 注册 | `services/sa_profile/186.json` | SA 186（进程 aimgr，库 libclimgr.z.so） |
| 进程配置 | `cli_tool_framework/etc/profile/aimgr.cfg` | ondemand、uid=aimgr、caps=KILL、SELinux 域 aimgr |

### 初始范围

**可能包含：**
- `execTool` System API 的参数契约（toolName/subcommand/args/challenge/options）
- 前台/让出(yieldMs)/后台/超时 四种会话语义
- 权限校验（系统应用 + `ohos.permission.EXEC_CLI_TOOL`）、HAP 沙箱校验
- 错误码 35700000–35700008、35700020

**明确不包含：**
- `execCmd`（shell 命令执行）、`SubscribeSession`/`SendMessage`/`RegisterFunction` 等同模块其他接口
- 工具注册/数据管理（`CliToolDataManager`/`RegisterFunction`）的内部实现
- 跨设备会话同步

### 初始假设

| 假设 | 类型 | 验证方式 | 状态 |
|------|------|----------|------|
| 调用方须为系统应用 HAP | 技术/安全 | 源码 `ValidateExecToolPermissions` | 已验证 |
| 沙箱由系统按调用方 Token 生成 | 技术 | 源码 `ToolUtil::GenerateSandboxConfig` | 已验证 |
| 会话为运行态内存对象，不持久化 | 兼容性 | 源码 `SessionRecord` 生命周期 | 已验证 |
| 并发上限由系统配置决定 | 技术 | 源码 `CcmUtil::GetCliConcurrencyLimit` | 已验证 |

### 初始分级判断

| 判断项 | 结果 | 依据 |
|--------|------|------|
| 复杂度 | 标准 | 单 System API，行为分支有限但涉及沙箱/IPC/会话生命周期 |
| 涉及仓数量 | 1 | `foundation/ability/ability_runtime` |
| 是否涉及 Public/System API | 是 | 新增 System API `execTool` |
| 是否涉及安全/性能关键路径 | 是 | 权限校验、沙箱、子进程、超时 |
| 是否跨 SIG | 否 | 全部在 Sig-Ability 内 |

### 进入澄清条件

- [x] 原始问题和期望结果已记录
- [ ] 需求来源和责任人已明确（待确认）
- [x] 初始范围和不包含项已记录
- [x] 关键假设和待澄清问题已列出
- [x] 复杂度有判断或明确为待定

---

## 二、澄清记录

> 本需求无原始对话记录，澄清项依据实现反推。标记「已澄清（实现反推）」的项需 Owner 复核确认。

### 待澄清问题

| 编号 | 问题 | 为什么需要澄清 | 状态 |
|------|------|----------------|------|
| Q-1 | `execTool` 是否仅限系统应用？ | 决定 API 开放范围与权限模型 | 已澄清（实现反推：仅系统应用） |
| Q-2 | 是否允许非 HAP 调用？ | 决定沙箱生成可行性 | 已澄清（实现反推：须 HAP） |
| Q-3 | 让出（yieldMs）与后台（background）是否互斥？ | 决定会话语义模型 | 已澄清（实现反推：yieldMs 仅前台生效） |
| Q-4 | 超时上限是否固定 1800 秒？ | 决定边界约束 | 已澄清（实现反推：MAX_TIMEOUT=1800） |
| Q-5 | 目标发行版本？ | 基线必需 | 待确认 |

### 讨论记录

| 日期 | 参与人 | 讨论主题 | 结论 | 后续动作 |
|------|--------|----------|------|----------|
| 2026-08-05 | AI 反推 | 会话语义 | 前台/让出/后台/超时四态 | Owner 复核 |

### 功能范围确认

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 核心功能包含哪些？ | execTool 执行已注册 CLI 工具；前台/让出/后台/超时会话；错误码 | 待确认 | 已确认（反推）/待确认 |
| 明确不包含哪些？ | execCmd、订阅、注册函数、跨设备同步 | 待确认 | 已确认（反推）/待确认 |
| 是否有分期策略？ | 否 | 待确认 | 已确认（反推） |

### 方案探索

| 编号 | 方案概述 | 优势 | 风险/代价 | 选择结论 |
|------|----------|------|-----------|----------|
| A-1 | SA（186/aimgr）统一承载：NAPI→IPC→服务端做权限/沙箱/子进程/会话管理 | 集中权限审计、统一沙箱、可并发控制、失败可上报 | 跨进程 IPC 开销、SA 须按需拉起 | 推荐 |
| A-2 | 应用进程内直接 fork+exec 工具 | 无 IPC 开销、延迟低 | 绕过权限审计、沙箱不可控、无并发上限、难审计 | 放弃 |

**取舍理由：** CLI 工具执行涉及权限/沙箱/超时/审计，集中到 SA 可统一安全边界与并发控制，IPC 开销可接受；A-2 安全风险不可接受。

### 上下文与知识源检索日志

| 编号 | 来源 | 查询/读取内容 | 关键发现 | 可信度 | 用于 | 命中/原因 |
|------|------|---------------|----------|--------|------|-----------|
| K-1 | 源码 `js_cli_manager.cpp` | `OnExecTool` 参数解析与回调派发 | 必填：toolName/subcommand/args/challenge，options 可选；argc<4 同步抛错 | 高 | 范围/API | 命中 |
| K-2 | 源码 `cli_tool_manager_service.cpp` | `ExecTool`/`ValidateExecToolPermissions`/`ValidateAndPrepareTool`/`SetupAndStartSession`/超时与让出处理 | 权限链、会话生命周期、超时=1800、yieldMs 仅前台、后台立即返回 | 高 | 设计/测试 | 命中 |
| K-3 | 源码 `tool_util.cpp` | `ValidateProperties`/`ValidateExecOptionsProperties`/`ValidateInputSchemaProperties` | subcommand 与 inputSchema 校验规则、help 特殊键 | 高 | 设计/测试 | 命中 |
| K-4 | `cli_error_code.h` | 错误码枚举 | 35700000–35700020 区间 | 高 | API | 命中 |
| K-5 | `ICliToolManager.idl` | IPC 接口签名 | `ExecTool(param, eventId, scheduler)` oneway 回复经 Scheduler | 高 | 设计 | 命中 |
| K-6 | `services/sa_profile/186.json`、`aimgr.cfg` | SA 注册与进程配置 | SA 186、进程 aimgr、ondemand、caps=KILL | 高 | 设计/构建 | 命中 |
| K-7 | `AGENTS.md` | 目标仓 Agent 指南 | 服务层与 SDK 层解耦、PermissionVerification、hisysevent 硬约束 | 高 | 设计约束 | 命中 |

**上下文结论：**
- 高可信结论：execTool 行为链路、参数契约、错误码、SA/进程模型均已由源码固化，可直接进入基线与设计。
- 待确认结论：目标发行版本、需求来源与责任人、并发上限具体数值（由产品配置）。
- 未使用来源及原因：未使用多仓知识库（本需求单仓内闭环）。

### 子系统影响

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 涉及哪些子系统？ | ability（ability_runtime） | 待确认 | 已确认（反推） |
| 是否需要新增子系统或部件？ | 否 | 待确认 | 已确认（反推） |

### API 变更评估

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 是否需要新增/修改 Public API？ | 否 | 待确认 | 已确认（反推） |
| 是否需要新增 System API？ | 是，1 个（`execTool`） | 待确认 | 已确认（反推） |
| 是否会废弃已有 API？ | 否 | 待确认 | 已确认（反推） |
| 是否需要新增权限声明？ | 是，`ohos.permission.EXEC_CLI_TOOL` | 待确认 | 已确认（反推） |

### 兼容性与非功能需求

| 类别 | 核心问题 | 结论 | 确认人 | 状态 |
|------|----------|------|--------|------|
| 兼容性 | 向前/向后兼容要求？破坏性变更？ | 新增 API，无破坏性；会话内存态无持久化迁移 | 待确认 | 已确认（反推） |
| 性能 | 响应时间/内存/并发要求？ | 并发受系统上限约束；无公开 P 指标 | 待确认 | 已确认（反推） |
| 安全 | 权限/隐私/加密/审计要求？ | 系统应用+权限+HAP 沙箱；失败经 HiSysEvent | 待确认 | 已确认（反推） |
| 可靠性 | 崩溃率/容错/恢复要求？ | SA 不可用回 35700000；单次调用不应致 SA 崩溃 | 待确认 | 已确认（反推） |

### 依赖与风险

| 依赖项 | 类型 | 说明 | 状态 |
|--------|------|------|------|
| `safwk`/`samgr` | 编译/运行 | SA 186 按需注册与拉起 | 已确认 |
| `access_token` | 运行 | 系统应用判定与权限校验 | 已确认 |
| `bundle_framework` | 运行 | HAP 判定与 bundleName 获取 | 已确认 |
| `kv_store` | 运行 | 工具/函数元数据存储（不影响 execTool 行为契约） | 已确认 |

| 风险 | 类型 | 影响 | 缓解措施 | 状态 |
|------|------|------|----------|------|
| 需求来源/Owner 未确认 | 进度 | 基线无法冻结 | 需求方/SIG 确认 | 待确认 |
| 目标版本未确认 | 进度 | 影响 @since 与发布 | proposal.target_release 确认 | 待确认 |
| 并发上限随产品配置 | 技术 | 行为语义不变但数值差异 | spec 标注「由系统配置决定」 | 已确认 |

### AC 完整性

- [x] 每个用户故事有验收标准
- [x] AC 全部使用 WHEN/THEN 格式
- [x] 覆盖正常流程、异常流程、边界条件
- [x] AC 可测试、可度量

### 澄清结论

- [x] 功能范围已完全明确
- [x] 子系统影响已识别
- [x] API 变更已评估
- [x] 兼容性和非功能需求已确认
- [x] 依赖和风险已识别且有缓解方案
- [x] AC 完整可测试
- [x] 标准及以上复杂度已完成方案探索（A-1/A-2 + 取舍理由）

**结论:** 条件通过（实现反推完整，待 Owner/需求方确认后转「通过」）

---

## 三、需求基线

> 澄清完成后固化。manifest.md 是事实源，此处为审批结论。

### 基线信息

| 字段 | 内容 |
|------|-----|
| 基线版本 | v1.0 |
| 基线日期 | 2026-08-05 |
| Owner | 待确认 |
| 确认人 | 需求方/模块Owner/SIG代表（待确认） |
| 复杂度 | 标准 |
| Profile | none |
| 目标发行版本 | OpenHarmony-6.0-Release（待确认） |
| 版本状态 | proposed |

### 问题陈述

系统应用缺少受控、可审计、受沙箱约束的 CLI 工具执行入口。本需求新增 `execTool` System API，由 SA 186（aimgr）统一承载权限校验、沙箱子进程创建、会话生命周期（前台/让出/后台/超时）与结果回传，并在失败路径经 HiSysEvent 上报。

### 目标和成功指标

| 目标 | 成功指标 | 验证方式 |
|------|----------|----------|
| 受控执行已注册 CLI 工具 | 合法调用返回 completed 会话与退出码 | 集成测试 |
| 权限/沙箱边界生效 | 非系统应用/缺权限/非 HAP 均被拒绝并返回正确错误码 | 单测 |
| 会话语义正确 | 前台/让出/后台/超时各态 reply 时机与 status 正确 | 集成测试 |
| 失败可归因 | 失败路径经 HiSysEvent 上报 bundleName/toolName/failureReason | hisysevent 校验 |

### 用户故事与 AC

| Story ID | 用户故事 | 优先级 |
|----------|----------|--------|
| US-1 | 作为系统应用开发者，我想要通过 execTool 按名称执行已注册 CLI 工具并获取会话信息，以便在系统应用内集成受沙箱约束的命令行工具能力 | P1 |

> AC 共 15 条（AC-1.1–AC-1.15），详见 `spec.md`。此处不重复摘录，避免与 spec 不一致。

### 范围边界

**包含：** `execTool` System API 行为契约、参数校验、会话生命周期（前台/让出/后台/超时）、错误码、权限/沙箱前置校验。
**不包含：** `execCmd`、`SubscribeSession`/`SendMessage`、`RegisterFunction`、工具注册数据管理、跨设备同步。

### 影响范围

| 子系统 | 仓库 | 模块/路径 | 当前职责 | 影响类型 | Owner |
|--------|------|-----------|----------|----------|-------|
| ability | foundation/ability/ability_runtime | cli_tool_framework/frameworks/js/napi/cli_tool_manager | NAPI 绑定 | 新增 API（既有实现固化契约） | 待确认 |
| ability | foundation/ability/ability_runtime | cli_tool_framework/services/climgr | 服务端实现 | 新增（既有实现固化契约） | 待确认 |
| ability | foundation/ability/ability_runtime | cli_tool_framework/interfaces/cli_tool | IPC 接口与数据结构 | 新增（既有实现固化契约） | 待确认 |
| ability | foundation/ability/ability_runtime | services/sa_profile/186.json | SA 注册 | 不变（已注册） | — |

### API 变更项清单

| API 名称 | 变更类型 | 开放范围 | 概要说明 |
|----------|----------|----------|----------|
| `cliTool.execTool` | 新增 | System | 按名称执行已注册 CLI 工具，返回 Promise<CliSessionInfo> |

### 不涉及项确认

| 维度 | 涉及？ | 依据 | 若涉及，进入哪个下游文档 |
|------|--------|------|--------------------------|
| 性能 | 否 | 无公开 P 指标；并发上限由系统配置，非本 API 行为契约 | — |
| 安全与权限 | 是 | 系统应用+EXEC_CLI_TOOL 权限、HAP 沙箱、失败上报 | design.md / spec.md |
| 兼容性 | 否 | 新增 API，无破坏性；会话内存态无迁移 | spec.md |
| API/SDK | 是 | 新增 System API | design.md / spec.md |
| IPC/跨进程 | 是 | NAPI→IPC→SA 186 | design.md |
| 构建与部件 | 否 | 复用既有 component/parts，无新增部件 | — |
| 国际化/无障碍 | 否 | 无界面 System API | — |
| 数据迁移 | 否 | 无持久化 | — |

### 变更控制

| 变更类型 | 触发条件 | 处理规则 |
|----------|----------|----------|
| 范围新增 | 新增用户故事或仓/模块 | 重新评估复杂度和设计影响 |
| AC 变更 | 修改可观察行为或错误码 | 重新审批基线和 Spec |
| API 变更 | 新增/修改 Public/System API | 触发设计审批 |
| 非功能指标变更 | 性能/安全/兼容性阈值变化 | 重新确认测试计划 |
| 目标版本变更 | 交付版本调整 | 更新 proposal.target_release |

### 进入设计/Spec 条件

- [x] 所有 P0/P1 用户故事有 AC
- [x] 每条 AC 可测试、可度量
- [x] 范围内/外已确认
- [ ] `proposal.target_release` 已确认或明确 TBD（待确认）
- [x] `manifest.profile` 已确认或明确 none
- [x] 涉及仓、模块、SIG 已识别
- [x] 不涉及项已标记 N/A
- [x] 变更控制规则已确认
- [x] 标准及以上复杂度的澄清问题已逐项关闭，且讨论记录包含需求方/Owner/SIG 明确确认（实现反推，待 Owner 复核）
- [x] 上下文与知识源检索日志已填写；未查询关键来源的原因已记录
- [x] 目标仓 Agent 指南已检查并记录关键约束（AGENTS.md 服务/SDK 解耦、PermissionVerification、hisysevent 硬约束）

**基线结论:** 条件通过
