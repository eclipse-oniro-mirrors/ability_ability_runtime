# AGENTS.md

## 1. 概述

本 AGENTS.md 用于指导开发与应用运行时管理相关特性分析、需求导入、设计、编码及验证各环节。在SDD流程开发各阶段，涉及本代码仓所有修改需要遵从本文档描述及其指向的其他更细则文档的内容，如果本文档指向的其他文档存在与本文档规定冲突内容，以本文档内容为准。本文档描述内容包括代码目录、知识体系、约束与限制三个章节，其中代码目录等内容就在本代码仓中，所引用的详细知识库内容均引自abilitykit_sdd，在执行具体要求前请确认该知识库已在本地存在，知识库的检索路径一般满足如下规则:
    - Windows: %LOCALAPPDATA%\sdd-insight\kb\（即 C:\Users\<工号对应机器用户>\AppData\Local\sdd-insight\kb\）
    - Linux / macOS（含 WSL 跑 opencode 的情况）:	~/.local/share/sdd-insight/kb/
如不存在，则应代码仓同级目录下执行git clone https://gitcode.com/abilitykit_public/abilitykit_sdd.git 下载代码仓后，在相应的代码仓中查找具体规则。
本仓库实现 OpenHarmony **元能力运行时（ability_runtime）**，核心职责是管理应用生命周期、组件调度、Extension 扩展组件管理、URI 权限管理和意图框架。最重要的架构边界是 **服务层（services/）与公共 SDK 接口层（interfaces/kits/）之间的双向解耦**——服务不可直接暴露内部实现到 SDK，SDK 不可反向依赖服务内部细节。

### 代码目录职责

| 目录                                                       | 职责                                                         | 高风险标记                                 |
| ---------------------------------------------------------- | ------------------------------------------------------------ | ------------------------------------------ |
| `services/abilitymgr/`                                     | AbilityManagerService（SA 180）：组件生命周期、启动/停止、连接、任务栈、拦截器、InsightIntent | 🔴 修改影响全局组件调度                     |
| `services/appmgr/`                                         | AppManagerService（SA 501）：应用进程生命周期、AppSpawn 协调、进程状态、子进程 | 🔴 修改影响所有应用进程                     |
| `services/uripermmgr/`                                     | UriPermissionManagerService（SA 183）：跨应用 URI 权限授予/撤销/校验 | 🔴 修改影响安全边界                         |
| `services/quickfixmgr/`                                    | QuickFixManagerService（SA 184）：应用热修复补丁管理         | 🟡 修改影响应用更新流程                     |
| `services/appdfr/`                                         | 应用 DFR：AppFreeze 管理、ANR 监听、崩溃收集                 | 🟡 修改影响故障归因                         |
| `services/common/`                                         | 服务侧工具封装：权限校验、日志封装、事件上报、HiSysEvent、XCollie | 🔴 包含 PermissionVerification 全局安全入口 |
| `interfaces/kits/native/`                                  | 公共 SDK 接口：Ability、Context、Extension、Caller/Callee、AppStartup | 🔴 公共 API 兼容性边界                      |
| `interfaces/kits/c/`                                       | C 语言公共 API                                               | 🔴 公共 API 兼容性边界                      |
| `interfaces/inner_api/`                                    | 内部组件接口：IPC 客户端/代理/桩、错误码工具                 | 🟡 内部接口，但仍需注意 IPC 兼容性          |
| `frameworks/native/ability/native/`                        | 原生框架实现：UIAbility、各 Extension、Context、InsightIntent 执行器 | 🟢 客户端侧实现，可随 SDK 接口联动修改      |
| `frameworks/native/appkit/`                                | 应用框架：Application、AbilityStage、TestRunner              | 🟢                                          |
| `frameworks/js/napi/`                                      | JavaScript NAPI 绑定                                         | 🟡 NAPI 签名是公共 API                      |
| `frameworks/ets/ani/`                                      | ArkTS ANI 绑定                                               | 🟡 ANI 签名是公共 API                       |
| `frameworks/cj/ffi/`                                       | CJ FFI 绑定                                                  | 🟡                                          |
| `frameworks/c/`                                            | C API 框架实现                                               | 🟡                                          |
| `agent_runtime_framework/`                                 | Agent 组件框架：AgentManagerService（SA 185）、AgentExtensinoAbility与AgentUIExtensionAbility | 🟢                                          |
| `tools/aa/`                                                | aa 命令行工具：Ability 调试管理                              | 🟢                                          |
| `js_environment/` / `ets_environment/` / `cj_environment/` | JS/ETS/CJ 运行时环境                                         | 🟢                                          |



## 2. 知识体系

### 2.1 Summarys: 领域概念与概述

**文档路径**：abilitykit_sdd/summarys

