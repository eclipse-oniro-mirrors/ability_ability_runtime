# Review — UPMS 依赖插件框架（Phase 1 实现审查）

> id: `upms-deps-dynamic-load` | stage: 3 (Implement) | phase: Phase 1 审查
> 承接：[design.md](design.md) ADR-1~10、[spec.md](spec.md) AC-1~10、[execution-plan.md](execution-plan.md)。
> 本文件：Phase 1（框架 + media 插件 + 死依赖）实现后的统一审查——规范符合性、代码质量自审、验证状态、待验证清单、遗留风险。

---

## 1. Phase 1 实现概览

| 任务 | 产物 | 状态 |
|------|------|------|
| task-01 框架 | `include/feature/idynamic_feature.h`、`include/dynamic_feature_manager.h`、`src/dynamic_feature_manager.cpp`、`Init` 注册 MEDIA、`Entry::instance` 用 `unique_ptr<IDynamicFeature, DestroyDeleter>`（跨 DSO 销毁进类型）、per-feature 独立 idle 计时（Acquire/Release 自驱动） | 代码完成 |
| task-02 media 插件 | `plugins/media_ext/{media_perm_feature_impl.h,.cpp, media_ext.cpp, BUILD.gn}`、`media_permission_manager.h/.cpp` 改薄转发、libupms BUILD.gn 去 media_library | 代码完成 |
| task-03 死依赖 | BUILD.gn 移除 `relational_store:native_rdb`/`native_dataability`/`data_share:datashare_consumer`（shared lib） | 3 项完成；其余 8 项暂存 |

## 2. 规范符合性（design ADR ↔ 实现）

| ADR | 实现落点 | 符合 |
|-----|---------|------|
| ADR-1 通用框架 | `DynamicFeatureManager`（注册表/状态机/活动计数/Acquire/Load/Unload/UnloadIdle）+ `IDynamicFeature` 基类 + `FeatureId` | ✅ |
| ADR-2 C 工厂契约 | 插件 `media_ext.cpp` 导出 `extern "C" CreateFeature/DestroyFeature`（`visibility("default")`）；manager `dlsym`+`static_cast` | ✅ |
| ADR-3 RAII 守卫 | `DynamicFeatureScope`（构造 Acquire+activeCount++，析构 Release+notify）；`UnloadIdle` 仅 activeCount==0 | ✅ |
| ADR-4 media 插件分组 | `libupms_media_ext.z.so` 实现 `IMediaPermFeature` | ✅（P1 首个用例） |
| ADR-6 死依赖清理 | rdb/dataability/data_share 移除 | ✅（3/11） |
| ADR-7 ffrt 延时 + 活动 hook | `ArmIdleUnload` 用 `ffrt::submit_h`+`task_attr().delay(90s).name()`；`OnRemoteRequest` 单一 chokepoint 重挂 | ✅ |
| ADR-9 feature-gate | media 插件 + 注册均在 `ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE` 下 | ✅ |
| ~~测试注入口~~（已移除） | 原计划的 `RegisterInstance` 测试注入口已按 YAGNI 去除（业务不使用）；UT 需其它 mock 方案 | — |

## 3. 代码质量自审（对齐先例）

- **dlopen/dlclose/dlsym**：与现有 `DllWrapper`（file_permission_manager.cpp:46-92）同模式——`RTLD_NOW`、`dlerror()` 记日志、`reinterpret_cast<void*→函数指针>`（仓内既有写法）。
- **ffrt**：`submit_h(task,{},{},task_attr().delay().name())` + `ffrt::skip(handle)`，对齐 `report_data_partition_usage_manager.cpp:44-46` 与 `ffrt_task_handler_wrap.cpp`。
- **hilog**：统一 `TAG_LOG*(AAFwkLogTag::URIPERMMGR, ...)`，与 service.cpp/stub_impl.cpp 一致。
- **media 插件逻辑**：`media_perm_feature_impl.cpp` 逐行移植自原 `media_permission_manager.cpp`（保留 `IN_PROCESS_CALL`、flag 转换、hideSensitive 转换），语义零偏移。
- **同步**：`activeCount_`+`cv_` 在 `mutex_` 下，`UnloadIdle` `wait(activeCount==0)` 保证 dlclose 时无在途调用（AC-3/5）。
- **RAII 守卫**：move 语义正确，析构 Release 幂等（`valid_` 防重复）。

