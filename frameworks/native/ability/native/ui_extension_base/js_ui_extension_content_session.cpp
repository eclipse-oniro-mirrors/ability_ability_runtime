/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "js_ui_extension_content_session.h"

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "event_handler.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ipc_skeleton.h"
#include "js_error_utils.h"
#include "js_extension_window.h"
#include "js_runtime_utils.h"
#include "js_ui_extension_callback.h"
#include "js_ui_extension_context.h"
#include "napi_common_start_options.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi/native_api.h"
#include "native_engine.h"
#include "native_value.h"
#include "string_wrapper.h"
#include "tokenid_kit.h"
#ifdef SUPPORT_SCREEN
#include "ui_content.h"
#endif // SUPPORT_SCREEN
#include "want.h"
#include "window.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr int32_t INDEX_THREE = 3;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_THREE = 3;
constexpr const char* PERMISSION_PRIVACY_WINDOW = "ohos.permission.PRIVACY_WINDOW";
const std::string UIEXTENSION_TARGET_TYPE_KEY = "ability.want.params.uiExtensionTargetType";
const std::string FLAG_AUTH_READ_URI_PERMISSION = "ability.want.params.uriPermissionFlag";
} // namespace

#define CHECK_IS_SYSTEM_APP                                                             \
do {                                                                                    \
    auto selfToken = IPCSkeleton::GetSelfTokenID();                                     \
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {      \
        TAG_LOGE(AAFwkTag::UI_EXT, "This application is not system-app,"                \
                "can not use system-api");                                              \
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);                   \
        return CreateJsUndefined(env);                                                  \
    }                                                                                   \
} while (0)

void AbilityResultListeners::AddListener(const uint64_t &uiExtensionComponentId,
    std::shared_ptr<AbilityResultListener> listener)
{
    if (uiExtensionComponentId == 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid session");
        return;
    }
    listeners_[uiExtensionComponentId] = listener;
}

void AbilityResultListeners::RemoveListener(const uint64_t &uiExtensionComponentId)
{
    if (listeners_.find(uiExtensionComponentId) != listeners_.end()) {
        listeners_.erase(uiExtensionComponentId);
    }
}

void AbilityResultListeners::OnAbilityResult(int requestCode, int resultCode, const Want &resultData)
{
    for (auto &item : listeners_) {
        if (item.second && item.second->IsMatch(requestCode)) {
            item.second->OnAbilityResult(requestCode, resultCode, resultData);
            return;
        }
    }
}

void UISessionAbilityResultListener::OnAbilityResult(int requestCode, int resultCode, const Want &resultData)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    auto callback = resultCallbacks_.find(requestCode);
    if (callback != resultCallbacks_.end()) {
        if (callback->second) {
            callback->second(resultCode, resultData, false);
        }
        resultCallbacks_.erase(requestCode);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

bool UISessionAbilityResultListener::IsMatch(int requestCode)
{
    return resultCallbacks_.find(requestCode) != resultCallbacks_.end();
}

void UISessionAbilityResultListener::OnAbilityResultInner(int requestCode, int resultCode, const Want &resultData)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    auto callback = resultCallbacks_.find(requestCode);
    if (callback != resultCallbacks_.end()) {
        if (callback->second) {
            callback->second(resultCode, resultData, true);
        }
        resultCallbacks_.erase(requestCode);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

void UISessionAbilityResultListener::SaveResultCallbacks(int requestCode, RuntimeTask&& task)
{
    resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
}

JsUIExtensionContentSession::~JsUIExtensionContentSession()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "~JsUIExtensionContentSession");
}

JsUIExtensionContentSession::JsUIExtensionContentSession(
    sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow,
    std::weak_ptr<AbilityRuntime::Context> &context,
    std::shared_ptr<AbilityResultListeners>& abilityResultListeners)
    : sessionInfo_(sessionInfo), uiWindow_(uiWindow), context_(context)
{
    listener_ = std::make_shared<UISessionAbilityResultListener>();
    if (abilityResultListeners == nullptr || sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null params");
    } else {
        abilityResultListeners->AddListener(sessionInfo->uiExtensionComponentId, listener_);
    }
}

JsUIExtensionContentSession::JsUIExtensionContentSession(
    sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow)
    : sessionInfo_(sessionInfo), uiWindow_(uiWindow) {}

void JsUIExtensionContentSession::Finalizer(napi_env env, void* data, void* hint)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    std::unique_ptr<JsUIExtensionContentSession>(static_cast<JsUIExtensionContentSession*>(data));
}

napi_value JsUIExtensionContentSession::StartAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnStartAbility);
}

napi_value JsUIExtensionContentSession::StartAbilityAsCaller(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnStartAbilityAsCaller);
}

napi_value JsUIExtensionContentSession::GetUIExtensionHostWindowProxy(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnGetUIExtensionHostWindowProxy);
}

napi_value JsUIExtensionContentSession::GetUIExtensionWindowProxy(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnGetUIExtensionWindowProxy);
}

napi_value JsUIExtensionContentSession::StartAbilityForResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnStartAbilityForResult);
}

napi_value JsUIExtensionContentSession::TerminateSelf(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnTerminateSelf);
}

napi_value JsUIExtensionContentSession::TerminateSelfWithResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnTerminateSelfWithResult);
}

napi_value JsUIExtensionContentSession::SendData(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnSendData);
}

