# 执行计划

> 将 Approved Spec 拆成可独立执行、可验证、可审查的 Task。本计划针对 `execTool` 既有实现进行契约固化与测试覆盖验证：实现已存在，Task 聚焦「验证既有链路符合 spec + 补齐测试缺口 + 跨仓协调」。

## Plan 元数据

| 字段 | 内容 |
|------|-----|
| Plan ID | PLAN-CLI-001 |
| 关联 Feature/Bug | FEAT-CLI-001（REQ-010） |
| 关联文档 | proposal.md / design.md / spec.md |
| 复杂度 | 标准 |
| 状态 | Draft |
| Owner | 待确认 |

## 输入状态

| 输入 | 路径 | 要求状态 |
|------|------|----------|
| Requirement | `proposal.md` | Approved（条件通过，待 Owner 转正） |
| Design | `design.md` | Approved（条件通过，待 Owner 转正） |
| Spec | `spec.md` | Approved（Draft→待转 Approved） |

## 受影响文件全量清单

| 仓 | 层（来自 design.md） | 文件路径 | 修改类型 | 说明 |
|----|---------------------|----------|----------|------|
| ability_runtime | NAPI | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp` | 验证/补测试 | `execTool`/`OnExecTool` 参数解析与回调 |
| ability_runtime | NAPI | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/include/js_cli_manager.h` | 只读参考 | 声明 |
| ability_runtime | InnerAPI/IPC | `cli_tool_framework/interfaces/cli_tool/ICliToolManager.idl` | 只读参考 | `ExecTool` oneway 声明 |
| ability_runtime | InnerAPI/IPC | `cli_tool_framework/interfaces/cli_tool/ICliToolManagerScheduler.idl` | 只读参考 | `SchedulerExecToolReplyEvent` 回复 |
| ability_runtime | InnerAPI/IPC | `cli_tool_framework/interfaces/cli_tool/include/exec_tool_param.h` | 验证 | ExecToolParam Parcelable |
| ability_runtime | InnerAPI/IPC | `cli_tool_framework/interfaces/cli_tool/include/exec_options.h` | 验证 | ExecOptions |
| ability_runtime | InnerAPI/IPC | `cli_tool_framework/interfaces/cli_tool/include/cli_session_info.h` | 验证 | CliSessionInfo |
| ability_runtime | InnerAPI/IPC | `cli_tool_framework/interfaces/cli_tool/include/exec_result.h` | 验证 | ExecResult |
| ability_runtime | InnerAPI/IPC | `cli_tool_framework/interfaces/cli_tool/include/cli_error_code.h` | 验证 | 错误码 357000xx |
| ability_runtime | InnerAPI/IPC | `cli_tool_framework/interfaces/cli_tool/src/cli_tool_mgr_client.cpp` | 验证 | 客户端 ExecTool + SA 拉起 |
| ability_runtime | 服务 | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | 验证 | 服务端 ExecTool 全链路 |
| ability_runtime | 服务 | `cli_tool_framework/services/climgr/src/tool_util.cpp` | 验证 | 参数/schema/沙箱校验 |
| ability_runtime | 服务 | `cli_tool_framework/services/climgr/src/process_manager.cpp` | 验证 | 子进程创建 |
| ability_runtime | 服务 | `cli_tool_framework/services/climgr/src/session_record.cpp` | 验证 | 会话状态机 |
| ability_runtime | 服务 | `cli_tool_framework/services/climgr/src/event_dispatcher.cpp` | 验证 | 事件/回复派发 |
| ability_runtime | DFX | `cli_tool_framework/services/common/src/cli_event_report*.cpp` | 验证 | HiSysEvent 上报 |
| ability_runtime | SA | `services/sa_profile/186.json` | 只读参考 | SA 186 注册 |
| ability_runtime | 配置 | `cli_tool_framework/etc/profile/aimgr.cfg` | 只读参考 | 进程/权限/caps |
| 跨仓（权限定义仓） | 权限 | `ohos.permission.EXEC_CLI_TOOL` 定义 | 跨仓协调 | 权限登记（不在本仓） |