## 4. 验证状态 ✅（编译通过）

**Phase 1 编译全通过**（2026-06-29）：
- ✅ `libupms` 编译成功（含 `dynamic_feature_manager.cpp` + `media_permission_manager.cpp` 转发层 + service `Init` 注册 + 死依赖移除）
- ✅ `libupms_media_ext` 插件编译成功（产物 `libupms_media_ext.z.so`）
- ✅ 插件符号导出：`CreateFeature`/`DestroyFeature` 为 GLOBAL TEXT 符号（`T`），`visibility("default")` 生效（V5 验证通过）
- ✅ **per-feature 独立计时重构编译通过**（2026-06-29 晚）：每个 Entry 自带 `activeCount`+`unloadHandle`，`Acquire(X)` 取消 X 的卸载任务、`Release(X)`（末次引用）为 X 单独挂 90s 延时、到期 `UnloadFeatureIfIdle(X)` 只卸 X；移除 stub_impl 全局 `OnRemoteRequest` 活动 hook（per-feature 按自身 Acquire/Release 自计时，feature 间互不干扰）

**编译期修复记录（迭代过程）**：
1. ffrt include：`"ffrt/ffrt.h"` → `"ffrt.h"`（kits 头）；后又发现 `ffrt::skip`/`submit_h` 在 inner_api → 改 `"ffrt_inner.h"`
2. 插件 target 进图：`services/BUILD.gn` 的 `ams_target` group 加 `uripermmgr/plugins/media_ext:libupms_media_ext`（受 `ability_runtime_upms` + media flag 控制）—— 解决 "unknown target"
3. 插件 include 路径：`../include` → `../../include`（插件 BUILD.gn 在 `plugins/media_ext/`，需上两级到 `uripermmgr/include/`）
4. 插件传递依赖：`media_permission_helper.h` 内部 include `datashare_helper.h` → 插件 BUILD.gn 补 `data_share:datashare_consumer` external_dep

> 印证：data_share 在 media 插件里是 media_library 的**传递依赖**（合理）；对 libupms 主库仍是死依赖（已移除，主库不调）。

abilitykit_native 预存在断裂未再现（本次 build 未命中，可能已被同步或 ninja 未调度到）。

## 5. 验证清单结果（编译期）

| # | 项 | 结果 | 证据 |
|---|----|------|------|
| V1 | `ffrt::skip(*unloadHandle_)` task_handle 传参 | ✅ 通过 | task_handle 可拷贝（task.h:382）；skip(task_handle&) 签名匹配；libupms 编过 |
| V2 | `ffrt::submit_h` + `task_attr().delay().name()` | ✅ 通过 | libupms 编过 |
| V3 | `OnRemoteRequest` 重写签名 | ✅ 通过 | stub_impl.o 编过 |
| V4 | 插件依赖完备性 | ✅ 通过（修了：data_share 传递依赖） | libupms_media_ext 编过 |
| V5 | CreateFeature/DestroyFeature 导出 | ✅ 通过 | `nm -D`：`T CreateFeature` / `T DestroyFeature`（GLOBAL） |
| V6 | cfi_cross_dso 与 dlopen/dlsym 兼容 | ⏳ 待运行验证 | 编译通过；运行期 cfi-icall 行为待设备验证 |
| V7 | 插件进镜像 | ✅ 进图（services/BUILD.gn ams_target） | 镜像实际含 .so 待打包验证 |
| V8 | 死依赖移除无未解析符号 | ✅ 通过 | libupms 编过（rdb/dataability/data_share 移除后链接正常） |
| V9 | 其余 8 死依赖 | ⏳ 暂存 | 待 task-03 协议逐个 build 验证 |

