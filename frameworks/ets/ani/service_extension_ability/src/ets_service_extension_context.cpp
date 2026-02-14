/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include "ets_service_extension_context.h"

#include "ability_manager_client.h"
#include "ani_common_ability_result.h"
#include "ani_common_start_options.h"
#include "ani_common_want.h"
#include "ani_enum_convert.h"
#include "remote_object_taihe_ani.h"
#include "common_fun_ani.h"
#include "ets_caller_complex.h"
#include "ets_context_utils.h"
#include "ets_error_utils.h"
#include "ets_extension_context.h"
#include "ets_start_abilities_observer.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {

std::recursive_mutex g_connectsLock;
int64_t g_serialNumber = 0;
static std::mutex g_connectsMutex;
static std::map<EtsConnectionKey, sptr<ETSServiceExtensionConnection>, EtsKeyCompare> g_connects;
const std::string APP_LINKING_ONLY = "appLinkingOnly";
const std::string KEY_REQUEST_ID = "com.ohos.param.requestId";
const std::string JSON_KEY_ERR_MSG = "errMsg";
constexpr const char *SERVICE_EXTENSION_CONTEXT_CLASS_NAME =
    "application.ServiceExtensionContext.ServiceExtensionContext";
constexpr const char *CLEANER_CLASS_NAME =
    "application.ServiceExtensionContext.Cleaner";
constexpr const int ANI_ALREADY_BINDED = 8;
constexpr const int FAILED_CODE = -1;
constexpr const char *SIGNATURE_CONNECT_SERVICE_EXTENSION =
    "C{@ohos.app.ability.Want.Want}C{ability.connectOptions.ConnectOptions}:l";
constexpr const char *SIGNATURE_CONNECT_SERVICE_EXTENSION_WITH_ACCOUNT =
    "C{@ohos.app.ability.Want.Want}iC{ability.connectOptions.ConnectOptions}:l";
constexpr const char *SIGNATURE_DISCONNECT_SERVICE_EXTENSION = "lC{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char* SIGNATURE_OPEN_ATOMIC_SERVICE = "C{std.core.String}C{utils.AbilityUtils.AsyncCallbackWrapper}"
    "C{@ohos.app.ability.AtomicServiceOptions.AtomicServiceOptions}:";
constexpr const char *COMPLETION_HANDLER_FAILURE_CODE =
    "@ohos.app.ability.CompletionHandlerForAtomicService.FailureCode;";
const std::string ATOMIC_SERVICE_PREFIX = "com.atomicservice.";
constexpr int32_t ARGC_ONE = 1;
constexpr int32_t ARGC_TWO = 2;
constexpr int32_t ARGC_THREE = 3;
constexpr int32_t ARGC_FOUR = 4;