**检查项：**
- [x] design.md 调用链每一层都有对应文件列出
- [x] 每个文件修改类型和职责说明明确
- [x] 无映射行的层缺失

## AC 到 Task 追溯

| AC | 来源 | Task | 验证方式 | 覆盖？ |
|----|------|------|----------|--------|
| AC-1.1 | spec.md | TASK-CLI-001 | 集成 | 是 |
| AC-1.2 | spec.md | TASK-CLI-002 | 集成 | 是 |
| AC-1.3 | spec.md | TASK-CLI-002 | 集成 | 是 |
| AC-1.4 | spec.md | TASK-CLI-003 | 集成 | 是 |
| AC-1.5 | spec.md | TASK-CLI-004 | 单测 | 是 |
| AC-1.6 | spec.md | TASK-CLI-004 | 单测 | 是 |
| AC-1.7 | spec.md | TASK-CLI-005 | 单测 | 是 |
| AC-1.8 | spec.md | TASK-CLI-005 | 单测 | 是 |
| AC-1.9 | spec.md | TASK-CLI-005 | 单测 | 是 |
| AC-1.10 | spec.md | TASK-CLI-005 | 单测 | 是 |
| AC-1.11 | spec.md | TASK-CLI-005 | 单测 | 是 |
| AC-1.12 | spec.md | TASK-CLI-006 | 单测 | 是 |
| AC-1.13 | spec.md | TASK-CLI-007 | 集成 | 是 |
| AC-1.14 | spec.md | TASK-CLI-004 | 集成 | 是 |
| AC-1.15 | spec.md | TASK-CLI-008 | 单测 | 是 |

## 首批实现边界

**首批必须实现：** TASK-CLI-001（正常链路）+ TASK-CLI-005（参数/边界校验）+ TASK-CLI-004（权限/非HAP），构成主链与安全边界。
**可后置：** TASK-CLI-008（DFX 跨仓协调）。
**不建议延后：** TASK-CLI-002/003（会话语义/超时）延后会导致主链行为不闭合。

## 阶段计划（如适用）

| 阶段 | 目标 | 关键 Task | 结束门槛 | 最小验证 |
|------|------|-----------|----------|----------|
| Phase-1 | 主链+安全+参数 | TASK-CLI-001,004,005,006 | 正常执行与异常路径单测通过 | `run -t UT -ts cli_tool_mgr_client_test` |
| Phase-2 | 会话语义+超时+并发 | TASK-CLI-002,003,007 | yield/background/timeout/上限验证通过 | `run -t UT -ts process_manager_test` |
| Phase-3 | DFX+SA 不可用 | TASK-CLI-008 | 失败上报与 SA 不可用覆盖 | `run -t UT -ts cli_event_report_test` |

## Task 粒度原则

- 每个 Task 对应一个可独立验收的最小能力闭环
- 文件范围、验证闭环和风险边界足够分离 → 拆分
- 简单变更：1-2 张 Task Card；本计划按主链/会话/超时/权限/参数/NAPI/并发/DFX 拆为 8 张
- 每个 Task 自包含，不依赖外部文件路径引用

## 禁止项

- [x] 没有 TBD / TODO / 占位符（「待确认」为需 Owner 决议项，非规格占位）
- [x] 没有"根据需要实现""酌情处理"等模糊指令
- [x] 没有跨 Task 隐式依赖（依赖显式声明在前置依赖列）
- [x] 没有要求 Agent 自行寻找未列出的上下文文件
- [x] 没有无验证方式的 AC
- [x] 没有"与 Task-N 类似""参考 Task-N 实现"等引用

## Task 列表

