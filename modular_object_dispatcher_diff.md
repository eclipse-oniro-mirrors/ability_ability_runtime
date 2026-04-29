# ModularObjectDispatcher 功能增强 - 复合类型支持变更说明

## 概述

本次修改为 ModularObjectDispatcher 增加了复合类型（complex type）的完整支持，包括：
- tlb.json 中 `type_info` 支持 `type`、`key_type`、`value_type`、`idl_type` 字段
- 支持嵌套复杂类型（如 `Map<i32, Map<i32, i64>>`）
- `MoInterfaceMeta` 中 `isCallback`/`isMainService` 合并为 `interface_type` 字段
- 新增 `OH_AbilityRuntime_MoDispatcher_TypeInfo` 类型和 `VT_ENUM` 枚举值
- 新增容器索引访问接口（`Set_GetAt`、`Map_GetKeyAt`、`Map_GetValueAt`）
- 新增 `Variant_Clear` 资源释放接口
- TLB JSON 解析增加严格校验

---

## 一、NDK 接口头文件变更

**文件**: `interfaces/kits/c/ability_runtime/modular_object_dispatcher.h`  
**文件**: `interface/sdk_c/AbilityKit/ability_runtime/modular_object_dispatcher.h`

### 1.1 新增 `VT_ENUM` 枚举值

| 变更前 | 变更后 |
|--------|--------|
| 枚举到 `VT_IPC_REMOTE_STUB = 20` 结束 | 新增 `VT_IPC_REMOTE_STUB = 20`, `VT_ENUM = 21` |

### 1.2 新增 `OH_AbilityRuntime_MoDispatcher_TypeInfo` 类型

**新增**：用于描述参数/返回值类型的递归结构体。

```c
typedef struct OH_AbilityRuntime_MoDispatcher_TypeInfo {
    OH_AbilityRuntime_MoDispatcher_ValueType vt;
    union {
        struct {
            OH_AbilityRuntime_MoDispatcher_ValueType keyType;
            struct OH_AbilityRuntime_MoDispatcher_TypeInfo *pValueType;
        } mapType;
        struct OH_AbilityRuntime_MoDispatcher_TypeInfo *pElementType;
        char* idlType;
    } u;
} OH_AbilityRuntime_MoDispatcher_TypeInfo;
```

### 1.3 Variant 新增 `enumVal` 字段

```c
// union 中新增：
int32_t enumVal;  // Enum value
```

### 1.4 新增函数

| 函数 | 说明 |
|------|------|
| `MoDispatcher_Variant_Clear` | 释放 Variant 持有的资源（字符串、容器句柄等） |
| `Set_GetAt` | 按索引获取 Set 元素 |
| `Map_GetKeyAt` | 按索引获取 Map 的 key |
| `Map_GetValueAt` | 按索引获取 Map 的 value |

### 1.5 函数签名变更

| 函数 | 变更前 | 变更后 |
|------|--------|--------|
| `Array_Create` | `(Vt elementType, uint32_t size, ...)` | `(TypeInfo* elementType, uint32_t size, ...)` |
| `Array_GetElementType` | `(handle, Vt*)` | `(handle, TypeInfo*)` |
| `Vector_Create` | `(Vt elementType, ...)` | `(TypeInfo* elementType, ...)` |
| `Vector_GetElementType` | `(handle, Vt*)` | `(handle, TypeInfo*)` |
| `Set_Create` | `(Vt elementType, SetHandle**)` | `(TypeInfo* elementType, SetHandle*)` |
| `Set_GetElementType` | `(handle, Vt*)` | `(handle, TypeInfo*)` |
| `Map_Create` | `(Vt keyType, Vt valueType, ...)` | `(ValueType keyType, TypeInfo* valueType, ...)` |
| `Map_GetValueType` | `(handle, Vt*)` | `(handle, TypeInfo*)` |
| `GetMethodReturnType` | `(desc, iface, method, Vt*)` | `(desc, iface, method, TypeInfo*)` |
| `GetMethodParamType` | `(desc, iface, method, index, Vt*)` | `(desc, iface, method, index, TypeInfo*)` |
| `GetStructFieldType` | `(td, struct, field, Vt*)` | `(td, struct, field, TypeInfo*)` |

---

## 二、内部类型定义变更

**文件**: `frameworks/c/ability_runtime/include/mo_dispatcher_types.h`

### 2.1 新增 `MoInterfaceType` 枚举

```cpp
enum class MoInterfaceType : uint32_t {
    NORMAL = 0,      // 普通接口
    MAIN_SERVICE = 1, // 主服务接口
    CALLBACK = 2,     // 回调接口
};
```

### 2.2 新增 `MoTypeInfo` 结构体