bool BindNativeMethods(ani_env *env, ani_class &cls)
{
    ani_status status = ANI_ERROR;
    std::array functions = {
        ani_native_function { "nativeTerminateSelf", "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsServiceExtensionContext::TerminateSelf) },
        ani_native_function { "nativeStartAbility",
            "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsServiceExtensionContext::StartAbility) },
        ani_native_function { "nativeStartAbility", "C{@ohos.app.ability.Want.Want}"
            "C{@ohos.app.ability.StartOptions.StartOptions}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsServiceExtensionContext::StartAbilityWithOption) },
        ani_native_function { "nativeStartServiceExtensionAbility",
            "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsServiceExtensionContext::StartServiceExtensionAbility) },
        ani_native_function { "nativeStopServiceExtensionAbility",
            "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsServiceExtensionContext::StopServiceExtensionAbility) },
        ani_native_function { "nativeConnectServiceExtensionAbility", SIGNATURE_CONNECT_SERVICE_EXTENSION,
            reinterpret_cast<void *>(EtsServiceExtensionContext::ConnectServiceExtensionAbility) },
        ani_native_function { "nativeDisconnectServiceExtensionAbility", SIGNATURE_DISCONNECT_SERVICE_EXTENSION,
            reinterpret_cast<void *>(EtsServiceExtensionContext::DisconnectServiceExtensionAbility) },
        ani_native_function{"nativeStartUIServiceExtensionAbility",
            "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void*>(EtsServiceExtensionContext::StartUIServiceExtension)},
        ani_native_function { "nativeWantCheck", "C{@ohos.app.ability.Want.Want}:",
            reinterpret_cast<void *>(EtsServiceExtensionContext::WantCheck) },
        ani_native_function {"nativeStartUIAbilities",
            "C{std.core.Array}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsServiceExtensionContext::StartUIAbilities)},
        ani_native_function { "nativeOpenAtomicService", SIGNATURE_OPEN_ATOMIC_SERVICE,
            reinterpret_cast<void *>(EtsServiceExtensionContext::OpenAtomicService) },
        ani_native_function { "nativePreStartMission",
            "C{std.core.String}C{std.core.String}C{std.core.String}C{std.core.String}"
            "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsServiceExtensionContext::PreStartMission) },
        ani_native_function { "nativeRequestModalUIExtension",
            "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsServiceExtensionContext::RequestModalUIExtension) },
        ani_native_function { "nativeConnectServiceExtensionAbilityWithAccount",
            SIGNATURE_CONNECT_SERVICE_EXTENSION_WITH_ACCOUNT,
            reinterpret_cast<void *>(EtsServiceExtensionContext::ConnectServiceExtensionAbilityWithAccount) },
        ani_native_function { "nativeStopServiceExtensionAbilityWithAccount",
            "C{@ohos.app.ability.Want.Want}iC{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsServiceExtensionContext::StopServiceExtensionAbilityWithAccount) },
        ani_native_function { "nativeStartServiceExtensionAbilityWithAccount",
            "C{@ohos.app.ability.Want.Want}iC{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsServiceExtensionContext::StartServiceExtensionAbilityWithAccount) },
        ani_native_function { "nativeStartRecentAbility", "C{@ohos.app.ability.Want.Want}"
            "C{utils.AbilityUtils.AsyncCallbackWrapper}C{@ohos.app.ability.StartOptions.StartOptions}:",
            reinterpret_cast<void *>(EtsServiceExtensionContext::StartRecentAbility) },
        ani_native_function { "nativeStartAbilityWithAccountSync", "C{@ohos.app.ability.Want.Want}"
            "iC{@ohos.app.ability.StartOptions.StartOptions}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsServiceExtensionContext::StartAbilityWithAccountAndOptions) },
        ani_native_function { "nativeStartAbilityWithAccountSync",
            "C{@ohos.app.ability.Want.Want}iC{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void *>(EtsServiceExtensionContext::StartAbilityWithAccount) },
        ani_native_function { "nativeStartAbilityAsCaller", "C{@ohos.app.ability.Want.Want}"
            "C{utils.AbilityUtils.AsyncCallbackWrapper}C{@ohos.app.ability.StartOptions.StartOptions}:",
            reinterpret_cast<void *>(EtsServiceExtensionContext::StartAbilityAsCaller) },
        ani_native_function { "nativeOpenLink", "C{std.core.String}"
            "C{utils.AbilityUtils.AsyncCallbackWrapper}C{@ohos.app.ability.OpenLinkOptions.OpenLinkOptions}:",
            reinterpret_cast<void *>(EtsServiceExtensionContext::OpenLink) },
        ani_native_function { "nativeOpenLinkCheck", "C{std.core.String}:",
            reinterpret_cast<void *>(EtsServiceExtensionContext::OpenLinkCheck) },
        ani_native_function { "nativeStartAbilityByCallWithAccount",
            "C{@ohos.app.ability.Want.Want}i:C{@ohos.app.ability.UIAbility.Caller}",
            reinterpret_cast<void *>(EtsServiceExtensionContext::StartAbilityByCallWithAccount) },
        ani_native_function { "nativeStartAbilityByCallSync",
            "C{@ohos.app.ability.Want.Want}:C{@ohos.app.ability.UIAbility.Caller}",
            reinterpret_cast<void *>(EtsServiceExtensionContext::StartAbilityByCall) },
    };
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::UI_EXT, "bind method status : %{public}d", status);
        return false;
    }
    ani_class cleanerCls = nullptr;
    status = env->FindClass(CLEANER_CLASS_NAME, &cleanerCls);
    if (status != ANI_OK || cleanerCls == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to find class, status : %{public}d", status);
        return false;
    }
    std::array CleanerMethods = {
        ani_native_function { "clean", nullptr, reinterpret_cast<void *>(EtsServiceExtensionContext::Finalizer) },
    };
    if ((status = env->Class_BindNativeMethods(cleanerCls, CleanerMethods.data(), CleanerMethods.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::UI_EXT, "bind method status : %{public}d", status);
        return false;
    }
    return true;
}

int32_t InsertConnection(sptr<ETSServiceExtensionConnection> connection,
    const AAFwk::Want &want, int32_t accountId = -1)
{
    std::lock_guard<std::recursive_mutex> lock(g_connectsLock);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null connection");
        return -1;
    }
    int32_t connectId = static_cast<int32_t>(g_serialNumber);
    EtsConnectionKey key;
    key.id = g_serialNumber;
    key.want = want;
    key.accountId = accountId;
    connection->SetConnectionId(key.id);
    g_connects.emplace(key, connection);
    g_serialNumber++;
    return connectId;
}

void RemoveConnection(int32_t connectId)
{
    std::lock_guard<std::recursive_mutex> lock(g_connectsLock);
    auto item = std::find_if(g_connects.begin(), g_connects.end(),
    [&connectId](const auto &obj) {
        return connectId == obj.first.id;
    });
    if (item != g_connects.end()) {
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "remove connection ability exist");
        if (item->second) {
            item->second->RemoveConnectionObject();
        }
        g_connects.erase(item);
    } else {
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "remove connection ability not exist");
    }
}
} // namespace

void EtsServiceExtensionContext::Finalizer(ani_env *env, ani_object obj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "TerminateSelf");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    ani_long nativeEtsContextPtr;
    if (env->Object_GetFieldByName_Long(obj, "nativeEtsContext", &nativeEtsContextPtr) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to get nativeEtsContext");
        return;
    }
    if (nativeEtsContextPtr != 0) {
        delete reinterpret_cast<EtsServiceExtensionContext *>(nativeEtsContextPtr);
    }
}

void EtsServiceExtensionContext::TerminateSelf(ani_env *env, ani_object aniObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "TerminateSelf");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    etsServiceExtensionContext->OnTerminateSelf(env, aniObj, callback);
}

void EtsServiceExtensionContext::StartServiceExtensionAbility(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "StartServiceExtensionAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    etsServiceExtensionContext->OnStartServiceExtensionAbility(env, aniObj, wantObj, callbackobj);
}

void EtsServiceExtensionContext::StartAbility(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object call)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "StartAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    etsServiceExtensionContext->OnStartAbility(env, aniObj, wantObj, nullptr, call);
}

void EtsServiceExtensionContext::StartAbilityWithOption(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object opt, ani_object call)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "StartAbilityWithOption");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    etsServiceExtensionContext->OnStartAbility(env, aniObj, wantObj, opt, call);
}

void EtsServiceExtensionContext::StartUIAbilities(ani_env *env, ani_object aniObj, ani_object wantListObj,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "StartUIAbilities");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    etsServiceExtensionContext->OnStartUIAbilities(env, aniObj, wantListObj, callback);
}

void EtsServiceExtensionContext::StartRecentAbility(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object call, ani_object optionsObj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "StartRecentAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_boolean isOptionsUndefined = ANI_FALSE;
    if ((status = env->Reference_IsUndefined(optionsObj, &isOptionsUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status: %{public}d", status);
        AppExecFwk::AsyncCallback(env, call,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM), nullptr);
        return;
    }
    if (isOptionsUndefined) {
        optionsObj = nullptr;
    }
    etsServiceExtensionContext->OnStartAbility(env, aniObj, wantObj, optionsObj, call, true);
}

void EtsServiceExtensionContext::StartAbilityWithAccountAndOptions(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_int accountId, ani_object optionsObj, ani_object call)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "StartAbilityWithAccountAndOptions");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    etsServiceExtensionContext->OnStartAbilityWithAccount(env, aniObj, wantObj, accountId, optionsObj, call);
}

void EtsServiceExtensionContext::StartAbilityWithAccount(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_int accountId, ani_object call)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "StartAbilityWithAccount");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    etsServiceExtensionContext->OnStartAbilityWithAccount(env, aniObj, wantObj, accountId, nullptr, call);
}

void EtsServiceExtensionContext::StartAbilityAsCaller(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object call, ani_object optionsObj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "StartAbilityAsCaller");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_boolean isOptionsUndefined = true;
    if ((status = env->Reference_IsUndefined(optionsObj, &isOptionsUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status: %{public}d", status);
        AppExecFwk::AsyncCallback(env, call,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM), nullptr);
        return;
    }
    if (isOptionsUndefined) {
        optionsObj = nullptr;
    }
    etsServiceExtensionContext->OnStartAbilityAsCaller(env, aniObj, wantObj, call, optionsObj);
}

void EtsServiceExtensionContext::StopServiceExtensionAbility(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "StopServiceExtensionAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    etsServiceExtensionContext->OnStopServiceExtensionAbility(env, aniObj, wantObj, callbackobj);
}

ani_long EtsServiceExtensionContext::ConnectServiceExtensionAbility(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_object connectOptionsObj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "ConnectServiceExtensionAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return FAILED_CODE;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return FAILED_CODE;
    }
    return etsServiceExtensionContext->OnConnectServiceExtensionAbility(env, aniObj, wantObj, connectOptionsObj);
}

