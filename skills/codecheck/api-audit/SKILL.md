---
name: api-audit
description: |
  针对指定 Kit 的对外 API 做全量一致性审计、实现缺陷扫描与测试用例完备度评估，输出 Markdown + CSV 双格式。
  当用户提到"接口审计"、"API审计"、"资料一致性"、"实现bug扫描"、"一致性扫描"、"audit api"、
  "interface audit"、"扫描接口"、"深度扫描api"、"测试用例完备度"、"测试覆盖度评估"、"用例缺失"、
  "汇总到csv"、"导出csv"等，且上下文涉及对外 API（C API 或 JS API）时触发此 skill。
  即使用户只说"帮我扫一下 xxxKit"，也应触发。
  支持用户口头指定 Kit 名（如 abilityKit、arkuiKit），自动定位 docs/interface/framework/service/test 路径。
---

# API 全量一致性审计、实现缺陷扫描与测试完备度评估

对指定 Kit 的所有对外 API 执行**三轮**深度扫描：
- **第一轮（框架层）**：资料文档 × 接口定义 × 框架实现 三方一致性 + 参数校验完整性
- **第二轮（服务侧）**：生命周期/线程安全 + IPC 序列化 + 资源泄漏 + 错误码语义 + 文档/实现一致性
- **第三轮（测试用例）**：参数正常值 × 参数奇异值 × 边界值覆盖度评估，识别缺失场景与补充优先级

最终输出**两种格式**：
- **Markdown 报告**（`<kit-name>_api_audit.md`）：分章节叙事 + 三轮独立表格 + 关键缺陷汇总（人类阅读友好）
- **CSV 宽表**（`<kit-name>_api_audit.csv`）：一个 API 一行，三轮结果横向并排（数据筛选、pivot、批量 review 友好）

两种格式同步生成，数据等价，仅呈现方式不同。

## 路径速查表（以 ability_runtime / AbilityKit 为基准，其他 Kit 同构）

> 以下路径基于 ability_runtime 总结，扫描其他 Kit 时按同样的目录同构映射（`<subsystem>/<module>` 替换即可）。**框架侧"动态实现"与"静态实现"目录毗邻、文件名仅前缀不同，极易找错——务必按下表定位，扫错会导致整轮结论作废。**

| 类别 | 路径（ability_runtime 基准） | 说明 |
| -- | -- | -- |
| 资料文档（中文） | `docs/zh-cn/application-dev/reference/apis-ability-kit/` | `capi-*.md`（C API）+ `js-apis-*.md`（JS API） |
| JS API 接口定义 | `code/interface/sdk-js/api/` 及 `api/<子域>/` | `@ohos.app.ability.*.d.ts`（Stage 新）、`@ohos.application.*.d.ts`（FA 旧，多 deprecated）；`*.d.ets` / `*.static.d.ets` 为 static 侧声明，审 dynamic 时忽略 |
| C API 接口定义 | `code/interface/sdk_c/AbilityKit/` 下 `*.h` + `lib*.ndk.json` | `lib*.ndk.json` 含 `first_introduced` 起始版本 |
| **框架侧·动态 JS 实现（必扫）** | `code/foundation/ability/ability_runtime/frameworks/native/ability/native/ability_runtime/js_*.cpp` | NAPI 实现的 Stage 模型 Context/Ability 类，如 `js_ability_context.cpp`、`js_ui_ability.cpp`、`js_service_extension_context.cpp`。**dynamic 接口的真正框架实现入口** |
| **框架侧·动态 JS NAPI 模块（必扫）** | `code/foundation/ability/ability_runtime/frameworks/js/napi/<module>/` | NAPI 模块注册入口（`*_module.cpp`）+ 实现。Stage vs FA 用 `BUILD.gn` 的 `relative_install_dir` 区分（见陷阱 #14） |
| 框架侧·C API 实现（必扫） | `code/foundation/ability/ability_runtime/frameworks/c/ability_runtime/` | C 接口包装层，NAPI 之外另一条 dynamic 路径 |
| **框架侧·静态实现（跳过，不扫）** | `frameworks/ets/ani/`、`frameworks/ets/ets/`、`frameworks/cj/ffi/`、`frameworks/native/ability/native/ability_runtime/ets_*.cpp`、`frameworks/native/ability/native/ability_runtime/cj_*.cpp` | ArkTS(ani/ets) 与 Cangjie(cj) **静态语言**实现，对应 static 接口，不纳入扫描 |
| **服务侧实现（必扫，R2 核心）** | `code/foundation/ability/ability_runtime/services/abilitymgr/src/`（含 `ability_manager_service.cpp`）<br>`code/foundation/ability/ability_runtime/services/appmgr/src/`（含 `app_mgr_service.cpp`）<br>以及 `services/uripermmgr/src/`、`services/dataobsmgr/src/`、`services/quickfixmgr/src/` | AbilityManagerService、AppMgrService 等。**R2 深度扫描必须覆盖，曾发生遗漏 `ability_manager_service.cpp` 导致服务侧 bug 全部漏报** |
| 测试用例（public） | `code/test/xts/acts/ability/<module>/` | JS：`*.test.ets`；C API：`entry/src/main/cpp/*.cpp` + `entry/src/ohosTest/ets/test/*.test.ets` |

### 动态 vs 静态框架实现的文件级判定（最高优先级，扫错全盘作废）

| 文件特征 | 语言/接口类型 | 是否扫描 | 典型示例 |
| -- | -- | -- | -- |
| `js_*.cpp`（函数签名含 `napi_env`/`napi_callback_info`） | 动态 JS（NAPI 类实现） | ✅ 必扫 | `js_ability_context.cpp`、`js_ui_ability.cpp` |
| `frameworks/js/napi/<module>/*_module.cpp` 及同目录实现 | 动态 JS（NAPI 模块注册） | ✅ 必扫 | `ability_context_module.cpp` |
| `frameworks/c/<module>/*.cpp` | C API（dynamic） | ✅ 必扫 | `frameworks/c/ability_runtime/*.cpp` |
| `ets_*.cpp` / `frameworks/ets/ani/**` / `frameworks/ets/ets/**` | 静态 ArkTS（ANI/ETS） | ❌ 跳过 | `ets_ui_ability_instance.cpp`、`@ohos.ability.ability.ets` |
| `cj_*.cpp` / `frameworks/cj/ffi/**` | 静态 Cangjie（CJ FFI） | ❌ 跳过 | `cj_ability_context.cpp`、`cj_ability_ffi.cpp` |

> 判定口诀：**文件名前缀 `js_` + NAPI 签名 = 动态（扫）；前缀 `ets_`/`cj_` 或位于 `ets/`、`cj/` 目录 = 静态（跳过）**。同一 Context 类（如 AbilityContext）在 `frameworks/native/ability/native/ability_runtime/` 下常同时存在 `js_`/`cj_`/`ets_` 三个版本的实现文件，只扫 `js_` 那一份。

### AbilityKit 接口文件定位表（d.ts × docs × 框架cpp 三方速查，审计前先查此表）

> **用法**：排查 AbilityKit 任一接口时，先按接口名在下表查到 **d.ts 文件 / docs 资料 md / 框架侧入口 cpp** 三方路径，再进入对应 Phase 扫描，避免反复 Glob 探测找错文件（内部类型 docs 文件名与 d.ts 不对应、bundle 子模块 docs 省略前缀、UI 驼峰大小写差异等都已在此表对齐）。
>
> **来源**：由 `build_abilitykit_csv.sh` 从 `@kit.AbilityKit.d.ts` 全量 import + docs 仓 `apis-ability-kit/` + `code/foundation/` 下 `nm_modname` 注册扫描自动生成，同步落盘到 `abilitykit_interface_files.csv`。cpp 路径以 `code/foundation/` 为基准相对路径。
>
> **标注说明**：「（纯类型声明/跨子系统未定位）」= 接口/枚举类型无独立运行时实现，或位于 foundation 之外的其他子系统仓（如 access_token/permission）；「仅 system api(sys)」= 该接口只有 `-sys.md` 系统能力资料；「（资料缺失）」= docs 仓无对应 md（真实缺失）。若同一接口在 docs 仓同时存在 public 与 `-sys` 两份 md（去掉 `-sys` 后缀+lowercase 后 lcore 相同），docs 列以 `; ` 分隔合并显示为单行（非 sys 在前、`-sys` 在后），备注列标「public + system 两份」；只有 `-sys` 版时保留单行，备注「仅 system api(sys)」。

#### 模块级接口（103）

