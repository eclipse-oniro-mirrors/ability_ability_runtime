/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "js_ability_manager.h"

#include <cstdint>
#include <cinttypes>
#include <memory>
#include <regex>
#include <future>

#include "ability_business_error.h"
#include "app_utils.h"
#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "acquire_share_data_callback_stub.h"
#include "app_mgr_interface.h"
#include "application_context.h"
#include "errors.h"
#include "event_runner.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "js_ability_foreground_state_observer.h"
#include "js_ability_manager_utils.h"
#include "js_error_utils.h"
#include "js_preload_ui_extension_callback_client.h"
#include "js_query_erms_observer.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_base_context.h"
#include "napi_common_configuration.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "permission_constants.h"
#include "preload_ui_extension_host_client.h"
#include "system_ability_definition.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AbilityRuntime {
using AbilityManagerClient = AAFwk::AbilityManagerClient;
namespace {
OHOS::sptr<OHOS::AppExecFwk::IAppMgr> GetAppManagerInstance()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::sptr<OHOS::IRemoteObject> appObject = systemAbilityManager->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID);
    return OHOS::iface_cast<OHOS::AppExecFwk::IAppMgr>(appObject);
}

constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr const char *ON_OFF_TYPE_ABILITY_FOREGROUND_STATE = "abilityForegroundState";
const std::string MAX_UINT64_VALUE = "18446744073709551615";
static std::shared_ptr<AppExecFwk::EventHandler> mainHandler_ = nullptr;

class JsAbilityManager final {
public:
    JsAbilityManager() = default;
    ~JsAbilityManager() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "finalizer called");
        std::unique_ptr<JsAbilityManager>(static_cast<JsAbilityManager*>(data));
    }

    static napi_value GetAbilityRunningInfos(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnGetAbilityRunningInfos);
    }

    static napi_value GetExtensionRunningInfos(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnGetExtensionRunningInfos);
    }

    static napi_value UpdateConfiguration(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnUpdateConfiguration);
    }

    static napi_value GetTopAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnGetTopAbility);
    }

    static napi_value AcquireShareData(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnAcquireShareData);
    }

    static napi_value NotifySaveAsResult(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnNotifySaveAsResult);
    }
    static napi_value GetForegroundUIAbilities(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAbilityManager, OnGetForegroundUIAbilities);
    }

    static napi_value On(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAbilityManager, OnOn);
    }

    static napi_value Off(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAbilityManager, OnOff);
    }

    static napi_value IsEmbeddedUIExtensionSupported(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnIsEmbeddedUIExtensionSupported);
    }

    static napi_value IsEmbeddedOpenAllowed(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnIsEmbeddedOpenAllowed);
    }

    static napi_value QueryAtomicServiceStartupRule(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnQueryAtomicServiceStartupRule);
    }

    static napi_value RestartSelfAtomicService(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnRestartSelfAtomicService);
    }

    static napi_value SetResidentProcessEnabled(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAbilityManager, OnSetResidentProcessEnabled);
    }

    static napi_value NotifyDebugAssertResult(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAbilityManager, OnNotifyDebugAssertResult);
    }

    static napi_value OnPreloadedUIExtensionAbilityLoaded(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnOnPreloadedUIExtensionAbilityLoaded);
    }

    static napi_value OffPreloadedUIExtensionAbilityLoaded(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnOffPreloadedUIExtensionAbilityLoaded);
    }

    static napi_value OnPreloadedUIExtensionAbilityDestroyed(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnOnPreloadedUIExtensionAbilityDestroyed);
    }

    static napi_value OffPreloadedUIExtensionAbilityDestroyed(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnOffPreloadedUIExtensionAbilityDestroyed);
    }

    static napi_value PreloadUIExtensionAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnPreloadUIExtensionAbility);
    }

    static napi_value ClearPreloadedUIExtensionAbility(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnClearPreloadedUIExtensionAbility);
    }

    static napi_value ClearPreloadedUIExtensionAbilities(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsAbilityManager, OnClearPreloadedUIExtensionAbilities);
    }