ani_long EtsServiceExtensionContext::ConnectServiceExtensionAbilityWithAccount(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_int accountId, ani_object connectOptionsObj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "ConnectServiceExtensionAbilityWithAccount");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return FAILED_CODE;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return FAILED_CODE;
    }
    return etsServiceExtensionContext->OnConnectServiceExtensionAbilityWithAccount(env, aniObj, wantObj,
        accountId, connectOptionsObj);
}

void EtsServiceExtensionContext::DisconnectServiceExtensionAbility(ani_env *env, ani_object aniObj,
    ani_long connectId, ani_object callback)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "DisconnectServiceExtensionAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    etsServiceExtensionContext->OnDisconnectServiceExtensionAbility(env, aniObj, connectId, callback);
}

void EtsServiceExtensionContext::WantCheck(ani_env *env, ani_object aniObj, ani_object wantObj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "WantCheck");
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env or aniObj");
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "UnwrapWant failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, want must be Want");
    }
}

void EtsServiceExtensionContext::StartUIServiceExtension(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "StartUIServiceExtension called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    etsServiceExtensionContext->OnStartUIServiceExtension(env, wantObj, callback);
}

void EtsServiceExtensionContext::PreStartMission(ani_env *env, ani_object aniObj, ani_string aniBundleName,
    ani_string aniModuleName, ani_string aniAbilityName, ani_string aniStartTime, ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "PreStartMission");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    etsServiceExtensionContext->OnPreStartMission(env, aniObj, aniBundleName, aniModuleName, aniAbilityName,
        aniStartTime, callbackobj);
}

void EtsServiceExtensionContext::RequestModalUIExtension(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "RequestModalUIExtension");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    etsServiceExtensionContext->OnRequestModalUIExtension(env, aniObj, wantObj, callbackobj);
}

void EtsServiceExtensionContext::OnStartUIServiceExtension(ani_env *env, ani_object wantObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, " OnStartUIServiceExtensioncalled");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "UnwrapWant failed");
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }
    int32_t innerErrCode = static_cast<int32_t>(ERR_OK);
    innerErrCode = context->StartUIServiceExtensionAbility(want);
    TAG_LOGD(AAFwkTag::CONTEXT, "StartUIServiceExtensionAbility code:%{public}d", innerErrCode);
    AppExecFwk::AsyncCallback(env, callback,
        EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode), nullptr);
}

void EtsServiceExtensionContext::StopServiceExtensionAbilityWithAccount(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_int accountId, ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "StopServiceExtensionAbilityWithAccount");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    etsServiceExtensionContext->OnStopServiceExtensionAbilityWithAccount(env, aniObj, wantObj,
        accountId, callbackobj);
}

void EtsServiceExtensionContext::OpenAtomicService(
    ani_env *env, ani_object aniObj, ani_string aniAppId, ani_object callbackObj, ani_object optionsObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OpenAtomicService called");
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsServiceExtensionContext");
        return;
    }
    etsServiceExtensionContext->OnOpenAtomicService(env, aniObj, aniAppId, callbackObj, optionsObj);
}

void EtsServiceExtensionContext::StartServiceExtensionAbilityWithAccount(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_int accountId, ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "StartServiceExtensionAbilityWithAccount");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    etsServiceExtensionContext->OnStartServiceExtensionAbilityWithAccount(env, aniObj, wantObj,
        accountId, callbackobj);
}

static bool CheckUrl(std::string &urlValue)
{
    if (urlValue.empty()) {
        return false;
    }
    Uri uri = Uri(urlValue);
    if (uri.GetScheme().empty()) {
        return false;
    }

    return true;
}

void EtsServiceExtensionContext::OpenLinkCheck(ani_env *env, ani_object aniObj, ani_string aniLink)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OpenLinkCheck called");
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env or aniObj");
        return;
    }
    std::string link("");
    if (!AppExecFwk::GetStdString(env, aniLink, link) || (!CheckUrl(link))) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "invalid link params");
        EtsErrorUtil::ThrowInvalidParamError(env, "link parameter invalid");
    }
}

void EtsServiceExtensionContext::OpenLink(ani_env *env, ani_object aniObj, ani_string linkStr,
    ani_object callbackobj, ani_object openLinkOptionsObj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OpenLink");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return;
    }
    etsServiceExtensionContext->OnOpenLink(env, aniObj, linkStr, callbackobj, openLinkOptionsObj);
}

ani_object EtsServiceExtensionContext::StartAbilityByCallWithAccount(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_int accountId)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "StartAbilityByCallWithAccount");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return nullptr;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return nullptr;
    }
    return etsServiceExtensionContext->OnStartAbilityByCallWithAccount(env, aniObj, wantObj, accountId);
}

ani_object EtsServiceExtensionContext::StartAbilityByCall(ani_env *env, ani_object aniObj, ani_object wantObj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "StartAbilityByCall");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return nullptr;
    }
    auto etsServiceExtensionContext = EtsServiceExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsServiceExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsServiceExtensionContext");
        return nullptr;
    }
    return etsServiceExtensionContext->OnStartAbilityByCallWithAccount(env, aniObj, wantObj, DEFAULT_ACCOUNT_ID);
}

EtsServiceExtensionContext *EtsServiceExtensionContext::GetEtsAbilityContext(
    ani_env *env, ani_object aniObj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "GetEtsAbilityContext");
    ani_class cls = nullptr;
    ani_long nativeContextLong;
    ani_field contextField = nullptr;
    ani_status status = ANI_ERROR;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(SERVICE_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to find class, status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindField(cls, "nativeEtsContext", &contextField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to find field, status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_GetField_Long(aniObj, contextField, &nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to get field, status : %{public}d", status);
        return nullptr;
    }
    auto weakContext = reinterpret_cast<EtsServiceExtensionContext *>(nativeContextLong);
    return weakContext;
}

void EtsServiceExtensionContext::OnTerminateSelf(ani_env *env, ani_object obj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnTerminateSelf");
    ani_object aniObject = nullptr;
    ErrCode ret = ERR_INVALID_VALUE;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "context is nullptr");
        ret = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ret));
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    ret = context->TerminateAbility();
    AppExecFwk::AsyncCallback(env, callback,
        EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
}

void EtsServiceExtensionContext::OnStartServiceExtensionAbility(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnStartServiceExtensionAbility");
    ani_object aniObject = nullptr;
    ErrCode ret = ERR_OK;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "context is nullptr");
        ret = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ret));
        AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "UnwrapWant failed");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "UnwrapWant failed");
        AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
        return;
    }
    ret = context->StartServiceExtensionAbility(want);
    aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
}

void EtsServiceExtensionContext::OnStopServiceExtensionAbility(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnStopServiceExtensionAbility");
    ani_object aniObject = nullptr;
    ErrCode ret = ERR_OK;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "context is nullptr");
        ret = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ret));
        AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "UnwrapWant failed");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "UnwrapWant failed");
        AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
        return;
    }
    ret = context->StopServiceExtensionAbility(want);
    aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
}

