/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "js_ui_extension_context.h"

#include <cstdint>

#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "event_handler.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "js_extension_context.h"
#include "js_error_utils.h"
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_uiservice_uiext_connection.h"
#include "js_ui_service_proxy.h"
#include "napi/native_api.h"
#include "napi_common_ability.h"
#include "napi_common_want.h"
#include "napi_common_util.h"
#include "napi_common_start_options.h"
#include "napi_remote_object.h"
#include "open_link_options.h"
#include "open_link/napi_common_open_link_options.h"
#include "start_options.h"
#include "uri.h"
#include "ui_extension_servicehost_stub_impl.h"
#include "ui_service_extension_connection_constants.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;

const std::string ATOMIC_SERVICE_PREFIX = "com.atomicservice.";
} // namespace

static std::map<UIExtensionConnectionKey, sptr<JSUIExtensionConnection>, key_compare> g_connects;
static int64_t g_serialNumber = 0;
void RemoveConnection(int64_t connectId)
{
    auto item = std::find_if(g_connects.begin(), g_connects.end(),
    [&connectId](const auto &obj) {
        return connectId == obj.first.id;
    });
    if (item != g_connects.end()) {
        TAG_LOGD(AAFwkTag::UI_EXT, "conn ability exists");
        if (item->second) {
            item->second->RemoveConnectionObject();
        }
        g_connects.erase(item);
    } else {
        TAG_LOGD(AAFwkTag::UI_EXT, "conn ability not exists");
    }
}

void FindConnection(AAFwk::Want& want, sptr<JSUIExtensionConnection>& connection, int64_t& connectId)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "Disconnect ability enter, connection:%{public}" PRId64, connectId);
    auto item = std::find_if(g_connects.begin(),
        g_connects.end(),
        [&connectId](const auto &obj) {
            return connectId == obj.first.id;
        });
    if (item != g_connects.end()) {
        // match id
        want = item->first.want;
        connection = item->second;
        TAG_LOGD(AAFwkTag::UI_EXT, "find conn ability exist");
    }
    return;
}

bool CheckConnectionParam(napi_env env, napi_value value, sptr<JSUIExtensionConnection>& connection, AAFwk::Want& want)
{
    if (!CheckTypeForNapiValue(env, value, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get connection object");
        return false;
    }
    connection->SetJsConnectionObject(value);
    UIExtensionConnectionKey key;
    key.id = g_serialNumber;
    key.want = want;
    connection->SetConnectionId(key.id);
    g_connects.emplace(key, connection);
    if (g_serialNumber < INT32_MAX) {
        g_serialNumber++;
    } else {
        g_serialNumber = 0;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "not find connection, create a new connection");
    return true;
}

void JsUIExtensionContext::Finalizer(napi_env env, void* data, void* hint)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    std::unique_ptr<JsUIExtensionContext>(static_cast<JsUIExtensionContext*>(data));
}

napi_value JsUIExtensionContext::StartAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContext, OnStartAbility);
}

napi_value JsUIExtensionContext::OpenLink(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContext, OnOpenLink);
}

napi_value JsUIExtensionContext::TerminateSelf(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContext, OnTerminateSelf);
}

napi_value JsUIExtensionContext::StartAbilityForResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContext, OnStartAbilityForResult);
}

napi_value JsUIExtensionContext::StartAbilityForResultAsCaller(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContext, OnStartAbilityForResultAsCaller);
}

napi_value JsUIExtensionContext::TerminateSelfWithResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContext, OnTerminateSelfWithResult);
}

napi_value JsUIExtensionContext::ConnectAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContext, OnConnectAbility);
}

napi_value JsUIExtensionContext::DisconnectAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContext, OnDisconnectAbility);
}

napi_value JsUIExtensionContext::ReportDrawnCompleted(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContext, OnReportDrawnCompleted);
}

napi_value JsUIExtensionContext::OpenAtomicService(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContext, OnOpenAtomicService);
}

napi_value JsUIExtensionContext::StartUIServiceExtension(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContext, OnStartUIServiceExtension);
}

napi_value JsUIExtensionContext::ConnectUIServiceExtension(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContext, OnConnectUIServiceExtension);
}

napi_value JsUIExtensionContext::DisconnectUIServiceExtension(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContext, OnDisconnectUIServiceExtension);
}