| 序号 | 接口/模块名 | d.ts 文件 | docs md 文件 | 框架侧入口 cpp | 备注 |
| -- | -- | -- | -- | -- | -- |
| 1 | `ability` | @ohos.ability.ability.d.ts | js-apis-ability-ability.md | ability/ability_runtime/frameworks/js/napi/ability/ability_module.cpp |  |
| 2 | `abilityAccessCtrl` | @ohos.abilityAccessCtrl.d.ts | js-apis-abilityAccessCtrl.md; js-apis-abilityAccessCtrl-sys.md | （纯类型声明/跨子系统未定位） | public + system 两份 |
| 3 | `errorCode` | @ohos.ability.errorCode.d.ts | js-apis-ability-errorCode.md | ability/ability_runtime/frameworks/js/napi/errorcode/ability_errorcode_module.cpp |  |
| 4 | `featureAbility` | @ohos.ability.featureAbility.d.ts | js-apis-ability-featureAbility.md | ability/ability_runtime/frameworks/js/napi/featureAbility/native_module.cpp |  |
| 5 | `particleAbility` | @ohos.ability.particleAbility.d.ts | js-apis-ability-particleAbility.md | ability/ability_runtime/frameworks/js/napi/particleAbility/native_module.cpp |  |
| 6 | `screenLockFileManager` | @ohos.ability.screenLockFileManager.d.ts | js-apis-screenLockFileManager.md; js-apis-screenLockFileManager-sys.md | （纯类型声明/跨子系统未定位） | public + system 两份 |
| 7 | `Ability` | @ohos.app.ability.Ability.d.ts | js-apis-app-ability-ability.md | ability/ability_runtime/frameworks/js/napi/ability/ability_module.cpp |  |
| 8 | `AbilityConstant` | @ohos.app.ability.AbilityConstant.d.ts | js-apis-app-ability-abilityConstant.md; js-apis-app-ability-abilityConstant-sys.md | ability/ability_runtime/frameworks/js/napi/ability_constant/ability_constant_module.cpp | public + system 两份 |
| 9 | `AbilityLifecycleCallback` | @ohos.app.ability.AbilityLifecycleCallback.d.ts | js-apis-app-ability-abilityLifecycleCallback.md | ability/ability_runtime/frameworks/js/napi/app/ability_lifecycle_callback/ability_lifecycle_callback_module.cpp |  |
| 10 | `abilityManager` | @ohos.app.ability.abilityManager.d.ts | js-apis-app-ability-abilityManager.md; js-apis-app-ability-abilityManager-sys.md | ability/ability_runtime/frameworks/js/napi/ability_manager/ability_manager_module.cpp | public + system 两份 |
| 11 | `AbilityStage` | @ohos.app.ability.AbilityStage.d.ts | js-apis-app-ability-abilityStage.md | ability/ability_runtime/frameworks/js/napi/app/ability_stage/ability_stage_module.cpp |  |
| 12 | `ActionExtensionAbility` | @ohos.app.ability.ActionExtensionAbility.d.ts | js-apis-app-ability-actionExtensionAbility.md | ability/ability_runtime/frameworks/js/napi/action_extension_ability/action_extension_ability_module.cpp |  |
| 13 | `application` | @ohos.app.ability.application.d.ts | js-apis-app-ability-application.md; js-apis-app-ability-application-sys.md | ability/ability_runtime/frameworks/js/napi/application/application_module.cpp | public + system 两份 |
| 14 | `ApplicationStateChangeCallback` | @ohos.app.ability.ApplicationStateChangeCallback.d.ts | js-apis-app-ability-applicationStateChangeCallback.md | ability/ability_runtime/frameworks/js/napi/app/application_state_change_callback/application_state_change_callback_module.cpp |  |
| 15 | `appManager` | @ohos.app.ability.appManager.d.ts | js-apis-app-ability-appManager.md; js-apis-app-ability-appManager-sys.md | ability/ability_runtime/frameworks/js/napi/app/js_app_manager/app_manager_module.cpp | public + system 两份 |
| 16 | `appRecovery` | @ohos.app.ability.appRecovery.d.ts | js-apis-app-ability-appRecovery.md | ability/ability_runtime/frameworks/js/napi/app/recovery/app_recovery_module.cpp |  |
| 17 | `AppServiceExtensionAbility` | @ohos.app.ability.AppServiceExtensionAbility.d.ts | js-apis-app-ability-appServiceExtensionAbility.md | ability/ability_runtime/frameworks/js/napi/app_service_extension_ability/app_service_extension_ability_module.cpp |  |
| 18 | `AtomicServiceOptions` | @ohos.app.ability.AtomicServiceOptions.d.ts | js-apis-app-ability-atomicServiceOptions.md | （纯类型声明/跨子系统未定位） |  |
| 19 | `AutoFillExtensionAbility` | @ohos.app.ability.AutoFillExtensionAbility.d.ts | js-apis-app-ability-autoFillExtensionAbility-sys.md | ability/ability_runtime/frameworks/js/napi/auto_fill_extension_ability/auto_fill_extension_ability_module.cpp | 仅system api(sys) |
| 20 | `autoFillManager` | @ohos.app.ability.autoFillManager.d.ts | js-apis-app-ability-autoFillManager.md; js-apis-app-ability-autoFillManager-sys.md | ability/ability_runtime/frameworks/js/napi/auto_fill_manager/auto_fill_manager_module.cpp | public + system 两份 |
| 21 | `autoStartupManager` | @ohos.app.ability.autoStartupManager.d.ts | js-apis-app-ability-autoStartupManager.md; js-apis-app-ability-autoStartupManager-sys.md | ability/ability_runtime/frameworks/js/napi/ability_auto_startup_manager/ability_auto_startup_manager_module.cpp | public + system 两份 |
| 22 | `ChildProcess` | @ohos.app.ability.ChildProcess.d.ts | js-apis-app-ability-childProcess.md | ability/ability_runtime/frameworks/js/napi/js_child_process_manager/js_child_process_manager.cpp |  |
| 23 | `ChildProcessArgs` | @ohos.app.ability.ChildProcessArgs.d.ts | js-apis-app-ability-childProcessArgs.md | ability/ability_runtime/frameworks/native/ability/native/child_process_manager/js_child_process.cpp |  |
| 24 | `childProcessManager` | @ohos.app.ability.childProcessManager.d.ts | js-apis-app-ability-childProcessManager.md | ability/ability_runtime/frameworks/js/napi/js_child_process_manager/native_module.cpp |  |
| 25 | `ChildProcessOptions` | @ohos.app.ability.ChildProcessOptions.d.ts | js-apis-app-ability-childProcessOptions.md | ability/ability_runtime/frameworks/native/ability/native/child_process_manager/js_child_process.cpp |  |
| 26 | `common` | @ohos.app.ability.common.d.ts | js-apis-app-ability-common.md; js-apis-app-ability-common-sys.md | communication/bluetooth/frameworks/js/napi/src/common/module_common.cpp | public + system 两份 |
| 27 | `CompletionHandler` | @ohos.app.ability.CompletionHandler.d.ts | js-apis-app-ability-completionHandler.md | （纯类型声明/跨子系统未定位） |  |
| 28 | `CompletionHandlerForAbilityStartCallback` | @ohos.app.ability.CompletionHandlerForAbilityStartCallback.d.ts | js-apis-app-ability-CompletionHandlerForAbilityStartCallback.md | ability/ability_runtime/frameworks/js/napi/completion_handler_for_abilitystartcallback/completion_handler_for_abilitystartcallback_module.cpp |  |
| 29 | `CompletionHandlerForAtomicService` | @ohos.app.ability.CompletionHandlerForAtomicService.d.ts | js-apis-app-ability-CompletionHandlerForAtomicService.md | ability/ability_runtime/frameworks/js/napi/completion_handler_for_atomic_service/completion_handler_for_atomic_service_module.cpp |  |
| 30 | `Configuration` | @ohos.app.ability.Configuration.d.ts | js-apis-app-ability-configuration.md | arkui/ace_engine/interfaces/napi/kits/configuration/js_configuration.cpp |  |
| 31 | `ConfigurationConstant` | @ohos.app.ability.ConfigurationConstant.d.ts | js-apis-app-ability-configurationConstant.md | ability/ability_runtime/frameworks/js/napi/configuration_constant/configuration_constant_module.cpp |  |
| 32 | `contextConstant` | @ohos.app.ability.contextConstant.d.ts | js-apis-app-ability-contextConstant.md; js-apis-app-ability-contextConstant-sys.md | ability/ability_runtime/frameworks/js/napi/application_context_constant/application_context_constant_module.cpp | public + system 两份 |
| 33 | `continueManager` | @ohos.app.ability.continueManager.d.ts | js-apis-app-ability-continueManager.md | ability/dmsfwk/interfaces/kits/napi/continuation_state_manager/js_continuation_state_manager.cpp |  |
| 34 | `dataUriUtils` | @ohos.app.ability.dataUriUtils.d.ts | js-apis-app-ability-dataUriUtils.md | ability/ability_runtime/frameworks/js/napi/abilityDataUriUtils/ability_data_uri_utils_module.cpp |  |
| 35 | `dialogRequest` | @ohos.app.ability.dialogRequest.d.ts | js-apis-app-ability-dialogRequest.md | ability/ability_runtime/frameworks/js/napi/js_dialog_request/native_module.cpp |  |
| 36 | `dialogSession` | @ohos.app.ability.dialogSession.d.ts | js-apis-app-ability-dialogSession-sys.md | ability/ability_runtime/frameworks/js/napi/js_dialog_session/native_module.cpp | 仅system api(sys) |
| 37 | `EmbeddableUIAbility` | @ohos.app.ability.EmbeddableUIAbility.d.ts | js-apis-app-ability-embeddableUIAbility.md | ability/ability_runtime/frameworks/js/napi/embeddable_ui_ability/embeddable_ui_ability_module.cpp |  |
| 38 | `EmbeddedUIExtensionAbility` | @ohos.app.ability.EmbeddedUIExtensionAbility.d.ts | js-apis-app-ability-embeddedUIExtensionAbility.md | ability/ability_runtime/frameworks/js/napi/embedded_ui_extension_ability/embedded_ui_extension_ability_module.cpp |  |
| 39 | `EnvironmentCallback` | @ohos.app.ability.EnvironmentCallback.d.ts | js-apis-app-ability-environmentCallback.md | ability/ability_runtime/frameworks/js/napi/app/environment_callback/environment_callback_module.cpp |  |
| 40 | `errorManager` | @ohos.app.ability.errorManager.d.ts | js-apis-app-ability-errorManager.md | ability/ability_runtime/frameworks/js/napi/app/error_manager/error_manager_module.cpp |  |
| 41 | `ExtensionAbility` | @ohos.app.ability.ExtensionAbility.d.ts | js-apis-app-ability-extensionAbility.md | ability/ability_runtime/frameworks/js/napi/extension_ability/extension_ability_module.cpp |  |
| 42 | `hyperSnapManager` | @ohos.app.ability.hyperSnapManager.d.ts | js-apis-app-ability-hyperSnapManager.md | ability/ability_runtime/frameworks/js/napi/app/hyper_snap_manager/hyper_snap_manager_module.cpp |  |
| 43 | `insightIntent` | @ohos.app.ability.insightIntent.d.ts | js-apis-app-ability-insightIntent.md; js-apis-app-ability-insightIntent-sys.md | ability/ability_runtime/frameworks/js/napi/insight_intent/insight_intent/native_module.cpp | public + system 两份 |
| 44 | `InsightIntentContext` | @ohos.app.ability.InsightIntentContext.d.ts | js-apis-app-ability-insightIntentContext.md | ability/ability_runtime/frameworks/js/napi/insight_intent_context/insight_intent_context_module.cpp |  |
| 45 | `InsightIntentDecorator` | @ohos.app.ability.InsightIntentDecorator.d.ts | js-apis-app-ability-InsightIntentDecorator.md | ability/ability_runtime/frameworks/js/napi/insight_intent/insight_intent_decorator/native_module.cpp |  |
| 46 | `insightIntentDriver` | @ohos.app.ability.insightIntentDriver.d.ts | js-apis-app-ability-insightIntentDriver-sys.md | ability/ability_runtime/frameworks/js/napi/insight_intent/insight_intent_driver/native_module.cpp | 仅system api(sys) |
| 47 | `InsightIntentEntryExecutor` | @ohos.app.ability.InsightIntentEntryExecutor.d.ts | js-apis-app-ability-InsightIntentEntryExecutor.md | ability/ability_runtime/frameworks/js/napi/insight_intent/insight_intent_entry_executor/insight_intent_entry_module.cpp |  |
| 48 | `InsightIntentExecutor` | @ohos.app.ability.InsightIntentExecutor.d.ts | js-apis-app-ability-insightIntentExecutor.md | ability/ability_runtime/frameworks/js/napi/insight_intent/insight_intent_executor/insight_intent_executor_module.cpp |  |
| 49 | `insightIntentProvider` | @ohos.app.ability.insightIntentProvider.d.ts | js-apis-app-ability-insightIntentProvider.md | ability/ability_runtime/frameworks/js/napi/insight_intent/insight_intent_provider/js_insight_intent_provider_module.cpp |  |
| 50 | `InteropAbilityLifecycleCallback` | @ohos.app.ability.InteropAbilityLifecycleCallback.d.ts | （资料缺失） | （纯类型声明/跨子系统未定位） | 未找到对应docs md |
| 51 | `kioskManager` | @ohos.app.ability.kioskManager.d.ts | js-apis-app-ability-kioskManager.md; js-apis-app-ability-kioskManager-sys.md | ability/ability_runtime/frameworks/js/napi/kiosk_manager/kiosk_manager_module.cpp | public + system 两份 |
| 52 | `missionManager` | @ohos.app.ability.missionManager.d.ts | js-apis-app-ability-missionManager-sys.md | ability/ability_runtime/frameworks/js/napi/js_mission_manager/native_module.cpp | 仅system api(sys) |
| 53 | `OpenLinkOptions` | @ohos.app.ability.OpenLinkOptions.d.ts | js-apis-app-ability-openLinkOptions.md | （纯类型声明/跨子系统未定位） |  |
| 54 | `PhotoEditorExtensionAbility` | @ohos.app.ability.PhotoEditorExtensionAbility.d.ts | js-apis-app-ability-photoEditorExtensionAbility.md | ability/ability_runtime/frameworks/js/napi/photo_editor_extension_ability/photo_editor_extension_ability_module.cpp |  |
| 55 | `quickFixManager` | @ohos.app.ability.quickFixManager.d.ts | js-apis-app-ability-quickFixManager-sys.md | ability/ability_runtime/frameworks/js/napi/quick_fix/native_module.cpp | 仅system api(sys) |
| 56 | `scriptManager` | @ohos.app.ability.scriptManager.d.ts | js-apis-app-ability-scriptManager.md | ability/ability_runtime/cli_tool_framework/frameworks/js/napi/script_manager/src/script_manager_module.cpp |  |
| 57 | `sendableContextManager` | @ohos.app.ability.sendableContextManager.d.ets | js-apis-app-ability-sendableContextManager.md | ability/ability_runtime/frameworks/js/napi/app/sendable_context_manager/native_module.cpp |  |
| 58 | `ServiceExtensionAbility` | @ohos.app.ability.ServiceExtensionAbility.d.ts | js-apis-app-ability-serviceExtensionAbility-sys.md | ability/ability_runtime/frameworks/js/napi/service_extension_ability/service_extension_ability_module.cpp | 仅system api(sys) |
| 59 | `ShareExtensionAbility` | @ohos.app.ability.ShareExtensionAbility.d.ts | js-apis-app-ability-shareExtensionAbility.md | ability/ability_runtime/frameworks/js/napi/share_extension_ability/share_extension_ability_module.cpp |  |
| 60 | `skillDriver` | @ohos.app.ability.skillDriver.d.ts | （资料缺失） | （纯类型声明/跨子系统未定位） | 未找到对应docs md |
| 61 | `StartOptions` | @ohos.app.ability.StartOptions.d.ts | js-apis-app-ability-startOptions.md; js-apis-app-ability-startOptions-sys.md | （纯类型声明/跨子系统未定位） | public + system 两份 |
| 62 | `systemConfiguration` | @ohos.app.ability.systemConfiguration.d.ts | js-apis-app-ability-systemConfiguration.md | ability/ability_runtime/frameworks/native/appkit/ability_runtime/context/js_system_configuration_updated_callback.cpp |  |
| 63 | `UIAbility` | @ohos.app.ability.UIAbility.d.ts | js-apis-app-ability-uiAbility.md | ability/ability_runtime/frameworks/js/napi/ability/ability_module.cpp |  |
| 64 | `UIExtensionAbility` | @ohos.app.ability.UIExtensionAbility.d.ts | js-apis-app-ability-uiExtensionAbility.md | ability/ability_runtime/frameworks/js/napi/ui_extension_ability/ui_extension_ability_module.cpp |  |
| 65 | `UIExtensionContentSession` | @ohos.app.ability.UIExtensionContentSession.d.ts | js-apis-app-ability-uiExtensionContentSession.md; js-apis-app-ability-uiExtensionContentSession-sys.md | ability/ability_runtime/frameworks/native/ability/native/ui_extension_base/js_ui_extension_content_session.cpp | public + system 两份 |
| 66 | `UIServiceExtensionAbility` | @ohos.app.ability.UIServiceExtensionAbility.d.ts | js-apis-app-ability-uiServiceExtensionAbility-sys.md | ability/ability_runtime/frameworks/js/napi/ui_service_extension_ability/ui_service_extension_ability_module.cpp | 仅system api(sys) |
| 67 | `verticalPanelManager` | @ohos.app.ability.verticalPanelManager.d.ts | js-apis-app-ability-verticalpanelmanager-sys.md | ability/ability_runtime/frameworks/js/napi/ability_vertical_panel/native_module.cpp | 仅system api(sys) |
| 68 | `Want` | @ohos.app.ability.Want.d.ts | js-apis-app-ability-want.md | （纯类型声明/跨子系统未定位） |  |
| 69 | `wantAgent` | @ohos.app.ability.wantAgent.d.ts | js-apis-app-ability-wantAgent.md; js-apis-app-ability-wantAgent-sys.md | ability/ability_runtime/frameworks/js/napi/wantagent/ability_want_agent/want_agent_module.cpp | public + system 两份 |
| 70 | `wantConstant` | @ohos.app.ability.wantConstant.d.ts | js-apis-app-ability-wantConstant.md; js-apis-app-ability-wantConstant-sys.md | ability/ability_runtime/frameworks/js/napi/wantConstant/native_module.cpp | public + system 两份 |
| 71 | `agentConstant` | @ohos.app.agent.agentConstant.d.ts | js-apis-app-agent-agentConstant.md; js-apis-app-agent-agentConstant-sys.md | ability/ability_runtime/agent_runtime_framework/frameworks/js/napi/agent_constant/agent_constant_module.cpp | public + system 两份 |
| 72 | `AgentExtensionAbility` | @ohos.app.agent.AgentExtensionAbility.d.ts | js-apis-app-agent-agentExtensionAbility.md; js-apis-app-agent-agentExtensionAbility-sys.md | ability/ability_runtime/agent_runtime_framework/frameworks/js/napi/agent_extension_ability/js_agent_extension_ability_module.cpp | public + system 两份 |
| 73 | `agentManager` | @ohos.app.agent.agentManager.d.ts | js-apis-app-agent-agentManager-sys.md | ability/ability_runtime/agent_runtime_framework/frameworks/js/napi/agent_manager/src/agent_manager_module.cpp | 仅system api(sys) |
| 74 | `AgentUIExtensionAbility` | @ohos.app.agent.AgentUIExtensionAbility.d.ts | js-apis-agent-agentUIExtensionAbility.md | ability/ability_runtime/agent_runtime_framework/frameworks/js/napi/agent_ui_extension_ability/js_agent_ui_extension_module.cpp |  |
| 75 | `StartupConfig` | @ohos.app.appstartup.StartupConfig.d.ts | js-apis-app-appstartup-startupConfig.md | ability/ability_runtime/frameworks/native/appkit/app_startup/js_startup_config.cpp |  |
| 76 | `StartupConfigEntry` | @ohos.app.appstartup.StartupConfigEntry.d.ts | js-apis-app-appstartup-startupConfigEntry.md | ability/ability_runtime/frameworks/js/napi/app_startup/startup_config_entry/startup_config_entry_module.cpp |  |
| 77 | `StartupListener` | @ohos.app.appstartup.StartupListener.d.ts | js-apis-app-appstartup-startupListener.md | ability/ability_runtime/frameworks/js/napi/app_startup/startup_listener/startup_listener_module.cpp |  |
| 78 | `startupManager` | @ohos.app.appstartup.startupManager.d.ts | js-apis-app-appstartup-startupManager.md | ability/ability_runtime/frameworks/js/napi/app_startup/startup_manager/startup_manager_module.cpp |  |
| 79 | `StartupTask` | @ohos.app.appstartup.StartupTask.d.ets | js-apis-app-appstartup-startupTask.md | ability/ability_runtime/frameworks/js/napi/app_startup/startup_task/startup_task_module.cpp |  |
| 80 | `businessAbilityRouter` | @ohos.app.businessAbilityRouter.d.ts | js-apis-businessAbilityRouter-sys.md | ability/ability_runtime/service_router_framework/interfaces/kits/js/serviceroutermgr/native_module.cpp | 仅system api(sys) |
| 81 | `cliManager` | @ohos.app.cli.cliManager.d.ts | js-apis-app-cli-cliManager-sys.md | ability/ability_runtime/cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/cli_tool_manager_module.cpp | 仅system api(sys) |
| 82 | `uriPermissionManager` | @ohos.application.uriPermissionManager.d.ts | js-apis-uripermissionmanager-sys.md | ability/ability_runtime/frameworks/js/napi/uri_permission/native_module.cpp | 仅system api(sys) |
| 83 | `bundle` | @ohos.bundle.d.ts | js-apis-Bundle.md; js-apis-Bundle-sys.md | bundlemanager/bundle_framework/interfaces/kits/js/bundlemgr/native_module.cpp | public + system 两份 |
| 84 | `appControl` | @ohos.bundle.appControl.d.ts | js-apis-appControl-sys.md | bundlemanager/bundle_framework/interfaces/kits/js/app_control/native_module.cpp | 仅system api(sys); bundle前缀省略 |
| 85 | `appDomainVerify` | @ohos.bundle.appDomainVerify.d.ts | js-apis-appDomainVerify-sys.md | bundlemanager/app_domain_verify/interfaces/kits/js/jsi/src/native_module.cpp | 仅system api(sys); bundle前缀省略 |
| 86 | `bundleManager` | @ohos.bundle.bundleManager.d.ts | js-apis-bundleManager.md; js-apis-bundleManager-sys.md | bundlemanager/bundle_framework/interfaces/kits/js/bundle_manager/native_module.cpp | public + system 两份 |
| 87 | `bundleMonitor` | @ohos.bundle.bundleMonitor.d.ts | js-apis-bundleMonitor-sys.md | bundlemanager/bundle_framework/interfaces/kits/js/bundle_monitor/bundle_monitor.cpp | 仅system api(sys); bundle前缀省略 |
| 88 | `bundleResourceManager` | @ohos.bundle.bundleResourceManager.d.ts | js-apis-bundleResourceManager-sys.md | bundlemanager/bundle_framework/interfaces/kits/js/bundle_resource/native_module.cpp | 仅system api(sys); bundle前缀省略 |
| 89 | `defaultAppManager` | @ohos.bundle.defaultAppManager.d.ts | js-apis-defaultAppManager.md; js-apis-defaultAppManager-sys.md | bundlemanager/bundle_framework/interfaces/kits/js/default_app/native_module.cpp | public + system 两份 |
| 90 | `distributedBundleManager` | @ohos.bundle.distributedBundleManager.d.ts | js-apis-distributedBundleManager-sys.md | bundlemanager/distributed_bundle_framework/interfaces/kits/js/distributedBundle/native_module.cpp | 仅system api(sys); bundle前缀省略 |
| 91 | `freeInstall` | @ohos.bundle.freeInstall.d.ts | js-apis-freeInstall-sys.md | bundlemanager/bundle_framework/interfaces/kits/js/free_install/native_module.cpp | 仅system api(sys); bundle前缀省略 |
| 92 | `innerBundleManager` | @ohos.bundle.innerBundleManager.d.ts | js-apis-Bundle-InnerBundleManager-sys.md | bundlemanager/bundle_framework/interfaces/kits/js/launchermgr/js_launcher.cpp | 仅system api(sys) |
| 93 | `installer` | @ohos.bundle.installer.d.ts | js-apis-installer-sys.md | bundlemanager/bundle_framework/interfaces/kits/js/installer/native_module.cpp | 仅system api(sys); bundle前缀省略 |
| 94 | `launcherBundleManager` | @ohos.bundle.launcherBundleManager.d.ts | js-apis-launcherBundleManager.md; js-apis-launcherBundleManager-sys.md | bundlemanager/bundle_framework/interfaces/kits/js/launcher_bundle_manager/native_module.cpp | public + system 两份 |
| 95 | `overlay` | @ohos.bundle.overlay.d.ts | js-apis-overlay.md; js-apis-overlay-sys.md | bundlemanager/bundle_framework/interfaces/kits/js/overlay/native_module.cpp | public + system 两份 |
| 96 | `pluginBundleManager` | @ohos.bundle.pluginBundleManager.d.ts | js-apis-pluginBundleManager.md | bundlemanager/bundle_framework/interfaces/kits/js/plugin_manager/native_module.cpp |  |
| 97 | `shortcutManager` | @ohos.bundle.shortcutManager.d.ts | js-apis-shortcutManager.md; js-apis-shortcutManager-sys.md | bundlemanager/bundle_framework/interfaces/kits/js/shortcut_manager/native_module.cpp | public + system 两份 |
| 98 | `skillManager` | @ohos.bundle.skillManager.d.ts | js-apis-skillManager.md | bundlemanager/bundle_framework/interfaces/kits/js/skill_manager/native_module.cpp |  |
| 99 | `continuationManager` | @ohos.continuation.continuationManager.d.ts | js-apis-continuation-continuationManager.md | ability/dmsfwk/interfaces/kits/napi/continuation_manager/continuation_manager_module.cpp |  |
| 100 | `distributedBundle` | @ohos.distributedBundle.d.ts | js-apis-Bundle-distributedBundle-sys.md | bundlemanager/distributed_bundle_framework/interfaces/kits/js/distributebundlemgr/native_module.cpp | 仅system api(sys); 路径模糊匹配 |
| 101 | `distributedMissionManager` | @ohos.distributedMissionManager.d.ts | js-apis-distributedMissionManager-sys.md | ability/ability_runtime/frameworks/js/napi/mission_manager/distributed_mission_manager.cpp | 仅system api(sys) |
| 102 | `privacyManager` | @ohos.privacyManager.d.ts | js-apis-privacyManager-sys.md | （纯类型声明/跨子系统未定位） | 仅system api(sys) |
| 103 | `package` | @system.package.d.ts | js-apis-system-package.md | bundlemanager/bundle_framework/interfaces/kits/js/package/native_module.cpp |  |

