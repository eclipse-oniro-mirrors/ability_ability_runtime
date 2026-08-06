# 设计文档：嵌入式元服务隐私弹框改为模应用体验

## 1. 架构上下文

### 1.1 现有链路（借鉴项，保持不动）

```
EmbeddableUIAbility 页面 onPageShow
  ↓
AMS DisposedRuleInterceptor → DisposedObserver::OnPageShow
  ↓ (HasAbilityKey 命中)
DisposedObserver::ExecuteUIExtension              [disposed_observer.cpp:159]
  │
  ├── [分支1] 嵌入式 (SCREEN_MODE = EMBEDDED_*)  [行 173-182] 借鉴项
  │     want.SetParam(IS_EMBEDDABLE_SERVICE, true)
  │     abilityRecord->CreateModalUIExtension(want)            ← 现有 IPC：CREATE_MODAL_UI_EXTENSION
  │       └─ scheduler_->CreateModalUIExtension(want)          ← AbilitySchedulerProxy
  │           └─ [IPC] → AbilitySchedulerStub::CreateModalUIExtensionInner
  │               └─ UIExtension::CreateModalUIExtension       [ui_extension.cpp:413]
  │                   └─ UIExtensionContext::CreateModalUIExtensionWithApp  [ui_extension_context.cpp:984]
  │                       └─ uiContent->CreateModalUIExtension(want, callback, config)
  │                           └─ UIExtensionModalCallback::OnReceive  ← 退出 key: ohos.param.exitEmbeddableUIExtension
  │
  ├── [分支2] 非 PAGE 模系统   [行 184-194]
  │     ModalSystemUiExtension->CreateModalUIExtension(want)
  │
  └── [分支3] PAGE 模应用      [行 196-208]
        abilityRecord->CreateModalUIExtension(want)
```

### 1.2 本次设计后链路（独立新增）

```
EmbeddableUIAbility 页面 onPageShow
  ↓
AMS DisposedObserver::OnPageShow
  ↓ (HasAbilityKey 命中)
DisposedObserver::ExecuteUIExtension
  │
  ├── [新增分支0] 嵌入式 + 本次需求场景识别   ← 新增判断，最前置
  │     ↓
  │   ExecuteEmbeddablePrivacyUIExtension()   ← 新增私有方法
  │     │  want.SetParam(IS_EMBEDDABLE_SERVICE, true)
  │     │  want.SetParam(EMBEDDABLE_PRIVACY_MODAL_FLAG, true)  ← 新增 Want 参数，区分新老链路
  │     ↓
  │   abilityRecord->CreateEmbeddablePrivacyUIExtension(want)  ← 新增 AbilityRecord 方法
  │     └─ scheduler_->CreateEmbeddablePrivacyUIExtension(want) ← 新增 Proxy 方法
  │         └─ [IPC: CREATE_EMBEDDABLE_PRIVACY_UI_EXTENSION]   ← 新增 IPC code
  │             └─ AbilitySchedulerStub::CreateEmbeddablePrivacyUIExtensionInner ← 新增 Stub 方法
  │                 └─ UIExtension::CreateEmbeddablePrivacyUIExtension  ← 新增 C++ 虚方法
  │                     └─ UIExtensionContext::CreateEmbeddablePrivacyUIExtensionWithApp ← 新增 Context 方法
  │                         └─ uiContent->CreateModalUIExtension(want, callback, config)  ← 仅 ArkUI 底层复用
  │                             └─ EmbeddablePrivacyModalCallback::OnReceive  ← 新增 Callback 类
  │                                 ← 新退出 key: ohos.param.exitEmbeddablePrivacyUIExtension
  │
  ├── [分支1] 现有嵌入式        ← 完全不变
  ├── [分支2] 现有非 PAGE 模系统 ← 完全不变
  └── [分支3] 现有 PAGE 模应用   ← 完全不变
```

### 1.3 关键约束
- 现有三条分支代码**逐行不变**，现有测试**全部继续通过**
- 新链路仅复用 ArkUI 层 `Ace::UIContent::CreateModalUIExtension`（无法新增，是 ArkUI 底层入口）
- 新链路其他元素（IPC、C++ 方法、Callback、退出 key）**全部新建**，命名与现有明确区分