napi_value JsUIExtensionContentSession::SetReceiveDataCallback(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnSetReceiveDataCallback);
}

napi_value JsUIExtensionContentSession::SetReceiveDataForResultCallback(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnSetReceiveDataForResultCallback);
}

napi_value JsUIExtensionContentSession::LoadContent(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnLoadContent);
}

napi_value JsUIExtensionContentSession::LoadContentByName(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnLoadContentByName);
}

napi_value JsUIExtensionContentSession::SetWindowBackgroundColor(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnSetWindowBackgroundColor);
}

napi_value JsUIExtensionContentSession::SetWindowPrivacyMode(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnSetWindowPrivacyMode);
}

napi_value JsUIExtensionContentSession::StartAbilityByType(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIExtensionContentSession, OnStartAbilityByType);
}

napi_value JsUIExtensionContentSession::OnStartAbility(napi_env env, NapiCallbackInfo& info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    CHECK_IS_SYSTEM_APP;

    if (info.argc == ARGC_ZERO) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    size_t unwrapArgc = 1;
    if (!OHOS::AppExecFwk::UnwrapWant(env, info.argv[0], want)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "parse want failed");
        ThrowInvalidParamError(env, "Parameter error: Failed to parse want! Want must be a Want.");
        return CreateJsUndefined(env);
    }
    if (!want.HasParameter(Want::PARAM_BACK_TO_OTHER_MISSION_STACK)) {
        want.SetParam(Want::PARAM_BACK_TO_OTHER_MISSION_STACK, true);
    }
    TAG_LOGI(AAFwkTag::UI_EXT, "StartAbility:%{public}s", want.GetElement().GetAbilityName().c_str());
    auto innerErrorCode = std::make_shared<int>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = StartAbilityExecuteCallback(
        want, unwrapArgc, env, info, innerErrorCode);

    NapiAsyncTask::CompleteCallback complete = [innerErrorCode](napi_env env, NapiAsyncTask& task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrorCode == 0) {
            task.ResolveWithNoError(env, CreateJsUndefined(env));
        } else {
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
        }
    };

    HandleEscape handleEscape(env);
    napi_value lastParam = (info.argc > unwrapArgc) ? info.argv[unwrapArgc] : nullptr;
    napi_value result = nullptr;
    if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        AddFreeInstallObserver(env, want, lastParam, &result);
        NapiAsyncTask::Schedule("JsUIExtensionContentSession::OnStartAbility", env,
            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), nullptr, nullptr));
    } else {
        NapiAsyncTask::Schedule("JsUIExtensionContentSession::OnStartAbility", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
    return handleEscape.Escape(result);
}

napi_value JsUIExtensionContentSession::OnGetUIExtensionHostWindowProxy(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    CHECK_IS_SYSTEM_APP;
    if (sessionInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null sessionInfo");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    HandleEscape handleEscape(env);
    napi_value jsExtensionWindow =
        Rosen::JsExtensionWindow::CreateJsExtensionWindow(env, uiWindow_, sessionInfo_->hostWindowId);
    if (jsExtensionWindow == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null jsExtensionWindow");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }
    auto value = JsRuntime::LoadSystemModuleByEngine(env, "application.extensionWindow", &jsExtensionWindow, 1);
    if (value == nullptr) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }
    return handleEscape.Escape(value->GetNapiValue());
}

napi_value JsUIExtensionContentSession::OnGetUIExtensionWindowProxy(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (sessionInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null sessionInfo");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    HandleEscape handleEscape(env);
    napi_value jsExtensionWindow =
        Rosen::JsExtensionWindow::CreateJsExtensionWindow(env, uiWindow_, sessionInfo_->hostWindowId);
    if (jsExtensionWindow == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null jsExtensionWindow");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }
    auto value = JsRuntime::LoadSystemModuleByEngine(env, "application.extensionWindow", &jsExtensionWindow, 1);
    if (value == nullptr) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }
    return handleEscape.Escape(value->GetNapiValue());
}

napi_value JsUIExtensionContentSession::OnStartAbilityAsCaller(napi_env env, NapiCallbackInfo& info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (info.argc == ARGC_ZERO) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, info.argv[0], want)) {
        ThrowInvalidParamError(env, "Parameter error: Parse want failed! Want must be a Want");
        return CreateJsUndefined(env);
    }
    decltype(info.argc) unwrapArgc = 1;
    TAG_LOGI(AAFwkTag::UI_EXT, "StartAbilityAsCaller:%{public}s", want.GetElement().GetAbilityName().c_str());
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        TAG_LOGD(AAFwkTag::UI_EXT, "OnStartAbilityAsCaller start options is used");
        AppExecFwk::UnwrapStartOptions(env, info.argv[INDEX_ONE], startOptions);
        unwrapArgc++;
    }
#ifdef SUPPORT_SCREEN
    (unwrapArgc == INDEX_ONE) ? InitDisplayId(want) : InitDisplayId(want, startOptions, env, info);
#endif
    NapiAsyncTask::CompleteCallback complete = [weak = context_, want, startOptions, unwrapArgc,
        sessionInfo = sessionInfo_](napi_env env, NapiAsyncTask& task, int32_t status) {
            HandleScope handleScope(env);
            auto context = weak.lock();
            if (!context) {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            if (sessionInfo == nullptr) {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }
            auto innerErrorCode = (unwrapArgc == 1) ?
                AAFwk::AbilityManagerClient::GetInstance()->
                StartAbilityAsCaller(want, context->GetToken(), sessionInfo->callerToken) :
                AAFwk::AbilityManagerClient::GetInstance()->
                StartAbilityAsCaller(want, startOptions, context->GetToken(), sessionInfo->callerToken);
            if (innerErrorCode == 0) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
            }
        };
    napi_value lastParam = (info.argc > unwrapArgc) ? info.argv[unwrapArgc] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsUIExtensionContentSession::OnStartAbilityAsCaller",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NapiAsyncTask::ExecuteCallback JsUIExtensionContentSession::StartAbilityExecuteCallback(AAFwk::Want& want,
    size_t& unwrapArgc, napi_env env, NapiCallbackInfo& info, std::shared_ptr<int> &innerErrorCode)
{
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[1], napi_object)) {
        TAG_LOGD(AAFwkTag::UI_EXT, "OnStartAbility start options is used");
        bool unWrapStartOptionsFlag = AppExecFwk::UnwrapStartOptions(env, info.argv[1], startOptions);
        if (!unWrapStartOptionsFlag) {
            ThrowInvalidParamError(env, "Parameter error: Parse startOptions failed! Options must be a StartOption.");
        }
        unwrapArgc++;
    }
    if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
            system_clock::now().time_since_epoch()).count());
        want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
    }