private:
    sptr<OHOS::AbilityRuntime::JSAbilityForegroundStateObserver> observerForeground_ = nullptr;
    sptr<JsQueryERMSObserver> queryERMSObserver_ = nullptr;
    std::vector<std::pair<napi_ref, int32_t>> loadedCallback_ = {};
    std::vector<std::pair<napi_ref, int32_t>> destroyCallback_ = {};
    std::mutex loadedCallbackMutex_;
    std::mutex destroyedCallbackMutex_;

    std::string ParseParamType(const napi_env &env, size_t argc, const napi_value *argv)
    {
        std::string type;
        if (argc > INDEX_ZERO && ConvertFromJsValue(env, argv[INDEX_ZERO], type)) {
            return type;
        }
        return "";
    }

    napi_value OnOn(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
        if (argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        if (!AppExecFwk::IsTypeForNapiValue(env, argv[INDEX_ONE], napi_object)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid param");
            ThrowInvalidParamError(env, "Parse param observer failed, must be a AbilityForegroundStateObserver.");
            return CreateJsUndefined(env);
        }

        std::string type = ParseParamType(env, argc, argv);
        if (type == ON_OFF_TYPE_ABILITY_FOREGROUND_STATE) {
            return OnOnAbilityForeground(env, argc, argv);
        }
        ThrowInvalidParamError(env, "Parse param type failed, must be a string, value must be abilityForegroundState.");
        return CreateJsUndefined(env);
    }

    napi_value OnOnAbilityForeground(napi_env env, size_t argc, napi_value *argv)
    {
        if (observerForeground_ == nullptr) {
            observerForeground_ = new (std::nothrow) JSAbilityForegroundStateObserver(env);
            if (observerForeground_ == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "null observerForeground_");
                ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
                return CreateJsUndefined(env);
            }
            napi_add_env_cleanup_hook(env, JSAbilityForegroundStateObserver::CleanUp,
                new wptr<JSAbilityForegroundStateObserver>(observerForeground_));
        }

        if (observerForeground_->IsEmpty()) {
            int32_t ret = GetAppManagerInstance()->RegisterAbilityForegroundStateObserver(observerForeground_);
            if (ret != NO_ERROR) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "error: %{public}d", ret);
                ThrowErrorByNativeErr(env, ret);
                return CreateJsUndefined(env);
            }
        }
        observerForeground_->AddJsObserverObject(argv[INDEX_ONE]);

        return CreateJsUndefined(env);
    }

    napi_value OnOff(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
        if (argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        if (argc == ARGC_TWO && !AppExecFwk::IsTypeForNapiValue(env, argv[INDEX_ONE], napi_object)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid param");
            ThrowInvalidParamError(env, "Parse param observer failed, must be a AbilityForegroundStateObserver.");
            return CreateJsUndefined(env);
        }

        std::string type = ParseParamType(env, argc, argv);
        if (type == ON_OFF_TYPE_ABILITY_FOREGROUND_STATE) {
            return OnOffAbilityForeground(env, argc, argv);
        }
        ThrowInvalidParamError(env, "Parse param type failed, must be a string, value must be abilityForegroundState.");
        return CreateJsUndefined(env);
    }

    bool CheckIsNumString(const std::string &numStr)
    {
        const std::regex regexJsperf(R"(^\d*)");
        std::match_results<std::string::const_iterator> matchResults;
        if (numStr.empty() || !std::regex_match(numStr, matchResults, regexJsperf)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "parse failed: %{public}s", numStr.c_str());
            return false;
        }
        if (MAX_UINT64_VALUE.length() < numStr.length() ||
            (MAX_UINT64_VALUE.length() == numStr.length() && MAX_UINT64_VALUE.compare(numStr) < 0)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "parse failed: %{public}s", numStr.c_str());
            return false;
        }
        return true;
    }

    napi_value OnNotifyDebugAssertResult(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
        HandleEscape handleEscape(env);
        if (argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string assertSessionStr;
        if (!ConvertFromJsValue(env, argv[INDEX_ZERO], assertSessionStr) || !CheckIsNumString(assertSessionStr)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "convert sessionId failed");
            ThrowInvalidParamError(env, "Parse param sessionId failed, must be a string.");
            return CreateJsUndefined(env);
        }
        uint64_t assertSessionId = std::stoull(assertSessionStr);
        if (assertSessionId == 0) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "convert sessionId failed");
            ThrowInvalidParamError(env, "Parse param sessionId failed, value must not be equal to zero.");
            return CreateJsUndefined(env);
        }
        int32_t userStatus;
        if (!ConvertFromJsValue(env, argv[INDEX_ONE], userStatus)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "convert status failed");
            ThrowInvalidParamError(env, "Parse param status failed, must be a UserStatus.");
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [assertSessionId, userStatus](napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            auto amsClient = AbilityManagerClient::GetInstance();
            if (amsClient == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "null amsClient");
                task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(AAFwk::INNER_ERR)));
                return;
            }
            auto ret = amsClient->NotifyDebugAssertResult(assertSessionId, static_cast<AAFwk::UserStatus>(userStatus));
            if (ret != ERR_OK) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "failed %{public}d", ret);
                task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(ret)));
                return;
            }
            task.ResolveWithNoError(env, CreateJsUndefined(env));
        };

        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsAbilityManager::OnNotifyDebugAssertResult", env,
            CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnOffAbilityForeground(napi_env env, size_t argc, napi_value *argv)
    {
        if (observerForeground_ == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
        if (argc == ARGC_TWO) {
            observerForeground_->RemoveJsObserverObject(argv[INDEX_ONE]);
        } else {
            observerForeground_->RemoveAllJsObserverObject();
        }

        if (observerForeground_->IsEmpty()) {
            int32_t ret = GetAppManagerInstance()->UnregisterAbilityForegroundStateObserver(observerForeground_);
            if (ret != NO_ERROR) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "error: %{public}d", ret);
                ThrowErrorByNativeErr(env, ret);
                return CreateJsUndefined(env);
            }
        }
        return CreateJsUndefined(env);
    }

    napi_value OnGetAbilityRunningInfos(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
        HandleEscape handleEscape(env);
        NapiAsyncTask::CompleteCallback complete =
            [](napi_env env, NapiAsyncTask &task, int32_t status) {
                HandleScope handleScope(env);
                std::vector<AAFwk::AbilityRunningInfo> infos;
                auto errcode = AbilityManagerClient::GetInstance()->GetAbilityRunningInfos(infos);
                if (errcode == 0) {
#ifdef ENABLE_ERRCODE
                    task.ResolveWithNoError(env, CreateJsAbilityRunningInfoArray(env, infos));
                } else {
                    task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(errcode)));
#else
                    task.Resolve(env, CreateJsAbilityRunningInfoArray(env, infos));
                } else {
                    task.Reject(env, CreateJsError(env, errcode, "Get mission infos failed."));
#endif
                }
            };

        napi_value lastParam = (info.argc == 0) ? nullptr : info.argv[0];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsAbilityManager::OnGetAbilityRunningInfos",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnGetExtensionRunningInfos(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
        HandleEscape handleEscape(env);
        if (info.argc == 0) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid argc");