---

## 2. 设计决策 (ADR)

### ADR-1: 新链路接入点 — ExecuteUIExtension 新增前置分支

**决策**：在 `DisposedObserver::ExecuteUIExtension` 函数最前置新增独立分支，判断"嵌入式 + 本次需求场景"，命中则调用新私有方法 `ExecuteEmbeddablePrivacyUIExtension` 并 return；不命中则继续走现有三条分支（逐行不变）。

**新增 Want 标记 `ohos.want.param.embeddablePrivacyModal`（bool）**：用于在新链路内部进一步区分本次需求场景；同时保留 `IS_EMBEDDABLE_SERVICE=true` 以维持隐私弹框对调用方类型的识别语义。

**理由**：
- 入口仍保留在 `onPageShow → ExecuteUIExtension`（符合需求）
- 现有三条分支零修改（满足"完全独立新路径"约束）
- 新增分支是"代码新增"（非"代码修改"），保持兼容性
- 后续若需关闭新链路，仅需移除新增分支

**替代方案**：
- 新建独立 Interceptor — 需在 AbilityManagerService 注册并管理优先级，改动面更大
- 新建独立 Observer — 需在 DisposedRuleInterceptor 增加管理逻辑，违反"不修改现有代码"

### ADR-2: 新 IPC 接口 — IAbilityScheduler 新增独立 code 与方法

**决策**：在 `IAbilityScheduler`（`interfaces/inner_api/ability_manager/include/ability_scheduler_interface.h`）新增：

```cpp
// 新增虚方法
virtual int32_t CreateEmbeddablePrivacyUIExtension(const Want &want) = 0;

// 新增 IPC code（enum 末尾追加）
CREATE_EMBEDDABLE_PRIVACY_UI_EXTENSION,
```

**Proxy 侧新增** `AbilitySchedulerProxy::CreateEmbeddablePrivacyUIExtension`，借鉴 `CreateModalUIExtension`（`ability_scheduler_proxy.cpp:1171`）的序列化方式（WriteInterfaceToken + WriteParcelable<Want> + SendTransactCmd）。

**Stub 侧新增** `AbilitySchedulerStub::CreateEmbeddablePrivacyUIExtensionInner`，在 `OnRemoteRequest` switch 中新增 case 分支，借鉴 `CreateModalUIExtensionInner`（`ability_scheduler_stub.cpp:741`）。

**理由**：
- 与现有 `CREATE_MODAL_UI_EXTENSION` 完全隔离，避免触发任何现有应用进程的解析路径
- IPC code 是有限的协议资源，但本次需求"完全独立"约束优先级高于 IPC code 节省
- 序列化格式与现有保持一致，便于实现与维护

**替代方案**：复用 `CREATE_MODAL_UI_EXTENSION` 并通过 Want 标记区分 — 违反"完全独立"约束，被否决。

### ADR-3: 新 C++ 进程内接口 — UIExtension 新增虚方法

**决策**：在 `UIExtension` 类（`interfaces/kits/native/ability/native/ui_extension_ability/ui_extension.h:95` 旁）新增：

```cpp
virtual int CreateEmbeddablePrivacyUIExtension(const AAFwk::Want &want);
```

实现位于 `frameworks/native/ability/native/ui_extension_ability/ui_extension.cpp`（在现有 `CreateModalUIExtension` 旁，借鉴其结构 `ui_extension.cpp:413-447`），调用新 Context 方法 `CreateEmbeddablePrivacyUIExtensionWithApp`。

**理由**：
- EmbeddableUIAbility 的 C++ 本体即 `UIExtension`，新方法应在此新增
- 虚方法允许未来其他 UIExtension 派生类按需重写
- 借鉴现有方法结构，但调用路径完全独立（不同的 Context 方法、不同的 Callback）

**替代方案**：
- 在 `Extension` 基类新增虚方法 — 影响所有 Extension 派生类（ServiceExtension、DataShareExtension 等），改动面过大
- 在 `UIExtensionBase` 模板类新增 — 模板膨胀，且 EmbeddableUIAbility 实际类型是 `UIExtension` 不是 `UIExtensionBase` 直接实例化