void EtsServiceExtensionContext::OnStartAbility(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object opt, ani_object callbackObj, bool isStartRecent)
{
    ani_object aniObject = nullptr;
    AAFwk::Want want;
    ErrCode errCode = ERR_OK;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "UnwrapWant filed");
        AppExecFwk::AsyncCallback(env, callbackObj, aniObject, nullptr);
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "context is nullptr");
        aniObject = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        AppExecFwk::AsyncCallback(env, callbackObj, aniObject, nullptr);
        return;
    }
    if (isStartRecent) {
        TAG_LOGI(AAFwkTag::SERVICE_EXT, "OnStartRecentAbility is called");
        want.SetParam(AAFwk::Want::PARAM_RESV_START_RECENT, true);
    }
    if ((want.GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND) {
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::system_clock::now().time_since_epoch()).count());
        want.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
        AddFreeInstallObserver(env, want, callbackObj, context);
    }
    if (opt != nullptr) {
        AAFwk::StartOptions startOptions;
        if (!AppExecFwk::UnwrapStartOptions(env, opt, startOptions)) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "UnwrapStartOptions failed");
            aniObject = EtsErrorUtil::CreateInvalidParamError(env, "UnwrapStartOptions failed");
            AppExecFwk::AsyncCallback(env, callbackObj, aniObject, nullptr);
            return;
        }
        errCode = context->StartAbilityWithAccount(want, DEFAULT_INVAL_VALUE, startOptions);
    } else {
        errCode = context->StartAbilityWithAccount(want, DEFAULT_INVAL_VALUE);
    }
    aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, errCode);
    if ((want.GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND) {
        if (errCode != ERR_OK && freeInstallObserver_ != nullptr) {
            std::string bundleName = want.GetElement().GetBundleName();
            std::string abilityName = want.GetElement().GetAbilityName();
            std::string startTime = want.GetStringParam(AAFwk::Want::PARAM_RESV_START_TIME);
            freeInstallObserver_->OnInstallFinished(bundleName, abilityName, startTime, errCode);
        }
    } else {
        AppExecFwk::AsyncCallback(env, callbackObj, aniObject, nullptr);
    }
}

void EtsServiceExtensionContext::OnStartAbilityWithAccount(ani_env *env, ani_object obj, ani_object wantObj,
    ani_int accountId, ani_object optionsObj, ani_object callbackObj)
{
    ani_object aniObject = nullptr;
    AAFwk::Want want;
    ErrCode errCode = ERR_OK;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "Parse param want failed, must be a Want.");
        AppExecFwk::AsyncCallback(env, callbackObj, aniObject, nullptr);
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "context is nullptr");
        errCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(errCode));
        AppExecFwk::AsyncCallback(env, callbackObj, aniObject, nullptr);
        return;
    }
    if ((want.GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND) {
        std::string startTime = std::to_string(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
                .count());
        want.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
        AddFreeInstallObserver(env, want, callbackObj, context);
    }
    if (optionsObj != nullptr) {
        AAFwk::StartOptions startOptions;
        AppExecFwk::UnwrapStartOptions(env, optionsObj, startOptions);
        errCode = context->StartAbilityWithAccount(want, accountId, startOptions);
    } else {
        errCode = context->StartAbilityWithAccount(want, accountId);
    }
    aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, errCode);
    if ((want.GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND) {
        if (errCode != ERR_OK && freeInstallObserver_ != nullptr) {
            std::string bundleName = want.GetElement().GetBundleName();
            std::string abilityName = want.GetElement().GetAbilityName();
            std::string startTime = want.GetStringParam(AAFwk::Want::PARAM_RESV_START_TIME);
            freeInstallObserver_->OnInstallFinished(bundleName, abilityName, startTime, errCode);
        }
        return;
    }
    AppExecFwk::AsyncCallback(env, callbackObj, aniObject, nullptr);
}

void EtsServiceExtensionContext::OnStartAbilityAsCaller(ani_env *env, ani_object obj, ani_object wantObj,
    ani_object callbackObj, ani_object optionsObj)
{
    ani_object aniObject = nullptr;
    AAFwk::Want want;
    ErrCode errCode = ERR_OK;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "Parse param want failed, must be a Want.");
        AppExecFwk::AsyncCallback(env, callbackObj, aniObject, nullptr);
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "context is nullptr");
        aniObject = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        AppExecFwk::AsyncCallback(env, callbackObj, aniObject, nullptr);
        return;
    }

    if (optionsObj != nullptr) {
        AAFwk::StartOptions startOptions;
        if (!AppExecFwk::UnwrapStartOptions(env, optionsObj, startOptions)) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "UnwrapStartOptions failed");
            aniObject = EtsErrorUtil::CreateInvalidParamError(env,
                "Parse param startOptions failed, startOptions must be StartOptions.");
            AppExecFwk::AsyncCallback(env, callbackObj, aniObject, nullptr);
            return;
        }
        errCode = context->StartAbilityAsCaller(want, startOptions);
    } else {
        errCode = context->StartAbilityAsCaller(want);
    }
    aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, errCode);
    AppExecFwk::AsyncCallback(env, callbackObj, aniObject, nullptr);
}

ani_long EtsServiceExtensionContext::OnConnectServiceExtensionAbility(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_object connectOptionsObj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnConnectServiceExtensionAbility call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return FAILED_CODE;
    }
    AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to UnwrapWant");
        EtsErrorUtil::ThrowInvalidParamError(env, "Failed to UnwrapWant");
        return FAILED_CODE;
    }
    ani_vm *etsVm = nullptr;
    if (env->GetVM(&etsVm) != ANI_OK || etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to getVM");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return FAILED_CODE;
    }
    sptr<ETSServiceExtensionConnection> connection = sptr<ETSServiceExtensionConnection>::MakeSptr(etsVm);
    connection->SetConnectionRef(connectOptionsObj);
    int32_t connectId = InsertConnection(connection, want);
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        RemoveConnection(connectId);
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
    }
    auto innerErrCode = context->ConnectAbility(want, connection);
    int32_t errcode = static_cast<int32_t>(GetJsErrorCodeByNativeError(innerErrCode));
    if (errcode) {
        connection->CallEtsFailed(errcode);
        RemoveConnection(connectId);
    }
    return connectId;
}

ani_long EtsServiceExtensionContext::OnConnectServiceExtensionAbilityWithAccount(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_int accountId, ani_object connectOptionsObj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnConnectServiceExtensionAbilityWithAccount call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return FAILED_CODE;
    }
    AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to UnwrapWant");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, must be a Want");
        return FAILED_CODE;
    }
    ani_vm *etsVm = nullptr;
    if (env->GetVM(&etsVm) != ANI_OK || etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to getVM");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return FAILED_CODE;
    }
    sptr<ETSServiceExtensionConnection> connection = sptr<ETSServiceExtensionConnection>::MakeSptr(etsVm);
    connection->SetConnectionRef(connectOptionsObj);
    int32_t connectId = InsertConnection(connection, want, accountId);
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        RemoveConnection(connectId);
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
    }
    auto innerErrCode = context->ConnectAbilityWithAccount(want, accountId, connection);
    int32_t errcode = static_cast<int32_t>(GetJsErrorCodeByNativeError(innerErrCode));
    if (errcode) {
        connection->CallEtsFailed(errcode);
        RemoveConnection(connectId);
    }
    return static_cast<ani_long>(connectId);
}

