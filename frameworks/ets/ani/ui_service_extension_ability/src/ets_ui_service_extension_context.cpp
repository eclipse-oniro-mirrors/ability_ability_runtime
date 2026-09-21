/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "ets_ui_service_extension_context.h"

#include <chrono>
#include <cstdint>

#include "ability_manager_client.h"
#include "ability_runtime/js_caller_complex.h"
#include "ani_common_start_options.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "remote_object_taihe_ani.h"
#include "ets_context_utils.h"
#include "ets_data_struct_converter.h"
#include "ets_error_utils.h"
#include "ets_extension_context.h"
#include "ets_free_install_observer.h"
#include "ets_runtime.h"
#include "ets_ui_extension_callback.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "start_options.h"
#include "ui_service_extension.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const int FAILED_CODE = -1;
constexpr int32_t ERROR_CODE_ONE = 1;
constexpr int32_t ERROR_CODE_TWO = 2;
constexpr int32_t INVALID_PARAM = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
constexpr const char *UI_SERVICE_CONTEXT_CLASS_NAME =
    "application.UIServiceExtensionContext.UIServiceExtensionContext";
const char *UI_SERVICE_EXTENSION_CONTEXT_CLEANER_CLASS_NAME = "application.UIServiceExtensionContext.Cleaner";
constexpr const char *SIGNATURE_START_ABILITY = "C{utils.AbilityUtils.AsyncCallbackWrapper}"
    "C{@ohos.app.ability.Want.Want}C{@ohos.app.ability.StartOptions.StartOptions}:";
constexpr const char *SIGNATURE_START_ABILITY_CHK = "C{@ohos.app.ability.Want.Want}"
    "C{@ohos.app.ability.StartOptions.StartOptions}:";
constexpr const char *SIGNATURE_TERMINATE_SELF = "C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *SIGNATURE_START_ABILITY_BY_TYPE = "C{std.core.String}C{std.core.Record}"
    "C{application.AbilityStartCallback.AbilityStartCallback}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *SIGNATURE_START_ABILITY_BY_TYPE_CHK = "C{std.core.String}C{std.core.Record}:";
constexpr const char *SIGNATURE_CONNECT_SERVICE_EXTENSION =
    "C{@ohos.app.ability.Want.Want}C{ability.connectOptions.ConnectOptions}:l";
constexpr const char *SIGNATURE_DISCONNECT_SERVICE_EXTENSION = "lC{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr int32_t ARGC_ONE = 1;
constexpr int32_t ARGC_TWO = 2;

struct ConnectionKey {
    AAFwk::Want want;
    int64_t id;
    int32_t accountId;
};
struct key_compare {
    bool operator()(const ConnectionKey &key1, const ConnectionKey &key2) const
    {
        if (key1.id < key2.id) {
            return true;
        }
        return false;
    }
};
static std::mutex g_connectsMutex;
static std::map<ConnectionKey, sptr<EtsUIServiceExtensionConnection>, key_compare> g_connects;
static int64_t g_serialNumber = 0;
} // namespace

EtsUIServiceExtensionContext* EtsUIServiceExtensionContext::GetEtsUIServiceExtensionContext(ani_env *env,
    ani_object obj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null env");
        return nullptr;
    }
    EtsUIServiceExtensionContext *etsContext = nullptr;
    ani_status status = ANI_ERROR;
    ani_long etsUIServiceContextLong = 0;
    if ((status = env->Object_GetFieldByName_Long(obj, "nativeUiExtensionContext",
        &etsUIServiceContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "status: %{public}d", status);
        return nullptr;
    }
    etsContext = reinterpret_cast<EtsUIServiceExtensionContext *>(etsUIServiceContextLong);
    if (etsContext == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "etsContext null");
        return nullptr;
    }
    return etsContext;
}

