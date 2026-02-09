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

#include "ets_ability_context.h"

#include <regex>

#include "ani_common_ability_result.h"
#include "ani_common_configuration.h"
#include "ani_common_start_options.h"
#include "ani_common_want.h"
#include "ani_enum_convert.h"
#include "ani_task.h"
#include "remote_object_taihe_ani.h"
#include "app_utils.h"
#include "cJSON.h"
#include "common_fun_ani.h"
#include "ets_caller_complex.h"
#include "ets_context_utils.h"
#include "ets_error_utils.h"
#include "ets_ui_extension_callback.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ipc_skeleton.h"
#include "json_utils.h"
#include "tokenid_kit.h"
#include "want.h"
#ifdef SUPPORT_GRAPHICS
#include "pixel_map_taihe_ani.h"
#endif
#include "ets_ui_service_proxy.h"
#include "ets_uiservice_ability_connection.h"
#include "ets_ui_ability_servicehost_stub_impl.h"
#ifdef SUPPORT_GRAPHICS
#include "pixel_map_taihe_ani.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
std::mutex EtsAbilityContext::requestCodeMutex_;
namespace {
static std::once_flag g_bindNativeMethodsFlag;
std::recursive_mutex g_connectsLock;
int64_t g_serialNumber = 0;
constexpr uint64_t MAX_REQUEST_CODE = (1ULL << 49) - 1;
constexpr size_t MAX_REQUEST_CODE_LENGTH = 15;
constexpr int32_t BASE_REQUEST_CODE_NUM = 10;\
constexpr const int FAILED_CODE = -1;
static std::mutex g_connectsMutex;
static std::map<EtsConnectionKey, sptr<ETSAbilityConnection>, EtsKeyCompare> g_connects;
constexpr const char* UI_ABILITY_CONTEXT_CLASS_NAME = "application.UIAbilityContext.UIAbilityContext";
constexpr const char* CLEANER_CLASS = "application.UIAbilityContext.Cleaner";
const std::string APP_LINKING_ONLY = "appLinkingOnly";
const std::string JSON_KEY_ERR_MSG = "errMsg";
constexpr const char* SIGNATURE_OPEN_LINK = "C{std.core.String}C{utils.AbilityUtils.AsyncCallbackWrapper}"
    "C{@ohos.app.ability.OpenLinkOptions.OpenLinkOptions}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *SIGNATURE_CONNECT_SERVICE_EXTENSION =
    "C{@ohos.app.ability.Want.Want}C{ability.connectOptions.ConnectOptions}:l";
constexpr const char *SIGNATURE_DISCONNECT_SERVICE_EXTENSION = "lC{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *SIGNATURE_OPEN_ATOMIC_SERVICE = "C{std.core.String}C{utils.AbilityUtils.AsyncCallbackWrapper}"
    "C{@ohos.app.ability.AtomicServiceOptions.AtomicServiceOptions}:";
const std::string ATOMIC_SERVICE_PREFIX = "com.atomicservice.";
constexpr const char *SIGNATURE_START_ABILITY_BY_TYPE =
    "C{std.core.String}C{std.core.Record}C{application.AbilityStartCallback.AbilityStartCallback}:C{@ohos.base."
    "BusinessError}";
constexpr const char *SIGNATURE_CONNECT_UI_SERVICE_EXTENSION =
    "C{@ohos.app.ability.Want.Want}C{application.UIServiceExtensionConnectCallback.UIServiceExtensionConnectCallback}"
    "C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *SIGNATURE_START_UI_SERVICE_EXTENSION =
    "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *SIGNATURE_DISCONNECT_UI_SERVICE_EXTENSION =
    "C{application.UIServiceProxy.UIServiceProxy}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *SIGNATURE_WANT_CHK = "C{@ohos.app.ability.Want.Want}:";
constexpr const char *SIGNATURE_DISCONNECT_UI_SERVICE_EXTENSION_CHK = "C{application.UIServiceProxy.UIServiceProxy}:";
constexpr const char* UI_SERVICE_HOSTPROXY_KEY = "ohos.ability.params.UIServiceHostProxy";
constexpr const char *REQUEST_RESULT_INNER_CLASS_NAME =
    "@ohos.app.ability.dialogRequest.dialogRequest.RequestResultInner";
constexpr const char *RESULT_ENUM_NAME = "@ohos.app.ability.dialogRequest.dialogRequest.ResultCode";
constexpr const char *SIGNATURE_REQUEST_DIALOG_SERVICE =
    "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *SIGNATURE_START_ABILITY_WITH_ACCOUNT =
    "C{@ohos.app.ability.Want.Want}iC{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *SIGNATURE_START_ABILITY_WITH_ACCOUNT_OPTIONS =
    "C{@ohos.app.ability.Want.Want}iC{@ohos.app.ability.StartOptions.StartOptions}"
    "C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *SIGNATURE_START_ABILITY_AS_CALLER = "C{@ohos.app.ability.Want.Want}"
    "C{utils.AbilityUtils.AsyncCallbackWrapper}C{@ohos.app.ability.StartOptions.StartOptions}:";
constexpr const char *SIGNATURE_START_RECENT_ABILITY = "C{@ohos.app.ability.Want.Want}"
    "C{utils.AbilityUtils.AsyncCallbackWrapper}C{@ohos.app.ability.StartOptions.StartOptions}:";
constexpr const char *COMPLETION_HANDLER_FAILURE_CODE =
    "@ohos.app.ability.CompletionHandlerForAtomicService.FailureCode";
constexpr int32_t ARGC_ONE = 1;
constexpr int32_t ARGC_TWO = 2;
constexpr int32_t ARGC_THREE = 3;
constexpr const char* SIGNATURE_RESTORE_WINDOW_STAGE = "C{arkui.stateManagement.storage.localStorage.LocalStorage}:";
constexpr const char* KEY_REQUEST_ID = "com.ohos.param.requestId";

int64_t RequestCodeFromStringToInt64(const std::string &requestCode)
{
    if (requestCode.size() > MAX_REQUEST_CODE_LENGTH) {
        TAG_LOGW(AAFwkTag::CONTEXT, "requestCode too long: %{public}s", requestCode.c_str());
        return 0;
    }
    std::regex formatRegex("^[1-9]\\d*|0$");
    std::smatch sm;
    if (!std::regex_match(requestCode, sm, formatRegex)) {
        TAG_LOGW(AAFwkTag::CONTEXT, "requestCode match failed: %{public}s", requestCode.c_str());
        return 0;
    }
    int64_t parsedRequestCode = 0;
    parsedRequestCode = strtoll(requestCode.c_str(), nullptr, BASE_REQUEST_CODE_NUM);
    if (parsedRequestCode < 0 || static_cast<uint64_t>(parsedRequestCode) > MAX_REQUEST_CODE) {
        TAG_LOGW(AAFwkTag::CONTEXT, "requestCode too large: %{public}s", requestCode.c_str());
        return 0;
    }
    return parsedRequestCode;
}

int32_t InsertConnection(sptr<ETSAbilityConnection> connection, const AAFwk::Want &want, int32_t accountId = -1)
{
    std::lock_guard<std::recursive_mutex> lock(g_connectsLock);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null connection");
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
        TAG_LOGD(AAFwkTag::CONTEXT, "remove connection ability exist");
        if (item->second) {
            item->second->RemoveConnectionObject();
        }
        g_connects.erase(item);
    } else {
        TAG_LOGD(AAFwkTag::CONTEXT, "remove connection ability not exist");
    }
}

bool CheckUrl(std::string &urlValue)
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
} // namespace

EtsAbilityContext::~EtsAbilityContext()
{
    if (env_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }
    if (localStorageRef_ != nullptr) {
        env_->GlobalReference_Delete(localStorageRef_);
        localStorageRef_ = nullptr;
    }
}

void EtsAbilityContext::Clean(ani_env *env, ani_object object)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "Clean called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }
    ani_long ptr = 0;
    ani_status status = env->Object_GetFieldByName_Long(object, "ptr", &ptr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "ptr GetField status: %{public}d", status);
        return;
    }
    if (ptr != 0) {
        delete reinterpret_cast<EtsAbilityContext *>(ptr);
    }
}

ani_object EtsAbilityContext::SetEtsAbilityContext(ani_env *env, std::shared_ptr<AbilityContext> context)
{
    if (env == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env or context");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = env->FindClass(UI_ABILITY_CONTEXT_CLASS_NAME, &cls);
    if (status != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "FindClass status: %{public}d, or null cls", status);
        return nullptr;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK ||
        method == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "ctor FindMethod status: %{public}d, or null method", status);
        return nullptr;
    }
    ani_object contextObj = nullptr;
    if ((status = env->Object_New(cls, method, &contextObj)) != ANI_OK || contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Object_New status: %{public}d, or null contextObj", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "setEtsAbilityContextPtr", "l:", &method)) != ANI_OK ||
        method == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "setEtsAbilityContextPtr FindMethod status: %{public}d, or null method", status);
        return nullptr;
    }
    std::unique_ptr<EtsAbilityContext> etsContext = std::make_unique<EtsAbilityContext>(env, context);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return nullptr;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityContext>(etsContext->context_);
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null workContext");
        return nullptr;
    }
    if (!ContextUtil::SetNativeContextLong(env, contextObj, (ani_long)workContext)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "SetNativeContextLong failed");
        delete workContext;
        return nullptr;
    }
    if ((status = env->Object_CallMethod_Void(contextObj, method, (ani_long)etsContext.release())) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "call contextObj method failed, status : %{public}d", status);
        delete workContext;
        return nullptr;
    }
    return contextObj;
}

EtsAbilityContext* EtsAbilityContext::GetEtsAbilityContext(ani_env *env, ani_object aniObj)
{
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env or aniObj");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return nullptr;
    }
    ani_long etsAbilityContextPtr = 0;
    ani_status status = env->Object_GetFieldByName_Long(aniObj, "etsAbilityContextPtr", &etsAbilityContextPtr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "etsAbilityContextPtr GetField status: %{public}d", status);
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return nullptr;
    }
    auto etsContext = reinterpret_cast<EtsAbilityContext *>(etsAbilityContextPtr);
    if (etsContext == nullptr) {
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        TAG_LOGE(AAFwkTag::CONTEXT, "etsAbilityContext null");
    }
    return etsContext;
}

void EtsAbilityContext::ConfigurationUpdated(ani_env *env, ani_object aniObj,
    std::shared_ptr<AppExecFwk::Configuration> config)
{
    if (env == nullptr || aniObj == nullptr || config == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "env or etsContext or config is null");
        return;
    }
    ani_ref configurationRef = AppExecFwk::WrapConfiguration(env, *config);
    ani_status status = env->Object_SetFieldByName_Ref(aniObj, "config", configurationRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "config SetField status: %{public}d", status);
        return;
    }
}

// to be done: free install
void EtsAbilityContext::StartAbility(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object call)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::CONTEXT, "StartAbility called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnStartAbility(env, aniObj, wantObj, nullptr, call);
}

void EtsAbilityContext::StartAbilityWithOptions(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object opt, ani_object call)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::CONTEXT, "StartAbilityWithOptions called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnStartAbility(env, aniObj, wantObj, opt, call);
}

void EtsAbilityContext::StartAbilitySyncCheck(ani_env *env, ani_object aniObj, ani_object opt)
{
    AAFwk::StartOptions startOptions;
    if (!AppExecFwk::UnwrapStartOptionsWithProcessOption(env, opt, startOptions)) {
        EtsErrorUtil::ThrowInvalidParamError(env,
            "Parse param startOptions failed, startOptions must be StartOptions.");
        TAG_LOGE(AAFwkTag::CONTEXT, "invalid options");
        return;
    }
}

// to be done: free install
void EtsAbilityContext::StartAbilityForResult(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "StartAbilityForResult called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnStartAbilityForResult(env, aniObj, wantObj, nullptr, callback);
}

// to be done: free install
void EtsAbilityContext::StartAbilityForResultWithOptions(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object startOptionsObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "StartAbilityForResultWithOptions called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnStartAbilityForResult(env, aniObj, wantObj, startOptionsObj, callback);
}

void EtsAbilityContext::TerminateSelf(ani_env *env, ani_object aniObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "TerminateSelf called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnTerminateSelf(env, aniObj, callback);
}

void EtsAbilityContext::TerminateSelfWithResult(
    ani_env *env, ani_object aniObj, ani_object abilityResult, ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "TerminateSelfWithResult called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnTerminateSelfWithResult(env, aniObj, abilityResult, callback);
}

void EtsAbilityContext::ReportDrawnCompleted(ani_env *env, ani_object aniObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "ReportDrawnCompleted called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnReportDrawnCompleted(env, aniObj, callback);
}

void EtsAbilityContext::StartServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "StartServiceExtensionAbility called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnStartServiceExtensionAbility(env, aniObj, wantObj, callbackobj,
        AppExecFwk::ExtensionAbilityType::SERVICE);
}