| Task ID | 目标 | 文件范围 | AC 映射 | 前置依赖 | 完成判据 | 验证命令 |
|---------|------|----------|---------|----------|----------|----------|
| TASK-CLI-001 | 正常执行链路契约验证 | climgr/cli_tool_mgr_client | AC-1.1 | 无 | 正常调用返回 completed 会话与退出码 | `run -t UT -ts cli_tool_mgr_service_test` |
| TASK-CLI-002 | 前台让出/后台会话语义验证 | cli_tool_manager_service/session_record | AC-1.2,AC-1.3 | TASK-CLI-001 | yieldMs 与 background reply 时机/状态正确 | `run -t UT -ts cli_tool_mgr_service_test` |
| TASK-CLI-003 | 超时终止与终态派发验证 | cli_tool_manager_service/process_manager | AC-1.4 | TASK-CLI-002 | timeout 触发 kill+timeout=true+错误事件 | `run -t UT -ts process_manager_test` |
| TASK-CLI-004 | 权限/非HAP异常路径验证 | cli_tool_manager_service/cli_tool_mgr_client | AC-1.5,AC-1.6,AC-1.14 | TASK-CLI-001 | 非系统/缺权限/非HAP 返回正确码 | `run -t UT -ts cli_tool_mgr_client_test` |
| TASK-CLI-005 | 工具/子命令/schema/超时边界校验验证 | tool_util/exec_options/exec_tool_param | AC-1.7–1.11 | TASK-CLI-001 | 各非法输入返回 35700005/35700002 | `run -t UT -ts tool_util_test` |
| TASK-CLI-006 | NAPI 参数同步校验验证 | js_cli_manager | AC-1.12 | TASK-CLI-001 | 必填缺失/类型错/argc<4 同步抛错 | `run -t UT -ts cli_tool_mgr_client_test` |
| TASK-CLI-007 | 会话并发上限验证 | cli_tool_manager_service | AC-1.13 | TASK-CLI-001 | 超上限返回 35700001 | `run -t UT -ts cli_tool_mgr_service_test` |
| TASK-CLI-008 | DFX 失败上报 + SA 不可用验证 | cli_event_report/cli_tool_mgr_client | AC-1.15 | TASK-CLI-004 | SA 不可用返回 35700000；失败经 HiSysEvent | `run -t UT -ts cli_event_report_test` |

## Task 详情

### TASK-CLI-001: 正常执行链路契约验证