std::shared_ptr<UIServiceExtensionContext> EtsUIServiceExtensionContext::GetContext()
{
    std::shared_ptr<UIServiceExtensionContext> context = nullptr;
    if (!context_.expired() && (context = context_.lock()) != nullptr) {
        return context;
    }
    return nullptr;
}

void EtsUIServiceExtensionContext::StartAbilityCheck(
    ani_env *env, ani_object obj, ani_object aniWant, ani_object aniStartOption)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "StartAbilityCheck called");
    if (env == nullptr || obj == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null env or obj");
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, aniWant, want)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "UnwrapWant  failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        return;
    }
    ani_status status = ANI_OK;
    ani_boolean isUndefined = false;
    if ((status = env->Reference_IsUndefined(aniStartOption, &isUndefined)) != ANI_OK) {
        return;
    }
    AAFwk::StartOptions startOptions;
    if (!isUndefined && !AppExecFwk::UnwrapStartOptions(env, aniStartOption, startOptions)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "UnwrapStartOptions filed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param startOptions failed.");
    }
}

void EtsUIServiceExtensionContext::StartAbility(
    ani_env *env, ani_object obj, ani_object callback, ani_object aniWant, ani_object aniStartOption)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "StartAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null env");
        return;
    }
    auto etsUiExtensionContext = GetEtsUIServiceExtensionContext(env, obj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnStartAbility(env, callback, aniWant, aniStartOption);
}

void EtsUIServiceExtensionContext::OnStartAbility(
    ani_env *env, ani_object callback, ani_object aniWant, ani_object aniStartOption)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnStartAbility Called");
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, aniWant, want)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "UnwrapWant  failed");
        return;
    }
    ani_status status = ANI_OK;
    ani_boolean isUndefined = false;
    if ((status = env->Reference_IsUndefined(aniStartOption, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to check undefined status : %{public}d", status);
        return;
    }
    AAFwk::StartOptions startOptions;
    if (!isUndefined && !AppExecFwk::UnwrapStartOptions(env, aniStartOption, startOptions)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "UnwrapStartOptions filed");
        return;
    }
#ifdef SUPPORT_SCREEN
    InitDisplayId(want, startOptions);
#endif
    int32_t innerErrCode = static_cast<int32_t>(ERR_OK);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null context");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, static_cast<AbilityRuntime::AbilityErrorCode>(
                AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)),
            nullptr);
        return;
    }
    innerErrCode = static_cast<int32_t>(context->StartAbility(want, startOptions));
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "StartAbility innerErrCode: %{public}d", innerErrCode);
    if (innerErrCode == static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)) {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, static_cast<AbilityRuntime::AbilityErrorCode>(
                AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)),
            nullptr);
    } else {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(innerErrCode)),
            nullptr);
    }
}

#ifdef SUPPORT_SCREEN
void EtsUIServiceExtensionContext::InitDisplayId(AAFwk::Want &want, AAFwk::StartOptions &startOptions)
{
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null context");
        return;
    }
    auto window = context->GetWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null window");
        return;
    }
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "startOption displayId %{public}d", startOptions.GetDisplayID());
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "window displayId %{public}" PRIu64, window->GetDisplayId());
    startOptions.SetDisplayID(window->GetDisplayId());
}
#endif

void EtsUIServiceExtensionContext::TerminateSelf(ani_env *env, ani_object obj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "TerminateSelf Called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null env");
        return;
    }
    auto etsUiExtensionContext = GetEtsUIServiceExtensionContext(env, obj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnTerminateSelf(env, callback);
}

void EtsUIServiceExtensionContext::OnTerminateSelf(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnTerminateSelf Called");
    int32_t innerErrCode = static_cast<int32_t>(ERR_OK);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "null context");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, ERROR_CODE_ONE, "Context is released"),
            nullptr);
        return;
    }
    innerErrCode= context->TerminateSelf();
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "TerminateSelf innerErrCode: %{public}d", innerErrCode);
    AppExecFwk::AsyncCallback(env, callback,
        EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(innerErrCode)),
        nullptr);
}

