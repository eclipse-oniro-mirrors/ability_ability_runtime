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

#include "ability_runtime/modular_object_extension_manager.h"

#include "modular_object_extension_manager.h"

#include <cinttypes>
#include <new>
#include <string>

#include "ability_business_error_utils.h"
#include "ability_manager_client.h"
#include "ability_manager/include/modular_object_extension_info.h"
#include "connect_options_impl.h"
#include "c_modular_object_connection_callback.h"
#include "c_modular_object_utils.h"
#include "hilog_tag_wrapper.h"
#include "modular_object_connection_manager.h"

using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;

struct OH_AbilityRuntime_AllModularObjectExtensionInfos {
    std::vector<OHOS::AAFwk::ModularObjectExtensionInfo> allMoeInfos;
};

#ifdef __cplusplus
extern "C" {
#endif

AbilityRuntime_ErrorCode OH_AbilityRuntime_ConnectModularObjectExtensionAbility(AbilityBase_Want *want,
    OH_AbilityRuntime_ConnectOptions *connectOptions, int64_t *connectionId)
{
    TAG_LOGD(AAFwkTag::EXT, "Connect Moe");
    if (connectOptions == nullptr || connectionId == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "invalid params");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (connectOptions->state == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null connectOptions state");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    {
        std::lock_guard<std::mutex> guard(connectOptions->state->mutex);
        if (!connectOptions->state->alive) {
            TAG_LOGE(AAFwkTag::EXT, "connect options already destroyed");
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
    }

    Want abilityWant;
    auto ret = CModularObjectUtils::TransformWant(want, abilityWant);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }

    auto callback = sptr<CModularObjectConnectionCallback>::MakeSptr(connectOptions->state);
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "Failed to create connect callback");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }

    int64_t newConnectionId = CModularObjectConnectionUtils::InsertConnection(callback);
    if (newConnectionId < 0) {
        TAG_LOGE(AAFwkTag::EXT, "Failed to insert connection");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }

    int32_t innerRet = ModularObjectConnectionManager::GetInstance().ConnectModularObjectExtension(
        abilityWant, callback);
    if (innerRet != ERR_OK) {
        CModularObjectConnectionUtils::RemoveConnectionCallback(newConnectionId);
        return CModularObjectUtils::ConvertConnectBusinessErrorCode(innerRet);
    }

    *connectionId = newConnectionId;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_DisconnectModularObjectExtensionAbility(int64_t connectionId)
{
    TAG_LOGD(AAFwkTag::EXT, "Disonnect Moe");
    sptr<CModularObjectConnectionCallback> callback;
    CModularObjectConnectionUtils::FindConnection(connectionId, callback);
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "Connection not found, id: %{public}" PRId64, connectionId);
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    int32_t innerRet = ModularObjectConnectionManager::GetInstance().DisconnectModularObjectExtension(callback);
    return CModularObjectUtils::ConvertConnectBusinessErrorCode(innerRet);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_ReleaseAllExtensionInfos(
    OH_AbilityRuntime_AllModObjExtensionInfosHandle *allExtensionInfos)
{
    if (!allExtensionInfos || !*allExtensionInfos) {
        TAG_LOGD(AAFwkTag::EXT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    delete *allExtensionInfos;
    *allExtensionInfos = nullptr;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoLaunchMode(
    OH_AbilityRuntime_ModObjExtensionInfoHandle extensionInfo, OH_AbilityRuntime_LaunchMode *launchMode)
{
    if (!extensionInfo || !launchMode) {
        TAG_LOGE(AAFwkTag::EXT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto info = reinterpret_cast<OHOS::AAFwk::ModularObjectExtensionInfo*>(extensionInfo);
    *launchMode = static_cast<OH_AbilityRuntime_LaunchMode>(info->launchMode);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoProcessMode(
    OH_AbilityRuntime_ModObjExtensionInfoHandle extensionInfo, OH_AbilityRuntime_ProcessMode *processMode)
{
    if (!extensionInfo || !processMode) {
        TAG_LOGE(AAFwkTag::EXT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto info = reinterpret_cast<OHOS::AAFwk::ModularObjectExtensionInfo*>(extensionInfo);
    *processMode = static_cast<OH_AbilityRuntime_ProcessMode>(info->processMode);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoThreadMode(
    OH_AbilityRuntime_ModObjExtensionInfoHandle extensionInfo, OH_AbilityRuntime_ThreadMode *threadMode)
{
    if (!extensionInfo || !threadMode) {
        TAG_LOGE(AAFwkTag::EXT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto info = reinterpret_cast<OHOS::AAFwk::ModularObjectExtensionInfo*>(extensionInfo);
    *threadMode = static_cast<OH_AbilityRuntime_ThreadMode>(info->threadMode);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoElementName(
    OH_AbilityRuntime_ModObjExtensionInfoHandle extensionInfo, AbilityBase_Element *element)
{
    if (!extensionInfo || !element) {
        TAG_LOGE(AAFwkTag::EXT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto info = reinterpret_cast<OHOS::AAFwk::ModularObjectExtensionInfo*>(extensionInfo);

    char* newBundleName = strdup(info->bundleName.c_str());
    if (!newBundleName) {
        TAG_LOGE(AAFwkTag::EXT, "strdup bundleName failed");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }

    char* newModuleName = strdup(info->moduleName.c_str());
    if (!newModuleName) {
        free(newBundleName);
        TAG_LOGE(AAFwkTag::EXT, "strdup moduleName failed");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }

    char* newAbilityName = strdup(info->abilityName.c_str());
    if (!newAbilityName) {
        free(newBundleName);
        free(newModuleName);
        TAG_LOGE(AAFwkTag::EXT, "strdup abilityName failed");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }

    element->bundleName = newBundleName;
    element->moduleName = newModuleName;
    element->abilityName = newAbilityName;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoDisableState(
    OH_AbilityRuntime_ModObjExtensionInfoHandle extensionInfo, bool *isDisabled)
{
    if (!extensionInfo || !isDisabled) {
        TAG_LOGE(AAFwkTag::EXT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto info = reinterpret_cast<OHOS::AAFwk::ModularObjectExtensionInfo*>(extensionInfo);
    *isDisabled = info->isDisabled;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_AcquireSelfModularObjectExtensionInfos(
    OH_AbilityRuntime_AllModObjExtensionInfosHandle *outOwnedAllExtensionInfos)
{
    if (!outOwnedAllExtensionInfos) {
        TAG_LOGE(AAFwkTag::EXT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    std::unique_ptr<OH_AbilityRuntime_AllModularObjectExtensionInfos> infos =
        std::make_unique<OH_AbilityRuntime_AllModularObjectExtensionInfos>();
    std::vector<OHOS::AAFwk::ModularObjectExtensionInfo> dataList;
    auto ret = OHOS::AAFwk::AbilityManagerClient::GetInstance()->QuerySelfModularObjectExtensionInfos(dataList);
    if (ret != OHOS::ERR_OK) {
        TAG_LOGE(AAFwkTag::EXT, "get modular object extension info inner error: %{public}d", ret);
        return ConvertToCommonBusinessErrorCode(ret);
    }
    infos->allMoeInfos = dataList;
    *outOwnedAllExtensionInfos = infos.release();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetCountFromAllModObjExtensionInfos(
    OH_AbilityRuntime_AllModObjExtensionInfosHandle allExtensionInfos, size_t *count)
{
    if (!allExtensionInfos || !count) {
        TAG_LOGE(AAFwkTag::EXT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *count = allExtensionInfos->allMoeInfos.size();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModObjExtensionInfoByIndex(
    OH_AbilityRuntime_AllModObjExtensionInfosHandle allExtensionInfos, size_t index,
    OH_AbilityRuntime_ModObjExtensionInfoHandle *extensionInfo)
{
    if (!allExtensionInfos || !extensionInfo) {
        TAG_LOGE(AAFwkTag::EXT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (index >= allExtensionInfos->allMoeInfos.size()) {
        TAG_LOGE(AAFwkTag::EXT, "index out of range");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *extensionInfo =
        reinterpret_cast<OH_AbilityRuntime_ModObjExtensionInfoHandle>(&(allExtensionInfos->allMoeInfos[index]));
    if (*extensionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "Failed to get extension info for index %zu", index);
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

#ifdef __cplusplus
}  // extern "C"
#endif