| 字段 | 内容 |
|------|-----|
| 任务目标 | 验证既有 execTool 正常链路符合 spec AC-1.1：合法调用返回 completed 会话与退出码 |
| AC 映射 | AC-1.1 |
| 前置依赖 | 无 |
| 非目标 | 不验证 yield/background/timeout（属 TASK-CLI-002/003） |
| 完成判据 | 集成测试覆盖：合法 toolName+subcommand+args+challenge→resolve status="completed"，result.exitCode=进程退出码 |
| 停止条件 | 发现既有实现与 spec 行为不一致且无法在 Task 范围内修复 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/cli_tool_mgr_service_test/` | 正常链路用例 |
| Test | `cli_tool_framework/test/unittest/process_manager_test/` | 子进程创建用例 |

**Spec Context**

AC-1.1：WHEN 系统应用持 EXEC_CLI_TOOL 调用 execTool，传入已注册 toolName、工具支持 subcommand、符合 inputSchema 的 args、非空 challenge THEN 接口返回 Promise，工具执行结束后 resolve 为 CliSessionInfo(status="completed"，result.exitCode 等于子进程退出码)。

R-1：合法输入→创建会话与受沙箱子进程执行工具，结束后 resolve CliSessionInfo{status="completed", result.exitCode=进程退出码, outputText, errorText, executionTime}。

**Design Context**

调用链：NAPI→cli_tool_client Proxy→SA 186 ExecTool(oneway)→权限/查找/校验/沙箱→ProcessManager.CreateChildProcess→IOMonitor.RegisterSession→EventDispatcher.DispatchExecToolReplyEvent→client Scheduler Stub→NapiAsyncTask.Resolve。设计 ADR-1（SA 集中承载）、ADR-2（oneway+回调）。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | Must：NAPI 不直接调 services 内部；经 InnerAPI/IPC |
| OH-ARCH-IPC-SAF | Must：ExecTool 为 oneway；回复经 Scheduler |

**Steps**

- [ ] 写失败测试：合法调用断言 resolve completed + exitCode
- [ ] 运行测试，确认当前结果
- [ ] 若不一致，做最小实现修正
- [ ] 运行测试，确认通过
- [ ] 填写完成证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts cli_tool_mgr_service_test` | PASS |
| 测试 | `run -t UT -ts process_manager_test` | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 验证 execTool 正常链路返回 completed 会话与退出码（AC-1.1） |
| 允许修改 | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp`、`cli_tool_framework/test/unittest/cli_tool_mgr_service_test/` |
| 允许新建 | 测试用例文件 |
| 只读参考 | `spec.md` AC-1.1、`design.md` ADR-1/ADR-2、`ICliToolManager.idl` |
| Spec 摘要 | AC-1.1 + R-1：合法输入→completed 会话+exitCode |
| Design 摘要 | NAPI→IPC→SA oneway+Scheduler 回复；ProcessManager.CreateChildProcess |
| 执行步骤 | 写失败测试→运行→最小修正→运行通过→记录证据 |
| 验证命令 | `run -t UT -ts cli_tool_mgr_service_test` / 期望: PASS |
| 完成规则 | 不得修改允许范围外文件；如需扩大范围停止并修订 Plan；无 fresh evidence 不得声明完成 |

### TASK-CLI-002: 前台让出/后台会话语义验证

| 字段 | 内容 |
|------|-----|
| 任务目标 | 验证 yieldMs 让出与 background 立即返回的 reply 时机与 status 符合 AC-1.2/AC-1.3 |
| AC 映射 | AC-1.2, AC-1.3 |
| 前置依赖 | TASK-CLI-001 |
| 非目标 | 不验证超时终止（TASK-CLI-003） |
| 完成判据 | background=true 立即 resolve running；foreground+yieldMs>0 在 yieldMs 处 resolve running 且转后台 |
| 停止条件 | 定时器或状态机行为与 spec 不一致且超范围 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/cli_tool_mgr_service_test/` | 会话语义用例 |
| Test | `cli_tool_framework/test/unittest/session_record_test/` | 状态机用例 |

**Spec Context**

AC-1.2：WHEN background=false 且 yieldMs>0 THEN 在 yieldMs 毫秒后 resolve 为 status="running"，会话转后台继续运行。
AC-1.3：WHEN background=true THEN 立即 resolve 为 status="running"。
R-2/R-3：yield 让出、background 立即返回。

**Design Context**

RegisterSessionWithMonitors：非 background 且 yieldMs!=0 → PostExecToolTask(yieldMs, isTimeout=false)；background → HandleBackgroundSessionReply 立即派发。HandleProcessYieldTimeout 切后台并派发 running。状态机 SPAWNING→RUNNING→CANCELLING/COMPLETED/FAILED。设计 ADR-3。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-IPC-SAF | Must：回复经 SchedulerExecToolReplyEvent |

**Steps**

- [ ] 写失败测试：background 立即返回 + yieldMs 让出时机
- [ ] 运行测试
- [ ] 最小修正
- [ ] 运行通过
- [ ] 记录证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts cli_tool_mgr_service_test` | PASS |
| 测试 | `run -t UT -ts session_record_test` | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 验证 yield 让出与 background 立即返回语义（AC-1.2/1.3） |
| 允许修改 | `cli_tool_manager_service.cpp`、`session_record.cpp`、对应测试 |
| 允许新建 | 测试用例 |
| 只读参考 | spec AC-1.2/1.3、design ADR-3、`session_record.h` 状态枚举 |
| Spec 摘要 | AC-1.2/1.3 + R-2/R-3 |
| Design 摘要 | RegisterSessionWithMonitors 调度 yield；HandleBackgroundSessionReply/HandleProcessYieldTimeout |
| 执行步骤 | 写失败测试→运行→修正→通过→证据 |
| 验证命令 | `run -t UT -ts cli_tool_mgr_service_test` / 期望: PASS |
| 完成规则 | 不改范围外文件；无 evidence 不声明完成 |

### TASK-CLI-003: 超时终止与终态派发验证

| 字段 | 内容 |
|------|-----|
| 任务目标 | 验证 timeout 秒后进程被终止、result.timeout=true、派发超时错误事件（AC-1.4） |
| AC 映射 | AC-1.4 |
| 前置依赖 | TASK-CLI-002 |
| 非目标 | 不验证正常完成路径 |
| 完成判据 | 进程达 timeout 被 kill，终态 result.timeout=true，订阅者收到 "session timed out" 错误事件 |
| 停止条件 | kill/定时器逻辑与 spec 不一致且超范围 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/process_manager_test/` | 超时/kill 用例 |

