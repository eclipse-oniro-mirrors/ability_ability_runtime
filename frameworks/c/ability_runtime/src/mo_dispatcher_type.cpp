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

#include "mo_dispatcher_types.h"

#include <cstdlib>
#include <cstring>
#include <utility>

#include "securec.h"

namespace OHOS::AbilityRuntime {

// ---- MoTypeInfo ----

void MoTypeInfo::FillCTypeInfo(OH_AbilityRuntime_ModObjDispatcher_TypeInfo* cType) const
{
    if (cType == nullptr) return;
    (void)memset_s(cType, sizeof(OH_AbilityRuntime_ModObjDispatcher_TypeInfo), 0,
        sizeof(OH_AbilityRuntime_ModObjDispatcher_TypeInfo));
    cType->vt = vt;
    if (vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP) {
        cType->u.mapType.keyType = mapKeyType;
        if (pMapValueType) {
            cType->u.mapType.pValueType = new (std::nothrow) OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
            if (cType->u.mapType.pValueType != nullptr) {
                pMapValueType->FillCTypeInfo(cType->u.mapType.pValueType);
            }
        }
    } else if (vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY) {
        if (pElementType) {
            cType->u.arrayType.pElementType = new (std::nothrow) OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
            if (cType->u.arrayType.pElementType != nullptr) {
                pElementType->FillCTypeInfo(cType->u.arrayType.pElementType);
            }
        }
        cType->u.arrayType.size = arraySize;
    } else if (vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR ||
               vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET) {
        if (pElementType) {
            cType->u.pElementType = new (std::nothrow) OH_AbilityRuntime_ModObjDispatcher_TypeInfo();
            if (cType->u.pElementType != nullptr) {
                pElementType->FillCTypeInfo(cType->u.pElementType);
            }
        }
    } else if (vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ENUM ||
               vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRUCT ||
               vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_IPC_REMOTE_PROXY) {
        if (!idlType.empty()) {
            cType->u.idlType = strdup(idlType.c_str());
        }
    }
}

void MoTypeInfo::ClearCTypeInfo(OH_AbilityRuntime_ModObjDispatcher_TypeInfo* cType)
{
    if (cType == nullptr) return;
    switch (cType->vt) {
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP:
            if (cType->u.mapType.pValueType != nullptr) {
                ClearCTypeInfo(cType->u.mapType.pValueType);
                delete cType->u.mapType.pValueType;
                cType->u.mapType.pValueType = nullptr;
            }
            break;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY:
            if (cType->u.arrayType.pElementType != nullptr) {
                ClearCTypeInfo(cType->u.arrayType.pElementType);
                delete cType->u.arrayType.pElementType;
                cType->u.arrayType.pElementType = nullptr;
            }
            break;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET:
            if (cType->u.pElementType != nullptr) {
                ClearCTypeInfo(cType->u.pElementType);
                delete cType->u.pElementType;
                cType->u.pElementType = nullptr;
            }
            break;
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ENUM:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRUCT:
        case OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_IPC_REMOTE_PROXY:
            if (cType->u.idlType != nullptr) {
                std::free(cType->u.idlType);
                cType->u.idlType = nullptr;
            }
            break;
        default:
            break;
    }
    cType->vt = OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_EMPTY;
}

std::shared_ptr<MoTypeInfo> MoTypeInfo::FromCTypeInfo(const OH_AbilityRuntime_ModObjDispatcher_TypeInfo* cType)
{
    if (cType == nullptr) return nullptr;
    auto result = std::make_shared<MoTypeInfo>();
    result->vt = cType->vt;
    if (cType->vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_MAP) {
        result->mapKeyType = cType->u.mapType.keyType;
        if (cType->u.mapType.pValueType == nullptr) return nullptr;
        result->pMapValueType = FromCTypeInfo(cType->u.mapType.pValueType);
    } else if (cType->vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ARRAY) {
        if (cType->u.arrayType.pElementType == nullptr) return nullptr;
        result->pElementType = FromCTypeInfo(cType->u.arrayType.pElementType);
        result->arraySize = cType->u.arrayType.size;
    } else if (cType->vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_VECTOR ||
               cType->vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_SET) {
        if (cType->u.pElementType == nullptr) return nullptr;
        result->pElementType = FromCTypeInfo(cType->u.pElementType);
    } else if (cType->vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_STRUCT) {
        if (cType->u.idlType == nullptr) return nullptr;
        result->idlType = cType->u.idlType;
    } else if (cType->vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_ENUM ||
               cType->vt == OH_ABILITY_RUNTIME_MOD_OBJ_DISPATCHER_VT_IPC_REMOTE_PROXY) {
        if (cType->u.idlType != nullptr) {
            result->idlType = cType->u.idlType;
        }
    }
    return result;
}

// ---- MoVariantStorage ----

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
