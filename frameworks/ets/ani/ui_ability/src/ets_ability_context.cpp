/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "ani_common_remote.h"
#include "ani_common_want.h"
#include "ani_enum_convert.h"
#include "ani_remote_object.h"
#include "app_utils.h"
#include "common_fun_ani.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "interop_js/arkts_esvalue.h"
#include "interop_js/hybridgref_ani.h"
#include "interop_js/hybridgref_napi.h"
#include "ets_context_utils.h"
#include "ets_error_utils.h"
#include "ets_ui_extension_callback.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
std::mutex EtsAbilityContext::requestCodeMutex_;
namespace {
static std::once_flag g_bindNativeMethodsFlag;
std::recursive_mutex g_connectsLock;
uint32_t g_serialNumber = 0;
constexpr uint64_t MAX_REQUEST_CODE = (1ULL << 49) - 1;
constexpr size_t MAX_REQUEST_CODE_LENGTH = 15;
constexpr int32_t BASE_REQUEST_CODE_NUM = 10;\
constexpr const int FAILED_CODE = -1;
static std::mutex g_connectsMutex;
static std::map<EtsConnectionKey, sptr<ETSAbilityConnection>, EtsKeyCompare> g_connects;
constexpr const char* UI_ABILITY_CONTEXT_CLASS_NAME = "Lapplication/UIAbilityContext/UIAbilityContext;";
constexpr const char* CLEANER_CLASS = "Lapplication/UIAbilityContext/Cleaner;";
const std::string APP_LINKING_ONLY = "appLinkingOnly";
constexpr const char* SIGNATURE_OPEN_LINK = "Lstd/core/String;Lutils/AbilityUtils/AsyncCallbackWrapper;"
    "L@ohos/app/ability/OpenLinkOptions/OpenLinkOptions;Lutils/AbilityUtils/AsyncCallbackWrapper;:V";
constexpr const char *SIGNATURE_ONCONNECT = "LbundleManager/ElementName/ElementName;L@ohos/rpc/rpc/IRemoteObject;:V";
constexpr const char *SIGNATURE_ONDISCONNECT = "LbundleManager/ElementName/ElementName;:V";
constexpr const char *SIGNATURE_CONNECT_SERVICE_EXTENSION =
    "L@ohos/app/ability/Want/Want;Lability/connectOptions/ConnectOptions;:J";
constexpr const char *SIGNATURE_DISCONNECT_SERVICE_EXTENSION = "JLutils/AbilityUtils/AsyncCallbackWrapper;:V";
constexpr const char* SIGNATURE_OPEN_ATOMIC_SERVICE = "Lstd/core/String;Lutils/AbilityUtils/AsyncCallbackWrapper;"
    "L@ohos/app/ability/AtomicServiceOptions/AtomicServiceOptions;:V";
const std::string ATOMIC_SERVICE_PREFIX = "com.atomicservice.";
constexpr const char *SIGNATURE_START_ABILITY_BY_TYPE =
    "Lstd/core/String;Lescompat/Record;Lapplication/AbilityStartCallback/AbilityStartCallback;:L@ohos/base/"
    "BusinessError;";

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
} // namespace

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
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":V", &method)) != ANI_OK ||
        method == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "ctor FindMethod status: %{public}d, or null method", status);
        return nullptr;
    }
    ani_object contextObj = nullptr;
    if ((status = env->Object_New(cls, method, &contextObj)) != ANI_OK || contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Object_New status: %{public}d, or null contextObj", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "setEtsAbilityContextPtr", "J:V", &method)) != ANI_OK ||
        method == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "setEtsAbilityContextPtr FindMethod status: %{public}d, or null method", status);
        return nullptr;
    }
    std::unique_ptr<EtsAbilityContext> etsContext = std::make_unique<EtsAbilityContext>(context);
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
        return nullptr;
    }
    ani_long etsAbilityContextPtr = 0;
    ani_status status = env->Object_GetFieldByName_Long(aniObj, "etsAbilityContextPtr", &etsAbilityContextPtr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "etsAbilityContextPtr GetField status: %{public}d", status);
        return nullptr;
    }
    auto etsContext = reinterpret_cast<EtsAbilityContext *>(etsAbilityContextPtr);
    return etsContext;
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
    etsContext->OnStartServiceExtensionAbility(env, aniObj, wantObj, callbackobj);
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
    etsContext->OnOpenLink(env, aniObj, aniLink, myCallbackobj, optionsObj, callbackobj, isOptionsUndefined,
        isCallbackUndefined);
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
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return FAILED_CODE;
    }
    return etsContext->OnConnectServiceExtensionAbility(env, aniObj, wantObj, connectOptionsObj);
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

