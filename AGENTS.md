# AGENTS.md

## 1. Code map

本 AGENTS.md 适用于 ability_runtime 仓库根目录。子目录 `services/abilitymgr/` 和 `services/uripermmgr/` 各有独立的 CLAUDE.md，提供更细粒度的架构和开发指引。

本仓库实现 OpenHarmony **元能力运行时（ability_runtime）**，核心职责是管理应用生命周期、组件调度、Extension 扩展组件管理、URI 权限管理和意图框架。最重要的架构边界是 **服务层（services/）与公共 SDK 接口层（interfaces/kits/）之间的双向解耦**——服务不可直接暴露内部实现到 SDK，SDK 不可反向依赖服务内部细节。

### 目录职责

| 目录 | 职责 | 高风险标记 |
|------|------|-----------|
| `services/abilitymgr/` | AbilityManagerService（SA 180）：组件生命周期、启动/停止、连接、任务栈、拦截器、InsightIntent | 🔴 修改影响全局组件调度 |
| `services/appmgr/` | AppManagerService（SA 501）：应用进程生命周期、AppSpawn 协调、进程状态、子进程 | 🔴 修改影响所有应用进程 |
| `services/uripermmgr/` | UriPermissionManagerService（SA 183）：跨应用 URI 权限授予/撤销/校验 | 🔴 修改影响安全边界 |
| `services/quickfixmgr/` | QuickFixManagerService（SA 184）：应用热修复补丁管理 | 🟡 修改影响应用更新流程 |
| `services/appdfr/` | 应用 DFR：AppFreeze 管理、ANR 监听、崩溃收集 | 🟡 修改影响故障归因 |
| `services/common/` | 服务侧工具封装：权限校验、日志封装、事件上报、HiSysEvent、XCollie | 🔴 包含 PermissionVerification 全局安全入口 |
| `interfaces/kits/native/` | 公共 SDK 接口：Ability、Context、Extension、Caller/Callee、AppStartup | 🔴 公共 API 兼容性边界 |
| `interfaces/kits/c/` | C 语言公共 API | 🔴 公共 API 兼容性边界 |
| `interfaces/inner_api/` | 内部组件接口：IPC 客户端/代理/桩、错误码工具 | 🟡 内部接口，但仍需注意 IPC 兼容性 |
| `frameworks/native/ability/native/` | 原生框架实现：UIAbility、各 Extension、Context、InsightIntent 执行器 | 🟢 客户端侧实现，可随 SDK 接口联动修改 |
| `frameworks/native/appkit/` | 应用框架：Application、AbilityStage、TestRunner | 🟢 |
| `frameworks/js/napi/` | JavaScript NAPI 绑定 | 🟡 NAPI 签名是公共 API |
| `frameworks/ets/ani/` | ArkTS ANI 绑定 | 🟡 ANI 签名是公共 API |
| `frameworks/cj/ffi/` | CJ FFI 绑定 | 🟡 |
| `frameworks/c/` | C API 框架实现 | 🟡 |
| `agent_runtime_framework/` | Agent 组件框架：AgentManagerService（SA 185）、AgentExtensinoAbility与AgentUIExtensionAbility | 🟢 |
| `tools/aa/` | aa 命令行工具：Ability 调试管理 | 🟢 |
| `js_environment/` / `ets_environment/` / `cj_environment/` | JS/ETS/CJ 运行时环境 | 🟢 |

### Where to look

| 任务类型 | 先看 |
|---|---|
| Ability 生命周期（启动/停止/前台/后台） | `services/abilitymgr/` → `services/abilitymgr/CLAUDE.md` |
| 应用进程管理（启动/杀死/预加载/ANR） | `services/appmgr/` → `app_mgr_service.cpp`、`app_running_manager.cpp` |
| URI 权限（授予/撤销/校验） | `services/uripermmgr/` → `services/uripermmgr/CLAUDE.md` |
| 拦截器相关（启动被拦截/生态规则/Kiosk） | `services/abilitymgr/src/interceptor/` |
| 任务栈/Mission 管理 | `services/abilitymgr/src/mission/` → `mission_list_manager.cpp` |
| InsightIntent（意图识别/分发/执行） | `services/abilitymgr/src/insight_intent/` + `frameworks/native/ability/native/insight_intent_executor/` |
| 新增 Extension 类型 | `interfaces/kits/native/` → `frameworks/native/ability/native/` → `frameworks/ets/ani/` → `frameworks/js/napi/` → `services/abilitymgr/` |
| 修改公共 SDK API 签名 | `interfaces/kits/native/` + `interfaces/kits/c/` → 需兼容性评估 |
| NAPI 绑定修改 | `frameworks/js/napi/<module>/` → 对应 `nativecommon` 共享组件 |
| ANI 绑定修改 | `frameworks/ets/ani/<module>/` → 对应 `ani_common/` 共享组件 |
| 子进程相关 | `frameworks/native/child_process/` + `services/appmgr/src/child_process_record.cpp` |
| 日志/DFX 修改 | `services/common/include/hilog_tag_wrapper.h` + `hisysevent.yaml` |
| 错误码修改 | `interfaces/inner_api/error_utils/include/ability_runtime_error_util.h` + `interfaces/kits/native/ability/native/ability_business_error/ability_business_error.h` |
| 权限校验逻辑 | `services/common/include/permission_verification.h` |
| QuickFix 热修复 | `services/quickfixmgr/src/quick_fix_manager_service.cpp` |
| AppFreeze/崩溃收集 | `services/appdfr/src/appfreeze_manager.cpp` + `application_anr_listener.cpp` |
| Agent 组件框架 | `agent_runtime_framework/services/agentmgr/` |
| 构建配置/特性开关 | `ability_runtime.gni` |
| 系统能力注册 | `services/sa_profile/` |

