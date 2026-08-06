# 特性规格：嵌入式元服务隐私弹框改为模应用体验

## 1. 特性概述

当嵌入式元服务（EmbeddableUIAbility）页面 `onPageShow` 触发拉起隐私弹框（应用市场 UIExtensionAbility）时，ability_runtime 通过**完全独立的新链路**（新 IPC code、新 C++ 接口、新 ModalCallback、新退出 key）以"模应用"方式呈现隐私弹框，弹框在嵌入式元服务窗口内模态显示而非覆盖整个系统。

新链路与现有 `disposed_observer.cpp::ExecuteUIExtension` 三条分支（嵌入式、非 PAGE 模系统、PAGE 模应用）完全隔离，现有代码逐行不变。

## 2. 用户故事

### US-1: 嵌入式元服务的隐私弹框模应用体验
**作为** 嵌入式元服务用户
**我想要** 当嵌入式元服务拉起隐私弹框时，弹框在我的元服务窗口内以模态方式呈现（而不是覆盖整个系统）
**以便** 我能清晰感知弹框来自当前元服务，体验对标跳出式元服务的隐私弹框

### US-2: 隐私弹框可主动通知元服务退出
**作为** 应用市场隐私弹框
**我想要** 在用户处理完隐私同意后，能通过约定的退出 key 通知嵌入式元服务退出
**以便** 元服务能正确响应隐私决策结果

### US-3: 框架可维护性（新旧链路隔离）
**作为** ability_runtime 维护者
**我想要** 新增的嵌入式隐私弹框模应用链路与现有模应用/模系统链路在代码层面完全隔离（不同函数名、不同 IPC code、不同 Callback 类、不同退出 key）
**以便** 修改或回退新链路时不影响现有路径，便于定位与维护

## 3. API 影响

### 3.1 新增 Inner API（IPC 接口）

| 接口 | 类型 | 说明 |
|------|------|------|
| `IAbilityScheduler::CreateEmbeddablePrivacyUIExtension(const Want &want)` | 虚方法 | AMS → EmbeddableUIAbility 进程的新 IPC 调用 |
| `IAbilityScheduler::CREATE_EMBEDDABLE_PRIVACY_UI_EXTENSION` | IPC code | 新增 enum 项 |

### 3.2 新增 Native API（C++ 内部接口）

| 接口 | 类型 | 说明 |
|------|------|------|
| `AbilityRecord::CreateEmbeddablePrivacyUIExtension(const Want &want)` | 公有方法 | AMS 侧 AbilityRecord 入口 |
| `UIExtension::CreateEmbeddablePrivacyUIExtension(const Want &want)` | 虚方法 | EmbeddableUIAbility 进程内 UIExtension 入口 |
| `UIExtensionContext::CreateEmbeddablePrivacyUIExtensionWithApp(const Want &want)` | 公有方法 | UIExtensionContext 内部实现 |
| `EmbeddablePrivacyModalCallback` | 新增类 | 模应用回调（OnRelease/OnError/OnDestroy/OnReceive） |
| `DisposedObserver::ExecuteEmbeddablePrivacyUIExtension(...)` | 私有方法 | AMS 侧 DisposedObserver 新增分支函数 |

### 3.3 新增 Want 常量

| 常量 | 值 | 用途 |
|------|---|------|
| `IS_EMBEDDABLE_SERVICE`（沿用现有） | `ohos.param.isCallerEmbeddableUIExtension` | 隐私弹框识别调用方类型（值=true 表示嵌入式元服务） |
| `EMBEDDABLE_PRIVACY_MODAL_FLAG`（新） | `ohos.want.param.embeddablePrivacyModal` | 新链路内部场景标记（值=true 走新链路） |
| `EMBEDDABLE_PRIVACY_EXIT_KEY`（新） | `ohos.param.exitEmbeddablePrivacyUIExtension` | 隐私弹框 → 元服务的退出通知 key（值=1 表示请求退出） |

### 3.4 现有接口变更
**无变更**。所有现有接口（`IAbilityScheduler::CreateModalUIExtension`、`UIExtension::CreateModalUIExtension`、`UIExtensionContext::CreateModalUIExtensionWithApp`、`UIExtensionModalCallback`）签名与行为保持完全不变。

### 3.5 JS/ArkTS 公共 SDK 变更
**无变更**。本次是 AMS 内部接口路径，不暴露到 JS/ArkTS 层。

## 4. 行为规则

### 4.1 触发识别规则