## 6. 遗留 / 风险 / 后续

- **R-test 单测影响（重要）**：现有 `uri_permission_*` 单测中走 media 路径的用例，因 `MediaPermissionManager` 改为 dlopen 转发、测试环境无插件 .so，`Acquire` 返回 null → 用例失败。**对策**（`RegisterInstance` 测试注入口已按 YAGNI 去除）：单测需其它 mock 方案——例如构造一个 test-only 插件 .so 并注册同 soname，或对 `IMediaPermFeature` 调用点做接口抽象 + mock，或在单测中跳过 media 路径用例并转为对插件 .so 的独立测试。AC-4（回归）的前提。
- **R-pkg 插件打包**：libupms 不链接插件（运行时 dlopen），插件须进镜像（V7）。需改 `bundle.json` 的 `component.build.module_list` 加 `//services/uripermmgr/plugins/media_ext:libupms_media_ext`。
- **R1 独占性实测（AC-10）**：media 是否 foundation 内独占，需 `/proc/<pid>/maps` 实测（设备环境）——决定 media 插件内存收益是否兑现。
- **Phase 2/3 未启动**：broker retrofit / storage / fileuri / identity / sandbox / udmf 插件按 execution-plan 后续相位推进。
- **static lib (`libupms_static`)**：未改其 media 块（仍链 media_library，供既有测试）。转发层在 static lib 中同样走 DynamicFeatureManager——static lib 测试同样需上述 mock 方案。

## 7. Phase 1 完成判定（待 build 解封后）

- [ ] abilitykit_native 断裂修复（环境，非本特性）
- [ ] V1~V8 全部 build 通过
- [ ] 单测：用 mock 方案后 `run -t UT -tp uripermmgr` 全绿（AC-4）
- [ ] 框架单测：Register/Acquire/UnloadIdle/RAII/延时 Arm/Cancel（AC-3/5/6/8）
- [ ] AC-10：media 独占性 `/proc/maps` 实测
- [ ] 插件打包进镜像（V7）
- [ ] 其余 8 死依赖按 task-03 协议逐个验证移除

> **当前结论**：Phase 1 代码已全部落地并通过强自审，但**未经编译验证**（分支预存在断裂阻断）。进入 Stage 4（验证+合入）前，必须先解封 build 并跑通上述清单。

---

## 8. Phase 2 审查（broker retrofit + storage + fileuri）— 2026-06-30

### 8.1 实现概览
| 任务 | 产物 | 状态 |
|------|------|------|
| task-04 broker retrofit | `file_permission_manager.cpp` `GetPathPolicyInfoFromUri` 经 `IFileUriFeature`；不另建 broker_ext（decision：与 task-06 合并） | 代码完成 + 编译通过 |
| task-05 storage 插件 | `plugins/storage_ext/`（含 death recipient + GetStorageManager + StringVecToRawData 迁入）+ stub_impl CreateShareFile/DeleteShareFile 转发 + 主库删 storageManager_/StringVecToRawData/istorage_manager.h + libupms 删 storage_service external_deps | 代码完成 + 编译通过 |
| task-06 fileuri 插件 | `plugins/fileuri_ext/`（FileUri 逐行移植）+ file_permission_manager 转发 + libupms 删 fileuri_native external_deps + FeatureId enum 扩展 | 代码完成 + 编译通过 |