### ADR-4: 新 Context 方法 — UIExtensionContext 新增 WithApp 方法

**决策**：在 `UIExtensionContext`（`interfaces/kits/native/ability/native/ui_extension_base/ui_extension_context.h:263` 旁）新增：

```cpp
ErrCode CreateEmbeddablePrivacyUIExtensionWithApp(const AAFwk::Want &want);
```

实现位于 `frameworks/native/ability/native/ui_extension_base/ui_extension_context.cpp`（在现有 `CreateModalUIExtensionWithApp` 旁，借鉴其结构 `ui_extension_context.cpp:984-1039`），但实例化新 Callback 类 `EmbeddablePrivacyModalCallback`。

**复用基础工具方法**（与业务逻辑无关，可安全复用）：
- `UIExtensionContext::GetUIContent()` — 获取 UIContent 指针
- `UIExtensionContext::IsUIExtensionExist(want)` — 去重检查
- `UIExtensionContext::EraseUIExtension(sessionId)` — map 清理
- `uiExtensionMap_` + `uiExtensionMutex_` — session 跟踪

**不复用**：
- 现有 `CreateModalUIExtensionWithApp` 方法（必须新建）
- 现有 `UIExtensionModalCallback`（必须新建子类/新类）

**理由**：业务逻辑独立（不同的 Callback 实例、不同的退出 key），但底层 UIContent 操作、session 管理是工具方法，复用符合 YAGNI。

### ADR-5: 新 ModalCallback 类 — EmbeddablePrivacyModalCallback

**决策**：新增独立类 `EmbeddablePrivacyModalCallback`，位于 `frameworks/native/ability/native/ui_extension_base/`，文件命名 `embeddable_privacy_modal_callback.h` / `.cpp`。

**借鉴结构**：`UIExtensionModalCallback`（`ui_extension_modal_callback.h:36-91`），实现同样的四个回调：
- `OnRelease()` — 关闭模应用 + Erase session
- `OnError()` — 同 OnRelease
- `OnDestroy()` — 仅 Erase session
- `OnReceive(data)` — 处理新退出 key

**新退出 key**：`ohos.param.exitEmbeddablePrivacyUIExtension`（与现有 `ohos.param.exitEmbeddableUIExtension` 明确区分）

**OnReceive 行为**：当收到新退出 key 且值为 1 时，调用 `context->TerminateSelfWithAnimation(nullptr)` 终止嵌入式元服务。

**理由**：
- 完全独立类，避免对现有 `UIExtensionModalCallback` 的任何修改
- 类名明确表达用途，便于维护
- 新退出 key 与现有隔离，避免隐私弹框误识别

### ADR-6: 新增 Want 参数常量

**决策**：新增两个 Want 参数常量：

| 常量名 | 值 | 用途 |
|--------|---|------|
| `EMBEDDABLE_PRIVACY_MODAL_FLAG` | `ohos.want.param.embeddablePrivacyModal` | 新链路内部场景标记（仅 AMS ↔ 应用进程内部使用） |
| `EMBEDDABLE_PRIVACY_EXIT_KEY` | `ohos.param.exitEmbeddablePrivacyUIExtension` | 隐私弹框 → 嵌入式元服务的退出通知 key |

**保留使用**：现有 `IS_EMBEDDABLE_SERVICE = "ohos.param.isCallerEmbeddableUIExtension"` 在新链路中继续向隐私弹框传递（语义不变，供应用市场隐私弹框识别调用方类型）。

**理由**：新链路需要内部场景标记（用于 AMS 与应用进程的协议约定）和独立退出 key（用于回调协议），二者必须与现有常量隔离。

### ADR-7: 命名约定 — 与现有路径明确区分

**决策**：所有新增函数/类/常量使用 `EmbeddablePrivacy` 前缀，避免与现有 `Modal` 命名冲突。