| 条件 | 行为 |
|------|------|
| `SCREEN_MODE_KEY = EMBEDDED_FULL_SCREEN_MODE` 且本次需求场景 | 进入新链路分支0 |
| `SCREEN_MODE_KEY = EMBEDDED_HALF_SCREEN_MODE` 且本次需求场景 | 进入新链路分支0 |
| `SCREEN_MODE_KEY = EMBEDDED_*` 但本次需求场景不满足 | 进入现有分支1（借鉴项，行为不变） |
| `SCREEN_MODE_KEY = JUMP_SCREEN_MODE` 或缺失 | 进入现有分支2 或 3（行为不变） |

**"本次需求场景"识别**：由 `DisposedObserver` 内部约定（例如特定的 disposedRule 配置或调试点），具体识别逻辑在实现阶段确定；本规格仅约束识别后的行为。

### 4.2 Want 参数传递规则

新链路在调用 `abilityRecord->CreateEmbeddablePrivacyUIExtension(want)` 之前，必须设置：
```
want.SetParam(IS_EMBEDDABLE_SERVICE, true)
want.SetParam(EMBEDDABLE_PRIVACY_MODAL_FLAG, true)
```

`IS_EMBEDDABLE_SERVICE=true` 由隐私弹框读取以区分调用方类型；`EMBEDDABLE_PRIVACY_MODAL_FLAG=true` 仅供新链路内部使用。

### 4.3 IPC 调用规则

| 项 | 规格 |
|----|------|
| IPC code | `CREATE_EMBEDDABLE_PRIVACY_UI_EXTENSION`（独立 code，不复用 `CREATE_MODAL_UI_EXTENSION`） |
| 序列化 | `WriteInterfaceToken` + `WriteParcelable<Want>`（与现有 `CreateModalUIExtension` 格式一致） |
| 调用模式 | `MessageOption::TF_ASYNC`（与现有保持一致） |
| 失败处理 | 返回 `INNER_ERR`（IPC 失败）或 `ERR_INVALID_VALUE`（参数错误），有错误日志 |

### 4.4 进程内执行规则

`UIExtension::CreateEmbeddablePrivacyUIExtension` 被调用后：
1. 获取 `UIExtensionContext`（若为空返回 `ERR_INVALID_VALUE`）
2. 获取 `handler_`（若为空返回 `ERR_INVALID_VALUE`）
3. PostTask 到 handler，异步调用 `UIExtensionContext::CreateEmbeddablePrivacyUIExtensionWithApp(want)`

`UIExtensionContext::CreateEmbeddablePrivacyUIExtensionWithApp` 被调用后：
1. 获取 `uiContent`（若为空返回 `ERR_INVALID_VALUE`，记错误日志）
2. 调用 `IsUIExtensionExist(want)` 去重检查（若已存在且非全局 UIContent 模式，返回 `ERR_OK`）
3. 创建 `EmbeddablePrivacyModalCallback` 实例
4. 配置 `Ace::ModalUIExtensionConfig`（`prohibitedRemoveByRouter=true`，其他选项按需）
5. 调用 `uiContent->CreateModalUIExtension(want, callback, config)` 获取 sessionId
6. sessionId==0 返回 `ERR_INVALID_VALUE`，否则初始化 callback（SetSessionId/SetUIContent/SetUIExtensionContext）
7. 将 sessionId 存入 `uiExtensionMap_`

### 4.5 回调规则

`EmbeddablePrivacyModalCallback::OnReceive(data)` 行为：
- 检查 `data.HasParam(EMBEDDABLE_PRIVACY_EXIT_KEY)`
- 若存在且值为 1：调用 `context->TerminateSelfWithAnimation(nullptr)`，记 `TAG_LOGI` 日志
- 其他情况：仅记 `TAG_LOGD` 日志

`OnRelease/OnError`：调用 `EraseUIExtension(sessionId_)` + `uiContent_->CloseModalUIExtension(sessionId_)`（SUPPORT_SCREEN 宏下）

`OnDestroy`：仅调用 `EraseUIExtension(sessionId_)`

### 4.6 兼容性规则

- 现有 `disposed_observer.cpp::ExecuteUIExtension` 三条分支代码逐行不变
- 现有 `UIExtension::CreateModalUIExtension` / `UIExtensionContext::CreateModalUIExtensionWithApp` / `UIExtensionModalCallback` 实现不变
- 现有所有单元测试继续通过
- 现有 `IS_EMBEDDABLE_SERVICE` 与 `ohos.param.exitEmbeddableUIExtension` 语义不变

## 5. 验收标准 (AC)

### P0 — 核心路径

#### AC1: AMS 识别嵌入式元服务调用方
**WHEN** AMS 拦截到目标 Ability 的 `onPageShow` 事件，其 Want 中 `SCREEN_MODE_KEY` 为 `EMBEDDED_FULL_SCREEN_MODE` 或 `EMBEDDED_HALF_SCREEN_MODE`，且本次需求场景识别成立
**THEN** AMS 进入 `DisposedObserver::ExecuteEmbeddablePrivacyUIExtension` 新增私有方法，**不**进入 `ExecuteUIExtension` 现有三条分支