void EtsAbilityContext::OpenLink(ani_env *env, ani_object aniObj, ani_string aniLink,
    ani_object myCallbackobj, ani_object optionsObj, ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OpenLink called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_boolean isOptionsUndefined = true;
    if ((status = env->Reference_IsUndefined(optionsObj, &isOptionsUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
    }
    ani_boolean isCallbackUndefined = true;
    if ((status = env->Reference_IsUndefined(callbackobj, &isCallbackUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
    }
    etsContext->OnOpenLink(env, aniObj, aniLink, myCallbackobj, optionsObj, callbackobj, !isOptionsUndefined,
        !isCallbackUndefined);
}

void EtsAbilityContext::OpenLinkCheck(ani_env *env, ani_object aniObj, ani_string aniLink)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OpenLinkCheck called");
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env or aniObj");
        return;
    }
    std::string link("");
    if (!AppExecFwk::GetStdString(env, aniLink, link) || !CheckUrl(link)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "invalid link params");
        EtsErrorUtil::ThrowInvalidParamError(
            env, "Parse param link or openLinkOptions failed, link must be string, openLinkOptions must be options.");
    }
}

bool EtsAbilityContext::IsTerminating(ani_env *env, ani_object aniObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "IsTerminating called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return false;
    }
    return etsContext->OnIsTerminating(env, aniObj);
}

void EtsAbilityContext::MoveAbilityToBackground(ani_env *env, ani_object aniObj, ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "MoveAbilityToBackground called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnMoveAbilityToBackground(env, aniObj, callbackobj);
}

void EtsAbilityContext::RequestModalUIExtension(ani_env *env, ani_object aniObj, ani_object pickerWantObj,
    ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "RequestModalUIExtension called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnRequestModalUIExtension(env, aniObj, pickerWantObj, callbackobj);
}

void EtsAbilityContext::SetMissionWindowIcon(ani_env *env, ani_object aniObj, ani_object pixelMapObj,
    ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "SetMissionWindowIcon called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnSetMissionWindowIcon(env, aniObj, pixelMapObj, callbackobj);
}

void EtsAbilityContext::BackToCallerAbilityWithResult(ani_env *env, ani_object aniObj,
    ani_object abilityResultObj, ani_string requestCodeObj, ani_object callBackObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "BackToCallerAbilityWithResult called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnBackToCallerAbilityWithResult(env, aniObj, abilityResultObj, requestCodeObj, callBackObj);
}

void EtsAbilityContext::SetMissionLabel(ani_env *env, ani_object aniObj, ani_string labelObj,
    ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "SetMissionLabel called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnSetMissionLabel(env, aniObj, labelObj, callbackObj);
}

ani_long EtsAbilityContext::ConnectServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object connectOptionsObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "ConnectServiceExtensionAbility called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return FAILED_CODE;
    }
    return etsContext->OnConnectServiceExtensionAbility(env, aniObj, wantObj, connectOptionsObj,
        AppExecFwk::ExtensionAbilityType::SERVICE);
}

void EtsAbilityContext::DisconnectServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_long connectId,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "DisconnectServiceExtensionAbility called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnDisconnectServiceExtensionAbility(env, aniObj, connectId, callback);
}

void EtsAbilityContext::SetColorMode(ani_env *env, ani_object aniObj, ani_enum_item colorMode)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "SetColorMode called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnSetColorMode(env, aniObj, colorMode);
}

ani_object EtsAbilityContext::StartAbilityByType(
    ani_env *env, ani_object aniObj, ani_string aniType, ani_ref aniWantParam, ani_object startCallback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "StartAbilityByType called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return nullptr;
    }
    return etsContext->OnStartAbilityByType(env, aniObj, aniType, aniWantParam, startCallback);
}

void EtsAbilityContext::OpenAtomicService(
    ani_env *env, ani_object aniObj, ani_string aniAppId, ani_object callbackObj, ani_object optionsObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OpenAtomicService called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnOpenAtomicService(env, aniObj, aniAppId, callbackObj, optionsObj);
}

void EtsAbilityContext::StartSelfUIAbilityInCurrentProcess(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_string aniSpecifiedFlag, ani_object call)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::CONTEXT, "StartSelfUIAbilityInCurrentProcess called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnStartSelfUIAbilityInCurrentProcess(env, aniObj, wantObj, aniSpecifiedFlag, nullptr, call);
}

void EtsAbilityContext::StartSelfUIAbilityInCurrentProcessWithOptions(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_string aniSpecifiedFlag, ani_object opt, ani_object call)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::CONTEXT, "StartSelfUIAbilityInCurrentProcessWithOptions called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnStartSelfUIAbilityInCurrentProcess(env, aniObj, wantObj, aniSpecifiedFlag, opt, call);
}

void EtsAbilityContext::OpenAtomicServiceCheck(ani_env *env, ani_object aniObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OpenAtomicServiceCheck called");
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env or aniObj");
        return;
    }
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    auto context = etsContext->context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
    }
}

ani_long EtsAbilityContext::ConnectServiceExtensionAbilityWithAccount(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_int aniAccountId, ani_object connectOptionsObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "ConnectServiceExtensionAbilityWithAccount called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return FAILED_CODE;
    }
    return etsContext->OnConnectServiceExtensionAbilityWithAccount(env, aniObj, wantObj,
        aniAccountId, connectOptionsObj);
}

void EtsAbilityContext::StopServiceExtensionAbilityWithAccount(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_int aniAccountId, ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "StopServiceExtensionAbilityWithAccount called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnStopServiceExtensionAbilityWithAccount(env, aniObj, wantObj, aniAccountId, callbackObj);
}

void EtsAbilityContext::StopServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "StopServiceExtensionAbility called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnStopServiceExtensionAbility(env, aniObj, wantObj, callbackObj,
        AppExecFwk::ExtensionAbilityType::SERVICE);
}

void EtsAbilityContext::StartServiceExtensionAbilityWithAccount(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_int aniAccountId, ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "StartServiceExtensionAbilityWithAccount called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnStartServiceExtensionAbilityWithAccount(env, aniObj, wantObj, aniAccountId, callbackObj);
}

ani_long EtsAbilityContext::ConnectAppServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object connectOptionsObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "ConnectAppServiceExtensionAbility called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return FAILED_CODE;
    }
    return etsContext->OnConnectServiceExtensionAbility(env, aniObj, wantObj, connectOptionsObj,
        AppExecFwk::ExtensionAbilityType::APP_SERVICE);
}

void EtsAbilityContext::DisconnectAppServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_long connectId,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "DisconnectAppServiceExtensionAbility called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnDisconnectServiceExtensionAbility(env, aniObj, connectId, callback);
}

void EtsAbilityContext::StartAppServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "StartAppServiceExtensionAbility called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnStartServiceExtensionAbility(env, aniObj, wantObj, callbackobj,
        AppExecFwk::ExtensionAbilityType::APP_SERVICE);
}

void EtsAbilityContext::StopAppServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object callbackobj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "StopAppServiceExtensionAbility called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnStopServiceExtensionAbility(env, aniObj, wantObj, callbackobj,
        AppExecFwk::ExtensionAbilityType::APP_SERVICE);
}

void EtsAbilityContext::StartAbilityWithAccount(
    ani_env *env, ani_object aniObj, ani_object aniWant, ani_int aniAccountId, ani_object call)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::CONTEXT, "StartAbilityWithAccount called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnStartAbilityWithAccount(env, aniObj, aniWant, aniAccountId, nullptr, call);
}

void EtsAbilityContext::StartAbilityWithAccountAndOptions(
    ani_env *env, ani_object aniObj, ani_object aniWant, ani_int aniAccountId, ani_object aniOpt, ani_object call)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::CONTEXT, "StartAbilityWithAccountAndOptions called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnStartAbilityWithAccount(env, aniObj, aniWant, aniAccountId, aniOpt, call);
}
#ifdef SUPPORT_SCREEN
void EtsAbilityContext::SetAbilityInstanceInfo(ani_env *env, ani_object aniObj, ani_string labelObj, ani_object iconObj,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "SetAbilityInstanceInfo called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnSetAbilityInstanceInfo(env, aniObj, labelObj, iconObj, callback);
}

void EtsAbilityContext::SetAbilityInstanceInfoCheck(ani_env *env, ani_object aniObj, ani_string labelObj,
    ani_object iconObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "SetAbilityInstanceInfoCheck called");
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env or aniObj");
        return;
    }
    std::string label;
    if (!AppExecFwk::GetStdString(env, labelObj, label) || label.empty()) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to parse label");
        EtsErrorUtil::ThrowInvalidParamError(env, "Invalid label.");
        return;
    }
    auto icon = OHOS::Media::PixelMapTaiheAni::GetNativePixelMap(env, iconObj);
    if (icon == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "parse icon failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse icon failed.");
        return;
    }
}

void EtsAbilityContext::SetMissionIcon(ani_env *env, ani_object aniObj, ani_object pixelMapObj,
    ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "SetMissionIcon called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnSetMissionIcon(env, aniObj, pixelMapObj, callbackObj);
}

void EtsAbilityContext::SetMissionIconCheck(
    ani_env *env, ani_object aniObj, ani_object pixelMapObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "SetMissionIconCheck called");
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env or aniObj");
        return;
    }
    auto icon = OHOS::Media::PixelMapTaiheAni::GetNativePixelMap(env, pixelMapObj);
    if (icon == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "parse icon failed");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return;
    }
}

void EtsAbilityContext::SetMissionContinueState(ani_env *env, ani_object aniObj, ani_object stateObj,
    ani_object callbackObj)
{
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnSetMissionContinueState(env, aniObj, stateObj, callbackObj);
}
#endif

void EtsAbilityContext::RestoreWindowStage(
    ani_env *env, ani_object aniObj, ani_object localStorageObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "RestoreWindowStage called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnRestoreWindowStage(env, aniObj, localStorageObj);
}

void EtsAbilityContext::StartAbilityAsCaller(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object callbackObj, ani_object startOptionsObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "StartAbilityAsCaller called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnStartAbilityAsCaller(env, aniObj, wantObj, callbackObj, startOptionsObj);
}

void EtsAbilityContext::StartRecentAbility(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object callbackObj, ani_object startOptionsObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "StartRecentAbility called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_boolean isOptionsUndefined = ANI_FALSE;
    if ((status = env->Reference_IsUndefined(startOptionsObj, &isOptionsUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        return;
    }
    if (isOptionsUndefined) {
        startOptionsObj = nullptr;
    }
    etsContext->OnStartAbility(env, aniObj, wantObj, startOptionsObj, callbackObj, true);
}

void EtsAbilityContext::SetOnNewWantSkipScenarios(ani_env *env, ani_object aniObj, ani_int etsScenarios,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "SetOnNewWantSkipScenarios called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnSetOnNewWantSkipScenarios(env, aniObj, etsScenarios, callback);
}

int32_t EtsAbilityContext::GenerateRequestCode()
{
    static int32_t curRequestCode_ = 0;
    std::lock_guard lock(requestCodeMutex_);
    curRequestCode_ = (curRequestCode_ == INT_MAX) ? 0 : (curRequestCode_ + 1);
    return curRequestCode_;
}

void EtsAbilityContext::InheritWindowMode(AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "InheritWindowMode");
#ifdef SUPPORT_SCREEN
    // only split mode need inherit
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context null");
        return;
    }
    auto windowMode = context->GetCurrentWindowMode();
    if (AAFwk::AppUtils::GetInstance().IsInheritWindowSplitScreenMode() &&
        (windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
            windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY)) {
        want.SetParam(AAFwk::Want::PARAM_RESV_WINDOW_MODE, windowMode);
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "window mode is %{public}d", windowMode);
#endif
}

void EtsAbilityContext::OnStartAbility(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object opt, ani_object call, bool isStartRecent)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, must be a Want");
        return;
    }
    InheritWindowMode(want);
    if (isStartRecent) {
        TAG_LOGD(AAFwkTag::CONTEXT, "startRecentAbility");
        want.SetParam(AAFwk::Want::PARAM_RESV_START_RECENT, true);
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        EtsErrorUtil::ThrowInvalidParamError(env, "null context");
        return;
    }
    if ((want.GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND) {
        std::string startTime = std::to_string(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
                .count());
        want.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
        AddFreeInstallObserver(env, want, call);
    }
    ErrCode innerErrCode = ERR_OK;
    AAFwk::StartOptions startOptions;
    if (opt != nullptr) {
        if (!AppExecFwk::UnwrapStartOptionsWithProcessOption(env, opt, startOptions)) {
            EtsErrorUtil::ThrowInvalidParamError(env,
                "Parse param startOptions failed, startOptions must be StartOptions.");
            TAG_LOGE(AAFwkTag::CONTEXT, "invalid options");
            return;
        }
        UnwrapCompletionHandlerInStartOptions(env, opt, startOptions);
        innerErrCode = context->StartAbility(want, startOptions, -1);
    } else {
        innerErrCode = context->StartAbility(want, -1);
    }
    ani_object aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode);
    if ((want.GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND) {
        if (innerErrCode != ERR_OK && freeInstallObserver_ != nullptr) {
            std::string bundleName = want.GetElement().GetBundleName();
            std::string abilityName = want.GetElement().GetAbilityName();
            std::string startTime = want.GetStringParam(AAFwk::Want::PARAM_RESV_START_TIME);
            freeInstallObserver_->OnInstallFinished(bundleName, abilityName, startTime, innerErrCode);
        }
        return;
    }
    AppExecFwk::AsyncCallback(env, call, aniObject, nullptr);
    if (innerErrCode != ERR_OK && !startOptions.requestId_.empty()) {
        std::string errMsg = want.GetBoolParam(AAFwk::Want::PARAM_RESV_START_RECENT, false) ?
            "Failed to call startRecentAbility" : "Failed to call startAbility";
        nlohmann::json jsonObject = nlohmann::json { { JSON_KEY_ERR_MSG, errMsg } };
        context->OnRequestFailure(startOptions.requestId_, want.GetElement(), jsonObject.dump());
    }
}