napi_value JsUIExtensionContext::OnStartAbility(napi_env env, NapiCallbackInfo& info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    size_t unwrapArgc = 0;
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    if (!CheckStartAbilityInputParam(env, info, want, startOptions, unwrapArgc)) {
        TAG_LOGD(AAFwkTag::UI_EXT, "Failed, input param type invalid");
        ThrowInvalidParamError(env, "Parse param want failed, want must be Want");
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, startOptions, unwrapArgc](napi_env env, NapiAsyncTask& task, int32_t status) {
            TAG_LOGD(AAFwkTag::UI_EXT, "startAbility begin");
            auto context = weak.lock();
            if (!context) {
                TAG_LOGE(AAFwkTag::UI_EXT, "context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }

            ErrCode innerErrorCode = (unwrapArgc == 1) ? context->StartAbility(want) :
                context->StartAbility(want, startOptions);
            if (innerErrorCode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
            }
        };

    napi_value lastParam = (info.argc == unwrapArgc) ? nullptr : info.argv[unwrapArgc];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSUIExtensionContext OnStartAbility",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

static bool CheckUrl(std::string &urlValue)
{
    if (urlValue.empty()) {
        return false;
    }
    Uri uri = Uri(urlValue);
    if (uri.GetScheme().empty() || uri.GetHost().empty()) {
        return false;
    }

    return true;
}

bool JsUIExtensionContext::CreateOpenLinkTask(const napi_env &env, const napi_value &lastParam,
    AAFwk::Want &want, int &requestCode)
{
    want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> uasyncTask =
    CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, &result);
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);
    RuntimeTask task = [env, asyncTask](int resultCode, const AAFwk::Want& want, bool isInner) {
        TAG_LOGI(AAFwkTag::UI_EXT, "OnOpenLink async callback is begin");
        HandleScope handleScope(env);
        napi_value abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            TAG_LOGW(AAFwkTag::UI_EXT, "wrap abilityResult error");
            asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
            return;
        }
        if (isInner) {
            asyncTask->Reject(env, CreateJsErrorByNativeErr(env, resultCode));
            return;
        }
        asyncTask->ResolveWithNoError(env, abilityResult);
    };
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "context is released");
        return false;
    }
    requestCode = context->GenerateCurRequestCode();
    context->InsertResultCallbackTask(requestCode, std::move(task));
    return true;
}

void JsUIExtensionContext::RemoveOpenLinkTask(int requestCode)
{
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "context is released");
        return;
    }
    context->RemoveResultCallbackTask(requestCode);
}

static bool ParseOpenLinkParams(const napi_env &env, const NapiCallbackInfo &info, std::string &linkValue,
    AAFwk::OpenLinkOptions &openLinkOptions, AAFwk::Want &want)
{
    if (info.argc != ARGC_THREE) {
        TAG_LOGE(AAFwkTag::UI_EXT, "wrong arguments num");
        return false;
    }

    if (!CheckTypeForNapiValue(env, info.argv[ARGC_ZERO], napi_string)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "link must be string");
        return false;
    }
    if (!ConvertFromJsValue(env, info.argv[ARGC_ZERO], linkValue) || !CheckUrl(linkValue)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "link parameter invalid");
        return false;
    }

    if (CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        TAG_LOGD(AAFwkTag::UI_EXT, "OpenLinkOptions is used");
        if (!AppExecFwk::UnwrapOpenLinkOptions(env, info.argv[INDEX_ONE], openLinkOptions, want)) {
            TAG_LOGE(AAFwkTag::UI_EXT, "openLinkOptions parse failed");
            return false;
        }
    }

    return true;
}

napi_value JsUIExtensionContext::OnOpenLink(napi_env env, NapiCallbackInfo& info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "OnOpenLink");

    std::string linkValue("");
    AAFwk::OpenLinkOptions openLinkOptions;
    AAFwk::Want want;
    want.SetParam(AppExecFwk::APP_LINKING_ONLY, false);

    if (!ParseOpenLinkParams(env, info, linkValue, openLinkOptions, want)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "parse openLink arguments failed");
        ThrowInvalidParamError(env,
            "Parse param link or openLinkOptions failed, link must be string, openLinkOptions must be options.");
        return CreateJsUndefined(env);
    }

    want.SetUri(linkValue);
    std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count());
    want.SetParam(Want::PARAM_RESV_START_TIME, startTime);

    int requestCode = -1;
    if (CheckTypeForNapiValue(env, info.argv[INDEX_TWO], napi_function)) {
        TAG_LOGD(AAFwkTag::UI_EXT, "completionHandler is used");
        CreateOpenLinkTask(env, info.argv[INDEX_TWO], want, requestCode);
    }
    return OnOpenLinkInner(env, want, requestCode, startTime, linkValue);
}