void EtsUIServiceExtensionContext::StartAbilityByTypeCheck(
    ani_env *env, ani_object obj, ani_string aniType, ani_object aniWantParam)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "StartAbilityByTypeCheck Called");
    if (env == nullptr || obj == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null env or obj");
        return;
    }
    std::string type;
    if (!OHOS::AppExecFwk::GetStdString(env, aniType, type)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "GetStdString Failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Incorrect parameter types, param type must be a string");
        return;
    }
    AAFwk::WantParams wantParam;
    if (!AppExecFwk::UnwrapWantParams(env, aniWantParam, wantParam)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "parse wantParam failed");
        EtsErrorUtil::ThrowError(env, INVALID_PARAM, "Parameter error. The type of \"WantParams\" must be array");
    }
}

void EtsUIServiceExtensionContext::StartAbilityByType(ani_env *env, ani_object obj,
    ani_string aniType, ani_object aniWantParam, ani_object abilityStartCallback, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "StartAbilityByType Called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null env");
        return;
    }
    auto etsUiExtensionContext = GetEtsUIServiceExtensionContext(env, obj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnStartAbilityByType(env, aniType, aniWantParam, abilityStartCallback, callback);
}

void EtsUIServiceExtensionContext::OnStartAbilityByType(ani_env *env, ani_string aniType,
    ani_object aniWantParam, ani_object abilityStartCallback, ani_object aniCallback)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnStartAbilityByType Called");
    std::string type;
    if (!OHOS::AppExecFwk::GetStdString(env, aniType, type)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "GetStdString Failed");
        return;
    }
    AAFwk::WantParams wantParam;
    if (!AppExecFwk::UnwrapWantParams(env, aniWantParam, wantParam)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "parse wantParam failed");
        return;
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "get aniVM failed");
        return;
    }
    std::shared_ptr<EtsUIExtensionCallback> callback = std::make_shared<EtsUIExtensionCallback>(aniVM);
    callback->SetEtsCallbackObject(abilityStartCallback);
    ani_ref completionHandler;
    ani_boolean isUndefined = false;
    AppExecFwk::GetPropertyRef(env, abilityStartCallback, "completionHandler", completionHandler, isUndefined);
    if (!isUndefined && completionHandler != nullptr) {
        callback->SetCompletionHandler(env, static_cast<ani_object>(completionHandler));
    }
    int32_t innerErrCode = static_cast<int32_t>(ERR_OK);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "null context");
        AppExecFwk::AsyncCallback(env, aniCallback,
            EtsErrorUtil::CreateError(env,
                static_cast<AbilityRuntime::AbilityErrorCode>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)),
            nullptr);
        return;
    }
    innerErrCode = context->StartAbilityByType(type, wantParam, callback);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "StartAbilityByType innerErrCode: %{public}d", innerErrCode);
    if (innerErrCode == ERR_OK) {
        callback->SetAsyncCallback(env, aniCallback);
    } else {
        AppExecFwk::AsyncCallback(env, aniCallback,
            EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(innerErrCode)),
            nullptr);
    }
}

ani_long EtsUIServiceExtensionContext::ConnectServiceExtensionAbility(ani_env *env, ani_object obj,
    ani_object aniWant, ani_object aniOptions)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Connect ServiceExtensionAbility called.");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null env");
        return FAILED_CODE;
    }
    auto etsUiExtensionContext = GetEtsUIServiceExtensionContext(env, obj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null etsUiExtensionContext");
        return FAILED_CODE;
    }
    return etsUiExtensionContext->OnConnectServiceExtensionAbility(env, aniWant, aniOptions);
}