#### 内部类型/上下文（78，docs 多以 `js-apis-inner-*` 单独成页，d.ts 位于 api 子目录；同 d.ts 若同时存在 public 与 `-sys` 两份 docs，合并到 docs 列以 `; ` 分隔）

| 序号 | 接口/模块名 | d.ts 文件 | docs md 文件 | 框架侧入口 cpp | 备注 |
| -- | -- | -- | -- | -- | -- |
| 1 | `abilityResult` | ability/abilityResult.d.ts | js-apis-inner-ability-abilityResult.md | ability/ability_runtime/frameworks/native/ability/native/ability_runtime/js_ability_context.cpp |  |
| 2 | `connectOptions` | ability/connectOptions.d.ts | js-apis-inner-ability-connectOptions.md | ability/dmsfwk/interfaces/kits/napi/ability_connection_manager/js_ability_connection_manager.cpp |  |
| 3 | `dataAbilityHelper` | ability/dataAbilityHelper.d.ts | js-apis-inner-ability-dataAbilityHelper.md | ability/ability_runtime/frameworks/js/napi/featureAbility/napi_data_ability_helper_utils.cpp |  |
| 4 | `dataAbilityOperation` | ability/dataAbilityOperation.d.ts | js-apis-inner-ability-dataAbilityOperation.md | ability/ability_runtime/frameworks/native/ability/native/data_ability_operation.cpp |  |
| 5 | `dataAbilityResult` | ability/dataAbilityResult.d.ts | js-apis-inner-ability-dataAbilityResult.md | ability/ability_runtime/frameworks/native/ability/native/data_ability_result.cpp |  |
| 6 | `startAbilityParameter` | ability/startAbilityParameter.d.ts | js-apis-inner-ability-startAbilityParameter.md | （未定位） |  |
| 7 | `want` | ability/want.d.ts | js-apis-inner-ability-want.md | ability/ability_runtime/frameworks/native/appkit/ability_runtime/ui_service_extension_context.cpp |  |
| 8 | `appVersionInfo` | app/appVersionInfo.d.ts | js-apis-inner-app-appVersionInfo.md | ability/ability_runtime/frameworks/js/napi/inner/napi_ability_common/napi_common_ability_execute_utils.cpp |  |
| 9 | `context` | app/context.d.ts | js-apis-inner-app-context.md | ability/ability_runtime/frameworks/native/appkit/ability_runtime/app/js_ability_stage.cpp |  |
| 10 | `AbilityFirstFrameStateData` | application/AbilityFirstFrameStateData.d.ts | js-apis-inner-application-abilityFirstFrameStateData-sys.md | ability/ability_runtime/frameworks/js/napi/app/js_app_manager/js_app_manager_utils.cpp | 仅system api(sys) |
| 11 | `AbilityFirstFrameStateObserver` | application/AbilityFirstFrameStateObserver.d.ts | js-apis-inner-application-abilityFirstFrameStateObserver-sys.md | ability/ability_runtime/frameworks/js/napi/app/js_app_manager/js_app_manager.cpp | 仅system api(sys) |
| 12 | `AbilityForegroundStateObserver` | application/AbilityForegroundStateObserver.d.ts | js-apis-inner-application-abilityForegroundStateObserver-sys.md | ability/ability_runtime/frameworks/js/napi/ability_manager/js_ability_manager.cpp | 仅system api(sys) |
| 13 | `AbilityMonitor` | application/AbilityMonitor.d.ts | js-apis-inner-application-abilityMonitor.md | ability/ability_runtime/frameworks/js/napi/app/ability_delegator/js_ability_delegator.cpp |  |
| 14 | `AbilityRunningInfo` | application/AbilityRunningInfo.d.ts | js-apis-inner-application-abilityRunningInfo.md | ability/ability_runtime/frameworks/js/napi/ability_manager/js_ability_manager.cpp |  |
| 15 | `AbilityStageContext` | application/AbilityStageContext.d.ts | js-apis-inner-application-abilityStageContext.md | ability/ability_runtime/frameworks/native/appkit/ability_runtime/app/js_ability_stage_context.cpp |  |
| 16 | `AbilityStageMonitor` | application/AbilityStageMonitor.d.ts | js-apis-inner-application-abilityStageMonitor.md | ability/ability_runtime/frameworks/js/napi/app/ability_delegator/js_ability_delegator.cpp |  |
| 17 | `AbilityStartCallback` | application/AbilityStartCallback.d.ts | js-apis-inner-application-abilityStartCallback.md | （未定位） |  |
| 18 | `AbilityStateData` | application/AbilityStateData.d.ts | js-apis-inner-application-abilityStateData.md | ability/ability_runtime/frameworks/js/napi/ability_manager/js_ability_manager.cpp |  |
| 19 | `AgentCard` | application/AgentCard.d.ts | js-apis-inner-application-AgentCard.md | ability/ability_runtime/frameworks/native/ability/native/ability_business_error/ability_business_error.cpp |  |
| 20 | `AgentExtensionConnectCallback` | application/AgentExtensionConnectCallback.d.ts | js-apis-inner-application-agentExtensionConnectCallback-sys.md | （未定位） | 仅system api(sys) |
| 21 | `AgentExtensionContext` | application/AgentExtensionContext.d.ts | js-apis-inner-application-agentExtensionContext.md | ability/ability_runtime/agent_runtime_framework/frameworks/js/napi/agent_extension_ability/src/js_agent_extension.cpp |  |
| 22 | `AgentHostProxy` | application/AgentHostProxy.d.ts | js-apis-inner-application-agentHostProxy.md | （未定位） |  |
| 23 | `AgentProxy` | application/AgentProxy.d.ts | js-apis-inner-application-agentProxy-sys.md | ability/ability_runtime/frameworks/native/ability/native/ability_business_error/ability_business_error.cpp | 仅system api(sys) |
| 24 | `AppForegroundStateObserver` | application/AppForegroundStateObserver.d.ts | js-apis-inner-application-appForegroundStateObserver-sys.md | ability/ability_runtime/frameworks/js/napi/app/js_app_manager/js_app_manager.cpp | 仅system api(sys) |
| 25 | `ApplicationContext` | application/ApplicationContext.d.ts | js-apis-inner-application-applicationContext.md; js-apis-inner-application-applicationContext-sys.md | ability/ability_runtime/frameworks/native/appkit/ability_runtime/context/js_application_context_utils.cpp | public + system 两份 |
| 26 | `ApplicationStateObserver` | application/ApplicationStateObserver.d.ts | js-apis-inner-application-applicationStateObserver.md | （未定位） |  |
| 27 | `AppServiceExtensionContext` | application/AppServiceExtensionContext.d.ts | js-apis-inner-application-appServiceExtensionContext.md | ability/ability_runtime/frameworks/native/ability/native/js_app_service_extension_context.cpp |  |
| 28 | `AppStateData` | application/AppStateData.d.ts | js-apis-inner-application-appStateData.md | ability/ability_runtime/frameworks/js/napi/app/app_manager/js_app_manager.cpp |  |
| 29 | `AutoFillExtensionContext` | application/AutoFillExtensionContext.d.ts | js-apis-inner-application-autoFillExtensionContext-sys.md | ability/ability_runtime/frameworks/native/ability/native/auto_fill_extension_ability/js_auto_fill_extension_context.cpp | 仅system api(sys) |
| 30 | `AutoFillPopupConfig` | application/AutoFillPopupConfig.d.ts | js-apis-inner-application-autoFillPopupConfig-sys.md | ability/ability_runtime/frameworks/native/ability/native/auto_fill_extension_ability/js_fill_request_callback.cpp | 仅system api(sys) |
| 31 | `AutoFillRect` | application/AutoFillRect.d.ts | js-apis-inner-application-autoFillRect.md | ability/ability_runtime/frameworks/js/napi/auto_fill_manager/js_auto_fill_manager_util.cpp |  |
| 32 | `AutoFillRequest` | application/AutoFillRequest.d.ts | js-apis-inner-application-autoFillRequest.md; js-apis-inner-application-autoFillRequest-sys.md | ability/ability_runtime/frameworks/js/napi/auto_fill_manager/js_auto_fill_manager.cpp | public + system 两份 |
| 33 | `AutoFillTriggerType` | application/AutoFillTriggerType.d.ts | js-apis-inner-application-autoFillTriggerType.md | ability/ability_runtime/frameworks/native/ability/native/auto_fill_extension_ability/js_auto_fill_extension_util.cpp |  |
| 34 | `AutoFillType` | application/AutoFillType.d.ts | js-apis-inner-application-autoFillType.md; js-apis-inner-application-autoFillType-sys.md | ability/ability_runtime/frameworks/native/ability/native/auto_fill_extension_ability/js_auto_fill_extension_util.cpp | public + system 两份 |
| 35 | `AutoStartupCallback` | application/AutoStartupCallback.d.ts | js-apis-inner-application-autoStartupCallback-sys.md | （未定位） | 仅system api(sys) |
| 36 | `AutoStartupInfo` | application/AutoStartupInfo.d.ts | js-apis-inner-application-autoStartupInfo-sys.md | ability/ability_runtime/frameworks/native/ability/native/auto_startup_callback/auto_startup_callback_proxy.cpp | 仅system api(sys) |
| 37 | `BaseContext` | application/BaseContext.d.ts | js-apis-inner-application-baseContext.md | ability/ability_runtime/frameworks/native/appkit/ability_runtime/context/js_context_utils.cpp |  |
| 38 | `CliToolEvent` | application/CliToolEvent.d.ts | js-apis-inner-application-cliToolEvent-sys.md | ability/ability_runtime/cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_session_event_callback.cpp | 仅system api(sys) |
| 39 | `Context` | application/Context.d.ts | js-apis-inner-application-context.md; js-apis-inner-application-context-sys.md | ability/ability_runtime/frameworks/native/appkit/ability_runtime/context/js_context_utils.cpp | public + system 两份 |
| 40 | `ContinuableInfo` | application/ContinuableInfo.d.ts | js-apis-inner-application-continuableInfo-sys.md | （未定位） | 仅system api(sys) |
| 41 | `ContinueCallback` | application/ContinueCallback.d.ts | js-apis-inner-application-continueCallback-sys.md | ability/ability_runtime/frameworks/js/napi/mission_manager/distributed_mission_manager.cpp | 仅system api(sys) |
| 42 | `ContinueDeviceInfo` | application/ContinueDeviceInfo.d.ts | js-apis-inner-application-continueDeviceInfo-sys.md | ability/ability_runtime/frameworks/js/napi/mission_manager/distributed_mission_manager.cpp | 仅system api(sys) |
| 43 | `ContinueMissionInfo` | application/ContinueMissionInfo.d.ts | js-apis-inner-application-continueMissionInfo-sys.md | ability/ability_runtime/frameworks/native/ability/native/distributed_ability_runtime/distributed_client.cpp | 仅system api(sys) |
| 44 | `CustomData` | application/CustomData.d.ts | js-apis-inner-application-customData-sys.md | ability/ability_runtime/frameworks/native/ability/native/auto_fill_extension_ability/auto_fill_extension_context.cpp | 仅system api(sys) |
| 45 | `EmbeddableUIAbilityContext` | application/EmbeddableUIAbilityContext.d.ts | js-apis-inner-application-EmbeddableUIAbilityContext.md | ability/ability_runtime/frameworks/native/ability/native/ui_extension_ability/js_embeddable_ui_ability_context.cpp |  |
| 46 | `ErrorObserver` | application/ErrorObserver.d.ts | js-apis-inner-application-errorObserver.md | ability/ability_runtime/frameworks/js/napi/app/error_manager/js_error_manager.cpp |  |
| 47 | `EventHub` | application/EventHub.d.ts | js-apis-inner-application-eventHub.md | （未定位） |  |
| 48 | `ExtensionContext` | application/ExtensionContext.d.ts | js-apis-inner-application-extensionContext.md | ability/ability_runtime/frameworks/native/ability/ability_runtime/js_extension_context.cpp |  |
| 49 | `ExtensionRunningInfo` | application/ExtensionRunningInfo.d.ts | js-apis-inner-application-extensionRunningInfo-sys.md | ability/ability_runtime/frameworks/js/napi/ability_manager/js_ability_manager.cpp | 仅system api(sys) |
| 50 | `LoopObserver` | application/LoopObserver.d.ts | js-apis-inner-application-loopObserver.md | ability/ability_runtime/frameworks/js/napi/app/error_manager/js_error_manager.cpp |  |
| 51 | `MissionCallbacks` | application/MissionCallbacks.d.ts | js-apis-inner-application-missionCallbacks-sys.md | （未定位） | 仅system api(sys) |
| 52 | `MissionDeviceInfo` | application/MissionDeviceInfo.d.ts | js-apis-inner-application-missionDeviceInfo-sys.md | ability/ability_runtime/frameworks/js/napi/mission_manager/distributed_mission_manager.cpp | 仅system api(sys) |
| 53 | `MissionInfo` | application/MissionInfo.d.ts | js-apis-inner-application-missionInfo-sys.md | ability/ability_runtime/frameworks/native/ability/native/distributed_ability_runtime/distributed_client.cpp | 仅system api(sys) |
| 54 | `MissionListener` | application/MissionListener.d.ts | js-apis-inner-application-missionListener-sys.md | ability/ability_runtime/frameworks/js/napi/js_mission_manager/mission_manager.cpp | 仅system api(sys) |
| 55 | `MissionParameter` | application/MissionParameter.d.ts | js-apis-inner-application-missionParameter-sys.md | ability/ability_runtime/frameworks/js/napi/mission_manager/distributed_mission_manager.cpp | 仅system api(sys) |
| 56 | `MissionSnapshot` | application/MissionSnapshot.d.ts | js-apis-inner-application-missionSnapshot-sys.md | ability/ability_runtime/frameworks/native/ability/native/distributed_ability_runtime/distributed_client.cpp | 仅system api(sys) |
| 57 | `MultiAppMode` | application/MultiAppMode.d.ts | js-apis-inner-application-multiAppMode-sys.md | （未定位） | 仅system api(sys) |
| 58 | `PageNodeInfo` | application/PageNodeInfo.d.ts | js-apis-inner-application-pageNodeInfo.md; js-apis-inner-application-pageNodeInfo-sys.md | ability/ability_runtime/frameworks/native/ability/native/auto_fill_extension_ability/js_auto_fill_extension_util.cpp | public + system 两份 |
| 59 | `ProcessData` | application/ProcessData.d.ts | js-apis-inner-application-processData.md | ability/ability_runtime/frameworks/js/napi/app/app_manager/js_app_manager_utils.cpp |  |
| 60 | `ProcessInformation` | application/ProcessInformation.d.ts | js-apis-inner-application-processInformation.md | （未定位） |  |
| 61 | `ProcessRunningInfo` | application/ProcessRunningInfo.d.ts | js-apis-inner-application-processRunningInfo.md | （未定位） |  |
| 62 | `RunningAppClone` | application/RunningAppClone.d.ts | js-apis-inner-application-runningAppClone-sys.md | ability/ability_runtime/frameworks/js/napi/app/js_app_manager/js_app_manager_utils.cpp | 仅system api(sys) |
| 63 | `RunningMultiAppInfo` | application/RunningMultiAppInfo.d.ts | js-apis-inner-application-runningMultiAppInfo-sys.md | ability/ability_runtime/frameworks/js/napi/app/js_app_manager/js_app_manager.cpp | 仅system api(sys) |
| 64 | `RunningMultiInstanceInfo` | application/RunningMultiInstanceInfo.d.ts | js-apis-inner-application-runningMultiInstanceInfo-sys.md | ability/ability_runtime/frameworks/js/napi/app/js_app_manager/js_app_manager_utils.cpp | 仅system api(sys) |
| 65 | `SendableContext` | application/SendableContext.d.ets | js-apis-inner-application-sendableContext.md | ability/ability_runtime/frameworks/js/napi/app/sendable_context_manager/js_sendable_context_manager.cpp |  |
| 66 | `ServiceExtensionContext` | application/ServiceExtensionContext.d.ts | js-apis-inner-application-serviceExtensionContext-sys.md | ability/ability_runtime/frameworks/native/ability/native/js_service_extension_context.cpp | 仅system api(sys) |
| 67 | `ToolEventCallback` | application/ToolEventCallback.d.ts | js-apis-inner-application-toolEventCallback-sys.md | （未定位） | 仅system api(sys) |
| 68 | `ToolInfo` | application/ToolInfo.d.ts | js-apis-inner-application-ToolInfo-sys.md | ability/ability_runtime/cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp | 仅system api(sys) |
| 69 | `UIAbilityContext` | application/UIAbilityContext.d.ts | js-apis-inner-application-uiAbilityContext.md; js-apis-inner-application-uiAbilityContext-sys.md | ability/ability_runtime/frameworks/native/ability/native/ability_runtime/js_ability_context.cpp | public + system 两份 |
| 70 | `UIExtensionContext` | application/UIExtensionContext.d.ts | js-apis-inner-application-uiExtensionContext.md; js-apis-inner-application-uiExtensionContext-sys.md | ability/ability_runtime/frameworks/native/ability/native/ui_extension_base/js_ui_extension_context.cpp | public + system 两份 |
| 71 | `UIServiceExtensionConnectCallback` | application/UIServiceExtensionConnectCallback.d.ts | js-apis-inner-application-uiServiceExtensionconnectcallback.md | （未定位） |  |
| 72 | `UIServiceExtensionContext` | application/UIServiceExtensionContext.d.ts | js-apis-inner-application-uiserviceExtensionContext-sys.md | ability/ability_runtime/frameworks/native/ability/native/ui_service_extension_ability/js_ui_service_extension_context.cpp | 仅system api(sys) |
| 73 | `UIServiceHostProxy` | application/UIServiceHostProxy.d.ts | js-apis-inner-application-uiservicehostproxy-sys.md | ability/ability_runtime/frameworks/native/ability/native/ui_service_extension_ability/connection/ui_service_host_proxy.cpp | 仅system api(sys) |
| 74 | `UIServiceProxy` | application/UIServiceProxy.d.ts | js-apis-inner-application-uiserviceproxy.md | ability/ability_runtime/frameworks/native/ability/native/ui_service_extension_ability/connection/ui_service_proxy.cpp |  |
| 75 | `ViewData` | application/ViewData.d.ts | js-apis-inner-application-viewData.md; js-apis-inner-application-viewData-sys.md | ability/ability_runtime/frameworks/native/ability/native/auto_fill_extension_ability/js_auto_fill_extension_util.cpp | public + system 两份 |
| 76 | `processInfo` | app/processInfo.d.ts | js-apis-inner-app-processInfo.md | ability/ability_runtime/frameworks/native/appkit/ability_runtime/context/js_application_context_utils.cpp |  |
| 77 | `triggerInfo` | wantAgent/triggerInfo.d.ts | js-apis-inner-wantAgent-triggerInfo.md; js-apis-inner-wantAgent-triggerInfo-sys.md | ability/ability_runtime/frameworks/js/napi/wantagent/napi_want_agent.cpp | public + system 两份 |
| 78 | `wantAgentInfo` | wantAgent/wantAgentInfo.d.ts | js-apis-inner-wantAgent-wantAgentInfo.md; js-apis-inner-wantAgent-wantAgentInfo-sys.md | ability/ability_runtime/frameworks/js/napi/wantagent/napi_want_agent.cpp | public + system 两份 |