napi_value JsUIExtensionContext::OnOpenLinkInner(napi_env env, const AAFwk::Want& want,
    int requestCode, const std::string& startTime, const std::string& url)
{
    auto innerErrorCode = std::make_shared<int>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, want, innerErrorCode, requestCode]() {
        auto context = weak.lock();
        if (!context) {
            TAG_LOGW(AAFwkTag::UI_EXT, "context is released");
            *innerErrorCode = static_cast<int>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }
        *innerErrorCode = context->OpenLink(want, requestCode);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrorCode, requestCode, startTime, url, this](
        napi_env env, NapiAsyncTask& task, int32_t status) {
        if (*innerErrorCode == 0) {
            TAG_LOGI(AAFwkTag::UI_EXT, "OpenLink succeeded");
            return;
        }
        if (freeInstallObserver_ == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "freeInstallObserver_ is nullptr");
            RemoveOpenLinkTask(requestCode);
            return;
        }
        if (*innerErrorCode == AAFwk::ERR_OPEN_LINK_START_ABILITY_DEFAULT_OK) {
            TAG_LOGI(AAFwkTag::UI_EXT, "start ability by default succeeded");
            freeInstallObserver_->OnInstallFinishedByUrl(startTime, url, ERR_OK);
            return;
        }
        TAG_LOGI(AAFwkTag::UI_EXT, "OpenLink failed");
        freeInstallObserver_->OnInstallFinishedByUrl(startTime, url, *innerErrorCode);
        RemoveOpenLinkTask(requestCode);
    };

    napi_value result = nullptr;
    AddFreeInstallObserver(env, want, nullptr, &result, false, true);
    NapiAsyncTask::ScheduleHighQos("JsUIExtensionContext::OnOpenLink", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), nullptr));

    return result;
}

napi_value JsUIExtensionContext::OnTerminateSelf(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "called");
    NapiAsyncTask::CompleteCallback complete =
        [weak = context_](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                TAG_LOGE(AAFwkTag::UI_EXT, "context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }

            ErrCode innerErrorCode = context->TerminateSelf();
            if (innerErrorCode == 0) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
            }
        };

    napi_value lastParam = (info.argc == ARGC_ZERO) ? nullptr : info.argv[INDEX_ZERO];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSUIExtensionContext OnTerminateSelf",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsUIExtensionContext::OnStartAbilityForResult(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (info.argc == ARGC_ZERO) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    size_t unwrapArgc = 0;
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    if (!CheckStartAbilityInputParam(env, info, want, startOptions, unwrapArgc)) {
        ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        TAG_LOGD(AAFwkTag::UI_EXT, "input param type invalid");
        return CreateJsUndefined(env);
    }
    napi_value lastParam = info.argc > unwrapArgc ? info.argv[unwrapArgc] : nullptr;
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> uasyncTask = CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, &result);
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);
    RuntimeTask task = [env, asyncTask](int resultCode, const AAFwk::Want &want, bool isInner) {
        TAG_LOGI(AAFwkTag::UI_EXT, "called");
        napi_value abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            TAG_LOGW(AAFwkTag::UI_EXT, "wrap abilityResult failed");
            asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
            return;
        }
        if (isInner) {
            asyncTask->Reject(env, CreateJsErrorByNativeErr(env, resultCode));
            return;
        }
        asyncTask->Resolve(env, abilityResult);
    };
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "context is released");
        asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
        return result;
    }
    want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
    int curRequestCode = context->GenerateCurRequestCode();
    (unwrapArgc == INDEX_ONE) ? context->StartAbilityForResult(want, curRequestCode, std::move(task))
                              : context->StartAbilityForResult(want, startOptions, curRequestCode, std::move(task));
    TAG_LOGD(AAFwkTag::UI_EXT, "OnStartAbilityForResult end");
    return result;
}

napi_value JsUIExtensionContext::OnTerminateSelfWithResult(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "called");
    if (info.argc == 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    int32_t resultCode = 0;
    AAFwk::Want want;
    if (!AppExecFwk::UnWrapAbilityResult(env, info.argv[INDEX_ZERO], resultCode, want)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "OnTerminateSelfWithResult Failed to parse ability result");
        ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete;
    SetCallbackForTerminateWithResult(resultCode, want, complete);
    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsUIExtensionContext::OnTerminateSelfWithResult",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
    return result;
}

napi_value JsUIExtensionContext::OnStartAbilityForResultAsCaller(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (info.argc == ARGC_ZERO) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    size_t unwrapArgc = 0;
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    if (!CheckStartAbilityInputParam(env, info, want, startOptions, unwrapArgc)) {
        ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        TAG_LOGD(AAFwkTag::UI_EXT, "Input param type invalid");
        return CreateJsUndefined(env);
    }
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> uasyncTask = CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &result);
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);
    RuntimeTask task = [env, asyncTask](int resultCode, const AAFwk::Want &want, bool isInner) {
        TAG_LOGI(AAFwkTag::UI_EXT, "called");
        napi_value abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            TAG_LOGW(AAFwkTag::UI_EXT, "Wrap abilityResult failed");
            asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
            return;
        }
        if (isInner) {
            asyncTask->Reject(env, CreateJsErrorByNativeErr(env, resultCode));
            return;
        }
        asyncTask->Resolve(env, abilityResult);
    };
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "The context is released");
        asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
        return result;
    }
    want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
    int curRequestCode = context->GenerateCurRequestCode();
    unwrapArgc == INDEX_ONE ?
        context->StartAbilityForResultAsCaller(want, curRequestCode, std::move(task)) :
        context->StartAbilityForResultAsCaller(want, startOptions, curRequestCode, std::move(task));
    TAG_LOGD(AAFwkTag::UI_EXT, "End.");
    return result;
}