| 现有（不动） | 新增 |
|-------------|------|
| `IAbilityScheduler::CreateModalUIExtension` | `IAbilityScheduler::CreateEmbeddablePrivacyUIExtension` |
| `CREATE_MODAL_UI_EXTENSION` (IPC code) | `CREATE_EMBEDDABLE_PRIVACY_UI_EXTENSION` (IPC code) |
| `AbilityRecord::CreateModalUIExtension` | `AbilityRecord::CreateEmbeddablePrivacyUIExtension` |
| `UIExtension::CreateModalUIExtension` | `UIExtension::CreateEmbeddablePrivacyUIExtension` |
| `UIExtensionContext::CreateModalUIExtensionWithApp` | `UIExtensionContext::CreateEmbeddablePrivacyUIExtensionWithApp` |
| `UIExtensionModalCallback` (类) | `EmbeddablePrivacyModalCallback` (类) |
| `ohos.param.exitEmbeddableUIExtension` | `ohos.param.exitEmbeddablePrivacyUIExtension` |

**理由**：通过命名强制隔离新旧链路，便于代码审查与日志定位。

---

## 3. 不涉及项确认

| 项 | 说明 |
|----|------|
| 修改 `disposed_observer.cpp` 现有三条分支代码 | 不涉及。仅在函数开头新增一条独立分支（代码新增，非修改） |
| 修改 `UIExtensionModalCallback` 现有类 | 不涉及。新增独立类 `EmbeddablePrivacyModalCallback` |
| 修改 `UIExtension::CreateModalUIExtension` 现有方法 | 不涉及。新增独立方法 `CreateEmbeddablePrivacyUIExtension` |
| 修改 `UIExtensionContext::CreateModalUIExtensionWithApp` | 不涉及。新增独立方法 |
| 修改 `JsEmbeddableUIAbilityContext` JS 绑定 | 不涉及。调用方是 AMS，不走 JS 层 |
| 修改应用市场侧隐私弹框 UIExtensionAbility | 不涉及。属其他仓 |
| 修改 ArkUI `Ace::UIContent::CreateModalUIExtension` | 不涉及。ArkUI 底层入口，必须复用 |
| 修改 `IS_EMBEDDABLE_SERVICE` 现有语义 | 不涉及。新链路沿用相同语义向隐私弹框传递 |
| 修改现有 `ohos.param.exitEmbeddableUIExtension` key | 不涉及。定义新 key 与之隔离 |
| 跨仓变更（window_manager、arkui_ace_engine、bundle_manager） | 不涉及 |

---

## 4. 修改物清单

### 4.1 AMS 服务端（services/abilitymgr）

| 文件 | 类型 | 修改内容 |
|------|------|----------|
| `src/disposed_observer.cpp` | 代码新增 | 在 `ExecuteUIExtension` 函数开头新增独立分支；新增私有方法 `ExecuteEmbeddablePrivacyUIExtension` |
| `include/disposed_observer.h` | 代码新增 | 新增私有方法声明 `ExecuteEmbeddablePrivacyUIExtension` |
| `src/ability_record.cpp` | 代码新增 | 新增方法 `CreateEmbeddablePrivacyUIExtension`，调用 `scheduler_->CreateEmbeddablePrivacyUIExtension(want)` |
| `include/ability_record.h` | 代码新增 | 新增方法声明 |
| `src/ability_scheduler_proxy.cpp` | 代码新增 | 新增 `CreateEmbeddablePrivacyUIExtension` Proxy 实现 |
| `include/ability_scheduler_proxy.h` | 代码新增 | 新增方法声明 |
| `src/ability_scheduler_stub.cpp` | 代码新增 | 新增 `OnRemoteRequest` case 分支 + `CreateEmbeddablePrivacyUIExtensionInner` 实现 |
| `include/ability_scheduler_stub.h` | 代码新增 | 新增方法声明 |

### 4.2 Inner API（interfaces/inner_api）

| 文件 | 类型 | 修改内容 |
|------|------|----------|
| `ability_manager/include/ability_scheduler_interface.h` | 代码新增 | 新增虚方法 + 新增 IPC code enum 项 |

### 4.3 框架层（frameworks/native + interfaces/kits/native）