## 2. Knowledge routing

遇到问题先定位场景，再读对应文档。以下文档包含完整领域概念和操作指引，不是可选背景阅读。

### Task-based routing

| 任务类别 | 先读 |
|---|---|
| 公共 API 或 SDK 行为变更 | `interfaces/kits/native/ability/native/ability_business_error/ability_business_error.h`（错误码兼容性边界）+ `interfaces/inner_api/error_utils/include/ability_runtime_error_util.h` |
| AbilityManagerService 架构或子管理器变更 | `services/abilitymgr/CLAUDE.md` |
| URI 权限/安全/认证变更 | `services/uripermmgr/CLAUDE.md` + `services/common/include/permission_verification.h` |
| DFX/日志/故障归因变更 | `services/common/include/hilog_tag_wrapper.h` + `hisysevent.yaml` |
| 构建特性开关变更 | `ability_runtime.gni` |
| 应用进程管理/AppSpawn 协调变更 | `services/appmgr/` → `app_mgr_service.h`、`app_spawn_client.h` |
| 拦截器新增或修改 | `services/abilitymgr/CLAUDE.md` 中的 Interceptor Framework 章节 |
| InsightIntent 意图框架变更 | `services/abilitymgr/src/insight_intent/` 目录结构 + `核心功能特性总结.md` 意图框架章节 |

### Path-based routing

| 修改路径 | 先读 |
|---|---|
| `services/abilitymgr/` | `services/abilitymgr/CLAUDE.md` |
| `services/uripermmgr/` | `services/uripermmgr/CLAUDE.md` |
| `services/common/` | `services/common/include/permission_verification.h` + `services/common/include/hilog_tag_wrapper.h` |
| `interfaces/kits/` | `interfaces/kits/native/ability/native/ability_business_error/ability_business_error.h`（公共 API 兼容性） |
| `interfaces/inner_api/error_utils/` | `interfaces/inner_api/error_utils/include/ability_runtime_error_util.h` |
| `frameworks/js/napi/` | 对应模块目录的 `native_module.cpp`（NAPI 模块注册入口） |
| `frameworks/ets/ani/` | 对应模块目录的 ANI 注册入口 + `ani_common/` 共享工具 |
| `services/appdfr/` | `services/appmgr/CLAUDE.md`（如存在）或 `services/appdfr/` 目录头文件 |

### Vocabulary-based routing

当任务、issue、日志、API 名称或变更文件包含以下术语时，在规划前先读对应文档：