napi_value JsUIExtensionContext::OnConnectAbility(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    // Check params count
    if (info.argc < ARGC_TWO) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    // Unwrap want and connection
    sptr<JSUIExtensionConnection> connection = new (std::nothrow) JSUIExtensionConnection(env);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "make conn failed");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, info.argv[0], want) ||
        !CheckConnectionParam(env, info.argv[1], connection, want)) {
        ThrowInvalidParamError(env,
            "Parse param want or connection failed, want must be Want and connection must be Connection.");
        return CreateJsUndefined(env);
    }
    int64_t connectId = connection->GetConnectionId();
    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, connection, connectId](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                TAG_LOGE(AAFwkTag::UI_EXT, "context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                RemoveConnection(connectId);
                return;
            }
            TAG_LOGD(AAFwkTag::UI_EXT, "ConnectAbility connection:%{public}d", static_cast<int32_t>(connectId));
            auto innerErrorCode = context->ConnectAbility(want, connection);
            int32_t errcode = static_cast<int32_t>(AbilityRuntime::GetJsErrorCodeByNativeError(innerErrorCode));
            if (errcode) {
                connection->CallJsFailed(errcode);
                RemoveConnection(connectId);
            }
            task.Resolve(env, CreateJsUndefined(env));
        };
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSUIExtensionConnection::OnConnectAbility",
        env, CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
    return CreateJsValue(env, connectId);
}

napi_value JsUIExtensionContext::OnDisconnectAbility(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "start");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    int64_t connectId = -1;
    if (!AppExecFwk::UnwrapInt64FromJS2(env, info.argv[INDEX_ZERO], connectId)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid connectId");
        ThrowInvalidParamError(env, "Parse param connectId failed, connectId must be number");
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    sptr<JSUIExtensionConnection> connection = nullptr;
    FindConnection(want, connection, connectId);
    // begin disconnect
    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, connection](
            napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                TAG_LOGW(AAFwkTag::UI_EXT, "context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            if (connection == nullptr) {
                TAG_LOGW(AAFwkTag::UI_EXT, "connection nullptr");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            TAG_LOGD(AAFwkTag::UI_EXT, "context->DisconnectAbility");
            auto innerErrorCode = context->DisconnectAbility(want, connection);
            if (innerErrorCode == 0) {
                task.Resolve(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
            }
        };

    napi_value lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSUIExtensionConnection::OnDisconnectAbility",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsUIExtensionContext::OnStartUIServiceExtension(napi_env env, NapiCallbackInfo& info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "OnStartUIServiceExtension is called");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    size_t unwrapArgc = 0;
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    if (!CheckStartAbilityInputParam(env, info, want, startOptions, unwrapArgc)) {
        TAG_LOGD(AAFwkTag::UI_EXT, "Failed, input param type invalid");
        ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, startOptions, unwrapArgc](napi_env env, NapiAsyncTask& task, int32_t status) {
            TAG_LOGD(AAFwkTag::UI_EXT, "StartUIServiceExtension begin");
            auto context = weak.lock();
            if (!context) {
                TAG_LOGE(AAFwkTag::UI_EXT, "context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            ErrCode innerErrorCode = ERR_OK;
            innerErrorCode = context->StartUIServiceExtension(want);
            if (innerErrorCode == 0) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
            }
        };

    napi_value lastParam = (info.argc == unwrapArgc) ? nullptr : info.argv[unwrapArgc];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSUIExtensionContext OnStartUIServiceExtension",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

bool JsUIExtensionContext::UnwrapConnectUIServiceExtensionParam(napi_env env, NapiCallbackInfo& info, AAFwk::Want& want)
{
    if (info.argc < ARGC_TWO) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "invalid argc");
        ThrowTooFewParametersError(env);
        return false;
    }
    bool unwrapResult = OHOS::AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want);
    if (!unwrapResult) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "failed, UnwrapWant failed");
        ThrowInvalidParamError(env, "parse want error");
        return false;
    }
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "callee:%{public}s.%{public}s", want.GetBundle().c_str(),
        want.GetElement().GetAbilityName().c_str());
    if (!CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "failed, callback type incorrect");
        ThrowInvalidParamError(env, "Incorrect parameter types");
        return false;
    }
    return true;
}

