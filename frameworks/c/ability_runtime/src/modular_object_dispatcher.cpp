
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

using OHOS::AbilityRuntime::ModObjDispatcherComplexTypeManager;
using OHOS::AbilityRuntime::ModObjDispatcherMetadataManager;
using OHOS::AbilityRuntime::ModObjDispatcherParamCodec;
using OHOS::MessageOption;
using OHOS::MessageParcel;

namespace OHOS::AbilityRuntime {
// Death recipient that clears cached metadata when the remote proxy dies.
// Holds weak_ptr to avoid extending metadataManager lifetime beyond the dispatcher.
class ProxyDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit ProxyDeathRecipient(std::weak_ptr<ModObjDispatcherMetadataManager> metadataManager)
        : metadataManager_(std::move(metadataManager)) {}
    ~ProxyDeathRecipient() override = default;
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override
    {
        auto mgr = metadataManager_.lock();
        if (mgr != nullptr) {
            TAG_LOGI(AAFwkTag::EXT, "Remote proxy died, clearing metadata cache");
            mgr->ClearCache();
        }
    }

private:
    std::weak_ptr<ModObjDispatcherMetadataManager> metadataManager_;
};
} // namespace OHOS::AbilityRuntime

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

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_CreateMainServiceInstance(
    OHIPCRemoteProxy* remoteProxy, OH_AbilityRuntime_ModObjDispatcherHandle* ppModObjDispatcher)
{
    if (remoteProxy == nullptr || ppModObjDispatcher == nullptr || *ppModObjDispatcher != nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "CreateMainServiceInstance: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* dispatcher = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher();
    if (dispatcher == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "Failed to allocate ModObjDispatcher instance");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    dispatcher->proxy = remoteProxy->remote;
    dispatcher->metadataManager = std::make_shared<ModObjDispatcherMetadataManager>();
    // Register death recipient to auto-release cached metadata when the remote peer dies.
    // Uses weak_ptr so the recipient does not extend metadataManager's lifetime.
    dispatcher->deathRecipient = OHOS::sptr<OHOS::AbilityRuntime::ProxyDeathRecipient>::MakeSptr(
        dispatcher->metadataManager);
    if (dispatcher->proxy != nullptr && dispatcher->deathRecipient != nullptr) {
        dispatcher->proxy->AddDeathRecipient(dispatcher->deathRecipient);
    }
    *ppModObjDispatcher = dispatcher;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_CreateSubInstance(
    OH_AbilityRuntime_ModObjDispatcherHandle mainServiceDispatcher,
    OHIPCRemoteProxy* subProxy, OH_AbilityRuntime_ModObjDispatcherHandle* ppModObjDispatcher)
{
    if (mainServiceDispatcher == nullptr || subProxy == nullptr || ppModObjDispatcher == nullptr
        || mainServiceDispatcher->metadataManager == nullptr || *ppModObjDispatcher != nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "CreateSubInstance: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* dispatcher = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher();
    if (dispatcher == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "Failed to allocate ModObjDispatcher sub-instance");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    dispatcher->proxy = subProxy->remote;
    dispatcher->metadataManager = mainServiceDispatcher->metadataManager;
    *ppModObjDispatcher = dispatcher;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void OH_AbilityRuntime_ModObjDispatcher_Release(OH_AbilityRuntime_ModObjDispatcherHandle* ppModObjDispatcher)
{
    if (ppModObjDispatcher == nullptr || *ppModObjDispatcher == nullptr) {
        return;
    }
    auto* dispatcher = *ppModObjDispatcher;
    if (dispatcher->proxy != nullptr && dispatcher->deathRecipient != nullptr) {
        dispatcher->proxy->RemoveDeathRecipient(dispatcher->deathRecipient);
    }
    dispatcher->deathRecipient = nullptr;
    delete dispatcher;
    *ppModObjDispatcher = nullptr;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_HasTypeDescriptor(
    OH_AbilityRuntime_ModObjDispatcherHandle pModObjDispatcher, uint32_t* pctinfo)
{
    if (pModObjDispatcher == nullptr || pctinfo == nullptr || pModObjDispatcher->metadataManager == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "HasTypeDescriptor: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = pModObjDispatcher->metadataManager->EnsureLoaded(pModObjDispatcher->proxy);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "HasTypeDescriptor: EnsureLoaded failed, ret=%{public}d", ret);
    }
    *pctinfo = (ret == ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) ? 1 : 0;
    return ret;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_GetTypeDescriptor(
    OH_AbilityRuntime_ModObjDispatcherHandle pModObjDispatcher,
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle* ppTypeDescriptor)
{
    if (pModObjDispatcher == nullptr || ppTypeDescriptor == nullptr || pModObjDispatcher->metadataManager == nullptr
        || *ppTypeDescriptor != nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetTypeDescriptor: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = pModObjDispatcher->metadataManager->EnsureLoaded(pModObjDispatcher->proxy);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "GetTypeDescriptor: EnsureLoaded failed, ret=%{public}d", ret);
        return ret;
    }
    auto* typeDesc = new (std::nothrow) OH_AbilityRuntime_ModularObjectDispatcher_TypeDescriptor();
    if (typeDesc == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetTypeDescriptor: allocate TypeDescriptor failed");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    typeDesc->metadataManager = pModObjDispatcher->metadataManager;
    *ppTypeDescriptor = typeDesc;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_QueryMainServiceInterfaceMemIDsOfNames(
    OH_AbilityRuntime_ModObjDispatcherHandle pModObjDispatcher, const char** rgszNames, uint32_t cNames,
    uint32_t* pMemID)
{
    if (pModObjDispatcher == nullptr || rgszNames == nullptr || pMemID == nullptr
        || pModObjDispatcher->metadataManager == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "QueryMemIDsOfNames: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = pModObjDispatcher->metadataManager->EnsureLoaded(pModObjDispatcher->proxy);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "QueryMemIDsOfNames: EnsureLoaded failed, ret=%{public}d", ret);
        return ret;
    }
    return pModObjDispatcher->metadataManager->QueryMainServiceInterfaceMemberIds(rgszNames, cNames, pMemID);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_CallMethod(
    OH_AbilityRuntime_ModObjDispatcherHandle pModObjDispatcher, uint32_t memID,
    OH_AbilityRuntime_ModObjDispatcher_InputParams* pInputParams,
    OH_AbilityRuntime_ModObjDispatcher_Variant* pResult,
    int32_t* pMethodErrCode)
{
    if (pModObjDispatcher == nullptr || pInputParams == nullptr || pResult == nullptr
        || pModObjDispatcher->metadataManager == nullptr || pMethodErrCode == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "CallMethod: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pMethodErrCode = 0;
    auto ret = pModObjDispatcher->metadataManager->EnsureLoaded(pModObjDispatcher->proxy);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "CallMethod: EnsureLoaded failed, ret=%{public}d", ret);
        return ret;
    }

    OHOS::AbilityRuntime::MoMethodMeta methodMeta;
    ret = pModObjDispatcher->metadataManager->GetMethodMeta(memID, &methodMeta);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "CallMethod: GetMethodMeta failed, memID=%{public}u, ret=%{public}d", memID, ret);
        return ret;
    }
    MessageParcel dataParcel;
    MessageParcel replyParcel;

    std::u16string ifaceDescriptor;
    ret = pModObjDispatcher->metadataManager->GetInterfaceDescriptor(
        methodMeta.interfaceName, &ifaceDescriptor);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "CallMethod: GetInterfaceDescriptor failed, ret=%{public}d", ret);
        return ret;
    }
    if (!dataParcel.WriteInterfaceToken(ifaceDescriptor)) {
        TAG_LOGE(AAFwkTag::EXT, "CallMethod: WriteInterfaceToken failed");
        return ABILITY_RUNTIME_ERROR_CODE_SEND_REQUEST_FAILED;
    }

    ret = ModObjDispatcherParamCodec::MarshalCallRequest(methodMeta, pInputParams, dataParcel);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "CallMethod: MarshalCallRequest failed, memID=%{public}u, ret=%{public}d",
            memID, ret);
        return ret;
    }

    MessageOption option(methodMeta.oneway ? MessageOption::TF_ASYNC : MessageOption::TF_SYNC);
    int ipcRet = pModObjDispatcher->proxy->SendRequest(
        methodMeta.ipcCode, dataParcel, replyParcel, option);
    if (ipcRet != 0) {
        TAG_LOGE(AAFwkTag::EXT, "CallMethod: SendRequest failed, memID=%{public}u, ipcCode=%{public}d, "
            "ipcRet=%{public}d", memID, methodMeta.ipcCode, ipcRet);
        return ABILITY_RUNTIME_ERROR_CODE_SEND_REQUEST_FAILED;
    }

    if (methodMeta.oneway) {
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    ret = ModObjDispatcherParamCodec::UnmarshalCallResult(methodMeta, replyParcel, pResult, pMethodErrCode);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "CallMethod: UnmarshalCallResult failed, memID=%{public}u, ret=%{public}d",
            memID, ret);
    }
    return ret;
}

