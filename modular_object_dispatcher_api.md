# Modular Object Dispatcher NDK 接口梳理

源文件：`modular_object_dispatcher.cpp`

## 一、接口分类总览

### 1. MoDispatcher 生命周期

| 接口 | 行号 | 说明 |
|------|------|------|
| `OH_AbilityRuntime_MoDispatcher_CreateInstance` | 42 | 从 IPC RemoteProxy 创建 MoDispatcher 实例 |
| `OH_AbilityRuntime_MoDispatcher_Release` | 59 | 释放 MoDispatcher 实例 |

### 2. TypeDescriptor 生命周期

| 接口 | 行号 | 说明 |
|------|------|------|
| `OH_AbilityRuntime_MoDispatcher_HasTypeDescriptor` | 68 | 检查远端是否有类型描述元数据 |
| `OH_AbilityRuntime_MoDispatcher_GetTypeDescriptor` | 83 | 获取 TypeDescriptor 句柄 |
| `OH_AbilityRuntime_TypeDescriptor_Release` | 186 | 释放 TypeDescriptor |

### 3. TypeDescriptor 元数据查询（接口/方法/参数）

| 接口 | 行号 | 说明 |
|------|------|------|
| `OH_AbilityRuntime_TypeDescriptor_GetVersion` | 195 | 获取元数据版本 |
| `OH_AbilityRuntime_TypeDescriptor_GetInterfaceCount` | 209 | 获取接口数量 |
| `OH_AbilityRuntime_TypeDescriptor_GetInterfaceName` | 218 | 按索引获取接口名 |
| `OH_AbilityRuntime_TypeDescriptor_GetInterfaceIsCallback` | 232 | 判断接口是否为回调类型 |
| `OH_AbilityRuntime_TypeDescriptor_GetMainServiceInterfaceName` | 241 | 获取主服务接口名 |
| `OH_AbilityRuntime_TypeDescriptor_GetMethodCount` | 255 | 获取某接口的方法数 |
| `OH_AbilityRuntime_TypeDescriptor_GetMethodName` | 265 | 按索引获取方法名 |
| `OH_AbilityRuntime_TypeDescriptor_GetMethodMemberId` | 280 | 根据方法名获取 memID |
| `OH_AbilityRuntime_TypeDescriptor_GetMethodReturnType` | 291 | 获取方法返回类型 |
| `OH_AbilityRuntime_TypeDescriptor_GetMethodParamCount` | 303 | 获取方法参数数量 |
| `OH_AbilityRuntime_TypeDescriptor_GetMethodParamType` | 314 | 按索引获取参数类型 |
| `OH_AbilityRuntime_TypeDescriptor_GetMethodParamName` | 327 | 按索引获取参数名 |

### 4. TypeDescriptor 枚举类型查询

| 接口 | 行号 | 说明 |
|------|------|------|
| `OH_AbilityRuntime_TypeDescriptor_GetEnumCount` | 344 | 枚举数量 |
| `OH_AbilityRuntime_TypeDescriptor_GetEnumName` | 353 | 按索引获取枚举名 |
| `OH_AbilityRuntime_TypeDescriptor_GetEnumEnumValueCount` | 367 | 某枚举的值数量 |
| `OH_AbilityRuntime_TypeDescriptor_GetEnumValueName` | 376 | 按索引获取枚举值名 |
| `OH_AbilityRuntime_TypeDescriptor_GetEnumValue` | 391 | 根据枚举值名获取 int 值 |

### 5. TypeDescriptor 结构体类型查询

| 接口 | 行号 | 说明 |
|------|------|------|
| `OH_AbilityRuntime_TypeDescriptor_GetStructCount` | 401 | 结构体数量 |
| `OH_AbilityRuntime_TypeDescriptor_GetStructName` | 410 | 按索引获取结构体名 |
| `OH_AbilityRuntime_TypeDescriptor_GetStructFieldCount` | 424 | 某结构体的字段数 |
| `OH_AbilityRuntime_TypeDescriptor_GetStructFieldName` | 433 | 按索引获取字段名 |
| `OH_AbilityRuntime_TypeDescriptor_GetStructFieldType` | 448 | 获取字段类型 |

### 6. 方法调用

| 接口 | 行号 | 说明 |
|------|------|------|
| `OH_AbilityRuntime_MoDispatcher_QueryMainServiceInterfaceMemIDsOfNames` | 104 | 批量查询方法名对应的 memID |
| `OH_AbilityRuntime_MoDispatcher_CallMethod` | 120 | 通过 memID 进行 IPC 方法调用 |

### 7. Variant 清理