void EtsServiceExtensionContext::OnDisconnectServiceExtensionAbility(ani_env *env, ani_object aniObj,
    ani_long connectId, ani_object callback)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnDisconnectServiceExtensionAbility call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    auto context = context_.lock();
    ani_object errorObject = nullptr;
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        errorObject = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        AppExecFwk::AsyncCallback(env, callback, errorObject, nullptr);
        return;
    }
    sptr<ETSServiceExtensionConnection> connection = nullptr;
    AAFwk::Want want;
    int32_t accountId = -1;
    {
        std::lock_guard<std::mutex> lock(g_connectsMutex);
        auto iter = std::find_if(
            g_connects.begin(), g_connects.end(), [&connectId](const auto &obj) { return connectId == obj.first.id; });
        if (iter != g_connects.end()) {
            want = iter->first.want;
            connection = iter->second;
            accountId = iter->first.accountId;
            g_connects.erase(iter);
        } else {
            TAG_LOGI(AAFwkTag::SERVICE_EXT, "Failed to found connection");
        }
    }
    if (!connection) {
        errorObject = EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
        AppExecFwk::AsyncCallback(env, callback, errorObject, nullptr);
        return;
    }
    context->DisconnectAbility(want, connection, accountId);
    AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
}

bool EtsServiceExtensionContext::UnwrapWantList(ani_env *env, ani_object wantListObj,
    std::vector<AAFwk::Want> &wantList)
{
    ani_array wantListArray = reinterpret_cast<ani_array>(wantListObj);
    ani_size arrayLength = 0;
    if (env->Array_GetLength(wantListArray, &arrayLength) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to get array length.");
        return false;
    }
    if (arrayLength < ARGC_ONE || arrayLength > ARGC_FOUR) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "wantList size not support");
        return false;
    }
    for (ani_size i = 0; i < arrayLength; i++) {
        ani_ref wantRef  = nullptr;
        if (env->Array_Get(wantListArray, i, &wantRef) != ANI_OK || wantRef == nullptr) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to get want object");
            return false;
        }
        ani_object wantObj = reinterpret_cast<ani_object>(wantRef);
        AAFwk::Want curWant;
        if (!OHOS::AppExecFwk::UnwrapWant(env, wantObj, curWant)) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "startUIAbilities parse want failed");
            return false;
        }
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "startUIAbilities ability:%{public}s",
            curWant.GetElement().GetAbilityName().c_str());
        wantList.emplace_back(curWant);
    }
    return true;
}

void EtsServiceExtensionContext::OnStartUIAbilities(ani_env *env, ani_object aniObj, ani_object wantListObj,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnStartUIAbilities");

    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INNER), nullptr);
        return;
    }
    ani_vm *etsVm = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GetVM(&etsVm)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return;
    }

    ErrCode innerErrCode = ERR_OK;
    std::vector<AAFwk::Want> wantList;
    std::string requestKey = std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    if (!EtsServiceExtensionContext::UnwrapWantList(env, wantListObj, wantList)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Unwrap wantList param failed.");
        ani_object aniObject = EtsErrorUtil::CreateInvalidParamError(env, "UnwrapWant filed");
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "startUIAbilities wantListLength: %{public}zu", wantList.size());
    EtsStartAbilitiesObserver::GetInstance().SetEtsVm(etsVm);
    EtsStartAbilitiesObserver::GetInstance().AddObserver(env, requestKey, callback);

    auto context = context_.lock();
    if (!context) {
        TAG_LOGW(AAFwkTag::SERVICE_EXT, "null context");
        innerErrCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        ani_object aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(innerErrCode));
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    innerErrCode = context->StartUIAbilities(wantList, requestKey);

    TAG_LOGD(AAFwkTag::SERVICE_EXT, "startUIAbilities complete innerErrCode: %{public}d", innerErrCode);
    if (innerErrCode == AAFwk::START_UI_ABILITIES_WAITING_SPECIFIED_CODE) {
        TAG_LOGI(AAFwkTag::SERVICE_EXT, "startUIAbilities waiting specified.");
        ani_object aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode);
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    EtsStartAbilitiesObserver::HandleFinished(requestKey, innerErrCode);
}

void EtsServiceExtensionContext::ClearFailedCallConnection(
    std::shared_ptr<ServiceExtensionContext> context, const std::shared_ptr<CallerCallBack> &callback)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "called");
    if (context == nullptr || callback == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context or callback");
        return;
    }

    context->ClearFailedCallConnection(callback);
}

void EtsServiceExtensionContext::AddFreeInstallObserver(ani_env *env, const AAFwk::Want &want, ani_object callbackObj,
    std::shared_ptr<ServiceExtensionContext> context, bool isAbilityResult, bool isOpenLink)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "AddFreeInstallObserver");
    if (!context) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        return;
    }
    if (!env) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    if (freeInstallObserver_ == nullptr) {
        ani_vm *etsVm = nullptr;
        ani_status status = ANI_ERROR;
        if ((status = env->GetVM(&etsVm)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
            return;
        }
        freeInstallObserver_ = new EtsFreeInstallObserver(etsVm);
        if (context->AddFreeInstallObserver(freeInstallObserver_)) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "addFreeInstallObserver error");
            return;
        }
    }
    std::string startTime = want.GetStringParam(AAFwk::Want::PARAM_RESV_START_TIME);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "addEtsObserver");
    if (isOpenLink) {
        std::string url = want.GetUriString();
        freeInstallObserver_->AddEtsObserverObject(env, startTime, url, callbackObj, isAbilityResult);
        return;
    }
    std::string bundleName = want.GetElement().GetBundleName();
    std::string abilityName = want.GetElement().GetAbilityName();
    freeInstallObserver_->AddEtsObserverObject(env, bundleName, abilityName, startTime, callbackObj, isAbilityResult);
}

