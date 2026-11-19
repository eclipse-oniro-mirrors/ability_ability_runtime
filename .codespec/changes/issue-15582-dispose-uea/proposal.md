# 嵌入式元服务隐私弹框改为模应用体验

## 原始需求

**需求来源**：云服务

**需求名称**：嵌入式元服务拉起隐私弹框从模系统体验改为模应用体验

**需求背景**：
嵌入式元服务（EmbeddableUIAbility）拉起隐私弹框当前为**模系统体验**（弹框覆盖整个系统，由 SystemUI 进程承载），元服务侧希望改为**模应用体验**（弹框在嵌入式元服务窗口内模态显示），体验对标跳出式拉起的元服务的隐私弹框。

**关键概念**：
- **嵌入式元服务**：对外是 `EmbeddableUIAbility`（JS 层继承自 UIAbility），C++ 本体是 `UIExtension`（继承自 `ExtensionBase<UIExtensionContext>`），按 `SCREEN_MODE_KEY` 区分 `EMBEDDED_FULL_SCREEN_MODE` / `EMBEDDED_HALF_SCREEN_MODE`
- **隐私弹框**：系统应用"应用市场"提供的 UIExtensionAbility
- **模系统 vs 模应用**：模系统走 `ModalSystemUiExtension->CreateModalUIExtension`；模应用走 `UIContent->CreateModalUIExtension`（在调用方窗口内模态显示）
- **触发点**：EmbeddableUIAbility 页面 `onPageShow` → AMS `DisposedRuleInterceptor` 拦截 → `DisposedObserver::ExecuteUIExtension` 路由

**输入**：嵌入式元服务拉起隐私弹框

**处理步骤**（澄清后版本）：
1. 拉起隐私弹框前判断拉起方是不是嵌入式元服务（依据 `SCREEN_MODE_KEY`）
2. 如果是嵌入式元服务，**走新增的独立路径**（不修改 `disposed_observer.cpp` 现有三条分支），由 AMS 通过新 IPC 调用嵌入式元服务进程内的新模应用接口
3. 嵌入式元服务进程内**新增模应用内部接口**（对标 `UIAbility::CreateModalUIExtension`，可借鉴 `UIExtensionContext::CreateModalUIExtensionWithApp` 的实现）
4. 新链路的 onReceive 回调中**定义请求退出 key**，用于隐私弹框（应用市场 UIExtensionAbility）告知嵌入式元服务退出

**输出**：在嵌入式元服务中拉起隐私弹框为模应用体验

---

## 澄清记录

| # | 模糊点 | 澄清结论 |
|---|--------|----------|
| 1 | 当前"模系统"拉起入口与代码路径 | 入口在 `services/abilitymgr/src/disposed_observer.cpp::ExecuteUIExtension`；在该函数现有嵌入式分支出现之前，所有隐私弹框统一走 `systemUIExtension->CreateModalUIExtension`。触发时机为 UIAbility 拉起到前台、`onPageShow` 触发拉起隐私弹框的 UIExtensionAbility |
| 2 | 交付边界（现有代码已部分实现的处理） | **选项 C：新增完全独立路径**。现有 `disposed_observer.cpp` 已有的嵌入式分支代码（行 173-182）和 `IS_EMBEDDABLE_SERVICE` 参数作为借鉴项保留不动，本次新增一条独立链路实现 |
| 3 | "嵌入式元服务进程内模应用内部接口"的调用方 | **选项 A：AMS（系统服务端）通过 IPC 调用**。EmbeddableUIAbility 应用代码不主动调用；AMS 在拦截到嵌入式元服务 `onPageShow` 后，通过新 IPC 调用 EmbeddableUIAbility 进程内的新 C++ 模应用方法（对标 `UIAbility::CreateModalUIExtension`，在 UIExtension 体系内新增） |
| 4 | "嵌入式元服务"对外概念与内部实现 | 对外是 `EmbeddableUIAbility`（JS 层 `extends UIAbility`），C++ 本体是 `UIExtension`（`ExtensionBase<UIExtensionContext>`），运行在嵌入式元服务独立进程内，由 `SCREEN_MODE_KEY` 区分嵌入式/弹出式 |
| 5 | 隐私弹框 UIExtensionAbility 归属 | 系统应用"应用市场"提供；`IS_EMBEDDABLE_SERVICE`（`ohos.param.isCallerEmbeddableUIExtension`）参数供隐私弹框读取，用于区分调用方是嵌入式元服务（UIExtension 本体）还是弹出式元服务（UIAbility） |
| 6 | 现有 `JsEmbeddableUIAbilityContext::OnRequestModalUIExtension` 在嵌入式模式下抛 Not support | **不在本次范围**。本次是 AMS 内部接口路径，不修改 JS 绑定层 |
| 7 | 现有退出 key `ohos.param.exitEmbeddableUIExtension` | 已定义于 `ui_extension_modal_callback.cpp:23`，新链路沿用相同语义或基于其在新的 callback 中实现等价逻辑（具体在 design 阶段决定） |
| 8 | 是否修改应用市场侧隐私弹框 UIExtensionAbility | **不在本次范围**，属于其他仓 |