| 接口 | 行号 | 说明 |
|------|------|------|
| `OH_AbilityRuntime_MoDispatcher_Variant_Clear` | 181 | 清理 Variant 中持有的资源 |

### 8. 复杂类型容器（Array / Vector / List / Set / Map / Struct）

每种容器均提供 `Create -> 操作(Get/Set/Add/...) -> Release` 的模式：

| 容器 | Create | 主要操作 | Release | 行号范围 |
|------|--------|----------|---------|----------|
| **Array** | `Array_Create` | Get/Set/GetSize/Resize | `Array_Release` | 458–490 |
| **Vector** | `Vector_Create` | Add/Get/GetSize/Clear | `Vector_Release` | 492–522 |
| **List** | `List_Create` | PushFront/Back, PopFront/Back, Insert/Remove/Get/GetSize/Clear | `List_Release` | 524–572 |
| **Set** | `Set_Create` | Add/Remove/Contains/GetSize/GetAt/Clear | `Set_Release` | 574–611 |
| **Map** | `Map_Create` | Put/Get/Remove/ContainsKey/GetSize/GetKeyAt/GetValueAt/Clear | `Map_Release` | 613–667 |
| **Struct** | `Struct_Create` | SetField/GetField/GetName | `Struct_Release` | 669–684 |

---

## 二、典型调用顺序

```
阶段1: 创建 MoDispatcher
│
├── OH_AbilityRuntime_MoDispatcher_CreateInstance(proxy, &handle)
│
├── OH_AbilityRuntime_MoDispatcher_HasTypeDescriptor(handle, &hasType)
│       确认远端支持类型描述
│
└── OH_AbilityRuntime_MoDispatcher_GetTypeDescriptor(handle, &typeDescriptor)

阶段2: 元数据查询 (TypeDescriptor)
│
├── TypeDescriptor_GetVersion()
├── TypeDescriptor_GetInterfaceCount() -> 循环 GetInterfaceName()
├── TypeDescriptor_GetMainServiceInterfaceName()
├── TypeDescriptor_GetMethodCount(iface) -> 循环 GetMethodName()
├── TypeDescriptor_GetMethodMemberId(iface, method, &memID)
├── TypeDescriptor_GetMethodParamCount / ParamType / ParamName()
├── TypeDescriptor_GetMethodReturnType()
├── TypeDescriptor_GetEnumCount / EnumName / EnumValue...
└── TypeDescriptor_GetStructCount / StructName / StructField...

阶段3: 准备参数 & 调用方法
│
├── 方式A: 通过元数据查询路径
│   QueryMainServiceInterfaceMemIDsOfNames(names, &memID)
│
├── 方式B: 快捷查询
│   TypeDescriptor_GetMethodMemberId(iface, method, &memID)
│
├── 构造 InputParams（可能包含复杂类型）
│   ├── Array_Create -> Array_Set -> ... -> Array_Release
│   ├── Vector_Create -> Vector_Add -> ... -> Vector_Release
│   ├── List_Create -> List_PushBack -> ... -> List_Release
│   ├── Set_Create -> Set_Add -> ... -> Set_Release
│   ├── Map_Create -> Map_Put -> ... -> Map_Release
│   └── Struct_Create -> Struct_SetField -> Struct_Release
│
├── CallMethod(handle, memID, inputParams, &result)
│
├── Variant_Clear(&result)              // 清理返回值
└── Variant_Clear(&inputParams中的Variant)  // 清理输入参数

阶段4: 释放资源
│
├── TypeDescriptor_Release(&typeDescriptor)
└── MoDispatcher_Release(&handle)
```

---

## 三、核心流程说明

1. **创建**：`CreateInstance` 接收一个 `OHIPCRemoteProxy*`，内部创建 `MoDispatcher` 并初始化 `MetadataManager`（懒加载，首次使用时通过 IPC 从远端拉取元数据）

2. **元数据驱动**：所有 `EnsureLoaded` 调用都会触发一次 IPC 获取远端类型描述信息，后续调用走缓存。`TypeDescriptor` 系列接口用于反射式查询远端服务支持哪些接口、方法、参数类型

3. **方法调用**：`CallMethod` 是核心 IPC 调用——根据 memID 查到 `MethodMeta`（含 ipcCode），校验参数，序列化后通过 `SendRequest` 同步 IPC，最后反序列化返回值

4. **复杂类型**：Array/Vector/List/Set/Map/Struct 六种容器用于构造方法的输入参数或解析返回值，与 `Variant` 配合使用

5. **释放**：所有 `*_Release` 接口采用二级指针模式（`Handle*`），释放后置 nullptr