#ifdef ENABLE_ERRCODE
            ThrowTooFewParametersError(env);
#endif
            return CreateJsUndefined(env);
        }
        int upperLimit = -1;
        if (!ConvertFromJsValue(env, info.argv[0], upperLimit)) {
#ifdef ENABLE_ERRCODE
            ThrowInvalidParamError(env, "Parse param upperLimit failed, must be a number.");
#endif
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [upperLimit](napi_env env, NapiAsyncTask &task, int32_t status) {
                HandleScope handleScope(env);
                std::vector<AAFwk::ExtensionRunningInfo> infos;
                auto errcode = AbilityManagerClient::GetInstance()->GetExtensionRunningInfos(upperLimit, infos);
                if (errcode == 0) {
#ifdef ENABLE_ERRCODE
                    task.ResolveWithNoError(env, CreateJsExtensionRunningInfoArray(env, infos));
                } else {
                    task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(errcode)));
#else
                    task.Resolve(env, CreateJsExtensionRunningInfoArray(env, infos));
                } else {
                    task.Reject(env, CreateJsError(env, errcode, "Get mission infos failed."));
#endif
                }
            };

        napi_value lastParam = (info.argc == 1) ? nullptr : info.argv[1];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsAbilityManager::OnGetExtensionRunningInfos",
            env, CreateAsyncTaskWithLastParam(env,
            lastParam, nullptr, std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnUpdateConfiguration(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "called");
        HandleEscape handleEscape(env);
        NapiAsyncTask::CompleteCallback complete;

        do {
            if (info.argc == 0) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid argc");
#ifdef ENABLE_ERRCODE
                ThrowTooFewParametersError(env);
#else
                complete = [](napi_env env, NapiAsyncTask& task, int32_t status) {
                    HandleScope handleScope(env);
                    task.Reject(env, CreateJsError(env, ERR_INVALID_VALUE, "no enough params."));
                };
#endif
                break;
            }

            AppExecFwk::Configuration changeConfig;
            if (!UnwrapConfiguration(env, info.argv[0], changeConfig)) {
#ifdef ENABLE_ERRCODE
                ThrowInvalidParamError(env, "Parse param config failed, must be a Configuration.");
#else
                complete = [](napi_env env, NapiAsyncTask& task, int32_t status) {
                    HandleScope handleScope(env);
                    task.Reject(env, CreateJsError(env, ERR_INVALID_VALUE, "config is invalid."));
                };
#endif
                break;
            }

            complete = [changeConfig](napi_env env, NapiAsyncTask& task, int32_t status) {
                HandleScope handleScope(env);
                auto errcode = GetAppManagerInstance()->UpdateConfiguration(changeConfig);
                if (errcode == 0) {
#ifdef ENABLE_ERRCODE
                    task.ResolveWithNoError(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(errcode)));
#else
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsError(env, errcode, "update config failed."));
#endif
                }
            };
        } while (0);

        napi_value lastParam = (info.argc == 1) ? nullptr : info.argv[1];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsAbilityManager::OnGetExtensionRunningInfos",
            env, CreateAsyncTaskWithLastParam(env,
            lastParam, nullptr, std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnGetTopAbility(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "called");
        HandleEscape handleEscape(env);
#ifdef ENABLE_ERRCODE
        auto selfToken = IPCSkeleton::GetSelfTokenID();
        if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "not system app");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
            return CreateJsUndefined(env);
        }