ani_object CreateEtsServiceExtensionContext(ani_env *env, std::shared_ptr<ServiceExtensionContext> context)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "CreateEtsServiceExtensionContext");
    if (env == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env or context");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object contextObj = nullptr;
    if ((env->FindClass(SERVICE_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to find class, status : %{public}d", status);
        return nullptr;
    }
    if (!BindNativeMethods(env, cls)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to BindNativeMethods");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", "l:", &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to find constructor, status : %{public}d", status);
        return nullptr;
    }
    std::unique_ptr<EtsServiceExtensionContext> workContext = std::make_unique<EtsServiceExtensionContext>(context);
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to create etsServiceExtensionContext");
        return nullptr;
    }
    auto serviceContextPtr = new std::weak_ptr<ServiceExtensionContext> (workContext->GetAbilityContext());
    if ((status = env->Object_New(cls, method, &contextObj, (ani_long)workContext.release())) != ANI_OK ||
        contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to create object, status : %{public}d", status);
        return nullptr;
    }
    if (!ContextUtil::SetNativeContextLong(env, contextObj, (ani_long)(serviceContextPtr))) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to setNativeContextLong ");
        return nullptr;
    }
    ContextUtil::CreateEtsBaseContext(env, cls, contextObj, context);
    CreateEtsExtensionContext(env, cls, contextObj, context, context->GetAbilityInfo());
    ani_ref *contextGlobalRef = new (std::nothrow) ani_ref;
    if (contextGlobalRef == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null contextGlobalRef");
        return nullptr;
    }
    if ((status = env->GlobalReference_Create(contextObj, contextGlobalRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "GlobalReference_Create failed status: %{public}d", status);
        delete contextGlobalRef;
        return nullptr;
    }
    context->Bind(contextGlobalRef);
    return contextObj;
}

ETSServiceExtensionConnection::ETSServiceExtensionConnection(ani_vm *etsVm) : etsVm_(etsVm) {}

ETSServiceExtensionConnection::~ETSServiceExtensionConnection()
{
    RemoveConnectionObject();
}

void ETSServiceExtensionConnection::SetConnectionId(int32_t id)
{
    connectionId_ = id;
}

void ETSServiceExtensionConnection::RemoveConnectionObject()
{
    if (etsVm_ != nullptr && stsConnectionRef_ != nullptr) {
        ani_env *env = nullptr;
        if (etsVm_->GetEnv(ANI_VERSION_1, &env) == ANI_OK && env != nullptr) {
            env->GlobalReference_Delete(stsConnectionRef_);
            stsConnectionRef_ = nullptr;
        }
    }
}

void ETSServiceExtensionConnection::CallEtsFailed(int32_t errorCode)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "CallEtsFailed");
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsVm");
        return;
    }
    if (stsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null stsConnectionRef_");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_OK;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to get env, status: %{public}d", status);
        return;
    }
    ani_ref funRef;
    if ((status = env->Object_GetPropertyByName_Ref(reinterpret_cast<ani_object>(stsConnectionRef_),
        "onFailed", &funRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "get onFailed failed status : %{public}d", status);
        return;
    }
    if (!AppExecFwk::IsValidProperty(env, funRef)) {
        TAG_LOGI(AAFwkTag::SERVICE_EXT, "invalid onFailed property");
        return;
    }
    ani_object errorCodeObj = AppExecFwk::CreateInt(env, errorCode);
    if (errorCodeObj == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null errorCodeObj");
        return;
    }
    ani_ref result;
    std::vector<ani_ref> argv = { errorCodeObj };
    if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_ONE, argv.data(),
        &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to call onFailed, status: %{public}d", status);
    }
}

void ETSServiceExtensionConnection::SetConnectionRef(ani_object connectOptionsObj)
{
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "etsVm_ is nullptr");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status: %{public}d", status);
        return;
    }
    if ((status = env->GlobalReference_Create(connectOptionsObj, &stsConnectionRef_)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status: %{public}d", status);
    }
}

void ETSServiceExtensionConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnAbilityConnectDone");
    if (etsVm_ == nullptr || stsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null stsConnectionRef or etsVm");
        return;
    }
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "GetEnv failed");
        return;
    }
    ani_ref refElement = AppExecFwk::WrapElementName(env, element);
    if (refElement == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null refElement");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null remoteObject");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    ani_object refRemoteObject = ANI_ohos_rpc_CreateJsRemoteObject(env, remoteObject);
    if (refRemoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null refRemoteObject");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    ani_status status = ANI_ERROR;
    ani_ref funRef;
    if ((status = env->Object_GetPropertyByName_Ref(reinterpret_cast<ani_object>(stsConnectionRef_),
        "onConnect", &funRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "get onConnect failed status : %{public}d", status);
        return;
    }
    if (!AppExecFwk::IsValidProperty(env, funRef)) {
        TAG_LOGI(AAFwkTag::SERVICE_EXT, "invalid onConnect property");
        return;
    }
    ani_ref result;
    std::vector<ani_ref> argv = { refElement, refRemoteObject};
    if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_TWO, argv.data(),
        &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to call onConnect, status: %{public}d", status);
    }
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

void ETSServiceExtensionConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnAbilityDisconnectDone");
    if (etsVm_ == nullptr || stsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null stsConnectionRef or etsVm");
        return;
    }
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "GetEnv failed");
        return;
    }
    ani_ref refElement = AppExecFwk::WrapElementName(env, element);
    if (refElement == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null refElement");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    ani_status status = ANI_ERROR;
    ani_ref funRef;
    if ((status = env->Object_GetPropertyByName_Ref(reinterpret_cast<ani_object>(stsConnectionRef_),
        "onDisconnect", &funRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "get onDisconnect failed status : %{public}d", status);
        return;
    }
    if (!AppExecFwk::IsValidProperty(env, funRef)) {
        TAG_LOGI(AAFwkTag::SERVICE_EXT, "invalid onDisconnect property");
        return;
    }
    ani_ref result;
    std::vector<ani_ref> argv = { refElement };
    if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_ONE, argv.data(),
        &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to call onDisconnect, status: %{public}d", status);
    }
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

void EtsServiceExtensionContext::CreateOnAtomicRequestSuccessResultCallback(ani_env *env, ani_ref refCompletionHandler,
    OnAtomicRequestSuccess &onRequestCallback, const char *callbackName)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "CreateOnAtomicRequestSuccessResultCallback called");
    ani_vm *etsVm = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GetVM(&etsVm)) != ANI_OK || etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "GetVM failed, status: %{public}d", status);
        env->GlobalReference_Delete(refCompletionHandler);
        return;
    }
    onRequestCallback = [etsVm, refCompletionHandler, callbackName](const std::string &appId) {
        ani_status status = ANI_ERROR;
        ani_env *env = nullptr;
        if ((status = etsVm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "GetEnv failed, status: %{public}d", status);
            return;
        }
        ani_string appIdStr = nullptr;
        if (env->String_NewUTF8(appId.c_str(), appId.size(), &appIdStr) != ANI_OK || !appIdStr) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "String_NewUTF8 for appId failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        ani_ref funcRef;
        if ((status = env->Object_GetFieldByName_Ref(reinterpret_cast<ani_object>(refCompletionHandler),
            callbackName, &funcRef)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "Object_GetFieldByName_Ref failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        if (!AppExecFwk::IsValidProperty(env, funcRef)) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "IsValidProperty failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        ani_ref result = nullptr;
        std::vector<ani_ref> argv = { appIdStr };
        if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funcRef), ARGC_ONE, argv.data(),
            &result)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "FunctionalObject_Call failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        env->GlobalReference_Delete(refCompletionHandler);
    };
}