---

## 需求基线

### 范围

- **包含**：
  - AMS 侧新增"识别嵌入式元服务 + 走新独立路径"的逻辑（**不修改 `disposed_observer.cpp` 现有三条分支**）
  - EmbeddableUIAbility 进程内新增 C++ 模应用内部接口（对标 `UIAbility::CreateModalUIExtension`，在 UIExtension 体系内新增，可借鉴 `UIExtensionContext::CreateModalUIExtensionWithApp`）
  - AMS ↔ EmbeddableUIAbility 进程间的新 IPC 接口（对标 UIAbility 既有 IPC 链路）
  - 新链路的 ModalCallback 中实现 onReceive 退出 key 处理，使隐私弹框可告知嵌入式元服务退出
  - `IS_EMBEDDABLE_SERVICE` 参数在新链路里继续向隐私弹框传递（保持语义兼容）
  - 单元测试覆盖新链路

- **排除**：
  - 修改 `disposed_observer.cpp` 现有代码（**完全独立新路径**）
  - 修改 `JsEmbeddableUIAbilityContext` 的 JS 绑定（不让应用代码主动调，调用方是 AMS）
  - 修改 `UIExtensionContext::CreateModalUIExtensionWithApp` 现有实现
  - 修改应用市场侧隐私弹框 UIExtensionAbility（属于其他仓）
  - 修改现有 `ohos.param.exitEmbeddableUIExtension` key 的语义（保持兼容）
  - 跨仓变更（window_manager、arkui_ace_engine 等）

### 涉及子系统/仓

| 仓 | 子系统/路径 | 说明 |
|----|------------|------|
| ability_runtime | `services/abilitymgr` | AMS 侧新独立路径的拦截与调度（在 `disposed_observer` 之外新增）、调用 AbilityScheduler 新 IPC |
| ability_runtime | `interfaces/inner_api` | 新 IPC 接口声明（AbilityScheduler 或同等 stub 扩展），对标 UIAbility 既有模应用 IPC |
| ability_runtime | `frameworks/native/ability/native` | UIExtension 体系（或对应 EmbeddableUIAbility 本体）新增 C++ 模应用内部接口；新增/复用 ModalCallback |
| ability_runtime | `interfaces/kits/native/ability/native` | 基类扩展声明（若新增虚方法） |

### 复杂度级别

**标准** — 单仓（ability_runtime）特性，涉及新 IPC 接口设计 + 新 C++ 内部接口 + 回调协议；无跨 SIG 协调，无安全/性能关键路径。

---

## 验收标准 (P0)

### AC1: AMS 识别嵌入式元服务调用方
**WHEN** AMS 拦截到目标 Ability 的 `onPageShow` 事件且其 Want 中 `SCREEN_MODE_KEY` 为 `EMBEDDED_FULL_SCREEN_MODE` 或 `EMBEDDED_HALF_SCREEN_MODE`
**THEN** AMS 识别为"嵌入式元服务调用方"，进入本次新增的独立处理路径（不落入 `disposed_observer.cpp` 现有三条分支）