#endif
        NapiAsyncTask::CompleteCallback complete =
            [](napi_env env, NapiAsyncTask &task, int32_t status) {
                HandleScope handleScope(env);
                AppExecFwk::ElementName elementName = AbilityManagerClient::GetInstance()->GetTopAbility();
#ifdef ENABLE_ERRCOE
                task.ResolveWithNoError(env, CreateJsElementName(env, elementName));
#else
                task.Resolve(env, CreateJsElementName(env, elementName));
#endif
            };

        napi_value lastParam = (info.argc == 0) ? nullptr : info.argv[0];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsAbilityManager::OnGetTopAbility",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnAcquireShareData(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "called");
        HandleEscape handleEscape(env);
        if (info.argc < ARGC_ONE) {
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        int32_t missionId = -1;
        if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], missionId)) {
            ThrowInvalidParamError(env, "Parse param missionId failed, must be a number.");
            return CreateJsUndefined(env);
        }
        napi_value lastParam = info.argc > ARGC_ONE  ? info.argv[INDEX_ONE] : nullptr;
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> uasyncTask = CreateAsyncTaskWithLastParam(
            env, lastParam, nullptr, nullptr, &result);
        std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);

        AAFwk::ShareRuntimeTask task = [env, asyncTask](int32_t resultCode, const AAFwk::WantParams &wantParam) {
            HandleScope handleScope(env);
            if (resultCode != 0) {
                asyncTask->Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(resultCode)));
                return;
            }
            napi_value abilityResult = AppExecFwk::WrapWantParams(env, wantParam);
            if (abilityResult == nullptr) {
                asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
            } else {
                asyncTask->ResolveWithNoError(env, abilityResult);
            }
        };
        sptr<AAFwk::AcquireShareDataCallbackStub> shareDataCallbackStub = new AAFwk::AcquireShareDataCallbackStub();
        mainHandler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
        shareDataCallbackStub->SetHandler(mainHandler_);
        shareDataCallbackStub->SetShareRuntimeTask(task);
        auto err = AbilityManagerClient::GetInstance()->AcquireShareData(missionId, shareDataCallbackStub);
        if (err != 0) {
            asyncTask->Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(err)));
        }
        return handleEscape.Escape(result);
    }

    napi_value OnNotifySaveAsResult(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "called");
        HandleEscape handleEscape(env);
        NapiAsyncTask::CompleteCallback complete;
        NapiAsyncTask::ExecuteCallback execute;

        do {
            if (info.argc < ARGC_TWO) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid argc");
                ThrowTooFewParametersError(env);
                break;
            }

            int reqCode = 0;
            if (!ConvertFromJsValue(env, info.argv[1], reqCode)) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "get requestCode failed");
                ThrowInvalidParamError(env, "Parse param requestCode failed, must be a number.");
                break;
            }

            AppExecFwk::Want want;
            int resultCode = ERR_OK;
            if (!AppExecFwk::UnWrapAbilityResult(env, info.argv[0], resultCode, want)) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "unwrap abilityResult failed");
                ThrowInvalidParamError(env, "Parse param parameter failed, must be a AbilityResult.");
                break;
            }

            auto sharedCode = std::make_shared<ErrCode>(ERR_OK);
            execute = [sharedCode, want, resultCode, reqCode]() {
                *sharedCode = AbilityManagerClient::GetInstance()->NotifySaveAsResult(want, resultCode, reqCode);
            };
            complete = [sharedCode](napi_env env, NapiAsyncTask& task, int32_t status) {
                HandleScope handleScope(env);
                auto errCode = *sharedCode;
                if (errCode == ERR_OK) {
                    task.ResolveWithNoError(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(errCode)));
                }
            };
        } while (0);

        napi_value lastParam = (info.argc == ARGC_TWO) ? nullptr : info.argv[ARGC_TWO];
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsAbilityManager::OnNotifySaveAsResult", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnGetForegroundUIAbilities(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
        HandleEscape handleEscape(env);
        NapiAsyncTask::CompleteCallback complete = [](napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            std::vector<AppExecFwk::AbilityStateData> list;
            int32_t ret = AbilityManagerClient::GetInstance()->GetForegroundUIAbilities(list);
            if (ret == ERR_OK) {
                task.ResolveWithNoError(env, CreateJsAbilityStateDataArray(env, list));
            } else {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "error: %{public}d", ret);
                task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(ret)));
            }
        };

        napi_value lastParam = (argc > ARGC_ZERO) ? argv[INDEX_ZERO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsAbilityManager::OnGetForegroundUIAbilities", env,
            CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnSetResidentProcessEnabled(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
        HandleEscape handleEscape(env);
        if (argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string bundleName;
        if (!ConvertFromJsValue(env, argv[INDEX_ZERO], bundleName) || bundleName.empty()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "parse bundleName failed, not string");
            ThrowInvalidParamError(env, "Parse param bundleName failed, must be a string.");
            return CreateJsUndefined(env);
        }

        bool enableState = false;
        if (!ConvertFromJsValue(env, argv[INDEX_ONE], enableState)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "parse enable failed, not boolean");
            ThrowInvalidParamError(env, "Parse param enable failed, must be a boolean.");
            return CreateJsUndefined(env);
        }

        auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute = [bundleName, enableState, innerErrorCode, env]() {
            auto amsClient = AbilityManagerClient::GetInstance();
            if (amsClient == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "null amsClient");
                *innerErrorCode = static_cast<int32_t>(AAFwk::INNER_ERR);
                return;
            }
            *innerErrorCode = amsClient->SetResidentProcessEnabled(bundleName, enableState);
        };

        NapiAsyncTask::CompleteCallback complete = [innerErrorCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            if (*innerErrorCode != ERR_OK) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "error: %{public}d",
                    *innerErrorCode);
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
                return;
            }
            task.ResolveWithNoError(env, CreateJsUndefined(env));
        };

        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsAbilityManager::OnSetResidentProcessEnabled", env,
            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnIsEmbeddedUIExtensionSupported(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
        HandleEscape handleEscape(env);
        bool isSupported = AAFwk::AppUtils::GetInstance().IsMultiProcessModel();
        napi_value result = CreateJsValue(env, isSupported);
        return handleEscape.Escape(result);
    }

    napi_value OnIsEmbeddedOpenAllowed(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
        HandleEscape handleEscape(env);
        if (info.argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        bool stageMode = false;
        napi_status status = OHOS::AbilityRuntime::IsStageContext(env, info.argv[0], stageMode);
        if (status != napi_ok || !stageMode) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "not stageMode");
            ThrowInvalidParamError(env, "Parse param context failed, must be a context of stageMode.");
            return CreateJsUndefined(env);
        }
        auto context = OHOS::AbilityRuntime::GetStageModeContext(env, info.argv[0]);
        if (context == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null context");
            ThrowInvalidParamError(env, "Parse param context failed, must not be nullptr.");
            return CreateJsUndefined(env);
        }
        auto uiAbilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
        if (uiAbilityContext == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null UIAbilityContext");
            ThrowInvalidParamError(env, "Parse param context failed, must be UIAbilityContext.");
            return CreateJsUndefined(env);
        }

        std::string appId;
        if (!ConvertFromJsValue(env, info.argv[1], appId)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "parse appId failed");
            ThrowInvalidParamError(env, "Parse param appId failed, must be a string.");
            return CreateJsUndefined(env);
        }

        auto token = uiAbilityContext->GetToken();
        auto sharedResult = std::make_shared<bool>(false);
        NapiAsyncTask::ExecuteCallback execute = [sharedResult, token, appId]() {
            *sharedResult = AbilityManagerClient::GetInstance()->IsEmbeddedOpenAllowed(token, appId);
        };

        NapiAsyncTask::CompleteCallback complete = [sharedResult](napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            task.Resolve(env, CreateJsValue(env, *sharedResult));
        };

        napi_value lastParam = (info.argc > ARGC_TWO) ? info. argv[ARGC_TWO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsAbilityManager::OnIsEmbeddedOpenAllowed", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    int AddQueryERMSObserver(napi_env env, sptr<IRemoteObject> token, const std::string &appId,
        const std::string &startTime, napi_value *result)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
        int ret = 0;
        if (queryERMSObserver_ == nullptr) {
            queryERMSObserver_ = new JsQueryERMSObserver(env);
        }
        queryERMSObserver_->AddJsObserverObject(appId, startTime, result);

        ret = AbilityManagerClient::GetInstance()->AddQueryERMSObserver(token, queryERMSObserver_);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "addQueryERMSObserver error");
            AtomicServiceStartupRule rule;
            queryERMSObserver_->OnQueryFinished(appId, startTime, rule, AAFwk::INNER_ERR);
            return ret;
        }
        return ERR_OK;
    }

    napi_value OnQueryAtomicServiceStartupRuleInner(napi_env env, sptr<IRemoteObject> token,
        const std::string &appId)
    {
        HandleEscape handleEscape(env);
        auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
        auto rule = std::make_shared<AtomicServiceStartupRule>();
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
            system_clock::now().time_since_epoch()).count());
        napi_value result = nullptr;
        auto ret = AddQueryERMSObserver(env, token, appId, startTime, &result);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "AddQueryERMSObserver failed, ret=%{public}d", ret);
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::ExecuteCallback execute = [innerErrorCode, rule, token, appId, startTime]() {
            *innerErrorCode = AbilityManagerClient::GetInstance()->QueryAtomicServiceStartupRule(
                token, appId, startTime, *rule);
        };

        NapiAsyncTask::CompleteCallback complete = [appId, startTime, innerErrorCode, rule,
            observer = queryERMSObserver_](
            napi_env env, NapiAsyncTask &task, int32_t status) {
            if (observer == nullptr) {
                TAG_LOGW(AAFwkTag::ABILITYMGR, "null observer");
                return;
            }
            if (*innerErrorCode == AAFwk::ERR_ECOLOGICAL_CONTROL_STATUS) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "openning dialog to confirm");
                return;
            }
            if (*innerErrorCode != ERR_OK) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "query failed: %{public}d", *innerErrorCode);
                observer->OnQueryFinished(appId, startTime, *rule, *innerErrorCode);
                return;
            }
            observer->OnQueryFinished(appId, startTime, *rule, ERR_OK);
        };

        NapiAsyncTask::Schedule("JsAbilityManager::OnQueryAtomicServiceStartupRule", env,
            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), nullptr));
        return handleEscape.Escape(result);
    }

    napi_value OnQueryAtomicServiceStartupRule(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
        HandleEscape handleEscape(env);
        if (info.argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        bool stageMode = false;
        napi_status status = OHOS::AbilityRuntime::IsStageContext(env, info.argv[0], stageMode);
        if (status != napi_ok || !stageMode) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "not stageMode");
            ThrowInvalidParamError(env, "Parse param context failed, must be a context of stageMode.");
            return CreateJsUndefined(env);
        }
        auto context = OHOS::AbilityRuntime::GetStageModeContext(env, info.argv[0]);
        if (context == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null context");
            ThrowInvalidParamError(env, "Parse param context failed, must not be nullptr.");
            return CreateJsUndefined(env);
        }
        auto uiAbilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
        if (uiAbilityContext == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null UIAbilityContext");
            ThrowInvalidParamError(env, "Parse param context failed, must be UIAbilityContext.");
            return CreateJsUndefined(env);
        }

        std::string appId;
        if (!ConvertFromJsValue(env, info.argv[1], appId)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "parse appId failed");
            ThrowInvalidParamError(env, "Parse param appId failed, must be a string.");
            return CreateJsUndefined(env);
        }

        auto token = uiAbilityContext->GetToken();
        return handleEscape.Escape(OnQueryAtomicServiceStartupRuleInner(env, token, appId));
    }

    napi_value OnRestartSelfAtomicServiceInner(napi_env env, sptr<IRemoteObject> token)
    {
        auto ret = AbilityManagerClient::GetInstance()->RestartSelfAtomicService(token);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "resatrt atomic service failed: %{public}d", ret);
            ThrowErrorByNativeErr(env, ret);
        }
        return CreateJsUndefined(env);
    }

    napi_value OnRestartSelfAtomicService(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
        if (info.argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        bool stageMode = false;
        napi_status status = OHOS::AbilityRuntime::IsStageContext(env, info.argv[0], stageMode);
        if (status != napi_ok || !stageMode) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "not stageMode");
            ThrowInvalidParamError(env, "Parse param context failed, must be a context of stageMode.");
            return CreateJsUndefined(env);
        }
        auto context = OHOS::AbilityRuntime::GetStageModeContext(env, info.argv[0]);
        if (context == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null context");
            ThrowInvalidParamError(env, "Parse param context failed, must not be nullptr.");
            return CreateJsUndefined(env);
        }
        auto uiAbilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
        if (uiAbilityContext == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null UIAbilityContext");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_UI_ABILITY_CONTEXT);
            return CreateJsUndefined(env);
        }

        auto token = uiAbilityContext->GetToken();
        return OnRestartSelfAtomicServiceInner(env, token);
    }

    bool CheckPreloadUIExtensionAbilityPermission(napi_env env)
    {
        auto selfToken = IPCSkeleton::GetSelfTokenID();
        if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Non-system app forbidden to call");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
            return false;
        }
        int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(
            selfToken, AAFwk::PermissionConstants::PERMISSION_PRELOAD_UI_EXTENSION_ABILITY);
        if (result != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "VerifyAccessToken fail");
            ThrowNoPermissionError(env, AAFwk::PermissionConstants::PERMISSION_PRELOAD_UI_EXTENSION_ABILITY);
            return false;
        }
        return true;
    }
    
    napi_value OnOnPreloadedUIExtensionAbilityLoaded(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "OnOnPreloadedUIExtensionAbilityLoaded called");
        HandleScope handleScope(env);
        if (!CheckPreloadUIExtensionAbilityPermission(env)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "no permission");
            return CreateJsUndefined(env);
        }
        if (info.argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ZERO], napi_function)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid param");
            ThrowInvalidParamError(env, "Parse param callback failed, must be a PreloadedUIExtensionAbilityLoadedFn.");
            return CreateJsUndefined(env);
        }
        napi_value newJsFunc = info.argv[INDEX_ZERO];
        std::lock_guard<std::mutex> lock(loadedCallbackMutex_);
        for (const auto &cb : loadedCallback_) {
            napi_value jsFunc = nullptr;
            bool isEquals = false;
            napi_get_reference_value(env, cb.first, &jsFunc);
            napi_strict_equals(env, newJsFunc, jsFunc, &isEquals);
            if (isEquals) {
                TAG_LOGW(AAFwkTag::ABILITYMGR, "callback already exists");
                return CreateJsUndefined(env);
            }
        }
        napi_ref callbackRef = nullptr;
        napi_create_reference(env, newJsFunc, 1, &callbackRef);
        auto client = std::make_shared<JsPreloadUIExtensionCallbackClient>(env, callbackRef);
        int32_t key = PreloadUIExtensionHostClient::GetInstance()->AddLoadedCallback(client);
        loadedCallback_.push_back({ callbackRef, key });
        return CreateJsUndefined(env);
    }

    napi_value OnOffPreloadedUIExtensionAbilityLoaded(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "OnOffPreloadedUIExtensionAbilityLoaded called");
        if (!CheckPreloadUIExtensionAbilityPermission(env)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "no permission");
            return CreateJsUndefined(env);
        }
        if (info.argc == ARGC_ONE && !AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ZERO], napi_function)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid param");
            ThrowInvalidParamError(env, "Parse param callback failed, must be a PreloadedUIExtensionAbilityLoadedFn.");
            return CreateJsUndefined(env);
        }
        std::lock_guard<std::mutex> lock(loadedCallbackMutex_);
        if (info.argc == ARGC_ZERO) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "callback is undefined");
            if (loadedCallback_.empty()) {
                TAG_LOGW(AAFwkTag::ABILITYMGR, "null callback");
                return CreateJsUndefined(env);
            }
            PreloadUIExtensionHostClient::GetInstance()->RemoveAllLoadedCallback();
            loadedCallback_.clear();
            return CreateJsUndefined(env);
        }
        auto it = std::find_if(loadedCallback_.begin(), loadedCallback_.end(),
            [env, targetFunc = info.argv[INDEX_ZERO]](const auto &cb) {
            napi_value jsFunc = nullptr;
            bool isEquals = false;
            napi_get_reference_value(env, cb.first, &jsFunc);
            napi_strict_equals(env, targetFunc, jsFunc, &isEquals);
            return isEquals;
        });
        if (it == loadedCallback_.end()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "callback not found");
            return CreateJsUndefined(env);
        }
        PreloadUIExtensionHostClient::GetInstance()->RemoveLoadedCallback(it->second);
        loadedCallback_.erase(it);
        return CreateJsUndefined(env);
    }

    napi_value OnOnPreloadedUIExtensionAbilityDestroyed(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "OnOnPreloadedUIExtensionAbilityDestroyed called");
        HandleScope handleScope(env);
        if (!CheckPreloadUIExtensionAbilityPermission(env)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "no permission");
            return CreateJsUndefined(env);
        }
        if (info.argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ZERO], napi_function)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid param");
            ThrowInvalidParamError(env, "Parse param callback failed, must be a "
                                        "PreloadedUIExtensionAbilityDestroyedFn.");
            return CreateJsUndefined(env);
        }
        napi_value newJsFunc = info.argv[INDEX_ZERO];
        std::lock_guard<std::mutex> lock(destroyedCallbackMutex_);
        for (const auto &cb : destroyCallback_) {
            napi_value jsFunc = nullptr;
            bool isEquals = false;
            napi_get_reference_value(env, cb.first, &jsFunc);
            napi_strict_equals(env, newJsFunc, jsFunc, &isEquals);
            if (isEquals) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "callback already exists");
                return CreateJsUndefined(env);
            }
        }
        napi_ref callbackRef = nullptr;
        napi_create_reference(env, newJsFunc, 1, &callbackRef);
        auto client = std::make_shared<JsPreloadUIExtensionCallbackClient>(env, callbackRef);
        int32_t key = PreloadUIExtensionHostClient::GetInstance()->AddDestroyCallback(client);
        destroyCallback_.push_back({ callbackRef, key });
        return CreateJsUndefined(env);
    }

    napi_value OnOffPreloadedUIExtensionAbilityDestroyed(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "OnOffPreloadedUIExtensionAbilityDestroyed called");
        if (!CheckPreloadUIExtensionAbilityPermission(env)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "no permission");
            return CreateJsUndefined(env);
        }
        if (info.argc == ARGC_ONE && !AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ZERO], napi_function)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid param");
            ThrowInvalidParamError(env, "Parse param callback failed, must be a "
                                        "PreloadedUIExtensionAbilityDestroyedFn.");
            return CreateJsUndefined(env);
        }
        std::lock_guard<std::mutex> lock(destroyedCallbackMutex_);
        if (info.argc == ARGC_ZERO) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "callback is undefined");
            if (destroyCallback_.empty()) {
                TAG_LOGW(AAFwkTag::ABILITYMGR, "null callback");
                return CreateJsUndefined(env);
            }
            PreloadUIExtensionHostClient::GetInstance()->RemoveAllDestroyCallback();
            destroyCallback_.clear();
            return CreateJsUndefined(env);
        }
        auto it = std::find_if(destroyCallback_.begin(), destroyCallback_.end(),
            [env, targetFunc = info.argv[INDEX_ZERO]](const auto &cb) {
            napi_value jsFunc = nullptr;
            bool isEquals = false;
            napi_get_reference_value(env, cb.first, &jsFunc);
            napi_strict_equals(env, targetFunc, jsFunc, &isEquals);
            return isEquals;
        });
        if (it == destroyCallback_.end()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "callback not found");
            return CreateJsUndefined(env);
        }
        PreloadUIExtensionHostClient::GetInstance()->RemoveDestroyCallback(it->second);
        destroyCallback_.erase(it);
        return CreateJsUndefined(env);
    }

    napi_value OnPreloadUIExtensionAbility(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "OnPreloadUIExtensionAbility called");
        HandleEscape handleEscape(env);
        if (info.argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        AAFwk::Want want;
        if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Parse want failed");
            ThrowInvalidParamError(env, "Parse param want failed, want must be Want.");
            return CreateJsUndefined(env);
        }
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> uAsyncTask =
            CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &result);
        std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uAsyncTask);
        PreloadTask task = [env, asyncTask](int32_t preloadId, int32_t innerErrCode) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "start async callback");
            HandleScope handleScope(env);
            if (innerErrCode == ERR_OK) {
                asyncTask->Resolve(env, CreateJsValue(env, preloadId));
            } else {
                asyncTask->Reject(env, CreateJsErrorByNativeErr(env, innerErrCode));
            }
        };
        auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
        if (context == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null context");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
        std::string bundleName = context->GetBundleName();
        PreloadUIExtensionHostClient::GetInstance()->PreloadUIExtensionAbility(want, bundleName, std::move(task));
        return handleEscape.Escape(result);
    }

    napi_value OnClearPreloadedUIExtensionAbility(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "OnClearPreloadedUIExtensionAbility called");
        HandleEscape handleEscape(env);
        if (info.argc < 1) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "params error: preloadId required");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        int32_t preloadId;
        if (!ConvertFromJsValue(env, info.argv[0], preloadId)) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "preloadId parse failed");
            ThrowInvalidParamError(env, "preloadId must be int");
            return CreateJsUndefined(env);
        }

        auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute = [preloadId, innerErrCode]() {
            *innerErrCode = AbilityManagerClient::GetInstance()->ClearPreloadedUIExtensionAbility(preloadId);
        };
        NapiAsyncTask::CompleteCallback complete = [innerErrCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            if (*innerErrCode == ERR_OK) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "OnClearPreloadedUIExtensionAbility failed %{public}d", *innerErrCode);
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            }
        };
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsApplicationContextUtils::OnClearPreloadedUIExtensionAbility", env,
            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnClearPreloadedUIExtensionAbilities(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "OnClearPreloadedUIExtensionAbilities called");
        HandleEscape handleEscape(env);
        auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute = [innerErrCode]() {
            *innerErrCode = AbilityManagerClient::GetInstance()->ClearPreloadedUIExtensionAbilities();
        };

        NapiAsyncTask::CompleteCallback complete = [innerErrCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            if (*innerErrCode == ERR_OK) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            }
        };
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsApplicationContextUtils::OnClearPreloadedUIExtensionAbilities", env,
            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }
};
} // namespace

