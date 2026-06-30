# Task 01 — 通用依赖插件框架骨架（Phase 1）

> item: `upms-deps-dynamic-load` | depends on: — | mode: 直接实现
> 规则来源：[design.md](../design.md) ADR-1/2/3/7、§5 代码点位；[spec.md](../spec.md) AC-2/3/5/6/8。本卡只含本任务上下文。

## 目标
新增通用插件框架：共享接口基类 + `DynamicFeatureManager`（注册表/状态机/活动计数/`Acquire` RAII 守卫/`Load`/`Unload`/`UnloadIdle`）+ ffrt 空闲延时任务 + stub_impl 活动 hook。框架可独立编译、单测。**本卡不含具体插件实现**（media 见 task-02）。

## 文件范围
- `interfaces/inner_api/uri_permission/feature/idynamic_feature.h`（新增，落点实现时确认）
- `services/uripermmgr/include/dynamic_feature_manager.h`（新增）
- `services/uripermmgr/src/dynamic_feature_manager.cpp`（新增）
- `services/uripermmgr/include/uri_permission_manager_stub_impl.h`（加活动 hook 相关）
- `services/uripermmgr/src/uri_permission_manager_stub_impl.cpp`（各 IPC 入口 hook）
- `services/uripermmgr/BUILD.gn`（+`ffrt:libffrt`、+`dynamic_feature_manager.cpp`）
- `test/unittest/uri_permission_impl_test/`（框架单测）

**不改**：对外 API、`IUriPermissionManager.idl`、sa_profile。

## 改动清单

### A. `idynamic_feature.h` — 接口基类（ADR-1/2）
```cpp
namespace OHOS::AAFwk {
class IDynamicFeature {  // 通用基类，所有插件 interface 继承它
public:
    virtual ~IDynamicFeature() = default;
};
// P1 只定义 media；其它 interface（ISandbox/IStorage/IFileUri/IBroker/IIdentity/IUdmf）
// 随 Phase 2/3 各任务加入。每插件 .so 导出：
// extern "C" IDynamicFeature* CreateFeature(); extern "C" void DestroyFeature(IDynamicFeature*);
}
```

### B. `dynamic_feature_manager.h` — 管理器 + RAII 守卫（ADR-1/3/7）
- 注册表项：`struct FeatureEntry { std::string soname; FeatureId id; void* handle=nullptr; IDynamicFeature* instance=nullptr; State state=NotLoaded; }`。
- `DynamicFeatureManager`（单例/DelayedSingleton，与仓内风格一致）：
  - `void Register(FeatureId id, const std::string& soname);`
  - `DynamicFeatureScope Acquire(FeatureId id);`（Load 若需 + activeCount++）
  - `template<typename IFace> IFace* DynamicFeatureScope::Get();`（static_cast 基类→IFace）
  - `void UnloadIdle();`（锁 + 等 activeCount==0 + 逐 Loaded 项 Destroy+dlclose + 置 NotLoaded）
  - 内部：`std::mutex mutex_; std::condition_variable cv_; std::atomic<int> activeCount_; std::map<FeatureId,FeatureEntry> registry_;` 延时：`std::optional<ffrt::task_handle> unloadHandle_; std::mutex timerMutex_;`
- `DynamicFeatureScope`（RAII）：构造 Acquire+activeCount++；析构 activeCount-- + 通知 cv；持 `DynamicFeatureManager*` + `FeatureId`。
- 常量：`UNLOAD_DELAY_TIME_US = 90000000`（90s）；任务名 `UPMS_FEATURE_UNLOAD_TASK`。

### C. `dynamic_feature_manager.cpp` — 实现
- `Load(entry)`：`dlopen(soname, RTLD_NOW)`；失败 `dlerror()` 记 TAG_LOGE；`dlsym("CreateFeature")` 造 instance；state=Loaded。
- `Unload(entry)`：`dlsym("DestroyFeature")`(或缓存) 调 destroy；`dlclose(handle)`；state=NotLoaded。
- `UnloadIdle()`：`std::unique_lock lk(mutex_); cv_.wait(lk, []{return activeCount_==0;});` 逐项 Unload。
- 延时任务（ADR-7）：`ArmUnload()`=`ffrt::submit_h(zeroCaptureLambdaUnloadIdle, {}, {}, ffrt::task_attr().delay(UNLOAD_DELAY_TIME_US).name(...))`；`CancelUnload()`=`ffrt::skip(handle)`。
- **零捕获 lambda**：`UnloadIdle` 经 manager 单例获取，不捕 this。
- 全程 hilog/hisysevent 记 Load/Unload 事件（AC-6）。

### D. stub_impl 活动 hook（ADR-7）
- 每个 public IPC 入口（Grant*/Check*/Revoke*/Verify*/Clear*）起始处调 `DynamicFeatureManager::GetInstance().ArmUnload()`（重挂延时：先 CancelUnload 再 Arm）。统一用宏 `UPMS_ACTIVITY_HOOK()` 避免遗漏。
- 实现期逐一核对 public 方法清单（参照 stub_impl.h 声明），确保穷尽（R5）。

### E. BUILD.gn
- `external_deps += ["ffrt:libffrt"]`。
- `libupms_sources += ["src/dynamic_feature_manager.cpp"]`。
- include_dirs 加 feature 头路径。

## 完成标准
- [ ] 框架三文件可编译（`./build.sh --build-target libupms` 过）。
- [ ] manager 单测：Register/Acquire/UnloadIdle 状态机、activeCount 归零才卸载、重 Load、延时 Arm/Cancel。
- [ ] RAII 守卫单测：作用域内指针有效、析构后 activeCount 递减。
- [ ] stub_impl 活动 hook 覆盖所有 public IPC 入口（清单核对）。
- [ ] 无对外 API 变更；现有 UT 不新增失败。

## 验证对应
AC-3（在途 IPC 安全：守卫）、AC-5（并发：activeCount）、AC-6（可观测）、AC-8（框架通用性）。
