# Task 06 — fileuri 插件 .so + 调用点改造（Phase 2，DESCOPED）

> item: `upms-deps-dynamic-load` | depends on: task-01 | mode: 直接实现 | status: ❌ DESCOPED（2026-07-01 范围调整：只处理 media+storage，fileuri 暂不处理；已实现的 fileuri_ext 插件目录 + ifile_uri_feature.h + file_permission_manager retrofit 已全部回退，libupms 恢复直链 fileuri_native，service/BUILD.gn 移除 FILEURI 注册/插件 deps）
> 规则来源：[design.md](../design.md) ADR-1/2/4/5；[spec.md](../spec.md) AC-1/2/3/9/10。
> 承接 [task-04](task-04-broker-retrofit.md) broker retrofit：file_permission_manager 的 fileuri 依赖由本插件承担。

## 目标
把 UPMS 对 `app_file_service:fileuri_native`（`AppFileService::ModuleFileUri::FileUri::GetRealPathBySA`）的使用封装进独立插件 .so `libupms_fileuri_ext.z.so`；主库移除 `fileuri_native` 链接 + `file_permission_manager` 调用改经 `Acquire`+`IFileUriFeature`。

## 文件范围（实际）
- `services/uripermmgr/include/feature/ifile_uri_feature.h`（新增）：`IFileUriFeature : IDynamicFeature`，`GetRealPathBySA(uriString, bundleName)`。
- `services/uripermmgr/plugins/fileuri_ext/`（新增目录）：
  - `file_uri_feature_impl.h/.cpp`：`FileUriFeatureImpl`，`GetRealPathBySA` 逐行移植自原 `file_permission_manager.cpp`（`AppFileService::ModuleFileUri::FileUri` 构造 + `GetRealPathBySA`），语义零偏移。
  - `fileuri_ext.cpp`：`extern "C" visibility("default") CreateFeature/DestroyFeature`。
  - `BUILD.gn`：`ohos_shared_library("libupms_fileuri_ext")`，external_deps `app_file_service:fileuri_native` + `ability_base:zuri`，sanitize/cfi 与 libupms 一致。
- `services/uripermmgr/src/file_permission_manager.cpp`（task-04 落点）：
  - `GetPathPolicyInfoFromUri`：`AppFileService::ModuleFileUri::FileUri`+直调 → `Acquire(FILEURI).Get<IFileUriFeature>()->GetRealPathBySA(...)`，null 检查（null 时 path 空，policyInfo.path 空，调用方按失败处理）。
  - 移除 `#include "file_uri.h"`，加 `dynamic_feature_manager.h` + `feature/ifile_uri_feature.h`。
- `services/uripermmgr/BUILD.gn`：libupms external_deps 移除 `app_file_service:fileuri_native`。
- `services/BUILD.gn`：ams_target 加 `uripermmgr/plugins/fileuri_ext:libupms_fileuri_ext`。
- `services/uripermmgr/src/uri_permission_manager_service.cpp`：`Init` 注册 `Register(FeatureId::FILEURI, "libupms_fileuri_ext.z.so")`。
- `services/uripermmgr/include/feature/idynamic_feature.h`：`FeatureId` enum 加 `BROKER`/`STORAGE`/`FILEURI`/`IDENTITY`/`SANDBOX`/`UDMF`（Phase 2/3 占位，P2 用 STORAGE/FILEURI）。

## 完成标准
- [x] `libupms_fileuri_ext.z.so` 构建通过；libupms 不再直链 `app_file_service:fileuri_native`。
- [x] `CreateFeature`/`DestroyFeature` 为 GLOBAL `T` 符号（nm -D 验证）。
- [x] 主库 `src/*.cpp` 无 `#include "file_uri.h"` / `AppFileService::ModuleFileUri` 残留引用。
- [ ] fileuri real-path 解析功能正确（AC-1/2）—— 待 UT + 设备。
- [ ] AC-9 强一致 + AC-10 独占性（fileuri_native 在 foundation 内是否独占 → task-07 实测）。

## 规则映射
- ADR-1 通用框架：`Acquire(FILEURI)` + per-feature 状态机。
- ADR-2 C 工厂：`fileuri_ext.cpp` 导出 `CreateFeature`/`DestroyFeature`。
- ADR-4 插件分组：fileuri 归 Phase 2 独占候选。
- ADR-5 broker retrofit：file_permission_manager 的 fileuri 依赖动态化（本任务达成，替代独立 broker_ext）。
- ADR-6 死依赖清理：libupms 移除 `app_file_service:fileuri_native`。

## 验证
- libupms build success（2026-06-30，含 file_permission_manager 转发 + fileuri_native 移除）。
- libupms_fileuri_ext build success；`nm -D libupms_fileuri_ext.z.so`：`T CreateFeature` / `T DestroyFeature`。
- R-pkg：ams_target 含 fileuri_ext deps → 插件进镜像。

## 遗留 / 风险
- **null path 语义**：插件未加载/加载失败时 `GetRealPathBySA` 返回空串，`policyInfo.path` 空——需确认调用方（`CheckFileManagerUriPermission` 等）对空 path 的处理与原"FileUri 构造失败"语义一致（AC-2）。待 UT 覆盖。
- **AC-10 独占性**：fileuri_native 是否 foundation 内独占 → task-07 `/proc/maps` 实测。
- **UT mock**：file_permission_manager 测试需 mock `IFileUriFeature`（框架 Acquire 注入 test 插件）。
- **static lib (`libupms_static`)**：仍链 `fileuri_native`（供既有测试），待 UT mock 方案后清理。