#ifdef SUPPORT_SCREEN
    (unwrapArgc == INDEX_ONE) ? InitDisplayId(want) : InitDisplayId(want, startOptions, env, info);
#endif
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, want, startOptions, unwrapArgc,
        sessionInfo = sessionInfo_, &observer = freeInstallObserver_, innerErrorCode]() {
        auto context = weak.lock();
        if (!context) {
            TAG_LOGW(AAFwkTag::UI_EXT, "null context");
            *innerErrorCode = static_cast<int>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }
        *innerErrorCode = (unwrapArgc == 1) ?
            AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByUIContentSession(want,
                context->GetToken(), sessionInfo, -1, -1) :
            AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByUIContentSession(want,
                startOptions, context->GetToken(), sessionInfo, -1, -1);
        if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND &&
            *innerErrorCode != 0 && observer != nullptr) {
            std::string bundleName = want.GetElement().GetBundleName();
            std::string abilityName = want.GetElement().GetAbilityName();
            std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
            observer->OnInstallFinished(bundleName, abilityName, startTime, *innerErrorCode);
        }
    };
    return execute;
}

napi_value JsUIExtensionContentSession::OnStartAbilityForResult(napi_env env, NapiCallbackInfo& info)
{
    CHECK_IS_SYSTEM_APP;
    if (info.argc == ARGC_ZERO) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, info.argv[0], want)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "parse want error");
        ThrowInvalidParamError(env, "Failed to parse want! Want must be a Want.");
        return CreateJsUndefined(env);
    }
    if (!want.HasParameter(Want::PARAM_BACK_TO_OTHER_MISSION_STACK)) {
        want.SetParam(Want::PARAM_BACK_TO_OTHER_MISSION_STACK, true);
    }
    size_t unwrapArgc = 1;
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[1], napi_object)) {
        if (!AppExecFwk::UnwrapStartOptions(env, info.argv[1], startOptions)) {
            ThrowInvalidParamError(env, "Parameter error: Parse startOptions failed! Options must be a StartOption");
        }
        unwrapArgc++;
    }
#ifdef SUPPORT_SCREEN
    (unwrapArgc == INDEX_ONE) ? InitDisplayId(want) : InitDisplayId(want, startOptions, env, info);
#endif

    napi_value lastParam = info.argc > unwrapArgc ? info.argv[unwrapArgc] : nullptr;
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> uasyncTask;
    if ((want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
            system_clock::now().time_since_epoch()).count());
        want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
        AddFreeInstallObserver(env, want, lastParam, &result, true);
        uasyncTask = CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, nullptr);
    } else {
        uasyncTask = CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, &result);
    }
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);
    if (asyncTask == nullptr || listener_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "%s", (asyncTask == nullptr) ? "null asyncTask" : "null listener_");
        return CreateJsUndefined(env);
    }
    StartAbilityForResultRuntimeTask(env, want, asyncTask, unwrapArgc, startOptions);
    return result;
}