ani_long EtsUIServiceExtensionContext::OnConnectServiceExtensionAbility(ani_env *env,
    ani_object aniWant, ani_object aniConnectObj)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnConnectServiceExtensionAbility called.");
    // Unwrap want and connection
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, aniWant, want)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "UnwrapWant  failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, must be a Want.");
        return FAILED_CODE;
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "get aniVM failed");
        return FAILED_CODE;
    }
    sptr<EtsUIServiceExtensionConnection> connection = sptr<EtsUIServiceExtensionConnection>::MakeSptr(aniVM);
    if (!CheckConnectionParam(env, aniConnectObj, connection, want)) {
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param options failed, must be a ConnectOptions.");
        return FAILED_CODE;
    }
    int64_t connectId = connection->GetConnectionId();
    int32_t innerErrorCode = static_cast<int32_t>(ERR_OK);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null context");
        EtsErrorUtil::ThrowError(env, static_cast<AbilityRuntime::AbilityErrorCode>(
            AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
        return FAILED_CODE;
    }
    innerErrorCode = context->ConnectServiceExtensionAbility(want, connection);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "ConnectServiceExtensionAbility innerErrorCode: %{public}d", innerErrorCode);
    if (innerErrorCode != RESULT_OK) {
        connection->CallEtsFailed(static_cast<int32_t>(GetJsErrorCodeByNativeError(innerErrorCode)));
        RemoveConnection(connectId);
    }
    return connectId;
}

bool EtsUIServiceExtensionContext::CheckConnectionParam(ani_env *env, ani_object connectObj,
    sptr<EtsUIServiceExtensionConnection>& connection, AAFwk::Want& want, int32_t accountId)
{
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null connection");
        return false;
    }
    connection->SetEtsConnectionObject(connectObj);
    ConnectionKey key;
    {
        std::lock_guard guard(g_connectsMutex);
        key.id = g_serialNumber;
        key.want = want;
        key.accountId = accountId;
        connection->SetConnectionId(key.id);
        g_connects.emplace(key, connection);
        if (g_serialNumber < INT32_MAX) {
            g_serialNumber++;
        } else {
            g_serialNumber = 0;
        }
    }
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Unable to find connection, make new one");
    return true;
}

void EtsUIServiceExtensionContext::RemoveConnection(int64_t connectId)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "RemoveConnection called");
    std::lock_guard guard(g_connectsMutex);
    auto item = std::find_if(g_connects.begin(), g_connects.end(),
    [&connectId](const auto &obj) {
        return connectId == obj.first.id;
    });
    if (item != g_connects.end()) {
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "remove conn ability exist.");
        if (item->second) {
            item->second->RemoveConnectionObject();
        }
        g_connects.erase(item);
    } else {
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "remove conn ability not exist.");
    }
}

void EtsUIServiceExtensionContext::DisConnectServiceExtensionAbility(ani_env *env, ani_object obj,
    ani_long aniConnectionId, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "DisConnect ServiceExtensionAbility start");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null env");
        return;
    }
    auto etsUiExtensionContext = GetEtsUIServiceExtensionContext(env, obj);
    if (etsUiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null etsUiExtensionContext");
        return;
    }
    etsUiExtensionContext->OnDisConnectServiceExtensionAbility(env, aniConnectionId, callback);
}

void EtsUIServiceExtensionContext::OnDisConnectServiceExtensionAbility(ani_env *env,
    ani_long aniConnectionId, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "DisConnectServiceExtensionAbility start");
    int64_t connectionId = static_cast<int64_t>(aniConnectionId);
    AAFwk::Want want;
    sptr<EtsUIServiceExtensionConnection> connection = nullptr;
    int32_t accountId = -1;
    FindConnection(want, connection, connectionId, accountId);
    // begin disconnect
    int32_t innerErrCode = static_cast<int32_t>(ERR_OK);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null context");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, ERROR_CODE_ONE, "Context is released"), nullptr);
        return;
    }
    if (connection == nullptr) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "null connection");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, ERROR_CODE_TWO, "not found connection"), nullptr);
        return;
    }
    innerErrCode = context->DisConnectServiceExtensionAbility(want, connection, accountId);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "DisConnectServiceExtensionAbility innerErrorCode: %{public}d", innerErrCode);
    AppExecFwk::AsyncCallback(env, callback,
        EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(innerErrCode)), nullptr);
}