bool JsUIExtensionContext::CheckConnectAlreadyExist(napi_env env, AAFwk::Want& want, napi_value callback,
    napi_value& result)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "called");
    sptr<JSUIServiceUIExtConnection> connection = nullptr;
    UIServiceConnection::FindUIServiceExtensionConnection(env, want, callback, connection);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "connection == nullptr");
        return false;
    }

    std::unique_ptr<NapiAsyncTask> uasyncTask = CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &result);
    napi_value proxy = connection->GetProxyObject();
    if (proxy == nullptr) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "can't got proxy object, wait for duplicated connect finish");
        connection->AddDuplicatedPendingTask(uasyncTask);
    } else {
        TAG_LOGI(AAFwkTag::UISERVC_EXT, "Resolve, got proxy object");
        uasyncTask->ResolveWithNoError(env, proxy);
    }
    return true;
}

napi_value JsUIExtensionContext::OnConnectUIServiceExtension(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "called");
    AAFwk::Want want;
    bool unwrapResult = UnwrapConnectUIServiceExtensionParam(env, info, want);
    if (!unwrapResult) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "UnwrapConnectUIServiceExtensionParam failed");
        return CreateJsUndefined(env);
    }
    napi_value callbackObject = nullptr;
    if (info.argc > ARGC_ONE) {
        callbackObject = info.argv[INDEX_ONE];
    }
    napi_value result = nullptr;
    bool duplicated = CheckConnectAlreadyExist(env, want, callbackObject, result);
    if (duplicated) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "duplicated");
        return result;
    }

    sptr<JSUIServiceUIExtConnection> connection = sptr<JSUIServiceUIExtConnection>::MakeSptr(env);
    sptr<UIExtensionServiceHostStubImpl> stub = connection->GetServiceHostStub();
    want.SetParam(UISERVICEHOSTPROXY_KEY, stub->AsObject());

    result = nullptr;
    std::unique_ptr<NapiAsyncTask> uasyncTask = CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &result);
    std::shared_ptr<NapiAsyncTask> uasyncTaskShared = std::move(uasyncTask);
    if (info.argc > ARGC_ONE) {
        connection->SetJsConnectionObject(callbackObject);
    }
    connection->SetNapiAsyncTask(uasyncTaskShared);
    UIServiceConnection::AddUIServiceExtensionConnection(want, connection);
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [weak = context_, want, uasyncTaskShared, connection](
            napi_env env, NapiAsyncTask& taskUseless, int32_t status) {
            DoConnectUIServiceExtension(env, weak, connection, uasyncTaskShared, want);
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsUIExtensionContext::OnConnectUIServiceExtension",
        env, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
    return result;
}

void JsUIExtensionContext::DoConnectUIServiceExtension(napi_env env,
    std::weak_ptr<UIExtensionContext> weakContext, sptr<JSUIServiceUIExtConnection> connection,
    std::shared_ptr<NapiAsyncTask> uasyncTaskShared, const AAFwk::Want& want)
{
    if (uasyncTaskShared == nullptr) {
        return;
    }

    uint64_t connectId = connection->GetConnectionId();
    auto context = weakContext.lock();
    if (!context) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Connect ability failed, context is released.");
        uasyncTaskShared->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
        UIServiceConnection::RemoveUIServiceExtensionConnection(connectId);
        return;
    }

    auto innerErrorCode = context->ConnectAbility(want, connection);
    AbilityErrorCode errcode = AbilityRuntime::GetJsErrorCodeByNativeError(innerErrorCode);
    if (errcode != AbilityErrorCode::ERROR_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "ConnectAbility failed, errcode is %{public}d.", errcode);
        uasyncTaskShared->Reject(env, CreateJsError(env, errcode));
        UIServiceConnection::RemoveUIServiceExtensionConnection(connectId);
    }
}

