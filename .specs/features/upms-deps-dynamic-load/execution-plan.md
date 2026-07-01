# Execution Plan — UPMS 依赖插件框架动态加载/卸载

> id: `upms-deps-dynamic-load` | stage: 3 (Implement) | complexity: 复杂
> 承接：[proposal.md](proposal.md) §3.9 + [design.md](design.md) ADR-1~10 + [spec.md](spec.md) AC-1~10。
> 本文件：任务分解、依赖、文件范围、规则映射、验证期望、交接。阶段：**Phase 1（框架+media+死依赖）→ Phase 2（broker retrofit+storage+fileuri）→ Phase 3（identity+sandbox+udmf）**。
> 代码遵循 OpenHarmony C++ 规范；日志用 `hilog_tag_wrapper.h` 的 `AAFwkLogTag::URIPERMMGR`；dlopen/dlclose 错误用 `dlerror()`（与现有 `DllWrapper` 一致）。

---

## 1. 实现总览

在 `services/uripermmgr/` 新增**通用依赖插件框架**（共享接口头 + `DynamicFeatureManager` + RAII 守卫 + ffrt 延时任务），把 UPMS 对外部/内部依赖的使用抽进独立插件 .so（interface 基类 + 子类实现 + 导出 C 工厂），管理器按需 dlopen、空闲 dlclose；并清理 ~11 个死依赖。单仓，不改对外 API / sa_profile / 原依赖仓。

## 2. 任务分解

### Phase 1（框架 + media + 死依赖）
| 任务 | 文件范围 | 依赖 | 完成标准 |
|------|---------|------|---------|
| [task-01-framework](tasks/task-01-framework.md) | 共享接口头 + `dynamic_feature_manager.h/.cpp` + stub_impl 活动 hook + BUILD.gn(+ffrt) | — | 框架可编译；manager 状态机/守卫/延时单测过 |
| [task-02-media-plugin](tasks/task-02-media-plugin.md) | `plugins/media_ext/` + media 调用点改造 + BUILD.gn | task-01 | media 插件 .so 构建；media 路径经接口工作；AC-2/3/4 |
| [task-03-dead-deps](tasks/task-03-dead-deps.md) | `BUILD.gn` external_deps 移除 ~11 | task-01 | 逐个 build 验证通过；UT 全绿 |

### Phase 2（storage 插件 + 独占实测）
> ⚠️ 2026-07-01 范围调整：原 Phase 2 含 broker(fileuri)/storage/fileuri 三插件，现**只保留 storage**；task-04(broker retrofit)/task-06(fileuri) DESCOPED（fileuri 暂不插件化，libupms 直链 fileuri_native，file_permission_manager 直调 FileUri）。

| 任务 | 文件范围 | 依赖 | 完成标准 |
|------|---------|------|---------|
| [task-05-storage-plugin](tasks/task-05-storage-plugin.md) | `plugins/storage_ext/` + storage 调用点改造 | task-01 | storage 插件；AC-1/2/3 |
| task-07-exclusivity-verify | `/proc/<pid>/maps` 实测 | task-02/05 | AC-10 media+storage 逐插件独占性判定 |

### ~~Phase 3（共享库，模块化）~~ — DESCOPED
> 2026-07-01 范围调整：identity/sandbox/udmf 暂不处理（共享库零近期内存收益，模块化非本次目标）。task-08/09/10 DESCOPED。

> Phase 2/3 任务卡在各自启动时编写（避免过早细化）；本计划先详化 Phase 1。

## 3. 执行顺序

```
Phase 1:  task-01 框架 ─┬─▶ task-02 media 插件 ─▶ task-07(部分) media 独占实测
                        └─▶ task-03 死依赖清理(并行)
Phase 2:  task-01 ─▶ task-05 storage ─▶ task-07 独占实测(media+storage)
[DESCOPED] task-04 broker/fileuri retrofit、task-06 fileuri 插件、Phase 3(identity/sandbox/udmf) — 2026-07-01 范围调整，暂不处理
```

## 4. 文件范围（Phase 1，精确）