void EtsUIServiceExtensionContext::FindConnection(
    AAFwk::Want &want, sptr<EtsUIServiceExtensionConnection> &connection, int64_t &connectId, int32_t &accountId)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "connection:%{public}d", static_cast<int32_t>(connectId));
    std::lock_guard guard(g_connectsMutex);
    auto item = std::find_if(g_connects.begin(),
        g_connects.end(),
        [&connectId](const auto &obj) {
            return connectId == obj.first.id;
        });
    if (item != g_connects.end()) {
        // match id
        want = item->first.want;
        connection = item->second;
        accountId = item->first.accountId;
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "find conn ability exist");
    }
}

ani_object CreateEtsUIServiceExtensionContext(ani_env *env, std::shared_ptr<UIServiceExtensionContext> context)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "CreateEtsUIServiceExtensionContext called");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context == nullptr || env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null context or env nullptr");
        return nullptr;
    }
    abilityInfo = context->GetAbilityInfo();
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object contextObj = nullptr;
    if ((env->FindClass(UI_SERVICE_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", "l:", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "status: %{public}d", status);
        return nullptr;
    }
    std::unique_ptr<EtsUIServiceExtensionContext> etsContext = std::make_unique<EtsUIServiceExtensionContext>(context);
    if ((status = env->Object_New(cls, method, &contextObj, reinterpret_cast<ani_long>(etsContext.release()))) !=
        ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "status: %{public}d", status);
        return nullptr;
    }
    EtsUIServiceExtensionContext::BindNativeMethods(env);
    auto workContext = new (std::nothrow)std::weak_ptr<AbilityRuntime::UIServiceExtensionContext>(context);
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null workContext");
        return nullptr;
    }
    if (!ContextUtil::SetNativeContextLong(env, contextObj, (ani_long)workContext)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "SetNativeContextLong failed");
        delete workContext;
        return nullptr;
    }
    if (!EtsUIServiceExtensionContext::BindNativePtrCleaner(env)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "status: %{public}d", status);
        delete workContext;
        return nullptr;
    }
    AbilityRuntime::ContextUtil::CreateEtsBaseContext(env, cls, contextObj, context);
    AbilityRuntime::CreateEtsExtensionContext(env, cls, contextObj, context, context->GetAbilityInfo());
    ani_ref *contextGlobalRef = new (std::nothrow) ani_ref;
    if (contextGlobalRef == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null contextGlobalRef");
        return nullptr;
    }
    if ((status = env->GlobalReference_Create(contextObj, contextGlobalRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "GlobalReference_Create failed status: %{public}d", status);
        delete contextGlobalRef;
        return nullptr;
    }
    context->Bind(contextGlobalRef);
    return contextObj;
}

bool EtsUIServiceExtensionContext::BindNativePtrCleaner(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "nullptr env");
        return false;
    }
    ani_class cleanerCls;
    ani_status status = env->FindClass(UI_SERVICE_EXTENSION_CONTEXT_CLEANER_CLASS_NAME, &cleanerCls);
    if (ANI_OK != status) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Not found Cleaner. status:%{public}d.", status);
        return false;
    }
    std::array methods = {
        ani_native_function { "clean", nullptr, reinterpret_cast<void *>(EtsUIServiceExtensionContext::Clean) },
    };
    if ((status = env->Class_BindNativeMethods(cleanerCls, methods.data(), methods.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "status: %{public}d", status);
        return false;
    }
    return true;
}