**文档内容**：abilitykit业务范围关键概念、概述、设计约束等基础知识。

**文档适用**：涉及abilitykit领域的相关问题、需求及业务讨论，均需要遵从本目录下文档所述相关术语、概念及要求。例如用户说"多HAP需要新增一个XXX规格"，需要先通过本目录下的文档查询多HAP相关概念。

**SDD适用**：可在Define/Specify/Design/Plan等各阶段使用，重点在Define阶段使用。

### 2.2 Standards：编码与设计规范

**文档路径**：abilitykit_sdd/standards

**文档内容**：abilitykit业务范围编码规范要求及通用设计规范要求基础知识。

**文档适用**：涉及abilitykit领域相关代码仓（主要包括ability_ability_runtime/ability_ability_base/bundlemanager_bundle_framework等代码仓）的代码修改，均需要严格遵从本文档规则。

**SDD适用**：可在Design/Plan等各阶段使用，重点在Plan阶段使用。

### 2.3 Specifications：模块场景与特性规格

**文档路径**：abilitykit_sdd/specifications

**文档内容**：abilitykit业务范围全量模块、业务场景及特性规格等基础知识。

**文档适用**：涉及abilitykit领域的相关问题、需求及业务讨论，均需要参考本目录下相关模块、场景及特性等基础知识。新增需求或业务讨论如果是或者新增模块、场景或特性，则需要关注不要与现有规格出现冲突；新增需求或业务讨论如果是对原有模块、场景或特性进行调整的，则需要和用户交互明确相关模块、场景和特性变更点。需要注意：不管是哪种情况，一旦出现specifications内容的变更（新增、删除或调整），都必须以当前用户身份向abilitykit_sdd代码仓中按照原有格式提交变更，并推送到远端请求Committer合入。

**SDD适用**：可在Define/Specify/Design/Plan等各阶段使用，重点在Specify/Design阶段使用。

### 2.4 Suits：研发工具套件

**文档路径**：abilitykit_sdd/suits

**文档内容**：abilitykit业务范围开发自测试工具套件。

**文档适用**：涉及abilitykit领域的代码在完成编码后进行测试验证阶段时使用本目录下的相关工具。

**SDD适用**：可在Plan阶段及Plan之后的阶段使用。

### 2.5 Special-Projects：专项知识

**文档路径**：abilitykit_sdd/special-projects

**文档内容**：编译构建、文档撰写等专项基础知识库，作为 standards/08-constraints.md 中相关 HARD 规则的支撑上下文。

**文档适用**：按任务类型触发检索。涉及编译/构建/BUILD.gn 配置/重构公共头文件/修改公共接口/新增源文件到测试 target 时，须检索 special-projects/build/build-guide.md 确认构建规范、依赖模式和调试策略；涉及文档撰写/修改时，须检索 special-projects/doc-writing/doc-writing-guide.md 确认结构模板和风格规则。

**SDD适用**：可在Plan阶段及Plan之后的阶段（Coding/Verify）按任务类型触发使用。

## 3.编码、验证、检视及上库规则

### 3.1 代码编写：Coding

**触发条件**：SDD流程中Plan阶段主动触发或用户主动要求生成代码时触发。

**执行规则**：

1）编码前严格读取abilitykit_sdd/standards所有规则后开始编码，编码及后续修改不得出现违反规则的实现。

2）涉及编译/构建/BUILD.gn 配置/重构公共头文件/修改公共接口/新增源文件到测试 target 时，须额外检索 abilitykit_sdd/special-projects/build/build-guide.md 确认构建规范、依赖模式和调试策略。

