# Spec — UPMS 依赖库动态加载/卸载（通用插件框架）

> id: `upms-deps-dynamic-load` | type: feature | complexity: **复杂** | profile: none
> 承接：[proposal.md](proposal.md) §3 基线 + [design.md](design.md) ADR-1~10。
> 范围经 Stage 2 全量依赖审计 + 独占性取证演进（见 design §0）：建通用插件框架，抽取 9 个依赖（独占候选 + 共享库）+ retrofit broker + 死依赖清理。
> `manifest.target_release` = 待确认（OQ-1）；`manifest.profile` = none；lineage 见 manifest。

---

## 1. 特性概述

为 UPMS（SA 183，`libupms.z.so`，`foundation` 进程）建一套**通用依赖插件框架**：把 UPMS 对外部/内部依赖的使用按功能聚类封装成 interface 基类，实现子类放进独立插件 .so；`DynamicFeatureManager` 按需 `dlopen`（调导出 C 工厂创建对象、返回基类指针）、空闲延时（90s 无 IPC）`dlclose`（内部维护加载状态）。覆盖 7 个插件：identity(bundle+access+ability_manager)、sandbox、media、udmf、storage、fileuri、broker(retrofit)。并清理 ~11 个死依赖。目标=降空闲内存（独占候选）+ 模块化（共享库），授权状态强一致（账本常驻 libupms，零丢失零错授权）。对应用完全透明。

## 2. 用户故事

- **US-1（系统/内存）**：作为内存预算紧张的设备，我希望 UPMS 在空闲窗口把独占依赖（media/storage/fileuri/broker）的内存还给 foundation。
- **US-2（安全/稳定）**：作为系统安全维护者，我希望任何插件的卸载/重载绝不破坏 URI 授权状态、绝不在卸载窗口丢失或错处理在途 IPC。
- **US-3（功能正确性）**：作为使用跨应用 URI 授权（media URI、UDMF Key、docs/沙箱 URI、分布式文件）的应用，我希望这些功能在插件化后行为完全一致。
- **US-4（可维护/演进）**：作为系统维护者，我希望有一套通用框架，后续新增可卸载依赖可零成本接入；且 UPMS 不再背负死依赖。
- **US-5（模块化）**：作为架构维护者，我希望 UPMS 与重依赖解耦（即使部分库当前因 foundation 共享而无近期内存收益），为未来各 SA 协同动态加载铺路。

## 3. 验收标准 (AC，WHEN/THEN，可测)

每条 AC 标注来源（proposal §3.5）与设计依据（design ADR）。

### P0
- **AC-1 插件空闲卸载与内存释放** [proposal P0-1 / ADR-1,3,7,10]
  - **WHEN** UPMS 持续空闲 ≥ 90s（`UNLOAD_DELAY_TIME`，无对外 IPC）
  - **THEN** `DynamicFeatureManager::UnloadIdle()` 对已加载插件执行 `dlclose`；对**独占候选**（media/storage/fileuri/broker）`/proc/<pid>/maps`/hiprofiler 测得对应 .so 真正移除、PSS 下降可稳定复测。
  - **门槛**：ADR-10——独占性 Stage 3/4 实测确认；非独占则该插件内存收益记零（仅模块化）。

- **AC-2 状态强一致** [proposal P0-2 / ADR-3,8]
  - **WHEN** 任一插件发生「空闲卸载 → 按需重载」完整周期（含 identity/sandbox/media/udmf/storage/fileuri/broker 各路径）
  - **THEN** 周期前后 UPMS 授权状态（`uriMap_`/`policyMap_`/`permissionTokenMap_`/`contentTokenIdSet_`）完全一致——零丢失、零错授权。
  - **验证**：覆盖各 URI 类型（media/docs/sandbox/分布式/UDMF Key）Grant→卸载→重载→Check/Revoke 逐项一致。