napi_value JsAbilityManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");

    std::unique_ptr<JsAbilityManager> jsAbilityManager = std::make_unique<JsAbilityManager>();
    napi_wrap(env, exportObj, jsAbilityManager.release(), JsAbilityManager::Finalizer, nullptr, nullptr);

    napi_set_named_property(env, exportObj, "AbilityState", AbilityStateInit(env));
    napi_set_named_property(env, exportObj, "UserStatus", UserStatusInit(env));

    const char *moduleName = "JsAbilityManager";
    BindNativeFunction(env, exportObj, "getAbilityRunningInfos", moduleName,
        JsAbilityManager::GetAbilityRunningInfos);
    BindNativeFunction(env, exportObj, "getExtensionRunningInfos", moduleName,
        JsAbilityManager::GetExtensionRunningInfos);
    BindNativeFunction(env, exportObj, "updateConfiguration", moduleName, JsAbilityManager::UpdateConfiguration);
    BindNativeFunction(env, exportObj, "getTopAbility", moduleName, JsAbilityManager::GetTopAbility);
    BindNativeFunction(env, exportObj, "acquireShareData", moduleName, JsAbilityManager::AcquireShareData);
    BindNativeFunction(env, exportObj, "notifySaveAsResult", moduleName, JsAbilityManager::NotifySaveAsResult);
    BindNativeFunction(
        env, exportObj, "getForegroundUIAbilities", moduleName, JsAbilityManager::GetForegroundUIAbilities);
    BindNativeFunction(env, exportObj, "on", moduleName, JsAbilityManager::On);
    BindNativeFunction(env, exportObj, "off", moduleName, JsAbilityManager::Off);
    BindNativeFunction(env, exportObj, "isEmbeddedOpenAllowed", moduleName, JsAbilityManager::IsEmbeddedOpenAllowed);
    BindNativeFunction(
        env, exportObj, "isEmbeddedUIExtensionSupported", moduleName, JsAbilityManager::IsEmbeddedUIExtensionSupported);
    BindNativeFunction(
        env, exportObj, "restartSelfAtomicService", moduleName, JsAbilityManager::RestartSelfAtomicService);
    BindNativeFunction(
        env, exportObj, "notifyDebugAssertResult", moduleName, JsAbilityManager::NotifyDebugAssertResult);
    BindNativeFunction(
        env, exportObj, "setResidentProcessEnabled", moduleName, JsAbilityManager::SetResidentProcessEnabled);
    BindNativeFunction(
        env, exportObj, "queryAtomicServiceStartupRule", moduleName, JsAbilityManager::QueryAtomicServiceStartupRule);
    BindNativeFunction(env, exportObj, "onPreloadedUIExtensionAbilityLoaded", moduleName,
        JsAbilityManager::OnPreloadedUIExtensionAbilityLoaded);
    BindNativeFunction(env, exportObj, "offPreloadedUIExtensionAbilityLoaded", moduleName,
        JsAbilityManager::OffPreloadedUIExtensionAbilityLoaded);
    BindNativeFunction(env, exportObj, "onPreloadedUIExtensionAbilityDestroyed", moduleName,
        JsAbilityManager::OnPreloadedUIExtensionAbilityDestroyed);
    BindNativeFunction(env, exportObj, "offPreloadedUIExtensionAbilityDestroyed", moduleName,
        JsAbilityManager::OffPreloadedUIExtensionAbilityDestroyed);
    BindNativeFunction(
        env, exportObj, "preloadUIExtensionAbility", moduleName, JsAbilityManager::PreloadUIExtensionAbility);
    BindNativeFunction(env, exportObj, "clearPreloadedUIExtensionAbility", moduleName,
        JsAbilityManager::ClearPreloadedUIExtensionAbility);
    BindNativeFunction(env, exportObj, "clearPreloadedUIExtensionAbilities", moduleName,
        JsAbilityManager::ClearPreloadedUIExtensionAbilities);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "end");
    return CreateJsUndefined(env);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