#### AC2: 新链路与现有路径完全隔离
**WHEN** 新链路被触发执行后
**THEN** `disposed_observer.cpp::ExecuteUIExtension` 现有三条分支代码逐行未改动；现有 `disposed_observer_test`、`ui_extension_modal_callback_test` 等测试用例全部继续通过

#### AC3: AMS 通过新 IPC 调用 EmbeddableUIAbility 进程内新接口
**WHEN** AMS 完成嵌入式识别并准备拉起隐私弹框
**THEN** AMS 调用 `abilityRecord->CreateEmbeddablePrivacyUIExtension(want)`，该方法通过 `scheduler_->CreateEmbeddablePrivacyUIExtension(want)` 发起 IPC，IPC code 为 `CREATE_EMBEDDABLE_PRIVACY_UI_EXTENSION`（不复用 `CREATE_MODAL_UI_EXTENSION`）；Want 中包含 `IS_EMBEDDABLE_SERVICE=true` 和 `EMBEDDABLE_PRIVACY_MODAL_FLAG=true`

#### AC4: EmbeddableUIAbility 进程内新接口创建模应用
**WHEN** EmbeddableUIAbility 进程内 `UIExtension::CreateEmbeddablePrivacyUIExtension` 收到 AMS 调用
**THEN** 方法异步调用 `UIExtensionContext::CreateEmbeddablePrivacyUIExtensionWithApp(want)`，后者在 EmbeddableUIAbility 的 UIContent 上调用 `CreateModalUIExtension`（**不**调用 `ModalSystemUiExtension->CreateModalUIExtension`），返回非零 sessionId 并存入 `uiExtensionMap_`

#### AC5: 隐私弹框以模应用体验呈现
**WHEN** EmbeddableUIAbility 通过新链路成功拉起隐私弹框
**THEN** 隐私弹框 UIExtensionAbility 在嵌入式元服务窗口内以模态方式显示，而非覆盖整个系统的模系统体验；隐私弹框可通过 Want 读取 `IS_EMBEDDABLE_SERVICE=true` 识别调用方为嵌入式元服务

#### AC6: onReceive 退出 key 触发嵌入式元服务退出
**WHEN** 隐私弹框 UIExtensionAbility 通过 `SendResult` 在 onReceive 回调中传入 `EMBEDDABLE_PRIVACY_EXIT_KEY = 1`
**THEN** `EmbeddablePrivacyModalCallback::OnReceive` 识别该 key，调用嵌入式元服务的 `TerminateSelfWithAnimation(nullptr)`，嵌入式元服务按预期退出；记 `TAG_LOGI(AAFwkTag::UI_EXT, ...)` 日志含 sessionId

#### AC7: IS_EMBEDDABLE_SERVICE 参数语义保持
**WHEN** 新链路向隐私弹框传递 Want
**THEN** Want 中 `ohos.param.isCallerEmbeddableUIExtension = true`；隐私弹框 UIExtensionAbility 读取后能区分调用方为嵌入式元服务（与现有约定语义一致）

### P1 — 边界路径

#### AC8: 弹出式元服务场景不受影响
**WHEN** 调用方为弹出式元服务（普通 UIAbility，`SCREEN_MODE_KEY` 为 `JUMP_SCREEN_MODE` 或缺失）
**THEN** 现有 `ExecuteUIExtension` 三条分支按原有逻辑路由，行为与本次变更前完全一致；新分支0 不被触发

#### AC9: 异常场景 — EmbeddableUIAbility 进程死亡
**WHEN** AMS 通过新 IPC 调用前/调用中，EmbeddableUIAbility 进程已死亡或 IPC 失败
**THEN** AMS 侧 `AbilitySchedulerProxy::CreateEmbeddablePrivacyUIExtension` 返回 `INNER_ERR`，记 `TAG_LOGE(AAFwkTag::ABILITYMGR, ...)` 日志；不发生 crash，不影响其他 Ability 调度

#### AC10: 异常场景 — UIContent 为空
**WHEN** `UIExtensionContext::CreateEmbeddablePrivacyUIExtensionWithApp` 被调用，但 `GetUIContent()` 返回空（窗口未就绪）
**THEN** 方法返回 `ERR_INVALID_VALUE`，记 `TAG_LOGE(AAFwkTag::UI_EXT, "null uiContent")`；不发生 crash

#### AC11: 异常场景 — handler 为空
**WHEN** `UIExtension::CreateEmbeddablePrivacyUIExtension` 被调用，但 `handler_` 为空
**THEN** 方法返回 `ERR_INVALID_VALUE`，记 `TAG_LOGE(AAFwkTag::UI_EXT, "null handler_")`；不发生 crash

