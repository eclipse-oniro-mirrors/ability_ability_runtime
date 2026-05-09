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

#ifndef ABILITY_RUNTIME_MO_DISPATCHER_COMPLEX_TYPE_MANAGER_H
#define ABILITY_RUNTIME_MO_DISPATCHER_COMPLEX_TYPE_MANAGER_H

#include <string>
#include <vector>

#include "mo_dispatcher_types.h"

namespace OHOS::AbilityRuntime {
class MoDispatcherComplexTypeManager {
public:
    static AbilityRuntime_ErrorCode ArrayCreate(OH_AbilityRuntime_MoDispatcher_TypeInfo* elementType,
        uint32_t size, OH_AbilityRuntime_MoDispatcher_ArrayHandle* ppArray);
    static AbilityRuntime_ErrorCode ArrayGetElementType(OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray,
        OH_AbilityRuntime_MoDispatcher_TypeInfo* pElementType);
    static AbilityRuntime_ErrorCode ArraySet(OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, uint32_t index,
        const OH_AbilityRuntime_MoDispatcher_Variant* pValue);
    static AbilityRuntime_ErrorCode ArrayGet(OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, uint32_t index,
        OH_AbilityRuntime_MoDispatcher_Variant* pValue);
    static AbilityRuntime_ErrorCode ArrayGetSize(OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, uint32_t* pSize);
    static void ArrayRelease(OH_AbilityRuntime_MoDispatcher_ArrayHandle* ppArray);

    static AbilityRuntime_ErrorCode VectorCreate(OH_AbilityRuntime_MoDispatcher_TypeInfo* elementType,
        OH_AbilityRuntime_MoDispatcher_VectorHandle* ppVector);
    static AbilityRuntime_ErrorCode VectorGetElementType(OH_AbilityRuntime_MoDispatcher_VectorHandle pVector,
        OH_AbilityRuntime_MoDispatcher_TypeInfo* pElementType);
    static AbilityRuntime_ErrorCode VectorAdd(OH_AbilityRuntime_MoDispatcher_VectorHandle pVector,
        const OH_AbilityRuntime_MoDispatcher_Variant* pValue);
    static AbilityRuntime_ErrorCode VectorGet(OH_AbilityRuntime_MoDispatcher_VectorHandle pVector, uint32_t index,
        OH_AbilityRuntime_MoDispatcher_Variant* pValue);
    static AbilityRuntime_ErrorCode VectorGetSize(OH_AbilityRuntime_MoDispatcher_VectorHandle pVector,
        uint32_t* pSize);
    static AbilityRuntime_ErrorCode VectorClear(OH_AbilityRuntime_MoDispatcher_VectorHandle pVector);
    static void VectorRelease(OH_AbilityRuntime_MoDispatcher_VectorHandle* ppVector);

    static AbilityRuntime_ErrorCode SetCreate(OH_AbilityRuntime_MoDispatcher_TypeInfo* elementType,
        OH_AbilityRuntime_MoDispatcher_SetHandle* ppSet);
    static AbilityRuntime_ErrorCode SetGetElementType(OH_AbilityRuntime_MoDispatcher_SetHandle pSet,
        OH_AbilityRuntime_MoDispatcher_TypeInfo* pElementType);
    static AbilityRuntime_ErrorCode SetAdd(OH_AbilityRuntime_MoDispatcher_SetHandle pSet,
        const OH_AbilityRuntime_MoDispatcher_Variant* pValue);
    static AbilityRuntime_ErrorCode SetRemove(OH_AbilityRuntime_MoDispatcher_SetHandle pSet,
        const OH_AbilityRuntime_MoDispatcher_Variant* pValue);
    static AbilityRuntime_ErrorCode SetContains(OH_AbilityRuntime_MoDispatcher_SetHandle pSet,
        const OH_AbilityRuntime_MoDispatcher_Variant* pValue, bool* pExists);
    static AbilityRuntime_ErrorCode SetGetSize(OH_AbilityRuntime_MoDispatcher_SetHandle pSet, uint32_t* pSize);
    static AbilityRuntime_ErrorCode SetGetAt(OH_AbilityRuntime_MoDispatcher_SetHandle pSet, uint32_t index,
        OH_AbilityRuntime_MoDispatcher_Variant* pValue);
    static AbilityRuntime_ErrorCode SetClear(OH_AbilityRuntime_MoDispatcher_SetHandle pSet);
    static void SetRelease(OH_AbilityRuntime_MoDispatcher_SetHandle* ppSet);