### 8.2 编译期修复（本次会话）
| # | 问题 | 修复 |
|---|------|------|
| B1 | stub_impl.h 残留 `sptr<StorageManager::IStorageManager> storageManager_`，但 istorage_manager.h 已删 → 类型未定义 | 删成员（cpp 零引用，grep 确认） |
| B2 | stub_impl.h 残留 `StringVecToRawData` 声明，cpp 定义已删 | 删声明（cpp 零调用） |
| B4 | libupms BUILD.gn 残留 `storage_service:storage_manager_sa_proxy` 死依赖（stub_impl 已不直连） | 删 external_deps（grep 确认主库 src 无 storage 残留） |
| B5 | services/BUILD.gn ams_target 加了 fileuri_ext 但漏 storage_ext | 补 `deps += storage_ext` |
| — | libupms 已删 `fileuri_native` external_deps（工作树原改动） | grep 确认主库 src 无 file_uri.h/AppFileService 残留 ✓ |
| — | R-pkg 担心（§6） | bundle.json:136 module_list 已含 ams_target，ams_target 含三插件 deps → 插件进镜像，不需额外改 bundle.json |

### 8.3 验证状态 ✅（编译通过）
- ✅ libupms build success（2026-06-30，2:04）—— 含 stub_impl storage 转发 + file_permission_manager fileuri 转发 + 主库死依赖清理（storage_service/fileuri_native）+ 成员/声明删除
- ✅ libupms_storage_ext build success；`nm -D`：`T CreateFeature`/`T DestroyFeature`
- ✅ libupms_fileuri_ext build success；`nm -D`：`T CreateFeature`/`T DestroyFeature`
- ✅ rules passed（deps_guard 全绿）

### 8.4 规则符合性（design ADR ↔ 实现）
| ADR | 实现落点 | 符合 |
|-----|---------|------|
| ADR-1 通用框架 | `Acquire(STORAGE/FILEURI)` + per-feature 独立计时（Phase 1 已实现） | ✅ |
| ADR-2 C 工厂 | storage_ext.cpp/fileuri_ext.cpp 导出 `CreateFeature`/`DestroyFeature`（visibility default） | ✅ |
| ADR-4 插件分组 | storage/fileuri 归 Phase 2 独占候选 | ✅ |
| ADR-5 broker retrofit | file_permission_manager 的 fileuri 依赖由 IFileUriFeature 承担（不另建 broker_ext） | ✅（decision 见 task-04） |
| ADR-6 死依赖清理 | libupms 移除 storage_service + fileuri_native（Phase 2 新增 2 项；累计 Phase1+2 共 5 项） | ✅ |

### 8.5 遗留 / 风险 / 后续
- **AC-10 独占性实测**：storage / fileuri 是否 foundation 内独占 → task-07 `/proc/maps` 实测（设备环境）；非独占 descoped。
- **UT mock（AC-4）**：stub_impl 不再直连 IStorageManager、file_permission_manager 不再直连 FileUri → 单测需 mock IStorageShareFeature/IFileUriFeature（经框架 Acquire 注入 test 插件或接口抽象）。Phase 1 R-test 同款阻塞，待统一 mock 方案。
- **static lib (`libupms_static`)**：仍链 storage_service + fileuri_native + data_share（供既有测试）；待 UT mock 方案后清理。
- **death recipient 时序（storage）**：插件析构先 RemoveDeathRecipient 再 dlclose（已实现）；ASAN 待验证 storage SA 死亡不回调已卸载内存（AC-3）。
- **null path 语义（fileuri）**：插件未加载时 GetRealPathBySA 返回空 → policyInfo.path 空；需 UT 确认调用方空 path 处理与原语义一致（AC-2）。
- **Phase 3 未启动**：identity / sandbox / udmf 插件（共享库，零近期内存收益，模块化）。

> **Phase 2 结论**：broker/storage/fileuri 三任务代码全部落地 + 编译验证通过（libupms + 2 插件 + 符号导出 + 主库死依赖清理）。Phase 1 §7 结论中"未经编译验证"已被 §4 与本节 §8.3 推翻——Phase 1+2 均编译通过。进入 Stage 4 前仍需：UT mock 方案 + AC-10 独占实测 + Phase 3（可选模块化）。