int32_t EtsAbilityContext::GenerateRequestCode()
{
    static int32_t curRequestCode_ = 0;
    std::lock_guard lock(requestCodeMutex_);
    curRequestCode_ = (curRequestCode_ == INT_MAX) ? 0 : (curRequestCode_ + 1);
    return curRequestCode_;
}

void EtsAbilityContext::InheritWindowMode(ani_env *env, ani_object aniObj, AAFwk::Want &want)
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
    ani_env *env, ani_object aniObj, ani_object wantObj, ani_object opt, ani_object call)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, must be a Want");
        return;
    }
    InheritWindowMode(env, aniObj, want);
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
        AddFreeInstallObserver(env, want, call, context);
    }
    ErrCode innerErrCode = ERR_OK;
    if (opt != nullptr) {
        AAFwk::StartOptions startOptions;
        if (!AppExecFwk::UnwrapStartOptionsWithProcessOption(env, opt, startOptions)) {
            EtsErrorUtil::ThrowInvalidParamError(env,
                "Parse param startOptions failed, startOptions must be StartOptions.");
            TAG_LOGE(AAFwkTag::CONTEXT, "invalid options");
            return;
        }
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
    }
    TAG_LOGE(AAFwkTag::CONTEXT, "displayId:%{public}d", startOptions.GetDisplayID());
    StartAbilityForResultInner(env, startOptions, want, context, startOptionsObj, callback);
}

void EtsAbilityContext::StartAbilityForResultInner(ani_env *env, const AAFwk::StartOptions &startOptions,
    AAFwk::Want &want, std::shared_ptr<AbilityContext> context, ani_object startOptionsObj, ani_object callback)
{
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
        AddFreeInstallObserver(env, want, callback, context, true);
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
        auto errCode = isInner ? resultCode : 0;
        AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef),
            EtsErrorUtil::CreateErrorByNativeErr(env, errCode), abilityResult);
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
    ani_object callbackobj)
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
    ret = context->StartServiceExtensionAbility(want);
    if (ret == ERR_OK) {
        errorObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ret));
    } else {
        errorObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    }
    AppExecFwk::AsyncCallback(env, callbackobj, errorObject, nullptr);
}

void EtsAbilityContext::OnOpenLink(ani_env *env, ani_object aniObj, ani_string aniLink, ani_object myCallbackobj,
    ani_object optionsObj, ani_object callbackobj, bool haveOptionsParm, bool haveCallBackParm)
{
    ani_object aniObject = nullptr;
    std::string link("");
    AAFwk::OpenLinkOptions openLinkOptions;
    AAFwk::Want want;
    want.SetParam(APP_LINKING_ONLY, false);
    if (!AppExecFwk::GetStdString(env, aniLink, link)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "parse link failed");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "Parse param link failed, link must be string.");
        AppExecFwk::AsyncCallback(env, myCallbackobj, aniObject, nullptr);
        return;
    }
    if (haveOptionsParm) {
        TAG_LOGD(AAFwkTag::CONTEXT, "OpenLink Have option");
        EtsAbilityContext::UnWrapOpenLinkOptions(env, optionsObj, openLinkOptions, want);
    }
    want.SetUri(link);
    std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count());
    want.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
    int requestCode = -1;
    ErrCode ret = ERR_OK;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetAbilityContext is nullptr");
        ret = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ret));
        AppExecFwk::AsyncCallback(env, myCallbackobj, aniObject, nullptr);
        return;
    }
    AddFreeInstallObserver(env, want, myCallbackobj, context, false, true);
    if (haveCallBackParm) {
        TAG_LOGD(AAFwkTag::CONTEXT, "OpenLink Have Callback");
        CreateOpenLinkTask(env, callbackobj, context, want, requestCode);
    }
    ret = context->OpenLink(want, requestCode);
    if (ret == AAFwk::ERR_OPEN_LINK_START_ABILITY_DEFAULT_OK) {
        aniObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(ERR_OK));
    } else {
        aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret));
    }
    AppExecFwk::AsyncCallback(env, myCallbackobj, aniObject, nullptr);
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
    errorObject = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(errCode));
    AppExecFwk::AsyncCallback(env, callbackObj, errorObject, nullptr);
}

ani_long EtsAbilityContext::OnConnectServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object connectOptionsObj)
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
    auto innerErrCode = context->ConnectAbility(want, connection);
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
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
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
    }
    OpenAtomicServiceInner(env, aniObj, want, startOptions, appId, callbackObj);
}