**Spec Context**

AC-1.4：WHEN 进程运行达 options.timeout 秒 THEN 进程被终止、result.timeout=true，并向订阅者派发超时错误事件。
R-4：timeout>0 达阈值→终止、timeout=true、错误事件；未后台则派发终态 reply。

**Design Context**

PostExecToolTask(timeoutMs, isTimeout=true) → HandleProcessTimeout：SetTimeout(true)、SetState(CANCELLING)、DispatchExecToolReplyEvent（未后台）、DispatchErrorEvent("session timed out")、ProcessManager.Killpg。设计 ADR-4（ffrt 延时）。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-ERROR-LOG | Must：超时经 HiSysEvent ReportCliTimeout 上报 |

**Steps**

- [ ] 写失败测试：timeout 触发 kill + timeout=true + 错误事件
- [ ] 运行测试
- [ ] 最小修正
- [ ] 运行通过
- [ ] 记录证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts process_manager_test` | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 验证超时终止与终态派发（AC-1.4） |
| 允许修改 | `cli_tool_manager_service.cpp`、`process_manager.cpp`、对应测试 |
| 允许新建 | 测试用例 |
| 只读参考 | spec AC-1.4 + R-4、design ADR-4 |
| Spec 摘要 | AC-1.4：timeout→kill+timeout=true+错误事件 |
| Design 摘要 | HandleProcessTimeout + Killpg + ffrt 延时任务 |
| 执行步骤 | 写失败测试→运行→修正→通过→证据 |
| 验证命令 | `run -t UT -ts process_manager_test` / 期望: PASS |
| 完成规则 | 不改范围外文件；无 evidence 不声明完成 |

### TASK-CLI-004: 权限/非HAP异常路径验证

| 字段 | 内容 |
|------|-----|
| 任务目标 | 验证非系统应用、缺权限、非 HAP 调用分别返回 35700008/35700007/35700003（AC-1.5/1.6/1.14） |
| AC 映射 | AC-1.5, AC-1.6, AC-1.14 |
| 前置依赖 | TASK-CLI-001 |
| 非目标 | 不验证工具不存在/参数非法（TASK-CLI-005） |
| 完成判据 | 三类调用方均被正确拒绝并返回对应错误码，失败经 HiSysEvent 上报 |
| 停止条件 | 权限/沙箱判定与 spec 不一致 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/cli_tool_mgr_client_test/` | 权限/非HAP 用例 |

**Spec Context**

AC-1.5：非系统应用→reject 35700008。AC-1.6：缺权限→reject 35700007。AC-1.14：非 HAP→reject 35700003。
R-5/R-11。

**Design Context**

ValidateExecToolPermissions：IsSystemAppByFullTokenID→ERR_NOT_SYSTEM_APP；VerifyAccessToken(EXEC_CLI_TOOL)→ERR_PERMISSION_DENIED。ValidateAndPrepareTool→GenerateSandboxConfig 失败→ERR_NOT_HAP。ReportCliExecuteFailed 上报。设计「安全基础检查」信任边界。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-API-LEVEL | Must：System API + 系统应用 + EXEC_CLI_TOOL |
| OH-ARCH-ERROR-LOG | Must：失败经 HiSysEvent |