void JsUIExtensionContentSession::StartAbilityForResultRuntimeTask(napi_env env,
    AAFwk::Want &want, std::shared_ptr<NapiAsyncTask> asyncTask, size_t& unwrapArgc,
    AAFwk::StartOptions startOptions)
{
    if (asyncTask == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null asyncTask");
        return;
    }
    std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
    RuntimeTask task = [env, asyncTask, element = want.GetElement(), flags = want.GetFlags(), startTime,
        &observer = freeInstallObserver_](int resultCode, const AAFwk::Want& want, bool isInner) {
        std::string bundleName = element.GetBundleName();
        std::string abilityName = element.GetAbilityName();
        HandleScope handleScope(env);
        napi_value abilityResult = AppExecFwk::WrapAbilityResult(env, resultCode, want);
        if (abilityResult == nullptr) {
            TAG_LOGW(AAFwkTag::UI_EXT, "null abilityResult");
            isInner = true;
            resultCode = ERR_INVALID_VALUE;
        }
        bool freeInstallEnable = (flags & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND &&
            observer != nullptr;
        if (freeInstallEnable) {
            isInner ? observer->OnInstallFinished(bundleName, abilityName, startTime, resultCode) :
                observer->OnInstallFinished(bundleName, abilityName, startTime, abilityResult);
        } else {
            isInner ? asyncTask->Reject(env, CreateJsErrorByNativeErr(env, resultCode)) :
                asyncTask->ResolveWithNoError(env, abilityResult);
        }
    };
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "null context");
        asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    } else {
        want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
        int curRequestCode = reinterpret_cast<UIExtensionContext*>(context.get())->GenerateCurRequestCode();
        if (listener_ == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null listener_");
            return;
        }
        listener_->SaveResultCallbacks(curRequestCode, std::move(task));
        ErrCode err = (unwrapArgc == 1) ?
            AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByUIContentSession(want,
                context->GetToken(), sessionInfo_, curRequestCode, -1) :
            AAFwk::AbilityManagerClient::GetInstance()->StartAbilityByUIContentSession(want,
                startOptions, context->GetToken(), sessionInfo_, curRequestCode, -1);
        if (err != ERR_OK && err != AAFwk::START_ABILITY_WAITING) {
            TAG_LOGE(AAFwkTag::UI_EXT, "StartAbilityForResult. ret:%{public}d", err);
            listener_->OnAbilityResultInner(curRequestCode, err, want);
        }
    }
}

napi_value JsUIExtensionContentSession::OnTerminateSelf(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "called");
    NapiAsyncTask::CompleteCallback complete =
        [sessionInfo = sessionInfo_](napi_env env, NapiAsyncTask& task, int32_t status) {
            HandleScope handleScope(env);
            if (sessionInfo == nullptr) {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            auto errorCode = AAFwk::AbilityManagerClient::GetInstance()->TerminateUIExtensionAbility(sessionInfo);
            if (errorCode == 0) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, errorCode));
            }
        };
    napi_value lastParam = (info.argc > ARGC_ZERO) ? info.argv[INDEX_ZERO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsUIExtensionContentSession::OnTerminateSelf",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsUIExtensionContentSession::OnTerminateSelfWithResult(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "called");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    int resultCode = 0;
    AAFwk::Want want;
    if (!AppExecFwk::UnWrapAbilityResult(env, info.argv[INDEX_ZERO], resultCode, want)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "parse ability result failed");
        ThrowInvalidParamError(env, "Parameter error: Failed to parse parameter! Parameter must be a AbilityResult.");
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete;
    SetCallbackForTerminateWithResult(resultCode, want, complete);

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsUIExtensionContentSession::OnTerminateSelfWithResult",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsUIExtensionContentSession::OnSendData(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    CHECK_IS_SYSTEM_APP;
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    AAFwk::WantParams params;
    if (!AppExecFwk::UnwrapWantParams(env, info.argv[INDEX_ZERO], params)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "parse param failed");
        ThrowInvalidParamError(env, "OnSendData Failed to parse param! Data must be a Record<string, Object>.");
        return CreateJsUndefined(env);
    }

    if (uiWindow_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiWindow_");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    Rosen::WMError ret = uiWindow_->TransferExtensionData(params);
    if (ret == Rosen::WMError::WM_OK) {
        TAG_LOGD(AAFwkTag::UI_EXT, "TransferExtensionData success");
    } else {
        TAG_LOGE(AAFwkTag::UI_EXT, "TransferExtensionData failed, ret=%{public}d", ret);
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }
    return CreateJsUndefined(env);
}