## 触发条件

满足以下任一即触发：
- 用户直接提到"接口审计"、"API审计"、"资料一致性扫描"、"实现bug扫描"、"深度扫描api"
- 用户提到"扫描 xxxKit 的 api"或类似说法
- 用户问"这些接口实现有没有 bug / 资料对不对"
- 用户提供的输入是接口列表 / Kit 名称 / 现有审计报告需要补充

如果用户只问"这个文件/函数有没有问题"，不触发本 skill（属于普通代码审查）。

## 输入识别

用户输入通常包含以下信息（缺失时主动询问或自动探测）：
1. **Kit 名**：如 abilityKit、arkuiKit、nativeKit（用于定位 docs/interface 目录）
2. **审计范围**：默认全量；若用户指定子集（如"只扫 capi"），按指定子集
3. **审计深度**：默认两轮全跑（框架层 + 服务侧）；除非用户明确说"只看资料一致性"
4. **输出路径**：默认写到 `<cwd>/<kit-name>_api_audit.md`

## 工作流程（必须按顺序执行）

### Phase 1 - 准备阶段

#### 1.1 定位 Kit 范围

> **具体路径见上方"路径速查表"**。下方为通用规则，ability_runtime 等常见 Kit 的确切目录已在速查表中列出，避免反复 Glob 探测。