**Steps**

- [ ] 写失败测试：三类调用方断言对应错误码
- [ ] 运行测试
- [ ] 最小修正
- [ ] 运行通过
- [ ] 记录证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts cli_tool_mgr_client_test` | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 验证权限/非HAP 异常路径错误码（AC-1.5/1.6/1.14） |
| 允许修改 | `cli_tool_manager_service.cpp`、`cli_tool_mgr_client.cpp`、对应测试 |
| 允许新建 | 测试用例 |
| 只读参考 | spec AC-1.5/1.6/1.14 + R-5/R-11、design 安全基础检查 |
| Spec 摘要 | 非系统→35700008；缺权限→35700007；非HAP→35700003 |
| Design 摘要 | ValidateExecToolPermissions + GenerateSandboxConfig + ReportCliExecuteFailed |
| 执行步骤 | 写失败测试→运行→修正→通过→证据 |
| 验证命令 | `run -t UT -ts cli_tool_mgr_client_test` / 期望: PASS |
| 完成规则 | 不改范围外文件；无 evidence 不声明完成 |

### TASK-CLI-005: 工具/子命令/schema/超时边界校验验证

| 字段 | 内容 |
|------|-----|
| 任务目标 | 验证工具/子命令不存在、schema 不匹配、timeout/yieldMs 越界返回 35700005/35700002（AC-1.7–1.11） |
| AC 映射 | AC-1.7, AC-1.8, AC-1.9, AC-1.10, AC-1.11 |
| 前置依赖 | TASK-CLI-001 |
| 非目标 | 不验证权限路径（TASK-CLI-004） |
| 完成判据 | 各非法输入返回正确错误码；空 args 合法；help 特殊键放行 |
| 停止条件 | 校验逻辑与 spec 不一致 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/tool_util_test/` | 工具/schema 校验 |
| Test | `cli_tool_framework/test/unittest/exec_options_test/` | timeout/yieldMs 边界 |
| Test | `cli_tool_framework/test/unittest/exec_tool_param_test/` | Parcelable + schema |

**Spec Context**

AC-1.7：toolName 未注册→35700005。AC-1.8：subcommand 非法→35700005。AC-1.9：args 不符 schema→35700002。AC-1.10：timeout<0 或 >1800→35700002。AC-1.11：非后台 yieldMs>timeout*1000→35700002。
R-6/R-7/R-8。

**Design Context**

ValidateProperties：subcommand 非空但工具无子命令/不在表→ERR_TOOL_NOT_EXIST。ValidateExecOptionsProperties：timeout<0/yieldMs<0/timeout>1800/yieldMs>timeout*1000→ERR_INVALID_PARAM。ValidateInputSchemaProperties：args 键须在 properties 且类型匹配；help 单独放行；空 args 合法。接口参数规约表。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-ERROR-LOG | Must：错误码 35700005/35700002 |

**Steps**

- [ ] 写失败测试：各非法输入断言错误码
- [ ] 运行测试
- [ ] 最小修正
- [ ] 运行通过
- [ ] 记录证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts tool_util_test` | PASS |
| 测试 | `run -t UT -ts exec_options_test` | PASS |
| 测试 | `run -t UT -ts exec_tool_param_test` | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 验证工具/子命令/schema/超时边界校验（AC-1.7–1.11） |
| 允许修改 | `tool_util.cpp`、`exec_options.cpp`、`exec_tool_param.cpp`、对应测试 |
| 允许新建 | 测试用例 |
| 只读参考 | spec AC-1.7–1.11 + R-6/R-7/R-8、design 接口参数规约 |
| Spec 摘要 | 工具/子命令→35700005；schema/超时边界→35700002 |
| Design 摘要 | ValidateProperties + ValidateExecOptionsProperties + ValidateInputSchemaProperties |
| 执行步骤 | 写失败测试→运行→修正→通过→证据 |
| 验证命令 | `run -t UT -ts tool_util_test` / 期望: PASS |
| 完成规则 | 不改范围外文件；无 evidence 不声明完成 |

### TASK-CLI-006: NAPI 参数同步校验验证

| 字段 | 内容 |
|------|-----|
| 任务目标 | 验证必填缺失/类型错/argc<4 同步抛 invalid parameter 异常（AC-1.12） |
| AC 映射 | AC-1.12 |
| 前置依赖 | TASK-CLI-001 |
| 非目标 | 不验证服务端校验（TASK-CLI-005） |
| 完成判据 | toolName/challenge 空或非串、args 非对象、argc<4 均同步抛错，不进入异步 |
| 停止条件 | NAPI 校验与 spec 不一致 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/cli_tool_mgr_client_test/` | NAPI 校验用例 |