napi_value JsUIExtensionContentSession::OnSetReceiveDataCallback(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    CHECK_IS_SYSTEM_APP;
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    if (!CheckTypeForNapiValue(env, info.argv[INDEX_ZERO], napi_function)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid param");
        ThrowInvalidParamError(env, "Parameter error: Callback must be a function");
        return CreateJsUndefined(env);
    }

    if (!isRegistered) {
        if (uiWindow_ == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null uiWindow_");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
        receiveDataCallback_ = std::make_shared<CallbackWrapper>();
        std::weak_ptr<CallbackWrapper> weakCallback(receiveDataCallback_);
        auto handler = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
        uiWindow_->RegisterTransferComponentDataListener([env, handler, weakCallback](
            const AAFwk::WantParams& wantParams) {
            if (handler) {
                handler->PostTask([env, weakCallback, wantParams]() {
                    JsUIExtensionContentSession::CallReceiveDataCallback(env, weakCallback, wantParams);
                    }, "JsUIExtensionContentSession:OnSetReceiveDataCallback");
            }
        });
        isRegistered = true;
    }

    napi_value callback = info.argv[INDEX_ZERO];
    if (receiveDataCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null receiveDataCallback_");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }
    napi_ref ref = nullptr;
    napi_create_reference(env, callback, 1, &ref);
    receiveDataCallback_->ResetCallback(std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
    return CreateJsUndefined(env);
}

napi_value JsUIExtensionContentSession::OnSetReceiveDataForResultCallback(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    CHECK_IS_SYSTEM_APP;
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    if (!CheckTypeForNapiValue(env, info.argv[INDEX_ZERO], napi_function)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid param");
        ThrowInvalidParamError(env, "Parameter error: Callback must be a function.");
        return CreateJsUndefined(env);
    }

    if (!isSyncRegistered) {
        if (uiWindow_ == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null uiWindow_");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
        receiveDataForResultCallback_ = std::make_shared<CallbackWrapper>();
        std::weak_ptr<CallbackWrapper> weakCallback(receiveDataForResultCallback_);
        auto handler = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
        uiWindow_->RegisterTransferComponentDataForResultListener([env, handler, weakCallback] (
            const AAFwk::WantParams& wantParams) -> AAFwk::WantParams {
                AAFwk::WantParams retWantParams;
                if (handler) {
                    handler->PostSyncTask([env, weakCallback, wantParams, &retWantParams]() {
                        JsUIExtensionContentSession::CallReceiveDataCallbackForResult(env, weakCallback,
                            wantParams, retWantParams);
                        }, "JsUIExtensionContentSession:OnSetReceiveDataForResultCallback");
                }
                return retWantParams;
        });
        isSyncRegistered = true;
    }
    napi_value callback = info.argv[INDEX_ZERO];
    if (receiveDataForResultCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null receiveDataForResultCallback_");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }
    napi_ref ref = nullptr;
    napi_create_reference(env, callback, 1, &ref);
    receiveDataForResultCallback_->ResetCallback(
        std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));

    return CreateJsUndefined(env);
}

napi_value JsUIExtensionContentSession::OnLoadContent(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    std::string contextPath;
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], contextPath)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid param");
        ThrowInvalidParamError(env, "Parameter error: Path must be a string.");
        return CreateJsUndefined(env);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "contextPath: %{public}s", contextPath.c_str());
    napi_value storage = nullptr;
    if (info.argc > ARGC_ONE) {
        if (!CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
            TAG_LOGE(AAFwkTag::UI_EXT, "invalid param");
            ThrowInvalidParamError(env, "Parameter error: Storage must be a LocalStorage.");
            return CreateJsUndefined(env);
        }
        storage = info.argv[INDEX_ONE];
    }
    if (uiWindow_ == nullptr || sessionInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiWindow_ or sessionInfo_");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    if (sessionInfo_->isAsyncModalBinding && isFirstTriggerBindModal_) {
        TAG_LOGD(AAFwkTag::UI_EXT, "Trigger binding UIExtension modal window");
        uiWindow_->TriggerBindModalUIExtension();
        isFirstTriggerBindModal_ = false;
    }
    sptr<IRemoteObject> parentToken = sessionInfo_->parentToken;
    Rosen::WMError ret = uiWindow_->NapiSetUIContent(contextPath, env, storage,
        Rosen::BackupAndRestoreType::NONE, parentToken);
    if (ret == Rosen::WMError::WM_OK) {
        TAG_LOGD(AAFwkTag::UI_EXT, "NapiSetUIContent success");
    } else {
        TAG_LOGE(AAFwkTag::UI_EXT, "NapiSetUIContent failed, ret=%{public}d", ret);
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }
    return CreateJsUndefined(env);
}

napi_value JsUIExtensionContentSession::OnLoadContentByName(napi_env env, NapiCallbackInfo& info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "LoadContentByName");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string name;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], name)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid param name");
        ThrowInvalidParamError(env, "Parameter error: name must be a string");
        return CreateJsUndefined(env);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "name: %{public}s", name.c_str());

    napi_value storage = nullptr;
    if (info.argc > ARGC_ONE) {
        if (!CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
            TAG_LOGE(AAFwkTag::UI_EXT, "Invalid param storage");
            ThrowInvalidParamError(env, "Parameter error: storage must be a object");
            return CreateJsUndefined(env);
        }
        storage = info.argv[INDEX_ONE];
    }

    if (uiWindow_ == nullptr || sessionInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "uiWindow or sessionInfo is null");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    if (sessionInfo_->isAsyncModalBinding && isFirstTriggerBindModal_) {
        TAG_LOGD(AAFwkTag::UI_EXT, "Trigger binding UIExtension modal window");
        uiWindow_->TriggerBindModalUIExtension();
        isFirstTriggerBindModal_ = false;
    }

    sptr<IRemoteObject> parentToken = sessionInfo_->parentToken;
    auto ret = uiWindow_->NapiSetUIContentByName(name, env, storage, Rosen::BackupAndRestoreType::NONE, parentToken);
    if (ret != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "NapiSetUIContentByName failed, ret: %{public}d", ret);
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    TAG_LOGD(AAFwkTag::UI_EXT, "NapiSetUIContentByName success");
    return CreateJsUndefined(env);
}

napi_value JsUIExtensionContentSession::OnSetWindowBackgroundColor(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    CHECK_IS_SYSTEM_APP;
    std::string color;
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], color)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid param");
        ThrowInvalidParamError(env, "Parameter error: Parse color failed! Color must be a string.");
        return CreateJsUndefined(env);
    }

    if (uiWindow_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiWindow_ ");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }
    Rosen::WMError ret = uiWindow_->SetBackgroundColor(color);
    if (ret == Rosen::WMError::WM_OK) {
        TAG_LOGD(AAFwkTag::UI_EXT, "SetBackgroundColor success");
    } else {
        TAG_LOGE(AAFwkTag::UI_EXT, "SetBackgroundColor failed, ret=%{public}d", ret);
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }
    return CreateJsUndefined(env);
}