- C API 接口定义：`code/interface/sdk_c/<KitNamePascal>/` 下所有 `*.h`
- JS API 接口定义：`code/interface/sdk-js/api/.d.ts` 中对应的 `@kit.<KitName>` 模块
- **资料文档（必须定位并实读，禁止仅凭 d.ts 推导资料描述）**：`docs/zh-cn/application-dev/reference/apis-<kit-name>/` 下 `capi-*.md`（C API）和 `js-apis-*.md`（JS API）。**资料文档文件名与 d.ts/.h 名称不一一对应**（如 `UIAbilityContext.d.ts` 对应 `js-apis-inner-application-uiAbilityContext.md`），**必须用 Glob 在 docs 目录按类名/方法名/namespace 搜索定位**，不能靠猜文件名。定位后必须 Read 实读，作为"资料描述"列与 docs × d.ts × 实现 三方语义一致性比对的唯一来源。
- NDK 版本声明：`code/interface/sdk_c/<KitNamePascal>/` 下所有 `lib*.ndk.json`（含 first_introduced）
- **框架实现（动态 JS）**：`frameworks/native/<module>/.../js_*.cpp`（NAPI 类实现）+ `frameworks/js/napi/<module>/`（NAPI 模块注册）+ `frameworks/c/<module>/`（C 包装层）。**这三处是 dynamic 接口的框架侧实现，必须扫描**。
- **框架实现（静态，跳过）**：`frameworks/ets/ani/`、`frameworks/ets/ets/`、`frameworks/cj/ffi/` 及 `ets_*.cpp`/`cj_*.cpp` 文件——对应 static 接口，不纳入扫描（见 1.2.1 与速查表判定规则）。
- **服务实现**：`code/foundation/<subsystem>/<module>/services/<mgr>/src/` 下所有 `*.cpp`（如 `services/abilitymgr/src/ability_manager_service.cpp`、`services/appmgr/src/app_mgr_service.cpp`）。**R2 必须全量扫描，不要只扫客户端代理**。
- 测试用例（public API）：`code/test/xts/acts/<subsystem>/<module>/` 下
  - C API 测试目录命名约定：`capi<module>` / `acts<module>test` / `acts<api>test`
  - JS API 测试目录命名约定：`<module>test` / `acts<module>test`
- 测试用例（system API）：通常缺失，需在报告中明确标注

若用户未指定 Kit，从打开的文件路径或上下文推测；推测失败时用 AskUserQuestion 询问。

#### 1.2 列出全量 API 清单
- 对每个 `.h` 文件提取所有对外函数声明、typedef、枚举
- 对每个 `.d.ts` 提取所有 export 的接口、方法、属性
- 与 NDK JSON 对照，得到每个 API 的起始版本、是否 system api

#### 1.2.1 按接口语言类型过滤（强制，必须执行）

JS API 在 `.d.ts` 中按目标语言分为两套独立接口，标注为 `static` / `dynamic` / `dynamic&static`：

- **static（静态语言接口）**：**直接跳过，不进入三轮扫描清单**。对应静态类型语言（ArkTS/TS）的接口，类型在编译期固化，本审计只覆盖动态语言侧。
- **dynamic（动态语言接口）**：**必须扫描**。对应动态语言（JS）的接口。
- **dynamic & static（同时提供静态与动态两套接口）**：**必须扫描**。审计范围聚焦其中 **dynamic** 部分（静态侧不在本审计覆盖范围内），并需额外关注"同一功能在两套接口下签名/语义是否一致"。

**执行要求**：
1. 在 1.2 列出全量 API 后，**必须**对每个 JS API 读取其语言类型标注（位于 `.d.ts` 的 `@language` / 接口上方注释 / Kit 级声明等位置，按仓库实际约定查找）。
2. 将标注为 **static only** 的 API 从扫描清单中剔除，并在最终报告"审计范围"章节注明被跳过的 static-only 接口数量与名单（便于复核）。
3. C API（`.h`）默认全部纳入扫描，不受本规则约束（C 接口不存在 static/dynamic 语言类型区分）。
4. 若 `.d.ts` 未标注语言类型，**默认按 dynamic 处理**（纳入扫描），并在报告中注明"语言类型未标注，按动态语言接口处理"。
5. **框架侧实现文件的二次过滤（与速查表"动态 vs 静态"判定一致）**：定位 dynamic JS API 的框架实现时，只读 `js_*.cpp`（NAPI 签名）与 `frameworks/js/napi/<module>/`；**不要**把同目录的 `ets_*.cpp`、`cj_*.cpp` 或 `frameworks/ets/ani/`、`frameworks/cj/ffi/` 当成 dynamic 实现来扫——那些是 static 侧（ArkTS/Cangjie）实现。同一接口类在 `frameworks/native/.../ability_runtime/` 下常并存 `js_`/`ets_`/`cj_` 三份实现文件，**只扫 `js_` 前缀那份**。

> 注：本规则只影响**是否扫描**，不影响扫描深度。一旦某接口进入清单，三轮（框架层 / 服务侧 / 测试用例）必须全部执行。

#### 1.3 创建任务清单
使用 TaskCreate 为每个子模块（按 `.h` 文件或子目录）创建一个扫描任务，便于跟踪进度。同时创建以下任务：
- "服务侧深度审计"任务（第二轮）
- "测试用例完备度审计"任务（第三轮）
- "汇总报告"任务（最终输出）

### Phase 2 - 第一轮：框架层三方一致性扫描

> **三方 = 资料文档（docs `js-apis-*.md`/`capi-*.md`）× 接口定义（`d.ts`/`.h`）× 框架实现（`js_*.cpp`/`frameworks/c/`）。** 本轮逐 API 核对这三方是否一致，并扫框架实现内部基础 bug。"资料"特指 docs 仓的 md 文件，**不是 d.ts 注释**——必须在 Phase 1 用 Glob 定位 docs md 并实读，禁止仅凭 d.ts 推导"资料描述"列（曾因此漏报三方不一致 + 版本号写错，见陷阱 #17）。**docs md 的 markdown 格式问题（`<sup>` 标签闭合、`<br/>` 等）不在审计范围内。**

#### 2.1 三方一致性核对（按维度，每个 API 必查）

**A. 资料 × 接口定义 × 实现 签名一致性**
- 方法名、参数名、参数类型、参数顺序、返回值类型：docs 章节签名 × d.ts/`.h` 声明 × 框架实现解析 三者必须完全一致
- 参数描述（含义、必填）：docs 参数表 × d.ts 可选参数 `?` 一致
- const 修饰、枚举/typedef 定义是否一致
- 不一致 = ❌ 三方不一致（最高价值发现，如文档说参数可选但 d.ts 必填、或类型不符）

**B. 资料 × 接口定义 × 实现 起始版本一致性（最高频出错点）**
- docs 章节标注的版本号（`<sup>X+</sup>` 或正文"起始版本：X"）× d.ts `@since X dynamic` / `.h` 的 `@since` × `lib*.ndk.json` 的 `first_introduced` **三者数值必须一致**（只比对版本数值，不关心 `<sup>` 标签格式本身）
- 抄版本时必须核对 d.ts `@since ... dynamic` 行，不要凭记忆（曾把 setColorMode 写成 13，实际 d.ts 与 docs 均为 18）

**C. 资料 × 接口定义 × 实现 错误码一致性**
- docs 错误码表 × d.ts `@throws` × 框架/服务实现实际返回 三者必须一致
- docs 错误码表是否覆盖 d.ts `@throws` 所有错误码；是否有"语义反转"（文档说返回 X 实现返回 Y，如应 401 实际 16000001）
- 设备行为差异声明的错误码（如 16000061）是否在错误码表中列出

**D. 框架实现 参数校验 与接口定义的一致性**
- 参数校验是否覆盖所有空指针 / 非法值 / 类型不符
- 错误码返回是否与 d.ts `@throws` / `.h` 声明一致（非法参数应返回 401，而非业务错误码）
- C API：销毁函数是否将指针置 null（双指针 vs 单指针）；字符串/缓冲区长度校验是否完整
- system api 标注是否一致

**E. 资料 接口描述与代码实现行为一致性（语义层，高价值）**

> 核对 docs 首段"功能描述"与实际代码（框架实现 + 服务侧）行为是否吻合——这是**比签名/版本/错误码一致性更深的语义层核对**，往往暴露真实可用性缺陷。逐项检查：

- **E1. 描述缺失 / 需补充说明**：资料描述过于简略，关键行为未说明，开发者照文档用会踩坑。
  - 例：`openLink` docs 只说"打开链接"，但实际触发原子服务免安装下载流程（`freeInstallManager_->StartFreeInstall`）；开发者以为是只读跳转，实际拉起网络/ERMS 下载。
  - 例：`startAbility` docs 未说明 `Want.PARAMS` 中某些 key（如 `sceneBundleName`、`enable reuse`）会被框架消费并影响启动行为。
- **E2. 隐含限制未声明**：代码中有 `CHECK_CALLER_IS_SYSTEM_APP` / `IsForegroundCheck` / `callerToken` 校验、特定 bundleName 白名单、最低 API level 门槛等，但 docs 完全未提及。
  - 例：方法实际仅系统应用可调，但 docs 无 `@systemapi` 标注，三方应用照用必然 16000001/201 权限错误。
  - 例：方法仅在前台调用有效，docs 未写，后台调静默失败。
- **E3. 副作用未声明**：调用会产生对外可见的状态变化或回调，但 docs 只描述主动行为。
  - 例：`IsEmbeddedOpenAllowed`（查询类语义）内部却调用 `StartFreeInstall` 拉起免安装——查询不该有副作用。
  - 例：`connectServiceExtensionAbility` 成功后会触发 `onConnect` 回调，docs 未说明回调时序与超时。
- **E4. 描述与实现不符 / 不准确**：docs 描述 A，代码实现 B。
  - 例：docs 说"返回结果通过 Promise resolve"，实际代码 reject（错误码语义反转）。
  - 例：docs 说参数 `options.timeout` 生效，实际代码完全忽略该字段。
  - 例：docs 说"仅支持 abilityType=A"，实际 switch case 还接受 B/C。
- **E5. 描述冗余 / 过时**：docs 描述了已废弃的字段、已删除的参数、不再生效的逻辑。
  - 例：参数在某版本后被框架忽略，docs 仍描述其作用。
  - 例：错误码已下线，docs 仍列在错误码表中。
- **E6. 描述模糊导致误用**：关键概念未定义清楚。
  - 例：`setMissionContinueState` docs 未说明"continue"具体指什么（多设备协同？流转？），开发者易误用。
  - 例：参数 `deviceId` 描述为"设备标识"，未说明空字符串、`undefined`、`local` 的语义差异。
- **E7. 行为时序未说明**：异步 API 的回调时序、并发调用的串行化保证未在 docs 体现。
  - 例：连续调用 `startAbility` 多次的执行顺序、是否会互相取消。
  - 例：回调中再次调用本 API 是否死锁/重入。

**判定与定级**：
- **E1/E2/E3（描述缺失/隐含限制/副作用）**：根据影响定级——会导致调用失败或预期外行为 = ❌；仅影响理解 = ⚠️。
- **E4（描述与实现不符）**：一律 ❌（最高优先级，开发者照文档必然踩坑）。
- **E5/E6/E7（冗余/模糊/时序）**：默认 ⚠️；若直接导致错误使用则 ❌。
- 每条发现必须给出：**docs md file:line**（描述所在）+ **代码 file:line**（实际行为所在）+ 一句话差异说明。

**F. 资料存在性与内容完整性（docs md 侧，不含 markdown 格式）**
- **存在性（首要）**：每个对外 API 在 docs md 是否有对应章节（`### <方法名>`）。d.ts 声明但 docs 缺章节 = ❌ 资料缺失（P0）
- **完整性**：参数表四列（名/类型/必填/描述）齐全、返回值表存在、示例代码存在
- **不扫描 markdown 格式规范**：`<sup>` 标签闭合、`<br/>` 标签、标题层级等 markdown 格式问题**不在审计范围内**，不记录、不报告。审计只关注 docs 描述内容与 d.ts/实现 的**语义一致性**

