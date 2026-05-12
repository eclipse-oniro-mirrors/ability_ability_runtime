
/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#include "modular_object_dispatcher.h"

#include <memory>
#include <string>

#include "hilog_tag_wrapper.h"
#include "ipc_inner_object.h"
#include "message_option.h"
#include "mo_dispatcher_complex_type_manager.h"
#include "mo_dispatcher_metadata_manager.h"
#include "mo_dispatcher_param_codec.h"
#include "mo_dispatcher_types.h"
#include "securec.h"

using OHOS::AbilityRuntime::MoDispatcherComplexTypeManager;
using OHOS::AbilityRuntime::MoDispatcherMetadataManager;
using OHOS::AbilityRuntime::MoDispatcherParamCodec;
using OHOS::MessageOption;
using OHOS::MessageParcel;

namespace {
AbilityRuntime_ErrorCode CopyStringToBuffer(const std::string& src, char* dst, uint32_t max)
{
    if (dst == nullptr || max == 0 || src.size() + 1 > max) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return strcpy_s(dst, max, src.c_str()) == EOK ? ABILITY_RUNTIME_ERROR_CODE_NO_ERROR :
        ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
}
} // namespace

#ifdef __cplusplus
extern "C" {
#endif

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_CreateInstance(
    OHIPCRemoteProxy* remoteProxy, OH_AbilityRuntime_MoDispatcherHandle* ppMoDispatcher)
{
    if (remoteProxy == nullptr || ppMoDispatcher == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* dispatcher = new (std::nothrow) OH_AbilityRuntime_MoDispatcher();
    if (dispatcher == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "Failed to allocate MoDispatcher instance");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    dispatcher->proxy = remoteProxy->remote;
    dispatcher->metadataManager = std::make_shared<MoDispatcherMetadataManager>();
    *ppMoDispatcher = dispatcher;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void OH_AbilityRuntime_MoDispatcher_Release(OH_AbilityRuntime_MoDispatcherHandle* ppMoDispatcher)
{
    if (ppMoDispatcher == nullptr || *ppMoDispatcher == nullptr) {
        return;
    }
    delete *ppMoDispatcher;
    *ppMoDispatcher = nullptr;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_HasTypeDescriptor(
    OH_AbilityRuntime_MoDispatcherHandle pMoDispatcher, uint32_t* pctinfo)
{
    if (pMoDispatcher == nullptr || pctinfo == nullptr || pMoDispatcher->metadataManager == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = pMoDispatcher->metadataManager->EnsureLoaded(pMoDispatcher->proxy);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "HasTypeDescriptor: EnsureLoaded failed, ret=%{public}d", ret);
    }
    *pctinfo = (ret == ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) ? 1 : 0;
    return (ret == ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) ? ABILITY_RUNTIME_ERROR_CODE_NO_ERROR :
        ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_GetTypeDescriptor(
    OH_AbilityRuntime_MoDispatcherHandle pMoDispatcher,
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle* ppTypeDescriptor)
{
    if (pMoDispatcher == nullptr || ppTypeDescriptor == nullptr || pMoDispatcher->metadataManager == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = pMoDispatcher->metadataManager->EnsureLoaded(pMoDispatcher->proxy);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "GetTypeDescriptor: EnsureLoaded failed, ret=%{public}d", ret);
        return ret;
    }
    auto* typeDesc = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_TypeDescriptor();
    if (typeDesc == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetTypeDescriptor: allocate TypeDescriptor failed");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    typeDesc->metadataManager = pMoDispatcher->metadataManager;
    *ppTypeDescriptor = typeDesc;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_QueryMainServiceInterfaceMemIDsOfNames(
    OH_AbilityRuntime_MoDispatcherHandle pMoDispatcher, const char** rgszNames, uint32_t cNames,
    uint32_t* pMemID)
{
    if (pMoDispatcher == nullptr || rgszNames == nullptr || pMemID == nullptr
        || pMoDispatcher->metadataManager == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = pMoDispatcher->metadataManager->EnsureLoaded(pMoDispatcher->proxy);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "QueryMemIDsOfNames: EnsureLoaded failed, ret=%{public}d", ret);
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    return pMoDispatcher->metadataManager->QueryMainServiceInterfaceMemberIds(rgszNames, cNames, pMemID);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_CallMethod(
    OH_AbilityRuntime_MoDispatcherHandle pMoDispatcher, uint32_t memID,
    OH_AbilityRuntime_MoDispatcher_InputParams* pInputParams,
    OH_AbilityRuntime_MoDispatcher_Variant* pResult)
{
    if (pMoDispatcher == nullptr || pInputParams == nullptr || pResult == nullptr
        || pMoDispatcher->metadataManager == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = pMoDispatcher->metadataManager->EnsureLoaded(pMoDispatcher->proxy);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "CallMethod: EnsureLoaded failed, ret=%{public}d", ret);
        return ret;
    }

    OHOS::AbilityRuntime::MoMethodMeta methodMeta;
    ret = pMoDispatcher->metadataManager->GetMethodMeta(memID, &methodMeta);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "CallMethod: GetMethodMeta failed, memID=%{public}u, ret=%{public}d", memID, ret);
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    MessageParcel dataParcel;
    MessageParcel replyParcel;

    std::u16string ifaceDescriptor;
    ret = pMoDispatcher->metadataManager->GetInterfaceDescriptor(
        methodMeta.interfaceName, &ifaceDescriptor);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "CallMethod: GetInterfaceDescriptor failed, ret=%{public}d", ret);
        return ret;
    }
    if (!dataParcel.WriteInterfaceToken(ifaceDescriptor)) {
        TAG_LOGE(AAFwkTag::EXT, "CallMethod: WriteInterfaceToken failed");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }

    ret = MoDispatcherParamCodec::MarshalCallRequest(methodMeta, pInputParams, dataParcel);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "CallMethod: MarshalCallRequest failed, memID=%{public}u, ret=%{public}d",
            memID, ret);
        return ret;
    }

    MessageOption option(methodMeta.oneway ? MessageOption::TF_ASYNC : MessageOption::TF_SYNC);
    int ipcRet = pMoDispatcher->proxy->SendRequest(
        methodMeta.ipcCode, dataParcel, replyParcel, option);
    if (ipcRet != 0) {
        TAG_LOGE(AAFwkTag::EXT, "CallMethod: SendRequest failed, memID=%{public}u, ipcCode=%{public}d, "
            "ipcRet=%{public}d", memID, methodMeta.ipcCode, ipcRet);
        return ABILITY_RUNTIME_ERROR_CODE_SEND_REQUEST_FAILED;
    }

    if (methodMeta.oneway) {
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    ret = MoDispatcherParamCodec::UnmarshalCallResult(methodMeta, replyParcel, pResult);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "CallMethod: UnmarshalCallResult failed, memID=%{public}u, ret=%{public}d",
            memID, ret);
    }
    return ret;
}