napi_value JsUIExtensionContentSession::OnSetWindowPrivacyMode(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    bool isPrivacyMode = false;
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], isPrivacyMode)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid param");
        ThrowInvalidParamError(env, "Parameter error: Failed to parse isPrivacyMode! IsPrivacyMode must be a boolean.");
        return CreateJsUndefined(env);
    }
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    int ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(selfToken, PERMISSION_PRIVACY_WINDOW);
    if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        ThrowNoPermissionError(env, PERMISSION_PRIVACY_WINDOW);
        return CreateJsUndefined(env);
    }

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
    auto asyncTask = [uiWindow = uiWindow_, isPrivacyMode, env, task = napiAsyncTask.get()]() {
        HandleScope handleScope(env);
        if (uiWindow == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null uiWindow");
            task->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
            return;
        }
        auto ret = uiWindow->SetPrivacyMode(isPrivacyMode);
        if (ret == Rosen::WMError::WM_OK) {
            task->ResolveWithNoError(env, CreateJsUndefined(env));
        } else {
            task->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
        }
        delete task;
    };
    if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_vip)) {
        napiAsyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
    } else {
        napiAsyncTask.release();
    }
    return result;
}

napi_value JsUIExtensionContentSession::OnStartAbilityByType(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "called");

    std::string type;
    AAFwk::WantParams wantParam;
    if (!CheckStartAbilityByTypeParam(env, info, type, wantParam)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "check startAbilityByCall param failed");
        return CreateJsUndefined(env);
    }
    wantParam.SetParam(UIEXTENSION_TARGET_TYPE_KEY, AAFwk::String::Box(type));
    AAFwk::Want want;
    want.SetParams(wantParam);
    if (wantParam.HasParam(FLAG_AUTH_READ_URI_PERMISSION)) {
        want.SetFlags(wantParam.GetIntParam(FLAG_AUTH_READ_URI_PERMISSION, 0));
        wantParam.Remove(FLAG_AUTH_READ_URI_PERMISSION);
    }
#ifdef SUPPORT_SCREEN
    InitDisplayId(want);
#endif
    std::shared_ptr<JsUIExtensionCallback> uiExtensionCallback = std::make_shared<JsUIExtensionCallback>(env);
    uiExtensionCallback->SetJsCallbackObject(info.argv[INDEX_TWO]);
    HandleEscape handleEscape(env);
    napi_value completionHandler = AppExecFwk::GetPropertyValueByPropertyName(
        env, info.argv[INDEX_TWO], "completionHandler", napi_object);
    if (completionHandler != nullptr) {
        uiExtensionCallback->SetCompletionHandler(env, completionHandler);
    }
    NapiAsyncTask::CompleteCallback complete = [uiWindow = uiWindow_, type, want, uiExtensionCallback]
        (napi_env env, NapiAsyncTask& task, int32_t status) {
            HandleScope handleScope(env);
            if (uiWindow == nullptr || uiWindow->GetUIContent() == nullptr) {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
#ifdef SUPPORT_SCREEN
            Ace::ModalUIExtensionCallbacks callback;
            callback.onError = [uiExtensionCallback](int arg, const std::string &str1, const std::string &str2) {
                uiExtensionCallback->OnError(arg);
            };
            callback.onRelease = [uiExtensionCallback](const auto &arg) {
                uiExtensionCallback->OnRelease(arg);
            };
            callback.onReceive = [uiExtensionCallback](const AAFwk::WantParams& data) {
                if (data.HasParam("onRequestSuccess")) {
                    auto successParam = data.GetWantParams("onRequestSuccess");
                    auto elementName = successParam.GetStringParam("name");
                    uiExtensionCallback->OnRequestSuccess(elementName);
                } else if (data.HasParam("onRequestFailure")) {
                    auto failureParam = data.GetWantParams("onRequestFailure");
                    auto elementName = failureParam.GetStringParam("name");
                    int32_t failureCode = failureParam.GetIntParam("failureCode", 0);
                    if (failureCode != 0 && failureCode != 1) {
                        TAG_LOGE(AAFwkTag::CONTEXT, "Invalid failureCode: %{public}d", failureCode);
                        return;
                    }
                    std::string failureMsg = failureParam.GetStringParam("failureMessage");
                    uiExtensionCallback->OnRequestFailure(elementName, failureCode, failureMsg);
                }
            };
            Ace::ModalUIExtensionConfig config;
            int32_t sessionId = uiWindow->GetUIContent()->CreateModalUIExtension(want, callback, config);
            if (sessionId == 0) {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
            } else {
                uiExtensionCallback->SetUIContent(uiWindow->GetUIContent());
                uiExtensionCallback->SetSessionId(sessionId);
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            }
#endif // SUPPORT_SCREEN
        };
    napi_value lastParam = (info.argc > ARGC_THREE) ? info.argv[INDEX_THREE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsUIExtensionContentSession::OnStartAbilityByType",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return handleEscape.Escape(result);
}

bool JsUIExtensionContentSession::CheckStartAbilityByTypeParam(napi_env env,
    NapiCallbackInfo& info, std::string& type, AAFwk::WantParams& wantParam)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "start");

    if (info.argc < ARGC_THREE) {
        TAG_LOGW(AAFwkTag::UI_EXT, "Not enough params");
        ThrowTooFewParametersError(env);
        return false;
    }

    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], type)) {
        TAG_LOGW(AAFwkTag::UI_EXT, "parse type failed");
        ThrowInvalidParamError(env, "Parameter error: Failed to parse type! Type must be a string.");
        return false;
    }

    if (!AppExecFwk::UnwrapWantParams(env, info.argv[INDEX_ONE], wantParam)) {
        TAG_LOGW(AAFwkTag::UI_EXT, "parse wantParam failed");
        ThrowInvalidParamError(env, "Parameter error: Failed to parse wantParam, must be a Record<string, Object>.");
        return false;
    }

    return true;
}

