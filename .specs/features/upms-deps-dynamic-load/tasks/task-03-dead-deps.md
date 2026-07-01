# Task 03 — 死依赖清理（Phase 1）

> item: `upms-deps-dynamic-load` | depends on: task-01（可与 task-02 并行） | mode: 直接实现
> 规则来源：[design.md](../design.md) ADR-6、§5；[spec.md](../spec.md) AC-7。
> 取证：全量依赖审计（4 路 Explore）确认以下依赖在 UPMS 源码零直接调用。

## 目标
从 `ohos_shared_library("libupms")` 的 `external_deps` 移除零调用死依赖，逐个 build 验证，永久减链接/可能减运行时映射（条件性收益，作观测项）。

## 待移除清单（external_deps，`services/uripermmgr/BUILD.gn`）— 实际结果 2026-06-30
| # | 依赖 | 取证（零调用） | 实际结果 |
|---|------|------|------|
| 1 | `relational_store:native_rdb` | grep 0 include/0 API | ✅ 移除（Phase 1） |
| 2 | `relational_store:native_dataability` | 同上 | ✅ 移除（Phase 1） |
| 3 | `data_share:datashare_consumer` | 同上 | ✅ 移除（Phase 1） |
| 4 | `init:libbeget_proxy` | 0 直接调用（grep beget 零） | ✅ 移除 |
| 5 | `init:libbegetutil` | 0 直接调用（stub_impl 死 include `parameter.h`，无 API 调用，连 include 删） | ✅ 移除 |
| 6 | `common_event_service:cesfwk_core` | 0（不订阅公共事件） | ✅ 移除 |
| 7 | `common_event_service:cesfwk_innerkits` | 0 | ✅ 移除 |
| 8 | `background_task_mgr:bgtaskmgr_innerkits` | 0（不申请后台任务） | ✅ 移除 |
| 9 | `i18n:intl_util` | 0（graphics-gate，flag 开亦零引用） | ✅ 移除（整个 graphics 块删） |
| 10 | `ability_base:configuration` | 0（只用 want/zuri） | ✅ 移除 |
| 11 | `appkit_manager_helper`（内部 deps） | ⚠️ 原取证误（类名 `BundleMgrHelper` 非 `BundleManagerHelper`） | ❌ 保留：`file_uri_distribution_utils.cpp:162/277/287/294` 用 `BundleMgrHelper::GetApplicationInfo/GetBundleInfo/GetCloneBundleInfo/GetSandboxBundleInfo` |
| — | `eventhandler:libeventhandler` | 0（task-01 ffrt 替代延时） | ✅ 移除 |
| — | `storage_service:storage_manager_sa_proxy` | 0（Phase 2 task-05 迁插件） | ✅ 移除（Phase 2） |
| — | `app_file_service:fileuri_native` | 0（Phase 2 task-06 迁插件） | ✅ 移除（Phase 2） |

> 累计移除 13 项死依赖；保留 1 项（appkit_manager_helper，非死依赖）。libupms build success（2026-06-30）。

## 执行步骤（逐个，防一次性断裂）
1. 每次移除**一个**依赖。
2. `./build.sh --product-name rk3568 --build-target libupms` 验证通过。
3. 若失败（传递需要）：回退该项，在完成标准注明「保留，原因=被 X 传递需要」。
4. 全部处理完跑 `run -t UT -tp uripermmgr` 确认无回归。

## 完成标准
- [x] 清单 11 项 + eventhandler 逐个 build 验证，记录「移除成功」或「保留+原因」（见上表）。
- [x] libupms 编译通过（2026-06-30 build success）；UT 全绿待 mock 方案（AC-7）。
- [ ] 运行时收益观测：`/proc/<pid>/maps` 对比移除前后 foundation 内这些 .so 是否去映射（条件性，非硬门槛，待设备）。
- [x] 完成标准表填回实际结果（13 移除 / 1 保留）。

## 风险
- **R8 传递链接断裂**：逐个验证 + 失败即回退，风险可控。
- eventhandler 与 task-01 ffrt 选型耦合：确认 task-01 用 ffrt 后再移除 eventhandler。