**输出要求**：
- **"资料描述"列必须引用 docs md 行号 + 摘录原文关键短语**（至少一句）。格式约定：`docs <md文件名>:<行号> "<原文片段>" + 一句话概括`。**禁止凭方法名概括**——如把 `connectServiceExtensionAbility` 写成"连接到 ServiceExtensionAbility，返回 connection id"这种描述不引用 docs md 任何具体内容，等于没读 docs（典型违规，曾发生在 serviceExtensionContext 扫描中）。
- **A/B/C/D/E/F 六项必须逐 API 全部扫过，不得抽样**：即使结论是"一致"也必须显式记录对照证据（如 `✅ 签名一致：docs md:NNNN 参数表 = d.ts:NN 参数声明 = cpp:NN 解析`），不能只标 ✅ 不给证据。
- **E 项（语义层）不得跳过**：E1-E7 必须逐项扫过。若某 API 的 docs 描述过于简略导致 E1-E7 全部不适用，需在"扫描结果"列显式注明 `E1-E7 不适用：docs 仅声明签名无功能描述`，不能默认跳过。把 E4（描述与实现不符）、E3（副作用未声明）误标为"实现 bug"而未关联 docs，等同于跳过 E 项。
- **docs 章节必须覆盖全部 API**：禁止只挑几个方法做 docs 章节（如只查版本号或错误码表），其余方法凭方法名推导。曾发生 36 个签名中只覆盖 6 个方法 docs 章节的情形。
- 优先级：**E 项（语义不一致）> A/B/C/D 项（签名/版本/错误码/校验不一致）> F 项（资料存在性/内容完整性）**。语义层不一致直接导致开发者误用 API，价值最高。
- 发现写入第一轮表格"扫描结果"列（⚠️ 资料问题 / ❌ 资料缺失或三方不一致 / ❌ 语义不一致），并在"关键缺陷汇总"单列"资料一致性"分组；E4 类语义不一致建议在"高危/中危"分组中复述一遍以引起重视。
- **执行完成后必须进入 Phase 2.4 强制自检**（见下），未通过自检禁止生成 CSV。

#### 2.2 框架实现内部 bug（基础）
- 内存分配/释放配对
- 整数溢出（strlen 赋值给 int32_t 等）
- 浅拷贝 vs 深拷贝（特别是 char*）
- 拼写错误（函数名、常量名、注释）

#### 2.3 拼写 / 文档错误
- 头文件注释中的拼写错误
- 文档 md 中的错误码描述语义反转
- 参数名/类型不一致

#### 2.4 docs × 代码比对覆盖率强制自检（必做，类似 R2 服务侧自检）

> 类似陷阱 #16 对第二轮（R2 服务侧）的强制约束，本节是对**第一轮 docs × 代码比对覆盖率**的硬性自检。曾发生 serviceExtensionContext 扫描时（2026-06-24），R1 第一章 36 个签名全标"✅ 一致"却完全未引用 docs md 行号，R1 第二章 docs 章节只覆盖 6 个方法、E1-E7 语义层核对全部跳过的假象——表面看做了 docs 比对（有 L1/L2 缺陷项），实际三方比对与语义层核对全部漏报。

**报告生成前必须完成以下 4 条自检，任一未通过必须补扫**：

1. **资料描述列行号引用率 = 100%**：R1 表格每个 API 的"资料描述"列必须含 `docs <md文件名>:<行号>` 形式引用。Grep 报告 MD 中 `docs .*\.md:[0-9]+` 出现次数，应 ≥ R1 API 总数。低于 100% 必须补扫对应 API 的 docs md 章节。曾发生 36 个签名 0 行号引用的违规。
2. **docs 章节覆盖率 ≥ 95%**：R1 表格 API 总数中，必须在对应 docs md 章节做过实读核对的占比 ≥ 95%（不要求每个都发现问题，但要求每个都读过）。曾发生只覆盖 6/25 个方法的情形。
3. **E1-E7 语义层核对显式记录**：报告"关键缺陷汇总·资料一致性"或"中危/高危"分组中，至少有一条 E 项发现（E1-E7 任一），或显式声明"全部 API 的 docs 描述仅声明签名无功能描述，E1-E7 不适用"。曾发生 E 项完全跳过、把 E4 描述与实现不符（如 fallback 跳过校验、失败被吞报告成功）误标为"实现 bug"未关联 docs 的情形。
4. **三方比对证据显式化**：R1 表格"扫描结果"列对标 ✅ 一致的 API，必须能从单元格内容中看出"哪三方、比对了什么维度、对照证据是什么"——不能只写"✅ 一致"四个字。允许在单元格内简写如 `✅ 签名(d.ts:NN=cpp:NN=docs md:NNN)/版本(9=9=9)/错误码(表完整)`。

**自检结果必须在报告"第一轮"章节末尾以一行声明**：

```
> **R1 docs 比对自检**：资料描述列行号引用 36/36（100%）；docs 章节覆盖 25/25（100%）；E1-E7 语义层核对完成（发现 E3 × 1、E4 × 2）；三方比对证据已显式化。
```

未达 100% 引用率或 95% 覆盖率时，**禁止生成 CSV**——CSV 是 MD 的派生，MD 不达标 CSV 必然不达标。

### Phase 3 - 第二轮：服务侧深度扫描

**这是本 skill 的核心价值。** 第一轮只覆盖框架层基础参数校验，必须深入到服务侧才能发现真正的 bug。

#### 3.1 追踪调用路径
对每个 C API：
1. 读 `frameworks/c/<module>/src/<file>.cpp` 找到 C 接口实现
2. 找到 C++ 包装类（如 AbilityManagerClient、AppMgr Client）
3. 追踪到服务侧 `services/<module>/src/<file>.cpp`
4. 注意 IPC 跨进程边界：客户端 -> Parcel 序列化 -> 服务端反序列化 -> 实际逻辑

对每个 **dynamic JS API**（NAPI 实现）：
1. 读 `frameworks/native/<module>/.../js_<class>.cpp`（如 `js_ability_context.cpp`）找到 `JsXxx::Method` 入口
2. 跟进 `OnXxx` 处理函数 → 调用 `AbilityManagerClient::GetInstance()->Xxx(...)`
3. 追踪到服务侧 `services/abilitymgr/src/ability_manager_service.cpp` 及其子管理器（`ability_connect_manager.cpp`、`mission_list_manager.cpp`、`free_install_manager.cpp` 等）
4. 注意 IPC 跨进程边界：NAPI → 客户端代理 → Parcel 序列化 → `ability_manager_service.cpp` stub 反序列化 → 实际逻辑

> **服务侧目录必须全量扫描，不能只扫客户端代理**（见速查表"服务侧实现"行）。典型完整路径：`services/abilitymgr/src/*.cpp`（所有文件，尤其 `ability_manager_service.cpp`、`*_stub.cpp`、`*_manager.cpp`）+ `services/appmgr/src/*.cpp`。曾发生只扫客户端代理而漏掉 `ability_manager_service.cpp`，导致 R2 服务侧 bug 全部漏报。

#### 3.2 服务侧审计要点（按优先级）

**A. 生命周期 / 线程安全（最高优先级）**
- lambda 捕获栈变量引用 / 裸指针，异步回调访问已析构对象
- RAII guard 析构是否无条件执行副作用（如发出回调）
- 回调 ID / handle 生成是否可能碰撞（纳秒时间戳、自增原子）
- 静态 bool 标志位无锁检查（signal 注册、初始化标志）
- 回调 map 的 operator[] 覆盖 vs insert_or_assign
- 锁顺序是否全局一致（A->B vs B->A 潜在死锁）
- 信号处理函数使用 `signal()` vs `sigaction`，`waitpid(-1)` 是否会收割任意子进程

**B. IPC 序列化 / 反序列化**
- ReadFromParcel / WriteToParcel 字段顺序匹配
- 枚举值直接 static_cast 不做范围校验（恶意 IPC 可注入非法值）
- size 字段未校验负数（int32_t size < 0）
- fd 通过 Parcel 传递时的所有权（是否 dup）
- TOCTOU（先检查后使用，期间被其他线程修改）

**C. 资源泄漏**
- 异常路径 fd 是否关闭（open 成功后续失败）
- 异常路径内存是否释放（new 后 return 错误）
- 异常路径锁是否释放
- 回调对象在超时/失败路径是否清理
- 嵌套容器反序列化中途失败时已分配的子容器是否释放
- Variant_Clear 是否覆盖所有类型分支（IPC proxy/stub 类型常被遗漏）

**D. 错误码语义**
- 文档声明的错误码触发条件是否真的触发
- 同一函数不同失败路径返回的错误码是否可区分
- 是否用通用错误码掩盖具体失败原因（如返回 INTERNAL 或 TIMEOUT 掩盖权限拒绝）
- 文档说"返回 X"但实现返回 Y
- 值类型参数文档说"为空返回 PARAM_INVALID"（值类型不可能为空）

**E. API 行为 / 副作用**
- 文档未声明的隐含限制（如"仅支持主进程调用"）
- 未初始化时静默成功 vs 文档声明返回 INTERNAL
- const 引用参数被修改（数据竞争）
- 权限校验在特例化路径被绕过

#### 3.3 验证发现的 bug
对每个服务侧发现的 bug，**必须**用 Read/Grep 工具读取原始代码二次确认：
1. 函数定义所在文件:行号
2. 触发条件（什么场景下会走到这个 bug）
3. 影响范围（崩溃 / 内存泄漏 / 安全绕过 / 错误码混乱）
4. 验证结论：✅ 真实 / ❌ 误报 / ⚠️ 部分正确

未经验证的 bug 不要写入最终报告。

### Phase 4 - 第三轮：测试用例完备度审计

#### 4.1 定位测试代码

- **public API 测试**：`code/test/xts/acts/<subsystem>/<module>/` 下
  - JS API：`.test.ets` / `.test.ts` 文件
  - C API：`entry/src/main/cpp/*.cpp`（C 包装层）+ `entry/src/ohosTest/ets/test/*.test.ets`（ets 测试入口）
- **system API 测试**：通常缺失（XTS 不强制覆盖 system api），需在报告中明确标注"system api 测试未提供"

定位方法：
1. 按模块名找测试目录（如 `capiabilityruntime` / `capichildprocess` / `actscapimodularobjecttest`）
2. 用 Grep 搜索每个 API 函数名在 `.cpp` / `.test.ets` 中的调用点
3. 记录每个调用点的行号，作为覆盖证据

#### 4.2 测试覆盖维度（每个 API 必查）

**A. 参数正常值覆盖**
- 典型用法是否被测试
- 返回值/输出参数是否被校验（不只是检查返回 0）
- 多次调用的累积效果是否被测试

**B. 参数奇异值覆盖**
- 每个指针参数单独传 nullptr（不要只测全 nullptr 组合）
- 每个数值参数的边界值：
  - `int32_t`：INT32_MIN / INT32_MAX / 0 / -1
  - `double`：NaN / Infinity / -0.0 / DBL_MAX / DBL_MIN
  - `size_t` / `uint32_t`：0 / SIZE_MAX / 极大值
  - 枚举：合法最小值 / 合法最大值 / 非法值（如 999、-1）
- 字符串：nullptr / 空字符串 / 超长字符串 / 特殊字符（中文、emoji、unicode、SQL 注入字符）
- buffer：bufferSize=0 / 极小值（恰好不足）/ 恰好够 / 极大值

**C. 生命周期与并发覆盖**
- 销毁后再访问（use-after-free）
- 重复创建/销毁（double-free）
- 销毁后指针是否被置 null
- 重复注册/解注册回调
- 并发调用（多线程/HVigor 并行测试）
- 回调中再调用 API（线程安全）
- 内存泄漏（循环创建不销毁）

**D. 错误路径覆盖**
- 失败路径上的资源是否释放
- 异步回调的超时/失败触发
- IPC 失败时的客户端行为
- 权限拒绝路径

#### 4.3 评估完备度评级

每个 API 综合打分：
- **A（≥80%）**：正常值、奇异值、边界值、错误路径均覆盖，关键生命周期场景已测
- **B（50-80%）**：正常值覆盖完整，奇异值部分覆盖，缺部分边界值
- **C（<50%）**：仅测 nullptr 或仅测正常值，边界值与生命周期未覆盖
- **D（几乎无覆盖）**：零测试或仅零星几个 nullptr 用例

#### 4.4 识别缺失场景

列出未覆盖场景，按补充优先级标记：
- **P0（必须补）**：零覆盖的核心功能路径；成功路径完全未测；关键失败路径未测；与文档矛盾的测试
- **P1（建议补）**：边界值、特殊字符、非法枚举值、关键并发场景
- **P2（可选）**：极少触发的边界、防御性测试

#### 4.5 关键检查清单（避免遗漏）

- ✅ 是否有"正常 context + 验证返回内容"的用例（不只是 nullptr 三件套）
- ✅ 销毁后访问是否被测试
- ✅ 异步 API 的成功回调是否被测试（不只是错误码）
- ✅ 测试是否与文档/头文件声明一致（如 nullptr 应返回什么）
- ✅ 是否存在测试用例调用了错误函数（如 LogPath_1600 实际调 contextGetLogPath_1500）
- ✅ system api 是否标注"测试未提供"

### Phase 5 - 并行加速

当 API 数量超过 30 个时，使用 Agent 工具并行扫描：
- 按子模块拆分（如 application_context 一组、native_child_process 一组、modular_object 一组）
- 每个 Agent 负责一组 API 的三轮扫描
- 主 Agent 负责汇总和二次验证
- Agent 提示词必须自包含（提供完整的文件路径、审计要点、报告格式）

### Phase 6 - 输出 Markdown 报告

#### 6.1 报告文件
默认写到 `<cwd>/<kit-name>_api_audit.md`。若文件已存在，追加新章节而不是覆盖。

#### 6.2 报告结构（严格按此结构生成）

