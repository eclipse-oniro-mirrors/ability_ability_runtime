# Task 05 — storage 插件 .so + 调用点改造（Phase 2）

> item: `upms-deps-dynamic-load` | depends on: task-01 | mode: 直接实现 | status: ✅ 完成（编译通过）
> 规则来源：[design.md](../design.md) ADR-1/2/4；[spec.md](../spec.md) AC-1/2/3/9/10。
> 把 UPMS 对 `storage_service`（`IStorageManager` 的 `CreateShareFile`/`DeleteShareFile`）的使用封装进独立插件 .so `libupms_storage_ext.z.so`；主库移除 `storage_service` 链接 + 调用改经 `Acquire`+`IStorageShareFeature`。

## 目标
分布式 docs URI 的 share-file 创建/删除经插件动态加载；插件持有 `IStorageManager` IPC proxy + death recipient；主库不再直链 `storage_service:storage_manager_sa_proxy`。

## 文件范围（实际）
- `services/uripermmgr/include/feature/istorage_share_feature.h`（新增）：`IStorageShareFeature : IDynamicFeature`，`CreateShareFile`/`DeleteShareFile`（vector 直传，序列化下沉插件）。
- `services/uripermmgr/plugins/storage_ext/`（新增目录）：
  - `storage_share_feature_impl.h/.cpp`：`StorageShareFeatureImpl`，含 `StorageDeathRecipient`（proxy 死亡清空）、`GetStorageManager`（samgr 取 SA + iface_cast + AddDeathRecipient）、`StringVecToRawData`（从 stub_impl 迁入）、`CreateShareFile`/`DeleteShareFile` 转发。析构前 `RemoveDeathRecipient`（防 dlclose 后悬空回调）。
  - `storage_ext.cpp`：`extern "C" visibility("default") CreateFeature/DestroyFeature`。
  - `BUILD.gn`：`ohos_shared_library("libupms_storage_ext")`，external_deps `storage_service:storage_manager_sa_proxy`，sanitize/cfi 与 libupms 一致。
- `services/uripermmgr/src/uri_permission_manager_stub_impl.cpp`：
  - `GrantBatchUriPermissionImpl`：`ConnectManager(storageManager_)`+`StringVecToRawData`+直调 → `Acquire(STORAGE).Get<IStorageShareFeature>()->CreateShareFile(uriVec,...)`，null 检查。
  - `DeleteShareFile`：同上改经插件。
  - 移除 `StringVecToRawData` 定义（迁插件）。
- `services/uripermmgr/include/uri_permission_manager_stub_impl.h`：
  - 移除 `#include "istorage_manager.h"`。
  - 移除 `sptr<StorageManager::IStorageManager> storageManager_` 成员（cpp 零引用，安全删）。
  - 移除 `void StringVecToRawData(...)` 声明（cpp 零调用，安全删）。
- `services/uripermmgr/BUILD.gn`：libupms external_deps 移除 `storage_service:storage_manager_sa_proxy`（死依赖清理，grep 确认主库 src 无 storage 残留引用）。
- `services/BUILD.gn`：ams_target 加 `uripermmgr/plugins/storage_ext:libupms_storage_ext`。
- `services/uripermmgr/src/uri_permission_manager_service.cpp`：`Init` 注册 `Register(FeatureId::STORAGE, "libupms_storage_ext.z.so")`。

## 完成标准
- [x] `libupms_storage_ext.z.so` 构建通过；libupms 不再直链 `storage_service`。
- [x] `CreateFeature`/`DestroyFeature` 为 GLOBAL `T` 符号（nm -D 验证）。
- [x] 主库 `src/*.cpp` 无 `istorage_manager`/`IStorageManager`/`StorageFileRawData`/`STORAGE_MANAGER_MANAGER_ID` 残留引用。
- [ ] storage share-file 路径功能正确（AC-1/2）—— 待 UT + 设备。
- [ ] AC-9 强一致 + AC-10 独占性（storage 在 foundation 内是否独占 → task-07 实测）。

## 规则映射
- ADR-1 通用框架：`Acquire(STORAGE)` + per-feature 状态机。
- ADR-2 C 工厂：`storage_ext.cpp` 导出 `CreateFeature`/`DestroyFeature`。
- ADR-4 插件分组：storage 归 Phase 2 独占候选。
- ADR-6 死依赖清理：libupms 移除 `storage_service:storage_manager_sa_proxy`。

## 验证
- libupms build success（2026-06-30，含 stub_impl 转发 + storage_service 移除 + 成员/声明删除）。
- libupms_storage_ext build success；`nm -D libupms_storage_ext.z.so`：`T CreateFeature` / `T DestroyFeature`。
- R-pkg：`bundle.json:136` module_list 含 `services:ams_target`，ams_target 含 storage_ext deps → 插件进镜像（不需额外改 bundle.json）。

## 遗留 / 风险
- **death recipient 与 dlclose 时序**：插件析构先 `RemoveDeathRecipient` 再 dlclose（已实现）；ASAN 需验证 storage SA 死亡时不回调已卸载插件内存（AC-3）。
- **AC-10 独占性**：storage_service 是否 foundation 内独占 → task-07 `/proc/maps` 实测；非独占 descoped。
- **UT mock**：stub_impl 不再直连 IStorageManager，单测需 mock `IStorageShareFeature`（经框架 Acquire 注入 test 插件或接口抽象）。
- **static lib (`libupms_static`)**：仍链 `storage_service`（供既有测试），待 UT mock 方案后一并清理。
