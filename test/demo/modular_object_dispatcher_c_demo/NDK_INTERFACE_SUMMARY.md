# ModularObjectDispatcher NDK 接口分类与调用顺序

## 1. 接口分类

### 1.1 Dispatcher 核心（对象生命周期与调用）
- `OH_AbilityRuntime_MoDispatcher_CreateInstance`
- `OH_AbilityRuntime_MoDispatcher_Release`
- `OH_AbilityRuntime_MoDispatcher_HasTypeDescriptor`
- `OH_AbilityRuntime_MoDispatcher_GetTypeDescriptor`
- `OH_AbilityRuntime_MoDispatcher_QueryMemIDsOfNames`
- `OH_AbilityRuntime_MoDispatcher_CallMethod`

### 1.2 TypeDescriptor 元数据查询（包/接口/枚举/结构体/成员）
- `OH_AbilityRuntime_TypeDescriptor_Release`
- `OH_AbilityRuntime_TypeDescriptor_GetPackage`
- `OH_AbilityRuntime_TypeDescriptor_GetVersion`
- `OH_AbilityRuntime_TypeDescriptor_GetMemIDsOfNames`
- `OH_AbilityRuntime_TypeDescriptor_GetMemberName`
- `OH_AbilityRuntime_TypeDescriptor_GetInterfaceCount`
- `OH_AbilityRuntime_TypeDescriptor_GetInterfaceName`
- `OH_AbilityRuntime_TypeDescriptor_GetInterfaceMemberId`
- `OH_AbilityRuntime_TypeDescriptor_GetInterfaceDescriptor`
- `OH_AbilityRuntime_TypeDescriptor_GetInterfaceIsCallback`
- `OH_AbilityRuntime_TypeDescriptor_GetMainServiceInterfaceName`
- `OH_AbilityRuntime_TypeDescriptor_GetEnumCount`
- `OH_AbilityRuntime_TypeDescriptor_GetEnumName`
- `OH_AbilityRuntime_TypeDescriptor_GetEnumMemberId`
- `OH_AbilityRuntime_TypeDescriptor_GetEnumEnumValueCount`
- `OH_AbilityRuntime_TypeDescriptor_GetEnumValueName`
- `OH_AbilityRuntime_TypeDescriptor_GetEnumValue`
- `OH_AbilityRuntime_TypeDescriptor_GetStructCount`
- `OH_AbilityRuntime_TypeDescriptor_GetStructName`
- `OH_AbilityRuntime_TypeDescriptor_GetStructMemberId`
- `OH_AbilityRuntime_TypeDescriptor_GetStructFieldCount`
- `OH_AbilityRuntime_TypeDescriptor_GetStructFieldName`
- `OH_AbilityRuntime_TypeDescriptor_GetStructFieldType`

### 1.3 InterfaceDescriptor 方法签名查询（基于 descriptor JSON）
- `OH_AbilityRuntime_TypeDescriptor_GetMethodCount`
- `OH_AbilityRuntime_TypeDescriptor_GetMethodName`
- `OH_AbilityRuntime_TypeDescriptor_GetMethodMemberId`
- `OH_AbilityRuntime_TypeDescriptor_GetMethodReturnType`
- `OH_AbilityRuntime_TypeDescriptor_GetMethodParamCount`
- `OH_AbilityRuntime_TypeDescriptor_GetMethodParamType`
- `OH_AbilityRuntime_TypeDescriptor_GetMethodParamName`

### 1.4 复杂类型容器接口
- `Array_*`：创建、读写、扩容、释放
- `Vector_*`：创建、追加、读写、清空、释放
- `Set_*`：创建、增删、包含判断、清空、释放
- `Map_*`：创建、Put/Get、Contains、清空、释放
- `Struct_*`：创建、字段读写、释放

## 2. 推荐调用顺序

1. `CreateInstance(remoteProxy)`
2. `HasTypeDescriptor`（可选预检查）
3. `GetTypeDescriptor`（首次触发 IPC 获取元数据）
4. 通过 `TypeDescriptor_*` / `GetMethod*` 查询接口、方法、参数、复杂类型信息
5. `QueryMemIDsOfNames` 将方法名映射到 `memID`
6. 构造 `Variant[] + Inputparams`
7. `CallMethod(memID, input, result)`
8. 释放复杂类型句柄、TypeDescriptor、Dispatcher

## 3. 首次调用时序（关键）

- `GetTypeDescriptor` -> MetadataManager `EnsureLoaded`
- `EnsureLoaded` 通过 `code=0x00FFFF` 向 stub 请求 tlb 元数据
- 优先读取 `replyParcel` 中的 `fd`，再回退字符串读取
- 解析 JSON，建立 `name <-> memID` / `memID -> method(ipcCode,type)` 映射
- `CallMethod` 时按映射做参数编解码并发起业务 IPC