```markdown
# {KitName} API 一致性与实现缺陷审计报告

- **审计范围**：（具体 .h 文件列表 / 模块）
- **审计维度**：资料文档 × 接口定义 × 框架实现 × 服务实现
- **起始版本来源**：（具体 ndk.json / d.ts 文件）
- **系统 API 判定**：（如何判定）
- **生成日期**：（YYYY/MM/DD）

> 列说明：扫描结果用三档符号标注——✅ 一致（资料 × 接口定义 × 实现 三方匹配）/ ⚠️ 轻微问题（文档/拼写/可读性，不影响功能）/ ❌ 具体 bug（含 file:line，影响功能或安全）。R1/R2/R3 全部使用符号前缀。

---

## 第一轮：框架层三方一致性扫描

### 一、<文件名>.h

| 文件 | 接口 | 类型 | 系统API | 起始版本 | 资料描述 | 扫描结果 | 修复方案 |
| -- | -- | -- | -- | -- | -- | -- | -- |
| xxx.h | OH_XXX_Yyy | CAPI | N | 13 | 获取...。 | ✅ 一致 | 无需修复 |
| xxx.h | OH_XXX_Zzz | CAPI | N | 15 | 设置...。 | ❌ **实现 bug**：xxx.cpp:NNN 触发条件 | 修复方案 |

---

## 第二轮：服务侧实现深度审计

### 一、<模块名> 服务侧

| 文件 | 接口 | 类型 | 系统API | 起始版本 | 资料描述 | 深度扫描结果 | 修复方案 |
| -- | -- | -- | -- | -- | -- | -- | -- |
| xxx.cpp | OH_XXX_Yyy | CAPI | N | 13 | 获取...。 | ✅ 一致 | 无需修复 |
| xxx.cpp | OH_XXX_Zzz | CAPI | N | 15 | 设置...。 | ❌ **服务侧 bug**：xxx.cpp:NNN 触发条件 | 修复方案 |

（深度扫描结果必须是服务侧 bug/不一致，不重复框架层发现；符号约定同 R1）

---

## 第三轮：测试用例完备度审计

### 一、<模块名> 测试覆盖

| 文件 | 接口（可分组） | 参数正常值覆盖 | 参数奇异值覆盖 | 缺失场景（优先级） | 完备度评级 | 推荐补充用例 |
| -- | -- | -- | -- | -- | -- | -- |
| xxx.h | OH_XXX_Yyy | ✅ 完整 / ⚠️ 部分 / ❌ 缺失 | ✅ 完整 / ⚠️ 部分 / ❌ 缺失 | （P0/P1/P2）具体缺失场景 | A/B/C/D | 一句话建议 |

---

## 关键缺陷汇总（按风险等级）

### 高危（功能性 bug / 内存安全 / 安全绕过）
| # | 位置 | 问题 | 影响 |
| -- | -- | -- | -- |

### 中危（一致性问题，影响 API 正确使用）
| # | 位置 | 问题 |

### 低危（文档/注释/拼写错误）
| # | 位置 | 问题 |

### 资料一致性（docs × d.ts × 实现语义不一致 / 资料缺失）
| # | 位置 | 问题 |
| -- | -- | -- |

---

## 测试覆盖关键缺失汇总（按补充优先级）

### 高优先级（P0，必须补充）
| # | 模块 | 问题 | 影响 |
| -- | -- | -- | -- |

### 中优先级（P1，建议补充）
| # | 模块 | 问题 |

### 低优先级（P2，可选）
| # | 模块 | 问题 |

---

## 修复与补充优先级建议
1. **立即修复**（H 系列）：内存安全/崩溃/安全绕过
2. **下版本修复**（M 系列）：API 行为/错误码不一致
3. **文档迭代修复**（L 系列）：拼写、参数名
4. **测试立即补充**（T-P0）：零覆盖或仅 nullptr 覆盖的模块
5. **测试持续补强**（T-P1/P2）：边界值、特殊字符、并发场景

---

*本报告由 AI 自动扫描生成，所有结论需结合具体调用场景二次确认。引用的文件:行号基于本次扫描时点的代码版本。*
```

#### 6.3 表格列说明（必须严格遵守）

**第一轮与第二轮表格**（8 列）：

| 列序 | 列名 | 内容要求 |
| -- | -- | -- |
| 1 | 文件 | sdk 所在的 .h / .d.ts 文件名（不含路径） |
| 2 | 接口 | 完整接口名（含函数名、枚举名、typedef 名） |
| 3 | 类型 | JSAPI 或 CAPI |
| 4 | 系统API | Y（system api）或 N（public） |
| 5 | 起始版本 | 数字（如 13、15、17、21）或 x.y.z 格式（如 26.0.0） |
| 6 | 资料描述 | **docs md 行号引用 + 原文片段摘录 + 一句话概括**。格式：`docs <md>:<行号> "<原文片段>" + 概括`。**禁止凭方法名概括**（如"启动 Ability，callback 异步回调"这种无 docs md 引用的描述属于违规，详见 Phase 2.1 输出要求与 Phase 2.4 自检） |
| 7 | 扫描结果 | ✅ 一致 / ⚠️ 轻微问题 / ❌ 具体 bug（**含 file:line**），三档含义见下方说明 |
| 8 | 修复方案 | 具体可执行的修复动作，不要泛泛而谈；✅ 一致时填"无需修复" |

**扫描结果三档符号约定**（R1/R2 通用，与 R3 风格对齐）：

- ✅ **一致**：资料 × 接口定义 × 实现（框架层或服务侧）三方完全匹配，无任何差异。修复方案列填"无需修复"。
- ⚠️ **轻微问题**：不影响功能的差异，包括文档拼写错误、注释歧义、错误码描述非标准、参数名可读性差、`[since X]` 版本区间标注缺失等。仍需给出 file:line。
- ❌ **具体 bug**：影响功能或安全的问题，包括 nullptr 解引用、内存泄漏、线程安全、IPC 边界防御缺失、参数校验缺失、行为与文档矛盾、错误码语义反转等。必须给出 file:line。

> 必须三档其一作为单元格开头；不允许只有纯文字描述而无符号前缀。

**扫描结果列必须自包含完整描述（最高优先级要求，适用于 MD 表格 + CSV）**：

- "扫描结果 / 深度扫描结果" 单元格内容**必须独立可读**——读者只看这一格就能理解问题位置、触发条件、影响、修复方向，不需要跳到其他章节或汇总表去查编号含义。
- **禁止**只写编号引用，如 `❌ 同上：H1+H2+M5`、`❌ S-H2 副作用`、`❌ 复用 on('applicationState') 路径，H1/H2/M5 同前`。这类写法要求读者去"关键缺陷汇总"章节二次查找，违背"一格可读"目标。
- 编号可作为**前缀索引**辅助检索，但必须**紧接着复述完整描述**，举例：
  - ✅ 合格：`❌ S-H2（副作用）：ability_manager_service.cpp:15846 IsEmbeddedOpenAllowed 内部调用 freeInstallManager_->StartFreeInstall(want, callerUserId, 0, callerToken) 触发原子服务免安装下载，与 .d.ts "查询是否允许"语义不符；调用方误以为只读查询却拉起网络/ERMS 流程`
  - ✅ 合格（多缺陷合并）：`❌ H1（无锁）js_app_state_observer.cpp:364-388 observer map data race；H2（filter 仅首次生效）js_app_manager.cpp:336-346；M5（析构泄漏）js_app_manager.cpp:71-80 未反注册 service observer`
  - ❌ 不合格：`❌ H1+H2+M5 同前` / `❌ 同 on('applicationState')：H1 无锁+H2 filter+M5 析构泄漏`
- 合并多个缺陷时，**每个**缺陷都必须独立给出 file:line + 一句话影响，不能只丢一串编号。
- CSV 的 `R1_扫描结果` / `R2_深度扫描结果` 列与 MD 表格的"扫描结果/深度扫描结果"列同样适用本要求。

**第三轮测试覆盖表格**（7 列）：

| 列序 | 列名 | 内容要求 |
| -- | -- | -- |
| 1 | 文件 | sdk 所在的 .h / .d.ts 文件名 |
| 2 | 接口（可分组） | 接口名；当一组接口缺失场景相同时可合并（如 "Get*Dir（10 个）"） |
| 3 | 参数正常值覆盖 | ✅ 完整 / ⚠️ 部分 / ❌ 缺失（简述已覆盖场景） |
| 4 | 参数奇异值覆盖 | ✅ 完整 / ⚠️ 部分 / ❌ 缺失（简述已覆盖奇异值） |
| 5 | 缺失场景（优先级） | 列出关键未覆盖场景，每个标 (P0/P1/P2) |
| 6 | 完备度评级 | A（≥80%）/ B（50-80%）/ C（<50%）/ D（几乎无覆盖） |
| 7 | 推荐补充用例 | 一句话给出最该补的 1-2 个用例 |

#### 6.4 关键缺陷汇总编号约定
- 第一轮高危：H1, H2, ...
- 第一轮中危：M1, M2, ...
- 第一轮低危：L1, L2, ...
- 第二轮服务侧：在编号前加 `S-` 前缀（S-H1、S-M1、S-L1）
- 第三轮测试缺失：用 `T-P0-N` / `T-P1-N` / `T-P2-N` 编号（按补充优先级）

> **编号定位**：编号仅用于"关键缺陷汇总"章节排序与读者检索定位，**不能代替描述**。任何表格（MD 表格或 CSV）的扫描结果列在引用编号时，必须复述完整 file:line + 问题描述 + 影响，使单元格自包含可读（详见 §6.3 自包含要求）。

### Phase 7 - 输出 CSV 宽表

#### 7.1 CSV 文件
默认写到 `<cwd>/<kit-name>_api_audit.csv`。UTF-8 编码，逗号分隔，**所有字段用双引号包裹**。

CSV 与 MD 数据等价但格式不同：
- MD：三轮分章节叙事，每轮独立表格，便于人类阅读
- CSV：一个 API 一行，三轮横向并排，便于数据筛选、pivot、批量 review

#### 7.2 CSV 列结构（严格 15 列宽表）

```
文件,接口,类型,系统API,起始版本,资料描述,R1_扫描结果,R1_修复方案,R2_深度扫描结果,R2_修复方案,R3_正常值覆盖,R3_奇异值覆盖,R3_缺失场景,R3_评级,R3_推荐补充用例
```

| 列序 | 列名 | 内容来源 | 说明 |
| -- | -- | -- | -- |
| 1 | 文件 | R1 第一列 | sdk 所在 .h / .d.ts 文件名 |
| 2 | 接口 | R1 第二列 | 完整接口名 |
| 3 | 类型 | R1 第三列 | CAPI / JSAPI |
| 4 | 系统API | R1 第四列 | Y / N |
| 5 | 起始版本 | R1 第五列 | 数字或 x.y.z |
| 6 | 资料描述 | R1 第六列 | 一句话语义 |
| 7 | R1_扫描结果 | R1 第七列 | 第一轮框架层结论（✅ 一致 / ⚠️ 轻微 / ❌ bug） |
| 8 | R1_修复方案 | R1 第八列 | 第一轮修复建议 |
| 9 | R2_深度扫描结果 | R2 第七列 | 第二轮服务侧结论（✅ 一致 / ⚠️ 轻微 / ❌ bug） |
| 10 | R2_修复方案 | R2 第八列 | 第二轮修复建议 |
| 11 | R3_正常值覆盖 | R3 第三列 | ✅/⚠️/❌ |
| 12 | R3_奇异值覆盖 | R3 第四列 | ✅/⚠️/❌ |
| 13 | R3_缺失场景 | R3 第五列 | 含 P0/P1/P2 标记 |
| 14 | R3_评级 | R3 第六列 | A/B/C/D |
| 15 | R3_推荐补充用例 | R3 第七列 | 一句话建议 |

#### 7.3 数据合并规则（关键）

**以第一轮为 master 列表**：CSV 行数 = 第一轮 API 总数，每个 API 一行。

对每个第一轮中的 API：
1. **R1 列（7-8）**：直接从第一轮表格拷贝"扫描结果"和"修复方案"，保留 ✅/⚠️/❌ 符号前缀
2. **R2 列（9-10）**：在第二轮中查找该 API：
   - **精确匹配**：接口名完全一致 → 直接拷贝
   - **分组匹配**：第二轮有分组行（如 `OH_AbilityRuntime_StartSelfUIAbility*（全部）` 或 `Get*Dir（10 个）`）包含该 API → 将分组行内容应用到该 API
   - **未提及**：R2 列填 `"✅ 一致"` 和 `"无需修复"`（与 R1 一致项格式保持一致，便于筛选）
3. **R3 列（11-15）**：在第三轮中查找该 API：
   - **精确匹配或分组匹配** → 拷贝 5 个字段
   - **未提及** → 5 个字段全部填 `"-"`

#### 7.4 字段清洗规则

- **去除 Markdown 标记**：移除字段内的 `**` 加粗、反引号 `` ` ``、`<br>` 等
- **双引号转义**：字段内的 `"` 转为 `""`（CSV 标准）
- **换行处理**：字段内的 `\n` 转为空格（避免破坏 CSV 行结构）
- **逗号保留**：字段内的逗号保留（因为字段已被双引号包裹）
- **file:line 证据保留**：所有 `xxx.cpp:NNN` 形式的证据完整保留
- **特殊接口名**：枚举/typedef/入口函数声明的括号后缀保留（如 `AbilityRuntime_AreaMode（枚举）`）
- **编号引用必须展开（最高优先级，与 §6.3 自包含要求一致）**：若 MD 表格的"扫描结果 / 深度扫描结果"列含编号引用（如 H1、S-H2、M5、T-P0-1），CSV 中**禁止**只搬编号或"同前/同上"等省略写法，必须把每个被引用缺陷的 file:line + 问题描述 + 影响**完整展开**进 CSV 单元格。读者只看 CSV 一格即应能理解全部问题，不需要打开 MD 二次查找。

#### 7.5 抽样验证（必做）

CSV 生成后**必须**抽样验证 5 个 API：
1. 选 1 个三轮都"一致"的 API（验证基础合并）
2. 选 1 个 R1 有 bug、R2 也有 bug 的 API（验证多轮 bug 串联）
3. 选 1 个 R3 评级为 D 的 API（验证测试覆盖合并）
4. 选 1 个 R2 分组行的 API（验证分组展开）
5. 选 1 个 R3 分组行的 API（验证分组展开）

报告抽样验证结果（每个 API 一行说明合并是否正确）。

#### 7.6 CSV 与 MD 一致性原则

- CSV 的数据必须与 MD 完全等价，只是呈现方式不同
- 若发现合并冲突（如 MD 中某 API 在 R1 是 bug 但 R2 是"一致"），保留各自原文，不要尝试"调和"
- MD 是权威源，CSV 是 MD 的派生视图；如果只能保留一个，保留 MD

## 常见陷阱（避免犯）