void EtsServiceExtensionContext::CreateOnAtomicRequestFailureResultCallback(ani_env *env, ani_ref refCompletionHandler,
    OnAtomicRequestFailure &onRequestCallback, const char *callbackName)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "CreateOnAtomicRequestFailureResultCallback called");
    ani_vm *etsVm = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GetVM(&etsVm)) != ANI_OK || etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "GetVM failed, status: %{public}d", status);
        env->GlobalReference_Delete(refCompletionHandler);
        return;
    }
    onRequestCallback = [etsVm, refCompletionHandler, callbackName](const std::string &appId,
        int32_t failureCode, const std::string &message) {
        ani_status status = ANI_ERROR;
        ani_env *env = nullptr;
        if ((status = etsVm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "GetEnv failed, status: %{public}d", status);
            return;
        }
        ani_string appIdStr = nullptr;
        if (env->String_NewUTF8(appId.c_str(), appId.size(), &appIdStr) != ANI_OK || !appIdStr) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "String_NewUTF8 for appId failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        ani_enum_item failureCodeObj {};
        OHOS::AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(
            env, COMPLETION_HANDLER_FAILURE_CODE, failureCode, failureCodeObj);
        ani_string messageStr = nullptr;
        if (env->String_NewUTF8(message.c_str(), message.size(), &messageStr) != ANI_OK || !messageStr) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "String_NewUTF8 for messageStr failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        ani_ref funcRef;
        if ((status = env->Object_GetFieldByName_Ref(reinterpret_cast<ani_object>(refCompletionHandler),
            callbackName, &funcRef)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "Object_GetFieldByName_Ref failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        if (!AppExecFwk::IsValidProperty(env, funcRef)) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "IsValidProperty failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        ani_ref result = nullptr;
        std::vector<ani_ref> argv = { appIdStr, failureCodeObj, messageStr };
        if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funcRef), ARGC_THREE, argv.data(),
            &result)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "FunctionalObject_Call failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        env->GlobalReference_Delete(refCompletionHandler);
    };
}

void EtsServiceExtensionContext::UnWrapCompletionHandlerForAtomicService(
    ani_env *env, ani_object param, AAFwk::StartOptions &options, const std::string &appId)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "UnWrapCompletionHandlerForAtomicService called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        return;
    }
    ani_ref completionHandler;
    if (!AppExecFwk::GetFieldRefByName(env, param, "completionHandlerForAtomicService", completionHandler) ||
        !completionHandler) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null completionHandlerForAtomicService");
        return;
    }
    ani_ref refCompletionHandler = nullptr;
    if (env->GlobalReference_Create(completionHandler, &refCompletionHandler) != ANI_OK ||
        !refCompletionHandler) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to create global ref for completionHandler.");
        return;
    }
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "completionHandlerForAtomicService exists");
    OnAtomicRequestSuccess onRequestSucc;
    OnAtomicRequestFailure onRequestFail;
    CreateOnAtomicRequestSuccessResultCallback(env, refCompletionHandler, onRequestSucc,
        "onAtomicServiceRequestSuccess");
    CreateOnAtomicRequestFailureResultCallback(env, refCompletionHandler, onRequestFail,
        "onAtomicServiceRequestFailure");
    std::string requestId =
        std::to_string(static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count()));
    if (context->AddCompletionHandlerForAtomicService(requestId, onRequestSucc, onRequestFail, appId) != ERR_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "add completionHandler failed");
        return;
    }
    options.requestId_ = requestId;
}

void EtsServiceExtensionContext::OnOpenAtomicService(
    ani_env *env, ani_object aniObj, ani_string aniAppId, ani_object callbackObj, ani_object optionsObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnOpenAtomicService");
    ani_status status = ANI_ERROR;
    ani_boolean isOptionsUndefined = true;
    ani_object errorObject = nullptr;
    if ((status = env->Reference_IsUndefined(optionsObj, &isOptionsUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        return;
    }
    std::string appId;
    if (!AppExecFwk::GetStdString(env, aniAppId, appId)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "parse appId failed");
        errorObject = EtsErrorUtil::CreateInvalidParamError(env, "Parse param appId failed, appId must be string.");
        AppExecFwk::AsyncCallback(env, callbackObj, errorObject, nullptr);
        return;
    }
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    if (!isOptionsUndefined) {
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "atomic service option is used.");
        if (!AppExecFwk::UnwrapAtomicServiceOptions(env, optionsObj, want, startOptions)) {
            TAG_LOGE(AAFwkTag::CONTEXT, "invalid atomic service options");
            errorObject = EtsErrorUtil::CreateInvalidParamError(env,
                "Parse param startOptions failed, startOptions must be StartOption.");
            AppExecFwk::AsyncCallback(env, callbackObj, errorObject, nullptr);
            return;
        }
        UnWrapCompletionHandlerForAtomicService(env, optionsObj, startOptions, appId);
    }
    OpenAtomicServiceInner(env, aniObj, want, startOptions, appId, callbackObj);
}

void EtsServiceExtensionContext::OpenAtomicServiceInner(ani_env *env, ani_object aniObj, AAFwk::Want &want,
    AAFwk::StartOptions &options, std::string appId, ani_object callbackObj)
{
    std::string bundleName = ATOMIC_SERVICE_PREFIX + appId;
    TAG_LOGD(AAFwkTag::CONTEXT, "bundleName: %{public}s", bundleName.c_str());
    want.SetBundle(bundleName);
    want.AddFlags(AAFwk::Want::FLAG_INSTALL_ON_DEMAND);
    std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count());
    want.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    AddFreeInstallObserver(env, want, callbackObj, context);

    ErrCode errCode = context->OpenAtomicService(want, options);
    if (errCode != 0) {
        TAG_LOGE(AAFwkTag::CONTEXT, "OpenAtomicService failed: %{public}d", errCode);
        if (freeInstallObserver_ != nullptr) {
            std::string bundleName = want.GetElement().GetBundleName();
            std::string abilityName = want.GetElement().GetAbilityName();
            freeInstallObserver_->OnInstallFinished(bundleName, abilityName, startTime, errCode);
        }
        if (!options.requestId_.empty()) {
            nlohmann::json jsonObject = nlohmann::json {
                { JSON_KEY_ERR_MSG, "failed to call openAtomicService" }
            };
            context->OnRequestFailure(options.requestId_, want.GetElement(), jsonObject.dump());
        }
    }
}

