# AGENTS.md

## 1. 概述

本 AGENTS.md 用于指导开发与应用运行时管理相关特性分析、需求导入、设计、编码及验证各环节。在SDD流程开发各阶段，涉及本代码仓所有修改需要遵从本文档描述及其指向的其他更细则文档的内容，如果本文档指向的其他文档存在与本文档规定冲突内容，以本文档内容为准。本文档描述内容包括代码目录、知识体系、约束与限制三个章节，其中代码目录等内容就在本代码仓中，所引用的详细知识库内容均引自https://gitcode.com/abilitykit_public/abilitykit_sdd，在执行具体要求前，应在ability_runtime代码仓同级目录下执行git clone https://gitcode.com/abilitykit_public/abilitykit_sdd.git 下载代码仓后，在相应的代码仓中查找具体规则。

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



## 3.编码、验证、检视及上库规则

### 3.1 代码编写：Coding

**触发条件**：SDD流程中Plan阶段主动触发或用户主动要求生成代码时触发。

**执行规则**：

1）编码前严格读取abilitykit_sdd/standards所有规则后开始编码，编码及后续修改不得出现违反规则的实现。

2）所有业务代码修改都需要包含测试用例的同步编写和生成，其中ArkTS 接口的XTS可参考[ArkTS XTS 生成Skill](https://gitcode.com/openharmony-ai-testdesign/oh-test-skills/tree/main/ohos-test-design/skills-refactor/ohos-test-arkts-xts-generation)生成，CAPI接口的XTS可参考[CAPI XTS 生成Skill](https://gitcode.com/openharmony-ai-testdesign/oh-test-skills/tree/main/ohos-test-design/skills-refactor/ohos-test-capi-xts-generation)

### 3.2 功能验证：Verify

**触发条件**：完成编码之后，用户要求执行功能验证时触发，需要注意这里的流程是在模拟器中执行，在执行完模拟器后，需要提醒用户进行真机复测。

**执行规则**：使用 abilitykit_sdd/suits/simulator_use Skill进行系统编译、UT及XTS测试用例编译、系统烧录和测试，输出测试报告。

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