- **AC-3 在途 IPC 安全** [proposal P0-3 / ADR-3]
  - **WHEN** 卸载期间（插件已 `dlclose`）有 IPC 到达（含各插件路径）
  - **THEN** 经 `Acquire`→`Load`(dlopen+CreateFeature)→基类指针→正确处理，**不 crash/不报错/不丢数据**。
  - **验证**：单测模拟「卸载态注入 IPC」，断言正确且无 crash；ASAN 无 UAF。

### P1
- **AC-4 功能不回归** [proposal P1-4 / ADR-4,9]
  - **WHEN** 引入插件框架
  - **THEN** UPMS 全部对外接口（Grant*/Check*/Revoke*/GrantByKey*/Clear*/Verify*，含 media/udmf/sandbox 各路径）行为与改造前完全一致。
  - **验证**：`run -t UT -tp uripermmgr`（含 uri_permission_manager_test/impl_test/test/perm_mgr_test）全绿。

- **AC-5 并发安全** [proposal P1-5 / ADR-3]
  - **WHEN** `dlclose`（UnloadIdle）与并发 IPC 同时发生
  - **THEN** 无 use-after-free/crash/死锁（活动计数保证 dlclose 仅在 active==0）。
  - **验证**：并发单测（多线程持续 IPC + 周期 UnloadIdle）；ASAN/TSAN。

- **AC-6 可观测** [proposal P1-6 / ADR-1]
  - **WHEN** 插件 Load/Unload/延时重挂事件发生
  - **THEN** hilog（`AAFwkLogTag::URIPERMMGR`）/hisysevent 记录插件名+结果。
  - **验证**：触发各事件查日志确认上报。

- **AC-7 死依赖清理** [（范围演进新增）/ ADR-6]
  - **WHEN** 从 libupms `external_deps` 移除 ~11 个零调用依赖
  - **THEN** `./build.sh --build-target libupms` 通过、UPMS 全功能不回归；条件性运行时占用下降（实测观测项）。
  - **验证**：逐个 build 验证 + UT 全绿 + maps 对比。

- **AC-8 框架通用性** [（复杂 tier 新增）/ ADR-1,2]
  - **WHEN** 新增一个可卸载依赖
  - **THEN** 仅需：定义 interface + 实现插件 .so（导出 `CreateFeature`/`DestroyFeature`）+ 注册到 manager，无需改框架核心。
  - **验证**：以 broker retrofit 为首个范例（AC-9）证明接入成本。

- **AC-9 broker retrofit** [（复杂 tier 新增）/ ADR-5]
  - **WHEN** 现有 `libams_broker_ext.z.so` 经 `IBrokerCheckFeature` 接入框架
  - **THEN** broker 调用经框架 `Acquire`+接口，空闲时随 `UnloadIdle` dlclose，URI check 功能不变。
  - **验证**：broker 路径单测 + 空闲后 maps 确认 broker .so 移除（独占）。

- **AC-10 独占性硬门槛（实测决策）** [（复杂 tier 新增）/ ADR-10]
  - **WHEN** Stage 3/4 对 media/storage/fileuri/broker 做 `/proc/<pid>/maps` 实测
  - **THEN** 逐插件判定独占性；非独占者明确 descoped 至「仅模块化」（内存收益记零），并记录决策。
  - **门槛**：AC-1 的实际收益范围由此决定。

## 4. 阶段划分（design §8，需求方调整：media→P1、broker→P2）

- **Phase 1（框架 + media + 死依赖）**：共享接口头 + `DynamicFeatureManager` + RAII 守卫 + 延时任务 + **media 插件（首个用例，真实独占回收）** + 死依赖清理。media 独占性（AC-10）作 P1 早期实测。验证 AC-1(media)/AC-2/AC-3/AC-4/AC-5/AC-6/AC-7/AC-8。
- **Phase 2（broker retrofit + 独占候选，过门槛后）**：broker retrofit（AC-9）+ storage/fileuri 插件化——过 AC-10 独占门槛；扩展 AC-1(broker/storage/fileuri)。
- **Phase 3（共享库插件，模块化）**：identity/sandbox/udmf 插件化——零近期内存收益，按模块化优先级排。