void EtsAbilityContext::OnStartAbilityForResult(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object startOptionsObj, ani_object callback)
{
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetAbilityContext is nullptr");
        EtsErrorUtil::ThrowErrorByNativeErr(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
        return;
    }
    AAFwk::Want want;
    AppExecFwk::UnwrapWant(env, wantObj, want);
    AAFwk::StartOptions startOptions;
    if (startOptionsObj) {
        AppExecFwk::UnwrapStartOptions(env, startOptionsObj, startOptions);
        UnwrapCompletionHandlerInStartOptions(env, startOptionsObj, startOptions);
    }
    TAG_LOGE(AAFwkTag::CONTEXT, "displayId:%{public}d", startOptions.GetDisplayID());
    StartAbilityForResultInner(env, startOptions, want, context, startOptionsObj, callback);
}

ani_object EtsAbilityContext::StartAbilityByCallWithAccount(
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_int aniAccount)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "StartAbilityByCall");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return nullptr;
    }
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
    }
    auto context = etsContext ? etsContext->context_.lock() : nullptr;
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetAbilityContext is nullptr");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return nullptr;
    }

    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "parse want failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        return nullptr;
    }
    auto callData = std::make_shared<StartAbilityByCallData>();
    auto callerCallBack = std::make_shared<CallerCallBack>();
    CallUtil::GenerateCallerCallBack(callData, callerCallBack);
    auto ret = context->StartAbilityByCall(want, callerCallBack, static_cast<int32_t>(aniAccount));
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::CONTEXT, "startAbility failed");
        EtsErrorUtil::ThrowErrorByNativeErr(env, ret);
        return nullptr;
    }
    CallUtil::WaitForCalleeObj(callData);

    if (callData->remoteCallee == nullptr) {
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return nullptr;
    }

    std::weak_ptr<AbilityContext> abilityContext(context);
    auto releaseCallFunc = [abilityContext] (std::shared_ptr<CallerCallBack> callback) -> ErrCode {
        auto contextForRelease = abilityContext.lock();
        if (contextForRelease == nullptr) {
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

void EtsAbilityContext::RestartAppWithWindow(ani_env *env, ani_object aniObj, ani_object wantObj)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "RestartAppWithWindow");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    auto context = etsContext ? etsContext->context_.lock() : nullptr;
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "GetAbilityContext is nullptr");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }

    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "parse want failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        return;
    }
    etsContext->InheritWindowMode(want);
    auto ret = context->RestartAppWithWindow(want);
    if (ret != ERR_OK) {
        EtsErrorUtil::ThrowErrorByNativeErr(env, ret);
    }
}

void EtsAbilityContext::StartAbilityForResultInner(ani_env *env, const AAFwk::StartOptions &startOptions,
    AAFwk::Want &want, std::shared_ptr<AbilityContext> context, ani_object startOptionsObj, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "env is null");
        return;
    }
    std::string startTime = std::to_string(
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count());
    ani_ref callbackRef = nullptr;
    env->GlobalReference_Create(callback, &callbackRef);
    ani_vm *etsVm = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GetVM(&etsVm)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        return;
    }
    if ((want.GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND) {
        want.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
        AddFreeInstallObserver(env, want, callback, true);
    }
    RuntimeTask task = [etsVm, callbackRef, element = want.GetElement(), flags = want.GetFlags(), startTime,
        observer = freeInstallObserver_](int resultCode, const AAFwk::Want &want, bool isInner) {
        TAG_LOGD(AAFwkTag::CONTEXT, "start async callback");
        ani_status status = ANI_ERROR;
        ani_env *env = nullptr;
        if ((status = etsVm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
            return;
        }
        std::string bundleName = element.GetBundleName();
        std::string abilityName = element.GetAbilityName();
        ani_object abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            TAG_LOGW(AAFwkTag::CONTEXT, "null abilityResult");
            isInner = true;
            resultCode = ERR_INVALID_VALUE;
        }
        if ((flags & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND
            && observer != nullptr) {
            isInner ? observer->OnInstallFinished(bundleName, abilityName, startTime, resultCode)
                    : observer->OnInstallFinished(bundleName, abilityName, startTime, abilityResult);
            return;
        }
        auto data = isInner ? nullptr : abilityResult;
        auto errCode = isInner ? resultCode : 0;
        AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef),
            EtsErrorUtil::CreateErrorByNativeErr(env, errCode), data);
    };
    auto requestCode = GenerateRequestCode();
    (startOptionsObj == nullptr) ? context->StartAbilityForResult(want, requestCode, std::move(task))
                                 : context->StartAbilityForResult(want, startOptions, requestCode, std::move(task));
    return;
}

void EtsAbilityContext::OnTerminateSelf(ani_env *env, ani_object aniObj, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env or aniObj");
        ani_object aniObject = EtsErrorUtil::CreateInvalidParamError(env, "env null");
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    ani_object aniObject = nullptr;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "context null");
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    ErrCode ret = context->TerminateSelf();
    if (ret == static_cast<ErrCode>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT) || ret == ERR_OK) {
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ret));
    } else {
        aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    }
    AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
}

void EtsAbilityContext::OnTerminateSelfWithResult(
    ani_env *env, ani_object aniObj, ani_object abilityResult, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        ani_object aniObject = EtsErrorUtil::CreateInvalidParamError(env, "env null");
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    ani_object aniObject = nullptr;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetAbilityContext is nullptr");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "context null");
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    AAFwk::Want want;
    int resultCode = 0;
    AppExecFwk::UnWrapAbilityResult(env, abilityResult, resultCode, want);
    context->SetTerminating(true);
    ErrCode ret = context->TerminateAbilityWithResult(want, resultCode);
    if (ret == static_cast<ErrCode>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT) || ret == ERR_OK) {
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ret));
    } else {
        aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    }
    AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
}

void EtsAbilityContext::OnReportDrawnCompleted(ani_env *env, ani_object aniObj, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        ani_object aniObject = EtsErrorUtil::CreateInvalidParamError(env, "env null");
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    ani_object aniObject = nullptr;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context null");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "context null");
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    ErrCode ret = context->ReportDrawnCompleted();
    if (ret == ERR_OK) {
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ret));
    } else {
        aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    }
    AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
}

void EtsAbilityContext::OnStartServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object callbackobj, AppExecFwk::ExtensionAbilityType extensionType)
{
    ani_object errorObject = nullptr;
    ErrCode ret = ERR_OK;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context is nullptr");
        ret = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        errorObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ret));
        AppExecFwk::AsyncCallback(env, callbackobj, errorObject, nullptr);
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "UnwrapWant filed");
        errorObject = EtsErrorUtil::CreateInvalidParamError(env, "UnwrapWant filed");
        AppExecFwk::AsyncCallback(env, callbackobj, errorObject, nullptr);
    }
    ret = context->StartExtensionAbilityWithExtensionType(want, extensionType);
    if (ret == ERR_OK) {
        errorObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ret));
    } else {
        errorObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    }
    AppExecFwk::AsyncCallback(env, callbackobj, errorObject, nullptr);
}

void EtsAbilityContext::OnStopServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object callbackobj, AppExecFwk::ExtensionAbilityType extensionType)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnStopServiceExtensionAbility");
    ani_object aniObject = nullptr;
    ErrCode ret = ERR_OK;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context is nullptr");
        ret = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ret));
        AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "UnwrapWant failed");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "UnwrapWant failed");
        AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
        return;
    }
    ret = context->StopExtensionAbilityWithExtensionType(want, extensionType);
    aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    AppExecFwk::AsyncCallback(env, callbackobj, aniObject, nullptr);
}

bool EtsAbilityContext::HandleAniLink(ani_env *env, ani_object myCallbackobj, ani_string aniLink,
    std::string &link, AAFwk::Want &want)
{
    ani_object aniObject = nullptr;
    if (!AppExecFwk::GetStdString(env, aniLink, link) || !CheckUrl(link)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "parse link failed");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "Parse param link failed, link must be string.");
        AppExecFwk::AsyncCallback(env, myCallbackobj, aniObject, nullptr);
        return false;
    }
    want.SetUri(link);
    return true;
}

void EtsAbilityContext::HandleOpenLinkOptions(ani_env *env, ani_object optionsObj,
    AAFwk::OpenLinkOptions &openLinkOptions, AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OpenLink Have option");
    want.SetParam(APP_LINKING_ONLY, false);
    AppExecFwk::UnWrapOpenLinkOptions(env, optionsObj, openLinkOptions, want);
    OnRequestResult onRequestSucc;
    OnRequestResult onRequestFail;
    ani_ref refCompletionHandler = nullptr;
    if (AppExecFwk::UnwrapOpenLinkCompletionHandler(env, optionsObj, refCompletionHandler,
        onRequestSucc, onRequestFail)) {
        AddCompletionHandlerForOpenLink(env, refCompletionHandler, want, onRequestSucc, onRequestFail);
    }
}

void EtsAbilityContext::OnOpenLink(ani_env *env, ani_object aniObj, ani_string aniLink, ani_object myCallbackobj,
    ani_object optionsObj, ani_object callbackobj, bool haveOptionsParm, bool haveCallBackParm)
{
    ani_object aniObject = nullptr;
    std::string link("");
    AAFwk::OpenLinkOptions openLinkOptions;
    AAFwk::Want want;
    if (!HandleAniLink(env, myCallbackobj, aniLink, link, want)) {
        return;
    }
    if (haveOptionsParm) {
        HandleOpenLinkOptions(env, optionsObj, openLinkOptions, want);
    }
    std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count());
    want.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
    int requestCode = -1;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetAbilityContext is nullptr");
        ErrCode ret = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ret));
        AppExecFwk::AsyncCallback(env, myCallbackobj, aniObject, nullptr);
        return;
    }
    AddFreeInstallObserver(env, want, myCallbackobj, false, true);
    if (haveCallBackParm) {
        TAG_LOGD(AAFwkTag::CONTEXT, "OpenLink Have Callback");
        CreateOpenLinkTask(env, callbackobj, context, want, requestCode);
    }
    ErrCode ret = context->OpenLink(want, requestCode, openLinkOptions.GetHideFailureTipDialog());
    if (ret == ERR_OK) {
        TAG_LOGI(AAFwkTag::CONTEXT, "openLink succeeded");
        return;
    }
    if (freeInstallObserver_ != nullptr) {
        if (ret == AAFwk::ERR_OPEN_LINK_START_ABILITY_DEFAULT_OK) {
            TAG_LOGI(AAFwkTag::CONTEXT, "start ability by default succeeded");
            freeInstallObserver_->OnInstallFinishedByUrl(startTime, link, ERR_OK);
        }
        freeInstallObserver_->OnInstallFinishedByUrl(startTime, link, ret);
        std::string requestId = want.GetStringParam(KEY_REQUEST_ID);
        nlohmann::json jsonObject = nlohmann::json {
            { JSON_KEY_ERR_MSG, "Failed to call openLink." }
        };
        context->OnOpenLinkRequestFailure(requestId, want.GetElement(), jsonObject.dump());
    }
}

bool EtsAbilityContext::OnIsTerminating(ani_env *env, ani_object aniObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "NativeIsTerminating");
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return false;
    }
    return context->IsTerminating();
}

void EtsAbilityContext::OnMoveAbilityToBackground(ani_env *env, ani_object aniObj, ani_object callback)
{
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)), nullptr);
        return;
    }
    ErrCode ret = ERR_OK;
    ani_object errorObject = nullptr;
    ret = context->MoveUIAbilityToBackground();
    errorObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    AppExecFwk::AsyncCallback(env, callback, errorObject, nullptr);
}

void EtsAbilityContext::OnRequestModalUIExtension(ani_env *env, ani_object aniObj, ani_object pickerWantObj,
    ani_object callbackObj)
{
    ani_object errorObject = nullptr;
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, pickerWantObj, want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "parse want failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        errorObject = EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
        AppExecFwk::AsyncCallback(env, callbackObj, errorObject, nullptr);
        return;
    }

    ErrCode ret = ERR_OK;
    ret = AAFwk::AbilityManagerClient::GetInstance()->RequestModalUIExtension(want);
    errorObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    AppExecFwk::AsyncCallback(env, callbackObj, errorObject, nullptr);
}

void EtsAbilityContext::OnSetMissionWindowIcon(ani_env *env, ani_object aniObj, ani_object pixelMapObj,
    ani_object callbackObj)
{
    ani_object errorObject = nullptr;
#ifdef SUPPORT_SCREEN
    auto pixelMap = Media::PixelMapTaiheAni::GetNativePixelMap(env, pixelMapObj);
    if (pixelMap == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "pixelMap is nullptr");
        errorObject = EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        AppExecFwk::AsyncCallback(env, callbackObj, errorObject, nullptr);
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        errorObject = EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
        AppExecFwk::AsyncCallback(env, callbackObj, errorObject, nullptr);
        return;
    }

    auto ret = context->SetMissionWindowIcon(pixelMap);
    errorObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
