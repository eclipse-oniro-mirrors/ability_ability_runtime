# Task 04 — broker retrofit（Phase 2，DESCOPED）

> item: `upms-deps-dynamic-load` | depends on: task-01 | mode: 直接实现 | status: ❌ DESCOPED（2026-07-01 范围调整：只处理 media+storage，fileuri/broker 及其 retrofit 暂不处理；已实现的 file_permission_manager IFileUriFeature retrofit 已回退为直调 FileUri）
> 规则来源：[design.md](../design.md) ADR-5；[spec.md](../spec.md) AC-9。
> ⚠️ 实现期 decision：原计划建独立 `plugins/broker_ext/` + 改造 `file_permission_manager.cpp`。实际评估后，`file_permission_manager` 的可动态化依赖仅 `app_file_service:fileuri_native`（`GetRealPathBySA`），无独立 broker 逻辑需封装。故 **不另建 broker_ext**，该依赖直接由 [task-06](task-06-fileuri-plugin.md) 的 `IFileUriFeature` 插件承担。ADR-5 "broker retrofit" 由 fileuri 插件达成。

## 目标（实际落点）
把 `file_permission_manager` 作为路径策略 broker 对 `fileuri_native` 的依赖动态化——`GetPathPolicyInfoFromUri` 经 `Acquire(FILEURI).Get<IFileUriFeature>()` 取 real path，主库不再直链 `app_file_service:fileuri_native`。

## 文件范围（实际）
- `services/uripermmgr/src/file_permission_manager.cpp`：`GetPathPolicyInfoFromUri` 改为经 `IFileUriFeature`；移除 `#include "file_uri.h"`，加 `dynamic_feature_manager.h` + `feature/ifile_uri_feature.h`。
- 调用点改造与 fileuri 插件实现在 [task-06](task-06-fileuri-plugin.md) 一并完成。

## 完成标准
- [x] `file_permission_manager.cpp` 不再 include `file_uri.h`、不再直连 `AppFileService::ModuleFileUri::FileUri`。
- [x] libupms 不再 external_deps `app_file_service:fileuri_native`（迁 fileuri 插件）。
- [x] libupms 编译通过（2026-06-30）。
- [ ] AC-9 授权状态强一致：卸载/重载周期 fileuri 路径策略一致（待 UT + ASAN）。

## 验证
- libupms build success（含 `file_permission_manager.cpp` 改造 + `fileuri_native` external_deps 移除）。
- grep 确认主库 `src/*.cpp` 无 `#include "file_uri.h"` / `AppFileService::ModuleFileUri` / `fileuri_native` 残留。

## 遗留
- AC-9 强一致 + AC-10 独占性（fileuri 在 foundation 内是否独占 → task-07 实测）待设备验证。