void EtsServiceExtensionContext::OnPreStartMission(ani_env *env, ani_object aniObj, ani_string aniBundleName,
    ani_string aniModuleName, ani_string aniAbilityName, ani_string aniStartTime, ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnPreStartMission");
    ani_object aniObject = nullptr;
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    std::string startTime;
    if (!AppExecFwk::GetStdString(env, aniBundleName, bundleName) ||
        !AppExecFwk::GetStdString(env, aniModuleName, moduleName) ||
        !AppExecFwk::GetStdString(env, aniAbilityName, abilityName) ||
        !AppExecFwk::GetStdString(env, aniStartTime, startTime)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "parse preStartMission failed");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "Parse params failed, params must be strings.");
        AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "context is nullptr");
        aniObject = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
        return;
    }
    auto ret = context->PreStartMission(bundleName, moduleName, abilityName, startTime);
    aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
}

void EtsServiceExtensionContext::OnRequestModalUIExtension(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnRequestModalUIExtension");
    ani_object aniObject = nullptr;
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "parse want failed");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "Parse param want failed, must be a Want.");
        AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "context is nullptr");
        aniObject = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INNER);
        AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
        return;
    }
    auto innerErrCode = AAFwk::AbilityManagerClient::GetInstance()->RequestModalUIExtension(want);
    aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(innerErrCode));
    AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
}

void EtsServiceExtensionContext::OnStopServiceExtensionAbilityWithAccount(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_int accountId, ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnStopServiceExtensionAbilityWithAccount");
    ani_object aniObject = nullptr;
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "parse want failed");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "Parse param want failed, want must be Want.");
        AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "context is nullptr");
        aniObject = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
        return;
    }
    auto innerErrCode = context->StopServiceExtensionAbility(want, accountId);
    aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(innerErrCode));
    AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
}

void EtsServiceExtensionContext::OnStartServiceExtensionAbilityWithAccount(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_int accountId, ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnStartServiceExtensionAbilityWithAccount");
    ani_object aniObject = nullptr;
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "parse want failed");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "Parse param want failed, want must be Want.");
        AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "context is nullptr");
        aniObject = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
        return;
    }
    auto innerErrCode = context->StartServiceExtensionAbility(want, accountId);
    aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(innerErrCode));
    AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
}

void EtsServiceExtensionContext::OnOpenLink(ani_env *env, ani_object aniObj, ani_string aniLink,
    ani_object callbackobj, ani_object openLinkOptionsObj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnOpenLink");
    ani_object aniObject = nullptr;
    std::string link;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    if (!AppExecFwk::GetStdString(env, aniLink, link) || !CheckUrl(link)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "parse link failed");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "Parse param link failed, link must be string.");
        AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
        return;
    }
    ani_status status = ANI_ERROR;
    ani_boolean isOptionsUndefined = true;
    if ((status = env->Reference_IsUndefined(openLinkOptionsObj, &isOptionsUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status: %{public}d", status);
    }
    AAFwk::Want want;
    want.SetParam(APP_LINKING_ONLY, false);
    AAFwk::OpenLinkOptions openLinkOptions;
    if (!isOptionsUndefined) {
        TAG_LOGI(AAFwkTag::SERVICE_EXT, "openlink have option");
        AppExecFwk::UnWrapOpenLinkOptions(env, openLinkOptionsObj, openLinkOptions, want);
        OnRequestResult onRequestSucc;
        OnRequestResult onRequestFail;
        ani_ref refCompletionHandler = nullptr;
        if (AppExecFwk::UnwrapOpenLinkCompletionHandler(env, openLinkOptionsObj, refCompletionHandler,
            onRequestSucc, onRequestFail)) {
            AddCompletionHandlerForOpenLink(env, refCompletionHandler, want, onRequestSucc, onRequestFail);
        }
    }

    want.SetUri(link);
    std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count());
    want.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);

    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "context is nullptr");
        aniObject = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
        return;
    }
    AddFreeInstallObserver(env, want, callbackobj, context, false, true);
    auto ret = context->OpenLink(want, -1, openLinkOptions.GetHideFailureTipDialog());
    if (ret != ERR_OK && freeInstallObserver_ != nullptr) {
        if (ret == AAFwk::ERR_OPEN_LINK_START_ABILITY_DEFAULT_OK) {
            TAG_LOGI(AAFwkTag::SERVICE_EXT, "start ability by default succeeded");
            freeInstallObserver_->OnInstallFinishedByUrl(startTime, link, ERR_OK);
        } else {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "OpenLink failed");
            freeInstallObserver_->OnInstallFinishedByUrl(startTime, link, ret);
            nlohmann::json jsonObject = nlohmann::json {
                { JSON_KEY_ERR_MSG, "Failed to call openLink" },
            };
            std::string requestId = want.GetStringParam(KEY_REQUEST_ID);
            context->OnOpenLinkRequestFailure(requestId, want.GetElement(), jsonObject.dump());
        }
    }
}

void EtsServiceExtensionContext::AddCompletionHandlerForOpenLink(ani_env *env, ani_ref refCompletionHandler,
    AAFwk::Want &want, OnRequestResult &onRequestSucc, OnRequestResult &onRequestFail)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "AddCompletionHandlerForOpenLink called");
    auto context = context_.lock();
    if (!context) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        env->GlobalReference_Delete(refCompletionHandler);
        return;
    }
    uint64_t time = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count());
    std::string requestId = std::to_string(time);
    if (context->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail) != ERR_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "add completionHandler failed");
        env->GlobalReference_Delete(refCompletionHandler);
        return;
    }
    want.SetParam(KEY_REQUEST_ID, requestId);
}

ani_object EtsServiceExtensionContext::OnStartAbilityByCallWithAccount(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_int accountId)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnStartAbilityByCallWithAccount");
    AAFwk::Want want;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return nullptr;
    }
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "parse want failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        return nullptr;
    }

    auto callData = std::make_shared<StartAbilityByCallData>();
    auto callerCallBack = std::make_shared<CallerCallBack>();
    CallUtil::GenerateCallerCallBack(callData, callerCallBack);
    CallUtil::SetOnReleaseOfCallerCallBack(callerCallBack);

    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "context is nullptr");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return nullptr;
    }

    auto ret = context->StartAbilityByCall(want, callerCallBack, accountId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "StartAbilityByCall failed");
        EtsErrorUtil::ThrowErrorByNativeErr(env, ret);
        return nullptr;
    }
    CallUtil::WaitForCalleeObj(callData);

    if (callData->err != 0) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "callComplete err");
        ClearFailedCallConnection(context, callerCallBack);
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return nullptr;
    }

    std::weak_ptr<ServiceExtensionContext> serviceExtensionContext(context);
    auto releaseCallFunc = [serviceExtensionContext] (std::shared_ptr<CallerCallBack> callback) -> ErrCode {
        auto contextForRelease = serviceExtensionContext.lock();
        if (contextForRelease == nullptr) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "null contextForRelease");
            return -1;
        }
        return contextForRelease->ReleaseCall(callback);
    };
    auto caller = EtsCallerComplex::CreateEtsCaller(env, releaseCallFunc, callData->remoteCallee, callerCallBack);
    if (caller == nullptr) {
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }
    return caller;
}

} // namespace AbilityRuntime
} // namespace OHOS