#endif
    AppExecFwk::AsyncCallback(env, callbackObj, errorObject, nullptr);
}

void EtsAbilityContext::OnBackToCallerAbilityWithResult(ani_env *env, ani_object aniObj,
    ani_object abilityResultObj, ani_string requestCodeObj, ani_object callBackObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnBackToCallerAbilityWithResult call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }
    ani_object errorObject = nullptr;
    AAFwk::Want want;
    int resultCode = 0;
    if (!OHOS::AppExecFwk::UnWrapAbilityResult(env, abilityResultObj, resultCode, want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "UnWrapAbilityResult failed");
        errorObject = EtsErrorUtil::CreateInvalidParamError(env, "Failed to parse abilityResult.");
        AppExecFwk::AsyncCallback(env, callBackObj, errorObject, nullptr);
        return;
    }
    std::string requestCodeStr;
    if (!AppExecFwk::GetStdString(env, requestCodeObj, requestCodeStr)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to parse label");
        errorObject = EtsErrorUtil::CreateInvalidParamError(env, "Failed to parse label.");
        AppExecFwk::AsyncCallback(env, callBackObj, errorObject, nullptr);
        return;
    }
    auto requestCode = RequestCodeFromStringToInt64(requestCodeStr);
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        AppExecFwk::AsyncCallback(env, callBackObj,
            EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)), nullptr);
        return;
    }

    ErrCode ret = ERR_OK;
    ret = context->BackToCallerAbilityWithResult(want, resultCode, requestCode);
    errorObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    AppExecFwk::AsyncCallback(env, callBackObj, errorObject, nullptr);
}

void EtsAbilityContext::OnSetMissionLabel(ani_env *env, ani_object aniObj, ani_string labelObj,
    ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnSetMissionLabel call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }
    ani_object errorObject = nullptr;
    std::string label;
    if (!AppExecFwk::GetStdString(env, labelObj, label)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to parse label");
        errorObject = EtsErrorUtil::CreateInvalidParamError(env, "Failed to parse label.");
        AppExecFwk::AsyncCallback(env, callbackObj, errorObject, nullptr);
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)), nullptr);
        return;
    }
    auto errCode = context->SetMissionLabel(label);
    if (errCode != ERR_OK) {
        errorObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(errCode));
    }
    AppExecFwk::AsyncCallback(env, callbackObj, errorObject, nullptr);
}

ani_long EtsAbilityContext::OnConnectServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object connectOptionsObj, AppExecFwk::ExtensionAbilityType extensionType)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnConnectServiceExtensionAbility call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return FAILED_CODE;
    }
    AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to UnwrapWant");
        EtsErrorUtil::ThrowInvalidParamError(env, "Failed to UnwrapWant");
        return FAILED_CODE;
    }
    ani_vm *etsVm = nullptr;
    if (env->GetVM(&etsVm) != ANI_OK || etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to getVM");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return FAILED_CODE;
    }
    sptr<ETSAbilityConnection> connection = sptr<ETSAbilityConnection>::MakeSptr(etsVm);
    connection->SetConnectionRef(connectOptionsObj);
    int32_t connectId = InsertConnection(connection, want);
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        RemoveConnection(connectId);
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return FAILED_CODE;
    }
    int32_t innerErrCode = 0;
    innerErrCode = context->ConnectExtensionAbilityWithExtensionType(want, connection, extensionType);
    int32_t errcode = static_cast<int32_t>(GetJsErrorCodeByNativeError(innerErrCode));
    if (errcode) {
        connection->CallEtsFailed(errcode);
        RemoveConnection(connectId);
        return FAILED_CODE;
    }
    return static_cast<ani_long>(connectId);
}

void EtsAbilityContext::OnDisconnectServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_long connectId,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnDisconnectServiceExtensionAbility call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    auto context = context_.lock();
    ani_object errorObject = nullptr;
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        errorObject = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        AppExecFwk::AsyncCallback(env, callback, errorObject, nullptr);
        return;
    }
    sptr<ETSAbilityConnection> connection = nullptr;
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
            TAG_LOGI(AAFwkTag::CONTEXT, "Failed to found connection");
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

void EtsAbilityContext::OnSetColorMode(ani_env *env, ani_object aniObj, ani_enum_item colorMode)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context is already released");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    ani_int mode = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, colorMode, mode)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Parse colorMode failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param colorMode failed, colorMode must be number.");
        return;
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "colorMode is %{public}d", mode);
    context->SetAbilityColorMode(static_cast<int32_t>(mode));
}

ani_object EtsAbilityContext::OnStartAbilityByType(
    ani_env *env, ani_object aniObj, ani_string aniType, ani_ref aniWantParam, ani_object startCallback)
{
    ani_object aniObject = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INNER);
    std::string type;
    if (!AppExecFwk::GetStdString(env, aniType, type)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "parse type failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param type failed, type must be string.");
        return aniObject;
    }

    AAFwk::WantParams wantParam;
    if (!AppExecFwk::UnwrapWantParams(env, aniWantParam, wantParam)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "parse wantParam failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        return aniObject;
    }

    ani_vm *vm = nullptr;
    if (env->GetVM(&vm) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "get vm failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Internal error.");
        return aniObject;
    }
    ErrCode innerErrCode = ERR_OK;
    std::shared_ptr<EtsUIExtensionCallback> callback = std::make_shared<EtsUIExtensionCallback>(vm);
    callback->SetEtsCallbackObject(startCallback);
    ani_ref completionHandler;
    ani_boolean isUndefined = false;
    AppExecFwk::GetPropertyRef(env, startCallback, "completionHandler", completionHandler, isUndefined);
    if (!isUndefined && completionHandler != nullptr) {
        callback->SetCompletionHandler(env, static_cast<ani_object>(completionHandler));
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
    }
#ifdef SUPPORT_SCREEN
    innerErrCode = context->StartAbilityByType(type, wantParam, callback);
#endif
    if (innerErrCode == ERR_OK) {
        return EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK);
    } else if (innerErrCode == static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)) {
        return EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
    } else {
        return EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode);
    }
}

void EtsAbilityContext::CreateOnAtomicRequestSuccessResultCallback(ani_env *env, ani_ref refCompletionHandler,
    OnAtomicRequestSuccess &onRequestCallback, const char *callbackName)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "CreateOnAtomicRequestSuccessResultCallback called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }
    ani_vm *etsVm = nullptr;
    ani_status status = env->GetVM(&etsVm);
    if (status != ANI_OK || etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetVM failed, status: %{public}d", status);
        env->GlobalReference_Delete(refCompletionHandler);
        return;
    }
    onRequestCallback = [etsVm, refCompletionHandler, callbackName](const std::string &appId) {
        ani_env *env = nullptr;
        ani_status status = etsVm->GetEnv(ANI_VERSION_1, &env);
        if (status != ANI_OK || env == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "GetEnv failed, status: %{public}d", status);
            return;
        }
        ani_string appIdStr = nullptr;
        if (env->String_NewUTF8(appId.c_str(), appId.size(), &appIdStr) != ANI_OK || appIdStr == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "String_NewUTF8 for appId failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        ani_ref funcRef;
        if ((status = env->Object_GetFieldByName_Ref(reinterpret_cast<ani_object>(refCompletionHandler),
            callbackName, &funcRef)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::CONTEXT, "Object_GetFieldByName_Ref failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        if (!AppExecFwk::IsValidProperty(env, funcRef)) {
            TAG_LOGE(AAFwkTag::CONTEXT, "IsValidProperty failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        ani_ref result = nullptr;
        std::vector<ani_ref> argv = { appIdStr };
        if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funcRef), ARGC_ONE, argv.data(),
            &result)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::CONTEXT, "FunctionalObject_Call failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        env->GlobalReference_Delete(refCompletionHandler);
    };
}

void EtsAbilityContext::CreateOnAtomicRequestFailureResultCallback(ani_env *env, ani_ref refCompletionHandler,
    OnAtomicRequestFailure &onRequestCallback, const char *callbackName)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "CreateOnAtomicRequestFailureResultCallback called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }
    ani_vm *etsVm = nullptr;
    ani_status status = env->GetVM(&etsVm);
    if (status != ANI_OK || etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetVM failed, status: %{public}d", status);
        env->GlobalReference_Delete(refCompletionHandler);
        return;
    }
    onRequestCallback = [etsVm, refCompletionHandler, callbackName](const std::string &appId,
        int32_t failureCode, const std::string &message) {
        ani_env *env = nullptr;
        ani_status status = etsVm->GetEnv(ANI_VERSION_1, &env);
        if (status != ANI_OK || env == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "GetEnv failed, status: %{public}d", status);
            return;
        }
        ani_string appIdStr = nullptr;
        if (env->String_NewUTF8(appId.c_str(), appId.size(), &appIdStr) != ANI_OK || appIdStr == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "String_NewUTF8 for appId failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        ani_enum_item failureCodeObj {};
        OHOS::AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(
            env, COMPLETION_HANDLER_FAILURE_CODE, failureCode, failureCodeObj);
        ani_string messageStr = nullptr;
        if (env->String_NewUTF8(message.c_str(), message.size(), &messageStr) != ANI_OK || messageStr == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "String_NewUTF8 for messageStr failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        ani_ref funcRef;
        if ((status = env->Object_GetFieldByName_Ref(reinterpret_cast<ani_object>(refCompletionHandler),
            callbackName, &funcRef)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::CONTEXT, "Object_GetFieldByName_Ref failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        if (!AppExecFwk::IsValidProperty(env, funcRef)) {
            TAG_LOGE(AAFwkTag::CONTEXT, "IsValidProperty failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        ani_ref result = nullptr;
        std::vector<ani_ref> argv = { appIdStr, failureCodeObj, messageStr };
        if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funcRef), ARGC_THREE, argv.data(),
            &result)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::CONTEXT, "FunctionalObject_Call failed");
            env->GlobalReference_Delete(refCompletionHandler);
            return;
        }
        env->GlobalReference_Delete(refCompletionHandler);
    };
}

void EtsAbilityContext::UnWrapCompletionHandlerForAtomicService(
    ani_env *env, ani_object param, AAFwk::StartOptions &options, const std::string &appId)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "UnWrapCompletionHandlerForAtomicService called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return;
    }
    ani_ref completionHandler = nullptr;
    if (!AppExecFwk::GetFieldRefByName(env, param, "completionHandlerForAtomicService", completionHandler) ||
        completionHandler == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null completionHandlerForAtomicService");
        return;
    }
    ani_ref refCompletionHandler = nullptr;
    if (env->GlobalReference_Create(completionHandler, &refCompletionHandler) != ANI_OK ||
        refCompletionHandler == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to create global ref for completionHandler.");
        return;
    }
    TAG_LOGI(AAFwkTag::CONTEXT, "completionHandlerForAtomicService exists");
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
        TAG_LOGE(AAFwkTag::CONTEXT, "add completionHandler failed");
        return;
    }
    options.requestId_ = requestId;
}

void EtsAbilityContext::OnOpenAtomicService(
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
        if (!AppExecFwk::UnwrapAtomicServiceOptions(env, optionsObj, want, startOptions)) {
            TAG_LOGE(AAFwkTag::CONTEXT, "UnwrapAtomicServiceOptions failed");
            errorObject = EtsErrorUtil::CreateInvalidParamError(env, "UnwrapAtomicServiceOptions failed.");
            AppExecFwk::AsyncCallback(env, callbackObj, errorObject, nullptr);
            return;
        }
        UnWrapCompletionHandlerForAtomicService(env, optionsObj, startOptions, appId);
    }
    OpenAtomicServiceInner(env, aniObj, want, startOptions, appId, callbackObj);
}

void EtsAbilityContext::OnStartSelfUIAbilityInCurrentProcess(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_string aniSpecifiedFlag, ani_object opt, ani_object call)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to parse want");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, must be a Want");
        return;
    }
    std::string specifiedFlag;
    if (!AppExecFwk::GetStdString(env, aniSpecifiedFlag, specifiedFlag)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to parse specifiedFlag");
        EtsErrorUtil::ThrowInvalidParamError(env, "Failed to parse specifiedFlag.");
        return;
    }
    ErrCode innerErrCode = ERR_OK;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        innerErrCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        ani_object aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode);
        AppExecFwk::AsyncCallback(env, call, aniObject, nullptr);
        return;
    }
    AAFwk::StartOptions startOptions;
    if (opt != nullptr) {
        if (!AppExecFwk::UnwrapStartOptionsWithProcessOption(env, opt, startOptions)) {
            EtsErrorUtil::ThrowInvalidParamError(env,
                "Parse param startOptions failed, startOptions must be StartOptions.");
            TAG_LOGE(AAFwkTag::CONTEXT, "invalid options");
            return;
        }
        innerErrCode = context->StartSelfUIAbilityInCurrentProcess(want, specifiedFlag, startOptions, true);
    } else {
        innerErrCode = context->StartSelfUIAbilityInCurrentProcess(want, specifiedFlag, startOptions, false);
    }
    ani_object aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode);
    AppExecFwk::AsyncCallback(env, call, aniObject, nullptr);
}