```cpp
struct MoTypeInfo {
    ValueType vt;
    ValueType mapKeyType;               // map 的 key 类型（简单类型）
    shared_ptr<MoTypeInfo> pMapValueType;  // map 的 value 类型
    shared_ptr<MoTypeInfo> pElementType;   // array/vector/set 的元素类型
    string idlType;                     // enum/interface/struct 的类型名

    TypeInfo* ToCTypeInfo() const;      // 转换为 C TypeInfo
    static shared_ptr<MoTypeInfo> FromCTypeInfo(const TypeInfo*); // 从 C TypeInfo 构建
};
```

### 2.3 `MoInterfaceMeta` 变更

| 字段 | 变更前 | 变更后 |
|------|--------|--------|
| `isCallback` | `bool isCallback` | 已移除 |
| `isMainService` | `bool isMainService` | 已移除 |
| `interfaceType` | 无 | `MoInterfaceType interfaceType = NORMAL` |

新增辅助方法：
```cpp
bool IsCallback() const { return interfaceType == MoInterfaceType::CALLBACK; }
bool IsMainService() const { return interfaceType == MoInterfaceType::MAIN_SERVICE; }
```

### 2.4 `MoStructFieldMeta` / `MoMethodParamMeta` / `MoMethodMeta` 类型变更

| 字段 | 变更前 | 变更后 |
|------|--------|--------|
| `MoStructFieldMeta.type` | `Vt type` | `shared_ptr<MoTypeInfo> typeInfo` |
| `MoMethodParamMeta.type` | `Vt type` | `shared_ptr<MoTypeInfo> typeInfo` |
| `MoMethodMeta.returnType` | `Vt returnType` | `shared_ptr<MoTypeInfo> returnType` |

### 2.5 容器结构体变更

| 结构体 | 变更前 | 变更后 |
|--------|--------|--------|
| `Array` | `Vt elementType` | `shared_ptr<MoTypeInfo> elementTypeInfo` |
| `Vector` | `Vt elementType` | `shared_ptr<MoTypeInfo> elementTypeInfo` |
| `Set` | `Vt elementType` | `shared_ptr<MoTypeInfo> elementTypeInfo` |
| `Map` | `Vt keyType, Vt valueType` | `ValueType keyType, shared_ptr<MoTypeInfo> valueTypeInfo` |
| `Struct` | `unordered_map<string, Vt> fieldTypes` | `unordered_map<string, shared_ptr<MoTypeInfo>> fieldTypes` |

---

## 三、元数据管理器变更

**文件**: `frameworks/c/ability_runtime/include/mo_dispatcher_metadata_manager.h`  
**文件**: `frameworks/c/ability_runtime/src/mo_dispatcher_metadata_manager.cpp`

### 3.1 ParseTypeInfo 增强

| 变更前 | 变更后 |
|--------|--------|
| `ParseTypeInfo(string) -> Vt` | `ParseTypeInfo(json) -> shared_ptr<MoTypeInfo>` (递归解析嵌套类型) |
| 只支持简单类型名 | 支持 map/array/vector/set/enum/interface/struct 嵌套 |

**嵌套类型解析逻辑**：
- `type == "map"` → 解析 `key_type`（必须为简单类型）和 `value_type`（递归）
- `type == "array"/"vector"/"set"` → 解析 `value_type`（递归）
- `type == "enum"/"interface"/"struct"` → 解析 `idl_type`

### 3.2 ParseMetadata 变更

#### interface_type 解析

```cpp
// 变更前：
interfaceMeta.isCallback = interfaceObj.value("is_callback", false);
interfaceMeta.isMainService = interfaceObj.value("is_main_service", false);

// 变更后：
interfaceMeta.interfaceType = static_cast<MoInterfaceType>(interfaceObj.value("interface_type", 0u));
// 校验 interface_type 必须为 0/1/2，否则返回 TLB_METADATA_INVALID
```

#### dispID 读取修复

```cpp
// 变更前（Bug）：读取 "memID"
uint32_t memId = obj["memID"].get<uint32_t>();

// 变更后（修复）：读取 "dispID"
uint32_t dispId = obj["dispID"].get<uint32_t>();
```

#### dispID 唯一性校验（新增）

解析过程中收集所有 dispID 到 `unordered_set<uint32_t> usedIds`，发现重复则返回 `TLB_METADATA_INVALID`。

#### 类型解析使用完整 TypeInfo

```cpp
// 变更前：
method.returnType = ParseTypeInfo(methodObj["return_type"].value("type", ""));
param.type = ParseTypeInfo(paramObj["type_info"].value("type", ""));

// 变更后：
method.returnType = ParseTypeInfo(methodObj["return_type"]);
param.typeInfo = ParseTypeInfo(paramObj["type_info"]);
```

### 3.3 新增 idl_type 校验

