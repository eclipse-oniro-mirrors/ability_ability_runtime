---
target_release: "7.0"
change_type: new-feature
issue: "https://gitcode.com/openharmony/ability_ability_runtime/issues/15382"
author: "@SKY2001"
date: "2026-08-03"
status: Approved
---

# Proposal

## 背景与问题

OpenHarmony 应用存在"在同一个沙箱环境内顺序执行一组 shell 命令"的诉求，典型如 `cd workpath; git log; git commit`——多条命令共享同一工作目录、同一环境与同一进程上下文。现有执行通道要么是一次性短命令（无会话），要么是跨进程隔离（无共享上下文），无法满足"同沙箱、可交互、可长驻"三者兼具的需求。需要在 cli SA 侧提供"长时沙箱 + 会话管理"的 shell 执行能力，并打通 fork 后 ATM 包装调用方 tokenId → 内核新接口设置 parentHapTokenId 的安全链路。

## 用户场景与业务触发

### US-1: 应用在同沙箱顺序执行一组命令
- **角色**：应用开发者 / 上层业务调用方
- **业务触发**：应用调用 cli SA shell 接口，发起一组需共享上下文的命令
- **业务价值**：多条命令共享同一沙箱/工作目录/进程上下文，避免每条命令各自起进程导致状态丢失

### US-2: 应用与长驻 shell 交互
- **角色**：应用开发者
- **业务触发**：应用以后台模式发起命令，获得 sessionId 后注入后续交互输入
- **业务价值**：支持交互式长会话（如问答 y/n、发送后续命令），用完主动清理

## 初始分级判断

| 判断项 | 结果 | 依据 |
|--------|------|------|
| 变更类型 | new-feature | 新增 cli SA 沙箱 shell 执行 + 会话管理能力 |
| 复杂度 | 标准 | 跨三仓（ability_ability_runtime / security_access_token / kernel），含状态机与并发清理 |
| 涉及仓数量 | 3 | ability_ability_runtime（主）、security_access_token、kernel |
| 是否涉及 Public/System API | 是 | 新增 cli SA 对外执行接口（动态接口） |
| 是否涉及安全/性能关键路径 | 是 | fork 后 ATM tokenId + 内核 parentHapTokenId 安全链路；沙箱内执行 |
| 是否跨 SIG | 是 | 跨 ability / security / kernel 三 SIG |

## 目标

1. cli SA 提供执行 shell 命令的接口，前台/后台统一入口、选项区分；后台返回 sessionId。
2. 支持应用获取命令执行结果、通过 sessionId 发送交互指令、清理 session。
3. 支持 policy 参数透传给 sandbox，接口层不解释语义。
4. 打通 fork 后 ATM 包装调用方 tokenId → 内核新接口设置 parentHapTokenId 的安全链路。
5. 仅提供动态接口。

## 非目标

- 静态接口（7.1 版本提供）。
- security_access_token / kernel 内部接口的具体实现（跨仓依赖，由对应仓跟踪）。
- JS/NAPI 绑定层（CliToolMGRClient 为 C++ 客户端，JS 绑定另立跟踪）。

## Agent Scope Guard

| 维度 | 范围/限制 | 需人工确认的触发条件 |
|------|-----------|----------------------|
| 允许仓库 | ability_ability_runtime | 需修改 security_access_token / kernel 时先确认 |
| 允许模块/目录 | `cli_tool_framework/`（interfaces/cli_tool/include、services/climgr、services/common、test） | 超出 cli_tool_framework 时先确认 |
| 禁止修改项 | security_access_token / kernel 仓内部实现；既有公共 IPC 序列化格式 | 需变更序列化格式时先评估兼容性 |
| 外部依赖/网络访问 | ATM 接口、内核新接口（本地系统调用） | 无网络访问 |

## 成功标准

| 标准 | 可观察指标 | 验证方式 |
|------|-----------|---------|
| 应用可执行一组 shell 命令并共享沙箱上下文 | 前台命令回调返回正确 exitCode/outputText | demo 调用 ExecCmd(background=false) |
| 后台命令可长驻交互 | 立即返回 sessionId；SendMessage 注入后输出响应；ClearSession 后进程消失 | demo 调用 ExecCmd(background=true) 全链路 |
| 沙箱内进程以正确父身份运行 | fork 后 parentHapTokenId 被设置 | 内核侧/ATM 侧验证 tokenId 链路 |

## 影响范围

| 子系统 | 仓库 | 模块/路径 | 影响类型 |
|--------|------|-----------|---------|
| AppFwk | ability_ability_runtime | `cli_tool_framework/interfaces/cli_tool/include` | 新增接口与数据结构 |
| AppFwk | ability_ability_runtime | `cli_tool_framework/services/climgr` | 新增 session 状态机、进程管理、工具 |
| AppFwk | ability_ability_runtime | `cli_tool_framework/services/common` | 新增 ATM 权限校验 |
| Security | security_access_token | ATM 接口 | 依赖（生成包装调用方 tokenId） |
| Kernel | kernel | 内核新接口 | 依赖（生成 tokenId 并设置 parentHapTokenId） |

## 假设与开放问题

### 假设
- security_access_token 的 ATM 包装 tokenId 接口已就绪或同步对齐。
- kernel 设置 parentHapTokenId 的新接口已就绪或同步对齐。
- 应用调用方具备调用 cli SA shell 接口的合法身份。

### 开放问题
- policy 的具体沙箱策略语义由 sandbox 侧定义，接口层不解释，需 sandbox 侧确认消费契约。
- JS/NAPI 绑定层何时补齐（动态接口先行，绑定另立）。

## 不涉及项确认

| 维度 | 是否涉及 | 依据 |
|------|---------|------|
| 性能 | 不涉及 | 无明确时延/吞吐 SLA 指标；仅设 64KB 输出缓冲防 OOM |
| 安全/权限 | 是 | 跨进程、沙箱内外、ATM tokenId + 内核 parentHapTokenId、权限校验 |
| 兼容性 | 不涉及 | 全新增能力，无既有 API 行为变更 |
| API/SDK | 是 | 新增动态接口（Public/System 级别待定） |
| IPC/跨进程 | 是 | CliToolMGRClient ↔ CliToolManagerService 经 ICliToolManager IPC |
| 构建/组件 | 是 | 新增 cli_tool_framework 模块，按 ability_runtime 既有 gn 构建 |
| 国际化/无障碍 | 不涉及 | 命令执行无 UI 文案 |
| 数据迁移 | 不涉及 | 全新能力，无既有数据格式 |
