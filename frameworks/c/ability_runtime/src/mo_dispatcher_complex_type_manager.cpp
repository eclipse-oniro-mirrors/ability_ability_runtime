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

#include "mo_dispatcher_complex_type_manager.h"

#include <algorithm>
#include <cstring>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

#include "hilog_tag_wrapper.h"
#include "securec.h"

namespace OHOS::AbilityRuntime {
namespace {
std::mutex g_structMetaMutex;
std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<MoTypeInfo>>> g_structFieldTypes;
std::unordered_map<std::string, std::vector<std::string>> g_structFieldOrder;

bool IsVariantHandleValid(const OH_AbilityRuntime_MoDispatcher_Variant* value)
{
    switch (value->vt) {
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ARRAY:
            return value->u.parrayVal != nullptr;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VECTOR:
            return value->u.pvectorVal != nullptr;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_SET:
            return value->u.psetVal != nullptr;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_MAP:
            return value->u.pmapVal != nullptr;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT:
            return value->u.pstructVal != nullptr;
        default:
            return true;
    }
}

// RAII helper: inserts a pointer into a visited set on construction, erases on destruction.
// Used to detect circular references in nested container types.
struct ScopedVisited {
    std::unordered_set<const void*>& visited;
    const void* ptr;
    bool inserted;
    ScopedVisited(std::unordered_set<const void*>& v, const void* p) : visited(v), ptr(p)
    {
        inserted = visited.insert(ptr).second;
    }
    ~ScopedVisited()
    {
        if (inserted) {
            visited.erase(ptr);
        }
    }
    explicit operator bool() const { return inserted; }
};

AbilityRuntime_ErrorCode CopyStringToBuffer(const std::string& src, char* dst, uint32_t maxLen)
{
    if (dst == nullptr || maxLen == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (src.size() + 1 > maxLen) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (strcpy_s(dst, maxLen, src.c_str()) != EOK) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

MoVariantStorage CreateDefaultVariantStorage(OH_AbilityRuntime_MoDispatcher_ValueType type)
{
    MoVariantStorage storage;
    storage.value.vt = type;
    if (type == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING) {
        storage.stringStorage = "";
        storage.value.u.bstrVal = const_cast<char*>(storage.stringStorage.c_str());
    }
    return storage;
}

// Deep copy a MoVariantStorage into another. Handles strings and nested containers recursively.
AbilityRuntime_ErrorCode DeepCopyStorage(const MoVariantStorage& src, MoVariantStorage& dst,
    std::unordered_set<const void*>& visited)
{
    dst.value = src.value;
    dst.stringStorage.clear();
    if (src.value.vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING) {
        dst.stringStorage = src.stringStorage;
        dst.value.u.bstrVal = const_cast<char*>(dst.stringStorage.c_str());
    } else if (src.value.vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ARRAY && src.value.u.parrayVal != nullptr) {
        ScopedVisited sv(visited, src.value.u.parrayVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "DeepCopyStorage: circular reference detected in array");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldArray = src.value.u.parrayVal;
        auto* newArray = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Array();
        if (newArray == nullptr) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        newArray->elementTypeInfo = oldArray->elementTypeInfo;
        newArray->elements.reserve(oldArray->elements.size());
        for (const auto& elem : oldArray->elements) {
            MoVariantStorage elemCopy;
            auto ret = DeepCopyStorage(elem, elemCopy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) { delete newArray; return ret; }
            newArray->elements.emplace_back(std::move(elemCopy));
        }
        dst.value.u.parrayVal = newArray;
    } else if (src.value.vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VECTOR && src.value.u.pvectorVal != nullptr) {
        ScopedVisited sv(visited, src.value.u.pvectorVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "DeepCopyStorage: circular reference detected in vector");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldVector = src.value.u.pvectorVal;
        auto* newVector = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Vector();
        if (newVector == nullptr) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        newVector->elementTypeInfo = oldVector->elementTypeInfo;
        newVector->elements.reserve(oldVector->elements.size());
        for (const auto& elem : oldVector->elements) {
            MoVariantStorage elemCopy;
            auto ret = DeepCopyStorage(elem, elemCopy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) { delete newVector; return ret; }
            newVector->elements.emplace_back(std::move(elemCopy));
        }
        dst.value.u.pvectorVal = newVector;
    } else if (src.value.vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_SET && src.value.u.psetVal != nullptr) {
        ScopedVisited sv(visited, src.value.u.psetVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "DeepCopyStorage: circular reference detected in set");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldSet = src.value.u.psetVal;
        auto* newSet = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Set();
        if (newSet == nullptr) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        newSet->elementTypeInfo = oldSet->elementTypeInfo;
        newSet->elements.reserve(oldSet->elements.size());
        for (const auto& elem : oldSet->elements) {
            MoVariantStorage elemCopy;
            auto ret = DeepCopyStorage(elem, elemCopy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) { delete newSet; return ret; }
            newSet->elements.emplace_back(std::move(elemCopy));
        }
        dst.value.u.psetVal = newSet;
    } else if (src.value.vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_MAP && src.value.u.pmapVal != nullptr) {
        ScopedVisited sv(visited, src.value.u.pmapVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "DeepCopyStorage: circular reference detected in map");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldMap = src.value.u.pmapVal;
        auto* newMap = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Map();
        if (newMap == nullptr) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        newMap->keyType = oldMap->keyType;
        newMap->valueTypeInfo = oldMap->valueTypeInfo;
        newMap->entries.reserve(oldMap->entries.size());
        for (const auto& entry : oldMap->entries) {
            std::pair<MoVariantStorage, MoVariantStorage> entryCopy;
            auto ret = DeepCopyStorage(entry.first, entryCopy.first, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) { delete newMap; return ret; }
            ret = DeepCopyStorage(entry.second, entryCopy.second, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) { delete newMap; return ret; }
            newMap->entries.emplace_back(std::move(entryCopy));
        }
        dst.value.u.pmapVal = newMap;
    } else if (src.value.vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT && src.value.u.pstructVal != nullptr) {
        ScopedVisited sv(visited, src.value.u.pstructVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "DeepCopyStorage: circular reference detected in struct");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldStruct = src.value.u.pstructVal;
        auto* newStruct = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Struct();
        if (newStruct == nullptr) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        newStruct->name = oldStruct->name;
        newStruct->fieldTypes = oldStruct->fieldTypes;
        for (const auto& field : oldStruct->fields) {
            MoVariantStorage fieldCopy;
            auto ret = DeepCopyStorage(field.second, fieldCopy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) { delete newStruct; return ret; }
            newStruct->fields[field.first] = std::move(fieldCopy);
        }
        dst.value.u.pstructVal = newStruct;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}
} // namespace

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::ArrayCreate(
    OH_AbilityRuntime_MoDispatcher_TypeInfo* elementType, uint32_t size,
    OH_AbilityRuntime_MoDispatcher_ArrayHandle* ppArray)
{
    if (ppArray == nullptr || elementType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto typeInfo = MoTypeInfo::FromCTypeInfo(elementType);
    if (typeInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* array = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Array();
    if (array == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "ArrayCreate: allocate failed");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    array->elementTypeInfo = typeInfo;
    array->elements.reserve(size);
    for (uint32_t i = 0; i < size; i++) {
        array->elements.emplace_back(CreateDefaultVariantStorage(typeInfo->vt));
    }
    *ppArray = array;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::ArrayGetElementType(
    OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, OH_AbilityRuntime_MoDispatcher_TypeInfo* pElementType)
{
    if (pArray == nullptr || pElementType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (pArray->elementTypeInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    pArray->elementTypeInfo->FillCTypeInfo(pElementType);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::ArraySet(OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray,
    uint32_t index, const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pArray == nullptr || pValue == nullptr || index >= pArray->elements.size()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = ValidateVariantType(pValue, pArray->elementTypeInfo->vt);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "ArraySet: type mismatch at index=%{public}u", index);
        return ret;
    }
    return StoreVariant(pValue, &pArray->elements[index]);
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::ArrayGet(OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray,
    uint32_t index, OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pArray == nullptr || pValue == nullptr || index >= pArray->elements.size()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return LoadVariant(pArray->elements[index], pValue);
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::ArrayGetSize(OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray,
    uint32_t* pSize)
{
    if (pArray == nullptr || pSize == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pSize = static_cast<uint32_t>(pArray->elements.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void MoDispatcherComplexTypeManager::ArrayRelease(OH_AbilityRuntime_MoDispatcher_ArrayHandle* ppArray)
{
    if (ppArray == nullptr || *ppArray == nullptr) {
        return;
    }
    delete *ppArray;
    *ppArray = nullptr;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::VectorCreate(
    OH_AbilityRuntime_MoDispatcher_TypeInfo* elementType, OH_AbilityRuntime_MoDispatcher_VectorHandle* ppVector)
{
    if (ppVector == nullptr || elementType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto typeInfo = MoTypeInfo::FromCTypeInfo(elementType);
    if (typeInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* vector = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Vector();
    if (vector == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "VectorCreate: allocate failed");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    vector->elementTypeInfo = typeInfo;
    *ppVector = vector;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::VectorGetElementType(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector, OH_AbilityRuntime_MoDispatcher_TypeInfo* pElementType)
{
    if (pVector == nullptr || pElementType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (pVector->elementTypeInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    pVector->elementTypeInfo->FillCTypeInfo(pElementType);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::VectorAdd(OH_AbilityRuntime_MoDispatcher_VectorHandle pVector,
    const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pVector == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = ValidateVariantType(pValue, pVector->elementTypeInfo->vt);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "VectorAdd: type mismatch");
        return ret;
    }
    MoVariantStorage storage;
    ret = StoreVariant(pValue, &storage);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    pVector->elements.emplace_back(std::move(storage));
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::VectorGet(OH_AbilityRuntime_MoDispatcher_VectorHandle pVector,
    uint32_t index, OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pVector == nullptr || pValue == nullptr || index >= pVector->elements.size()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return LoadVariant(pVector->elements[index], pValue);
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::VectorGetSize(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector, uint32_t* pSize)
{
    if (pVector == nullptr || pSize == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pSize = static_cast<uint32_t>(pVector->elements.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::VectorClear(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector)
{
    if (pVector == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    pVector->elements.clear();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void MoDispatcherComplexTypeManager::VectorRelease(OH_AbilityRuntime_MoDispatcher_VectorHandle* ppVector)
{
    if (ppVector == nullptr || *ppVector == nullptr) {
        return;
    }
    delete *ppVector;
    *ppVector = nullptr;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::SetCreate(
    OH_AbilityRuntime_MoDispatcher_TypeInfo* elementType, OH_AbilityRuntime_MoDispatcher_SetHandle* ppSet)
{
    if (ppSet == nullptr || elementType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto typeInfo = MoTypeInfo::FromCTypeInfo(elementType);
    if (typeInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* set = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Set();
    if (set == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "SetCreate: allocate failed");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    set->elementTypeInfo = typeInfo;
    *ppSet = set;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::SetGetElementType(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, OH_AbilityRuntime_MoDispatcher_TypeInfo* pElementType)
{
    if (pSet == nullptr || pElementType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (pSet->elementTypeInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    pSet->elementTypeInfo->FillCTypeInfo(pElementType);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::SetAdd(OH_AbilityRuntime_MoDispatcher_SetHandle pSet,
    const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pSet == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = ValidateVariantType(pValue, pSet->elementTypeInfo->vt);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    for (const auto& element : pSet->elements) {
        if (VariantEquals(element, pValue)) {
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
    }
    MoVariantStorage storage;
    ret = StoreVariant(pValue, &storage);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    pSet->elements.emplace_back(std::move(storage));
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::SetRemove(OH_AbilityRuntime_MoDispatcher_SetHandle pSet,
    const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pSet == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto iter = std::find_if(pSet->elements.begin(), pSet->elements.end(),
        [pValue](const MoVariantStorage& item) { return VariantEquals(item, pValue); });
    if (iter != pSet->elements.end()) {
        pSet->elements.erase(iter);
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::SetContains(OH_AbilityRuntime_MoDispatcher_SetHandle pSet,
    const OH_AbilityRuntime_MoDispatcher_Variant* pValue, bool* pExists)
{
    if (pSet == nullptr || pValue == nullptr || pExists == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pExists = std::any_of(pSet->elements.begin(), pSet->elements.end(),
        [pValue](const MoVariantStorage& item) { return VariantEquals(item, pValue); });
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::SetGetSize(OH_AbilityRuntime_MoDispatcher_SetHandle pSet,
    uint32_t* pSize)
{
    if (pSet == nullptr || pSize == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pSize = static_cast<uint32_t>(pSet->elements.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::SetGetAt(OH_AbilityRuntime_MoDispatcher_SetHandle pSet,
    uint32_t index, OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pSet == nullptr || pValue == nullptr || index >= pSet->elements.size()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return LoadVariant(pSet->elements[index], pValue);
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::SetClear(OH_AbilityRuntime_MoDispatcher_SetHandle pSet)
{
    if (pSet == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    pSet->elements.clear();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void MoDispatcherComplexTypeManager::SetRelease(OH_AbilityRuntime_MoDispatcher_SetHandle* ppSet)
{
    if (ppSet == nullptr || *ppSet == nullptr) {
        return;
    }
    delete *ppSet;
    *ppSet = nullptr;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::MapCreate(
    OH_AbilityRuntime_MoDispatcher_ValueType keyType, OH_AbilityRuntime_MoDispatcher_TypeInfo* valueType,
    OH_AbilityRuntime_MoDispatcher_MapHandle* ppMap)
{
    if (ppMap == nullptr || valueType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto valueTypeInfo = MoTypeInfo::FromCTypeInfo(valueType);
    if (valueTypeInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* map = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Map();
    if (map == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "MapCreate: allocate failed");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    map->keyType = keyType;
    map->valueTypeInfo = valueTypeInfo;
    *ppMap = map;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::MapGetKeyType(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, OH_AbilityRuntime_MoDispatcher_ValueType* pKeyType)
{
    if (pMap == nullptr || pKeyType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pKeyType = pMap->keyType;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::MapGetValueType(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, OH_AbilityRuntime_MoDispatcher_TypeInfo* pValueType)
{
    if (pMap == nullptr || pValueType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (pMap->valueTypeInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    pMap->valueTypeInfo->FillCTypeInfo(pValueType);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::MapPut(OH_AbilityRuntime_MoDispatcher_MapHandle pMap,
    const OH_AbilityRuntime_MoDispatcher_Variant* pKey, const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pMap == nullptr || pKey == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = ValidateVariantType(pKey, pMap->keyType);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "MapPut: key type mismatch");
        return ret;
    }
    ret = ValidateVariantType(pValue, pMap->valueTypeInfo->vt);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "MapPut: value type mismatch");
        return ret;
    }

    for (auto& entry : pMap->entries) {
        if (VariantEquals(entry.first, pKey)) {
            return StoreVariant(pValue, &entry.second);
        }
    }

    std::pair<MoVariantStorage, MoVariantStorage> entry;
    ret = StoreVariant(pKey, &entry.first);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    ret = StoreVariant(pValue, &entry.second);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    pMap->entries.emplace_back(std::move(entry));
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::MapGet(OH_AbilityRuntime_MoDispatcher_MapHandle pMap,
    const OH_AbilityRuntime_MoDispatcher_Variant* pKey, OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pMap == nullptr || pKey == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    for (const auto& entry : pMap->entries) {
        if (VariantEquals(entry.first, pKey)) {
            return LoadVariant(entry.second, pValue);
        }
    }
    return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::MapRemove(OH_AbilityRuntime_MoDispatcher_MapHandle pMap,
    const OH_AbilityRuntime_MoDispatcher_Variant* pKey)
{
    if (pMap == nullptr || pKey == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto iter = std::find_if(pMap->entries.begin(), pMap->entries.end(),
        [pKey](const auto& item) { return VariantEquals(item.first, pKey); });
    if (iter != pMap->entries.end()) {
        pMap->entries.erase(iter);
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::MapContainsKey(OH_AbilityRuntime_MoDispatcher_MapHandle pMap,
    const OH_AbilityRuntime_MoDispatcher_Variant* pKey, bool* pExists)
{
    if (pMap == nullptr || pKey == nullptr || pExists == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pExists = std::any_of(pMap->entries.begin(), pMap->entries.end(),
        [pKey](const auto& item) { return VariantEquals(item.first, pKey); });
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::MapGetSize(OH_AbilityRuntime_MoDispatcher_MapHandle pMap,
    uint32_t* pSize)
{
    if (pMap == nullptr || pSize == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pSize = static_cast<uint32_t>(pMap->entries.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::MapGetKeyAt(OH_AbilityRuntime_MoDispatcher_MapHandle pMap,
    uint32_t index, OH_AbilityRuntime_MoDispatcher_Variant* pKey)
{
    if (pMap == nullptr || pKey == nullptr || index >= pMap->entries.size()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return LoadVariant(pMap->entries[index].first, pKey);
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::MapGetValueAt(OH_AbilityRuntime_MoDispatcher_MapHandle pMap,
    uint32_t index, OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pMap == nullptr || pValue == nullptr || index >= pMap->entries.size()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return LoadVariant(pMap->entries[index].second, pValue);
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::MapClear(OH_AbilityRuntime_MoDispatcher_MapHandle pMap)
{
    if (pMap == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    pMap->entries.clear();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void MoDispatcherComplexTypeManager::MapRelease(OH_AbilityRuntime_MoDispatcher_MapHandle* ppMap)
{
    if (ppMap == nullptr || *ppMap == nullptr) {
        return;
    }
    delete *ppMap;
    *ppMap = nullptr;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::StructCreate(const char* structName,
    OH_AbilityRuntime_MoDispatcher_StructHandle* ppStruct)
{
    if (structName == nullptr || ppStruct == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* object = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Struct();
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "StructCreate: allocate failed for '%{public}s'", structName);
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    object->name = structName;
    {
        std::lock_guard<std::mutex> lock(g_structMetaMutex);
        auto iter = g_structFieldTypes.find(object->name);
        if (iter != g_structFieldTypes.end()) {
            object->fieldTypes = iter->second;
        }
    }
    *ppStruct = object;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::StructGetName(
    OH_AbilityRuntime_MoDispatcher_StructHandle pStruct, char* pbstrName, uint32_t cMaxName)
{
    if (pStruct == nullptr || pbstrName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return CopyStringToBuffer(pStruct->name, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::StructSetField(
    OH_AbilityRuntime_MoDispatcher_StructHandle pStruct, const char* szName,
    const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pStruct == nullptr || szName == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto fieldTypeIter = pStruct->fieldTypes.find(szName);
    if (fieldTypeIter == pStruct->fieldTypes.end()) {
        TAG_LOGE(AAFwkTag::EXT, "StructSetField: field '%{public}s' not found in struct '%{public}s'",
            szName, pStruct->name.c_str());
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    auto ret = ValidateVariantType(pValue, fieldTypeIter->second->vt);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "StructSetField: type mismatch for field '%{public}s'", szName);
        return ret;
    }
    MoVariantStorage storage;
    ret = StoreVariant(pValue, &storage);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    pStruct->fields[szName] = std::move(storage);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::StructGetField(
    OH_AbilityRuntime_MoDispatcher_StructHandle pStruct, const char* szName,
    OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pStruct == nullptr || szName == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto iter = pStruct->fields.find(szName);
    if (iter == pStruct->fields.end()) {
        TAG_LOGE(AAFwkTag::EXT, "StructGetField: field '%{public}s' not found in struct '%{public}s'",
            szName, pStruct->name.c_str());
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    return LoadVariant(iter->second, pValue);
}

void MoDispatcherComplexTypeManager::StructRelease(OH_AbilityRuntime_MoDispatcher_StructHandle* ppStruct)
{
    if (ppStruct == nullptr || *ppStruct == nullptr) {
        return;
    }
    delete *ppStruct;
    *ppStruct = nullptr;
}

void MoDispatcherComplexTypeManager::Variant_Clear(OH_AbilityRuntime_MoDispatcher_Variant* pVariant)
{
    if (pVariant == nullptr) {
        return;
    }
    switch (pVariant->vt) {
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING:
            if (pVariant->u.bstrVal != nullptr) {
                std::free(pVariant->u.bstrVal);
                pVariant->u.bstrVal = nullptr;
            }
            break;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ARRAY:
            if (pVariant->u.parrayVal != nullptr) {
                ArrayRelease(&pVariant->u.parrayVal);
            }
            break;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VECTOR:
            if (pVariant->u.pvectorVal != nullptr) {
                VectorRelease(&pVariant->u.pvectorVal);
            }
            break;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_SET:
            if (pVariant->u.psetVal != nullptr) {
                SetRelease(&pVariant->u.psetVal);
            }
            break;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_MAP:
            if (pVariant->u.pmapVal != nullptr) {
                MapRelease(&pVariant->u.pmapVal);
            }
            break;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT:
            if (pVariant->u.pstructVal != nullptr) {
                StructRelease(&pVariant->u.pstructVal);
            }
            break;
        default:
            break;
    }
    (void)memset_s(pVariant, sizeof(OH_AbilityRuntime_MoDispatcher_Variant), 0,
        sizeof(OH_AbilityRuntime_MoDispatcher_Variant));
    pVariant->vt = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
}

void MoDispatcherComplexTypeManager::TypeInfo_Clear(OH_AbilityRuntime_MoDispatcher_TypeInfo* pTypeInfo)
{
    MoTypeInfo::ClearCTypeInfo(pTypeInfo);
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::StoreVariant(
    const OH_AbilityRuntime_MoDispatcher_Variant* src, MoVariantStorage* dst)
{
    if (src == nullptr || dst == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    dst->value = *src;
    dst->stringStorage.clear();
    std::unordered_set<const void*> visited;
    if (src->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING) {
        dst->stringStorage = (src->u.bstrVal != nullptr) ? src->u.bstrVal : "";
        dst->value.u.bstrVal = const_cast<char*>(dst->stringStorage.c_str());
    } else if (src->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ARRAY && src->u.parrayVal != nullptr) {
        ScopedVisited sv(visited, src->u.parrayVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "StoreVariant: circular reference detected in array");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldArray = src->u.parrayVal;
        auto* newArray = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Array();
        if (newArray == nullptr) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        newArray->elementTypeInfo = oldArray->elementTypeInfo;
        newArray->elements.reserve(oldArray->elements.size());
        for (const auto& elem : oldArray->elements) {
            MoVariantStorage elemCopy;
            auto ret = DeepCopyStorage(elem, elemCopy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) { delete newArray; return ret; }
            newArray->elements.emplace_back(std::move(elemCopy));
        }
        dst->value.u.parrayVal = newArray;
    } else if (src->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VECTOR && src->u.pvectorVal != nullptr) {
        ScopedVisited sv(visited, src->u.pvectorVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "StoreVariant: circular reference detected in vector");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldVector = src->u.pvectorVal;
        auto* newVector = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Vector();
        if (newVector == nullptr) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        newVector->elementTypeInfo = oldVector->elementTypeInfo;
        newVector->elements.reserve(oldVector->elements.size());
        for (const auto& elem : oldVector->elements) {
            MoVariantStorage elemCopy;
            auto ret = DeepCopyStorage(elem, elemCopy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) { delete newVector; return ret; }
            newVector->elements.emplace_back(std::move(elemCopy));
        }
        dst->value.u.pvectorVal = newVector;
    } else if (src->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_SET && src->u.psetVal != nullptr) {
        ScopedVisited sv(visited, src->u.psetVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "StoreVariant: circular reference detected in set");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldSet = src->u.psetVal;
        auto* newSet = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Set();
        if (newSet == nullptr) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        newSet->elementTypeInfo = oldSet->elementTypeInfo;
        newSet->elements.reserve(oldSet->elements.size());
        for (const auto& elem : oldSet->elements) {
            MoVariantStorage elemCopy;
            auto ret = DeepCopyStorage(elem, elemCopy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) { delete newSet; return ret; }
            newSet->elements.emplace_back(std::move(elemCopy));
        }
        dst->value.u.psetVal = newSet;
    } else if (src->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_MAP && src->u.pmapVal != nullptr) {
        ScopedVisited sv(visited, src->u.pmapVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "StoreVariant: circular reference detected in map");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldMap = src->u.pmapVal;
        auto* newMap = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Map();
        if (newMap == nullptr) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        newMap->keyType = oldMap->keyType;
        newMap->valueTypeInfo = oldMap->valueTypeInfo;
        newMap->entries.reserve(oldMap->entries.size());
        for (const auto& entry : oldMap->entries) {
            std::pair<MoVariantStorage, MoVariantStorage> entryCopy;
            auto ret = DeepCopyStorage(entry.first, entryCopy.first, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) { delete newMap; return ret; }
            ret = DeepCopyStorage(entry.second, entryCopy.second, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) { delete newMap; return ret; }
            newMap->entries.emplace_back(std::move(entryCopy));
        }
        dst->value.u.pmapVal = newMap;
    } else if (src->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT && src->u.pstructVal != nullptr) {
        ScopedVisited sv(visited, src->u.pstructVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "StoreVariant: circular reference detected in struct");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldStruct = src->u.pstructVal;
        auto* newStruct = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Struct();
        if (newStruct == nullptr) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        newStruct->name = oldStruct->name;
        newStruct->fieldTypes = oldStruct->fieldTypes;
        for (const auto& field : oldStruct->fields) {
            MoVariantStorage fieldCopy;
            auto ret = DeepCopyStorage(field.second, fieldCopy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) { delete newStruct; return ret; }
            newStruct->fields[field.first] = std::move(fieldCopy);
        }
        dst->value.u.pstructVal = newStruct;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::LoadVariant(const MoVariantStorage& src,
    OH_AbilityRuntime_MoDispatcher_Variant* dst)
{
    if (dst == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *dst = src.value;
    if (dst->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING) {
        // Deep copy: allocate new string so caller can safely Variant_Clear
        const char* srcStr = src.stringStorage.c_str();
        size_t len = src.stringStorage.size();
        auto* mem = static_cast<char*>(std::malloc(len + 1));
        if (mem == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "LoadVariant: malloc failed for string, len=%{public}zu", len);
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        if (strcpy_s(mem, len + 1, srcStr) != EOK) {
            std::free(mem);
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        dst->u.bstrVal = mem;
    } else if (dst->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ARRAY && dst->u.parrayVal != nullptr) {
        std::unordered_set<const void*> visited;
        ScopedVisited sv(visited, dst->u.parrayVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "LoadVariant: circular reference detected in array");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldArray = dst->u.parrayVal;
        auto* newArray = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Array();
        if (newArray == nullptr) {
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        newArray->elementTypeInfo = oldArray->elementTypeInfo;
        newArray->elements.reserve(oldArray->elements.size());
        for (const auto& elem : oldArray->elements) {
            MoVariantStorage copy;
            auto ret = DeepCopyStorage(elem, copy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newArray;
                dst->u.parrayVal = nullptr;
                return ret;
            }
            newArray->elements.emplace_back(std::move(copy));
        }
        dst->u.parrayVal = newArray;
    } else if (dst->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VECTOR && dst->u.pvectorVal != nullptr) {
        std::unordered_set<const void*> visited;
        ScopedVisited sv(visited, dst->u.pvectorVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "LoadVariant: circular reference detected in vector");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldVector = dst->u.pvectorVal;
        auto* newVector = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Vector();
        if (newVector == nullptr) {
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        newVector->elementTypeInfo = oldVector->elementTypeInfo;
        newVector->elements.reserve(oldVector->elements.size());
        for (const auto& elem : oldVector->elements) {
            MoVariantStorage copy;
            auto ret = DeepCopyStorage(elem, copy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newVector;
                dst->u.pvectorVal = nullptr;
                return ret;
            }
            newVector->elements.emplace_back(std::move(copy));
        }
        dst->u.pvectorVal = newVector;
    } else if (dst->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_SET && dst->u.psetVal != nullptr) {
        std::unordered_set<const void*> visited;
        ScopedVisited sv(visited, dst->u.psetVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "LoadVariant: circular reference detected in set");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldSet = dst->u.psetVal;
        auto* newSet = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Set();
        if (newSet == nullptr) {
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        newSet->elementTypeInfo = oldSet->elementTypeInfo;
        newSet->elements.reserve(oldSet->elements.size());
        for (const auto& elem : oldSet->elements) {
            MoVariantStorage copy;
            auto ret = DeepCopyStorage(elem, copy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newSet;
                dst->u.psetVal = nullptr;
                return ret;
            }
            newSet->elements.emplace_back(std::move(copy));
        }
        dst->u.psetVal = newSet;
    } else if (dst->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_MAP && dst->u.pmapVal != nullptr) {
        std::unordered_set<const void*> visited;
        ScopedVisited sv(visited, dst->u.pmapVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "LoadVariant: circular reference detected in map");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldMap = dst->u.pmapVal;
        auto* newMap = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Map();
        if (newMap == nullptr) {
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        newMap->keyType = oldMap->keyType;
        newMap->valueTypeInfo = oldMap->valueTypeInfo;
        newMap->entries.reserve(oldMap->entries.size());
        for (const auto& entry : oldMap->entries) {
            std::pair<MoVariantStorage, MoVariantStorage> copy;
            auto ret = DeepCopyStorage(entry.first, copy.first, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newMap;
                dst->u.pmapVal = nullptr;
                return ret;
            }
            ret = DeepCopyStorage(entry.second, copy.second, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newMap;
                dst->u.pmapVal = nullptr;
                return ret;
            }
            newMap->entries.emplace_back(std::move(copy));
        }
        dst->u.pmapVal = newMap;
    } else if (dst->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT && dst->u.pstructVal != nullptr) {
        std::unordered_set<const void*> visited;
        ScopedVisited sv(visited, dst->u.pstructVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "LoadVariant: circular reference detected in struct");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldStruct = dst->u.pstructVal;
        auto* newStruct = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Struct();
        if (newStruct == nullptr) {
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        newStruct->name = oldStruct->name;
        newStruct->fieldTypes = oldStruct->fieldTypes;
        for (const auto& field : oldStruct->fields) {
            MoVariantStorage copy;
            auto ret = DeepCopyStorage(field.second, copy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newStruct;
                dst->u.pstructVal = nullptr;
                return ret;
            }
            newStruct->fields[field.first] = std::move(copy);
        }
        dst->u.pstructVal = newStruct;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::ValidateVariantType(
    const OH_AbilityRuntime_MoDispatcher_Variant* value, OH_AbilityRuntime_MoDispatcher_ValueType expectedType)
{
    if (value == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (value->vt == expectedType) {
        if (!IsVariantHandleValid(value)) {
            TAG_LOGE(AAFwkTag::EXT, "ValidateVariantType: vt=%{public}d but required handle is null",
                static_cast<int32_t>(value->vt));
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if ((expectedType == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_PROXY &&
        value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_STUB) ||
        (expectedType == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_STUB &&
        value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_PROXY)) {
        if (!IsVariantHandleValid(value)) {
            TAG_LOGE(AAFwkTag::EXT, "ValidateVariantType: vt=%{public}d but required handle is null",
                static_cast<int32_t>(value->vt));
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    return ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH;
}

bool MoDispatcherComplexTypeManager::VariantEquals(const MoVariantStorage& lhs,
    const OH_AbilityRuntime_MoDispatcher_Variant* rhs)
{
    if (rhs == nullptr) {
        return false;
    }
    if (lhs.value.vt != rhs->vt) {
        return false;
    }
    switch (lhs.value.vt) {
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY:
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VOID:
            return true;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_BOOL:
            return lhs.value.u.boolVal == rhs->u.boolVal;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I8:
            return lhs.value.u.i8Val == rhs->u.i8Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I16:
            return lhs.value.u.i16Val == rhs->u.i16Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32:
            return lhs.value.u.i32Val == rhs->u.i32Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I64:
            return lhs.value.u.i64Val == rhs->u.i64Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U8:
            return lhs.value.u.u8Val == rhs->u.u8Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U16:
            return lhs.value.u.u16Val == rhs->u.u16Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U32:
            return lhs.value.u.u32Val == rhs->u.u32Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U64:
            return lhs.value.u.u64Val == rhs->u.u64Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F32:
            return lhs.value.u.f32Val == rhs->u.f32Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F64:
            return lhs.value.u.f64Val == rhs->u.f64Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING: {
            const char* lhsStr = lhs.stringStorage.c_str();
            const char* rhsStr = (rhs->u.bstrVal != nullptr) ? rhs->u.bstrVal : "";
            return std::strcmp(lhsStr, rhsStr) == 0;
        }
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ARRAY:
            return lhs.value.u.parrayVal == rhs->u.parrayVal;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VECTOR:
            return lhs.value.u.pvectorVal == rhs->u.pvectorVal;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_SET:
            return lhs.value.u.psetVal == rhs->u.psetVal;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_MAP:
            return lhs.value.u.pmapVal == rhs->u.pmapVal;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT:
            return lhs.value.u.pstructVal == rhs->u.pstructVal;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_PROXY:
            return lhs.value.u.premoteProxyVal == rhs->u.premoteProxyVal;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_STUB:
            return lhs.value.u.premoteStubVal == rhs->u.premoteStubVal;
        default:
            return false;
    }
}

void MoDispatcherComplexTypeManager::RegisterStructMetadata(const std::vector<MoStructMeta>& structs)
{
    std::lock_guard<std::mutex> lock(g_structMetaMutex);
    g_structFieldTypes.clear();
    g_structFieldOrder.clear();
    for (const auto& structMeta : structs) {
        std::unordered_map<std::string, std::shared_ptr<MoTypeInfo>> fieldTypeMap;
        std::vector<std::string> fieldOrder;
        for (const auto& field : structMeta.fields) {
            fieldTypeMap[field.name] = field.typeInfo;
            fieldOrder.emplace_back(field.name);
        }
        g_structFieldTypes[structMeta.name] = std::move(fieldTypeMap);
        g_structFieldOrder[structMeta.name] = std::move(fieldOrder);
    }
}

bool MoDispatcherComplexTypeManager::GetStructFieldType(const std::string& structName, const std::string& fieldName,
    std::shared_ptr<MoTypeInfo>* fieldType)
{
    if (fieldType == nullptr) {
        return false;
    }
    std::lock_guard<std::mutex> lock(g_structMetaMutex);
    auto structIter = g_structFieldTypes.find(structName);
    if (structIter == g_structFieldTypes.end()) {
        return false;
    }
    auto fieldIter = structIter->second.find(fieldName);
    if (fieldIter == structIter->second.end()) {
        return false;
    }
    *fieldType = fieldIter->second;
    return true;
}

bool MoDispatcherComplexTypeManager::GetStructFieldNames(const std::string& structName,
    std::vector<std::string>* fieldNames)
{
    if (fieldNames == nullptr) {
        return false;
    }
    std::lock_guard<std::mutex> lock(g_structMetaMutex);
    auto iter = g_structFieldOrder.find(structName);
    if (iter == g_structFieldOrder.end()) {
        return false;
    }
    *fieldNames = iter->second;
    return true;
}

} // namespace OHOS::AbilityRuntime