void OH_AbilityRuntime_ModObjDispatcher_VariantClear(OH_AbilityRuntime_ModObjDispatcher_Variant* pVariant)
{
    ModObjDispatcherComplexTypeManager::Variant_Clear(pVariant);
}

void OH_AbilityRuntime_ModObjDispatcher_TypeInfoClear(OH_AbilityRuntime_ModObjDispatcher_TypeInfo* pTypeInfo)
{
    ModObjDispatcherComplexTypeManager::TypeInfo_Clear(pTypeInfo);
}

void OH_AbilityRuntime_TypeDescriptor_Release(OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle *pTypeDescriptor)
{
    if (pTypeDescriptor == nullptr || *pTypeDescriptor == nullptr) {
        return;
    }
    delete *pTypeDescriptor;
    *pTypeDescriptor = nullptr;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetVersion(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, char* pbstrVersion, uint32_t cMaxVersion)
{
    if (pTypeDescriptor == nullptr || pbstrVersion == nullptr || cMaxVersion == 0) {
        TAG_LOGE(AAFwkTag::EXT, "GetVersion: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string version;
    auto ret = pTypeDescriptor->metadataManager->GetVersion(&version);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "GetVersion: failed, ret=%{public}d", ret);
        return ret;
    }
    return CopyStringToBuffer(version, pbstrVersion, cMaxVersion);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceCount(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t* pcInterfaces)
{
    if (pTypeDescriptor == nullptr || pcInterfaces == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetInterfaceCount: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = pTypeDescriptor->metadataManager->GetInterfaceCount(pcInterfaces);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "GetInterfaceCount: failed, ret=%{public}d", ret);
    }
    return ret;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceName(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t index, char* pbstrName,
    uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || cMaxName == 0) {
        TAG_LOGE(AAFwkTag::EXT, "GetInterfaceName: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string name;
    auto ret = pTypeDescriptor->metadataManager->GetInterfaceName(index, &name);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "GetInterfaceName: failed, index=%{public}u, ret=%{public}d", index, ret);
        return ret;
    }
    return CopyStringToBuffer(name, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceIsCallback(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrName, bool* pIsCallback)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || pIsCallback == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetInterfaceIsCallback: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = pTypeDescriptor->metadataManager->GetInterfaceIsCallback(pbstrName, pIsCallback);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "GetInterfaceIsCallback: failed, name=%{public}s, ret=%{public}d", pbstrName, ret);
    }
    return ret;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMainServiceInterfaceName(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, char* pbstrName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || cMaxName == 0) {
        TAG_LOGE(AAFwkTag::EXT, "GetMainServiceInterfaceName: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string name;
    auto ret = pTypeDescriptor->metadataManager->GetMainServiceInterfaceName(&name);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "GetMainServiceInterfaceName: failed, ret=%{public}d", ret);
        return ret;
    }
    return CopyStringToBuffer(name, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodCount(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, uint32_t* pcMethods)
{
    if (pTypeDescriptor == nullptr || pbstrInterfaceName == nullptr || pcMethods == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodCount: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetMethodCount(pbstrInterfaceName, pcMethods);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodName(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, uint32_t index, char* pbstrName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrInterfaceName == nullptr || pbstrName == nullptr || cMaxName == 0) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodName: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string methodName;
    auto ret = pTypeDescriptor->metadataManager->GetMethodName(pbstrInterfaceName, index, &methodName);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodName: query failed, index=%{public}u, ret=%{public}d", index, ret);
        return ret;
    }
    return CopyStringToBuffer(methodName, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodMemberId(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, const char* pbstrMethodName, uint32_t* pMemID)
{
    if (pTypeDescriptor == nullptr || pbstrInterfaceName == nullptr || pbstrMethodName == nullptr
        || pMemID == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodMemberId: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetMethodMemberId(pbstrInterfaceName, pbstrMethodName, pMemID);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodReturnType(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, const char* pbstrMethodName,
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo* pReturnType)
{
    if (pTypeDescriptor == nullptr || pbstrInterfaceName == nullptr || pbstrMethodName == nullptr
        || pReturnType == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodReturnType: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetMethodReturnType(
        pbstrInterfaceName, pbstrMethodName, pReturnType);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodParamCount(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, const char* pbstrMethodName, uint32_t* pcParams)
{
    if (pTypeDescriptor == nullptr || pbstrInterfaceName == nullptr || pbstrMethodName == nullptr
        || pcParams == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodParamCount: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetMethodParamCount(pbstrInterfaceName, pbstrMethodName, pcParams);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodParamType(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, const char* pbstrMethodName,
    uint32_t iParamIndex, OH_AbilityRuntime_ModObjDispatcher_TypeInfo* pParamType)
{
    if (pTypeDescriptor == nullptr || pbstrInterfaceName == nullptr || pbstrMethodName == nullptr
        || pParamType == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodParamType: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetMethodParamType(
        pbstrInterfaceName, pbstrMethodName, iParamIndex, pParamType);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodParamName(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor,
    const char* pbstrInterfaceName, const char* pbstrMethodName,
    uint32_t iParamIndex, char* pbstrName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrInterfaceName == nullptr || pbstrMethodName == nullptr
        || pbstrName == nullptr || cMaxName == 0) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodParamName: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string paramName;
    auto ret = pTypeDescriptor->metadataManager->GetMethodParamName(
        pbstrInterfaceName, pbstrMethodName, iParamIndex, &paramName);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "GetMethodParamName: query failed, index=%{public}u, ret=%{public}d", iParamIndex, ret);
        return ret;
    }
    return CopyStringToBuffer(paramName, pbstrName, cMaxName);
}
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumCount(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t* pcEnums)
{
    if (pTypeDescriptor == nullptr || pcEnums == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetEnumCount: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetEnumCount(pcEnums);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumName(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t index, char* pbstrName,
    uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || cMaxName == 0) {
        TAG_LOGE(AAFwkTag::EXT, "GetEnumName: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string name;
    auto ret = pTypeDescriptor->metadataManager->GetEnumName(index, &name);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "GetEnumName: query failed, index=%{public}u, ret=%{public}d", index, ret);
        return ret;
    }
    return CopyStringToBuffer(name, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumValueCount(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrEnumName,
    uint32_t* pcValues)
{
    if (pTypeDescriptor == nullptr || pbstrEnumName == nullptr || pcValues == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetEnumValueCount: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetEnumValueCount(pbstrEnumName, pcValues);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumValueName(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrEnumName,
    uint32_t iValueIndex, char* pbstrValueName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrEnumName == nullptr || pbstrValueName == nullptr || cMaxName == 0) {
        TAG_LOGE(AAFwkTag::EXT, "GetEnumValueName: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string valueName;
    auto ret = pTypeDescriptor->metadataManager->GetEnumValueName(pbstrEnumName, iValueIndex, &valueName);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "GetEnumValueName: query failed, index=%{public}u, ret=%{public}d", iValueIndex, ret);
        return ret;
    }
    return CopyStringToBuffer(valueName, pbstrValueName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumValue(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrEnumName,
    const char* pbstrValueName, int32_t* pValue)
{
    if (pTypeDescriptor == nullptr || pbstrEnumName == nullptr || pbstrValueName == nullptr || pValue == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetEnumValue: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetEnumValue(pbstrEnumName, pbstrValueName, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructCount(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t* pcStructs)
{
    if (pTypeDescriptor == nullptr || pcStructs == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetStructCount: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetStructCount(pcStructs);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructName(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t index, char* pbstrName,
    uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || cMaxName == 0) {
        TAG_LOGE(AAFwkTag::EXT, "GetStructName: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string name;
    auto ret = pTypeDescriptor->metadataManager->GetStructName(index, &name);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "GetStructName: query failed, index=%{public}u, ret=%{public}d", index, ret);
        return ret;
    }
    return CopyStringToBuffer(name, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructFieldCount(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrStructName,
    uint32_t* pcFields)
{
    if (pTypeDescriptor == nullptr || pbstrStructName == nullptr || pcFields == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetStructFieldCount: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetStructFieldCount(pbstrStructName, pcFields);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructFieldName(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrStructName,
    uint32_t iFieldIndex, char* pbstrFieldName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrStructName == nullptr || pbstrFieldName == nullptr || cMaxName == 0) {
        TAG_LOGE(AAFwkTag::EXT, "GetStructFieldName: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string name;
    auto ret = pTypeDescriptor->metadataManager->GetStructFieldName(pbstrStructName, iFieldIndex, &name);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "GetStructFieldName: query failed, index=%{public}u, ret=%{public}d", iFieldIndex, ret);
        return ret;
    }
    return CopyStringToBuffer(name, pbstrFieldName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructFieldType(
    OH_AbilityRuntime_ModObjDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrStructName,
    const char* pbstrFieldName, OH_AbilityRuntime_ModObjDispatcher_TypeInfo* pFieldType)
{
    if (pTypeDescriptor == nullptr || pbstrStructName == nullptr || pbstrFieldName == nullptr ||
        pFieldType == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "GetStructFieldType: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetStructFieldType(pbstrStructName, pbstrFieldName, pFieldType);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_ArrayCreate(
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo* elementType, uint32_t size,
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle* ppArray)
{
    if (elementType == nullptr || ppArray == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "ArrayCreate: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return ModObjDispatcherComplexTypeManager::ArrayCreate(elementType, size, ppArray);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_ArrayGetElementType(
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle pArray, OH_AbilityRuntime_ModObjDispatcher_TypeInfo* pElementType)
{
    if (pArray == nullptr || pElementType == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "ArrayGetElementType: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return ModObjDispatcherComplexTypeManager::ArrayGetElementType(pArray, pElementType);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_ArraySet(
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle pArray, uint32_t index,
    const OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    return ModObjDispatcherComplexTypeManager::ArraySet(pArray, index, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_ArrayGet(
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle pArray, uint32_t index,
    OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    return ModObjDispatcherComplexTypeManager::ArrayGet(pArray, index, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_ArrayGetSize(
    OH_AbilityRuntime_ModObjDispatcher_ArrayHandle pArray, uint32_t* pSize)
{
    return ModObjDispatcherComplexTypeManager::ArrayGetSize(pArray, pSize);
}

void OH_AbilityRuntime_ModObjDispatcher_ArrayRelease(OH_AbilityRuntime_ModObjDispatcher_ArrayHandle* ppArray)
{
    ModObjDispatcherComplexTypeManager::ArrayRelease(ppArray);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_VectorCreate(
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo* elementType, OH_AbilityRuntime_ModObjDispatcher_VectorHandle* ppVector)
{
    if (elementType == nullptr || ppVector == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "VectorCreate: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return ModObjDispatcherComplexTypeManager::VectorCreate(elementType, ppVector);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_VectorGetElementType(
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle pVector, OH_AbilityRuntime_ModObjDispatcher_TypeInfo* pElementType)
{
    if (pVector == nullptr || pElementType == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "VectorGetElementType: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return ModObjDispatcherComplexTypeManager::VectorGetElementType(pVector, pElementType);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_VectorAdd(
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle pVector, const OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    return ModObjDispatcherComplexTypeManager::VectorAdd(pVector, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_VectorGet(
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle pVector, uint32_t index,
    OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    return ModObjDispatcherComplexTypeManager::VectorGet(pVector, index, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_VectorGetSize(
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle pVector, uint32_t* pSize)
{
    return ModObjDispatcherComplexTypeManager::VectorGetSize(pVector, pSize);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_VectorClear(
    OH_AbilityRuntime_ModObjDispatcher_VectorHandle pVector)
{
    return ModObjDispatcherComplexTypeManager::VectorClear(pVector);
}

void OH_AbilityRuntime_ModObjDispatcher_VectorRelease(OH_AbilityRuntime_ModObjDispatcher_VectorHandle* ppVector)
{
    ModObjDispatcherComplexTypeManager::VectorRelease(ppVector);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_SetCreate(
    OH_AbilityRuntime_ModObjDispatcher_TypeInfo* elementType, OH_AbilityRuntime_ModObjDispatcher_SetHandle* ppSet)
{
    if (elementType == nullptr || ppSet == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return ModObjDispatcherComplexTypeManager::SetCreate(elementType, ppSet);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_SetGetElementType(
    OH_AbilityRuntime_ModObjDispatcher_SetHandle pSet, OH_AbilityRuntime_ModObjDispatcher_TypeInfo* pElementType)
{
    if (pSet == nullptr || pElementType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return ModObjDispatcherComplexTypeManager::SetGetElementType(pSet, pElementType);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_SetAdd(
    OH_AbilityRuntime_ModObjDispatcher_SetHandle pSet, const OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    return ModObjDispatcherComplexTypeManager::SetAdd(pSet, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_SetRemove(
    OH_AbilityRuntime_ModObjDispatcher_SetHandle pSet, const OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    return ModObjDispatcherComplexTypeManager::SetRemove(pSet, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_SetContains(
    OH_AbilityRuntime_ModObjDispatcher_SetHandle pSet,
    const OH_AbilityRuntime_ModObjDispatcher_Variant* pValue, bool* pExists)
{
    return ModObjDispatcherComplexTypeManager::SetContains(pSet, pValue, pExists);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_SetGetSize(
    OH_AbilityRuntime_ModObjDispatcher_SetHandle pSet, uint32_t* pSize)
{
    return ModObjDispatcherComplexTypeManager::SetGetSize(pSet, pSize);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_SetGetAt(
    OH_AbilityRuntime_ModObjDispatcher_SetHandle pSet, uint32_t index,
    OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    return ModObjDispatcherComplexTypeManager::SetGetAt(pSet, index, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_SetClear(OH_AbilityRuntime_ModObjDispatcher_SetHandle pSet)
{
    return ModObjDispatcherComplexTypeManager::SetClear(pSet);
}

void OH_AbilityRuntime_ModObjDispatcher_SetRelease(OH_AbilityRuntime_ModObjDispatcher_SetHandle* ppSet)
{
    ModObjDispatcherComplexTypeManager::SetRelease(ppSet);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapCreate(
    OH_AbilityRuntime_ModObjDispatcher_ValueType keyType, OH_AbilityRuntime_ModObjDispatcher_TypeInfo* valueType,
    OH_AbilityRuntime_ModObjDispatcher_MapHandle* ppMap)
{
    if (valueType == nullptr || ppMap == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "MapCreate: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return ModObjDispatcherComplexTypeManager::MapCreate(keyType, valueType, ppMap);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapGetKeyType(
    OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap, OH_AbilityRuntime_ModObjDispatcher_ValueType* pKeyType)
{
    return ModObjDispatcherComplexTypeManager::MapGetKeyType(pMap, pKeyType);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapGetValueType(
    OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap, OH_AbilityRuntime_ModObjDispatcher_TypeInfo* pValueType)
{
    if (pMap == nullptr || pValueType == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "MapGetValueType: invalid param");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return ModObjDispatcherComplexTypeManager::MapGetValueType(pMap, pValueType);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapPut(
    OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap, const OH_AbilityRuntime_ModObjDispatcher_Variant* pKey,
    const OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    return ModObjDispatcherComplexTypeManager::MapPut(pMap, pKey, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapGet(
    OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap, const OH_AbilityRuntime_ModObjDispatcher_Variant* pKey,
    OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    return ModObjDispatcherComplexTypeManager::MapGet(pMap, pKey, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapRemove(
    OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap, const OH_AbilityRuntime_ModObjDispatcher_Variant* pKey)
{
    return ModObjDispatcherComplexTypeManager::MapRemove(pMap, pKey);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapContainsKey(
    OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap,
    const OH_AbilityRuntime_ModObjDispatcher_Variant* pKey, bool* pExists)
{
    return ModObjDispatcherComplexTypeManager::MapContainsKey(pMap, pKey, pExists);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapGetSize(
    OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap, uint32_t* pSize)
{
    return ModObjDispatcherComplexTypeManager::MapGetSize(pMap, pSize);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapGetKeyAt(
    OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap, uint32_t index,
    OH_AbilityRuntime_ModObjDispatcher_Variant* pKey)
{
    return ModObjDispatcherComplexTypeManager::MapGetKeyAt(pMap, index, pKey);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapGetValueAt(
    OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap, uint32_t index,
    OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    return ModObjDispatcherComplexTypeManager::MapGetValueAt(pMap, index, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_MapClear(OH_AbilityRuntime_ModObjDispatcher_MapHandle pMap)
{
    return ModObjDispatcherComplexTypeManager::MapClear(pMap);
}

void OH_AbilityRuntime_ModObjDispatcher_MapRelease(OH_AbilityRuntime_ModObjDispatcher_MapHandle* ppMap)
{
    ModObjDispatcherComplexTypeManager::MapRelease(ppMap);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_StructCreate(
    const char* structName, OH_AbilityRuntime_ModObjDispatcher_StructHandle* ppStruct)
{
    return ModObjDispatcherComplexTypeManager::StructCreate(structName, ppStruct);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_StructGetName(
    OH_AbilityRuntime_ModObjDispatcher_StructHandle pStruct, char* pbstrName, uint32_t cMaxName)
{
    return ModObjDispatcherComplexTypeManager::StructGetName(pStruct, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_StructSetField(
    OH_AbilityRuntime_ModObjDispatcher_StructHandle pStruct, const char* szName,
    const OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    return ModObjDispatcherComplexTypeManager::StructSetField(pStruct, szName, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjDispatcher_StructGetField(
    OH_AbilityRuntime_ModObjDispatcher_StructHandle pStruct, const char* szName,
    OH_AbilityRuntime_ModObjDispatcher_Variant* pValue)
{
    return ModObjDispatcherComplexTypeManager::StructGetField(pStruct, szName, pValue);
}

void OH_AbilityRuntime_ModObjDispatcher_StructRelease(OH_AbilityRuntime_ModObjDispatcher_StructHandle* ppStruct)
{
    ModObjDispatcherComplexTypeManager::StructRelease(ppStruct);
}

#ifdef __cplusplus
}
#endif