ani_long EtsAbilityContext::OnConnectServiceExtensionAbilityWithAccount(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_int aniAccountId, ani_object connectOptionsObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnConnectServiceExtensionAbilityWithAccount call");
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "non system app forbidden to call");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return FAILED_CODE;
    }
    AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to UnwrapWant");
        EtsErrorUtil::ThrowInvalidParamError(env, "Failed to UnwrapWant");
        return FAILED_CODE;
    }
    ani_vm *etsVm = nullptr;
    if (env->GetVM(&etsVm) != ANI_OK || etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to getVM");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return FAILED_CODE;
    }
    sptr<ETSAbilityConnection> connection = sptr<ETSAbilityConnection>::MakeSptr(etsVm);
    connection->SetConnectionRef(connectOptionsObj);
    int32_t connectId = InsertConnection(connection, want, aniAccountId);
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        RemoveConnection(connectId);
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return FAILED_CODE;
    }
    auto innerErrCode = context->ConnectAbilityWithAccount(want, aniAccountId, connection);
    int32_t errcode = static_cast<int32_t>(GetJsErrorCodeByNativeError(innerErrCode));
    if (errcode) {
        connection->CallEtsFailed(errcode);
        RemoveConnection(connectId);
        return FAILED_CODE;
    }
    return static_cast<ani_long>(connectId);
}

void EtsAbilityContext::OnStopServiceExtensionAbilityWithAccount(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_int aniAccountId, ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnStopServiceExtensionAbilityWithAccount call");
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to UnwrapWant");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateInvalidParamError(env, "Failed to UnwrapWant"), nullptr);
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context is nullptr");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }
    auto innerErrCode = context->StopServiceExtensionAbility(want, aniAccountId);
    AppExecFwk::AsyncCallback(env, callbackObj,
        EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(innerErrCode)), nullptr);
}

void EtsAbilityContext::OnStartServiceExtensionAbilityWithAccount(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_int aniAccountId, ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnStartServiceExtensionAbilityWithAccount call");
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to UnwrapWant");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateInvalidParamError(env, "Failed to UnwrapWant"), nullptr);
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context is nullptr");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }
    auto innerErrCode = context->StartServiceExtensionAbility(want, aniAccountId);
    AppExecFwk::AsyncCallback(env, callbackObj,
        EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(innerErrCode)), nullptr);
}

void EtsAbilityContext::OnRevokeDelegator(ani_env *env, ani_object aniObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnRevokeDelegator called");
    ani_object aniObject = nullptr;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context null");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "context null");
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    ErrCode innerErrCode = ERR_OK;
    innerErrCode = context->RevokeDelegator();
    if (innerErrCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "RevokeDelegator failed, innerErrCode: %{public}d", innerErrCode);
    }
    AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode), nullptr);
}

 
void EtsAbilityContext::OnStartAbilityForResultWithAccount(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_int etsAccountId, ani_object startOptionsObj, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "non system app forbidden to call");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "parse want failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        return;
    }
    InheritWindowMode(want);
    int32_t accountId = static_cast<int32_t>(etsAccountId);
    AAFwk::StartOptions startOptions;
    if (startOptionsObj) {
        AppExecFwk::UnwrapStartOptions(env, startOptionsObj, startOptions);
        UnwrapCompletionHandlerInStartOptions(env, startOptionsObj, startOptions);
    }
    OnStartAbilityForResultWithAccountInner(env, startOptions, want, accountId, startOptionsObj, callback);
}

void EtsAbilityContext::OnStartAbilityForResultWithAccountInner(ani_env *env, const AAFwk::StartOptions &startOptions,
    AAFwk::Want &want, const int accountId, ani_object startOptionsObj, ani_object callback)
{
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }
    std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count());
    ani_ref callbackRef = nullptr;
    env->GlobalReference_Create(callback, &callbackRef);
    ani_vm *etsVm = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GetVM(&etsVm)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        return;
    }
    if ((want.GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND) {
        want.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
        AddFreeInstallObserver(env, want, callback, true);
    }
    RuntimeTask task = [etsVm, callbackRef, element = want.GetElement(), flags = want.GetFlags(), startTime,
        observer = freeInstallObserver_](int resultCode, const AAFwk::Want &want, bool isInner) {
        TAG_LOGD(AAFwkTag::CONTEXT, "start async callback");
        ani_status status = ANI_ERROR;
        ani_env *env = nullptr;
        if ((status = etsVm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
            return;
        }
        std::string bundleName = element.GetBundleName();
        std::string abilityName = element.GetAbilityName();
        ani_object abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            TAG_LOGW(AAFwkTag::CONTEXT, "null abilityResult");
            isInner = true;
            resultCode = ERR_INVALID_VALUE;
        }
        if ((flags & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND
            && observer != nullptr) {
            isInner ? observer->OnInstallFinished(bundleName, abilityName, startTime, resultCode)
                    : observer->OnInstallFinished(bundleName, abilityName, startTime, abilityResult);
            return;
        }
        auto data = isInner ? nullptr : abilityResult;
        auto errCode = isInner ? resultCode : 0;
        AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef),
            EtsErrorUtil::CreateErrorByNativeErr(env, errCode), data);
    };
    auto requestCode = GenerateRequestCode();
    (startOptionsObj == nullptr) ? context->StartAbilityForResultWithAccount(want, accountId, requestCode,
        std::move(task)) : context->StartAbilityForResultWithAccount(want, accountId, startOptions, requestCode,
            std::move(task));
}

void EtsAbilityContext::OnRestoreWindowStage(
    ani_env *env, ani_object aniObj, ani_object localStorageObj)
{
    auto context = context_.lock();
    if (!context) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    ani_status status = ANI_ERROR;
    if (localStorageRef_ != nullptr) {
        env->GlobalReference_Delete(localStorageRef_);
        localStorageRef_ = nullptr;
    }
    if ((status = env->GlobalReference_Create(localStorageObj, &localStorageRef_)) != ANI_OK ||
        localStorageRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status : %{public}d", status);
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    auto errCode = context->RestoreWindowStage(reinterpret_cast<void *>(localStorageRef_));
    if (errCode != 0) {
        env->GlobalReference_Delete(localStorageRef_);
        localStorageRef_ = nullptr;
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
}

void EtsAbilityContext::AddFreeInstallObserver(
    ani_env *env, const AAFwk::Want &want, ani_object callback, bool isAbilityResult, bool isOpenLink)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "AddFreeInstallObserver");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }
    auto context = context_.lock();
    if (!context) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return;
    }
    if (freeInstallObserver_ == nullptr) {
        ani_vm *etsVm = nullptr;
        ani_status status = ANI_ERROR;
        if ((status = env->GetVM(&etsVm)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        }
        if (etsVm == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "null etsVm");
            return;
        }
        freeInstallObserver_ = new (std::nothrow) EtsFreeInstallObserver(etsVm);
        if (freeInstallObserver_ == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "null freeInstallObserver");
            return;
        }
        if (context->AddFreeInstallObserver(freeInstallObserver_) != ERR_OK) {
            TAG_LOGE(AAFwkTag::CONTEXT, "addFreeInstallObserver error");
            return;
        }
    }
    std::string startTime = want.GetStringParam(AAFwk::Want::PARAM_RESV_START_TIME);
    if (isOpenLink) {
        std::string url = want.GetUriString();
        freeInstallObserver_->AddEtsObserverObject(env, startTime, url, callback, isAbilityResult);
        return;
    }
    std::string bundleName = want.GetElement().GetBundleName();
    std::string abilityName = want.GetElement().GetAbilityName();
    freeInstallObserver_->AddEtsObserverObject(env, bundleName, abilityName, startTime, callback, isAbilityResult);
}

void EtsAbilityContext::CreateOpenLinkTask(ani_env *env, const ani_object callbackobj,
    std::shared_ptr<AbilityContext> context, AAFwk::Want &want, int &requestCode)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "CreateOpenLinkTask");
    want.SetParam(AAFwk::Want::PARAM_RESV_FOR_RESULT, true);
    ani_vm *etsVm = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GetVM(&etsVm)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        return;
    }
    ani_ref callbackRef = nullptr;
    if ((status = env->GlobalReference_Create(callbackobj, &callbackRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        return;
    }
    RuntimeTask task = [etsVm, callbackRef] (int resultCode, const AAFwk::Want &want, bool isInner) {
    TAG_LOGD(AAFwkTag::CONTEXT, "start async callback");
    ani_status status = ANI_ERROR;
    ani_env *env = nullptr;
    if ((status = etsVm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        return;
    }
    ani_object abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
    if (abilityResult == nullptr) {
        TAG_LOGW(AAFwkTag::CONTEXT, "null abilityResult");
        isInner = true;
        resultCode = ERR_INVALID_VALUE;
    }
    auto errCode = isInner ? resultCode : 0;
    AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef),
        EtsErrorUtil::CreateErrorByNativeErr(env, errCode), abilityResult);
    };
    requestCode = GenerateRequestCode();
    context->InsertResultCallbackTask(requestCode, std::move(task));
}

bool EtsAbilityContext::IsInstanceOf(ani_env *env, ani_object aniObj)
{
    ani_class cls {};
    ani_status status = ANI_ERROR;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return false;
    }
    if ((status = env->FindClass(UI_ABILITY_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        return false;
    }
    ani_boolean isInstanceOf = false;
    if ((status = env->Object_InstanceOf(aniObj, cls, &isInstanceOf)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        return false;
    }
    return isInstanceOf;
}

void EtsAbilityContext::NativeOnSetRestoreEnabled(ani_env *env, ani_object aniObj, ani_boolean aniEnabled)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "NativeOnSetRestoreEnabled");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }

    auto abilityContext = GetEtsAbilityContext(env, aniObj);
    if (abilityContext == nullptr) {
        TAG_LOGW(AAFwkTag::CONTEXT, "null abilityContext");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }

    bool enabled = static_cast<bool>(aniEnabled);
    auto context = abilityContext->context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    context->SetRestoreEnabled(enabled);
}

void EtsAbilityContext::NativeChangeAbilityVisibility(ani_env *env, ani_object aniObj,
    ani_boolean isShow, ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "ChangeAbilityVisibility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }
    auto etsContext = EtsAbilityContext::GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    auto context = etsContext->context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }
    bool showFlag = static_cast<bool>(isShow);
    ErrCode errCode = context->ChangeAbilityVisibility(showFlag);
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "failed, errCode: %{public}d", errCode);
        AppExecFwk::AsyncCallback(env, callbackObj, EtsErrorUtil::CreateErrorByNativeErr(env, errCode), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callbackObj, EtsErrorUtil::CreateErrorByNativeErr(env, ERR_OK), nullptr);
}

void EtsAbilityContext::OpenAtomicServiceInner(ani_env *env, ani_object aniObj, AAFwk::Want &want,
    AAFwk::StartOptions &options, std::string appId, ani_object callbackObj)
{
    std::string bundleName = ATOMIC_SERVICE_PREFIX + appId;
    TAG_LOGD(AAFwkTag::CONTEXT, "bundleName: %{public}s", bundleName.c_str());
    want.SetBundle(bundleName);
    want.AddFlags(AAFwk::Want::FLAG_INSTALL_ON_DEMAND);
    std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count());
    want.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    AddFreeInstallObserver(env, want, callbackObj, true);
    ani_vm *etsVm = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GetVM(&etsVm)) != ANI_OK || etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        return;
    }
    RuntimeTask task = [etsVm, element = want.GetElement(), startTime, observer = freeInstallObserver_](
        int resultCode, const AAFwk::Want &want, bool isInner) {
        ani_env *env = nullptr;
        if (etsVm->GetEnv(ANI_VERSION_1, &env) != ANI_OK || env == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "null env");
            return;
        }
        if (observer == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "null observer");
            return;
        }
        std::string bundleName = element.GetBundleName();
        std::string abilityName = element.GetAbilityName();
        ani_object abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            TAG_LOGW(AAFwkTag::CONTEXT, "null abilityResult");
            isInner = true;
            resultCode = ERR_INVALID_VALUE;
        }
        isInner ? observer->OnInstallFinished(bundleName, abilityName, startTime, resultCode)
                : observer->OnInstallFinished(bundleName, abilityName, startTime, abilityResult);
    };
    want.SetParam(AAFwk::Want::PARAM_RESV_FOR_RESULT, true);
    auto requestCode = GenerateRequestCode();
    ErrCode errCode = context->OpenAtomicService(want, options, requestCode, std::move(task));
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

void EtsAbilityContext::WantCheck(ani_env *env, ani_object aniObj, ani_object wantObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "ConnectUIServiceExtensionCheck called");
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env or aniObj");
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "UnwrapWant failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, want must be Want");
        return;
    }
}

void EtsAbilityContext::ConnectUIServiceExtension(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object uiServiceExtConCallbackObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "ConnectUIServiceExtension called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnConnectUIServiceExtension(env, wantObj, uiServiceExtConCallbackObj, callback);
}

void EtsAbilityContext::RequestDialogService(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object call)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "RequestDialogService called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnRequestDialogService(env, aniObj, wantObj, call);
}