napi_value JsUIExtensionContentSession::CreateJsUIExtensionContentSession(napi_env env,
    sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow,
    std::weak_ptr<AbilityRuntime::Context> context,
    std::shared_ptr<AbilityResultListeners>& abilityResultListeners)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "start");
    HandleEscape handleEscape(env);
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null object");
        return CreateJsUndefined(env);
    }

    std::unique_ptr<JsUIExtensionContentSession> jsSession =
        std::make_unique<JsUIExtensionContentSession>(sessionInfo, uiWindow, context, abilityResultListeners);
    napi_wrap(env, object, jsSession.release(), Finalizer, nullptr, nullptr);

    const char *moduleName = "JsUIExtensionContentSession";
    BindNativeFunction(env, object, "terminateSelf", moduleName, TerminateSelf);
    BindNativeFunction(env, object, "terminateSelfWithResult", moduleName, TerminateSelfWithResult);
    BindNativeFunction(env, object, "sendData", moduleName, SendData);
    BindNativeFunction(env, object, "setReceiveDataCallback", moduleName, SetReceiveDataCallback);
    BindNativeFunction(env, object, "setReceiveDataForResultCallback", moduleName, SetReceiveDataForResultCallback);
    BindNativeFunction(env, object, "loadContent", moduleName, LoadContent);
    BindNativeFunction(env, object, "loadContentByName", moduleName, LoadContentByName);
    BindNativeFunction(env, object, "setWindowBackgroundColor", moduleName, SetWindowBackgroundColor);
    BindNativeFunction(env, object, "setWindowPrivacyMode", moduleName, SetWindowPrivacyMode);
    BindNativeFunction(env, object, "startAbility", moduleName, StartAbility);
    BindNativeFunction(env, object, "startAbilityForResult", moduleName, StartAbilityForResult);
    BindNativeFunction(env, object, "startAbilityByType", moduleName, StartAbilityByType);
    BindNativeFunction(env, object, "startAbilityAsCaller", moduleName, StartAbilityAsCaller);
    BindNativeFunction(env, object, "getUIExtensionHostWindowProxy", moduleName, GetUIExtensionHostWindowProxy);
    BindNativeFunction(env, object, "getUIExtensionWindowProxy", moduleName, GetUIExtensionWindowProxy);
    return handleEscape.Escape(object);
}

napi_value JsUIExtensionContentSession::CreateJsUIExtensionContentSession(napi_env env,
    sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    HandleEscape handleEscape(env);
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null object");
        return CreateJsUndefined(env);
    }

    std::unique_ptr<JsUIExtensionContentSession> jsSession =
        std::make_unique<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    napi_wrap(env, object, jsSession.release(), Finalizer, nullptr, nullptr);

    const char *moduleName = "JsUIExtensionContentSession";
    BindNativeFunction(env, object, "terminateSelf", moduleName, TerminateSelf);
    BindNativeFunction(env, object, "terminateSelfWithResult", moduleName, TerminateSelfWithResult);
    BindNativeFunction(env, object, "sendData", moduleName, SendData);
    BindNativeFunction(env, object, "setReceiveDataCallback", moduleName, SetReceiveDataCallback);
    BindNativeFunction(env, object, "setReceiveDataForResultCallback", moduleName, SetReceiveDataForResultCallback);
    BindNativeFunction(env, object, "loadContent", moduleName, LoadContent);
    BindNativeFunction(env, object, "setWindowBackgroundColor", moduleName, SetWindowBackgroundColor);
    BindNativeFunction(env, object, "setWindowPrivacyMode", moduleName, SetWindowPrivacyMode);
    BindNativeFunction(env, object, "startAbility", moduleName, StartAbility);
    BindNativeFunction(env, object, "startAbilityForResult", moduleName, StartAbilityForResult);
    BindNativeFunction(env, object, "startAbilityByType", moduleName, StartAbilityByType);
    BindNativeFunction(env, object, "startAbilityAsCaller", moduleName, StartAbilityAsCaller);
    BindNativeFunction(env, object, "getUIExtensionHostWindowProxy", moduleName, GetUIExtensionHostWindowProxy);
    BindNativeFunction(env, object, "getUIExtensionWindowProxy", moduleName, GetUIExtensionWindowProxy);
    return handleEscape.Escape(object);
}

void JsUIExtensionContentSession::CallReceiveDataCallback(napi_env env,
    std::weak_ptr<CallbackWrapper> weakCallback, const AAFwk::WantParams& wantParams)
{
    auto cbWrapper = weakCallback.lock();
    if (cbWrapper == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "null cbWrapper");
        return;
    }
    auto callback = cbWrapper->GetCallback();
    if (callback == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "null callback");
        return;
    }
    HandleScope handleScope(env);
    napi_value method = callback->GetNapiValue();
    if (method == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "null method");
        return;
    }
    napi_value napiWantParams = AppExecFwk::WrapWantParams(env, wantParams);
    if (napiWantParams == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null napiWantParams");
        return;
    }
    napi_value argv[] = {napiWantParams};
    napi_value global = nullptr;
    napi_get_global(env, &global);
    napi_call_function(env, global, method, ARGC_ONE, argv, nullptr);
}