void EtsAbilityContext::AddFreeInstallObserver(ani_env *env, const AAFwk::Want &want, ani_object callback,
    const std::shared_ptr<AbilityContext> &context, bool isAbilityResult, bool isOpenLink)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "AddFreeInstallObserver");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null env");
        return;
    }
    if (context == nullptr) {
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

void EtsAbilityContext::UnWrapOpenLinkOptions(ani_env *env, ani_object optionsObj,
    AAFwk::OpenLinkOptions &openLinkOptions, AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "UnWrapOpenLinkOptions");
    ani_status status = ANI_ERROR;
    ani_ref ParamRef = nullptr;
    if ((status = env->Object_GetPropertyByName_Ref(optionsObj, "parameters", &ParamRef)) == ANI_OK) {
        AAFwk::WantParams wantParam;
        if (AppExecFwk::UnwrapWantParams(env, ParamRef, wantParam)) {
            want.SetParams(wantParam);
        } else {
            TAG_LOGE(AAFwkTag::CONTEXT, "UnwrapWantParams failed");
        }
    }
    if ((status = env->Object_GetPropertyByName_Ref(optionsObj, APP_LINKING_ONLY.c_str(), &ParamRef)) == ANI_OK) {
        bool appLinkingOnly = false;
        AppExecFwk::GetFieldBoolByName(env, optionsObj, "appLinkingOnly", appLinkingOnly);
        openLinkOptions.SetAppLinkingOnly(appLinkingOnly);
        want.SetParam(APP_LINKING_ONLY, appLinkingOnly);
    }
    if (!want.HasParameter(APP_LINKING_ONLY)) {
        want.SetParam(APP_LINKING_ONLY, false);
    }
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

void EtsAbilityContext::ConfigurationUpdated(ani_env *env, std::shared_ptr<AppExecFwk::ETSNativeReference> &etsContext,
    const std::shared_ptr<AppExecFwk::Configuration> &config)
{
    if (env == nullptr || etsContext == nullptr || config == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "env or etsContext or config is null");
        return;
    }
    ani_ref configurationRef = AppExecFwk::WrapConfiguration(env, *config);
    ani_status status = env->Object_SetFieldByName_Ref(etsContext->aniObj, "config", configurationRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "config SetField status: %{public}d", status);
        return;
    }
}

ani_object EtsAbilityContext::NativeTransferStatic(ani_env *env, ani_object, ani_object input)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "transfer static UIAbilityContext");
    void *unwrapResult = nullptr;
    bool success = arkts_esvalue_unwrap(env, input, &unwrapResult);
    if (!success) {
        TAG_LOGE(AAFwkTag::UIABILITY, "failed to unwrap");
        EtsErrorUtil::ThrowEtsTransferClassError(env);
        return nullptr;
    }
    if (unwrapResult == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null unwrapResult");
        EtsErrorUtil::ThrowEtsTransferClassError(env);
        return nullptr;
    }
    auto context = reinterpret_cast<std::weak_ptr<AbilityContext> *>(unwrapResult)->lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null context");
        EtsErrorUtil::ThrowEtsTransferClassError(env);
        return nullptr;
    }
    auto abilityContext = Context::ConvertTo<AbilityContext>(context);
    if (abilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext");
        EtsErrorUtil::ThrowEtsTransferClassError(env);
        return nullptr;
    }
    auto &bindingObj = abilityContext->GetBindingObject();
    if (bindingObj == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null bindingObj");
        EtsErrorUtil::ThrowEtsTransferClassError(env);
        return nullptr;
    }
    auto staticContext = bindingObj->Get<ani_ref>();
    if (staticContext == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null staticContext");
        EtsErrorUtil::ThrowEtsTransferClassError(env);
        return nullptr;
    }
    return reinterpret_cast<ani_object>(*staticContext);
}

ani_object EtsAbilityContext::NativeTransferDynamic(ani_env *env, ani_object, ani_object input)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "transfer dynamic UIAbilityContext");
    if (!IsInstanceOf(env, input)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "not UIAbilityContext");
        EtsErrorUtil::ThrowEtsTransferClassError(env);
        return nullptr;
    }
    auto context = ContextUtil::GetBaseContext(env, input);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null context");
        EtsErrorUtil::ThrowEtsTransferClassError(env);
        return nullptr;
    }
    auto abilityContext = Context::ConvertTo<AbilityContext>(context);
    if (abilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext");
        EtsErrorUtil::ThrowEtsTransferClassError(env);
        return nullptr;
    }
    auto &bindingObj = abilityContext->GetBindingObject();
    if (bindingObj == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null bindingObj");
        EtsErrorUtil::ThrowEtsTransferClassError(env);
        return nullptr;
    }
    auto dynamicContext = bindingObj->Get<NativeReference>();
    if (dynamicContext == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null dynamicContext");
        EtsErrorUtil::ThrowEtsTransferClassError(env);
        return nullptr;
    }
    // Not support yet
    EtsErrorUtil::ThrowEtsTransferClassError(env);
    return nullptr;
}