ani_object EtsAbilityContext::WrapRequestDialogResult(ani_env *env, int32_t resultCode, const AAFwk::Want &want)
{
    ani_class requestResultInner = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return nullptr;
    }
    ani_status status = env->FindClass(REQUEST_RESULT_INNER_CLASS_NAME, &requestResultInner);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "RequestResultInner FindClass status: %{public}d", status);
        return nullptr;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(requestResultInner, "<ctor>", ":", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "RequestResultInner Class_FindMethod status: %{public}d", status);
        return nullptr;
    }
    ani_object object = nullptr;
    if ((status = env->Object_New(requestResultInner, method, &object)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "RequestResultInner Object_New status: %{public}d", status);
        return nullptr;
    }
    ani_enum_item resultItem = nullptr;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(env, RESULT_ENUM_NAME, resultCode, resultItem) ||
        resultItem == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "resultItem failed, or null resultItem");
        return nullptr;
    }
    status = env->Object_SetPropertyByName_Ref(object, "result", resultItem);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Object_SetPropertyByName_Ref status: %{public}d", status);
        return nullptr;
    }
    status = env->Object_SetPropertyByName_Ref(object, "want", AppExecFwk::WrapWant(env, want));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Object_SetPropertyByName_Ref status: %{public}d", status);
        return nullptr;
    }
    return object;
}

void EtsAbilityContext::OnRequestDialogService(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object call)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }
    AAFwk::Want want;
    AppExecFwk::UnwrapWant(env, wantObj, want);
    TAG_LOGD(AAFwkTag::CONTEXT, "target:%{public}s.%{public}s", want.GetBundle().c_str(),
        want.GetElement().GetAbilityName().c_str());
    ani_vm *vm = nullptr;
    ani_status status = env->GetVM(&vm);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetVM status: %{public}d", status);
        return;
    }
    ani_ref callbackRef = nullptr;
    env->GlobalReference_Create(call, &callbackRef);
    RequestDialogResultTask task =
        [vm, callbackRef](int32_t resultCode, const AAFwk::Want &resultWant) {
        bool isAttachThread = false;
        ani_env *env = AppExecFwk::AttachAniEnv(vm, isAttachThread);
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "null env");
            return;
        }
        ani_object requestResult = EtsAbilityContext::WrapRequestDialogResult(env, resultCode, resultWant);
        if (requestResult == nullptr) {
            TAG_LOGW(AAFwkTag::CONTEXT, "null requestResult");
            EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        } else {
            ani_object errorObj = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK);
            AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef), errorObj, requestResult);
        }
        env->GlobalReference_Delete(callbackRef);
        AppExecFwk::DetachAniEnv(vm, isAttachThread);
        TAG_LOGD(AAFwkTag::CONTEXT, "end async callback");
    };

    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "context null");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
    } else {
        auto errCode = context->RequestDialogService(want, std::move(task));
        if (errCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::UIABILITY, "errCode: %{public}d", errCode);
            EtsErrorUtil::ThrowError(env, EtsErrorUtil::CreateError(env, GetJsErrorCodeByNativeError(errCode)));
        }
    }
}

void EtsAbilityContext::OnConnectUIServiceExtension(ani_env *env, ani_object wantObj,
    ani_object uiServiceExtConCallbackObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnConnectUIServiceExtension called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GetVM failed");
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "input param want failed");
        return;
    }
    if (CheckConnectAlreadyExist(env, want, uiServiceExtConCallbackObj, callback)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "duplicated");
        return;
    }
    sptr<EtsUIServiceExtAbilityConnection> connection = sptr<EtsUIServiceExtAbilityConnection>::MakeSptr(aniVM);
    sptr<EtsUIAbilityServiceHostStubImpl> stub = connection->GetServiceHostStub();
    want.SetParam(UI_SERVICE_HOSTPROXY_KEY, stub->AsObject());
    connection->SetConnectionRef(uiServiceExtConCallbackObj);
    connection->SetAniAsyncCallback_(callback);
    EtsUIServiceConnection::InsertUIServiceAbilityConnection(connection, want);
    int64_t connectId = connection->GetConnectionId();
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callback),
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(
                env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)),
                    AAFwk::EtsUIServiceProxy::CreateEmptyProxyObject(env));
        EtsUIServiceConnection::RemoveUIServiceAbilityConnection(connectId);
        return;
    }
    int32_t innerErrorCode = context->ConnectUIServiceExtensionAbility(want, connection);
    TAG_LOGD(AAFwkTag::CONTEXT, "ConnectUIServiceExtensionAbility errcode: %{public}d.", innerErrorCode);
    if (innerErrorCode != static_cast<int32_t>(AbilityErrorCode::ERROR_OK)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "errcode: %{public}d.", innerErrorCode);
        AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callback),
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(
                env, static_cast<int32_t>(innerErrorCode)), AAFwk::EtsUIServiceProxy::CreateEmptyProxyObject(env));
        EtsUIServiceConnection::RemoveUIServiceAbilityConnection(connectId);
    }
}

void EtsAbilityContext::StartUIServiceExtension(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "StartUIServiceExtension called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnStartUIServiceExtension(env, wantObj, callback);
}

void EtsAbilityContext::OnStartUIServiceExtension(ani_env *env, ani_object wantObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnStartUIServiceExtension called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "UnwrapWant failed");
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
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

void EtsAbilityContext::DisconnectUIServiceExtensionCheck(ani_env *env, ani_object aniObj, ani_object proxyObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "DisconnectUIServiceExtensionCheck called");
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env or aniObj");
        return;
    }
    AAFwk::EtsUIServiceProxy* proxy = AAFwk::EtsUIServiceProxy::GetEtsUIServiceProxy(env, proxyObj);
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null proxy");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter verification failed");
        return;
    }
}

void EtsAbilityContext::DisconnectUIServiceExtension(ani_env *env, ani_object aniObj, ani_object proxyObj,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "DisconnectUIServiceExtension called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnDisconnectUIServiceExtension(env, proxyObj, callback);
}

void EtsAbilityContext::OnDisconnectUIServiceExtension(ani_env *env, ani_object proxyObj, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }
    AAFwk::EtsUIServiceProxy* proxy = AAFwk::EtsUIServiceProxy::GetEtsUIServiceProxy(env, proxyObj);
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null proxy");
        return;
    }
    int64_t connectId = proxy->GetConnectionId();
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }
    AAFwk::Want want;
    sptr<EtsUIServiceExtAbilityConnection> connection = nullptr;
    EtsUIServiceConnection::FindUIServiceAbilityConnection(connectId, want, connection);
    TAG_LOGD(AAFwkTag::CONTEXT, "connection:%{public}d.", static_cast<int32_t>(connectId));
    if (connection == nullptr) {
        TAG_LOGW(AAFwkTag::CONTEXT, "null connection");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INNER), nullptr);
        EtsUIServiceConnection::RemoveUIServiceAbilityConnection(connectId);
        return;
    }
    context->DisconnectAbility(want, connection);
    AppExecFwk::AsyncCallback(env, callback,
        EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
}

bool EtsAbilityContext::CheckConnectAlreadyExist(ani_env *env, const AAFwk::Want& want,
    ani_object callback, ani_object myCallback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "CheckConnectAlreadyExist called");
    sptr<EtsUIServiceExtAbilityConnection> connection = nullptr;
    EtsUIServiceConnection::FindUIServiceAbilityConnection(env, want, callback, connection);
    if (connection == nullptr) {
        TAG_LOGD(AAFwkTag::CONTEXT, "null connection");
        return false;
    }
    ani_ref proxy = connection->GetProxyObject();
    if (proxy == nullptr) {
        TAG_LOGW(AAFwkTag::CONTEXT, "null proxy");
        connection->AddDuplicatedPendingCallback(myCallback);
    } else {
        TAG_LOGI(AAFwkTag::CONTEXT, "Resolve, got proxy object");
        AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(myCallback),
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(
                env, static_cast<int32_t>(AbilityErrorCode::ERROR_OK)), reinterpret_cast<ani_object>(proxy));
    }
    return true;
}

#ifdef SUPPORT_SCREEN
void EtsAbilityContext::OnSetAbilityInstanceInfo(ani_env *env, ani_object aniObj, ani_string labelObj,
    ani_object iconObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnSetAbilityInstanceInfo called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }

    std::string label;
    if (!AppExecFwk::GetStdString(env, labelObj, label) || label.empty()) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to parse label");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param label failed");
        return;
    }

    auto icon = OHOS::Media::PixelMapTaiheAni::GetNativePixelMap(env, iconObj);

    ani_object errorObj = nullptr;
    ani_vm *etsVm = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GetVM(&etsVm)) != ANI_OK || etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        errorObj = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INNER);
        AppExecFwk::AsyncCallback(env, callback, errorObj, nullptr);
        return;
    }

    ani_ref callbackRef = nullptr;
    if ((status = env->GlobalReference_Create(callback, &callbackRef)) != ANI_OK || callbackRef == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        errorObj = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INNER);
        AppExecFwk::AsyncCallback(env, callback, errorObj, nullptr);
        return;
    }

    OnSetAbilityInstanceInfoInner(env, label, icon, callback, etsVm, callbackRef);
}

void EtsAbilityContext::OnSetAbilityInstanceInfoInner(ani_env *env, std::string& label,
    std::shared_ptr<OHOS::Media::PixelMap> icon, ani_object callback, ani_vm *etsVm, ani_ref callbackRef)
{
    if (env == nullptr || etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env or etsVm");
        return;
    }
    if (icon == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to unwrap PixelMap");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param icon failed");
        env->GlobalReference_Delete(callbackRef);
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        EtsErrorUtil::ThrowErrorByNativeErr(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
        env->GlobalReference_Delete(callbackRef);
        return;
    }
    ani_object errorObj = nullptr;
    auto task = [context, label, icon, etsVm, callbackRef]() {
        ani_status status = ANI_ERROR;
        ani_env *env = nullptr;
        if ((status = etsVm->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
            return;
        }
        ani_object errorObj = nullptr;
        ErrCode ret = context->SetAbilityInstanceInfo(label, icon);
        if (ret == ERR_OK) {
            errorObj = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK);
            AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef), errorObj, nullptr);
            env->GlobalReference_Delete(callbackRef);
        } else {
            errorObj = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
            AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef), errorObj, nullptr);
            env->GlobalReference_Delete(callbackRef);
        }
    };
    if (AniTask::AniSendEvent(task) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to sendEvent");
        errorObj = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INNER);
        AppExecFwk::AsyncCallback(env, callback, errorObj, nullptr);
        env->GlobalReference_Delete(callbackRef);
    }
}

void EtsAbilityContext::OnSetMissionIcon(ani_env *env, ani_object aniObj, ani_object pixelMapObj,
    ani_object callbackObj)
{
    auto pixelMap = OHOS::Media::PixelMapTaiheAni::GetNativePixelMap(env, pixelMapObj);
    if (pixelMap == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Unwrap pixelMap failed");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM), nullptr);
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context is nullptr");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }
    auto innerErrCode = context->SetMissionIcon(pixelMap);
    AppExecFwk::AsyncCallback(env, callbackObj,
        EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(innerErrCode)), nullptr);
}

void EtsAbilityContext::OnSetMissionContinueState(ani_env *env, ani_object aniObj, ani_object stateObj,
    ani_object callbackObj)
{
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context is nullptr");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }
    int state = 0;
    bool result = AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, stateObj, state);
    if (result == false) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Unwrap state failed");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM), nullptr);
        return;
    }
    AAFwk::ContinueState fwkState = AAFwk::ContinueState::CONTINUESTATE_UNKNOWN;
    if (state == AAFwk::ContinueState::CONTINUESTATE_ACTIVE) {
        fwkState = AAFwk::ContinueState::CONTINUESTATE_ACTIVE;
    } else {
        fwkState = AAFwk::ContinueState::CONTINUESTATE_INACTIVE;
    }
    auto innerErrCode = context->SetMissionContinueState(fwkState);
    AppExecFwk::AsyncCallback(env, callbackObj,
        EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(innerErrCode)), nullptr);
}
#endif

void EtsAbilityContext::OnStartAbilityAsCaller(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object callbackObj, ani_object startOptionsObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "StartAbilityAsCaller called");
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, must be a Want");
        return;
    }
    InheritWindowMode(want);
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        EtsErrorUtil::ThrowInvalidParamError(env, "null context");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_boolean isOptionsUndefined = true;
    if ((status = env->Reference_IsUndefined(startOptionsObj, &isOptionsUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        return;
    }
    ErrCode innerErrCode = ERR_OK;
    AAFwk::StartOptions startOptions;
    if (!isOptionsUndefined) {
        if (!AppExecFwk::UnwrapStartOptionsWithProcessOption(env, startOptionsObj, startOptions)) {
            EtsErrorUtil::ThrowInvalidParamError(env,
                "Parse param startOptions failed, startOptions must be StartOptions.");
            TAG_LOGE(AAFwkTag::CONTEXT, "invalid options");
            return;
        }
        UnwrapCompletionHandlerInStartOptions(env, startOptionsObj, startOptions);
        innerErrCode = context->StartAbilityAsCaller(want, startOptions, -1);
    } else {
        innerErrCode = context->StartAbilityAsCaller(want, -1);
    }
    ani_object aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode);
    AppExecFwk::AsyncCallback(env, callbackObj, aniObject, nullptr);
    if (innerErrCode != ERR_OK && !startOptions.requestId_.empty()) {
        nlohmann::json jsonObject = nlohmann::json {
            { JSON_KEY_ERR_MSG, "Failed to call startAbilityAsCaller" }
        };
        context->OnRequestFailure(startOptions.requestId_, want.GetElement(), jsonObject.dump());
    }
}