void JsUIExtensionContentSession::CallReceiveDataCallbackForResult(napi_env env,
    std::weak_ptr<CallbackWrapper> weakCallback, const AAFwk::WantParams& wantParams, AAFwk::WantParams& retWantParams)
{
    auto cbWrapper = weakCallback.lock();
    if (cbWrapper == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null cbWrapper");
        return;
    }
    auto callback = cbWrapper->GetCallback();
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null callback");
        return;
    }
    HandleScope handleScope(env);
    napi_value method = reinterpret_cast<napi_value>(callback->Get());
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null method");
        return;
    }
    napi_value napiWantParams = AppExecFwk::WrapWantParams(env, wantParams);
    if (napiWantParams == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null napiWantParams");
        return;
    }
    napi_value argv[] = {napiWantParams};
    napi_value global = nullptr;
    napi_get_global(env, &global);
    napi_value ret = nullptr;
    napi_call_function(env, global, method, ARGC_ONE, argv, &ret);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null ret");
        return;
    }

    if (!AppExecFwk::UnwrapWantParams(env, ret, retWantParams)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "parse param failed");
        return;
    }
}

void JsUIExtensionContentSession::AddFreeInstallObserver(napi_env env,
    const AAFwk::Want &want, napi_value callback, napi_value* result, bool isAbilityResult)
{
    // adapter free install async return install and start result
    int ret = 0;
    if (freeInstallObserver_ == nullptr) {
        freeInstallObserver_ = new JsFreeInstallObserver(env);
        auto context = context_.lock();
        if (!context) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null context");
            return;
        }
        ret = AAFwk::AbilityManagerClient::GetInstance()->AddFreeInstallObserver(context->GetToken(),
            freeInstallObserver_);
    }

    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "AddFreeInstallObserver failed");
    } else {
        TAG_LOGI(AAFwkTag::UI_EXT, "AddJsObserverObject");
        // build a callback observer with last param
        std::string bundleName = want.GetElement().GetBundleName();
        std::string abilityName = want.GetElement().GetAbilityName();
        std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
        freeInstallObserver_->AddJsObserverObject(
            bundleName, abilityName, startTime, callback, result, isAbilityResult);
    }
}

void JsUIExtensionContentSession::SetCallbackForTerminateWithResult(int32_t resultCode, AAFwk::Want& want,
    NapiAsyncTask::CompleteCallback& complete)
{
    complete =
        [weak = context_, uiWindow = uiWindow_, sessionInfo = sessionInfo_, want, resultCode](napi_env env,
            NapiAsyncTask& task, int32_t status) {
            HandleScope handleScope(env);
            auto extensionContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::UIExtensionContext>(weak.lock());
            if (!extensionContext) {
                TAG_LOGE(AAFwkTag::UI_EXT, "null extensionContext");
            } else {
                auto token = extensionContext->GetToken();
                AAFwk::AbilityManagerClient::GetInstance()->TransferAbilityResultForExtension(token, resultCode, want);
            }

            if (uiWindow == nullptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "null uiWindow");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            auto ret = uiWindow->TransferAbilityResult(resultCode, want);
            if (ret != Rosen::WMError::WM_OK) {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            auto errorCode = AAFwk::AbilityManagerClient::GetInstance()->TerminateUIExtensionAbility(sessionInfo);
            if (errorCode == 0) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, errorCode));
            }
        };
}
#ifdef SUPPORT_SCREEN
void JsUIExtensionContentSession::InitDisplayId(AAFwk::Want &want)
{
    auto context = AbilityRuntime::Context::ConvertTo<AbilityRuntime::UIExtensionContext>(context_.lock());
    if (!context) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }

    auto window = context->GetWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null window");
        return;
    }

    TAG_LOGI(AAFwkTag::UI_EXT, "window displayId %{public}" PRIu64, window->GetDisplayId());
    want.SetParam(AAFwk::Want::PARAM_RESV_DISPLAY_ID, static_cast<int32_t>(window->GetDisplayId()));
}

void JsUIExtensionContentSession::InitDisplayId(AAFwk::Want &want, AAFwk::StartOptions &startOptions,
    napi_env &env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "OnStartAbilityForResult start options is used");
    auto context = AbilityRuntime::Context::ConvertTo<AbilityRuntime::UIExtensionContext>(context_.lock());
    if (!context) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }

    auto window = context->GetWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null window");
        return;
    }

    int32_t displayId = 0;
    if (info.argc > ARGC_ONE && CheckTypeForNapiValue(env, info.argv[1], napi_object)
        && AppExecFwk::UnwrapInt32ByPropertyName(env, info.argv[1], "displayId", displayId)) {
        TAG_LOGI(AAFwkTag::UI_EXT, "startOption displayId %{public}d", startOptions.GetDisplayID());
        return;
    }

    TAG_LOGI(AAFwkTag::UI_EXT, "window displayId %{public}" PRIu64, window->GetDisplayId());
    startOptions.SetDisplayID(window->GetDisplayId());
}
#endif
}  // namespace AbilityRuntime
}  // namespace OHOS
