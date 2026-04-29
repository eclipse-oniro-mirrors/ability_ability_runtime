# ModularObjectDispatcher C Demo

## 覆盖能力
- stub 侧通过 `fd` 回传 `tlb.json` 元数据（`code=0x00FFFF`）
- proxy 侧创建 `MoDispatcher`，查询 TypeDescriptor 全量信息
- 按方法名查询 `memID`，执行动态调用
- 基础类型调用：`add(i32, i32) -> i32`
- 复杂类型调用：`sumUserIds(array<struct UserInfo>) -> i32`

## 关键流程
1. `OH_IPCRemoteStub_Create` 创建服务 stub，并在 `OnRemoteRequest` 中处理：
   - `CODE_GET_TLB_FD`：写临时文件并通过 `OH_IPCParcel_WriteFileDescriptor` 返回
   - 业务 code：解析参数并返回 `Variant`
2. 客户端通过 `OH_AbilityRuntime_MoDispatcher_CreateInstance` 创建调度器
3. 调用 `GetTypeDescriptor`/`TypeDescriptor_*` 完成接口、方法、参数、结构体查询
4. `QueryMemIDsOfNames` 得到方法 `memID`
5. 构造 `Inputparams`，调用 `CallMethod` 完成动态调用

## 构建
在 GN 根目录执行：

```bash
gn gen out/default
ninja -C out/default modular_object_dispatcher_c_demo
```

目标 label：
`//foundation/ability/ability_runtime/test/demo/modular_object_dispatcher_c_demo:modular_object_dispatcher_c_demo`

## 预期输出示例
- `hasTypeDescriptor=1`
- `package=com.example.dispatcher version=1.0 mainService=IDataService`
- `method[0]: name=add ...`
- `method[1]: name=sumUserIds ...`
- `add result=12`
- `sumUserIds result=40`