void EtsAbilityContext::RevokeDelegator(ani_env *env, ani_object aniObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "RevokeDelegator called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnRevokeDelegator(env, aniObj, callback);
}

void EtsAbilityContext::StartAbilityForResultWithAccount(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_int etsAccountId, ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "StartAbilityForResultWithAccount called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnStartAbilityForResultWithAccount(env, aniObj, wantObj, etsAccountId, nullptr, callback);
}

void EtsAbilityContext::StartAbilityForResultWithAccountCheck(ani_env *env, ani_object aniObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "StartAbilityForResultWithAccountCheck called");
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env or aniObj");
        return;
    }
    if (!AppExecFwk::CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::CONTEXT, "non system app forbidden to call");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
}

void EtsAbilityContext::StartAbilityForResultWithAccountVoid(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_int etsAccountId, ani_object startOptionsObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "StartAbilityForResultWithAccountVoid called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnStartAbilityForResultWithAccount(env, aniObj, wantObj, etsAccountId, startOptionsObj, callback);
}

void EtsAbilityContext::StartAbilityForResultWithAccountResult(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_int etsAccountId, ani_object startOptionsObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "StartAbilityForResultWithAccountResult called");
    auto etsContext = GetEtsAbilityContext(env, aniObj);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsContext");
        return;
    }
    etsContext->OnStartAbilityForResultWithAccount(env, aniObj, wantObj, etsAccountId, startOptionsObj, callback);
}

void EtsAbilityContext::OnStartAbilityWithAccount(
    ani_env *env, ani_object aniObj, ani_object aniWant, ani_int aniAccountId, ani_object aniOpt, ani_object call)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AAFwk::Want want;
    AppExecFwk::UnwrapWant(env, aniWant, want);
    InheritWindowMode(want);
    TAG_LOGI(AAFwkTag::CONTEXT, "ability:%{public}s", want.GetElement().GetAbilityName().c_str());
    ErrCode innerErrCode = ERR_OK;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context null");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    if ((want.GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND) {
        std::string startTime = std::to_string(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
                .count());
        want.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
        AddFreeInstallObserver(env, want, call);
    }
    AAFwk::StartOptions startOptions;
    if (aniOpt != nullptr) {
        TAG_LOGD(AAFwkTag::CONTEXT, "start options is used");
        AppExecFwk::UnwrapStartOptions(env, aniOpt, startOptions);
        UnwrapCompletionHandlerInStartOptions(env, aniOpt, startOptions);
        innerErrCode = context->StartAbilityWithAccount(want, aniAccountId, startOptions, -1);
    } else {
        innerErrCode = context->StartAbilityWithAccount(want, aniAccountId, -1);
    }
    ani_object aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, innerErrCode);
    if ((want.GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND) {
        if (innerErrCode != ERR_OK && freeInstallObserver_ != nullptr) {
            std::string bundleName = want.GetElement().GetBundleName();
            std::string abilityName = want.GetElement().GetAbilityName();
            std::string startTime = want.GetStringParam(AAFwk::Want::PARAM_RESV_START_TIME);
            freeInstallObserver_->OnInstallFinished(bundleName, abilityName, startTime, innerErrCode);
        }
        return;
    }
    AppExecFwk::AsyncCallback(env, call, aniObject, nullptr);
    if (innerErrCode != ERR_OK && !startOptions.requestId_.empty()) {
        nlohmann::json jsonObject = nlohmann::json {
            { JSON_KEY_ERR_MSG, "Failed to call startAbilityWithAccount" }
        };
        context->OnRequestFailure(startOptions.requestId_, want.GetElement(), jsonObject.dump());
    }
}

void EtsAbilityContext::OnSetOnNewWantSkipScenarios(ani_env *env, ani_object aniObj, ani_int etsScenarios,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnSetOnNewWantSkipScenarios called");
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context null");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }
    int32_t scenarios = static_cast<int32_t>(etsScenarios);
    ErrCode innerErrCode = ERR_OK;
    innerErrCode = context->SetOnNewWantSkipScenarios(scenarios);
    if (innerErrCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "SetOnNewWantSkipScenarios failed, innerErrCode: %{public}d", innerErrCode);
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INNER),
            nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, ERR_OK), nullptr);
}

