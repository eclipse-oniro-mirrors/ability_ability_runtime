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

#ifndef ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_METADATA_MANAGER_H
#define ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_METADATA_MANAGER_H

#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "mo_dispatcher_types.h"
#include "nlohmann/json.hpp"

namespace OHOS::AbilityRuntime {

class ModObjDispatcherMetadataManager {
public:
    static constexpr uint32_t IPC_CODE_GET_TLB_FD = 1;
    static constexpr const char16_t INTERFACE_DESCRIPTOR[] = u"ohos.abilityruntime.ModularObjectService";

    AbilityRuntime_ErrorCode EnsureLoaded(OHOS::IRemoteObject* proxy);

    AbilityRuntime_ErrorCode QueryMainServiceInterfaceMemberIds(const char** names, uint32_t count,
        uint32_t* memberIds) const;

    AbilityRuntime_ErrorCode GetMethodMeta(uint32_t memberId, MoMethodMeta* methodMeta) const;

    AbilityRuntime_ErrorCode GetVersion(std::string* version) const;
    AbilityRuntime_ErrorCode GetMainServiceInterfaceName(std::string* interfaceName) const;

    AbilityRuntime_ErrorCode GetInterfaceCount(uint32_t* count) const;
    AbilityRuntime_ErrorCode GetInterfaceName(uint32_t index, std::string* name) const;
    AbilityRuntime_ErrorCode GetInterfaceIsCallback(const std::string& interfaceName, bool* isCallback) const;
    AbilityRuntime_ErrorCode GetInterfaceDescriptor(const std::string& interfaceName,
        std::u16string* descriptor) const;

    AbilityRuntime_ErrorCode GetMethodCount(const std::string& interfaceName, uint32_t* count) const;
    AbilityRuntime_ErrorCode GetMethodName(const std::string& interfaceName, uint32_t index,
        std::string* methodName) const;
    AbilityRuntime_ErrorCode GetMethodMemberId(const std::string& interfaceName, const std::string& methodName,
        uint32_t* memberId) const;
    AbilityRuntime_ErrorCode GetMethodReturnType(const std::string& interfaceName, const std::string& methodName,
        OH_AbilityRuntime_ModObjDispatcher_TypeInfo* returnType) const;
    AbilityRuntime_ErrorCode GetMethodParamCount(const std::string& interfaceName, const std::string& methodName,
        uint32_t* count) const;
    AbilityRuntime_ErrorCode GetMethodParamType(const std::string& interfaceName, const std::string& methodName,
        uint32_t paramIndex, OH_AbilityRuntime_ModObjDispatcher_TypeInfo* paramType) const;
    AbilityRuntime_ErrorCode GetMethodParamName(const std::string& interfaceName, const std::string& methodName,
        uint32_t paramIndex, std::string* paramName) const;

    AbilityRuntime_ErrorCode GetEnumCount(uint32_t* count) const;
    AbilityRuntime_ErrorCode GetEnumName(uint32_t index, std::string* name) const;
    AbilityRuntime_ErrorCode GetEnumValueCount(const std::string& enumName, uint32_t* count) const;
    AbilityRuntime_ErrorCode GetEnumValueName(const std::string& enumName, uint32_t index,
        std::string* valueName) const;
    AbilityRuntime_ErrorCode GetEnumValue(const std::string& enumName, const std::string& valueName,
        int32_t* value) const;

    AbilityRuntime_ErrorCode GetStructCount(uint32_t* count) const;
    AbilityRuntime_ErrorCode GetStructName(uint32_t index, std::string* name) const;
    AbilityRuntime_ErrorCode GetStructFieldCount(const std::string& structName, uint32_t* count) const;
    AbilityRuntime_ErrorCode GetStructFieldName(const std::string& structName, uint32_t index,
        std::string* fieldName) const;
    AbilityRuntime_ErrorCode GetStructFieldType(const std::string& structName, const std::string& fieldName,
        OH_AbilityRuntime_ModObjDispatcher_TypeInfo* fieldType) const;

private:
    // Static entry point for descriptor-based parsing with validation
    static AbilityRuntime_ErrorCode ParseTypeInfoFromJson(const nlohmann::json& typeInfoObj,
        std::shared_ptr<MoTypeInfo>& result);

    // Validate that idl_type references a declared type in the tlb
    bool IsIdlTypeDeclared(const std::string& idlType) const;

    // Fill a C TypeInfo struct from MoTypeInfo (shallow copy of the top-level fields only)
    static void FillCTypeInfo(OH_AbilityRuntime_ModObjDispatcher_TypeInfo* cType,
        const std::shared_ptr<MoTypeInfo>& moType);

    AbilityRuntime_ErrorCode ParseMetadata(const std::string& jsonText);
    AbilityRuntime_ErrorCode RequestMetadataJson(OHOS::IRemoteObject* proxy, std::string* jsonText);

    bool loaded_ = false;
    std::string version_;
    std::string mainServiceInterface_;

    std::vector<MoInterfaceMeta> interfaces_;
    std::vector<MoEnumMeta> enums_;
    std::vector<MoStructMeta> structs_;

    std::unordered_map<std::string, uint32_t> nameToMemberId_;
    std::unordered_map<uint32_t, std::string> memberIdToName_;
    std::unordered_map<uint32_t, MoMethodMeta> memberIdToMethod_;

    mutable std::mutex mutex_;
};
} // namespace OHOS::AbilityRuntime

#endif // ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_METADATA_MANAGER_H