1. **不要只扫框架层**：第一轮的"已扫描 / 一致"结论不代表实现无 bug，必须深入服务侧
2. **不要把参数校验缺失当主bug**：服务侧真正的 bug 才是价值所在
3. **不要写无 file:line 的结论**：每个 bug 必须有具体定位
4. **不要跳过二次验证**：服务侧 bug 容易误报，必须 Read 原始代码确认
5. **不要混淆一轮/二轮/三轮**：每轮发现独立编号，不要跨轮重复
6. **不要遗漏 fd 生命周期**：跨 IPC 边界的 fd 必须 dup，销毁路径必须 close
7. **不要忽视 RAII guard 析构**：析构无条件回调常掩盖真正失败原因
8. **不要相信值类型为空的文档**：值类型（int、enum）不可能为空，文档说"为空返回 PARAM_INVALID"必有误
9. **测试审计不要只数 nullptr 用例数**：很多测试套看似有几十个用例，实际全是 nullptr 参数测试（如 context.h 37 个用例全是 nullptr 三件套），无一个正常值验证
10. **测试审计要对照文档/头文件**：测试期望的错误码与文档/头文件声明一致才算覆盖，矛盾的测试本身就是 P0 缺陷（如 DestroyChildProcessConfigs 测试期望 nullptr 返回 401，与头文件声明的 NCP_NO_ERROR 矛盾）
11. **不要把"测试调用了 API"等同于"覆盖了功能"**：必须验证测试是否校验了返回值、输出参数、副作用，而不只是调用一次
12. **system API 测试缺失要明确标注**：XTS 通常不提供 system api 测试，需在报告中显式说明"system api 测试未提供"，不要静默跳过
13. **不要扫描 static-only 的 JS API**：JS API 按目标语言分两套——static（静态语言接口，ArkTS/TS）与 dynamic（动态语言接口，JS）。标注为 **static only** 的接口直接跳过（见 1.2.1），本审计只覆盖 dynamic 侧；dynamic 与 dynamic&static 必须扫描，其中 dynamic&static 只审 dynamic 部分。若 `.d.ts` 未标注语言类型，默认按 dynamic 纳入。
    - **框架侧实现文件同样要按此过滤（曾扫错）**：dynamic JS API 的框架实现是 `frameworks/native/<module>/.../js_*.cpp`（NAPI 签名 `napi_env`/`napi_callback_info`）与 `frameworks/js/napi/<module>/`；同目录的 `ets_*.cpp`（ArkTS/ANI）、`cj_*.cpp`（Cangjie/FFI）以及 `frameworks/ets/ani/`、`frameworks/cj/ffi/` 是 **static 侧实现，一并跳过**。判定口诀见速查表"动态 vs 静态"行：`js_` 前缀扫，`ets_`/`cj_` 前缀跳。
14. **JS API 框架实现路径必须用 BUILD.gn 的 relative_install_dir 区分新/旧同名模块（最高优先级陷阱，曾导致整个 R2 重扫）**：
    - 同一 Kit 常有新旧两套 NAPI 模块并存，目录名仅差一个层级，**极易找错**。
    - 典型案例：`@ohos.app.ability.appManager.d.ts`（新，Stage 模型 since 9）vs `@ohos.application.appManager.d.ts`（旧 FA 模型，多已 deprecated）
      - 新接口实现：`frameworks/js/napi/app/js_app_manager/` → `BUILD.gn` 中 `relative_install_dir = "module/app/ability"`
      - 旧接口实现：`frameworks/js/napi/app/app_manager/` → `BUILD.gn` 中 `relative_install_dir = "module/application"`
      - 仅目录名 `js_app_manager` vs `app_manager` 的细微差异，肉眼难辨；但模块 ID（`@ohos.app.ability.X` vs `@ohos.application.X`）有本质区别
    - **正确判定流程**：
      1. 先看 `.d.ts` 的 namespace：`@ohos.app.ability.X` 是新接口（Stage 模型），`@ohos.application.X` 是旧接口（FA 模型，多已 deprecated）
      2. 在候选目录的 `BUILD.gn` 中找 `relative_install_dir` 字段
      3. `"module/app/ability"` 对应新接口；`"module/application"` 对应旧接口
      4. 若仍不确定，看 `app_manager_module.cpp` 中 `napi_module` 的 `nm_modname` 字段，或看模块注册时绑定的 `.d.ts` namespace 字符串
    - **强制要求**：写报告前必须用 Grep/Read 核对 `relative_install_dir` 字段，证明所选框架目录与 `.d.ts` 一一对应，否则整个第二轮扫描结论会作废（曾发生：扫了旧接口的 `js_app_manager.cpp:170` 报 nullptr bug，但实际新接口实现完全不同，bug 不存在）
    - **推广**：此陷阱不限于 appManager，所有 Stage→FA 演进中重命名的 Kit 都适用（如 `@ohos.app.ability.formInfo` vs `@ohos.application.formInfo`、`@ohos.app.ability.wantAgent` vs `@ohos.application.wantAgent` 等），均以 `relative_install_dir` 为准
15. **不要用编号引用代替完整描述（最高优先级，曾导致 CSV/MD 单元格难读）**：扫描结果列（无论 MD 表格还是 CSV）若只写 `❌ H1 无锁+H2 filter+M5 析构泄漏` 或 `❌ S-H2 副作用` 这种纯编号引用，读者必须跳到"关键缺陷汇总"章节才能理解问题——违背"一格自包含可读"原则（详见 §6.3、§7.4）。
    - **正确做法**：编号可作为前缀索引，但必须复述 file:line + 问题 + 影响。例如：
      - ❌ 不合格：`❌ S-H2 副作用：xxx`（需查汇总章节才知道 S-H2 是什么）
      - ✅ 合格：`❌ S-H2（副作用）：ability_manager_service.cpp:15846 IsEmbeddedOpenAllowed 内部调用 freeInstallManager_->StartFreeInstall 触发免安装下载，与查询语义不符`
    - 多缺陷合并时，**每个**缺陷都必须独立给出 file:line + 一句话影响，不能省略。
    - **强制自检**：生成 CSV 前必须自检——若任一 R1/R2 扫描结果单元格不含 file:line 或不含具体问题描述（仅含编号或"同前/同上/复用 X 路径"），必须重新展开。
16. **服务侧目录必须全量扫描，不能只扫客户端代理（曾导致 R2 全部漏报）**：dynamic JS API 的调用链是 `js_*.cpp` → `AbilityManagerClient`（客户端代理）→ IPC → `ability_manager_service.cpp`（服务端 stub + 实现）。**只扫到客户端代理不算完成 R2**——真正的 bug（线程安全、IPC 序列化、资源泄漏、错误码语义）绝大多数在服务端。
    - **必须覆盖**：`services/abilitymgr/src/` 下所有文件（`ability_manager_service.cpp`、各 `*_stub.cpp`、各子管理器 `*_manager.cpp`）+ `services/appmgr/src/`（`app_mgr_service.cpp` 等）。具体路径见速查表"服务侧实现"行。
    - **强制自检**：R2 报告生成前，Grep 确认报告引用的 file:line 中至少包含一处 `services/` 路径下的文件；若全部 file:line 都在 `frameworks/` 下，说明服务侧未扫到，必须补扫。
    - 典型案例：曾发生只扫 `frameworks/js/napi/app/js_app_manager/` 客户端侧，漏掉 `ability_manager_service.cpp`，导致 R2 服务侧 bug（如 `IsEmbeddedOpenAllowed` 副作用、observer map data race）全部漏报。
17. **必须实读 docs/*.md 资料文档，核对 资料×接口定义×实现 三方一致性，禁止仅凭 d.ts/.h 推导"资料描述"（最高优先级，曾导致三方不一致全部漏报 + 版本号写错）**：docs 资料是审计四要素之一（资料文档×接口定义×框架实现×服务实现），"资料"特指 docs 仓的 md，不是 d.ts 注释。极易被忽略——因为 d.ts 注释里也有描述，AI 倾向直接抄 d.ts 而不去 docs 仓定位 md。这会导致：
    - **三方不一致全部漏报**：docs × d.ts × 实现 的签名/版本/错误码/参数描述不一致是高价值发现，但仅看 d.ts 完全无法察觉 docs 侧偏差（如 docs 参数描述与 d.ts 不符、docs 错误码表缺项、docs 示例缺失）。
    - **版本号/描述写错**：抄 d.ts 注释时容易记错版本。曾把 moveAbilityToBackground 写成 11、setColorMode 写成 13，实际 d.ts 与 docs 均为 12/18。
    - **docs 文件名与 d.ts/.h 名称不一一对应**：不能靠猜文件名。如 `UIAbilityContext.d.ts` → `js-apis-inner-application-uiAbilityContext.md`（多 `inner-application-` 前缀、驼峰转小写带连字符）。**必须用 Glob 在 `docs/zh-cn/application-dev/reference/apis-<kit>/` 按类名/方法名搜索定位**。
    - **强制流程**（见 Phase 1.1 与 Phase 2.1）：①Phase 1 用 Glob 定位 docs md 并 Read 实读；②"资料描述"列内容必须来自 docs md 首段；③Phase 2.1 按维度核对三方一致性（签名/起始版本/错误码/参数校验）+ 资料存在性与内容完整性（参数表/示例是否齐全）；④发现写入"关键缺陷汇总·资料一致性"分组；⑤**docs md 的 markdown 格式问题（`<sup>` 闭合标签、`<br/>` 等）不纳入审计范围**，只关注语义内容一致性。
    - **强制全量 + E 项语义层不能跳过（曾导致 serviceExtensionContext 假象）**：实读 docs md 不等于"逐 API 全覆盖"——曾发生（2026-06-24）只读 docs 的 6 个方法章节（仅查版本标签和错误码表），剩 19 个方法凭方法名概括"资料描述"列，R1 第一章 36 个签名全标 ✅ 一致但 A/B/C/D/E 三方比对全部跳过的假象。**必须**：(a) 每个 API 都进入对应 docs md 章节实读；(b) E1-E7 语义层逐项扫过；(c) "资料描述"列引用 docs md 行号 + 原文片段；(d) 报告生成前进入 Phase 2.4 docs 比对强制自检。
18. **docs × 代码比对不能抽样几个方法，必须全量 + E1-E7 语义层核对（最高优先级，曾导致 serviceExtensionContext R1 假象）**：陷阱 #17 强调了"必须读 docs"，但执行时仍可能出现"读了 docs 却只查版本标签 + 错误码表 + 抽样几个方法"的形式化执行，导致 docs × 代码语义层比对全部漏报。
    - **典型案例（serviceExtensionContext 2026-06-24）**：R1 第一章 36 个签名全标 ✅ 一致，"资料描述"列写"启动 Ability，callback 异步回调"这种凭方法名概括（无 docs md 行号引用）；R1 第二章 docs 章节只覆盖 6 个方法；E1-E7 语义层完全跳过——实际 openLink 的 fallback 跳过 domainVerify（E4 描述与实现不符）、connectServiceExtensionAbilityWithAccount 失败被吞报告成功（E3 副作用未声明 + E4）等典型 docs × 代码语义不一致全部漏标。
    - **强制流程**（详见 Phase 2.4 强制自检）：
      1. 资料描述列行号引用率 = 100%（每个 API 都引用 docs md 行号 + 原文片段）
      2. docs 章节覆盖率 ≥ 95%（每个 API 都进入对应 docs md 章节实读）
      3. E1-E7 语义层核对显式记录（至少一条 E 项发现，或显式声明全部不适用）
      4. 三方比对证据显式化（✅ 一致必须附 d.ts/docs/cpp 行号对照证据）
    - **强制自检**：报告生成前必须按 Phase 2.4 四条自检逐项验证，未达标禁止生成 CSV。

## 特殊情况处理

- **用户只指定一个具体 API**：仅扫描该 API，但三轮全跑，MD + CSV 都生成
- **用户要求补充现有报告**：读现有报告（MD 和 CSV 都读），识别已扫描范围，只追加未覆盖部分，避免重复；CSV 需整体重新生成（不能追加，否则破坏宽表结构）
- **用户只要 CSV 不要 MD**：仍需先完成三轮扫描，CSV 是 MD 的派生，不能跳过 MD 直接生成 CSV
- **用户只要 MD 不要 CSV**：正常输出 MD，跳过 Phase 7
- **找不到服务实现目录**：在报告中标注"未找到服务实现，仅完成框架层扫描"，不要编造服务侧发现
- **找不到测试用例目录**：在报告中标注"未找到 XTS 测试套件，测试覆盖审计跳过"，不要编造覆盖结论
- **API 数量超过 100 个**：使用 Agent 工具按子模块并行扫描（三轮都并行），CSV 合并阶段由主 Agent 统一处理
- **跨 Kit 引用**：如 abilityKit 引用了 ability_base 的 Want，应一并审计被引用侧的实现和测试
- **system api 测试套件缺失**：在第三轮报告中明确标注"system api 测试未提供"，public api 仍正常审计
- **CSV 字段内含特殊字符**：严格按 7.4 字段清洗规则处理（双引号转义、换行转空格），否则会破坏 CSV 结构

## 工具使用建议

- **Glob**：查找 `*.h`、`*.md`、`*.ndk.json` 文件
- **Grep**：在源码中查找函数定义、调用点
- **Read**：读取具体文件内容（二次验证 bug 必用）
- **Write**：生成 MD 报告和 CSV 文件
- **Agent**：大规模并行扫描时使用，每个 Agent 负责一个子模块；CSV 合并阶段也可用 Agent 处理大量数据
- **TaskCreate / TaskUpdate**：跟踪每个子模块的扫描进度

## 参考报告

完整的三轮审计 + CSV 示例参见：
- **Markdown 报告**：`abilitykit_api_audit.md`（672 行，含 14 个子模块、三轮独立表格、关键缺陷汇总）
- **CSV 宽表**：`abilitykit_api_audit.csv`（204 行 × 15 列，每个 API 一行，三轮结果横向并排）

两个文件数据等价，CSV 是 MD 的派生视图。