**Spec Context**

AC-1.12：toolName 空/非串、challenge 空/非串、args 非对象、实参<4 → 同步抛 invalid parameter 异常。
R-9：NAPI 层校验先于服务端。

**Design Context**

OnExecTool：argc<INDEX_FOUR→ThrowTooFewParametersError；UnwrapStringFromJS2(toolName) 失败或空→ThrowInvalidParamError("Tool toolName is required")；subcommand 非串→抛错；UnwrapWantParams(args) 失败→抛错；challenge 空或非串→抛错；UnwrapExecOptions 失败→抛错。BindNativeFunction("execTool")。调用链层级 NAPI 层。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | Must：NAPI 层做同步参数校验先于 IPC |

**Steps**

- [ ] 写失败测试：各缺失/类型错/argc<4 断言同步抛错
- [ ] 运行测试
- [ ] 最小修正
- [ ] 运行通过
- [ ] 记录证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts cli_tool_mgr_client_test` | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 验证 NAPI 参数同步校验（AC-1.12） |
| 允许修改 | `js_cli_manager.cpp`、对应测试 |
| 允许新建 | 测试用例 |
| 只读参考 | spec AC-1.12 + R-9、design 调用链 NAPI 层 |
| Spec 摘要 | 必填缺失/类型错/argc<4→同步抛错 |
| Design 摘要 | OnExecTool 参数解析与 ThrowInvalidParamError |
| 执行步骤 | 写失败测试→运行→修正→通过→证据 |
| 验证命令 | `run -t UT -ts cli_tool_mgr_client_test` / 期望: PASS |
| 完成规则 | 不改范围外文件；无 evidence 不声明完成 |

### TASK-CLI-007: 会话并发上限验证

| 字段 | 内容 |
|------|-----|
| 任务目标 | 验证活动会话数达系统上限时返回 35700001（AC-1.13） |
| AC 映射 | AC-1.13 |
| 前置依赖 | TASK-CLI-001 |
| 非目标 | 不验证单会话内部行为 |
| 完成判据 | 会话数≥上限→reject 35700001，不排队 |
| 停止条件 | 并发保护与 spec 不一致 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/cli_tool_mgr_service_test/` | 上限用例 |

**Spec Context**

AC-1.13：会话数达上限→reject 35700001。
R-10：全局会话表 + 系统配置上限。

**Design Context**

ValidateSessionLimit：GetCliConcurrencyLimit→sessions_ 表 size≥上限→ERR_SESSION_LIMIT_EXCEEDED。sessionsMutex_ 互斥保护。设计 ADR-5。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-ERROR-LOG | Must：错误码 35700001 |

**Steps**