| 文件 | 类型 | 修改内容 |
|------|------|----------|
| `interfaces/kits/native/ability/native/ui_extension_ability/ui_extension.h` | 代码新增 | UIExtension 类新增 `virtual int CreateEmbeddablePrivacyUIExtension(const Want &want);` |
| `frameworks/native/ability/native/ui_extension_ability/ui_extension.cpp` | 代码新增 | 实现 `UIExtension::CreateEmbeddablePrivacyUIExtension`，调用新 Context 方法 |
| `interfaces/kits/native/ability/native/ui_extension_base/ui_extension_context.h` | 代码新增 | UIExtensionContext 类新增 `ErrCode CreateEmbeddablePrivacyUIExtensionWithApp(const Want &want);` |
| `frameworks/native/ability/native/ui_extension_base/ui_extension_context.cpp` | 代码新增 | 实现新方法，借鉴 `CreateModalUIExtensionWithApp` 结构，实例化 `EmbeddablePrivacyModalCallback` |
| `interfaces/kits/native/ability/native/ui_extension_base/embeddable_privacy_modal_callback.h` | 新建文件 | 新增 `EmbeddablePrivacyModalCallback` 类声明 |
| `frameworks/native/ability/native/ui_extension_base/embeddable_privacy_modal_callback.cpp` | 新建文件 | 新增 `EmbeddablePrivacyModalCallback` 类实现（OnRelease/OnError/OnDestroy/OnReceive） |

### 4.4 Want 常量定义（共享）

| 文件 | 类型 | 修改内容 |
|------|------|----------|
| 现有 `want.h` / `ability_constant.h` / 新增 `embeddable_privacy_constants.h`（择一） | 代码新增 | 定义 `EMBEDDABLE_PRIVACY_MODAL_FLAG` 与 `EMBEDDABLE_PRIVACY_EXIT_KEY` 常量；建议集中在新公共头文件供 AMS 侧与应用进程侧共享 |

### 4.5 单元测试

| 文件 | 类型 | 修改内容 |
|------|------|----------|
| `test/unittest/disposed_observer_test/disposed_observer_test.cpp` | 代码新增 | 新增用例覆盖新分支触发与 Want 参数传递 |
| `test/unittest/ability_scheduler_stub_test/`（若存在）或同等测试目录 | 代码新增 | 新增用例覆盖新 IPC code 的 Stub 解析 |
| `test/unittest/embeddable_privacy_modal_callback_test/` | 新建目录 | 新增用例覆盖 `EmbeddablePrivacyModalCallback::OnReceive` 退出 key 处理 |
| `test/unittest/ui_extension_context_test/`（若存在）或同等 | 代码新增 | 新增用例覆盖 `CreateEmbeddablePrivacyUIExtensionWithApp` |

### 4.6 构建配置

| 文件 | 类型 | 修改内容 |
|------|------|----------|
| 相关 `BUILD.gn` | 代码新增 | 将新增 `.cpp` 文件加入源文件列表 |

---

## 5. 架构图

```
                    ┌──────────────────────────────────────────┐
                    │  EmbeddableUIAbility 页面 onPageShow     │
                    └──────────────────┬───────────────────────┘
                                       ↓
                    ┌──────────────────────────────────────────┐
                    │  DisposedObserver::OnPageShow             │
                    │  (保持现有逻辑不变)                        │
                    └──────────────────┬───────────────────────┘
                                       ↓
                    ┌──────────────────────────────────────────┐
                    │  DisposedObserver::ExecuteUIExtension     │
                    │                                          │
                    │  ┌──[新增分支0]──────────────────────┐  │
                    │  │ if (IsEmbeddableStart(screenMode) │  │
                    │  │     && 本次需求场景)              │  │
                    │  │   ExecuteEmbeddablePrivacyUIExt() │  │ ← 新增私有方法
                    │  │   return;                         │  │
                    │  └──────────────────────────────────┘  │
                    │                                          │
                    │  [分支1] 现有嵌入式       (不变)        │
                    │  [分支2] 现有非PAGE模系统 (不变)        │
                    │  [分支3] 现有PAGE模应用   (不变)        │
                    └──────────────────┬───────────────────────┘
                                       ↓ (新分支0)
          ┌──────────────────────────────────────────────────┐
          │  AMS 侧新链路                                    │
          │                                                  │
          │  abilityRecord->CreateEmbeddablePrivacyUIExtension│
          │    └─ scheduler_->CreateEmbeddablePrivacyUIExt   │ ← 新 Proxy 方法
          └────────────────────┬─────────────────────────────┘
                               ↓
                    ┌──────────────────────────┐
                    │ IPC: CREATE_EMBEDDABLE_  │ ← 新 IPC code
                    │      PRIVACY_UI_EXTENSION│
                    └──────────┬───────────────┘
                               ↓
          ┌──────────────────────────────────────────────────┐
          │  EmbeddableUIAbility 进程内（UIExtension 本体）   │
          │                                                  │
          │  AbilitySchedulerStub::                          │
          │    CreateEmbeddablePrivacyUIExtensionInner       │ ← 新 Stub 方法
          │    └─ UIExtension::CreateEmbeddablePrivacyUIExt  │ ← 新虚方法
          │        └─ UIExtensionContext::                   │
          │           CreateEmbeddablePrivacyUIExtensionApp  │ ← 新 Context 方法
          │            └─ uiContent->CreateModalUIExtension  │ ← 复用 ArkUI 底层
          │                └─ EmbeddablePrivacyModalCallback │ ← 新 Callback 类
          │                    .OnReceive → 退出 key 处理    │
          └──────────────────────────────────────────────────┘
```

