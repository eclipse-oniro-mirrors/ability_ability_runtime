# Design — UPMS 依赖库动态加载/卸载（通用插件框架）

> id: `upms-deps-dynamic-load` | type: feature | complexity: **复杂**（Stage 2 由 standard 升级） | profile: none
> 承接：[proposal.md](proposal.md) §3 基线（范围经 Stage 2 全量依赖审计 + 独占性取证后演进，见 §0）。
> 取证依据（本轮已读）：
> - 全量依赖审计：4 路 Explore（数据/可选/身份/核心四聚类），逐依赖 文件:行号 取证
> - 独占性取证：`services/*/BUILD.gn` external_deps 命中 + `services/sa_profile/*.json` 进程归属（foundation 内驻留 SA 180/182/183/185/501）
> - 现有 dlopen 先例：[file_permission_manager.cpp:34-92](../../../services/uripermmgr/src/file_permission_manager.cpp#L34-L92)（`DllWrapper`，已 dlopen `libams_broker_ext.z.so`）
> - 延时先例：[service_router_mgr_service.cpp:44,86-111](../../../service_router_framework/services/srms/src/service_router_mgr_service.cpp#L44)（SRMS `DelayUnloadTask`）
> - deps_wrapper：[interfaces/inner_api/deps_wrapper/](../../../interfaces/inner_api/deps_wrapper/)（编译时条件编译，非运行时动态加载，不可复用）

---

## 0. 范围演进记录（Stage 2 触发，需求方逐轮确认）

| 轮次 | 决策 |
|------|------|
| Stage 1 基线 | 机制=依赖库粒度 dlopen/dlclose；目标=rdb/data_share/media_library/udmf；complexity=standard |
| Stage 2 调研① | rdb/data_share 零直接调用（假目标）→ 需求方确认「分三类彻底处理」 |
| Stage 2 全量审计 | 发现 ~11 个死依赖；sandbox_manager 实为高频核心；deps_wrapper 不可复用 |
| Stage 2 独占性取证 | bundle/access_token/sandbox/udmf/ability_manager 与 abilitymgr/appmgr 同进程共享→UPMS 侧 dlclose 零近期收益；media/storage/fileuri/broker 为独占候选 |
| **最终（需求方确认）** | **建通用插件框架；独占候选 + 共享库都抽（共享库为模块化，接受零近期内存收益）；retrofit broker；死依赖清理。complexity→复杂** |

## 1. 设计目标（一句话）

为 UPMS（SA 183，`foundation` 进程）建一套**通用依赖插件框架**：把 UPMS 对外部/内部依赖的使用按功能聚类封装成 interface 基类、实现子类放进独立插件 .so，管理器按需 `dlopen`（调导出 C 工厂创建对象、返回基类指针）、空闲 `dlclose`（内部维护加载状态）；并清理死依赖。以此降低空闲内存（独占候选）+ 获得模块化（共享库），且 URI 授权状态强一致。

## 2. 独占性矩阵（决定内存收益，本设计出发点）

> 同进程动态库按引用计数共享。UPMS 侧 `dlclose` 只有在该库是 foundation 进程内**唯一使用者**时才真正卸载、回收内存。共享库即使抽进插件 .so 并 dlclose，引用计数不归零、不卸载、**零近期内存收益**（仅为模块化/未来 foundation 内其它 SA 也卸载时铺路）。

| 依赖 | foundation 内独占? | 抽取内存收益 | 处理 |
|------|------|------|------|
| `media_library` | ✅ 独占候选 | 可能回收 | 插件 .so（feature-gate） |
| `storage_service` | ✅ 独占候选 | 可能回收 | 插件 .so |
| `app_file_service`(fileuri) | ✅ 独占候选 | 可能回收 | 插件 .so |
| `libams_broker_ext`(broker) | ✅ 独占（已 dlopen） | 可回收 | retrofit 进框架 |
| `bundle_framework` | ❌ 与 abilitymgr/appmgr 共享 | 零近期收益 | 插件 .so（模块化） |
| `access_token` | ❌ 共享 | 零近期收益 | 插件 .so（模块化） |
| `sandbox_manager` | ❌ 与 appmgr 共享 | 零近期收益 | 插件 .so（模块化） |
| `udmf` | ❌ 与 abilitymgr 共享 | 零近期收益 | 插件 .so（feature-gate） |
| `ability_manager` | ❌ AMS 同进程 | 零近期收益 | 插件 .so（模块化） |

**独占性目前仅 ability_runtime 内确认；media/storage/fileuri 是否被 foundation 内其它子系统 SA 共用，需 Stage 3/4 用 `/proc/<pid>/maps` 实测确认（ADR-10 硬门槛）。**

## 3. 关键设计决策 (ADR)

### ADR-1：通用插件框架（核心架构）
- **interface 基类**：每类功能一个抽象接口（继承通用 `IDynamicFeature`），定义该类对外方法。接口头放共享位置（如 `interfaces/inner_api/uri_permission/` 下新增 `feature/` 或 `services/uripermmgr/include/feature/`），libupms 与各插件 .so 共同 include。
- **实现子类在插件 .so**：每个插件 .so 实现对应 interface，链接原依赖，把"使用该依赖的代码"搬入（thin 转发：接口方法 → 原 SDK 调用）。
- **管理器 `DynamicFeatureManager`**：注册表 `categoryId → { soName, handle, state(NotLoaded/Loaded), instance }`；`Load()`=dlopen+dlsym 工厂+create；`Unload()`=destroy+dlclose；`UnloadIdle()`=卸载全部 Loaded。
- **对外用法**：`auto scope = manager.Acquire(cat); auto* f = scope.Get<IXxxFeature>(); f->Method(...);` —— 基类指针供外部调用。
- **加载状态内部维护**：每 category 一个状态机；`Acquire` 触发 `Load`（若 NotLoaded），`UnloadIdle` 仅在活动计数归零时执行（ADR-3）。
- **取舍**：通用框架一次性投入，后续新增可卸载依赖零成本接入；代价是首次设计/实现成本高（复杂度升级的理由）。

### ADR-2：插件 .so 契约（共享接口 + C 工厂）
- 共享接口头：`IDynamicFeature`（空基类）+ 各 `IXxxFeature`（纯虚方法）。
- 每个插件 .so 导出**统一 C 工厂符号**：
  ```c
  extern "C" IDynamicFeature* CreateFeature();   // 构造具体子类，以基类指针返回
  extern "C" void DestroyFeature(IDynamicFeature*);
  ```
- 管理器 `dlsym("CreateFeature")` 取工厂，`static_cast<IXxxFeature*>(base)` 给调用方。
- 插件 .so 的 `external_deps` 链接原依赖（如 sandbox_manager）；`libupms` **移除**该依赖的 `external_deps` + 移除对应调用代码。
- **理由**：C 符号可 dlsym（C++ 类方法不可直接 dlsym）；工厂返回基类指针、管理器转型，解耦 libupms 与具体实现。

### ADR-3：活动守卫 RAII（强一致命门，AC-2/AC-3/AC-5）
- `DynamicFeatureScope guard = manager.Acquire(cat)`：构造 → `Load`(若需) + `activeCount++`；析构 → `activeCount--`。
- 返回的基类指针**仅在 guard 存活期有效**（.so 已加载）；guard 析构后不得持有/使用（卸载后悬空）。
- `UnloadIdle()`：取锁 → 等 `activeCount==0` → 对每个 Loaded category `Destroy`+`dlclose` → 置 NotLoaded。
- **保证**：dlclose 执行时无任何线程持有/调用插件对象 → 无 use-after-free。
- **调用点规约**：所有插件调用必须包在 `Acquire` 守卫作用域内，不跨作用域传指针。

### ADR-4：插件分组（7 个 ext .so）
| 插件 .so | 封装依赖 | interface | 独占? | feature-gate |
|----------|---------|-----------|------|------|
| `libupms_identity_ext.z.so` | bundle_framework + access_token + ability_manager | `IIdentityFeature`（GetBundleInfo/GetTokenInfo/IsSystemApp/GetCollaborator） | ❌共享 | 常开 |
| `libupms_sandbox_ext.z.so` | sandbox_manager | `ISandboxPolicyFeature`（Set/Check/UnSet/StartAccessingPolicy） | ❌共享 | SANDBOXMANAGER |
| `libupms_media_ext.z.so` | media_library | `IMediaPermFeature`（Grant/Check/CancelPhotoUri） | ✅独占候选 | MEDIA_LIBRARY_ENABLE |
| `libupms_udmf_ext.z.so` | udmf | `IUdmfFeature`（GetBatchData/AddPrivilege/GetBundleNameByUdKey） | ❌共享 | UDMF_ENABLE |
| `libupms_storage_ext.z.so` | storage_service | `IStorageShareFeature`（Create/DeleteShareFile） | ✅独占候选 | 常开 |
| `libupms_fileuri_ext.z.so` | app_file_service | `IFileUriFeature`（FileUri 解析/GetRealPath） | ✅独占候选 | 常开 |
| `libupms_broker_ext.z.so`（retrofit） | broker C 函数 | `IBrokerCheckFeature`（CheckUri） | ✅独占 | 常开 |
- **分组理由**：按功能内聚；独占候选尽量独立（最大化可回收粒度）；identity 三库同属"身份/应用信息查询"合一个；fileuri 与 storage 虽都涉文件 URI 但独占性与调用点不同，暂分立（可合并，Stage 3 定）。
- **取舍**：7 个 .so 偏多；若实现期发现开销过大，可将 fileuri+storage 合并、或将共享库进一步合并。分组可在 Stage 3 微调。

### ADR-5：broker retrofit 进统一框架
- 现有 [`DllWrapper`](../../../services/uripermmgr/src/file_permission_manager.cpp#L46-L92) 单 so 单函数、static 单例从不卸载。改造为：定义 `IBrokerCheckFeature`，broker .so 实现 + 导出 `CreateFeature`，纳入 `DynamicFeatureManager` 管理。
- 收益：broker 成为框架首个落地用例（验证框架），且获得空闲 dlclose（独占，确定回收）。

### ADR-6：死依赖清理（~11 个）
- 从 `ohos_shared_library("libupms")` 的 `external_deps` 移除零调用依赖：`relational_store:native_rdb`、`native_dataability`、`data_share:datashare_consumer`、`init:libbeget_proxy`、`init:libbegetutil`、`common_event_service:cesfwk_core`/`cesfwk_innerkits`、`background_task_mgr:bgtaskmgr_innerkits`、`i18n:intl_util`、`ability_base:configuration`；内部 `appkit_manager_helper`。
- **`eventhandler`**：UPMS 零调用，但若延时任务选 EventHandler 则需保留；倾向 ffrt（ADR-7）→ eventhandler 可清。
- **逐个 build 验证**：每个移除需 `./build.sh --build-target libupms` 通过（防传递需要）。安全度高（零调用），收益条件性（仅 libupms 独占映射时才减运行时占用，作实测观测项）。

### ADR-7：per-feature 独立空闲计时（非全局），SA 本体不卸
- 仿 SRMS 用 **ffrt**（共享线程池，无独立线程）：`ffrt::submit_h`+`task_attr().delay(UNLOAD_DELAY_TIME_US)`+`ffrt::skip(handle)` 取消；常量 `UNLOAD_DELAY_TIME_US = 90000000`（90s）。
- **每个 feature 独立计时**：`Entry` 自带 `activeCount`（该 feature 的活动 RAII 守卫数）+ `unloadHandle`（该 feature 自己的延时任务）。
- **触发语义（usage-driven per-feature）**：
  - `Acquire(X)`：X 的 activeCount++，**取消 X 的待执行卸载任务**（X 又被使用了）。
  - `Release(X)`（X 的最后一个守卫释放，activeCount→0）：为 **X 单独**挂 90s 延时任务。
  - 延时到期 → `UnloadFeatureIfIdle(X)`：若 X 的 activeCount 仍为 0，**只卸载 X**（dlclose），不动其它 feature。
- **互不干扰**：media 在用时，storage 仍可因自身 90s 空闲被单独卸载——feature 级独立。
- **不调 `UnloadSystemAbility`**：SA 本体常驻，授权账本始终在 libupms 内存。
- **取舍（相对早期全局模型）**：早期方案是"全局 activeCount + 全局延时 + 任一 IPC 重挂全局计时 + 到期卸全部"，会因任一 IPC 阻止所有 feature 卸载；改为 per-feature 后，每个 feature 按自身 Acquire/Release 自计时，卸载粒度=单 feature，内存回收更精细、更及时。代价：每 feature 多一个 task_handle + activeCount 字段，且不再有"任一 IPC=系统活跃"的全局语义（无需 stub_impl 活动 hook）。
- `BUILD.gn` `external_deps` +`"ffrt:libffrt"`（`ffrt_inner.h` 提供 submit_h/skip）。

### ADR-8：共享库抽取的零近期收益说明（需求方已接受）
- bundle_framework/access_token/sandbox_manager/udmf/ability_manager 抽进插件 .so，内存上**近期零收益**（foundation 内 abilitymgr/appmgr 共享，引用计数不归零）。
- 抽取价值=**模块化 + 为未来铺路**：若后续 abilitymgr/appmgr 也采纳同类动态加载，则这些库可能被回收；且插件化降低 libupms 冷启动链接/重定位开销（观测项）。
- 强一致同样适用（ADR-3）：共享库插件卸载/重载周期内 UPMS 调用经守卫正确处理，授权状态零丢失。

### ADR-9：feature-gate 一致性
- `ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE`/`UDMF_ENABLE`/`SANDBOXMANAGER` 语义不变：flag 关 → 对应插件 .so 不构建、libupms 对应路径返回 `CAPABILITY_NOT_SUPPORT(801)`（现状语义）；flag 开 → 经插件动态加载。
- 对应用/下游完全透明，无新增对外 API。

### ADR-10：独占性硬门槛（Stage 3/4 实测）
- media/storage/fileuri/broker 的实际内存收益，必须用 `/proc/<foundation_pid>/maps`（或 hiprofiler）实测确认：插件 dlclose 后对应 .so 是否真正从 foundation 地址空间移除。
- **若发现被 foundation 内其它子系统 SA 共享（非独占）→ 该插件内存收益为零**，评估是否 descoped（仅保留模块化价值）。
- 此门槛决定 AC-1 的实际可观测收益范围。

## 4. 架构（插件框架 + 加载/卸载链路）

```
┌──────────────────────────── libupms.z.so (SA 183, 常驻) ────────────────────────────┐
│  授权账本 uriMap_/policyMap_... (纯内存, 跨卸载保留)                                  │
│  DynamicFeatureManager (新增): 注册表+状态机+活动计数+Acquire/UnloadIdle               │
│  DelayUnloadTask (ffrt, 90s, IPC 活跃度驱动)                                         │
│  共享接口头: IDynamicFeature / IXxxFeature                                           │
└────────────────────────────────────────────────────────────────────────────────────┘
   IPC 入口 ──活动钩子──> 重挂 90s 延时
                              │ 90s 到期(空闲)
                              ▼
                    DynamicFeatureManager::UnloadIdle()  (activeCount==0)
                              │ dlclose 各 Loaded 插件
   ┌──────────┬──────────────┬───────────────┬────────────┬──────────────┬─────────────┐
   ▼          ▼              ▼               ▼            ▼              ▼             ▼
 identity   sandbox        media          udmf        storage        fileuri       broker
 ext(.so)   ext(.so)       ext(.so)       ext(.so)    ext(.so)       ext(.so)      ext(.so)
 ├bundle    ├sandbox_mgr   ├media_lib     ├udmf       ├storage_svc   ├fileuri      └(retrofit)
 ├access    └(共享,0收益)  └(独占✓)       └(共享)     └(独占✓)       └(独占✓)        (独占✓)
 └ability_mgr
 (共享,0收益)
   │ dlclose → 各自原依赖随插件卸载(仅独占候选真正回收)
   ▼
   下次 IPC → 调用点 Acquire(cat) → Load(dlopen+CreateFeature) → 基类指针 → 正确服务 (强一致)
```

## 5. 关键代码点位（实现锚点，Stage 3 execution-plan/task 展开）

| 位置 | 改动 |
|------|------|
| `interfaces/inner_api/uri_permission/feature/`（新增共享头） | `IDynamicFeature` 基类 + `IIdentityFeature`/`ISandboxPolicyFeature`/`IMediaPermFeature`/`IUdmfFeature`/`IStorageShareFeature`/`IFileUriFeature`/`IBrokerCheckFeature` |
| `services/uripermmgr/include/dynamic_feature_manager.h` + `src/dynamic_feature_manager.cpp`（新增） | 管理器：注册表/状态机/活动计数/`Acquire`(RAII scope)/`Load`/`Unload`/`UnloadIdle`；hilog/hisysevent |
| `services/uripermmgr/src/uri_permission_manager_stub_impl.cpp` | per-feature 模型下无需全局活动钩子（各插件经 `Acquire`/`Release` 自计时）；后续相位把 bundle/access/sandbox/storage 调用改为经 `Acquire`+基类指针 |
| `services/uripermmgr/src/file_uri_distribution_utils.cpp` | bundle/access/identity 调用迁入 identity 插件，改经接口 |
| `services/uripermmgr/src/file_permission_manager.cpp` | fileuri/broker 调用改经接口；`DllWrapper` 退役为 broker 插件 |
| `services/uripermmgr/src/media_permission_manager.cpp` / `upms_udmf_utils.cpp` | 核心迁入 media/udmf 插件 |
| `services/uripermmgr/plugins/`（新增目录） | 7 个插件 .so 子目录：identity/sandbox/media/udmf/storage/fileuri/broker，各含实现子类 + `CreateFeature`/`DestroyFeature` + BUILD.gn（链原依赖） |
| `services/uripermmgr/BUILD.gn` | 移除 ~11 死依赖 + 9 个被抽依赖的 external_deps（迁入各插件）；+`ffrt:libffrt`；+插件 .so target |

## 6. Stage 1 → Stage 2 不涉及项 carry-through

| 项 | Stage 1 结论 | Stage 2 确认 |
|----|-------------|-------------|
| SA 实体级动态卸载 | N/A | 确认 N/A（SA 常驻，ADR-7 不调 UnloadSystemAbility） |
| 核心库（ipc/samgr/safwk/hilog/hitrace）卸载 | N/A | 确认 N/A（服务存活必需，不抽） |
| 跨仓 / 改依赖库本身 | N/A | 确认 N/A（插件 .so 在 ability_runtime 内新建，原依赖仓不动） |
| 具体内存 MB 量化 | N/A | 确认 N/A（定性验收：证 PSS 释放即可） |
| 授权/撤销主链路语义 | N/A | 确认 N/A（账本仍在 libupms，仅调用改经接口） |
| 新增对外 API | N/A | 确认 N/A（对应用透明） |
| 授权账本持久化 | N/A | 确认 N/A（账本跨卸载周期保留） |

## 7. 约束 / 风险 / 取舍

| 风险 | 等级 | 缓解 |
|------|------|------|
| **R1 独占性误判 → media/storage/fileuri 实为共享 → 零内存收益** | **高** | ADR-10：Stage 3/4 `/proc/maps` 实测硬门槛；非独占则该插件 descoped 至仅模块化 |
| **R2 插件对象/SDK 单例在 dlclose 后悬空 → UAF** | 高（安全/稳定） | ADR-3：RAII 守卫 + 活动计数，dlclose 仅在 active==0；调用不跨守卫作用域；ASAN 验证（AC-2/3/5） |
| **R3 共享库抽取零近期收益** | 中 | ADR-8：需求方已接受（模块化价值）；design 明示，不夸大内存收益 |
| **R4 dlclose 不保证物理释放 / allocator 持有 vmem** | 中 | 已知；定性验收，PSS 降幅仅观测 |
| **R5 活动钩子遗漏 → 空闲误判 → 卸载在途调用** | 中 | ADR-3 活动计数兜底（钩子漏则 activeCount>0 阻止 dlclose）；统一 hook 宏 |
| **R6 7 个插件 .so 数量偏多 / 维护成本** | 中 | ADR-4：Stage 3 可合并（fileuri+storage 等）；插件薄转发，低复杂度 |
| **R7 高频路径经接口+守卫的性能开销** | 中 | 守卫=一次原子加减+可能 dlopen（仅首加载）；活动期内不重复 Load；需测 Check 高频路径 |
| **R8 死依赖移除致传递链接断裂** | 低 | ADR-6：逐个 build 验证 |

## 8. 验证策略（design 层面，详见 spec.md §AC）

- **框架单测**：`DynamicFeatureManager` 状态机（Load 幂等/UnloadIdle 计数归零/重 Load）、RAII 守卫、延时任务重挂/到期。
- **插件契约单测**：每个插件的 `CreateFeature`/`DestroyFeature` + 接口方法转发正确性。
- **强一致（AC-2/3）**：模拟「空闲卸载→卸载态注入 IPC→Load 恢复→正确服务」，断言授权零丢失、无 crash；ASAN。
- **独占性实测（AC-1/ADR-10 硬门槛）**：`/proc/<pid>/maps` 对比插件 dlclose 前后 .so 是否真移除，逐插件判定独占。
- **回归（AC-4）**：`run -t UT -tp uripermmgr` 全绿。
- **分阶段交付（需求方调整：media 提前至 P1，broker 调整至 P2）**：
  - **Phase 1（框架 + media + 死依赖）**：共享接口头 + `DynamicFeatureManager` + RAII 守卫 + 延时任务(ffrt) + **media 插件（首个落地用例，真实独占回收）** + 死依赖清理。⚠️ 取舍：media 作为首个用例比 broker 更难（C++ SDK 静态单例 + wrapper .so + C 工厂），框架首次集成落在较难目标；但更早兑现内存收益，且框架迟早须支持 SDK 单例类插件。media 独占性（ADR-10）作为 P1 早期实测项。
  - **Phase 2（broker retrofit + 独占候选）**：broker retrofit（接入框架、获空闲 dlclose）+ storage/fileuri 插件化——过 ADR-10 独占门槛。
  - **Phase 3（共享库插件，模块化）**：identity/sandbox/udmf 插件化——零近期内存收益，按模块化优先级排。

## 9. 待确认（进 Stage 3 前）
- OQ-1 `target_release`：Stage 3 向 owner 确认。
- **阶段划分是否采纳**（Phase 1/2/3）：影响 execution-plan 拆分与验证顺序，需需求方 Stage 2 审批时确认。
- **插件分组是否调整**（7 个是否合并）：Stage 3 实现时定。
- **共享接口头落点**：`interfaces/inner_api/uri_permission/feature/` vs `services/uripermmgr/include/feature/`——前者更规范（inner_api），Stage 3 定。