napi_value JsUIExtensionContext::OnDisconnectUIServiceExtension(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    AAFwk::JsUIServiceProxy* proxy = nullptr;
    napi_status status = napi_unwrap(env, info.argv[INDEX_ZERO], reinterpret_cast<void**>(&proxy));
    if (status != napi_ok || proxy == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "napi_unwrap err or proxy == nullptr");
        ThrowInvalidParamError(env, "Parameter verification failed");
        return CreateJsUndefined(env);
    }

    int64_t connectId = proxy->GetConnectionId();
    AAFwk::Want want;
    sptr<JSUIServiceUIExtConnection> connection = nullptr;
    UIServiceConnection::FindUIServiceExtensionConnection(connectId, want, connection);

    TAG_LOGI(AAFwkTag::UISERVC_EXT, "connection:%{public}d.", static_cast<int32_t>(connectId));
    NapiAsyncTask::CompleteCallback complete =
        [weak = context_, want, connectId, connection](
            napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                TAG_LOGW(AAFwkTag::UISERVC_EXT, "OnDisconnectUIServiceExtension context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                UIServiceConnection::RemoveUIServiceExtensionConnection(connectId);
            } else if (!connection) {
                TAG_LOGW(AAFwkTag::UISERVC_EXT, "connection nullptr");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                UIServiceConnection::RemoveUIServiceExtensionConnection(connectId);
            } else {
                TAG_LOGI(AAFwkTag::UISERVC_EXT, "context->DisconnectAbility");
                context->DisconnectAbility(want, connection);
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            }
        };

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsUIExtensionContext::OnDisconnectUIServiceExtension",
        env, CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsUIExtensionContext::OnReportDrawnCompleted(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, innerErrorCode]() {
        auto context = weak.lock();
        if (!context) {
            TAG_LOGW(AAFwkTag::UI_EXT, "context is released");
            *innerErrorCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }
        *innerErrorCode = context->ReportDrawnCompleted();
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrorCode](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (*innerErrorCode == ERR_OK) {
            task.Resolve(env, CreateJsUndefined(env));
        } else {
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
        }
    };

    napi_value lastParam = info.argv[INDEX_ZERO];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsUIExtensionContext::OnReportDrawnCompleted",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsUIExtensionContext::OnOpenAtomicService(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "start");
    if (info.argc == ARGC_ZERO) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string appId;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], appId)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "OnOpenAtomicService, parse appId failed");
        ThrowInvalidParamError(env, "Parse param appId failed, appId must be string.");
        return CreateJsUndefined(env);
    }

    decltype(info.argc) unwrapArgc = ARGC_ONE;
    Want want;
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        TAG_LOGD(AAFwkTag::UI_EXT, "OnOpenAtomicService atomic service options is used");
        if (!AppExecFwk::UnwrapStartOptionsAndWant(env, info.argv[INDEX_ONE], startOptions, want)) {
            TAG_LOGE(AAFwkTag::UI_EXT, "Fail to parse atomic service options");
            ThrowInvalidParamError(env,
                "Parse param startOptions failed, startOptions must be StartOption.");
            return CreateJsUndefined(env);
        }
        unwrapArgc++;
    }

    std::string bundleName = ATOMIC_SERVICE_PREFIX + appId;
    TAG_LOGD(AAFwkTag::UI_EXT, "bundleName: %{public}s", bundleName.c_str());
    want.SetBundle(bundleName);
    return OpenAtomicServiceInner(env, info, want, startOptions, unwrapArgc);
}

void JsUIExtensionContext::SetCallbackForTerminateWithResult(int32_t resultCode, AAFwk::Want& want,
    NapiAsyncTask::CompleteCallback& complete)
{
    complete =
        [weak = context_, want, resultCode](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                TAG_LOGE(AAFwkTag::UI_EXT, "context is released");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            auto extensionContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::UIExtensionContext>(context);
            if (!extensionContext) {
                TAG_LOGE(AAFwkTag::UI_EXT, "extensionContext is nullptr");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
                return;
            }
            auto token = extensionContext->GetToken();
            AAFwk::AbilityManagerClient::GetInstance()->TransferAbilityResultForExtension(token, resultCode, want);
#ifdef SUPPORT_SCREEN
            sptr<Rosen::Window> uiWindow = context->GetWindow();
            if (!uiWindow) {
                TAG_LOGE(AAFwkTag::UI_EXT, "uiWindow is nullptr");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
                return;
            }
            auto ret = uiWindow->TransferAbilityResult(resultCode, want);
            if (ret != Rosen::WMError::WM_OK) {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
                return;
            }
#endif // SUPPORT_SCREEN
            auto errorCode = context->TerminateSelf();
            if (errorCode == 0) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, errorCode));
            }
        };
}

napi_value JsUIExtensionContext::OpenAtomicServiceInner(napi_env env, NapiCallbackInfo& info, Want &want,
    const AAFwk::StartOptions &options, size_t unwrapArgc)
{
    want.AddFlags(Want::FLAG_INSTALL_ON_DEMAND);
    std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count());
    want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
    napi_value result = nullptr;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "context is released");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return CreateJsUndefined(env);
    }
    AddFreeInstallObserver(env, want, nullptr, &result, true);
    RuntimeTask task = [env, element = want.GetElement(), startTime, &observer = freeInstallObserver_](
        int resultCode, const AAFwk::Want& want, bool isInner) {
        TAG_LOGD(AAFwkTag::UI_EXT, "OnOpenAtomicService async callback is begin");
        if (observer == nullptr) {
            TAG_LOGW(AAFwkTag::UI_EXT, "observer is nullptr");
            return;
        }
        HandleScope handleScope(env);
        std::string bundleName = element.GetBundleName();
        std::string abilityName = element.GetAbilityName();
        napi_value abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            TAG_LOGW(AAFwkTag::UI_EXT, "wrap abilityResult failed");
            isInner = true;
            resultCode = ERR_INVALID_VALUE;
        }
        if (isInner) {
            observer->OnInstallFinished(bundleName, abilityName, startTime, resultCode);
        } else {
            observer->OnInstallFinished(bundleName, abilityName, startTime, abilityResult);
        }
    };
    want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
    auto curRequestCode = context->GenerateCurRequestCode();
    context->OpenAtomicService(want, options, curRequestCode, std::move(task));
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
    return result;
}