### AC2: 新路径与现有路径完全隔离
**WHEN** 新独立路径被触发执行
**THEN** `disposed_observer.cpp::ExecuteUIExtension` 现有三条分支（嵌入式分支行 173-182、非 PAGE 模系统分支、PAGE 模应用分支）的代码与行为保持不变；现有所有测试用例继续通过

### AC3: AMS 通过新 IPC 调用 EmbeddableUIAbility 进程内新接口
**WHEN** AMS 完成嵌入式元服务识别并准备拉起隐私弹框
**THEN** AMS 通过本次新增的 IPC 接口（对标 UIAbility 既有模应用 IPC 链路）调用 EmbeddableUIAbility 进程内的新 C++ 模应用方法，传递的 Want 包含隐私弹框目标信息和 `IS_EMBEDDABLE_SERVICE=true`

### AC4: EmbeddableUIAbility 进程内新接口创建模应用
**WHEN** EmbeddableUIAbility 进程内的新 C++ 模应用接口收到 AMS 的调用
**THEN** 接口在 EmbeddableUIAbility 自己的 UIContent 上调用 `CreateModalUIExtension` 创建模应用（**不调用** `ModalSystemUiExtension->CreateModalUIExtension`），返回 sessionId 用于后续回调管理

### AC5: 隐私弹框以模应用体验呈现
**WHEN** EmbeddableUIAbility 通过新链路成功拉起隐私弹框
**THEN** 隐私弹框 UIExtensionAbility 在嵌入式元服务窗口内以模态方式显示（模应用体验），而非覆盖整个系统的模系统体验；隐私弹框能通过 `IS_EMBEDDABLE_SERVICE` 参数识别调用方为嵌入式元服务

### AC6: onReceive 退出 key 触发嵌入式元服务退出
**WHEN** 隐私弹框 UIExtensionAbility 通过 `SendResult` 在 onReceive 回调中传入约定的退出 key（值为退出标志）
**THEN** 新链路的 ModalCallback 识别该 key，调用嵌入式元服务的退出路径（如 `TerminateSelfWithAnimation`），嵌入式元服务按预期退出

### AC7: IS_EMBEDDABLE_SERVICE 参数语义保持
**WHEN** 新链路向隐私弹框传递 Want
**THEN** Want 中 `ohos.param.isCallerEmbeddableUIExtension = true`，隐私弹框 UIExtensionAbility 读取后能区分调用方为嵌入式元服务（与现有约定语义一致）

---

## 验收标准 (P1)

### AC8: 弹出式元服务场景不受影响
**WHEN** 调用方为弹出式元服务（普通 UIAbility，`SCREEN_MODE_KEY` 为 `JUMP_SCREEN_MODE` 或缺失）
**THEN** 走现有 `disposed_observer.cpp` 既有路径，行为与本次变更前完全一致

### AC9: 异常场景 — EmbeddableUIAbility 进程死亡
**WHEN** AMS 通过新 IPC 调用前/调用中，EmbeddableUIAbility 进程已死亡或 IPC 失败
**THEN** AMS 侧有错误日志（AAFwkTag::ABILITYMGR），不发生 crash，不影响其他 Ability 调度

### AC10: 异常场景 — UIContent 为空
**WHEN** EmbeddableUIAbility 进程内新接口被调用，但当前 UIContent 为空（窗口未就绪）
**THEN** 接口返回错误码（如 `ERR_INVALID_VALUE`），有错误日志（AAFwkTag::UI_EXT），不发生 crash

### AC11: 单元测试覆盖
**WHEN** 执行 `test/unittest/` 下本次新增/扩展的测试套件
**THEN** 覆盖以下场景：嵌入式识别、新 IPC 调用、模应用创建、退出 key 回调、IS_EMBEDDABLE_SERVICE 传递、UIContent 为空、IPC 失败；所有用例通过

### AC12: 日志可观测性
**WHEN** 新链路任意环节被触发
**THEN** 关键节点有 TAG_LOGI/TAG_LOGD 日志（含 screenMode、sessionId、bundleName 等关键字段），便于线上问题定位