#### AC12: 单元测试覆盖
**WHEN** 执行 `test/unittest/` 下本次新增/扩展的测试套件
**THEN** 至少覆盖：嵌入式识别、新 IPC code Stub 解析、新 Context 方法调用、`EmbeddablePrivacyModalCallback::OnReceive` 退出 key 响应、UIContent 为空、handler 为空、IPC 失败；所有用例通过

#### AC13: 日志可观测性
**WHEN** 新链路任意环节被触发
**THEN** 关键节点有 `TAG_LOGI`/`TAG_LOGD` 日志（tag：`AAFwkTag::ABILITYMGR` / `AAFwkTag::UI_EXT` / `AAFwkTag::UIABILITY`），含 `screenMode`、`sessionId`、`bundleName` 等关键字段

#### AC14: 命名隔离
**WHEN** 代码审查执行
**THEN** 所有新增函数/类/IPC code/常量使用 `EmbeddablePrivacy` 前缀（或等价独立命名），与现有 `CreateModalUIExtension` / `UIExtensionModalCallback` / `CREATE_MODAL_UI_EXTENSION` / `ohos.param.exitEmbeddableUIExtension` 命名明确区分

## 6. 测试场景

### 6.1 单元测试场景

| # | 场景描述 | 前置条件 | 预期结果 |
|---|----------|----------|----------|
| T1 | 嵌入式全屏 + 隐私场景 | SCREEN_MODE=EMBEDDED_FULL，本次需求场景识别成立 | 进入 `ExecuteEmbeddablePrivacyUIExtension`，不进入现有三条分支 |
| T2 | 嵌入式半屏 + 隐私场景 | SCREEN_MODE=EMBEDDED_HALF，本次需求场景识别成立 | 同 T1 |
| T3 | Want 参数传递校验 | 进入新链路 | want 含 IS_EMBEDDABLE_SERVICE=true + EMBEDDABLE_PRIVACY_MODAL_FLAG=true |
| T4 | 新 IPC code 解析 | Stub 收到 CREATE_EMBEDDABLE_PRIVACY_UI_EXTENSION | 正确分发到 `CreateEmbeddablePrivacyUIExtensionInner`，Want 反序列化成功 |
| T5 | UIExtension::CreateEmbeddablePrivacyUIExtension 正常路径 | context/handler 均非空 | PostTask 成功，最终调用 Context 方法 |
| T6 | UIExtension handler 为空 | handler_=nullptr | 返回 ERR_INVALID_VALUE，记错误日志 |
| T7 | Context UIContent 为空 | GetUIContent()=nullptr | 返回 ERR_INVALID_VALUE，记错误日志 |
| T8 | ModalCallback OnReceive 退出 key=1 | data 含 EMBEDDABLE_PRIVACY_EXIT_KEY=1 | 调用 TerminateSelfWithAnimation |
| T9 | ModalCallback OnReceive 退出 key=0 | data 含 EMBEDDABLE_PRIVACY_EXIT_KEY=0 | 不调用 TerminateSelfWithAnimation |
| T10 | ModalCallback OnReceive 无退出 key | data 不含退出 key | 不调用 TerminateSelfWithAnimation，仅 DEBUG 日志 |
| T11 | ModalCallback OnRelease | 触发 OnRelease | EraseUIExtension + CloseModalUIExtension 调用 |
| T12 | ModalCallback context 已销毁 | weak_ptr lock 失败 | 仅记错误日志，不 crash |
| T13 | IPC 失败 | SendTransactCmd 返回非 NO_ERROR | Proxy 返回 INNER_ERR，记错误日志 |
| T14 | 现有嵌入式分支不变 | SCREEN_MODE=EMBEDDED 但本次需求场景不成立 | 进入现有分支1，行为不变 |

### 6.2 兼容性回归测试

| # | 场景描述 | 预期结果 |
|---|----------|----------|
| R1 | 现有 `disposed_observer_test` 全部用例 | 全部通过 |
| R2 | 现有 `ui_extension_modal_callback_test` 全部用例 | 全部通过 |
| R3 | 现有 `ui_extension_context_test` 相关用例 | 全部通过 |
| R4 | 弹出式元服务拉起隐私弹框 | 走现有路径，行为不变 |

### 6.3 集成测试场景（手动）

| # | 场景描述 | 验证点 |
|---|----------|--------|
| I1 | 嵌入式元服务拉起隐私弹框 | 弹框在元服务窗口内模态显示（模应用体验），非覆盖整个系统 |
| I2 | 用户在隐私弹框中同意/拒绝 | 弹框通过新退出 key 通知元服务，元服务正确退出 |
| I3 | 应用市场隐私弹框读取 IS_EMBEDDABLE_SERVICE | 正确识别调用方为嵌入式元服务 |