void JsUIExtensionContext::AddFreeInstallObserver(napi_env env, const AAFwk::Want &want, napi_value callback,
    napi_value *result, bool isAbilityResult, bool isOpenLink)
{
    // adapter free install async return install and start result
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    int ret = 0;
    if (freeInstallObserver_ == nullptr) {
        freeInstallObserver_ = new JsFreeInstallObserver(env);
        auto context = context_.lock();
        if (!context) {
            TAG_LOGE(AAFwkTag::CONTEXT, "context is nullptr.");
            return;
        }
        ret = context->AddFreeInstallObserver(freeInstallObserver_);
    }

    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "AddFreeInstallObserver error");
    }
    std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
    if (!isOpenLink) {
        TAG_LOGI(AAFwkTag::UI_EXT, "AddJsObserverObject");
        std::string bundleName = want.GetElement().GetBundleName();
        std::string abilityName = want.GetElement().GetAbilityName();
        freeInstallObserver_->AddJsObserverObject(
            bundleName, abilityName, startTime, callback, result, isAbilityResult);
    }
    std::string url = want.GetUriString();
    freeInstallObserver_->AddJsObserverObject(startTime, url, callback, result, isAbilityResult);
}

napi_value JsUIExtensionContext::CreateJsUIExtensionContext(napi_env env,
    std::shared_ptr<UIExtensionContext> context)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context) {
        abilityInfo = context->GetAbilityInfo();
    }
    napi_value objValue = CreateJsExtensionContext(env, context, abilityInfo);

    std::unique_ptr<JsUIExtensionContext> jsContext = std::make_unique<JsUIExtensionContext>(context);
    napi_wrap(env, objValue, jsContext.release(), Finalizer, nullptr, nullptr);

    const char *moduleName = "JsUIExtensionContext";
    BindNativeFunction(env, objValue, "startAbility", moduleName, StartAbility);
    BindNativeFunction(env, objValue, "openLink", moduleName, OpenLink);
    BindNativeFunction(env, objValue, "terminateSelf", moduleName, TerminateSelf);
    BindNativeFunction(env, objValue, "startAbilityForResult", moduleName, StartAbilityForResult);
    BindNativeFunction(env, objValue, "terminateSelfWithResult", moduleName, TerminateSelfWithResult);
    BindNativeFunction(env, objValue, "startAbilityForResultAsCaller", moduleName, StartAbilityForResultAsCaller);
    BindNativeFunction(env, objValue, "connectServiceExtensionAbility", moduleName, ConnectAbility);
    BindNativeFunction(env, objValue, "disconnectServiceExtensionAbility", moduleName, DisconnectAbility);
    BindNativeFunction(env, objValue, "reportDrawnCompleted", moduleName, ReportDrawnCompleted);
    BindNativeFunction(env, objValue, "openAtomicService", moduleName, OpenAtomicService);
    BindNativeFunction(env, objValue, "startUIServiceExtensionAbility", moduleName, StartUIServiceExtension);
    BindNativeFunction(env, objValue, "connectUIServiceExtensionAbility", moduleName, ConnectUIServiceExtension);
    BindNativeFunction(env, objValue, "disconnectUIServiceExtensionAbility", moduleName, DisconnectUIServiceExtension);

    return objValue;
}

bool JsUIExtensionContext::CheckStartAbilityInputParam(napi_env env, NapiCallbackInfo& info,
    AAFwk::Want& want, AAFwk::StartOptions& startOptions, size_t& unwrapArgc) const
{
    if (info.argc < ARGC_ONE) {
        return false;
    }
    unwrapArgc = ARGC_ZERO;
    // Check input want
    if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
        return false;
    }
    if (!want.HasParameter(Want::PARAM_BACK_TO_OTHER_MISSION_STACK)) {
        want.SetParam(Want::PARAM_BACK_TO_OTHER_MISSION_STACK, true);
    }
    ++unwrapArgc;
    if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[1], napi_object)) {
        AppExecFwk::UnwrapStartOptions(env, info.argv[1], startOptions);
        unwrapArgc++;
    }
    return true;
}

JSUIExtensionConnection::JSUIExtensionConnection(napi_env env) : env_(env) {}