## 5. 插件契约与边界（design ADR-1/2/3）

- **共享接口**：`IDynamicFeature`（基类）+ 7 个 `IXxxFeature`（纯虚）。落点 Stage 3 定（倾向 `interfaces/inner_api/uri_permission/feature/`）。
- **C 工厂**：每插件导出 `extern "C" IDynamicFeature* CreateFeature()` / `void DestroyFeature(IDynamicFeature*)`。
- **调用规约**：所有插件调用包在 `DynamicFeatureScope guard = manager.Acquire(cat)` 作用域内；基类指针不跨作用域持有（卸载后悬空）。
- **不抽（核心库）**：ipc/samgr/safwk/hilog/hisysevent/hitrace/c_utils/eventhandler(if retained for timer)/perm_verification/app_util 等——服务存活/每次调用必需。
- **SA 本体**：`libupms.z.so` 常驻，不调 `UnloadSystemAbility`。

## 6. 兼容性 / API 影响

- **对外 API**：无新增/变更/废弃，对应用与下游完全透明。
- **feature flag**：`ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE`/`UDMF_ENABLE`/`SANDBOXMANAGER` 语义不变（关→对应插件不构建、返回 `CAPABILITY_NOT_SUPPORT(801)`；开→经插件动态加载）。
- **进程/SA**：UPMS 仍在 foundation、SA 183、`run-on-create:false`；`sa_profile/183.json` 不改。
- **依赖库本身**：各原依赖仓**不动**（单仓约束；插件在 ability_runtime 内链接它们）。

## 7. 可追溯性

| AC | proposal §3.5 | design ADR | 验证手段 | 阶段 |
|----|---------------|-----------|---------|------|
| AC-1 插件卸载/内存释放 | P0-1 | 1,3,7,10 | maps/hiprofiler PSS 对比 | P1(media)/P2(broker,storage,fileuri) |
| AC-2 状态强一致 | P0-2 | 3,8 | Grant→卸载→重载→Check 一致 | P1/P2/P3 |
| AC-3 在途 IPC 安全 | P0-3 | 3 | 卸载态注入 IPC 单测 + ASAN | P1/P2/P3 |
| AC-4 功能不回归 | P1-4 | 4,9 | uripermmgr UT 全套 | P1/P2/P3 |
| AC-5 并发安全 | P1-5 | 3 | 并发单测 + ASAN/TSAN | P1 |
| AC-6 可观测 | P1-6 | 1 | hilog/hisysevent 查验 | P1 |
| AC-7 死依赖清理 | （新增） | 6 | 编译 + UT + maps 对比 | P1 |
| AC-8 框架通用性 | （新增） | 1,2 | broker retrofit 示例 | P1 |
| AC-9 broker retrofit | （新增） | 5 | broker 路径单测 + maps | P2 |
| AC-10 独占性门槛 | （新增） | 10 | /proc/maps 实测决策 | P1(media)/P2前(broker,storage,fileuri) |

## 8. 不涉及项确认表（carry-through）

| 项 | 是否涉及 | 说明 |
|----|---------|------|
| SA 实体级动态卸载 | N/A | 已证不可行；SA 常驻 |
| 核心库（ipc/samgr/safwk/hilog...）卸载 | N/A | 服务存活必需 |
| 跨仓 / 改依赖库本身 | N/A | 单仓；插件在 ability_runtime 内 |
| 具体内存 MB 量化 | N/A | 定性验收：证 PSS 释放即可 |
| 授权/撤销主链路语义改写 | N/A | 账本仍在 libupms，仅调用改经接口 |
| 新增对外 API | N/A | 对应用透明 |
| 授权账本持久化 | N/A | 账本跨卸载周期保留 |
| sa_profile 改动 | N/A | 183.json 不改 |