| 文件 | 改动类型 |
|------|---------|
| `interfaces/inner_api/uri_permission/feature/idynamic_feature.h`（新增，落点 Stage 3 定） | 通用基类 `IDynamicFeature` + `IMediaPermFeature` 等（P1 仅 media，余随相位加） |
| `services/uripermmgr/include/dynamic_feature_manager.h`（新增） | 管理器 + RAII `DynamicFeatureScope` + 注册接口 |
| `services/uripermmgr/src/dynamic_feature_manager.cpp`（新增） | 实现：注册表/状态机/活动计数/`Acquire`/`Load`/`Unload`/`UnloadIdle`/延时任务 |
| `services/uripermmgr/src/uri_permission_manager_stub_impl.cpp/.h` | 每个 public IPC 入口加活动 hook（重挂延时） |
| `services/uripermmgr/plugins/media_ext/`（新增目录） | `media_perm_feature_impl.h/.cpp` + `media_ext.cpp`(C 工厂) + `BUILD.gn`(链 media_library) |
| `services/uripermmgr/src/media_permission_manager.cpp` | media 调用改为经 `Acquire`+`IMediaPermFeature` |
| `services/uripermmgr/BUILD.gn` | +`ffrt:libffrt`；+`dynamic_feature_manager.cpp`；media 源码迁入插件；移除 media_library external_deps（迁插件）；死依赖移除（task-03） |
| `services/uripermmgr/include/media_permission_manager.h` | 调用签名调整（经接口） |

## 5. 规则映射（design ADR ↔ 实现）

| ADR | 实现落点 |
|-----|---------|
| ADR-1 通用框架 | `IDynamicFeature` 基类 + `DynamicFeatureManager`（注册表+状态机+Acquire/Load/Unload/UnloadIdle） |
| ADR-2 C 工厂契约 | 插件导出 `extern "C" IDynamicFeature* CreateFeature()` / `DestroyFeature()`；manager dlsym + static_cast |
| ADR-3 RAII 守卫 | `DynamicFeatureScope`（Acquire+activeCount++/--）；UnloadIdle 仅 active==0 时 dlclose |
| ADR-4 插件分组 | P1: media；P2: broker/storage/fileuri；P3: identity/sandbox/udmf |
| ADR-5 broker retrofit | P2 task-04 |
| ADR-6 死依赖清理 | P1 task-03（~11 个） |
| ADR-7 ffrt 延时 + 活动 hook | manager 持 ffrt task_handle + timerMutex_；stub_impl 各 IPC 入口重挂 |
| ADR-8 共享库零收益说明 | P3 任务卡明示 |
| ADR-9 feature-gate | media/udmf/sandbox 插件 .so 仅 flag 开时构建；flag 关返回 801 |
| ADR-10 独占门槛 | task-07 `/proc/maps` 实测 |

## 6. 验证期望（逐阶段最低验证）

| 阶段 | 验证 | 对应 AC |
|------|------|--------|
| 编译 | `./build.sh --product-name rk3568 --build-target libupms` + 各插件 .so target 通过 | — |
| UT | `run -t UT -tp uripermmgr`（含 uri_permission_impl_test 等）全绿 | AC-2/3/4/5/6/7/8 |
| 独占实测 | `/proc/<foundation_pid>/maps` 对比插件 dlclose 前后 | AC-1/10 |
| 强一致/并发 | 卸载态注入 IPC 单测 + ASAN；并发 UnloadIdle 单测 + TSAN | AC-2/3/5 |
| **硬门槛** | AC-10 独占性：media（P1）/broker·storage·fileuri（P2）逐插件实测；非独占 descoped | AC-1/10 |

## 7. 风险控制（实现期）

- **media SDK 单例 dlclose 安全**：media 插件 dlclose 前，确保无 MediaPermissionHelper 活动引用；调用经 `Acquire` 守卫、不跨作用域持指针；ASAN 验证（R2）。
- **独占性实测前置（media）**：task-02 完成后立即 task-07 测 media 独占性；若 media 被其它子系统共享 → descoped，回 Stage 2 重评（R1）。
- **活动 hook 穷尽**：所有对外 IPC 入口必须重挂延时，统一用 hook 宏避免遗漏（R5）。
- **死依赖逐个验证**：每个移除后 build 一次，防传递链接断裂（R8）。
- **mock 兼容**：单测构造 manager 不依赖 SA 单例；插件 C 工厂可 mock。

## 8. 交接（handoff）

- **前置条件**：Stage 2 已批准（✅ 2026-06-28）。
- **执行方**：本会话直接实现（Phase 1 规模适中）；Phase 2/3 视情况派发 subagent。
- **放行**：Stage 3 内 plan+task→代码→review 无需额外审批；但 **AC-10 独占门槛**为 Phase 2 投入的硬关卡。
- **收尾**：每 Phase 实现+测试通过写 review.md（统一审查），全 Phase 完成进 Stage 4（验证+合入+复盘）。