bool EtsAbilityContext::IsInstanceOf(ani_env *env, ani_object aniObj)
{
    ani_class cls {};
    ani_status status = ANI_ERROR;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return false;
    }
    if ((status = env->FindClass(UI_ABILITY_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "status: %{public}d", status);
        return false;
    }
    ani_boolean isInstanceOf = false;
    if ((status = env->Object_InstanceOf(aniObj, cls, &isInstanceOf)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "status: %{public}d", status);
        return false;
    }
    return isInstanceOf;
}

void EtsAbilityContext::NativeOnSetRestoreEnabled(ani_env *env, ani_object aniObj, ani_boolean aniEnabled)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "NativeOnSetRestoreEnabled");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
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
    AddFreeInstallObserver(env, want, callbackObj, context, true);
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
    ErrCode ErrCode = context->OpenAtomicService(want, options, requestCode, std::move(task));
    if (ErrCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "OpenAtomicService failed, ErrCode: %{public}d", ErrCode);
    }
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
                "L@ohos/app/ability/Want/Want;Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
                reinterpret_cast<void *>(EtsAbilityContext::StartAbility) },
            ani_native_function { "nativeStartAbilitySync",
                "L@ohos/app/ability/Want/Want;L@ohos/app/ability/StartOptions/StartOptions;Lutils/AbilityUtils/"
                "AsyncCallbackWrapper;:V",
                reinterpret_cast<void *>(EtsAbilityContext::StartAbilityWithOptions) },
            ani_native_function { "nativeStartAbilityForResult",
                "L@ohos/app/ability/Want/Want;Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
                reinterpret_cast<void *>(EtsAbilityContext::StartAbilityForResult) },
            ani_native_function { "nativeStartAbilityForResult",
                "L@ohos/app/ability/Want/Want;L@ohos/app/ability/StartOptions/StartOptions;Lutils/AbilityUtils/"
                "AsyncCallbackWrapper;:V",
                reinterpret_cast<void *>(EtsAbilityContext::StartAbilityForResultWithOptions) },
            ani_native_function { "nativeTerminateSelfSync", "Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
                reinterpret_cast<void *>(EtsAbilityContext::TerminateSelf) },
            ani_native_function { "nativeTerminateSelfWithResult",
                "Lability/abilityResult/AbilityResult;Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
                reinterpret_cast<void *>(EtsAbilityContext::TerminateSelfWithResult) },
            ani_native_function { "nativeReportDrawnCompletedSync", "Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
                reinterpret_cast<ani_int *>(EtsAbilityContext::ReportDrawnCompleted) },
            ani_native_function { "nativeStartServiceExtensionAbility",
                "L@ohos/app/ability/Want/Want;Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
                reinterpret_cast<void*>(EtsAbilityContext::StartServiceExtensionAbility) },
            ani_native_function { "nativeOpenLink", SIGNATURE_OPEN_LINK,
                reinterpret_cast<void*>(EtsAbilityContext::OpenLink) },
            ani_native_function { "nativeIsTerminating", ":Z",
                reinterpret_cast<void*>(EtsAbilityContext::IsTerminating) },
            ani_native_function { "nativeMoveAbilityToBackground", "Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
                reinterpret_cast<void*>(EtsAbilityContext::MoveAbilityToBackground) },
            ani_native_function { "nativeRequestModalUIExtension",
                "L@ohos/app/ability/Want/Want;Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
                reinterpret_cast<void*>(EtsAbilityContext::RequestModalUIExtension) },
            ani_native_function { "nativeBackToCallerAbilityWithResult",
                "Lability/abilityResult/AbilityResult;Lstd/core/String;Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
                reinterpret_cast<void*>(EtsAbilityContext::BackToCallerAbilityWithResult) },
            ani_native_function { "nativeSetMissionLabel",
                "Lstd/core/String;Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
                reinterpret_cast<void *>(EtsAbilityContext::SetMissionLabel) },
            ani_native_function { "nativeConnectServiceExtensionAbility", SIGNATURE_CONNECT_SERVICE_EXTENSION,
                reinterpret_cast<void *>(EtsAbilityContext::ConnectServiceExtensionAbility) },
            ani_native_function { "nativeDisconnectServiceExtensionAbility", SIGNATURE_DISCONNECT_SERVICE_EXTENSION,
                reinterpret_cast<void *>(EtsAbilityContext::DisconnectServiceExtensionAbility) },
            ani_native_function {"nativeSetColorMode",
                "L@ohos/app/ability/ConfigurationConstant/ConfigurationConstant/ColorMode;:V",
                reinterpret_cast<void*>(EtsAbilityContext::SetColorMode)},
            ani_native_function { "nativeStartAbilityByTypeSync", SIGNATURE_START_ABILITY_BY_TYPE,
                reinterpret_cast<void*>(EtsAbilityContext::StartAbilityByType) },
            ani_native_function { "nativeOpenAtomicService", SIGNATURE_OPEN_ATOMIC_SERVICE,
                reinterpret_cast<void *>(EtsAbilityContext::OpenAtomicService) },
            ani_native_function { "nativeOnSetRestoreEnabled", "Z:V",
                reinterpret_cast<void*>(EtsAbilityContext::NativeOnSetRestoreEnabled) },
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