namespace {
bool BindNativeMethods(ani_env *env, ani_class &cls)
{
    ani_status status = env->FindClass(UI_ABILITY_CONTEXT_CLASS_NAME, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "FindClass status: %{public}d", status);
        return false;
    }
    std::call_once(g_bindNativeMethodsFlag, [&status, env, cls]() {
        std::array functions = {
            ani_native_function { "nativeStartAbilitySync",
                "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsAbilityContext::StartAbility) },
            ani_native_function { "nativeStartAbilitySync",
                "C{@ohos.app.ability.Want.Want}C{@ohos.app.ability.StartOptions.StartOptions}C{utils.AbilityUtils."
                "AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsAbilityContext::StartAbilityWithOptions) },
            ani_native_function { "nativeStartAbilitySyncCheck",
                "C{@ohos.app.ability.StartOptions.StartOptions}:",
                reinterpret_cast<void *>(EtsAbilityContext::StartAbilitySyncCheck) },
            ani_native_function { "nativeStartAbilityForResult",
                "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsAbilityContext::StartAbilityForResult) },
            ani_native_function { "nativeStartAbilityForResult",
                "C{@ohos.app.ability.Want.Want}C{@ohos.app.ability.StartOptions.StartOptions}C{utils.AbilityUtils."
                "AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsAbilityContext::StartAbilityForResultWithOptions) },
            ani_native_function { "nativeTerminateSelfSync", "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsAbilityContext::TerminateSelf) },
            ani_native_function { "nativeTerminateSelfWithResult",
                "C{ability.abilityResult.AbilityResult}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsAbilityContext::TerminateSelfWithResult) },
            ani_native_function { "nativeStartAbilityByCallWithAccountSync",
                "C{@ohos.app.ability.Want.Want}i:C{@ohos.app.ability.UIAbility.Caller}",
                reinterpret_cast<void*>(EtsAbilityContext::StartAbilityByCallWithAccount) },
            ani_native_function { "nativeReportDrawnCompletedSync", "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<ani_int *>(EtsAbilityContext::ReportDrawnCompleted) },
            ani_native_function { "nativeStartServiceExtensionAbility",
                "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void*>(EtsAbilityContext::StartServiceExtensionAbility) },
            ani_native_function { "nativeOpenLink", SIGNATURE_OPEN_LINK,
                reinterpret_cast<void*>(EtsAbilityContext::OpenLink) },
            ani_native_function { "nativeOpenLinkCheck", "C{std.core.String}:",
                reinterpret_cast<void *>(EtsAbilityContext::OpenLinkCheck) },
            ani_native_function { "nativeIsTerminating", ":z",
                reinterpret_cast<void*>(EtsAbilityContext::IsTerminating) },
            ani_native_function { "nativeMoveAbilityToBackground", "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void*>(EtsAbilityContext::MoveAbilityToBackground) },
            ani_native_function { "nativeRequestModalUIExtension",
                "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void*>(EtsAbilityContext::RequestModalUIExtension) },
            ani_native_function { "nativeBackToCallerAbilityWithResult",
                "C{ability.abilityResult.AbilityResult}C{std.core.String}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void*>(EtsAbilityContext::BackToCallerAbilityWithResult) },
            ani_native_function { "nativeSetMissionLabel",
                "C{std.core.String}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsAbilityContext::SetMissionLabel) },
            ani_native_function { "nativeConnectServiceExtensionAbility", SIGNATURE_CONNECT_SERVICE_EXTENSION,
                reinterpret_cast<void *>(EtsAbilityContext::ConnectServiceExtensionAbility) },
            ani_native_function { "nativeDisconnectServiceExtensionAbility", SIGNATURE_DISCONNECT_SERVICE_EXTENSION,
                reinterpret_cast<void *>(EtsAbilityContext::DisconnectServiceExtensionAbility) },
            ani_native_function {"nativeSetColorMode",
                "C{@ohos.app.ability.ConfigurationConstant.ConfigurationConstant.ColorMode}:",
                reinterpret_cast<void*>(EtsAbilityContext::SetColorMode)},
            ani_native_function { "nativeStartAbilityByTypeSync", SIGNATURE_START_ABILITY_BY_TYPE,
                reinterpret_cast<void *>(EtsAbilityContext::StartAbilityByType) },
            ani_native_function { "nativeOpenAtomicService", SIGNATURE_OPEN_ATOMIC_SERVICE,
                reinterpret_cast<void *>(EtsAbilityContext::OpenAtomicService) },
            ani_native_function { "nativeOpenAtomicServiceCheck", ":",
                reinterpret_cast<void *>(EtsAbilityContext::OpenAtomicServiceCheck) },
            ani_native_function{"nativeConnectUIServiceExtensionAbility", SIGNATURE_CONNECT_UI_SERVICE_EXTENSION,
                reinterpret_cast<void*>(EtsAbilityContext::ConnectUIServiceExtension)},
            ani_native_function{"nativeStartUIServiceExtensionAbility", SIGNATURE_START_UI_SERVICE_EXTENSION,
                reinterpret_cast<void*>(EtsAbilityContext::StartUIServiceExtension)},
            ani_native_function{"nativeDisconnectUIServiceExtensionAbility", SIGNATURE_DISCONNECT_UI_SERVICE_EXTENSION,
                reinterpret_cast<void*>(EtsAbilityContext::DisconnectUIServiceExtension)},
            ani_native_function{"nativeWantCheck", SIGNATURE_WANT_CHK,
                reinterpret_cast<void*>(EtsAbilityContext::WantCheck)},
            ani_native_function{"nativeDisconnectUIServiceExtensionCheck",
                SIGNATURE_DISCONNECT_UI_SERVICE_EXTENSION_CHK,
                reinterpret_cast<void*>(EtsAbilityContext::DisconnectUIServiceExtensionCheck)},
            ani_native_function { "nativeRequestDialogService", SIGNATURE_REQUEST_DIALOG_SERVICE,
                reinterpret_cast<void*>(EtsAbilityContext::RequestDialogService) },
            ani_native_function { "nativeStartSelfUIAbilityInCurrentProcessSync",
                "C{@ohos.app.ability.Want.Want}C{std.core.String}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsAbilityContext::StartSelfUIAbilityInCurrentProcess) },
            ani_native_function { "nativeStartSelfUIAbilityInCurrentProcessSync",
                "C{@ohos.app.ability.Want.Want}C{std.core.String}C{@ohos.app.ability.StartOptions.StartOptions}C{utils."
                "AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsAbilityContext::StartSelfUIAbilityInCurrentProcessWithOptions) },
            ani_native_function { "nativeOnSetRestoreEnabled", "z:",
                reinterpret_cast<void*>(EtsAbilityContext::NativeOnSetRestoreEnabled) },
            ani_native_function { "nativeConnectServiceExtensionAbilityWithAccount",
                "C{@ohos.app.ability.Want.Want}iC{ability.connectOptions.ConnectOptions}:l",
                reinterpret_cast<void*>(EtsAbilityContext::ConnectServiceExtensionAbilityWithAccount) },
            ani_native_function { "nativeStopServiceExtensionAbilityWithAccount",
                "C{@ohos.app.ability.Want.Want}iC{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void*>(EtsAbilityContext::StopServiceExtensionAbilityWithAccount) },
            ani_native_function { "nativeStopServiceExtensionAbility",
                "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void*>(EtsAbilityContext::StopServiceExtensionAbility) },
            ani_native_function { "nativeStartServiceExtensionAbilityWithAccount",
                "C{@ohos.app.ability.Want.Want}iC{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void*>(EtsAbilityContext::StartServiceExtensionAbilityWithAccount) },
            ani_native_function { "nativeChangeAbilityVisibility", "zC{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void*>(EtsAbilityContext::NativeChangeAbilityVisibility) },
            ani_native_function { "nativeConnectAppServiceExtensionAbility", SIGNATURE_CONNECT_SERVICE_EXTENSION,
                reinterpret_cast<void *>(EtsAbilityContext::ConnectAppServiceExtensionAbility) },
            ani_native_function { "nativeDisconnectAppServiceExtensionAbility", SIGNATURE_DISCONNECT_SERVICE_EXTENSION,
                reinterpret_cast<void *>(EtsAbilityContext::DisconnectAppServiceExtensionAbility) },
            ani_native_function { "nativeStartAppServiceExtensionAbility",
                "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsAbilityContext::StartAppServiceExtensionAbility) },
            ani_native_function { "nativeStopAppServiceExtensionAbility",
                "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsAbilityContext::StopAppServiceExtensionAbility) },
            ani_native_function { "nativeStartAbilityWithAccountSync", SIGNATURE_START_ABILITY_WITH_ACCOUNT,
                reinterpret_cast<void *>(EtsAbilityContext::StartAbilityWithAccount) },
            ani_native_function { "nativeStartAbilityWithAccountSync", SIGNATURE_START_ABILITY_WITH_ACCOUNT_OPTIONS,
                reinterpret_cast<void *>(EtsAbilityContext::StartAbilityWithAccountAndOptions) },
#ifdef SUPPORT_GRAPHICS
            ani_native_function { "nativeSetAbilityInstanceInfo",
                "C{std.core.String}C{@ohos.multimedia.image.image.PixelMap}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void*>(EtsAbilityContext::SetAbilityInstanceInfo) },
            ani_native_function { "nativeSetAbilityInstanceInfoCheck",
                "C{std.core.String}C{@ohos.multimedia.image.image.PixelMap}:",
                reinterpret_cast<void *>(EtsAbilityContext::SetAbilityInstanceInfoCheck) },
            ani_native_function { "nativeSetMissionIcon",
                "C{@ohos.multimedia.image.image.PixelMap}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void*>(EtsAbilityContext::SetMissionIcon) },
            ani_native_function { "nativeSetMissionIconCheck", "C{@ohos.multimedia.image.image.PixelMap}:",
                reinterpret_cast<void *>(EtsAbilityContext::SetMissionIconCheck) },
#endif
            ani_native_function { "nativeRevokeDelegator", "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void *>(EtsAbilityContext::RevokeDelegator) },
            ani_native_function { "nativeStartAbilityForResultWithAccount",
                "C{@ohos.app.ability.Want.Want}iC{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void*>(EtsAbilityContext::StartAbilityForResultWithAccount) },
            ani_native_function { "nativeStartAbilityForResultWithAccountCheck", ":",
                reinterpret_cast<void *>(EtsAbilityContext::StartAbilityForResultWithAccountCheck) },
            ani_native_function { "nativeStartAbilityForResultWithAccountVoid",
                "C{@ohos.app.ability.Want.Want}iC{@ohos.app.ability.StartOptions.StartOptions}"
                "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void*>(EtsAbilityContext::StartAbilityForResultWithAccountVoid) },
            ani_native_function { "nativeStartAbilityForResultWithAccountResult",
                "C{@ohos.app.ability.Want.Want}iC{@ohos.app.ability.StartOptions.StartOptions}"
                "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void*>(EtsAbilityContext::StartAbilityForResultWithAccountResult) },
            ani_native_function { "nativeRestoreWindowStage", SIGNATURE_RESTORE_WINDOW_STAGE,
                reinterpret_cast<void *>(EtsAbilityContext::RestoreWindowStage) },
            ani_native_function { "nativeStartAbilityAsCaller", SIGNATURE_START_ABILITY_AS_CALLER,
                reinterpret_cast<void *>(EtsAbilityContext::StartAbilityAsCaller) },
            ani_native_function { "nativeStartRecentAbility", SIGNATURE_START_RECENT_ABILITY,
                reinterpret_cast<void *>(EtsAbilityContext::StartRecentAbility) },
            ani_native_function { "nativeRestartAppSync", "C{@ohos.app.ability.Want.Want}:",
                reinterpret_cast<void*>(EtsAbilityContext::RestartAppWithWindow) },
            ani_native_function { "nativeSetMissionWindowIcon",
                "C{@ohos.multimedia.image.image.PixelMap}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void*>(EtsAbilityContext::SetMissionWindowIcon) },
            ani_native_function { "nativeSetMissionContinueState",
                "C{@ohos.app.ability.AbilityConstant.AbilityConstant.ContinueState}"
                "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void*>(EtsAbilityContext::SetMissionContinueState) },
            ani_native_function { "nativeSetOnNewWantSkipScenarios",
                "iC{utils.AbilityUtils.AsyncCallbackWrapper}:",
                reinterpret_cast<void*>(EtsAbilityContext::SetOnNewWantSkipScenarios) },
        };
        if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK) {
            TAG_LOGE(AAFwkTag::CONTEXT, "Class_BindNativeMethods failed status: %{public}d", status);
            return;
        }

        ani_class cleanerCls = nullptr;
        if ((status = env->FindClass(CLEANER_CLASS, &cleanerCls)) != ANI_OK || cleanerCls == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "Cleaner FindClass failed status: %{public}d, or null cleanerCls", status);
            return;
        }
        std::array cleanerMethods = {
            ani_native_function {"clean", nullptr, reinterpret_cast<void *>(EtsAbilityContext::Clean) },
        };
        if ((status = env->Class_BindNativeMethods(cleanerCls, cleanerMethods.data(), cleanerMethods.size())) !=
            ANI_OK) {
            TAG_LOGE(AAFwkTag::CONTEXT, "cleanerCls Class_BindNativeMethods failed status: %{public}d", status);
            return;
        }
    });
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Class_BindNativeMethods status: %{public}d", status);
        return false;
    }
    return true;
}
} // namespace

ani_object CreateEtsAbilityContext(ani_env *env, std::shared_ptr<AbilityContext> context)
{
    if (env == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env or context");
        return nullptr;
    }
    ani_class cls = nullptr;
    if (!BindNativeMethods(env, cls)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "BindNativeMethods failed");
        return nullptr;
    }
    ani_object contextObj = EtsAbilityContext::SetEtsAbilityContext(env, context);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null contextObj");
        return nullptr;
    }
    ContextUtil::CreateEtsBaseContext(env, cls, contextObj, context);

    auto abilityInfo = context->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null abilityInfo");
        return nullptr;
    }
    ani_ref abilityInfoRef = AppExecFwk::CommonFunAni::ConvertAbilityInfo(env, *abilityInfo);
    ani_status status = env->Object_SetFieldByName_Ref(contextObj, "abilityInfo", abilityInfoRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Object_SetFieldByName_Ref status: %{public}d", status);
        return nullptr;
    }

    auto configuration = context->GetConfiguration();
    if (configuration == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null configuration");
        return nullptr;
    }
    ani_object configurationObj = AppExecFwk::WrapConfiguration(env, *configuration);
    if ((status = env->Object_SetFieldByName_Ref(contextObj, "config", configurationObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Object_SetFieldByName_Ref status: %{public}d", status);
        return nullptr;
    }
    return contextObj;
}

ETSAbilityConnection::ETSAbilityConnection(ani_vm *etsVm) : etsVm_(etsVm) {}

ETSAbilityConnection::~ETSAbilityConnection()
{
    RemoveConnectionObject();
}

void ETSAbilityConnection::SetConnectionId(int32_t id)
{
    connectionId_ = id;
}

void ETSAbilityConnection::ReleaseObjectReference(ani_ref etsObjRef)
{
    if (etsVm_ == nullptr || etsObjRef == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "etsVm_ or etsObjRef null");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetEnv status:%{public}d", status);
        return;
    }
    if ((status = env->GlobalReference_Delete(etsObjRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GlobalReference_Delete status: %{public}d", status);
    }
}

void ETSAbilityConnection::RemoveConnectionObject()
{
    if (etsVm_ != nullptr && etsConnectionRef_ != nullptr) {
        ani_env *env = nullptr;
        if (etsVm_->GetEnv(ANI_VERSION_1, &env) == ANI_OK && env != nullptr) {
            env->GlobalReference_Delete(etsConnectionRef_);
            etsConnectionRef_ = nullptr;
        }
    }
}

void ETSAbilityConnection::CallEtsFailed(int32_t errorCode)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "CallEtsFailed");
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsVm");
        return;
    }
    if (etsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsConnectionRef_");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_OK;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to get env, status: %{public}d", status);
        return;
    }
    ani_ref funRef;
    if ((status = env->Object_GetPropertyByName_Ref(reinterpret_cast<ani_object>(etsConnectionRef_),
        "onFailed", &funRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "get onFailed failed status : %{public}d", status);
        return;
    }
    if (!AppExecFwk::IsValidProperty(env, funRef)) {
        TAG_LOGI(AAFwkTag::CONTEXT, "invalid onFailed property");
        return;
    }
    ani_object errorCodeObj = AppExecFwk::CreateInt(env, errorCode);
    if (errorCodeObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null errorCodeObj");
        return;
    }
    ani_ref result;
    std::vector<ani_ref> argv = { errorCodeObj };
    if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_ONE, argv.data(),
        &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to call onFailed, status: %{public}d", status);
    }
}

void ETSAbilityConnection::SetConnectionRef(ani_object connectOptionsObj)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "SetConnectionRef callled");
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "etsVm_ is nullptr");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        return;
    }
    if ((status = env->GlobalReference_Create(connectOptionsObj, &etsConnectionRef_)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
    }
}

void ETSAbilityConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnAbilityConnectDone");
    HandleOnAbilityConnectDone(element, remoteObject, resultCode);
}

void ETSAbilityConnection::HandleOnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "HandleOnAbilityConnectDone called");
    if (etsVm_ == nullptr || etsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null stsConnectionRef or etsVm");
        return;
    }
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetEnv failed");
        return;
    }
    ani_ref refElement = AppExecFwk::WrapElementName(env, element);
    if (refElement == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null refElement");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null remoteObject");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    ani_object refRemoteObject = ANI_ohos_rpc_CreateJsRemoteObject(env, remoteObject);
    if (refRemoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null refRemoteObject");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    ani_status status = ANI_ERROR;
    ani_ref funRef;
    if ((status = env->Object_GetPropertyByName_Ref(reinterpret_cast<ani_object>(etsConnectionRef_),
        "onConnect", &funRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "get onConnect failed status : %{public}d", status);
        return;
    }
    if (!AppExecFwk::IsValidProperty(env, funRef)) {
        TAG_LOGI(AAFwkTag::CONTEXT, "invalid onConnect property");
        return;
    }
    ani_ref result;
    std::vector<ani_ref> argv = { refElement, refRemoteObject};
    if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_TWO, argv.data(),
        &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to call onConnect, status: %{public}d", status);
    }
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}
void ETSAbilityConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnAbilityDisconnectDone");
    HandleOnAbilityDisconnectDone(element, resultCode);
}

void ETSAbilityConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "HandleOnAbilityDisconnectDone called");
    if (etsVm_ == nullptr || etsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null stsConnectionRef or etsVm");
        return;
    }
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetEnv failed");
        return;
    }
    ani_ref refElement = AppExecFwk::WrapElementName(env, element);
    if (refElement == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null refElement");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    ani_status status = ANI_ERROR;
    ani_ref funRef;
    if ((status = env->Object_GetPropertyByName_Ref(reinterpret_cast<ani_object>(etsConnectionRef_),
        "onDisconnect", &funRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "get onDisconnect failed status : %{public}d", status);
        return;
    }
    if (!AppExecFwk::IsValidProperty(env, funRef)) {
        TAG_LOGI(AAFwkTag::CONTEXT, "invalid onDisconnect property");
        return;
    }
    ani_ref result;
    std::vector<ani_ref> argv = { refElement };
    if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_ONE, argv.data(),
        &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to call onDisconnect, status: %{public}d", status);
    }
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

void EtsAbilityContext::UnwrapCompletionHandlerInStartOptions(ani_env *env, ani_object param,
    AAFwk::StartOptions &options)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "UnwrapCompletionHandlerInStartOptions called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "env null");
        return;
    }
    auto context = context_.lock();
    if (!context) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        return;
    }
    ani_ref completionHandler;
    if (!AppExecFwk::GetFieldRefByName(env, param, "completionHandler", completionHandler) ||
        !completionHandler) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null completionHandler");
        return;
    }
    ani_ref refCompletionHandler = nullptr;
    if (env->GlobalReference_Create(completionHandler, &refCompletionHandler) != ANI_OK ||
        !refCompletionHandler) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to create global ref for completionHandler");
        return;
    }
    OnRequestResult onRequestSucc;
    OnRequestResult onRequestFail;
    AppExecFwk::CreateOnRequestResultCallback(env, refCompletionHandler, "onRequestSuccess", onRequestSucc);
    AppExecFwk::CreateOnRequestResultCallback(env, refCompletionHandler, "onRequestFailure", onRequestFail);
    uint64_t time = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count());
    std::string requestId = std::to_string(time);
    if (context->AddCompletionHandler(requestId, onRequestSucc, onRequestFail) != ERR_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "add completionHandler failed");
        env->GlobalReference_Delete(refCompletionHandler);
        return;
    }
    options.requestId_ = requestId;
}

void EtsAbilityContext::AddCompletionHandlerForOpenLink(ani_env *env, ani_ref refCompletionHandler,
    AAFwk::Want &want, OnRequestResult &onRequestSucc, OnRequestResult &onRequestFail)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "AddCompletionHandlerForOpenLink called");
    auto context = context_.lock();
    if (!context) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        env->GlobalReference_Delete(refCompletionHandler);
        return;
    }
    uint64_t time = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count());
    std::string requestId = std::to_string(time);
    if (context->AddCompletionHandlerForOpenLink(requestId, onRequestSucc, onRequestFail) != ERR_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "add completionHandler failed");
        env->GlobalReference_Delete(refCompletionHandler);
        return;
    }
    want.SetParam(KEY_REQUEST_ID, requestId);
}
} // namespace AbilityRuntime
} // namespace OHOS