3）所有业务代码修改都需要包含测试用例的同步编写和生成，其中ArkTS 接口的XTS可参考[ArkTS XTS 生成Skill](https://gitcode.com/openharmony-ai-testdesign/oh-test-skills/tree/main/ohos-test-design/skills-refactor/ohos-test-arkts-xts-generation)生成，CAPI接口的XTS可参考[CAPI XTS 生成Skill](https://gitcode.com/openharmony-ai-testdesign/oh-test-skills/tree/main/ohos-test-design/skills-refactor/ohos-test-capi-xts-generation)

3）**InnerAPI 自动 UT 生成**（触发判定：变更涉及位于 `interfaces/inner_api/` 且无 d.ts/NAPI 适配层的 C++ 接口，消费方为命令行工具或系统 SA——按新增/变更的 client/proxy 公开方法逐个判定）：

   - **生成时机在本阶段（与代码同步编写）**：自动加载 `abilitykit_sdd/suits/ohos-innerapi-ut-gen` skill，按其步骤生成**专用 UT 目标**（mock 基础设施 + 每方法至少三类用例：null proxy / 失败传播 / 成功转发 + BUILD.gn）并完成 ninja 编译验证；
   - **模式约束**：必须采用该 skill 的「client .cpp 编入测试 + samgr 链 shadow（iservice_registry.h 同名替换）」专用目标模式——自包含单一宏视图，规避跨库虚调用 CFI 陷阱；禁止依赖"测试二进制内 mock + 真实 .so 内分发"的跨 DSO 模式（__cfi_check 会拦截，本仓已实证）；测试目标的 `defines` 必须与被测库 TU 的 define 集合对齐（防止 vtable 槽位错位）；
   - **执行归 3.2 Verify**：生成的专用 UT 目标与其他 UT 一起纳入模拟器执行集，结果按 skill 要求回填测试目录下的 `ut_report.md`；
   - **mock 维护**：若该 InnerAPI 接口新增了虚函数（带默认体或纯虚），需同步为既有完整接口 mock（如 `mock_iabilitymanager.h`）补齐 override，保持 mock 完整性。

### 3.2 功能验证：Verify

**触发条件**：完成编码后**自动触发**（无需等待用户显式要求；SDD 流程中由 Verify 阶段承接）；用户主动要求验证时可随时提前触发。

**定位**：模拟器验证（模式 B：构建变更库 → 热替换 → UT + IPC 冒烟 + 原路径回归）是 Verify 的**默认必做环节与整个交付循环的组成部分**，不是可条件跳过的探测项；**不设降级决策树**——跑不起来就修，直到跑通。在模拟器验证执行完成后，需要提醒用户进行真机复测。

**执行规则**：使用 abilitykit_sdd/suits/simulator_use Skill进行系统编译、UT及XTS测试用例编译、系统烧录和测试，输出测试报告（UT 集合包含 3.1 第 3 条由 ohos-innerapi-ut-gen 生成的专用目标，结果回填其测试目录下 ut_report.md）。优先走 simulator_use 的**模式 B（增量热替换验证）**：只构建变更库并热替换到既有镜像（含 UT 执行与 IPC 冒烟），全量镜像构建仅在模式 B 前置不满足时使用。

**遇障处置（修复优先——以下都是把验证跑起来的手段，按故障类型对号入座依次尝试）**：

| 故障类型 | 处置 |
|----------|------|
| 构建图/环境级故障（ninja 报 `multiple rules generate`、gn regen 报旧绝对路径、CMake 缓存拒绝构建等，error 不在本次修改文件） | ① `simulator_use/scripts/preflight_env.py --check` 诊断 → ② `--fix` 自愈（幂等，重跑一次）→ ③ out/ 生成物级修复（构建图重复边去重、状态文件时序修正等，零源码改动）→ ④ 重试构建；仍失败则带 check/fix 输出继续排查或上报用户，**不以降级收尾** |
| 代码编译错误（error 落在本次修改文件） | 修代码后重试（P8-2.19：区分 error/warning，逐条定位） |
| QEMU 起不来 / hdc 连不上 | 按 simulator_use 排障规程处理直至跑通：环境诊断（`qemu_run.sh diagnose --verbose`）、容器/无 KVM 用 `--accel tcg`、串口日志（serial.log）定位启动阶段、崩溃按 faultlog 栈定位；栈落在独立库 → 规则 13 vtable 部署闭包补推后重启宿主再验 |
| UT / IPC 冒烟用例失败 | 按问题闭环回 Coding 小循环修（用例缺陷修用例、代码缺陷修代码，P8-2.11 先归属判定），修复后全量复跑 |

**降级例外（唯一路径）**：仅在**用户明确同意放弃模拟器验证**时，才降级为编译级/结构检查 + 移交登记（反伪口径：登记降级原因、已尝试的修复手段与用户确认记录）。禁止未经用户确认以环境故障/构建故障为由自行降级。

### 3.3 代码检视：Review

**触发条件**：完成验证且验证报告通过后主动触发或当用户表达"检视一下代码""帮我审一下""做一次 code review""深度扫描""安全审计""查高危 bug""接口审计"等检视意图时，需要通过abilitykit_sdd/suits/codecheck中的Skills进行代码的检视。

**执行规则**：

- 仅检视，禁止直接修改代码。
- 检视发现的问题，经过用户确认允许修改代码后，回到3.1 Coding阶段，进入小循环：Coding->Verify->Review，直到检视代码无新增问题。

### 3.4 代码上库：Commit

**触发条件**：代码检视完成且无新增问题后主动询问用户是否触发，或当用户表达"帮我提交代码"时触发。**禁止**未经用户确认直接提交代码。

**执行规则**：
1、确定修改涉及代码仓；
2、使用abilitykit_sdd/suits/atomic-fork-pr-workflow Skill提交并推送pr。