    static AbilityRuntime_ErrorCode MapCreate(OH_AbilityRuntime_MoDispatcher_ValueType keyType,
        OH_AbilityRuntime_MoDispatcher_TypeInfo* valueType, OH_AbilityRuntime_MoDispatcher_MapHandle* ppMap);
    static AbilityRuntime_ErrorCode MapGetKeyType(OH_AbilityRuntime_MoDispatcher_MapHandle pMap,
        OH_AbilityRuntime_MoDispatcher_ValueType* pKeyType);
    static AbilityRuntime_ErrorCode MapGetValueType(OH_AbilityRuntime_MoDispatcher_MapHandle pMap,
        OH_AbilityRuntime_MoDispatcher_TypeInfo* pValueType);
    static AbilityRuntime_ErrorCode MapPut(OH_AbilityRuntime_MoDispatcher_MapHandle pMap,
        const OH_AbilityRuntime_MoDispatcher_Variant* pKey, const OH_AbilityRuntime_MoDispatcher_Variant* pValue);
    static AbilityRuntime_ErrorCode MapGet(OH_AbilityRuntime_MoDispatcher_MapHandle pMap,
        const OH_AbilityRuntime_MoDispatcher_Variant* pKey, OH_AbilityRuntime_MoDispatcher_Variant* pValue);
    static AbilityRuntime_ErrorCode MapRemove(OH_AbilityRuntime_MoDispatcher_MapHandle pMap,
        const OH_AbilityRuntime_MoDispatcher_Variant* pKey);
    static AbilityRuntime_ErrorCode MapContainsKey(OH_AbilityRuntime_MoDispatcher_MapHandle pMap,
        const OH_AbilityRuntime_MoDispatcher_Variant* pKey, bool* pExists);
    static AbilityRuntime_ErrorCode MapGetSize(OH_AbilityRuntime_MoDispatcher_MapHandle pMap, uint32_t* pSize);
    static AbilityRuntime_ErrorCode MapGetKeyAt(OH_AbilityRuntime_MoDispatcher_MapHandle pMap, uint32_t index,
        OH_AbilityRuntime_MoDispatcher_Variant* pKey);
    static AbilityRuntime_ErrorCode MapGetValueAt(OH_AbilityRuntime_MoDispatcher_MapHandle pMap, uint32_t index,
        OH_AbilityRuntime_MoDispatcher_Variant* pValue);
    static AbilityRuntime_ErrorCode MapClear(OH_AbilityRuntime_MoDispatcher_MapHandle pMap);
    static void MapRelease(OH_AbilityRuntime_MoDispatcher_MapHandle* ppMap);

    static AbilityRuntime_ErrorCode StructCreate(const char* structName,
        OH_AbilityRuntime_MoDispatcher_StructHandle* ppStruct);
    static AbilityRuntime_ErrorCode StructGetName(OH_AbilityRuntime_MoDispatcher_StructHandle pStruct, char* pbstrName,
        uint32_t cMaxName);
    static AbilityRuntime_ErrorCode StructSetField(OH_AbilityRuntime_MoDispatcher_StructHandle pStruct,
        const char* szName, const OH_AbilityRuntime_MoDispatcher_Variant* pValue);
    static AbilityRuntime_ErrorCode StructGetField(OH_AbilityRuntime_MoDispatcher_StructHandle pStruct,
        const char* szName, OH_AbilityRuntime_MoDispatcher_Variant* pValue);
    static void StructRelease(OH_AbilityRuntime_MoDispatcher_StructHandle* ppStruct);

    static void Variant_Clear(OH_AbilityRuntime_MoDispatcher_Variant* pVariant);
    static void TypeInfo_Clear(OH_AbilityRuntime_MoDispatcher_TypeInfo* pTypeInfo);

    static AbilityRuntime_ErrorCode StoreVariant(const OH_AbilityRuntime_MoDispatcher_Variant* src,
        MoVariantStorage* dst);
    static AbilityRuntime_ErrorCode LoadVariant(const MoVariantStorage& src,
        OH_AbilityRuntime_MoDispatcher_Variant* dst);
    static AbilityRuntime_ErrorCode ValidateVariantType(const OH_AbilityRuntime_MoDispatcher_Variant* value,
        OH_AbilityRuntime_MoDispatcher_ValueType expectedType);
    static bool VariantEquals(const MoVariantStorage& lhs, const OH_AbilityRuntime_MoDispatcher_Variant* rhs);

    static void RegisterStructMetadata(const std::vector<MoStructMeta>& structs);
    static bool GetStructFieldType(const std::string& structName, const std::string& fieldName,
        std::shared_ptr<MoTypeInfo>* fieldType);
    static bool GetStructFieldNames(const std::string& structName, std::vector<std::string>* fieldNames);

};
} // namespace OHOS::AbilityRuntime

#endif // ABILITY_RUNTIME_MO_DISPATCHER_COMPLEX_TYPE_MANAGER_H
