# Task 02 — media 插件 .so + 调用点改造（Phase 1，框架首个用例）

> item: `upms-deps-dynamic-load` | depends on: task-01 | mode: 直接实现
> 规则来源：[design.md](../design.md) ADR-1/2/3/4/9/10、§5；[spec.md](../spec.md) AC-1/2/3/4。
> ⚠️ 取舍（design §8）：media 作为框架首个用例，比 broker 难（C++ SDK 静态单例 + wrapper .so + C 工厂）。media 独占性（AC-10）为本任务后置实测项（task-07）。

## 目标
把 UPMS 对 `media_library:media_permission_helper` 的使用封装进独立插件 .so `libupms_media_ext.z.so`：插件实现 `IMediaPermFeature`、链接 media_library、导出 C 工厂；UPMS 主库移除 media_library 链接 + media 调用改为经 `Acquire`+接口。media 路径功能与改造前一致。

## 现状取证（实现前需读确认）
- 调用点：[`media_permission_manager.cpp:45,68,99,138`](../../../../services/uripermmgr/src/media_permission_manager.cpp)（`GetMediaPermissionHelper`/`CheckPhotoUriPermission`/`GrantPhotoUriPermission`/`CancelPhotoUriPermission`）。
- 入口：`uri_permission_manager_stub_impl.cpp:603,1308,1650`（Grant/Check/Revoke 经 `MediaPermissionManager::GetInstance()`）。
- feature-gate：`ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE`；BUILD.gn:106-110 条件加入 media 源码 + external_deps。
- SDK 单例：`Media::MediaPermissionHelper::GetMediaPermissionHelper()` 返回静态单例（dlclose 安全命门，R2）。

## 文件范围
- `interfaces/inner_api/uri_permission/feature/idynamic_feature.h`（task-01 已建，本任务加 `IMediaPermFeature`）
- `services/uripermmgr/plugins/media_ext/`（新增目录）
  - `media_perm_feature_impl.h/.cpp`（实现 `IMediaPermFeature`）
  - `media_ext.cpp`（`extern "C" CreateFeature/DestroyFeature`）
  - `BUILD.gn`（`ohos_shared_library("libupms_media_ext")`，链 media_library，导出 C 符号）
- `services/uripermmgr/src/media_permission_manager.cpp`（调用改经接口）
- `services/uripermmgr/include/media_permission_manager.h`（签名调整）
- `services/uripermmgr/BUILD.gn`（media 源码迁插件；libupms 移除 media_library external_deps；+media_ext 依赖关系）

## 改动清单

### A. `IMediaPermFeature`（ADR-1/2）
```cpp
class IMediaPermFeature : public IDynamicFeature {
public:
    virtual int32_t CheckPhotoUriPermission(const std::vector<std::string>& uris, uint32_t callerToken,
        uint32_t targetToken, std::vector<bool>& results) = 0;
    virtual int32_t GrantPhotoUriPermission(const std::vector<std::string>& uris, uint32_t callerToken,
        uint32_t targetToken) = 0;  // 签名以现状 media_permission_manager.cpp 实际参数为准
    virtual int32_t CancelPhotoUriPermission(uint32_t targetToken) = 0;
};
```
（精确签名实现期对照 `media_permission_manager.cpp` 现状确认。）

### B. 插件实现 `libupms_media_ext.z.so`（ADR-2/4）
- `MediaPermFeatureImpl : public IMediaPermFeature`：方法内转发到 `Media::MediaPermissionHelper::GetMediaPermissionHelper()`（把 media_permission_manager.cpp 现有调用代码搬入）。
- `media_ext.cpp`：`extern "C" IDynamicFeature* CreateFeature(){ return new MediaPermFeatureImpl(); }` / `extern "C" void DestroyFeature(IDynamicFeature* p){ delete p; }`。
- BUILD.gn：`ohos_shared_library("libupms_media_ext")`，`external_deps += ["media_library:media_permission_helper"]`，sanitize/cfi 与 libupms 一致，导出 CreateFeature/DestroyFeature（`-Wl,--export-dynamic` 或符号可见性配置）。

### C. UPMS 调用点改造（ADR-3）
- `MediaPermissionManager` 内调用改为：
  ```cpp
  auto scope = DynamicFeatureManager::GetInstance().Acquire(FeatureId::MEDIA);
  auto* f = scope.Get<IMediaPermFeature>();
  if (f == nullptr) return ERR_INVALID_VALUE;  // 或对应错误码
  return f->CheckPhotoUriPermission(...);
  ```
- 调用全程在 `scope` 守卫作用域内；不跨作用域持 `f`。
- `MediaPermissionManager` 不再 include `media_permission_helper.h`、不再直连 media_library。

### D. BUILD.gn 调整
- libupms：移除 `ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE` 分支里的 `media_permission_manager.cpp` 源码（迁插件）与 `media_library:media_permission_helper` external_deps。
- media_permission_manager.cpp 保留在 libupms（作为接口转发薄层）**或**整体迁入插件——实现期定（倾向保留薄层在 libupms，仅调用经接口；插件持有真实 SDK 调用）。**实现期确认**：确保 stub_impl.cpp:603/1308/1650 经 `MediaPermissionManager` 仍可用。
- feature-gate：flag 关时 `IMediaPermFeature` 不构建、路径返回 `CAPABILITY_NOT_SUPPORT(801)`（现状语义，ADR-9）。

### E. 注册（ADR-1）
- UPMS Init/单例构造时 `DynamicFeatureManager::GetInstance().Register(FeatureId::MEDIA, "libupms_media_ext.z.so");`（仅 flag 开时）。

## 完成标准
- [ ] `libupms_media_ext.z.so` 构建通过；libupms 不再直链 media_library。
- [ ] media URI（`file://media/`）Grant/Check/Revoke 经接口功能正确（AC-4）。
- [ ] 卸载态注入 media IPC → `Acquire`→Load→正确处理，无 crash（AC-3，ASAN）。
- [ ] 卸载/重载周期 media 路径授权状态一致（AC-2）。
- [ ] **后置 task-07**：`/proc/maps` 实测 media 独占性（AC-10）；非独占 → descoped 决策。

## 风险
- **R2 SDK 单例**：media_permission_helper 静态单例在插件 dlclose 后随之析构；确保无跨守卫引用、UnloadIdle 时 activeCount==0。ASAN 验证。
- **签名不一致**：IMediaPermFeature 签名须与现状 media_permission_manager 完全对齐，避免语义偏移。
- **R1 独占性**：media 可能在 foundation 内被其它子系统 SA 共享 → task-07 实测定夺。