- [ ] 写失败测试：填满会话后调用断言 35700001
- [ ] 运行测试
- [ ] 最小修正
- [ ] 运行通过
- [ ] 记录证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts cli_tool_mgr_service_test` | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 验证会话并发上限（AC-1.13） |
| 允许修改 | `cli_tool_manager_service.cpp`、对应测试 |
| 允许新建 | 测试用例 |
| 只读参考 | spec AC-1.13 + R-10、design ADR-5 |
| Spec 摘要 | 超上限→35700001 |
| Design 摘要 | ValidateSessionLimit + sessions_ mutex |
| 执行步骤 | 写失败测试→运行→修正→通过→证据 |
| 验证命令 | `run -t UT -ts cli_tool_mgr_service_test` / 期望: PASS |
| 完成规则 | 不改范围外文件；无 evidence 不声明完成 |

### TASK-CLI-008: DFX 失败上报 + SA 不可用验证

| 字段 | 内容 |
|------|-----|
| 任务目标 | 验证 SA 不可用返回 35700000（AC-1.15）；失败路径经 HiSysEvent 上报 bundleName/toolName/failureReason |
| AC 映射 | AC-1.15 |
| 前置依赖 | TASK-CLI-004 |
| 非目标 | 不验证正常路径 DFX |
| 完成判据 | SA 拉起失败/代理空→reject 35700000；失败路径 HiSysEvent 字段完整 |
| 停止条件 | DFX 事件与 hisysevent.yaml 定义冲突 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/cli_event_report_test/` | DFX 上报用例 |
| Test | `cli_tool_framework/test/unittest/cli_tool_mgr_client_test/` | SA 不可用用例 |

**Spec Context**

AC-1.15：CliToolManager SA 未就绪/代理获取失败→reject 35700000。
R-12：SA 未加载/代理空→35700000。

**Design Context**

cli_tool_client LoadCliToolMgrService 失败→GET_CLI_TOOL_MGR_SERVICE_FAILED。ReportCliExecuteFailed/ReportCliTimeout 上报 bundleName/toolName/failureReason/duration。hisysevent.yaml 为硬性合约。设计异常传播时序图。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-ERROR-LOG | Must：不破坏 hisysevent.yaml 已发布事件；不为通过测试删事件 |
| OH-ARCH-IPC-SAF | Must：SA 按需拉起，失败回 35700000 |

**Steps**

- [ ] 写失败测试：SA 不可用断言 35700000；失败上报断言 HiSysEvent 字段
- [ ] 运行测试
- [ ] 最小修正
- [ ] 校验 hisysevent.yaml 未被破坏
- [ ] 运行通过
- [ ] 记录证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts cli_event_report_test` | PASS |
| 测试 | `run -t UT -ts cli_tool_mgr_client_test` | PASS |
| 静态检查 | `hisysevent.yaml` 事件定义未被破坏 | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 验证 SA 不可用 35700000 + 失败 HiSysEvent 上报（AC-1.15） |
| 允许修改 | `cli_event_report*.cpp`、`cli_tool_mgr_client.cpp`、对应测试 |
| 允许新建 | 测试用例 |
| 只读参考 | spec AC-1.15 + R-12、design 异常传播时序图、`hisysevent.yaml` |
| Spec 摘要 | SA 不可用→35700000；失败经 HiSysEvent |
| Design 摘要 | LoadCliToolMgrService + ReportCliExecuteFailed/ReportCliTimeout |
| 执行步骤 | 写失败测试→运行→修正→校验 yaml→通过→证据 |
| 验证命令 | `run -t UT -ts cli_event_report_test` / 期望: PASS |
| 完成规则 | 不改 hisysevent.yaml 已发布事件；不改范围外文件；无 evidence 不声明完成 |

## Plan 自审清单

- [x] 每个 P0/P1 AC 至少映射到一个 Task（AC-1.1–1.15 全覆盖）
- [x] 每个 Task 文件范围明确
- [x] 每个 Task 明确前置依赖、非目标、完成判据和停止条件
- [x] 每个 Task 有验证命令
- [x] Task 粒度形成能力闭环
- [x] 没有 TBD/TODO/占位符（「待确认」为 Owner 决议项）
- [x] 没有要求 Agent 自行寻找未列出的上下文
- [x] 交接信息自包含（Handoff Summary 完整）
- [x] 每个 Task 验证在完成时立即执行并记录证据
- [x] 超 3000 行阈值的 Task 已说明不拆分理由（本计划 Task 均聚焦验证+补测试，上下文未超阈值）
