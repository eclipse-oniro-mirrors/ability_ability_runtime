# G4 同族 API 模式化误用

> 来源：known-defect-patterns.md G4。

## 特征信号 / grep 线索

- `napi_open_handle_scope`、`napi_create_reference`、`napi_coerce_to_native_binding_object`、`napi_queue_async_work_with_qos`、`napi_wrap` 调用后不判断 scope 值与返回值。
- `iface_cast` 使用点（历史上单模块 500+ 处误用）。
- 智能指针手动 `get()` 后 delete / 托管内存被二次释放（double free）。

## 历史案例

- 同一 NAPI scope 误用在 js_ui_appearance / napi_context / napi_async_work_callback 等多文件重复出现 → 内存泄漏。
- `dbmsi` 两文件智能指针使用错误 → double free。
- `environment_callback.cpp` NAPI 引用泄漏 → Use-After-Free。

## 检查点

- 命中一处即 grep 全库同族 API，按文件分组列出。
- 该 API 是否有封装缺失（应加 wrapper 强制返回值检查）？
