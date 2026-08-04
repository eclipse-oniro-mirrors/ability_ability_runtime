# G14 返回值与异常分支处理

> 来源：known-defect-patterns.md G14。

## 特征信号 / grep 线索

- `ReadParcelable` / `iface_cast` / `QueryAbilityInfo` / `GetBundleInfo` 等查询/转换接口返回值未判空即解引用。
- 异常分支（如 `bundleMgrHelper->QueryAbilityInfo` 失败）打印日志后返回 `true` / `ERR_OK`，而非错误码。
- `std::stoi` / `std::stol` / JSON 类型转换无 `try-catch` 兜底。
- NAPI 函数（`napi_create_reference`、`napi_open_handle_scope` 等）调用后未判断返回值和 scope 值。

## 历史案例

- `ability_manager_stub.cpp` 中 `ReadParcelable` 返回空指针未校验即解引用。
- `distribute_manager.cpp`、`match_manager.cpp` 存在空指针解引用。
- `bundleMgrHelper->QueryAbilityInfo` 失败分支打印日志但返回 `true` → 异常分支影响正常流程。
- `render_state_observer_proxy.cpp` 接口异常返回风险。
- `app_launch_data.cpp`、`app_mgr_proxy.cpp`、`app_mgr_service.cpp` 函数返回值逻辑错误，可能导致功能失效。
- `ability_manager_client_c.cpp` 空指针解引用。
- `cj_ability_delegators.cpp`、`user_controller.cpp`、`window_pid_visibility_changed_listener.cpp` 空指针解引用 / map 非法访问。
- `request_id_util.cpp` 整数上溢。

## 检查点

- 所有 `ReadParcelable` / `iface_cast` 调用点是否都有 `if (xxx == nullptr) return ERR_xxx` 保护？
- 异常分支是否返回明确的错误码，而非 `true` / `ERR_OK`？
- 类型转换（`stoi`、`asXXX`）是否有 `try-catch` 或类型前置校验？
- 命中一处即 grep 全库同族 API，按文件分组列出。