| 术语 | 风险提示 | 先读 |
|---|---|---|
| UIAbility / Ability | 不是通用"能力"，而是 OpenHarmony 有界面的应用组件核心抽象，有严格生命周期状态机 | `services/abilitymgr/CLAUDE.md` |
| Extension / ExtensionAbility | 不是通用"扩展"，而是 OpenHarmony 特定的无界面服务组件体系（10+ 类型各有独立生命周期） | `services/abilitymgr/CLAUDE.md` |
| Want / WantAgent | 不是通用"意图"，而是 OpenHarmony 组件间调度的结构化描述对象，包含 bundle/module/ability 元数据 | `interfaces/inner_api/wantagent/` |
| Mission / 任务栈 | 不是通用"任务"，而是 OpenHarmony 管理多 Ability 实例的栈结构，有持久化和跨设备同步 | `services/abilitymgr/CLAUDE.md` 中 Mission 章节 |
| AppSpawn / 应用孵化 | 不是通用"进程创建"，而是 OpenHarmony 专属的应用进程孵化服务，有安全沙箱和参数注入机制 | `services/appmgr/` → `app_spawn_client.h` |
| URI Permission / UriPerm | 不是通用"文件权限"，而是跨应用临时 URI 访问授权，涉及 Media/Docs/Sandbox 三种分发策略 | `services/uripermmgr/CLAUDE.md` |
| InsightIntent / 意图框架 | 不是通用"AI 意图"，而是 OpenHarmony 系统级意图标准体系，连接应用内业务功能 | 意图框架章节 |
| Kiosk / 展台模式 | 不是通用"锁定"，而是系统级设备锁定运行模式，涉及拦截器链和启动控制 | `services/abilitymgr/CLAUDE.md` |
| Interceptor / 拦截器 | 不是通用"中间件"，而是 AbilityManagerService 的启动拦截链，10+ 拦截器按顺序执行 | `services/abilitymgr/CLAUDE.md` 中 Interceptor 章节 |
| DFX / HiLog / HiSysEvent / XCollie | 不是通用"日志"，而是 OpenHarmony 故障归因体系，事件定义在 hisysevent.yaml 是硬性约束 | `hisysevent.yaml` + `hilog_tag_wrapper.h` |
| FreeInstall / 免安装 | 不是通用"按需加载"，而是 OpenHarmony 原子化服务免安装机制，涉及 Bundle 和分发协调 | `services/abilitymgr/src/free_install_manager.cpp` |
| KeepAlive / 保活 | 不是通用"后台运行"，而是系统级关键进程保活机制，有独立数据管理 | `services/abilitymgr/src/keep_alive/` |
| SA / SystemAbility / 系统能力 | 不是通用"微服务"，而是 OpenHarmony 通过 samgr 注册的系统能力，有 SA ID、进程绑定和按需启动配置 | `services/sa_profile/` |
| NAPI / ANI / CJ FFI | 三种多语言绑定机制，共享 nativecommon/ani_common 组件，签名变更都是公共 API 变更 | `frameworks/js/napi/` + `frameworks/ets/ani/ani_common/` |
| PermissionVerification | 不是通用"权限检查"，而是 ability_runtime 的全局安全入口，所有组件操作都经过此单例 | `services/common/include/permission_verification.h` |
| QuickFix / 热修复 | 不是通用"热更新"，而是 OpenHarmony 应用级补丁机制，有 Apply/Revert 和版本管理 | `services/quickfixmgr/` |
| AutoFill / 自动填充 | 不是通用"表单填充"，而是跨应用隐私保护的自动填充扩展，有独立的 Extension 和 Manager | `ability_runtime.gni` 中 `ability_runtime_auto_fill` |

在规划中声明：
- 任务类别
- 已读知识文档
- 发现的约束
- 是否需要使用 Skill 或子目录 CLAUDE.md

## 3. Constraints and boundaries

### Architecture/domain invariants

- 公共 SDK 接口（`interfaces/kits/`）表达稳定的能力意图，不暴露服务内部实现细节。
- 权限校验必须在能力入口点执行：所有 Ability/Extension 的启动、连接、数据操作均经过 `PermissionVerification` 单例。
- 服务层（`services/`）与框架层（`frameworks/`）通过 `interfaces/inner_api/` 的 IPC 接口解耦，不直接引用对方内部头文件。
- `services/common/` 是服务层共享基础设施，不依赖特定子服务业务逻辑。
- DFX 事件定义（`hisysevent.yaml`）是故障归因的硬性合约，事件名、参数类型、级别不可随意修改。
- `ability_runtime.gni` 中的特性标志控制编译时行为，默认值变更影响所有下游产品。
- FA 模型和 Stage 模型共存：修改任一模型的行为时，必须确认不影响另一模型。
- Mission 持久化数据格式（RDB）是跨版本兼容性约束，不可破坏已写入的数据结构。

### Do not

- 不要在 `interfaces/kits/` 中修改公共 API 签名、错误码值、生命周期语义，除非任务明确要求且有兼容性评估。
- 不要在 `services/abilitymgr/` 或 `services/appmgr/` 中绕过 `PermissionVerification` 的权限校验。
- 不要修改 IPC Stub/Proxy 的序列化格式——它影响跨版本和跨进程兼容性。
- 不要修改 `hisysevent.yaml` 中已发布事件的参数类型或删除参数——影响线上故障归因。
- 不要修改 `ability_runtime_error_util.h` 和 `ability_business_error.h` 中已发布的错误码数值——它们是公共合约。
- 不要为通过测试而删除日志、HiSysEvent 事件、错误码或诊断信息。
- 不要在 `services/abilitymgr/` 中直接调用 AppManagerService 内部方法——应通过 `interfaces/inner_api/app_manager/` 的 IPC 接口。
- 不要在 `frameworks/native/` 中直接调用 `services/` 的内部头文件——应通过 `interfaces/inner_api/` 的客户端接口。
- 不要跳过 BUILD.gn 更新——添加源文件后必须更新对应 BUILD.gn 和 `.gni` 源文件列表。
- 不要新增第三方依赖——必须先在 `bundle.json` 中声明并通过 License 审查。

