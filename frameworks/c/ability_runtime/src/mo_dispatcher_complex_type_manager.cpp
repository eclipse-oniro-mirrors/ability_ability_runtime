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

bool IsVariantHandleValid(const OH_AbilityRuntime_ModObjDispatcher_Variant* value)
{
    switch (value->vt) {
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY:
            return value->u.parrayVal != nullptr;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR:
            return value->u.pvectorVal != nullptr;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET:
            return value->u.psetVal != nullptr;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP:
            return value->u.pmapVal != nullptr;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRUCT:
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

// Validate that vt is a known, valid value type.
bool IsValidValueTypeForCreate(OH_AbilityRuntime_ModObjDispatcher_ValueType vt)
{
    switch (vt) {
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_BOOL:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I8:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I16:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I64:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U8:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U16:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U32:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U64:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F32:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F64:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ENUM:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRUCT:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_IPC_REMOTE_PROXY:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_IPC_REMOTE_STUB:
            return true;
        default:
            return false;
    }
}

// Validate that keyType is a simple/comparable type (not a complex container).
bool IsValidMapKeyType(OH_AbilityRuntime_ModObjDispatcher_ValueType keyType)
{
    switch (keyType) {
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_BOOL:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I8:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I16:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I64:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U8:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U16:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U32:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U64:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F32:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F64:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ENUM:
            return true;
        default:
            return false;
    }
}

// Forward declaration for deep comparison
bool VariantDeepEquals(const MoVariantStorage& lhs, const OH_AbilityRuntime_ModObjDispatcher_Variant* rhs,
    std::unordered_set<const void*>& visited);

// Recursively compare array contents
bool ArrayDeepEquals(const OH_AbilityRuntime_ModularObjectDispatcher_Array& lhs,
    const OH_AbilityRuntime_ModularObjectDispatcher_Array& rhs, std::unordered_set<const void*>& visited)
{
    if (lhs.elements.size() != rhs.elements.size()) {
        return false;
    }
    if (lhs.elementTypeInfo == nullptr || rhs.elementTypeInfo == nullptr) {
        return lhs.elementTypeInfo == rhs.elementTypeInfo;
    }
    if (lhs.elementTypeInfo->vt != rhs.elementTypeInfo->vt) {
        return false;
    }
    for (size_t i = 0; i < lhs.elements.size(); i++) {
        OH_AbilityRuntime_ModObjDispatcher_Variant rhsVar = rhs.elements[i].value;
        if (!VariantDeepEquals(lhs.elements[i], &rhsVar, visited)) {
            return false;
        }
    }
    return true;
}

// Recursively compare map contents
bool MapDeepEquals(const OH_AbilityRuntime_ModularObjectDispatcher_Map& lhs,
    const OH_AbilityRuntime_ModularObjectDispatcher_Map& rhs, std::unordered_set<const void*>& visited)
{
    if (lhs.entries.size() != rhs.entries.size()) {
        return false;
    }
    if (lhs.keyType != rhs.keyType) {
        return false;
    }
    for (size_t i = 0; i < lhs.entries.size(); i++) {
        OH_AbilityRuntime_ModObjDispatcher_Variant rhsKey = rhs.entries[i].first.value;
        if (!VariantDeepEquals(lhs.entries[i].first, &rhsKey, visited)) {
            return false;
        }
        OH_AbilityRuntime_ModObjDispatcher_Variant rhsVal = rhs.entries[i].second.value;
        if (!VariantDeepEquals(lhs.entries[i].second, &rhsVal, visited)) {
            return false;
        }
    }
    return true;
}

// Deep variant comparison (handles nested containers by content, not pointer)
bool VariantDeepEquals(const MoVariantStorage& lhs, const OH_AbilityRuntime_ModObjDispatcher_Variant* rhs,
    std::unordered_set<const void*>& visited)
{
    if (rhs == nullptr) {
        return false;
    }
    if (lhs.value.vt != rhs->vt) {
        return false;
    }
    switch (lhs.value.vt) {
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VOID:
            return true;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_BOOL:
            return lhs.value.u.boolVal == rhs->u.boolVal;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I8:
            return lhs.value.u.i8Val == rhs->u.i8Val;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I16:
            return lhs.value.u.i16Val == rhs->u.i16Val;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I32:
            return lhs.value.u.i32Val == rhs->u.i32Val;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_I64:
            return lhs.value.u.i64Val == rhs->u.i64Val;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U8:
            return lhs.value.u.u8Val == rhs->u.u8Val;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U16:
            return lhs.value.u.u16Val == rhs->u.u16Val;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U32:
            return lhs.value.u.u32Val == rhs->u.u32Val;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_U64:
            return lhs.value.u.u64Val == rhs->u.u64Val;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F32:
            return lhs.value.u.f32Val == rhs->u.f32Val;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_F64:
            return lhs.value.u.f64Val == rhs->u.f64Val;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING: {
            const char* lhsStr = lhs.stringStorage.c_str();
            const char* rhsStr = (rhs->u.bstrVal != nullptr) ? rhs->u.bstrVal : "";
            return std::strcmp(lhsStr, rhsStr) == 0;
        }
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY: {
            if (lhs.value.u.parrayVal == nullptr || rhs->u.parrayVal == nullptr) {
                return lhs.value.u.parrayVal == rhs->u.parrayVal;
            }
            if (!visited.insert(lhs.value.u.parrayVal).second) {
                return true; // Already visiting this node, assume equal to avoid infinite recursion
            }
            bool result = ArrayDeepEquals(*lhs.value.u.parrayVal, *rhs->u.parrayVal, visited);
            visited.erase(lhs.value.u.parrayVal);
            return result;
        }
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR: {
            auto* lhsVec = lhs.value.u.pvectorVal;
            auto* rhsVec = rhs->u.pvectorVal;
            if (lhsVec == nullptr || rhsVec == nullptr) {
                return lhsVec == rhsVec;
            }
            if (lhsVec->elements.size() != rhsVec->elements.size()) {
                return false;
            }
            for (size_t i = 0; i < lhsVec->elements.size(); i++) {
                OH_AbilityRuntime_ModObjDispatcher_Variant rhsElem = rhsVec->elements[i].value;
                if (!VariantDeepEquals(lhsVec->elements[i], &rhsElem, visited)) {
                    return false;
                }
            }
            return true;
        }
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET: {
            auto* lhsSet = lhs.value.u.psetVal;
            auto* rhsSet = rhs->u.psetVal;
            if (lhsSet == nullptr || rhsSet == nullptr) {
                return lhsSet == rhsSet;
            }
            if (!visited.insert(lhsSet).second) {
                return true; // Already visiting, avoid infinite recursion
            }
            bool result = false;
            if (lhsSet->elements.size() == rhsSet->elements.size()) {
                result = true;
                std::vector<bool> matched(rhsSet->elements.size(), false);
                for (size_t i = 0; i < lhsSet->elements.size() && result; i++) {
                    bool found = false;
                    for (size_t j = 0; j < rhsSet->elements.size(); j++) {
                        if (!matched[j]) {
                            OH_AbilityRuntime_ModObjDispatcher_Variant rhsElem = rhsSet->elements[j].value;
                            if (VariantDeepEquals(lhsSet->elements[i], &rhsElem, visited)) {
                                matched[j] = true;
                                found = true;
                                break;
                            }
                        }
                    }
                    if (!found) {
                        result = false;
                    }
                }
            }
            visited.erase(lhsSet);
            return result;
        }
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP: {
            if (lhs.value.u.pmapVal == nullptr || rhs->u.pmapVal == nullptr) {
                return lhs.value.u.pmapVal == rhs->u.pmapVal;
            }
            if (!visited.insert(lhs.value.u.pmapVal).second) {
                return true;
            }
            bool result = MapDeepEquals(*lhs.value.u.pmapVal, *rhs->u.pmapVal, visited);
            visited.erase(lhs.value.u.pmapVal);
            return result;
        }
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRUCT: {
            if (lhs.value.u.pstructVal == nullptr || rhs->u.pstructVal == nullptr) {
                return lhs.value.u.pstructVal == rhs->u.pstructVal;
            }
            if (lhs.value.u.pstructVal->fields.size() != rhs->u.pstructVal->fields.size()) {
                return false;
            }
            for (const auto& field : lhs.value.u.pstructVal->fields) {
                auto rhsIter = rhs->u.pstructVal->fields.find(field.first);
                if (rhsIter == rhs->u.pstructVal->fields.end()) {
                    return false;
                }
                OH_AbilityRuntime_ModObjDispatcher_Variant rhsField = rhsIter->second.value;
                if (!VariantDeepEquals(field.second, &rhsField, visited)) {
                    return false;
                }
            }
            return true;
        }
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_IPC_REMOTE_PROXY:
            return lhs.value.u.premoteProxyVal == rhs->u.premoteProxyVal;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_IPC_REMOTE_STUB:
            return lhs.value.u.premoteStubVal == rhs->u.premoteStubVal;
        default:
            return false;
    }
}

// Recursively compare two MoTypeInfo trees for structural equality.
bool TypeInfoMatches(const std::shared_ptr<MoTypeInfo>& actual, const std::shared_ptr<MoTypeInfo>& expected)
{
    if (actual == nullptr || expected == nullptr) {
        return actual == expected;
    }
    if (actual->vt != expected->vt) {
        return false;
    }
    switch (actual->vt) {
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP:
            if (actual->mapKeyType != expected->mapKeyType) {
                return false;
            }
            return TypeInfoMatches(actual->pMapValueType, expected->pMapValueType);
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY:
            if (actual->arraySize != 0 && expected->arraySize != 0 && actual->arraySize != expected->arraySize) {
                return false; // Fixed-size array size mismatch
            }
            return TypeInfoMatches(actual->pElementType, expected->pElementType);
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET:
            return TypeInfoMatches(actual->pElementType, expected->pElementType);
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRUCT:
            // Struct identity is by idlType name
            return actual->idlType == expected->idlType;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ENUM:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_IPC_REMOTE_PROXY:
            return true; // idlType is informational, vt match is sufficient
        default:
            return true; // Simple types match by vt alone
    }
}

// Deep variant type validation — checks that the variant's internal structure
// matches the full expected MoTypeInfo tree, not just the top-level vt.
AbilityRuntime_ErrorCode ValidateVariantTypeDeep(
    const OH_AbilityRuntime_ModObjDispatcher_Variant* value, const std::shared_ptr<MoTypeInfo>& expectedInfo)
{
    if (value == nullptr || expectedInfo == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "ValidateVariantTypeDeep: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    // Top-level vt check
    if (value->vt != expectedInfo->vt) {
        // IPC proxy/stub interchange
        if ((expectedInfo->vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_IPC_REMOTE_PROXY &&
            value->vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_IPC_REMOTE_STUB) ||
            (expectedInfo->vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_IPC_REMOTE_STUB &&
            value->vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_IPC_REMOTE_PROXY)) {
            if (!IsVariantHandleValid(value)) {
                return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
            }
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
        TAG_LOGE(AAFwkTag::EXT, "ValidateVariantTypeDeep: vt mismatch, actual=%{public}d, expected=%{public}d",
            static_cast<int32_t>(value->vt), static_cast<int32_t>(expectedInfo->vt));
        return ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH;
    }
    if (!IsVariantHandleValid(value)) {
        TAG_LOGE(AAFwkTag::EXT, "ValidateVariantTypeDeep: vt=%{public}d but required handle is null",
            static_cast<int32_t>(value->vt));
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    // For complex types, validate the nested TypeInfo structure
    switch (value->vt) {
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY: {
            auto* arr = value->u.parrayVal;
            if (arr == nullptr || arr->elementTypeInfo == nullptr) {
                return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
            }
            if (expectedInfo->pElementType == nullptr) {
                return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
            }
            if (!TypeInfoMatches(arr->elementTypeInfo, expectedInfo->pElementType)) {
                TAG_LOGE(AAFwkTag::EXT, "ValidateVariantTypeDeep: array element type mismatch");
                return ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH;
            }
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR: {
            auto* vec = value->u.pvectorVal;
            if (vec == nullptr || vec->elementTypeInfo == nullptr) {
                return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
            }
            if (expectedInfo->pElementType == nullptr) {
                return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
            }
            if (!TypeInfoMatches(vec->elementTypeInfo, expectedInfo->pElementType)) {
                TAG_LOGE(AAFwkTag::EXT, "ValidateVariantTypeDeep: vector element type mismatch");
                return ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH;
            }
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET: {
            auto* set = value->u.psetVal;
            if (set == nullptr || set->elementTypeInfo == nullptr) {
                return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
            }
            if (expectedInfo->pElementType == nullptr) {
                return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
            }
            if (!TypeInfoMatches(set->elementTypeInfo, expectedInfo->pElementType)) {
                TAG_LOGE(AAFwkTag::EXT, "ValidateVariantTypeDeep: set element type mismatch");
                return ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH;
            }
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP: {
            auto* map = value->u.pmapVal;
            if (map == nullptr) {
                return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
            }
            // Validate keyType
            if (map->keyType != expectedInfo->mapKeyType) {
                TAG_LOGE(AAFwkTag::EXT, "ValidateVariantTypeDeep: map keyType mismatch, "
                    "actual=%{public}d, expected=%{public}d",
                    static_cast<int32_t>(map->keyType), static_cast<int32_t>(expectedInfo->mapKeyType));
                return ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH;
            }
            // Validate value TypeInfo
            if (map->valueTypeInfo != nullptr && expectedInfo->pMapValueType != nullptr) {
                if (!TypeInfoMatches(map->valueTypeInfo, expectedInfo->pMapValueType)) {
                    TAG_LOGE(AAFwkTag::EXT, "ValidateVariantTypeDeep: map value type mismatch");
                    return ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH;
                }
            }
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRUCT: {
            auto* st = value->u.pstructVal;
            if (st == nullptr) {
                return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
            }
            // Struct identity check by name
            if (!expectedInfo->idlType.empty() && st->name != expectedInfo->idlType) {
                TAG_LOGE(AAFwkTag::EXT, "ValidateVariantTypeDeep: struct name mismatch, "
                    "actual='%{public}s', expected='%{public}s'",
                    st->name.c_str(), expectedInfo->idlType.c_str());
                return ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH;
            }
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
        default:
            // Simple types (bool, i32, string, enum, etc.) — vt match is sufficient
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
}

AbilityRuntime_ErrorCode CopyStringToBuffer(const std::string& src, char* dst, uint32_t maxLen)
{
    if (dst == nullptr || maxLen == 0) {
        TAG_LOGE(AAFwkTag::EXT, "CopyStringToBuffer: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (src.size() + 1 > maxLen) {
        TAG_LOGE(AAFwkTag::EXT, "CopyStringToBuffer: buffer too small, need=%{public}zu, maxLen=%{public}u",
            src.size() + 1, maxLen);
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (strcpy_s(dst, maxLen, src.c_str()) != EOK) {
        TAG_LOGE(AAFwkTag::EXT, "CopyStringToBuffer: strcpy_s failed");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

MoVariantStorage CreateDefaultVariantStorage(OH_AbilityRuntime_ModObjDispatcher_ValueType type)
{
    MoVariantStorage storage;
    storage.value.vt = type;
    if (type == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING) {
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
    if (src.value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING) {
        dst.stringStorage = src.stringStorage;
        dst.value.u.bstrVal = const_cast<char*>(dst.stringStorage.c_str());
    } else if (src.value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY && src.value.u.parrayVal != nullptr) {
        ScopedVisited sv(visited, src.value.u.parrayVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "DeepCopyStorage: circular reference detected in array");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldArray = src.value.u.parrayVal;
        auto* newArray = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher_Array();
        if (newArray == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "DeepCopyStorage: allocate array failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        newArray->elementTypeInfo = oldArray->elementTypeInfo;
        newArray->elements.reserve(oldArray->elements.size());
        for (const auto& elem : oldArray->elements) {
            MoVariantStorage elemCopy;
            auto ret = DeepCopyStorage(elem, elemCopy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newArray;
                return ret;
            }
            newArray->elements.emplace_back(std::move(elemCopy));
        }
        dst.value.u.parrayVal = newArray;
    } else if (src.value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR && src.value.u.pvectorVal != nullptr) {
        ScopedVisited sv(visited, src.value.u.pvectorVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "DeepCopyStorage: circular reference detected in vector");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldVector = src.value.u.pvectorVal;
        auto* newVector = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher_Vector();
        if (newVector == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "DeepCopyStorage: allocate vector failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        newVector->elementTypeInfo = oldVector->elementTypeInfo;
        newVector->elements.reserve(oldVector->elements.size());
        for (const auto& elem : oldVector->elements) {
            MoVariantStorage elemCopy;
            auto ret = DeepCopyStorage(elem, elemCopy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newVector;
                return ret;
            }
            newVector->elements.emplace_back(std::move(elemCopy));
        }
        dst.value.u.pvectorVal = newVector;
    } else if (src.value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET && src.value.u.psetVal != nullptr) {
        ScopedVisited sv(visited, src.value.u.psetVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "DeepCopyStorage: circular reference detected in set");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldSet = src.value.u.psetVal;
        auto* newSet = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher_Set();
        if (newSet == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "DeepCopyStorage: allocate set failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        newSet->elementTypeInfo = oldSet->elementTypeInfo;
        newSet->elements.reserve(oldSet->elements.size());
        for (const auto& elem : oldSet->elements) {
            MoVariantStorage elemCopy;
            auto ret = DeepCopyStorage(elem, elemCopy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newSet;
                return ret;
            }
            newSet->elements.emplace_back(std::move(elemCopy));
        }
        dst.value.u.psetVal = newSet;
    } else if (src.value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP && src.value.u.pmapVal != nullptr) {
        ScopedVisited sv(visited, src.value.u.pmapVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "DeepCopyStorage: circular reference detected in map");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldMap = src.value.u.pmapVal;
        auto* newMap = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher_Map();
        if (newMap == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "DeepCopyStorage: allocate map failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        newMap->keyType = oldMap->keyType;
        newMap->valueTypeInfo = oldMap->valueTypeInfo;
        newMap->entries.reserve(oldMap->entries.size());
        for (const auto& entry : oldMap->entries) {
            std::pair<MoVariantStorage, MoVariantStorage> entryCopy;
            auto ret = DeepCopyStorage(entry.first, entryCopy.first, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newMap;
                return ret;
            }
            ret = DeepCopyStorage(entry.second, entryCopy.second, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newMap;
                return ret;
            }
            newMap->entries.emplace_back(std::move(entryCopy));
        }
        dst.value.u.pmapVal = newMap;
    } else if (src.value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRUCT && src.value.u.pstructVal != nullptr) {
        ScopedVisited sv(visited, src.value.u.pstructVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "DeepCopyStorage: circular reference detected in struct");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldStruct = src.value.u.pstructVal;
        auto* newStruct = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher_Struct();
        if (newStruct == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "DeepCopyStorage: allocate struct failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        };
        newStruct->name = oldStruct->name;
        newStruct->fieldTypes = oldStruct->fieldTypes;
        for (const auto& field : oldStruct->fields) {
            MoVariantStorage fieldCopy;
            auto ret = DeepCopyStorage(field.second, fieldCopy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newStruct;
                return ret;
            }
            newStruct->fields[field.first] = std::move(fieldCopy);
        }
        dst.value.u.pstructVal = newStruct;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}
} // namespace

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::ArrayCreate(
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo* elementType, uint32_t size,
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle* ppArray)
{
    if (ppArray == nullptr || elementType == nullptr || *ppArray != nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "ArrayCreate: null param or handle already initialized");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (!IsValidValueTypeForCreate(elementType->vt)) {
        TAG_LOGE(AAFwkTag::EXT, "ArrayCreate: invalid vt=%{public}d", static_cast<int32_t>(elementType->vt));
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto typeInfo = MoTypeInfo::FromCTypeInfo(elementType);
    if (typeInfo == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "ArrayCreate: FromCTypeInfo failed");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* array = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher_Array();
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

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::ArrayGetElementType(
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle pArray, OH_AbilityRuntime_ModObjDispatcher_TypeInfo* pElementType)
{
    if (pArray == nullptr || pElementType == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "ArrayGetElementType: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (pArray->elementTypeInfo == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "ArrayGetElementType: elementTypeInfo is null");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    pArray->elementTypeInfo->FillCTypeInfo(pElementType);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::ArraySet(OH_AbilityRuntime_ModObjDispatcher_ArrayHandle pArray,
    uint32_t index, const OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    if (pArray == nullptr || pValue == nullptr || index >= pArray->elements.size()) {
        TAG_LOGE(AAFwkTag::EXT, "ArraySet: null param or index out of range");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = ValidateVariantTypeDeep(pValue, pArray->elementTypeInfo);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "ArraySet: type mismatch at index=%{public}u", index);
        return ret;
    }
    return StoreVariant(pValue, &pArray->elements[index]);
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::ArrayGet(OH_AbilityRuntime_ModObjDispatcher_ArrayHandle pArray,
    uint32_t index, OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    if (pArray == nullptr || pValue == nullptr || index >= pArray->elements.size()) {
        TAG_LOGE(AAFwkTag::EXT, "ArrayGet: null param or index out of range");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return LoadVariant(pArray->elements[index], pValue);
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::ArrayGetSize(OH_AbilityRuntime_ModObjDispatcher_ArrayHandle pArray,
    uint32_t* pSize)
{
    if (pArray == nullptr || pSize == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "ArrayGetSize: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pSize = static_cast<uint32_t>(pArray->elements.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void ModObjDispatcherComplexTypeManager::ArrayRelease(OH_AbilityRuntime_ModObjDispatcher_ArrayHandle* ppArray)
{
    if (ppArray == nullptr || *ppArray == nullptr) {
        return;
    }
    delete *ppArray;
    *ppArray = nullptr;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::VectorCreate(
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo* elementType, OH_AbilityRuntime_ModObjDispatcher_VectorHandle* ppVector)
{
    if (ppVector == nullptr || elementType == nullptr || *ppVector != nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "VectorCreate: null param or handle already initialized");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (!IsValidValueTypeForCreate(elementType->vt)) {
        TAG_LOGE(AAFwkTag::EXT, "VectorCreate: invalid vt=%{public}d", static_cast<int32_t>(elementType->vt));
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto typeInfo = MoTypeInfo::FromCTypeInfo(elementType);
    if (typeInfo == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "VectorCreate: FromCTypeInfo failed");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* vector = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher_Vector();
    if (vector == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "VectorCreate: allocate failed");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    vector->elementTypeInfo = typeInfo;
    *ppVector = vector;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::VectorGetElementType(
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle pVector, OH_AbilityRuntime_ModObjDispatcher_TypeInfo* pElementType)
{
    if (pVector == nullptr || pElementType == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "VectorGetElementType: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (pVector->elementTypeInfo == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "VectorGetElementType: elementTypeInfo is null");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    pVector->elementTypeInfo->FillCTypeInfo(pElementType);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::VectorAdd(OH_AbilityRuntime_ModObjDispatcher_VectorHandle pVector,
    const OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    if (pVector == nullptr || pValue == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "VectorAdd: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = ValidateVariantTypeDeep(pValue, pVector->elementTypeInfo);
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

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::VectorGet(OH_AbilityRuntime_ModObjDispatcher_VectorHandle pVector,
    uint32_t index, OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    if (pVector == nullptr || pValue == nullptr || index >= pVector->elements.size()) {
        TAG_LOGE(AAFwkTag::EXT, "VectorGet: null param or index out of range");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return LoadVariant(pVector->elements[index], pValue);
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::VectorGetSize(
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle pVector, uint32_t* pSize)
{
    if (pVector == nullptr || pSize == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "VectorGetSize: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pSize = static_cast<uint32_t>(pVector->elements.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::VectorClear(
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle pVector)
{
    if (pVector == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "VectorClear: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    pVector->elements.clear();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void ModObjDispatcherComplexTypeManager::VectorRelease(OH_AbilityRuntime_ModObjDispatcher_VectorHandle* ppVector)
{
    if (ppVector == nullptr || *ppVector == nullptr) {
        return;
    }
    delete *ppVector;
    *ppVector = nullptr;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::SetCreate(
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo* elementType, OH_AbilityRuntime_ModObjDispatcher_SetHandle* ppSet)
{
    if (ppSet == nullptr || elementType == nullptr || *ppSet != nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "SetCreate: null param or handle already initialized");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (!IsValidValueTypeForCreate(elementType->vt)) {
        TAG_LOGE(AAFwkTag::EXT, "SetCreate: invalid vt=%{public}d", static_cast<int32_t>(elementType->vt));
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto typeInfo = MoTypeInfo::FromCTypeInfo(elementType);
    if (typeInfo == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "SetCreate: FromCTypeInfo failed");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* set = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher_Set();
    if (set == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "SetCreate: allocate failed");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    set->elementTypeInfo = typeInfo;
    *ppSet = set;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::SetGetElementType(
    OH_AbilityRuntime_ModObjDispatcher_SetHandle pSet, OH_AbilityRuntime_ModObjDispatcher_TypeInfo* pElementType)
{
    if (pSet == nullptr || pElementType == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "SetGetElementType: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (pSet->elementTypeInfo == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "SetGetElementType: elementTypeInfo is null");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    pSet->elementTypeInfo->FillCTypeInfo(pElementType);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::SetAdd(OH_AbilityRuntime_ModObjDispatcher_SetHandle pSet,
    const OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    if (pSet == nullptr || pValue == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "SetAdd: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = ValidateVariantTypeDeep(pValue, pSet->elementTypeInfo);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "SetAdd: type mismatch");
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

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::SetRemove(OH_AbilityRuntime_ModObjDispatcher_SetHandle pSet,
    const OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    if (pSet == nullptr || pValue == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "SetRemove: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = ValidateVariantTypeDeep(pValue, pSet->elementTypeInfo);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "SetRemove: type mismatch");
        return ret;
    }
    auto iter = std::find_if(pSet->elements.begin(), pSet->elements.end(), [pValue](const MoVariantStorage& item) {
        return VariantEquals(item, pValue);
    });
    if (iter == pSet->elements.end()) {
        TAG_LOGE(AAFwkTag::EXT, "SetRemove: element not found");
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    pSet->elements.erase(iter);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::SetContains(OH_AbilityRuntime_ModObjDispatcher_SetHandle pSet,
    const OH_AbilityRuntime_ModObjDispatcher_Variant* pValue, bool* pExists)
{
    if (pSet == nullptr || pValue == nullptr || pExists == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "SetContains: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = ValidateVariantTypeDeep(pValue, pSet->elementTypeInfo);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "SetContains: type mismatch");
        return ret;
    }
    *pExists = std::any_of(pSet->elements.begin(), pSet->elements.end(), [pValue](const MoVariantStorage& item) {
        return VariantEquals(item, pValue);
    });
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::SetGetSize(OH_AbilityRuntime_ModObjDispatcher_SetHandle pSet,
    uint32_t* pSize)
{
    if (pSet == nullptr || pSize == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "SetGetSize: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pSize = static_cast<uint32_t>(pSet->elements.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::SetGetAt(OH_AbilityRuntime_ModObjDispatcher_SetHandle pSet,
    uint32_t index, OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    if (pSet == nullptr || pValue == nullptr || index >= pSet->elements.size()) {
        TAG_LOGE(AAFwkTag::EXT, "SetGetAt: null param or index out of range");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return LoadVariant(pSet->elements[index], pValue);
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::SetClear(OH_AbilityRuntime_ModObjDispatcher_SetHandle pSet)
{
    if (pSet == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "SetClear: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    pSet->elements.clear();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void ModObjDispatcherComplexTypeManager::SetRelease(OH_AbilityRuntime_ModObjDispatcher_SetHandle* ppSet)
{
    if (ppSet == nullptr || *ppSet == nullptr) {
        return;
    }
    delete *ppSet;
    *ppSet = nullptr;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::MapCreate(
    OH_AbilityRuntime_ModObjDispatcher_ValueType keyType, OH_AbilityRuntime_ModObjDispatcher_TypeInfo* valueType,
    OH_AbilityRuntime_ModObjDispatcher_MapHandle* ppMap)
{
    if (ppMap == nullptr || valueType == nullptr || *ppMap != nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "MapCreate: null param or handle already initialized");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (!IsValidMapKeyType(keyType)) {
        TAG_LOGE(AAFwkTag::EXT, "MapCreate: keyType=%{public}d is not a valid map key type",
            static_cast<int32_t>(keyType));
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto valueTypeInfo = MoTypeInfo::FromCTypeInfo(valueType);
    if (valueTypeInfo == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "MapCreate: FromCTypeInfo failed");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* map = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher_Map();
    if (map == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "MapCreate: allocate failed");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    map->keyType = keyType;
    map->valueTypeInfo = valueTypeInfo;
    *ppMap = map;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::MapGetKeyType(
    OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap, OH_AbilityRuntime_ModObjDispatcher_ValueType* pKeyType)
{
    if (pMap == nullptr || pKeyType == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "MapGetKeyType: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pKeyType = pMap->keyType;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::MapGetValueType(
    OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap, OH_AbilityRuntime_ModObjDispatcher_TypeInfo* pValueType)
{
    if (pMap == nullptr || pValueType == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "MapGetValueType: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (pMap->valueTypeInfo == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "MapGetValueType: valueTypeInfo is null");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    pMap->valueTypeInfo->FillCTypeInfo(pValueType);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::MapPut(OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap,
    const OH_AbilityRuntime_ModObjDispatcher_Variant* pKey, const OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    if (pMap == nullptr || pKey == nullptr || pValue == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "MapPut: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = ValidateVariantType(pKey, pMap->keyType);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "MapPut: key type mismatch");
        return ret;
    }
    ret = ValidateVariantTypeDeep(pValue, pMap->valueTypeInfo);
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

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::MapGet(OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap,
    const OH_AbilityRuntime_ModObjDispatcher_Variant* pKey, OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    if (pMap == nullptr || pKey == nullptr || pValue == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "MapGet: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = ValidateVariantType(pKey, pMap->keyType);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "MapGet: key type mismatch");
        return ret;
    }
    for (const auto& entry : pMap->entries) {
        if (VariantEquals(entry.first, pKey)) {
            return LoadVariant(entry.second, pValue);
        }
    }
    TAG_LOGE(AAFwkTag::EXT, "MapGet: key not found");
    return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::MapRemove(OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap,
    const OH_AbilityRuntime_ModObjDispatcher_Variant* pKey)
{
    if (pMap == nullptr || pKey == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "MapRemove: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = ValidateVariantType(pKey, pMap->keyType);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "MapRemove: key type mismatch");
        return ret;
    }
    auto iter = std::find_if(pMap->entries.begin(), pMap->entries.end(), [pKey](const auto& item) {
        return VariantEquals(item.first, pKey);
    });
    if (iter == pMap->entries.end()) {
        TAG_LOGE(AAFwkTag::EXT, "MapRemove: key not found");
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    pMap->entries.erase(iter);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::MapContainsKey(OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap,
    const OH_AbilityRuntime_ModObjDispatcher_Variant* pKey, bool* pExists)
{
    if (pMap == nullptr || pKey == nullptr || pExists == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "MapContainsKey: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = ValidateVariantType(pKey, pMap->keyType);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "MapContainsKey: key type mismatch");
        return ret;
    }
    *pExists = std::any_of(pMap->entries.begin(), pMap->entries.end(), [pKey](const auto& item) {
        return VariantEquals(item.first, pKey);
    });
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::MapGetSize(OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap,
    uint32_t* pSize)
{
    if (pMap == nullptr || pSize == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "MapGetSize: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pSize = static_cast<uint32_t>(pMap->entries.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::MapGetKeyAt(OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap,
    uint32_t index, OH_AbilityRuntime_ModObjDispatcher_Variant* pKey)
{
    if (pMap == nullptr || pKey == nullptr || index >= pMap->entries.size()) {
        TAG_LOGE(AAFwkTag::EXT, "MapGetKeyAt: null param or index out of range");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return LoadVariant(pMap->entries[index].first, pKey);
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::MapGetValueAt(OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap,
    uint32_t index, OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    if (pMap == nullptr || pValue == nullptr || index >= pMap->entries.size()) {
        TAG_LOGE(AAFwkTag::EXT, "MapGetValueAt: null param or index out of range");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return LoadVariant(pMap->entries[index].second, pValue);
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::MapClear(OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap)
{
    if (pMap == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "MapClear: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    pMap->entries.clear();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void ModObjDispatcherComplexTypeManager::MapRelease(OH_AbilityRuntime_ModObjDispatcher_MapHandle* ppMap)
{
    if (ppMap == nullptr || *ppMap == nullptr) {
        return;
    }
    delete *ppMap;
    *ppMap = nullptr;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::StructCreate(const char* structName,
    OH_AbilityRuntime_ModObjDispatcher_StructHandle* ppStruct)
{
    if (structName == nullptr || ppStruct == nullptr || *ppStruct != nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "StructCreate: null param or handle already initialized");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* object = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher_Struct();
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

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::StructGetName(
    OH_AbilityRuntime_ModObjDispatcher_StructHandle pStruct, char* pbstrName, uint32_t cMaxName)
{
    if (pStruct == nullptr || pbstrName == nullptr || cMaxName == 0) {
        TAG_LOGE(AAFwkTag::EXT, "StructGetName: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return CopyStringToBuffer(pStruct->name, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::StructSetField(
    OH_AbilityRuntime_ModObjDispatcher_StructHandle pStruct, const char* szName,
    const OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    if (pStruct == nullptr || szName == nullptr || pValue == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "StructSetField: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto fieldTypeIter = pStruct->fieldTypes.find(szName);
    if (fieldTypeIter == pStruct->fieldTypes.end()) {
        TAG_LOGE(AAFwkTag::EXT, "StructSetField: field '%{public}s' not found in struct '%{public}s'",
            szName, pStruct->name.c_str());
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    auto ret = ValidateVariantTypeDeep(pValue, fieldTypeIter->second);
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

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::StructGetField(
    OH_AbilityRuntime_ModObjDispatcher_StructHandle pStruct, const char* szName,
    OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    if (pStruct == nullptr || szName == nullptr || pValue == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "StructGetField: null param");
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

void ModObjDispatcherComplexTypeManager::StructRelease(OH_AbilityRuntime_ModObjDispatcher_StructHandle* ppStruct)
{
    if (ppStruct == nullptr || *ppStruct == nullptr) {
        return;
    }
    delete *ppStruct;
    *ppStruct = nullptr;
}

void ModObjDispatcherComplexTypeManager::Variant_Clear(OH_AbilityRuntime_ModObjDispatcher_Variant* pVariant)
{
    if (pVariant == nullptr) {
        return;
    }
    switch (pVariant->vt) {
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING:
            if (pVariant->u.bstrVal != nullptr) {
                std::free(pVariant->u.bstrVal);
                pVariant->u.bstrVal = nullptr;
            }
            break;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY:
            if (pVariant->u.parrayVal != nullptr) {
                ArrayRelease(&pVariant->u.parrayVal);
            }
            break;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR:
            if (pVariant->u.pvectorVal != nullptr) {
                VectorRelease(&pVariant->u.pvectorVal);
            }
            break;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET:
            if (pVariant->u.psetVal != nullptr) {
                SetRelease(&pVariant->u.psetVal);
            }
            break;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP:
            if (pVariant->u.pmapVal != nullptr) {
                MapRelease(&pVariant->u.pmapVal);
            }
            break;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRUCT:
            if (pVariant->u.pstructVal != nullptr) {
                StructRelease(&pVariant->u.pstructVal);
            }
            break;
        default:
            break;
    }
    (void)memset_s(pVariant, sizeof(OH_AbilityRuntime_ModObjDispatcher_Variant), 0,
        sizeof(OH_AbilityRuntime_ModObjDispatcher_Variant));
    pVariant->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY;
}

void ModObjDispatcherComplexTypeManager::TypeInfo_Clear(OH_AbilityRuntime_ModObjDispatcher_TypeInfo* pTypeInfo)
{
    MoTypeInfo::ClearCTypeInfo(pTypeInfo);
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::StoreVariant(
    const OH_AbilityRuntime_ModObjDispatcher_Variant* src, MoVariantStorage* dst)
{
    if (src == nullptr || dst == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "StoreVariant: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    // Release old nested resources before overwriting to avoid leak
    if (dst->value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING) {
        dst->stringStorage.clear();
        dst->value.u.bstrVal = nullptr;
    } else if (dst->value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY && dst->value.u.parrayVal != nullptr) {
        delete dst->value.u.parrayVal;
        dst->value.u.parrayVal = nullptr;
    } else if (dst->value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR && dst->value.u.pvectorVal != nullptr) {
        delete dst->value.u.pvectorVal;
        dst->value.u.pvectorVal = nullptr;
    } else if (dst->value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET && dst->value.u.psetVal != nullptr) {
        delete dst->value.u.psetVal;
        dst->value.u.psetVal = nullptr;
    } else if (dst->value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP && dst->value.u.pmapVal != nullptr) {
        delete dst->value.u.pmapVal;
        dst->value.u.pmapVal = nullptr;
    } else if (dst->value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRUCT && dst->value.u.pstructVal != nullptr) {
        delete dst->value.u.pstructVal;
        dst->value.u.pstructVal = nullptr;
    }
    dst->value = *src;
    dst->stringStorage.clear();
    std::unordered_set<const void*> visited;
    if (src->vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING) {
        dst->stringStorage = (src->u.bstrVal != nullptr) ? src->u.bstrVal : "";
        dst->value.u.bstrVal = const_cast<char*>(dst->stringStorage.c_str());
    } else if (src->vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY && src->u.parrayVal != nullptr) {
        ScopedVisited sv(visited, src->u.parrayVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "StoreVariant: circular reference detected in array");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldArray = src->u.parrayVal;
        auto* newArray = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher_Array();
        if (newArray == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "StoreVariant: allocate array failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        newArray->elementTypeInfo = oldArray->elementTypeInfo;
        newArray->elements.reserve(oldArray->elements.size());
        for (const auto& elem : oldArray->elements) {
            MoVariantStorage elemCopy;
            auto ret = DeepCopyStorage(elem, elemCopy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newArray;
                return ret;
            }
            newArray->elements.emplace_back(std::move(elemCopy));
        }
        dst->value.u.parrayVal = newArray;
    } else if (src->vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR && src->u.pvectorVal != nullptr) {
        ScopedVisited sv(visited, src->u.pvectorVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "StoreVariant: circular reference detected in vector");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldVector = src->u.pvectorVal;
        auto* newVector = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher_Vector();
        if (newVector == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "StoreVariant: allocate vector failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        newVector->elementTypeInfo = oldVector->elementTypeInfo;
        newVector->elements.reserve(oldVector->elements.size());
        for (const auto& elem : oldVector->elements) {
            MoVariantStorage elemCopy;
            auto ret = DeepCopyStorage(elem, elemCopy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newVector;
                return ret;
            }
            newVector->elements.emplace_back(std::move(elemCopy));
        }
        dst->value.u.pvectorVal = newVector;
    } else if (src->vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET && src->u.psetVal != nullptr) {
        ScopedVisited sv(visited, src->u.psetVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "StoreVariant: circular reference detected in set");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldSet = src->u.psetVal;
        auto* newSet = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher_Set();
        if (newSet == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "StoreVariant: allocate set failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        newSet->elementTypeInfo = oldSet->elementTypeInfo;
        newSet->elements.reserve(oldSet->elements.size());
        for (const auto& elem : oldSet->elements) {
            MoVariantStorage elemCopy;
            auto ret = DeepCopyStorage(elem, elemCopy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newSet;
                return ret;
            }
            newSet->elements.emplace_back(std::move(elemCopy));
        }
        dst->value.u.psetVal = newSet;
    } else if (src->vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP && src->u.pmapVal != nullptr) {
        ScopedVisited sv(visited, src->u.pmapVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "StoreVariant: circular reference detected in map");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldMap = src->u.pmapVal;
        auto* newMap = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher_Map();
        if (newMap == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "StoreVariant: allocate map failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        newMap->keyType = oldMap->keyType;
        newMap->valueTypeInfo = oldMap->valueTypeInfo;
        newMap->entries.reserve(oldMap->entries.size());
        for (const auto& entry : oldMap->entries) {
            std::pair<MoVariantStorage, MoVariantStorage> entryCopy;
            auto ret = DeepCopyStorage(entry.first, entryCopy.first, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newMap;
                return ret;
            }
            ret = DeepCopyStorage(entry.second, entryCopy.second, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newMap;
                return ret;
            }
            newMap->entries.emplace_back(std::move(entryCopy));
        }
        dst->value.u.pmapVal = newMap;
    } else if (src->vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRUCT && src->u.pstructVal != nullptr) {
        ScopedVisited sv(visited, src->u.pstructVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "StoreVariant: circular reference detected in struct");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldStruct = src->u.pstructVal;
        auto* newStruct = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher_Struct();
        if (newStruct == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "StoreVariant: allocate struct failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        newStruct->name = oldStruct->name;
        newStruct->fieldTypes = oldStruct->fieldTypes;
        for (const auto& field : oldStruct->fields) {
            MoVariantStorage fieldCopy;
            auto ret = DeepCopyStorage(field.second, fieldCopy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newStruct;
                return ret;
            }
            newStruct->fields[field.first] = std::move(fieldCopy);
        }
        dst->value.u.pstructVal = newStruct;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::LoadVariant(const MoVariantStorage& src,
    OH_AbilityRuntime_ModObjDispatcher_Variant* dst)
{
    if (dst == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "LoadVariant: null param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    // Do NOT assign src.value directly to dst first (that would leak internal pointers
    // if deep copy fails below). Instead, build result into a local temp and assign only on success.
    // For simple types, direct copy is safe. For complex types, construct into temp first.
    if (src.value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING) {
        const char* srcStr = src.stringStorage.c_str();
        size_t len = src.stringStorage.size();
        auto* mem = static_cast<char*>(std::malloc(len + 1));
        if (mem == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "LoadVariant: malloc failed for string, len=%{public}zu", len);
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        if (strcpy_s(mem, len + 1, srcStr) != EOK) {
            std::free(mem);
            TAG_LOGE(AAFwkTag::EXT, "LoadVariant: strcpy_s failed for string");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        (void)memset_s(dst, sizeof(*dst), 0, sizeof(*dst));
        dst->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING;
        dst->u.bstrVal = mem;
    } else if (src.value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY && src.value.u.parrayVal != nullptr) {
        std::unordered_set<const void*> visited;
        ScopedVisited sv(visited, src.value.u.parrayVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "LoadVariant: circular reference detected in array");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldArray = src.value.u.parrayVal;
        auto* newArray = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher_Array();
        if (newArray == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "LoadVariant: allocate array failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        newArray->elementTypeInfo = oldArray->elementTypeInfo;
        newArray->elements.reserve(oldArray->elements.size());
        for (const auto& elem : oldArray->elements) {
            MoVariantStorage copy;
            auto ret = DeepCopyStorage(elem, copy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newArray;
                return ret;
            }
            newArray->elements.emplace_back(std::move(copy));
        }
        (void)memset_s(dst, sizeof(*dst), 0, sizeof(*dst));
        dst->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY;
        dst->u.parrayVal = newArray;
    } else if (src.value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR && src.value.u.pvectorVal != nullptr) {
        std::unordered_set<const void*> visited;
        ScopedVisited sv(visited, src.value.u.pvectorVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "LoadVariant: circular reference detected in vector");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldVector = src.value.u.pvectorVal;
        auto* newVector = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher_Vector();
        if (newVector == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "LoadVariant: allocate vector failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        newVector->elementTypeInfo = oldVector->elementTypeInfo;
        newVector->elements.reserve(oldVector->elements.size());
        for (const auto& elem : oldVector->elements) {
            MoVariantStorage copy;
            auto ret = DeepCopyStorage(elem, copy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newVector;
                return ret;
            }
            newVector->elements.emplace_back(std::move(copy));
        }
        (void)memset_s(dst, sizeof(*dst), 0, sizeof(*dst));
        dst->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR;
        dst->u.pvectorVal = newVector;
    } else if (src.value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET && src.value.u.psetVal != nullptr) {
        std::unordered_set<const void*> visited;
        ScopedVisited sv(visited, src.value.u.psetVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "LoadVariant: circular reference detected in set");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldSet = src.value.u.psetVal;
        auto* newSet = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher_Set();
        if (newSet == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "LoadVariant: allocate set failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        newSet->elementTypeInfo = oldSet->elementTypeInfo;
        newSet->elements.reserve(oldSet->elements.size());
        for (const auto& elem : oldSet->elements) {
            MoVariantStorage copy;
            auto ret = DeepCopyStorage(elem, copy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newSet;
                return ret;
            }
            newSet->elements.emplace_back(std::move(copy));
        }
        (void)memset_s(dst, sizeof(*dst), 0, sizeof(*dst));
        dst->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET;
        dst->u.psetVal = newSet;
    } else if (src.value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP && src.value.u.pmapVal != nullptr) {
        std::unordered_set<const void*> visited;
        ScopedVisited sv(visited, src.value.u.pmapVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "LoadVariant: circular reference detected in map");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldMap = src.value.u.pmapVal;
        auto* newMap = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher_Map();
        if (newMap == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "LoadVariant: allocate map failed");
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
                return ret;
            }
            ret = DeepCopyStorage(entry.second, copy.second, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newMap;
                return ret;
            }
            newMap->entries.emplace_back(std::move(copy));
        }
        (void)memset_s(dst, sizeof(*dst), 0, sizeof(*dst));
        dst->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP;
        dst->u.pmapVal = newMap;
    } else if (src.value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRUCT && src.value.u.pstructVal != nullptr) {
        std::unordered_set<const void*> visited;
        ScopedVisited sv(visited, src.value.u.pstructVal);
        if (!sv) {
            TAG_LOGE(AAFwkTag::EXT, "LoadVariant: circular reference detected in struct");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* oldStruct = src.value.u.pstructVal;
        auto* newStruct = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher_Struct();
        if (newStruct == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "LoadVariant: allocate struct failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        newStruct->name = oldStruct->name;
        newStruct->fieldTypes = oldStruct->fieldTypes;
        for (const auto& field : oldStruct->fields) {
            MoVariantStorage copy;
            auto ret = DeepCopyStorage(field.second, copy, visited);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                delete newStruct;
                return ret;
            }
            newStruct->fields[field.first] = std::move(copy);
        }
        (void)memset_s(dst, sizeof(*dst), 0, sizeof(*dst));
        dst->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRUCT;
        dst->u.pstructVal = newStruct;
    } else {
        // Simple types (bool, i32, f64, enum, etc.) or null handles — safe to copy directly
        *dst = src.value;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode ModObjDispatcherComplexTypeManager::ValidateVariantType(
    const OH_AbilityRuntime_ModObjDispatcher_Variant* value, OH_AbilityRuntime_ModObjDispatcher_ValueType expectedType)
{
    if (value == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "ValidateVariantType: null param");
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
    if ((expectedType == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_IPC_REMOTE_PROXY &&
        value->vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_IPC_REMOTE_STUB) ||
        (expectedType == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_IPC_REMOTE_STUB &&
        value->vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_IPC_REMOTE_PROXY)) {
        if (!IsVariantHandleValid(value)) {
            TAG_LOGE(AAFwkTag::EXT, "ValidateVariantType: vt=%{public}d but required handle is null",
                static_cast<int32_t>(value->vt));
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    TAG_LOGE(AAFwkTag::EXT, "ValidateVariantType: type mismatch, actual=%{public}d, expected=%{public}d",
        static_cast<int32_t>(value->vt), static_cast<int32_t>(expectedType));
    return ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH;
}

bool ModObjDispatcherComplexTypeManager::VariantEquals(const MoVariantStorage& lhs,
    const OH_AbilityRuntime_ModObjDispatcher_Variant* rhs)
{
    std::unordered_set<const void*> visited;
    return VariantDeepEquals(lhs, rhs, visited);
}

void ModObjDispatcherComplexTypeManager::RegisterStructMetadata(const std::vector<MoStructMeta>& structs)
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

bool ModObjDispatcherComplexTypeManager::GetStructFieldType(const std::string& structName, const std::string& fieldName,
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

bool ModObjDispatcherComplexTypeManager::GetStructFieldNames(const std::string& structName,
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

// MoVariantStorage member functions — must be defined here where struct types are complete
MoVariantStorage::~MoVariantStorage()
{
    ReleaseNestedResources();
}

MoVariantStorage::MoVariantStorage(MoVariantStorage&& other) noexcept
    : value(other.value), stringStorage(std::move(other.stringStorage))
{
    if (value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING) {
        value.u.bstrVal = const_cast<char*>(stringStorage.c_str());
    }
    (void)memset_s(&other.value, sizeof(other.value), 0, sizeof(other.value));
    other.stringStorage.clear();
}

MoVariantStorage& MoVariantStorage::operator=(MoVariantStorage&& other) noexcept
{
    if (this != &other) {
        ReleaseNestedResources();
        value = other.value;
        stringStorage = std::move(other.stringStorage);
        if (value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING) {
            value.u.bstrVal = const_cast<char*>(stringStorage.c_str());
        }
        (void)memset_s(&other.value, sizeof(other.value), 0, sizeof(other.value));
        other.stringStorage.clear();
    }
    return *this;
}

void MoVariantStorage::ReleaseNestedResources()
{
    if (value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRING) {
        stringStorage.clear();
    } else if (value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY && value.u.parrayVal != nullptr) {
        delete value.u.parrayVal;
        value.u.parrayVal = nullptr;
    } else if (value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR && value.u.pvectorVal != nullptr) {
        delete value.u.pvectorVal;
        value.u.pvectorVal = nullptr;
    } else if (value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET && value.u.psetVal != nullptr) {
        delete value.u.psetVal;
        value.u.psetVal = nullptr;
    } else if (value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP && value.u.pmapVal != nullptr) {
        delete value.u.pmapVal;
        value.u.pmapVal = nullptr;
    } else if (value.vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRUCT && value.u.pstructVal != nullptr) {
        delete value.u.pstructVal;
        value.u.pstructVal = nullptr;
    }
}

} // namespace OHOS::AbilityRuntime
