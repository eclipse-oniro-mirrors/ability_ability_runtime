/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_TYPES_H
#define ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_TYPES_H

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "ipc_inner_object.h"
#include "modular_object_dispatcher.h"

namespace OHOS::AbilityRuntime {
class ModObjDispatcherMetadataManager;

// Enum for interface_type in tlb.json: 0=normal, 1=mainservice, 2=callback
enum class MoInterfaceType : uint32_t {
    NORMAL = 0,
    MAIN_SERVICE = 1,
    CALLBACK = 2,
};

// Internal representation of OH_AbilityRuntime_ModObjDispatcher_TypeInfo (recursive tree)
struct MoTypeInfo {
    OH_AbilityRuntime_ModObjDispatcher_ValueType vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY;
    // For map: keyType + pValueType
    OH_AbilityRuntime_ModObjDispatcher_ValueType mapKeyType = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY;
    std::shared_ptr<MoTypeInfo> pMapValueType;
    // For array/vector/set: pElementType (shared by all three)
    std::shared_ptr<MoTypeInfo> pElementType;
    // For array: fixed size (0 means dynamic/unspecified)
    uint32_t arraySize = 0;
    // For enum/interface/struct: idlType
    std::string idlType;

    // Recursively fill an existing C TypeInfo (deep copy with strdup/new).
    // Caller must eventually call OH_AbilityRuntime_ModObjDispatcher_TypeInfo_Clear to release.
    void FillCTypeInfo(OH_AbilityRuntime_ModObjDispatcher_TypeInfo* cType) const;

    // Recursively release heap resources inside a C TypeInfo filled by FillCTypeInfo.
    static void ClearCTypeInfo(OH_AbilityRuntime_ModObjDispatcher_TypeInfo* cType);

    // Build from C TypeInfo (recursive)
    static std::shared_ptr<MoTypeInfo> FromCTypeInfo(const OH_AbilityRuntime_ModObjDispatcher_TypeInfo* cType);
};

struct MoStructFieldMeta {
    std::string name;
    std::shared_ptr<MoTypeInfo> typeInfo;
    uint32_t memberId = 0;
};

struct MoStructMeta {
    std::string name;
    uint32_t memberId = 0;
    std::vector<MoStructFieldMeta> fields;
};

struct MoMethodParamMeta {
    std::string name;
    std::shared_ptr<MoTypeInfo> typeInfo;
};

struct MoMethodMeta {
    std::string interfaceName;
    std::string name;
    uint32_t memberId = 0;
    uint32_t ipcCode = 0;
    bool oneway = false;
    std::shared_ptr<MoTypeInfo> returnType;
    std::vector<MoMethodParamMeta> params;
};

struct MoEnumValueMeta {
    std::string name;
    int32_t value = 0;
    uint32_t memberId = 0;
};

struct MoEnumMeta {
    std::string name;
    uint32_t memberId = 0;
    std::vector<MoEnumValueMeta> values;
};

struct MoInterfaceMeta {
    std::string name;
    std::string descriptor;
    uint32_t memberId = 0;
    MoInterfaceType interfaceType = MoInterfaceType::NORMAL;
    std::string descriptorJson;
    std::vector<MoMethodMeta> methods;

    // Convenience helpers derived from interfaceType
    bool IsCallback() const { return interfaceType == MoInterfaceType::CALLBACK; }
    bool IsMainService() const { return interfaceType == MoInterfaceType::MAIN_SERVICE; }
};

struct MoVariantStorage {
    OH_AbilityRuntime_ModObjDispatcher_Variant value {};
    std::string stringStorage;

    ~MoVariantStorage();
    MoVariantStorage() = default;
    MoVariantStorage(const MoVariantStorage&) = delete;
    MoVariantStorage& operator=(const MoVariantStorage&) = delete;
    MoVariantStorage(MoVariantStorage&& other) noexcept;
    MoVariantStorage& operator=(MoVariantStorage&& other) noexcept;

private:
    void ReleaseNestedResources();
};
} // namespace OHOS::AbilityRuntime

struct OH_AbilityRuntime_ModularObjectDispatcher {
    OHOS::sptr<OHOS::IRemoteObject> proxy = nullptr;
    std::shared_ptr<OHOS::AbilityRuntime::ModObjDispatcherMetadataManager> metadataManager;
};

struct OH_AbilityRuntime_ModularObjectDispatcher_TypeDescriptor {
    std::shared_ptr<OHOS::AbilityRuntime::ModObjDispatcherMetadataManager> metadataManager;
};

struct OH_AbilityRuntime_ModularObjectDispatcher_Array {
    std::shared_ptr<OHOS::AbilityRuntime::MoTypeInfo> elementTypeInfo;
    std::vector<OHOS::AbilityRuntime::MoVariantStorage> elements;
};

struct OH_AbilityRuntime_ModularObjectDispatcher_Vector {
    std::shared_ptr<OHOS::AbilityRuntime::MoTypeInfo> elementTypeInfo;
    std::vector<OHOS::AbilityRuntime::MoVariantStorage> elements;
};

struct OH_AbilityRuntime_ModularObjectDispatcher_Set {
    std::shared_ptr<OHOS::AbilityRuntime::MoTypeInfo> elementTypeInfo;
    std::vector<OHOS::AbilityRuntime::MoVariantStorage> elements;
};

struct OH_AbilityRuntime_ModularObjectDispatcher_Map {
    OH_AbilityRuntime_ModObjDispatcher_ValueType keyType = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY;
    std::shared_ptr<OHOS::AbilityRuntime::MoTypeInfo> valueTypeInfo;
    std::vector<std::pair<OHOS::AbilityRuntime::MoVariantStorage, OHOS::AbilityRuntime::MoVariantStorage>> entries;
};

struct OH_AbilityRuntime_ModularObjectDispatcher_Struct {
    std::string name;
    std::unordered_map<std::string, std::shared_ptr<OHOS::AbilityRuntime::MoTypeInfo>> fieldTypes;
    std::unordered_map<std::string, OHOS::AbilityRuntime::MoVariantStorage> fields;
};

#endif // ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_TYPES_H