---

## 6. 时序图（关键场景：嵌入式元服务拉起隐私弹框 + 退出）

```
EmbeddableUIAbility      AMS              EmbeddableUIAbility进程        隐私弹框
   (UIExtension)        (AMS)             (UIExtension本体)            (应用市场)
       │                  │                       │                       │
   onPageShow            │                       │                       │
       │─────────────────>│                       │                       │
       │                  │ ExecuteUIExtension    │                       │
       │                  │ ─[新增分支0]          │                       │
       │                  │ ExecuteEmbeddablePrivacyUIExtension          │
       │                  │──┐                    │                       │
       │                  │   │ want.SetParam    │                       │
       │                  │   │ (IS_EMBEDDABLE_  │                       │
       │                  │   │  SERVICE=true,   │                       │
       │                  │   │  EMBEDDABLE_     │                       │
       │                  │   │  PRIVACY_MODAL_  │                       │
       │                  │   │  FLAG=true)      │                       │
       │                  │<──┘                    │                       │
       │                  │ scheduler_->CreateEmbeddablePrivacyUIExt     │
       │                  │──────────────────────>│                       │
       │                  │   [IPC: CREATE_EMBEDDABLE_PRIVACY_UI_EXT]    │
       │                  │                       │ UIExtension::         │
       │                  │                       │  CreateEmbeddable...  │
       │                  │                       │ Context::Create...    │
       │                  │                       │ uiContent->           │
       │                  │                       │  CreateModalUIExtension
       │                  │                       │─────────────────────>│
       │                  │                       │   (模应用呈现)       │
       │                  │                       │                  显示弹框
       │                  │                       │                  (读取 IS_EMBEDDABLE_
       │                  │                       │                   SERVICE 区分调用方)
       │                  │                       │                       │
       │                  │                       │                  用户同意/拒绝
       │                  │                       │<─────────────────────│
       │                  │                       │ OnReceive(data)       │
       │                  │                       │  data[exitKey]=1      │
       │                  │                       │ TerminateSelfWithAnim │
       │                  │                       │← 嵌入式元服务退出     │
       │                  │                       │                       │
```

---

## 7. 关键风险与缓解

| 风险 | 缓解措施 |
|------|----------|
| IPC code 顺序冲突（不同版本兼容） | 新增 enum 项追加在末尾，不插入中间；版本协商不在本次范围 |
| `EmbeddablePrivacyModalCallback` 生命周期管理 | 借鉴 `UIExtensionModalCallback` 的 `weak_ptr<UIExtensionContext>` 设计，避免悬空 |
| 隐私弹框未识别新退出 key | 新 key 与 `IS_EMBEDDABLE_SERVICE` 配套使用；隐私弹框侧重逻辑属应用市场仓，本次仅保证 ability_runtime 侧正确传递与响应 |
| ArkUI `CreateModalUIExtension` 行为变更 | 本次仅调用，不修改 ArkUI；行为契约由 ArkUI 保证 |