JSUIExtensionConnection::~JSUIExtensionConnection()
{
    ReleaseNativeReference(jsConnectionObject_.release());
}

void JSUIExtensionConnection::ReleaseNativeReference(NativeReference* ref)
{
    if (ref == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "ReleaseNativeReference: ref == nullptr");
        return;
    }
    uv_loop_t *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "ReleaseNativeReference: failed to get uv loop.");
        delete ref;
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "ReleaseNativeReference: failed to create work.");
        delete ref;
        return;
    }
    work->data = reinterpret_cast<void *>(ref);
    int ret = uv_queue_work(loop, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
        if (work == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "ReleaseNativeReference: work is nullptr.");
            return;
        }
        if (work->data == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "ReleaseNativeReference: data is nullptr.");
            delete work;
            work = nullptr;
            return;
        }
        NativeReference *refPtr = reinterpret_cast<NativeReference *>(work->data);
        delete refPtr;
        refPtr = nullptr;
        delete work;
        work = nullptr;
    });
    if (ret != 0) {
        delete ref;
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
}

void JSUIExtensionConnection::SetConnectionId(int64_t id)
{
    connectionId_ = id;
}

int64_t JSUIExtensionConnection::GetConnectionId()
{
    return connectionId_;
}

void JSUIExtensionConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "resultCode:%{public}d", resultCode);
    wptr<JSUIExtensionConnection> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([connection, element, remoteObject, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSUIExtensionConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "connectionSptr nullptr");
                return;
            }
            connectionSptr->HandleOnAbilityConnectDone(element, remoteObject, resultCode);
        });

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSUIExtensionConnection::OnAbilityConnectDone",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSUIExtensionConnection::HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "start, resultCode:%{public}d", resultCode);
    // wrap ElementName
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env_, element);

    // wrap RemoteObject
    napi_value napiRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(env_, remoteObject);
    napi_value argv[] = {napiElementName, napiRemoteObject};
    if (jsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "jsConnectionObject_ null");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get object");
        return;
    }
    napi_value methodOnConnect = nullptr;
    napi_get_named_property(env_, obj, "onConnect", &methodOnConnect);
    if (methodOnConnect == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get onConnect from object");
        return;
    }
    napi_call_function(env_, obj, methodOnConnect, ARGC_TWO, argv, nullptr);
}

void JSUIExtensionConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "resultCode:%{public}d", resultCode);
    wptr<JSUIExtensionConnection> connection = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([connection, element, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSUIExtensionConnection> connectionSptr = connection.promote();
            if (!connectionSptr) {
                TAG_LOGI(AAFwkTag::UI_EXT, "connectionSptr nullptr");
                return;
            }
            connectionSptr->HandleOnAbilityDisconnectDone(element, resultCode);
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JSUIExtensionConnection::OnAbilityDisconnectDone",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JSUIExtensionConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "resultCode:%{public}d", resultCode);
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env_, element);
    napi_value argv[] = {napiElementName};
    if (jsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "jsConnectionObject_ nullptr");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get object");
        return;
    }
    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onDisconnect", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get onDisconnect from object");
        return;
    }

    // release connect
    RemoveConnection(connectionId_);
    napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
}

void JSUIExtensionConnection::SetJsConnectionObject(napi_value jsConnectionObject)
{
    napi_ref value = nullptr;
    napi_create_reference(env_, jsConnectionObject, 1, &value);
    jsConnectionObject_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(value));
}

void JSUIExtensionConnection::RemoveConnectionObject()
{
    jsConnectionObject_.reset();
}

void JSUIExtensionConnection::CallJsFailed(int32_t errorCode)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "CallJsFailed enter");
    if (jsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "jsConnectionObject_ nullptr");
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "wrong to get object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onFailed", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get onFailed from object");
        return;
    }
    napi_value argv[] = { CreateJsValue(env_, errorCode) };
    napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
    TAG_LOGD(AAFwkTag::UI_EXT, "CallJsFailed end");
}

napi_value JSUIExtensionConnection::CallObjectMethod(const char* name, napi_value const *argv, size_t argc)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "name:%{public}s", name);
    if (!jsConnectionObject_) {
        TAG_LOGW(AAFwkTag::CONTEXT, "Not found jsConnectionObject_");
        return nullptr;
    }

    HandleScope handleScope(env_);
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to get jsConnectionObject_ object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, name, &method);
    if (!CheckTypeForNapiValue(env_, method, napi_function)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Failed to get '%{public}s' from jsConnectionObject_ object", name);
        return nullptr;
    }
    napi_value result = nullptr;
    napi_call_function(env_, obj, method, argc, argv, &result);
    TAG_LOGD(AAFwkTag::CONTEXT, "CallFunction(%{public}s) ok", name);
    return result;
}

}  // namespace AbilityRuntime
}  // namespace OHOS