void ETSAbilityConnection::RemoveConnectionObject()
{
    if (etsVm_ != nullptr && stsConnectionRef_ != nullptr) {
        ani_env *env = nullptr;
        if (etsVm_->GetEnv(ANI_VERSION_1, &env) == ANI_OK && env != nullptr) {
            env->GlobalReference_Delete(stsConnectionRef_);
            stsConnectionRef_ = nullptr;
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
    if (stsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null stsConnectionRef_");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_OK;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to get env, status: %{public}d", status);
        return;
    }
    status = env->Object_CallMethodByName_Void(reinterpret_cast<ani_object>(stsConnectionRef_), "onFailed", "D:V",
        static_cast<double>(errorCode));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to call onFailed, status: %{public}d", status);
    }
}

void ETSAbilityConnection::SetConnectionRef(ani_object connectOptionsObj)
{
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
    if ((status = env->GlobalReference_Create(connectOptionsObj, &stsConnectionRef_)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
    }
}

void ETSAbilityConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnAbilityConnectDone");
    if (etsVm_ == nullptr || stsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null stsConnectionRef or etsVm");
        return;
    }
    ani_env *env = AttachCurrentThread();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetEnv failed");
        return;
    }
    ani_ref refElement = AppExecFwk::WrapElementName(env, element);
    if (refElement == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null refElement");
        DetachCurrentThread();
        return;
    }
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null remoteObject");
        DetachCurrentThread();
        return;
    }
    ani_object refRemoteObject = AniRemote::CreateAniRemoteObject(env, remoteObject);
    if (refRemoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null refRemoteObject");
        DetachCurrentThread();
        return;
    }
    ani_status status = ANI_ERROR;
    if ((status = env->Object_CallMethodByName_Void(reinterpret_cast<ani_object>(stsConnectionRef_), "onConnect",
        SIGNATURE_ONCONNECT, refElement, refRemoteObject)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to call onConnect, status: %{public}d", status);
    }
    DetachCurrentThread();
}
void ETSAbilityConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "OnAbilityDisconnectDone");
    if (etsVm_ == nullptr || stsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null stsConnectionRef or etsVm");
        return;
    }
    ani_env *env = AttachCurrentThread();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetEnv failed");
        return;
    }
    ani_ref refElement = AppExecFwk::WrapElementName(env, element);
    if (refElement == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null refElement");
        DetachCurrentThread();
        return;
    }
    ani_status status = ANI_ERROR;
    if ((status = env->Object_CallMethodByName_Void(reinterpret_cast<ani_object>(stsConnectionRef_), "onDisconnect",
        SIGNATURE_ONDISCONNECT, refElement)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to call onDisconnect, status: %{public}d", status);
    }
    DetachCurrentThread();
}

ani_env *ETSAbilityConnection::AttachCurrentThread()
{
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) == ANI_OK) {
        return env;
    }
    ani_option interopEnabled { "--interop=disable", nullptr };
    ani_options aniArgs { 1, &interopEnabled };
    if ((status = etsVm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status: %{public}d", status);
        return nullptr;
    }
    isAttachThread_ = true;
    return env;
}

void ETSAbilityConnection::DetachCurrentThread()
{
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null etsVm");
        return;
    }
    if (isAttachThread_) {
        etsVm_->DetachCurrentThread();
        isAttachThread_ = false;
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