void EtsUIServiceExtensionContext::Clean(ani_env *env, ani_object object)
{
    ani_long ptr = 0;
    if (ANI_OK != env->Object_GetFieldByName_Long(object, "nativeServiceExtensionContext", &ptr)) {
        return;
    }

    if (ptr != 0) {
        delete reinterpret_cast<EtsUIServiceExtensionContext*>(ptr);
        ptr = 0;
    }
}

bool EtsUIServiceExtensionContext::BindNativeMethods(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "env nullptr");
        return false;
    }
    std::array functions = {
        ani_native_function{"nativeStartAbility", SIGNATURE_START_ABILITY,
            reinterpret_cast<void *>(EtsUIServiceExtensionContext::StartAbility)},
        ani_native_function{"nativeTerminateSelf", SIGNATURE_TERMINATE_SELF,
            reinterpret_cast<void *>(EtsUIServiceExtensionContext::TerminateSelf)},
        ani_native_function{
            "nativeStartAbilityByType", SIGNATURE_START_ABILITY_BY_TYPE,
            reinterpret_cast<void *>(EtsUIServiceExtensionContext::StartAbilityByType)},
        ani_native_function{"nativeConnectServiceExtensionAbility", SIGNATURE_CONNECT_SERVICE_EXTENSION,
            reinterpret_cast<void *>(EtsUIServiceExtensionContext::ConnectServiceExtensionAbility)},
        ani_native_function{"nativeDisconnectServiceExtensionAbility", SIGNATURE_DISCONNECT_SERVICE_EXTENSION,
            reinterpret_cast<void *>(EtsUIServiceExtensionContext::DisConnectServiceExtensionAbility)},
        ani_native_function{"nativeStartAbilityCheck", SIGNATURE_START_ABILITY_CHK,
            reinterpret_cast<void *>(EtsUIServiceExtensionContext::StartAbilityCheck)},
        ani_native_function{"nativeStartAbilityByTypeCheck", SIGNATURE_START_ABILITY_BY_TYPE_CHK,
            reinterpret_cast<void *>(EtsUIServiceExtensionContext::StartAbilityByTypeCheck)},
    };
    ani_class cls {};
    ani_status status = env->FindClass(UI_SERVICE_CONTEXT_CLASS_NAME, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "FindClass failed status: %{public}d", status);
        return false;
    }
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Class_BindNativeMethods status: %{public}d", status);
        return false;
    }
    return true;
}

EtsUIServiceExtensionConnection::EtsUIServiceExtensionConnection(ani_vm *etsVm) : etsVm_(etsVm) {}

EtsUIServiceExtensionConnection::~EtsUIServiceExtensionConnection()
{
    if (etsConnectionObject_ == nullptr) {
        return;
    }
    ReleaseObjectReference(etsConnectionObject_);
}

void EtsUIServiceExtensionConnection::SetConnectionId(int64_t id)
{
    connectionId_ = id;
}

int64_t EtsUIServiceExtensionConnection::GetConnectionId()
{
    return connectionId_;
}

void EtsUIServiceExtensionConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnAbilityConnectDone, resultCode:%{public}d", resultCode);
    HandleOnAbilityConnectDone(element, remoteObject, resultCode);
}

void EtsUIServiceExtensionConnection::HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "resultCode:%{public}d", resultCode);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "HandleOnAbilityConnectDone called");
    if (etsVm_ == nullptr || etsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null stsConnectionRef or etsVm");
        return;
    }
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "GetEnv failed");
        return;
    }
    ani_ref refElement = AppExecFwk::WrapElementName(env, element);
    if (refElement == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null refElement");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null remoteObject");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    ani_object refRemoteObject = ANI_ohos_rpc_CreateJsRemoteObject(env, remoteObject);
    if (refRemoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null refRemoteObject");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    ani_status status = ANI_ERROR;
    ani_ref funRef;
    if ((status = env->Object_GetPropertyByName_Ref(reinterpret_cast<ani_object>(etsConnectionObject_),
        "onConnect", &funRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "get onConnect failed status : %{public}d", status);
        return;
    }
    if (!AppExecFwk::IsValidProperty(env, funRef)) {
        TAG_LOGI(AAFwkTag::UISERVC_EXT, "invalid onConnect property");
        return;
    }
    ani_ref result;
    std::vector<ani_ref> argv = { refElement, refRemoteObject};
    if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_TWO, argv.data(),
        &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to call onConnect, status: %{public}d", status);
    }
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

void EtsUIServiceExtensionConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnAbilityDisconnectDone, resultCode:%{public}d", resultCode);
    HandleOnAbilityDisconnectDone(element, resultCode);
}

void EtsUIServiceExtensionConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "HandleOnAbilityDisconnectDone, resultCode:%{public}d", resultCode);
    if (etsVm_ == nullptr || etsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null stsConnectionRef or etsVm");
        return;
    }
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "GetEnv failed");
        return;
    }
    ani_ref refElement = AppExecFwk::WrapElementName(env, element);
    if (refElement == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null refElement");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    ani_status status = ANI_ERROR;
    ani_ref funRef;
    if ((status = env->Object_GetPropertyByName_Ref(reinterpret_cast<ani_object>(etsConnectionObject_),
        "onDisconnect", &funRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "get onDisconnect failed status : %{public}d", status);
        return;
    }
    if (!AppExecFwk::IsValidProperty(env, funRef)) {
        TAG_LOGI(AAFwkTag::UISERVC_EXT, "invalid onDisconnect property");
        return;
    }
    ani_ref result;
    std::vector<ani_ref> argv = { refElement };
    if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_ONE, argv.data(),
        &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to call onDisconnect, status: %{public}d", status);
    }
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

void EtsUIServiceExtensionConnection::SetEtsConnectionObject(ani_object connObject)
{
    if (connObject == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "etsConnectionObject null");
        ReleaseObjectReference(etsConnectionObject_);
        etsConnectionObject_ = nullptr;
        return;
    }
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "etsVm_ null");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "GetEnv status:%{public}d", status);
        return;
    }
    ani_ref global = nullptr;
    if ((status = env->GlobalReference_Create(connObject, &global)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "status : %{public}d", status);
        return;
    }
    etsConnectionObject_ = global;
}

void EtsUIServiceExtensionConnection::ReleaseObjectReference(ani_ref etsObjRef)
{
    if (etsVm_ == nullptr || etsObjRef == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "etsVm_ or etsObjRef null");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "GetEnv status:%{public}d", status);
        return;
    }
    if ((status = env->GlobalReference_Delete(etsObjRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "GlobalReference_Delete status: %{public}d", status);
    }
}

void EtsUIServiceExtensionConnection::RemoveConnectionObject()
{
    ReleaseObjectReference(etsConnectionObject_);
    etsConnectionObject_ = nullptr;
}

void EtsUIServiceExtensionConnection::CallEtsFailed(int32_t errorCode)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "CallEtsFailed");
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null etsVm");
        return;
    }
    if (etsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null etsConnectionObject_");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_OK;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get env, status: %{public}d", status);
        return;
    }
    ani_ref funRef;
    if ((status = env->Object_GetPropertyByName_Ref(reinterpret_cast<ani_object>(etsConnectionObject_),
        "onFailed", &funRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "get onFailed failed status : %{public}d", status);
        return;
    }
    if (!AppExecFwk::IsValidProperty(env, funRef)) {
        TAG_LOGI(AAFwkTag::UISERVC_EXT, "invalid onFailed property");
        return;
    }
    ani_object errorCodeObj = AppExecFwk::CreateInt(env, errorCode);
    if (errorCodeObj == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null errorCodeObj");
        return;
    }
    ani_ref result;
    std::vector<ani_ref> argv = { errorCodeObj };
    if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_ONE, argv.data(),
        &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to call onFailed, status: %{public}d", status);
    }
}
} // namespace AbilityRuntime
}  // namespace OHOS
