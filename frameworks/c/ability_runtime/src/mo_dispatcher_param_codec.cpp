
/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#include "mo_dispatcher_param_codec.h"

#include <cstdlib>
#include <cstring>

#include "hilog_tag_wrapper.h"
#include "ipc_inner_object.h"
#include "mo_dispatcher_complex_type_manager.h"
#include "securec.h"

namespace OHOS::AbilityRuntime {
namespace {
AbilityRuntime_ErrorCode CheckWrite(bool success)
{
    return success ? ABILITY_RUNTIME_ERROR_CODE_NO_ERROR : ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
}

AbilityRuntime_ErrorCode WriteTypeTag(MessageParcel& parcel, OH_AbilityRuntime_MoDispatcher_ValueType type)
{
    return CheckWrite(parcel.WriteInt32(static_cast<int32_t>(type)));
}

AbilityRuntime_ErrorCode ReadTypeTag(MessageParcel& parcel, OH_AbilityRuntime_MoDispatcher_ValueType* type)
{
    int32_t rawType = parcel.ReadInt32();
    *type = static_cast<OH_AbilityRuntime_MoDispatcher_ValueType>(rawType);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}
} // namespace

AbilityRuntime_ErrorCode MoDispatcherParamCodec::MarshalCallRequest(const MoMethodMeta& methodMeta,
    const OH_AbilityRuntime_MoDispatcher_InputParams* inputParams, MessageParcel& dataParcel)
{
    if (inputParams == nullptr || inputParams->rgvarg == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (inputParams->cArgs != methodMeta.params.size()) {
        TAG_LOGE(AAFwkTag::EXT,
            "MarshalCallRequest: arg count mismatch, expected=%{public}zu, actual=%{public}u",
            methodMeta.params.size(), inputParams->cArgs);
        return ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH;
    }
    for (uint32_t i = 0; i < inputParams->cArgs; i++) {
        auto expectedVt = methodMeta.params[i].typeInfo
            ? methodMeta.params[i].typeInfo->vt : OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
        auto ret = MoDispatcherComplexTypeManager::ValidateVariantType(&inputParams->rgvarg[i], expectedVt);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
            TAG_LOGE(AAFwkTag::EXT, "MarshalCallRequest: param[%{public}u] type mismatch, expected=%{public}d, "
                "actual=%{public}d", i, static_cast<int32_t>(expectedVt),
                static_cast<int32_t>(inputParams->rgvarg[i].vt));
            return ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH;
        }
        ret = WriteRawValue(dataParcel, methodMeta.params[i].typeInfo, &inputParams->rgvarg[i]);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
            TAG_LOGE(AAFwkTag::EXT, "MarshalCallRequest: param[%{public}u] write failed, ret=%{public}d", i, ret);
            return ret;
        }
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherParamCodec::UnmarshalCallResult(const MoMethodMeta& methodMeta,
    MessageParcel& replyParcel, OH_AbilityRuntime_MoDispatcher_Variant* result)
{
    if (result == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    // Read application-level error code from reply
    int32_t errCode = replyParcel.ReadInt32();
    if (errCode != 0) {
        TAG_LOGE(AAFwkTag::EXT, "UnmarshalCallResult: remote returned errCode=%{public}d", errCode);
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    // For void return type, nothing more to read
    if (methodMeta.returnType == nullptr ||
        methodMeta.returnType->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VOID ||
        methodMeta.returnType->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY) {
        result->vt = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VOID;
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    return ReadRawValue(replyParcel, methodMeta.returnType, result);
}

AbilityRuntime_ErrorCode MoDispatcherParamCodec::WriteVariant(MessageParcel& parcel,
    const OH_AbilityRuntime_MoDispatcher_Variant* value)
{
    if (value == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = WriteTypeTag(parcel, value->vt);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }

    switch (value->vt) {
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY:
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VOID:
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_BOOL:
            return CheckWrite(parcel.WriteInt8(value->u.boolVal ? 1 : 0));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I8:
            return CheckWrite(parcel.WriteInt8(value->u.i8Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I16:
            return CheckWrite(parcel.WriteInt16(value->u.i16Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32:
            return CheckWrite(parcel.WriteInt32(value->u.i32Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I64:
            return CheckWrite(parcel.WriteInt64(value->u.i64Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U8:
            return CheckWrite(parcel.WriteInt8(static_cast<int8_t>(value->u.u8Val)));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U16:
            return CheckWrite(parcel.WriteInt16(static_cast<int16_t>(value->u.u16Val)));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U32:
            return CheckWrite(parcel.WriteInt32(static_cast<int32_t>(value->u.u32Val)));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U64:
            return CheckWrite(parcel.WriteInt64(static_cast<int64_t>(value->u.u64Val)));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F32:
            return CheckWrite(parcel.WriteFloat(value->u.f32Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F64:
            return CheckWrite(parcel.WriteDouble(value->u.f64Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING:
            return CheckWrite(parcel.WriteString(value->u.bstrVal == nullptr ? "" : value->u.bstrVal));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ENUM:
            return CheckWrite(parcel.WriteInt32(value->u.enumVal));
        default:
            break;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ARRAY) {
        if (value->u.parrayVal == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "WriteVariant: array value is nullptr");
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        ret = CheckWrite(parcel.WriteInt32(static_cast<int32_t>(value->u.parrayVal->elementTypeInfo ? value->u.parrayVal->elementTypeInfo->vt : OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY)));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        ret = CheckWrite(parcel.WriteInt32(static_cast<int32_t>(value->u.parrayVal->elements.size())));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (const auto& item : value->u.parrayVal->elements) {
            OH_AbilityRuntime_MoDispatcher_Variant temp;
            MoDispatcherComplexTypeManager::LoadVariant(item, &temp);
            ret = WriteVariant(parcel, &temp);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VECTOR) {
        if (value->u.pvectorVal == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "WriteVariant: vector value is nullptr");
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        ret = CheckWrite(parcel.WriteInt32(static_cast<int32_t>(value->u.pvectorVal->elementTypeInfo ? value->u.pvectorVal->elementTypeInfo->vt : OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY)));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        ret = CheckWrite(parcel.WriteInt32(static_cast<int32_t>(value->u.pvectorVal->elements.size())));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (const auto& item : value->u.pvectorVal->elements) {
            OH_AbilityRuntime_MoDispatcher_Variant temp;
            MoDispatcherComplexTypeManager::LoadVariant(item, &temp);
            ret = WriteVariant(parcel, &temp);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_SET) {
        if (value->u.psetVal == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "WriteVariant: set value is nullptr");
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        ret = CheckWrite(parcel.WriteInt32(static_cast<int32_t>(value->u.psetVal->elementTypeInfo ? value->u.psetVal->elementTypeInfo->vt : OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY)));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        ret = CheckWrite(parcel.WriteInt32(static_cast<int32_t>(value->u.psetVal->elements.size())));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (const auto& item : value->u.psetVal->elements) {
            OH_AbilityRuntime_MoDispatcher_Variant temp;
            MoDispatcherComplexTypeManager::LoadVariant(item, &temp);
            ret = WriteVariant(parcel, &temp);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_MAP) {
        if (value->u.pmapVal == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "WriteVariant: map value is nullptr");
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        ret = CheckWrite(parcel.WriteInt32(static_cast<int32_t>(value->u.pmapVal->keyType)));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        ret = CheckWrite(parcel.WriteInt32(static_cast<int32_t>(value->u.pmapVal->valueTypeInfo ? value->u.pmapVal->valueTypeInfo->vt : OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY)));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        ret = CheckWrite(parcel.WriteInt32(static_cast<int32_t>(value->u.pmapVal->entries.size())));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (const auto& item : value->u.pmapVal->entries) {
            OH_AbilityRuntime_MoDispatcher_Variant key;
            OH_AbilityRuntime_MoDispatcher_Variant val;
            MoDispatcherComplexTypeManager::LoadVariant(item.first, &key);
            MoDispatcherComplexTypeManager::LoadVariant(item.second, &val);
            ret = WriteVariant(parcel, &key);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            ret = WriteVariant(parcel, &val);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT) {
        if (value->u.pstructVal == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "WriteVariant: struct value is nullptr");
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        ret = CheckWrite(parcel.WriteString(value->u.pstructVal->name));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        ret = CheckWrite(parcel.WriteInt32(static_cast<int32_t>(value->u.pstructVal->fields.size())));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (const auto& item : value->u.pstructVal->fields) {
            ret = CheckWrite(parcel.WriteString(item.first));
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            OH_AbilityRuntime_MoDispatcher_Variant temp;
            MoDispatcherComplexTypeManager::LoadVariant(item.second, &temp);
            ret = WriteVariant(parcel, &temp);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_PROXY) {
        if (value->u.premoteProxyVal == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "WriteVariant: remote proxy value is nullptr");
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        return CheckWrite(parcel.WriteRemoteObject(value->u.premoteProxyVal->remote));
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_STUB) {
        if (value->u.premoteStubVal == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "WriteVariant: remote stub value is nullptr");
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        return CheckWrite(parcel.WriteRemoteObject(value->u.premoteStubVal->remote));
    }
    TAG_LOGE(AAFwkTag::EXT, "WriteVariant: unknown value type=%{public}d", static_cast<int32_t>(value->vt));
    return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
}

AbilityRuntime_ErrorCode MoDispatcherParamCodec::ReadVariant(MessageParcel& parcel,
    OH_AbilityRuntime_MoDispatcher_Variant* value)
{
    if (value == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = ReadTypeTag(parcel, &value->vt);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }

    switch (value->vt) {
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY:
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VOID:
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_BOOL: {
            int8_t v = parcel.ReadInt8();
            value->u.boolVal = (v != 0);
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I8:
            value->u.i8Val = parcel.ReadInt8();
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I16:
            value->u.i16Val = parcel.ReadInt16();
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32:
            value->u.i32Val = parcel.ReadInt32();
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I64:
            value->u.i64Val = parcel.ReadInt64();
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U8: {
            int8_t v = parcel.ReadInt8();
            value->u.u8Val = static_cast<uint8_t>(v);
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U16: {
            int16_t v = parcel.ReadInt16();
            value->u.u16Val = static_cast<uint16_t>(v);
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U32: {
            int32_t v = parcel.ReadInt32();
            value->u.u32Val = static_cast<uint32_t>(v);
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U64: {
            int64_t v = parcel.ReadInt64();
            value->u.u64Val = static_cast<uint64_t>(v);
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F32:
            value->u.f32Val = parcel.ReadFloat();
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F64:
            value->u.f64Val = parcel.ReadDouble();
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING: {
            std::string text = parcel.ReadString();
            const auto len = text.size();
            auto* mem = static_cast<char*>(std::malloc(len + 1));
            if (mem == nullptr) {
                TAG_LOGE(AAFwkTag::EXT, "ReadVariant: malloc failed for string, len=%{public}zu", len);
                return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
            }
            if (strcpy_s(mem, len + 1, text.c_str()) != EOK) {
                TAG_LOGE(AAFwkTag::EXT, "ReadVariant: strcpy_s failed for string");
                std::free(mem);
                return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
            }
            value->u.bstrVal = mem;
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ENUM:
            value->u.enumVal = parcel.ReadInt32();
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        default:
            break;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ARRAY) {
        int32_t elemType = parcel.ReadInt32();
        int32_t size = parcel.ReadInt32();
        if (size < 0) {
            TAG_LOGE(AAFwkTag::EXT, "ReadVariant: array size is negative, size=%{public}d", size);
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        OH_AbilityRuntime_MoDispatcher_ArrayHandle array = nullptr;
        OH_AbilityRuntime_MoDispatcher_TypeInfo elemTypeInfo;
        std::memset(&elemTypeInfo, 0, sizeof(elemTypeInfo));
        elemTypeInfo.vt = static_cast<OH_AbilityRuntime_MoDispatcher_ValueType>(elemType);
        ret = MoDispatcherComplexTypeManager::ArrayCreate(&elemTypeInfo,
            static_cast<uint32_t>(size), &array);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (uint32_t i = 0; i < static_cast<uint32_t>(size); i++) {
            OH_AbilityRuntime_MoDispatcher_Variant elem;
            ret = ReadVariant(parcel, &elem);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            ret = MoDispatcherComplexTypeManager::ArraySet(array, i, &elem);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        value->u.parrayVal = array;
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VECTOR) {
        int32_t elemType = parcel.ReadInt32();
        int32_t size = parcel.ReadInt32();
        if (size < 0) {
            TAG_LOGE(AAFwkTag::EXT, "ReadVariant: vector size is negative, size=%{public}d", size);
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        OH_AbilityRuntime_MoDispatcher_VectorHandle vector = nullptr;
        OH_AbilityRuntime_MoDispatcher_TypeInfo elemTypeInfo;
        std::memset(&elemTypeInfo, 0, sizeof(elemTypeInfo));
        elemTypeInfo.vt = static_cast<OH_AbilityRuntime_MoDispatcher_ValueType>(elemType);
        ret = MoDispatcherComplexTypeManager::VectorCreate(&elemTypeInfo, &vector);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (uint32_t i = 0; i < static_cast<uint32_t>(size); i++) {
            OH_AbilityRuntime_MoDispatcher_Variant elem;
            ret = ReadVariant(parcel, &elem);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            ret = MoDispatcherComplexTypeManager::VectorAdd(vector, &elem);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        value->u.pvectorVal = vector;
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_SET) {
        int32_t elemType = parcel.ReadInt32();
        int32_t size = parcel.ReadInt32();
        if (size < 0) {
            TAG_LOGE(AAFwkTag::EXT, "ReadVariant: set size is negative, size=%{public}d", size);
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        OH_AbilityRuntime_MoDispatcher_SetHandle setHandle = nullptr;
        OH_AbilityRuntime_MoDispatcher_TypeInfo elemTypeInfo;
        std::memset(&elemTypeInfo, 0, sizeof(elemTypeInfo));
        elemTypeInfo.vt = static_cast<OH_AbilityRuntime_MoDispatcher_ValueType>(elemType);
        ret = MoDispatcherComplexTypeManager::SetCreate(&elemTypeInfo, &setHandle);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (uint32_t i = 0; i < static_cast<uint32_t>(size); i++) {
            OH_AbilityRuntime_MoDispatcher_Variant elem;
            ret = ReadVariant(parcel, &elem);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            ret = MoDispatcherComplexTypeManager::SetAdd(setHandle, &elem);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        value->u.psetVal = setHandle;
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_MAP) {
        int32_t keyType = parcel.ReadInt32();
        int32_t valueType = parcel.ReadInt32();
        int32_t size = parcel.ReadInt32();
        if (size < 0) {
            TAG_LOGE(AAFwkTag::EXT, "ReadVariant: map size is negative, size=%{public}d", size);
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        OH_AbilityRuntime_MoDispatcher_MapHandle map = nullptr;
        OH_AbilityRuntime_MoDispatcher_TypeInfo valueTypeInfo;
        std::memset(&valueTypeInfo, 0, sizeof(valueTypeInfo));
        valueTypeInfo.vt = static_cast<OH_AbilityRuntime_MoDispatcher_ValueType>(valueType);
        ret = MoDispatcherComplexTypeManager::MapCreate(static_cast<OH_AbilityRuntime_MoDispatcher_ValueType>(keyType),
            &valueTypeInfo, &map);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (uint32_t i = 0; i < static_cast<uint32_t>(size); i++) {
            OH_AbilityRuntime_MoDispatcher_Variant k;
            OH_AbilityRuntime_MoDispatcher_Variant v;
            ret = ReadVariant(parcel, &k);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            ret = ReadVariant(parcel, &v);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            ret = MoDispatcherComplexTypeManager::MapPut(map, &k, &v);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        value->u.pmapVal = map;
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT) {
        std::string structName = parcel.ReadString();
        int32_t fieldCount = parcel.ReadInt32();
        if (fieldCount < 0) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        OH_AbilityRuntime_MoDispatcher_StructHandle structObj = nullptr;
        ret = MoDispatcherComplexTypeManager::StructCreate(structName.c_str(), &structObj);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (uint32_t i = 0; i < static_cast<uint32_t>(fieldCount); i++) {
            std::string fieldName = parcel.ReadString();
            OH_AbilityRuntime_MoDispatcher_Variant fieldValue;
            ret = ReadVariant(parcel, &fieldValue);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            ret = MoDispatcherComplexTypeManager::StructSetField(structObj, fieldName.c_str(), &fieldValue);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        value->u.pstructVal = structObj;
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_PROXY) {
        auto remoteObj = parcel.ReadRemoteObject();
        if (!remoteObj) {
            TAG_LOGE(AAFwkTag::EXT, "ReadVariant: read remote proxy object failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* proxy = new (std::nothrow) OHIPCRemoteProxy();
        if (proxy == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "ReadVariant: allocate OHIPCRemoteProxy failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        proxy->remote = remoteObj;
        value->u.premoteProxyVal = proxy;
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_STUB) {
        auto remoteObj = parcel.ReadRemoteObject();
        if (!remoteObj) {
            TAG_LOGE(AAFwkTag::EXT, "ReadVariant: read remote stub object failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* stub = new (std::nothrow) OHIPCRemoteStub();
        if (stub == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "ReadVariant: allocate OHIPCRemoteStub failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        stub->remote = remoteObj;
        value->u.premoteStubVal = stub;
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    TAG_LOGE(AAFwkTag::EXT, "ReadVariant: unknown value type=%{public}d", static_cast<int32_t>(value->vt));
    return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
}
AbilityRuntime_ErrorCode MoDispatcherParamCodec::WriteRawValue(MessageParcel& parcel,
    const std::shared_ptr<MoTypeInfo>& typeInfo, const OH_AbilityRuntime_MoDispatcher_Variant* value)
{
    if (typeInfo == nullptr || value == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    switch (typeInfo->vt) {
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY:
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VOID:
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_BOOL:
            return CheckWrite(parcel.WriteInt8(value->u.boolVal ? 1 : 0));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I8:
            return CheckWrite(parcel.WriteInt8(value->u.i8Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I16:
            return CheckWrite(parcel.WriteInt16(value->u.i16Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32:
            return CheckWrite(parcel.WriteInt32(value->u.i32Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I64:
            return CheckWrite(parcel.WriteInt64(value->u.i64Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U8:
            return CheckWrite(parcel.WriteInt8(static_cast<int8_t>(value->u.u8Val)));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U16:
            return CheckWrite(parcel.WriteInt16(static_cast<int16_t>(value->u.u16Val)));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U32:
            return CheckWrite(parcel.WriteInt32(static_cast<int32_t>(value->u.u32Val)));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U64:
            return CheckWrite(parcel.WriteInt64(static_cast<int64_t>(value->u.u64Val)));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F32:
            return CheckWrite(parcel.WriteFloat(value->u.f32Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F64:
            return CheckWrite(parcel.WriteDouble(value->u.f64Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING:
            std::string strVal(value->u.bstrVal != nullptr ? value->u.bstrVal : "");
            return CheckWrite(parcel.WriteString(strVal));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ENUM:
            return CheckWrite(parcel.WriteInt32(value->u.enumVal));
        default:
            break;
    }
    if (typeInfo->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ARRAY) {
        if (value->u.parrayVal == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "WriteRawValue: array value is nullptr");
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        auto ret = CheckWrite(parcel.WriteInt32(static_cast<int32_t>(value->u.parrayVal->elements.size())));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (const auto& item : value->u.parrayVal->elements) {
            OH_AbilityRuntime_MoDispatcher_Variant temp;
            MoDispatcherComplexTypeManager::LoadVariant(item, &temp);
            ret = WriteRawValue(parcel, typeInfo->pElementType, &temp);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (typeInfo->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VECTOR) {
        if (value->u.pvectorVal == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "WriteRawValue: vector value is nullptr");
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        auto ret = CheckWrite(parcel.WriteInt32(static_cast<int32_t>(value->u.pvectorVal->elements.size())));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (const auto& item : value->u.pvectorVal->elements) {
            OH_AbilityRuntime_MoDispatcher_Variant temp;
            MoDispatcherComplexTypeManager::LoadVariant(item, &temp);
            ret = WriteRawValue(parcel, typeInfo->pElementType, &temp);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (typeInfo->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_SET) {
        if (value->u.psetVal == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "WriteRawValue: set value is nullptr");
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        auto ret = CheckWrite(parcel.WriteInt32(static_cast<int32_t>(value->u.psetVal->elements.size())));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (const auto& item : value->u.psetVal->elements) {
            OH_AbilityRuntime_MoDispatcher_Variant temp;
            MoDispatcherComplexTypeManager::LoadVariant(item, &temp);
            ret = WriteRawValue(parcel, typeInfo->pElementType, &temp);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (typeInfo->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_MAP) {
        if (value->u.pmapVal == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "WriteRawValue: map value is nullptr");
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        auto ret = CheckWrite(parcel.WriteInt32(static_cast<int32_t>(value->u.pmapVal->entries.size())));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        auto keyTypeInfo = std::make_shared<MoTypeInfo>();
        keyTypeInfo->vt = typeInfo->mapKeyType;
        for (const auto& item : value->u.pmapVal->entries) {
            OH_AbilityRuntime_MoDispatcher_Variant key;
            OH_AbilityRuntime_MoDispatcher_Variant val;
            MoDispatcherComplexTypeManager::LoadVariant(item.first, &key);
            MoDispatcherComplexTypeManager::LoadVariant(item.second, &val);
            ret = WriteRawValue(parcel, keyTypeInfo, &key);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            ret = WriteRawValue(parcel, typeInfo->pMapValueType, &val);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (typeInfo->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT) {
        if (value->u.pstructVal == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "WriteRawValue: struct value is nullptr");
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        std::vector<std::string> fieldNames;
        if (!MoDispatcherComplexTypeManager::GetStructFieldNames(typeInfo->idlType, &fieldNames)) {
            TAG_LOGE(AAFwkTag::EXT, "WriteRawValue: struct '%{public}s' metadata not found",
                typeInfo->idlType.c_str());
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        for (const auto& fieldName : fieldNames) {
            std::shared_ptr<MoTypeInfo> fieldType;
            MoDispatcherComplexTypeManager::GetStructFieldType(typeInfo->idlType, fieldName, &fieldType);
            auto it = value->u.pstructVal->fields.find(fieldName);
            if (it != value->u.pstructVal->fields.end()) {
                OH_AbilityRuntime_MoDispatcher_Variant temp;
                MoDispatcherComplexTypeManager::LoadVariant(it->second, &temp);
                auto ret = WriteRawValue(parcel, fieldType, &temp);
                if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            } else {
                // Field not set, write zero/default
                OH_AbilityRuntime_MoDispatcher_Variant temp;
                std::memset(&temp, 0, sizeof(temp));
                temp.vt = fieldType ? fieldType->vt : OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
                auto ret = WriteRawValue(parcel, fieldType, &temp);
                if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            }
        }
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (typeInfo->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_PROXY) {
        if (value->u.premoteProxyVal == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "WriteRawValue: remote proxy value is nullptr");
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        return CheckWrite(parcel.WriteRemoteObject(value->u.premoteProxyVal->remote));
    }
    if (typeInfo->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_STUB) {
        if (value->u.premoteStubVal == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "WriteRawValue: remote stub value is nullptr");
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        return CheckWrite(parcel.WriteRemoteObject(value->u.premoteStubVal->remote));
    }
    TAG_LOGE(AAFwkTag::EXT, "WriteRawValue: unknown type=%{public}d", static_cast<int32_t>(typeInfo->vt));
    return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
}

AbilityRuntime_ErrorCode MoDispatcherParamCodec::ReadRawValue(MessageParcel& parcel,
    const std::shared_ptr<MoTypeInfo>& typeInfo, OH_AbilityRuntime_MoDispatcher_Variant* value)
{
    if (typeInfo == nullptr || value == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    value->vt = typeInfo->vt;
    switch (typeInfo->vt) {
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY:
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VOID:
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_BOOL: {
            int8_t v = parcel.ReadInt8();
            value->u.boolVal = (v != 0);
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I8:
            value->u.i8Val = parcel.ReadInt8();
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I16:
            value->u.i16Val = parcel.ReadInt16();
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32:
            value->u.i32Val = parcel.ReadInt32();
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I64:
            value->u.i64Val = parcel.ReadInt64();
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U8: {
            int8_t v = parcel.ReadInt8();
            value->u.u8Val = static_cast<uint8_t>(v);
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U16: {
            int16_t v = parcel.ReadInt16();
            value->u.u16Val = static_cast<uint16_t>(v);
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U32: {
            int32_t v = parcel.ReadInt32();
            value->u.u32Val = static_cast<uint32_t>(v);
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U64: {
            int64_t v = parcel.ReadInt64();
            value->u.u64Val = static_cast<uint64_t>(v);
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F32:
            value->u.f32Val = parcel.ReadFloat();
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F64:
            value->u.f64Val = parcel.ReadDouble();
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING: {
            std::string text = parcel.ReadString();
            const auto len = text.size();
            auto* mem = static_cast<char*>(std::malloc(len + 1));
            if (mem == nullptr) {
                TAG_LOGE(AAFwkTag::EXT, "ReadRawValue: malloc failed for string, len=%{public}zu", len);
                return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
            }
            if (strcpy_s(mem, len + 1, text.c_str()) != EOK) {
                TAG_LOGE(AAFwkTag::EXT, "ReadRawValue: strcpy_s failed for string");
                std::free(mem);
                return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
            }
            value->u.bstrVal = mem;
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ENUM:
            value->u.enumVal = parcel.ReadInt32();
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        default:
            break;
    }
    if (typeInfo->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ARRAY) {
        int32_t size = parcel.ReadInt32();
        if (size < 0) {
            TAG_LOGE(AAFwkTag::EXT, "ReadRawValue: array size is negative, size=%{public}d", size);
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        OH_AbilityRuntime_MoDispatcher_TypeInfo elemTypeInfo;
        std::memset(&elemTypeInfo, 0, sizeof(elemTypeInfo));
        if (typeInfo->pElementType) {
            typeInfo->pElementType->FillCTypeInfo(&elemTypeInfo);
        }
        OH_AbilityRuntime_MoDispatcher_ArrayHandle array = nullptr;
        auto ret = MoDispatcherComplexTypeManager::ArrayCreate(&elemTypeInfo,
            static_cast<uint32_t>(size), &array);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (uint32_t i = 0; i < static_cast<uint32_t>(size); i++) {
            OH_AbilityRuntime_MoDispatcher_Variant elem;
            ret = ReadRawValue(parcel, typeInfo->pElementType, &elem);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            ret = MoDispatcherComplexTypeManager::ArraySet(array, i, &elem);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        value->u.parrayVal = array;
        MoTypeInfo::ClearCTypeInfo(&elemTypeInfo);
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (typeInfo->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VECTOR) {
        int32_t size = parcel.ReadInt32();
        if (size < 0) {
            TAG_LOGE(AAFwkTag::EXT, "ReadRawValue: vector size is negative, size=%{public}d", size);
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        OH_AbilityRuntime_MoDispatcher_TypeInfo elemTypeInfo;
        std::memset(&elemTypeInfo, 0, sizeof(elemTypeInfo));
        if (typeInfo->pElementType) {
            typeInfo->pElementType->FillCTypeInfo(&elemTypeInfo);
        }
        OH_AbilityRuntime_MoDispatcher_VectorHandle vector = nullptr;
        auto ret = MoDispatcherComplexTypeManager::VectorCreate(&elemTypeInfo, &vector);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (uint32_t i = 0; i < static_cast<uint32_t>(size); i++) {
            OH_AbilityRuntime_MoDispatcher_Variant elem;
            ret = ReadRawValue(parcel, typeInfo->pElementType, &elem);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            ret = MoDispatcherComplexTypeManager::VectorAdd(vector, &elem);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        value->u.pvectorVal = vector;
        MoTypeInfo::ClearCTypeInfo(&elemTypeInfo);
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (typeInfo->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_SET) {
        int32_t size = parcel.ReadInt32();
        if (size < 0) {
            TAG_LOGE(AAFwkTag::EXT, "ReadRawValue: set size is negative, size=%{public}d", size);
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        OH_AbilityRuntime_MoDispatcher_TypeInfo elemTypeInfo;
        std::memset(&elemTypeInfo, 0, sizeof(elemTypeInfo));
        if (typeInfo->pElementType) {
            typeInfo->pElementType->FillCTypeInfo(&elemTypeInfo);
        }
        OH_AbilityRuntime_MoDispatcher_SetHandle setHandle = nullptr;
        auto ret = MoDispatcherComplexTypeManager::SetCreate(&elemTypeInfo, &setHandle);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (uint32_t i = 0; i < static_cast<uint32_t>(size); i++) {
            OH_AbilityRuntime_MoDispatcher_Variant elem;
            ret = ReadRawValue(parcel, typeInfo->pElementType, &elem);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            ret = MoDispatcherComplexTypeManager::SetAdd(setHandle, &elem);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        value->u.psetVal = setHandle;
        MoTypeInfo::ClearCTypeInfo(&elemTypeInfo);
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (typeInfo->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_MAP) {
        int32_t size = parcel.ReadInt32();
        if (size < 0) {
            TAG_LOGE(AAFwkTag::EXT, "ReadRawValue: map size is negative, size=%{public}d", size);
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        OH_AbilityRuntime_MoDispatcher_TypeInfo valueTypeInfo;
        std::memset(&valueTypeInfo, 0, sizeof(valueTypeInfo));
        if (typeInfo->pMapValueType) {
            typeInfo->pMapValueType->FillCTypeInfo(&valueTypeInfo);
        }
        OH_AbilityRuntime_MoDispatcher_MapHandle map = nullptr;
        auto ret = MoDispatcherComplexTypeManager::MapCreate(typeInfo->mapKeyType, &valueTypeInfo, &map);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        auto keyTypeInfo = std::make_shared<MoTypeInfo>();
        keyTypeInfo->vt = typeInfo->mapKeyType;
        for (uint32_t i = 0; i < static_cast<uint32_t>(size); i++) {
            OH_AbilityRuntime_MoDispatcher_Variant k;
            OH_AbilityRuntime_MoDispatcher_Variant v;
            ret = ReadRawValue(parcel, keyTypeInfo, &k);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            ret = ReadRawValue(parcel, typeInfo->pMapValueType, &v);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            ret = MoDispatcherComplexTypeManager::MapPut(map, &k, &v);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        value->u.pmapVal = map;
        MoTypeInfo::ClearCTypeInfo(&valueTypeInfo);
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (typeInfo->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT) {
        std::vector<std::string> fieldNames;
        if (!MoDispatcherComplexTypeManager::GetStructFieldNames(typeInfo->idlType, &fieldNames)) {
            TAG_LOGE(AAFwkTag::EXT, "ReadRawValue: struct '%{public}s' metadata not found",
                typeInfo->idlType.c_str());
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        OH_AbilityRuntime_MoDispatcher_StructHandle structObj = nullptr;
        auto ret = MoDispatcherComplexTypeManager::StructCreate(typeInfo->idlType.c_str(), &structObj);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (const auto& fieldName : fieldNames) {
            std::shared_ptr<MoTypeInfo> fieldType;
            MoDispatcherComplexTypeManager::GetStructFieldType(typeInfo->idlType, fieldName, &fieldType);
            OH_AbilityRuntime_MoDispatcher_Variant fieldVal;
            std::memset(&fieldVal, 0, sizeof(fieldVal));
            ret = ReadRawValue(parcel, fieldType, &fieldVal);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                MoDispatcherComplexTypeManager::StructRelease(&structObj);
                return ret;
            }
            ret = MoDispatcherComplexTypeManager::StructSetField(structObj, fieldName.c_str(), &fieldVal);
            MoDispatcherComplexTypeManager::Variant_Clear(&fieldVal);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                MoDispatcherComplexTypeManager::StructRelease(&structObj);
                return ret;
            }
        }
        value->u.pstructVal = structObj;
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (typeInfo->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_PROXY) {
        auto remoteObj = parcel.ReadRemoteObject();
        if (!remoteObj) {
            TAG_LOGE(AAFwkTag::EXT, "ReadRawValue: read remote proxy object failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* proxy = new (std::nothrow) OHIPCRemoteProxy();
        if (proxy == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "ReadRawValue: allocate OHIPCRemoteProxy failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        proxy->remote = remoteObj;
        value->u.premoteProxyVal = proxy;
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (typeInfo->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_STUB) {
        auto remoteObj = parcel.ReadRemoteObject();
        if (!remoteObj) {
            TAG_LOGE(AAFwkTag::EXT, "ReadRawValue: read remote stub object failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        auto* stub = new (std::nothrow) OHIPCRemoteStub();
        if (stub == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "ReadRawValue: allocate OHIPCRemoteStub failed");
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        stub->remote = remoteObj;
        value->u.premoteStubVal = stub;
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    TAG_LOGE(AAFwkTag::EXT, "ReadRawValue: unknown type=%{public}d", static_cast<int32_t>(typeInfo->vt));
    return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
}
} // namespace OHOS::AbilityRuntime