void OH_AbilityRuntime_MoDispatcher_Variant_Clear(OH_AbilityRuntime_MoDispatcher_Variant* pVariant)
{
    MoDispatcherComplexTypeManager::Variant_Clear(pVariant);
}

void OH_AbilityRuntime_MoDispatcher_TypeInfo_Clear(OH_AbilityRuntime_MoDispatcher_TypeInfo* pTypeInfo)
{
    MoDispatcherComplexTypeManager::TypeInfo_Clear(pTypeInfo);
}

void OH_AbilityRuntime_TypeDescriptor_Release(OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle *pTypeDescriptor)
{
    if (pTypeDescriptor == nullptr || *pTypeDescriptor == nullptr) {
        return;
    }
    delete *pTypeDescriptor;
    *pTypeDescriptor = nullptr;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetVersion(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, char* pbstrVersion, uint32_t cMaxVersion)
{
    if (pTypeDescriptor == nullptr || pbstrVersion == nullptr || cMaxVersion == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string version;
    auto ret = pTypeDescriptor->metadataManager->GetVersion(&version);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return CopyStringToBuffer(version, pbstrVersion, cMaxVersion);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t* pcInterfaces)
{
    if (pTypeDescriptor == nullptr || pcInterfaces == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetInterfaceCount(pcInterfaces);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t index, char* pbstrName,
    uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string name;
    auto ret = pTypeDescriptor->metadataManager->GetInterfaceName(index, &name);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return CopyStringToBuffer(name, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceIsCallback(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrName, bool* pIsCallback)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || pIsCallback == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetInterfaceIsCallback(pbstrName, pIsCallback);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMainServiceInterfaceName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, char* pbstrName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string name;
    auto ret = pTypeDescriptor->metadataManager->GetMainServiceInterfaceName(&name);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return CopyStringToBuffer(name, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, uint32_t* pcMethods)
{
    if (pTypeDescriptor == nullptr || pbstrInterfaceName == nullptr || pcMethods == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetMethodCount(pbstrInterfaceName, pcMethods);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, uint32_t index, char* pbstrName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrInterfaceName == nullptr || pbstrName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string methodName;
    auto ret = pTypeDescriptor->metadataManager->GetMethodName(pbstrInterfaceName, index, &methodName);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return CopyStringToBuffer(methodName, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodMemberId(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, const char* pbstrMethodName, uint32_t* pMemID)
{
    if (pTypeDescriptor == nullptr || pbstrInterfaceName == nullptr || pbstrMethodName == nullptr
        || pMemID == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetMethodMemberId(pbstrInterfaceName, pbstrMethodName, pMemID);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodReturnType(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, const char* pbstrMethodName,
    OH_AbilityRuntime_MoDispatcher_TypeInfo* pReturnType)
{
    if (pTypeDescriptor == nullptr || pbstrInterfaceName == nullptr || pbstrMethodName == nullptr
        || pReturnType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetMethodReturnType(
        pbstrInterfaceName, pbstrMethodName, pReturnType);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodParamCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, const char* pbstrMethodName, uint32_t* pcParams)
{
    if (pTypeDescriptor == nullptr || pbstrInterfaceName == nullptr || pbstrMethodName == nullptr
        || pcParams == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetMethodParamCount(pbstrInterfaceName, pbstrMethodName, pcParams);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodParamType(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, const char* pbstrMethodName,
    uint32_t iParamIndex, OH_AbilityRuntime_MoDispatcher_TypeInfo* pParamType)
{
    if (pTypeDescriptor == nullptr || pbstrInterfaceName == nullptr || pbstrMethodName == nullptr
        || pParamType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetMethodParamType(
        pbstrInterfaceName, pbstrMethodName, iParamIndex, pParamType);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodParamName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, const char* pbstrMethodName,
    uint32_t iParamIndex, char* pbstrName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrInterfaceName == nullptr || pbstrMethodName == nullptr
        || pbstrName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string paramName;
    auto ret = pTypeDescriptor->metadataManager->GetMethodParamName(
        pbstrInterfaceName, pbstrMethodName, iParamIndex, &paramName);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return CopyStringToBuffer(paramName, pbstrName, cMaxName);
}
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t* pcEnums)
{
    if (pTypeDescriptor == nullptr || pcEnums == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetEnumCount(pcEnums);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t index, char* pbstrName,
    uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string name;
    auto ret = pTypeDescriptor->metadataManager->GetEnumName(index, &name);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return CopyStringToBuffer(name, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumValueCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrEnumName,
    uint32_t* pcValues)
{
    if (pTypeDescriptor == nullptr || pbstrEnumName == nullptr || pcValues == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetEnumValueCount(pbstrEnumName, pcValues);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumValueName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrEnumName,
    uint32_t iValueIndex, char* pbstrValueName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrEnumName == nullptr || pbstrValueName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string valueName;
    auto ret = pTypeDescriptor->metadataManager->GetEnumValueName(pbstrEnumName, iValueIndex, &valueName);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return CopyStringToBuffer(valueName, pbstrValueName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumValue(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrEnumName,
    const char* pbstrValueName, int32_t* pValue)
{
    if (pTypeDescriptor == nullptr || pbstrEnumName == nullptr || pbstrValueName == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetEnumValue(pbstrEnumName, pbstrValueName, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t* pcStructs)
{
    if (pTypeDescriptor == nullptr || pcStructs == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetStructCount(pcStructs);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t index, char* pbstrName,
    uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string name;
    auto ret = pTypeDescriptor->metadataManager->GetStructName(index, &name);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return CopyStringToBuffer(name, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructFieldCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrStructName,
    uint32_t* pcFields)
{
    if (pTypeDescriptor == nullptr || pbstrStructName == nullptr || pcFields == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetStructFieldCount(pbstrStructName, pcFields);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructFieldName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrStructName,
    uint32_t iFieldIndex, char* pbstrFieldName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrStructName == nullptr || pbstrFieldName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string name;
    auto ret = pTypeDescriptor->metadataManager->GetStructFieldName(pbstrStructName, iFieldIndex, &name);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return CopyStringToBuffer(name, pbstrFieldName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructFieldType(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrStructName,
    const char* pbstrFieldName, OH_AbilityRuntime_MoDispatcher_TypeInfo* pFieldType)
{
    if (pTypeDescriptor == nullptr || pbstrStructName == nullptr || pbstrFieldName == nullptr ||
        pFieldType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetStructFieldType(pbstrStructName, pbstrFieldName, pFieldType);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_Create(
    OH_AbilityRuntime_MoDispatcher_TypeInfo* elementType, uint32_t size,
    OH_AbilityRuntime_MoDispatcher_ArrayHandle* ppArray)
{
    if (elementType == nullptr || ppArray == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return MoDispatcherComplexTypeManager::ArrayCreate(elementType, size, ppArray);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_GetElementType(
    OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, OH_AbilityRuntime_MoDispatcher_TypeInfo* pElementType)
{
    if (pArray == nullptr || pElementType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return MoDispatcherComplexTypeManager::ArrayGetElementType(pArray, pElementType);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_Set(
    OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, uint32_t index,
    const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    return MoDispatcherComplexTypeManager::ArraySet(pArray, index, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_Get(
    OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, uint32_t index,
    OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    return MoDispatcherComplexTypeManager::ArrayGet(pArray, index, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_GetSize(
    OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, uint32_t* pSize)
{
    return MoDispatcherComplexTypeManager::ArrayGetSize(pArray, pSize);
}

void OH_AbilityRuntime_MoDispatcher_Array_Release(OH_AbilityRuntime_MoDispatcher_ArrayHandle* ppArray)
{
    MoDispatcherComplexTypeManager::ArrayRelease(ppArray);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_Create(
    OH_AbilityRuntime_MoDispatcher_TypeInfo* elementType, OH_AbilityRuntime_MoDispatcher_VectorHandle* ppVector)
{
    if (elementType == nullptr || ppVector == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return MoDispatcherComplexTypeManager::VectorCreate(elementType, ppVector);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_GetElementType(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector, OH_AbilityRuntime_MoDispatcher_TypeInfo* pElementType)
{
    if (pVector == nullptr || pElementType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return MoDispatcherComplexTypeManager::VectorGetElementType(pVector, pElementType);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_Add(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector, const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    return MoDispatcherComplexTypeManager::VectorAdd(pVector, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_Get(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector, uint32_t index, OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    return MoDispatcherComplexTypeManager::VectorGet(pVector, index, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_GetSize(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector, uint32_t* pSize)
{
    return MoDispatcherComplexTypeManager::VectorGetSize(pVector, pSize);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_Clear(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector)
{
    return MoDispatcherComplexTypeManager::VectorClear(pVector);
}

void OH_AbilityRuntime_MoDispatcher_Vector_Release(OH_AbilityRuntime_MoDispatcher_VectorHandle* ppVector)
{
    MoDispatcherComplexTypeManager::VectorRelease(ppVector);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_Create(
    OH_AbilityRuntime_MoDispatcher_TypeInfo* elementType, OH_AbilityRuntime_MoDispatcher_SetHandle* ppSet)
{
    if (elementType == nullptr || ppSet == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return MoDispatcherComplexTypeManager::SetCreate(elementType, ppSet);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_GetElementType(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, OH_AbilityRuntime_MoDispatcher_TypeInfo* pElementType)
{
    if (pSet == nullptr || pElementType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return MoDispatcherComplexTypeManager::SetGetElementType(pSet, pElementType);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_Add(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    return MoDispatcherComplexTypeManager::SetAdd(pSet, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_Remove(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    return MoDispatcherComplexTypeManager::SetRemove(pSet, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_Contains(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, const OH_AbilityRuntime_MoDispatcher_Variant* pValue, bool* pExists)
{
    return MoDispatcherComplexTypeManager::SetContains(pSet, pValue, pExists);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_GetSize(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, uint32_t* pSize)
{
    return MoDispatcherComplexTypeManager::SetGetSize(pSet, pSize);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_GetAt(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, uint32_t index,
    OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{ 
    return MoDispatcherComplexTypeManager::SetGetAt(pSet, index, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_Clear(OH_AbilityRuntime_MoDispatcher_SetHandle pSet)
{
    return MoDispatcherComplexTypeManager::SetClear(pSet);
}

void OH_AbilityRuntime_MoDispatcher_Set_Release(OH_AbilityRuntime_MoDispatcher_SetHandle* ppSet)
{
    MoDispatcherComplexTypeManager::SetRelease(ppSet);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_Create(
    OH_AbilityRuntime_MoDispatcher_ValueType keyType, OH_AbilityRuntime_MoDispatcher_TypeInfo* valueType,
    OH_AbilityRuntime_MoDispatcher_MapHandle* ppMap)
{
    if (valueType == nullptr || ppMap == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return MoDispatcherComplexTypeManager::MapCreate(keyType, valueType, ppMap);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_GetKeyType(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, OH_AbilityRuntime_MoDispatcher_ValueType* pKeyType)
{
    return MoDispatcherComplexTypeManager::MapGetKeyType(pMap, pKeyType);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_GetValueType(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, OH_AbilityRuntime_MoDispatcher_TypeInfo* pValueType)
{
    if (pMap == nullptr || pValueType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return MoDispatcherComplexTypeManager::MapGetValueType(pMap, pValueType);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_Put(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, const OH_AbilityRuntime_MoDispatcher_Variant* pKey,
    const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    return MoDispatcherComplexTypeManager::MapPut(pMap, pKey, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_Get(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, const OH_AbilityRuntime_MoDispatcher_Variant* pKey,
    OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    return MoDispatcherComplexTypeManager::MapGet(pMap, pKey, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_Remove(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, const OH_AbilityRuntime_MoDispatcher_Variant* pKey)
{
    return MoDispatcherComplexTypeManager::MapRemove(pMap, pKey);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_ContainsKey(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, const OH_AbilityRuntime_MoDispatcher_Variant* pKey, bool* pExists)
{
    return MoDispatcherComplexTypeManager::MapContainsKey(pMap, pKey, pExists);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_GetSize(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, uint32_t* pSize)
{
    return MoDispatcherComplexTypeManager::MapGetSize(pMap, pSize);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_GetKeyAt(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, uint32_t index,
    OH_AbilityRuntime_MoDispatcher_Variant* pKey)
{
    return MoDispatcherComplexTypeManager::MapGetKeyAt(pMap, index, pKey);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_GetValueAt(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, uint32_t index,
    OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    return MoDispatcherComplexTypeManager::MapGetValueAt(pMap, index, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_Clear(OH_AbilityRuntime_MoDispatcher_MapHandle pMap)
{
    return MoDispatcherComplexTypeManager::MapClear(pMap);
}

void OH_AbilityRuntime_MoDispatcher_Map_Release(OH_AbilityRuntime_MoDispatcher_MapHandle* ppMap)
{
    MoDispatcherComplexTypeManager::MapRelease(ppMap);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Struct_Create(
    const char* structName, OH_AbilityRuntime_MoDispatcher_StructHandle* ppStruct)
{
    return MoDispatcherComplexTypeManager::StructCreate(structName, ppStruct);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Struct_GetName(
    OH_AbilityRuntime_MoDispatcher_StructHandle pStruct, char* pbstrName, uint32_t cMaxName)
{
    return MoDispatcherComplexTypeManager::StructGetName(pStruct, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Struct_SetField(
    OH_AbilityRuntime_MoDispatcher_StructHandle pStruct, const char* szName,
    const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    return MoDispatcherComplexTypeManager::StructSetField(pStruct, szName, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Struct_GetField(
    OH_AbilityRuntime_MoDispatcher_StructHandle pStruct, const char* szName,
    OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    return MoDispatcherComplexTypeManager::StructGetField(pStruct, szName, pValue);
}

void OH_AbilityRuntime_MoDispatcher_Struct_Release(OH_AbilityRuntime_MoDispatcher_StructHandle* ppStruct)
{
    MoDispatcherComplexTypeManager::StructRelease(ppStruct);
}

#ifdef __cplusplus
}
#endif