### Ask before

- 修改 `interfaces/kits/` 中任何已有 API 的签名或语义。
- 涉及安全/权限/信任的行为变更（特别是 URI 权限、AppSpawn 安全参数、PermissionVerification 规则）。
- 涉及 IPC 协议兼容性或序列化格式变更。
- 涉及 Mission 持久化数据格式变更——影响跨版本数据兼容性。
- 新增或修改第三方依赖、License。
- 删除或重命名公共 API。
- 修改 `ability_runtime.gni` 中特性标志的默认值。
- 修改 `services/sa_profile/` 中 SA 注册配置（SA ID、进程名、库路径）。
- 修改 `hisysevent.yaml` 中已发布事件的定义。
- 修改 FA 模型相关代码（API 8 及更早版本的兼容性约束）。
- 修改 AppSpawn 协调逻辑（影响应用进程安全沙箱）。
- 修改 AppFreeze 或 ANR 检测逻辑（影响线上故障归因）。

### 反模式（不要这样做）

- ❌ 在 `services/abilitymgr/` 中直接调用 AppMgrService 内部方法（应通过 IPC 接口）
- ❌ 在 `frameworks/` 中直接引用 `services/` 内部头文件（应通过 `interfaces/inner_api/`）
- ❌ 不经兼容性评估修改公共 API 签名或错误码值（应先评估影响范围）
- ❌ 为通过测试删除 HiSysEvent 事件或日志（应修复根因）
- ❌ 绕过 PermissionVerification 权限校验（应确认权限规则）
- ❌ 修改 IPC Stub/Proxy 序列化格式但不更新版本协商（应同时更新兼容性处理）
- ❌ 添加源文件但不更新 BUILD.gn（应同步更新构建定义）
- ❌ 修改特性标志默认值但不通知产品配置团队（应先协调）

## 4. Verification

构建命令从 OpenHarmony 源码根目录执行，不在本子目录执行。

### Minimum checks

- 编译验证：`./build.sh --product-name <product> --build-target ability_runtime`
- 编译特定服务：`./build.sh --product-name <product> --build-target abilityms`（AMS）或 `libappms`（AppMS）
- 单元测试：`run -t UT -tp ability_runtime`
- 模块测试：`run -t UT -ts ability_caller_fw_module_test`
- Fuzz 测试：`run -t UT -ts AbilityAppDebugInfoFuzzTest`

### Task-specific checks

| 变更类型 | 最小验证 |
|---|---|
| 修改 AbilityManagerService 内部实现 | 编译通过 + `ability_manager_service_first_test` 及后续编号单测 |
| 修改 AppManagerService 内部实现 | 编译通过 + `app_manager_service_test` 相关单测 |
| 修改 URI 权限逻辑 | 编译通过 + `uri_permission_manager_test` + `uri_permission_impl_test` |
| 修改拦截器 | 编译通过 + `ability_connect_manager_test` + 相关拦截器单测 |
| 新增/修改公共 API（interfaces/kits/） | 编译通过 + 全量单测 + NAPI/ANI/CJ 各语言绑定编译 + 兼容性评估 + `ability_business_error.h` 错误码一致性检查 |
| 新增/修改 NAPI 绑定 | 编译通过 + 对应 NAPI 模块单测 |
| 新增/修改 ANI 绑定 | 编译通过 + 对应 ANI 模块单测 |
| 修改 DFX/日志 | 编译通过 + `hisysevent.yaml` 事件定义未被破坏 + 相关 DFR 单测 |
| 修改特性标志 | 编译通过 + 受影响模块单测 + 检查 `ability_runtime.gni` 与 `bundle.json` features 一致性 |
| 新增 Extension 类型 | 编译通过 + 按步骤清单（7 步）全部完成 + 各层绑定编译 + 服务端管理单测 |
| 仅测试变更 | 运行变更的测试 + 至少一个相邻相关测试 |

### Done definition

任务完成仅当：
1. 请求的行为已实现。
2. 相关编译/测试/lint/兼容性检查已运行，或已说明无法运行的原因。
3. 最终回复包含：变更摘要、变更文件列表、验证命令和结果、兼容性/权限/DFX 影响评估（如相关）。
4. 不包含无关的格式化、重构或附带变更。
5. 如涉及公共 API 变更，已标注兼容性影响。

### Final response format

完成任务时，回复应包含：
- 变更摘要
- 变更文件列表
- 验证命令和结果
- 兼容性、权限、DFX 或跨设备影响（如相关）
- 遗留风险或后续项