解析完成后验证所有 `idl_type` 引用的类型在 tlb.json 中有声明：
- enum 类型的 idl_type 必须在 enums 中存在
- struct 类型的 idl_type 必须在 structs 中存在
- interface 类型的 idl_type 必须在 interfaces 中存在

不满足则返回 `TLB_METADATA_INVALID`。

### 3.4 方法签名变更

| 方法 | 变更前 | 变更后 |
|------|--------|--------|
| `GetMethodReturnTypeFromDescriptor` | 最后参数 `Vt*` | 最后参数 `TypeInfo*` |
| `GetMethodParamTypeFromDescriptor` | 最后参数 `Vt*` | 最后参数 `TypeInfo*` |
| `GetStructFieldType` | 最后参数 `Vt*` | 最后参数 `TypeInfo*` |

---

## 四、复杂类型管理器变更

**文件**: `frameworks/c/ability_runtime/include/mo_dispatcher_complex_type_manager.h`  
**文件**: `frameworks/c/ability_runtime/src/mo_dispatcher_complex_type_manager.cpp`

### 4.1 函数签名变更

所有 Create 和 GetElementType/GetValueType 函数从 `Vt` 改为 `TypeInfo*`。

### 4.2 新增接口

| 函数 | 说明 |
|------|------|
| `Variant_Clear` | 释放 Variant 资源（free 字符串、Release 容器句柄） |
| `SetGetAt` | 按索引返回 Set 元素的深拷贝 |
| `MapGetKeyAt` | 按索引返回 Map key 的深拷贝 |
| `MapGetValueAt` | 按索引返回 Map value 的深拷贝 |

### 4.3 Set_Create 签名修复

```cpp
// 变更前（有缺陷的双重指针）：
SetCreate(Vt elementType, SetHandle** ppSet)

// 变更后（单指针，与其他容器一致）：
SetCreate(TypeInfo* elementType, SetHandle* ppSet)
```

---

## 五、参数编解码器变更

**文件**: `frameworks/c/ability_runtime/src/mo_dispatcher_param_codec.cpp`

### 5.1 VT_ENUM 序列化支持

WriteVariant 新增：
```cpp
case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ENUM:
    return CheckWrite(parcel.WriteInt32(value->u.enumVal));
```

ReadVariant 新增：
```cpp
case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ENUM:
    value->u.enumVal = parcel.ReadInt32();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
```

### 5.2 类型校验适配

ValidateInputParams 从 `MoTypeInfo` 提取顶层 `vt` 进行校验。

---

## 六、tlb.json 格式变更

**文件**: `foundation/ability/ability_runtime/tlb.json`

### 6.1 interface_type 替代 isCallback/isMainService

```json
// 变更前：
{
    "name": "ICalculator",
    "is_callback": true,
    "is_main_service": false
}

// 变更后：
{
    "name": "ICalculator",
    "interface_type": 2  // 0=normal, 1=mainservice, 2=callback
}
```

### 6.2 复杂类型示例

```json
{
    "name": "val",
    "type_info": {
        "type": "map",
        "key_type": { "type": "i32" },
        "value_type": {
            "type": "map",
            "key_type": { "type": "i32" },
            "value_type": { "type": "i64" }
        }
    }
}
```

```json
{
    "name": "id",
    "type_info": { "type": "interface", "idl_type": "ICalculator" }
}
```

```json
"return_type": { "type": "enum", "idl_type": "StatusCode" }
```

---

## 七、错误码使用

所有校验失败统一返回 `ABILITY_RUNTIME_ERROR_CODE_TLB_METADATA_INVALID`：
- dispID 重复
- interface_type 值不在 0/1/2 范围
- mainService 接口不存在或有多个
- type_info 字段不合法（缺少必要字段、key_type 不是简单类型）
- idl_type 引用的类型未在 tlb.json 中声明

---

## 八、文件清单

| 文件路径 | 变更类型 |
|---------|---------|
| `interfaces/kits/c/ability_runtime/modular_object_dispatcher.h` | 重大更新 |
| `frameworks/c/ability_runtime/include/mo_dispatcher_types.h` | 重大更新 |
| `frameworks/c/ability_runtime/include/mo_dispatcher_metadata_manager.h` | 重大更新 |
| `frameworks/c/ability_runtime/include/mo_dispatcher_complex_type_manager.h` | 重大更新 |
| `frameworks/c/ability_runtime/src/mo_dispatcher_metadata_manager.cpp` | 重大更新 |
| `frameworks/c/ability_runtime/src/mo_dispatcher_complex_type_manager.cpp` | 重大更新 |
| `frameworks/c/ability_runtime/src/mo_dispatcher_param_codec.cpp` | 小幅更新 |
| `frameworks/c/ability_runtime/src/modular_object_dispatcher.cpp` | 重大更新 |
| `tlb.json` | 格式更新 |
