/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "js_app_manager.h"

#include <cstddef>
#include <cstdint>
#include <mutex>

#include "ability_manager_client.h"
#include "ability_manager_interface.h"
#include "ability_runtime_error_util.h"
#include "app_mgr_interface.h"
#include "application_info.h"
#include "event_runner.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "js_app_foreground_state_observer.h"
#include "js_app_manager_utils.h"
#include "js_app_state_observer.h"
#ifdef SUPPORT_GRAPHICS
#include "js_ability_first_frame_state_observer.h"
#ifdef SUPPORT_SCREEN
#include "tokenid_kit.h"
#endif
#endif
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_util.h"
#include "system_ability_definition.h"


namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr int32_t INDEX_THREE = 3;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr size_t ARGC_FOUR = 4;
constexpr const char* ON_OFF_TYPE = "applicationState";
constexpr const char* ON_OFF_TYPE_SYNC = "applicationStateEvent";
constexpr const char* ON_OFF_TYPE_APP_FOREGROUND_STATE = "appForegroundState";
constexpr const char* ON_OFF_TYPE_ABILITY_FIRST_FRAME_STATE = "abilityFirstFrameState";

class JsAppManager final {
public:
    JsAppManager(sptr<OHOS::AppExecFwk::IAppMgr> appManager,
        sptr<OHOS::AAFwk::IAbilityManager> abilityManager) : appManager_(appManager),
        abilityManager_(abilityManager) {}
    ~JsAppManager()
    {
        if (observer_ != nullptr) {
            TAG_LOGI(AAFwkTag::APPMGR, "set valid false");
            observer_->SetValid(false);
        }
        if (observerForeground_ != nullptr) {
            observerForeground_->SetValid(false);
        }
    }

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGI(AAFwkTag::APPMGR, "finalizer called");
        std::unique_ptr<JsAppManager>(static_cast<JsAppManager*>(data));
    }

    static napi_value On(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnOn);
    }

    static napi_value Off(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnOff);
    }

    static napi_value GetForegroundApplications(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnGetForegroundApplications);
    }

    static napi_value GetRunningProcessInformation(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnGetRunningProcessInformation);
    }

    static napi_value GetRunningProcessInformationByBundleType(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnGetRunningProcessInformationByBundleType);
    }

    static napi_value IsRunningInStabilityTest(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnIsRunningInStabilityTest);
    }

    static napi_value GetRunningMultiAppInfo(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnGetRunningMultiAppInfo);
    }

    static napi_value KillProcessWithAccount(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnKillProcessWithAccount);
    }

    static napi_value KillProcessesByBundleName(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnKillProcessesByBundleName);
    }

    static napi_value ClearUpApplicationData(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnClearUpApplicationData);
    }

    static napi_value ClearUpAppData(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnClearUpAppData);
    }

    static napi_value TerminateMission(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnTerminateMission);
    }

    static napi_value IsSharedBundleRunning(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnIsSharedBundleRunning);
    }

    static napi_value GetAppMemorySize(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnGetAppMemorySize);
    }

    static napi_value IsRamConstrainedDevice(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnIsRamConstrainedDevice);
    }

    static napi_value GetProcessMemoryByPid(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnGetProcessMemoryByPid);
    }

    static napi_value GetRunningProcessInfoByBundleName(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnGetRunningProcessInfoByBundleName);
    }

    static napi_value IsApplicationRunning(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnIsApplicationRunning);
    }

    static napi_value IsAppRunning(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnIsAppRunning);
    }
#ifdef SUPPORT_SCREEN
    static bool CheckCallerIsSystemApp()
    {
        auto selfToken = IPCSkeleton::GetSelfTokenID();
        if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
            return false;
        }
        return true;
    }
#endif

    static bool IsParasNullOrUndefined(napi_env env, const napi_value& para)
    {
        return AppExecFwk::IsTypeForNapiValue(env, para, napi_null) ||
            AppExecFwk::IsTypeForNapiValue(env, para, napi_undefined);
    }

    static bool IsJSFunctionExist(napi_env env, const napi_value para, const std::string& methodName)
    {
        if (para == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null param");
            return false;
        }
        napi_ref ref = nullptr;
        napi_create_reference(env, para, 1, &ref);
        NativeReference* nativeReference = reinterpret_cast<NativeReference *>(ref);
        auto object = nativeReference->GetNapiValue();
        napi_value method = nullptr;
        napi_get_named_property(env, object, methodName.c_str(), &method);
        if (method == nullptr) {
            napi_delete_reference(env, ref);
            TAG_LOGE(AAFwkTag::APPMGR, "Get name from object Failed.");
            return false;
        }
        if (!AppExecFwk::IsTypeForNapiValue(env, method, napi_function)) {
            napi_delete_reference(env, ref);
            TAG_LOGE(AAFwkTag::APPMGR, "Illegal type not a function.");
            return false;
        }
        napi_delete_reference(env, ref);
        return true;
    }

    static napi_value PreloadApplication(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnPreloadApplication);
    }

private:
    sptr<OHOS::AppExecFwk::IAppMgr> appManager_ = nullptr;
    sptr<OHOS::AAFwk::IAbilityManager> abilityManager_ = nullptr;
    sptr<JSAppStateObserver> observer_ = nullptr;
    sptr<JSAppStateObserver> observerSync_ = nullptr;
    sptr<JSAppForegroundStateObserver> observerForeground_ = nullptr;
    int32_t serialNumber_ = 0;

    napi_value OnOn(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        std::string type = ParseParamType(env, argc, argv);
        if (type == ON_OFF_TYPE_SYNC) {
            return OnOnNew(env, argc, argv);
        } else if (type == ON_OFF_TYPE_APP_FOREGROUND_STATE) {
            return OnOnForeground(env, argc, argv);
        } else if (type == ON_OFF_TYPE_ABILITY_FIRST_FRAME_STATE) {
#ifdef SUPPORT_SCREEN
            return OnOnAbilityFirstFrameState(env, argc, argv);
#else
            TAG_LOGE(AAFwkTag::APPMGR, "not support");
            return CreateJsUndefined(env);
#endif
        }

        return OnOnOld(env, argc, argv);
    }

    napi_value OnOnOld(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        if (argc < ARGC_TWO) { // support 2 or 3 params, if > 3 params, ignore other params
            TAG_LOGE(AAFwkTag::APPMGR, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        if (!CheckOnOffType(env, argc, argv)) {
            ThrowInvalidParamError(env, "Parse param type failed, must be a string,"
                "value must be applicationState, appForegroundState or abilityFirstFrameState.");
            return CreateJsUndefined(env);
        }

        if (appManager_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "appManager nullptr");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }

        std::vector<std::string> bundleNameList;
        // unwarp observer
        if (observer_ == nullptr) {
            observer_ = new JSAppStateObserver(env);
        }
        if (argc > ARGC_TWO) {
            AppExecFwk::UnwrapArrayStringFromJS(env, argv[INDEX_TWO], bundleNameList);
        }
        int32_t ret = appManager_->RegisterApplicationStateObserver(observer_, bundleNameList);
        if (ret == 0) {
            TAG_LOGD(AAFwkTag::APPMGR, "success.");
            int64_t observerId = serialNumber_;
            observer_->AddJsObserverObject(observerId, argv[INDEX_ONE]);
            if (serialNumber_ < INT32_MAX) {
                serialNumber_++;
            } else {
                serialNumber_ = 0;
            }
            return CreateJsValue(env, observerId);
        } else {
            TAG_LOGE(AAFwkTag::APPMGR, "wrong error:%{public}d.", ret);
            ThrowErrorByNativeErr(env, ret);
            return CreateJsUndefined(env);
        }
    }

    napi_value OnOnNew(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        if (argc < ARGC_TWO) { // support 2 or 3 params, if > 3 params, ignore other params
            TAG_LOGE(AAFwkTag::APPMGR, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        if (!AppExecFwk::IsTypeForNapiValue(env, argv[INDEX_ONE], napi_object)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Invalid param");
            ThrowInvalidParamError(env, "Parse param observer failed, must be a ApplicationStateObserver.");
            return CreateJsUndefined(env);
        }
        std::vector<std::string> bundleNameList;
        if (argc > ARGC_TWO) {
            AppExecFwk::UnwrapArrayStringFromJS(env, argv[INDEX_TWO], bundleNameList);
        }
        if (observerSync_ == nullptr) {
            observerSync_ = new JSAppStateObserver(env);
        }
        if (appManager_ == nullptr || observerSync_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "appManager or observer is nullptr");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
        int32_t ret = appManager_->RegisterApplicationStateObserver(observerSync_, bundleNameList);
        if (ret == 0) {
            TAG_LOGD(AAFwkTag::APPMGR, "success.");
            int32_t observerId = serialNumber_;
            observerSync_->AddJsObserverObject(observerId, argv[INDEX_ONE]);
            if (serialNumber_ < INT32_MAX) {
                serialNumber_++;
            } else {
                serialNumber_ = 0;
            }
            return CreateJsValue(env, observerId);
        } else {
            TAG_LOGE(AAFwkTag::APPMGR, "Wrong error:%{public}d.", ret);
            ThrowErrorByNativeErr(env, ret);
            return CreateJsUndefined(env);
        }
    }

    napi_value OnOnForeground(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        if (argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        if (!AppExecFwk::IsTypeForNapiValue(env, argv[INDEX_ONE], napi_object)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Invalid param.");
            ThrowInvalidParamError(env, "Parse param observer failed, must be a AppForegroundStateObserver.");
            return CreateJsUndefined(env);
        }
        if (observerForeground_ == nullptr) {
            observerForeground_ = new (std::nothrow) JSAppForegroundStateObserver(env);
        }

        if (appManager_ == nullptr || observerForeground_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null appManager or observer");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }

        if (observerForeground_->IsEmpty()) {
            int32_t ret = appManager_->RegisterAppForegroundStateObserver(observerForeground_);
            if (ret != NO_ERROR) {
                TAG_LOGE(AAFwkTag::APPMGR, "Failed error: %{public}d.", ret);
                ThrowErrorByNativeErr(env, ret);
                return CreateJsUndefined(env);
            }
        }
        observerForeground_->AddJsObserverObject(argv[INDEX_ONE]);
        return CreateJsUndefined(env);
    }

    napi_value OnOff(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        std::string type = ParseParamType(env, argc, argv);
        if (type == ON_OFF_TYPE_SYNC) {
            return OnOffNew(env, argc, argv);
        } else if (type == ON_OFF_TYPE_APP_FOREGROUND_STATE) {
            return OnOffForeground(env, argc, argv);
        } else if (type == ON_OFF_TYPE_ABILITY_FIRST_FRAME_STATE) {
#ifdef SUPPORT_SCREEN
            return OnOffAbilityFirstFrameState(env, argc, argv);
#else
            TAG_LOGE(AAFwkTag::APPMGR, "not support");
            return CreateJsUndefined(env);
#endif
        }

        return OnOffOld(env, argc, argv);
    }

#ifdef SUPPORT_SCREEN
    napi_value OnOnAbilityFirstFrameState(napi_env env, size_t argc, napi_value *argv)
    {
        if (!CheckCallerIsSystemApp()) {
            TAG_LOGE(AAFwkTag::APPMGR, "Current app is not system app");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
            return CreateJsUndefined(env);
        }
        JSAbilityFirstFrameStateObserverManager::GetInstance()->Init(env);
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        if (argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        if (!AppExecFwk::IsTypeForNapiValue(env, argv[INDEX_ONE], napi_object) ||
            !IsJSFunctionExist(env, argv[INDEX_ONE], "onAbilityFirstFrameDrawn")) {
            TAG_LOGE(AAFwkTag::APPMGR, "Invalid param.");
            ThrowInvalidParamError(env, "Parse param observer failed, must be a AbilityFirstFrameStateObserver.");
            return CreateJsUndefined(env);
        }
        std::string bundleName;
        if (argc == ARGC_THREE) {
            if (!IsParasNullOrUndefined(env, argv[INDEX_TWO]) &&
                (!ConvertFromJsValue(env, argv[INDEX_TWO], bundleName) || bundleName.empty())) {
                TAG_LOGE(AAFwkTag::APPMGR, "Get bundleName error or bundleName empty!");
                ThrowInvalidParamError(env, "Parse param bundleName failed, must be a string.");
                return CreateJsUndefined(env);
            }
        }

        sptr<JSAbilityFirstFrameStateObserver> observer = new (std::nothrow) JSAbilityFirstFrameStateObserver(env);
        if (abilityManager_ == nullptr || observer == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "AbilityManager_ or observer is nullptr.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }

        if (JSAbilityFirstFrameStateObserverManager::GetInstance()->IsObserverObjectExist(argv[INDEX_ONE])) {
            TAG_LOGE(AAFwkTag::APPMGR, "observer exist");
            return CreateJsUndefined(env);
        }
        int32_t ret = abilityManager_->RegisterAbilityFirstFrameStateObserver(observer, bundleName);
        if (ret != NO_ERROR) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed error: %{public}d.", ret);
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
        observer->SetJsObserverObject(argv[INDEX_ONE]);
        JSAbilityFirstFrameStateObserverManager::GetInstance()->AddJSAbilityFirstFrameStateObserver(observer);
        return CreateJsUndefined(env);
    }

    napi_value OnOffAbilityFirstFrameState(napi_env env, size_t argc, napi_value *argv)
    {
        if (!CheckCallerIsSystemApp()) {
            TAG_LOGE(AAFwkTag::APPMGR, "Current app is not system app");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
            return CreateJsUndefined(env);
        }
        JSAbilityFirstFrameStateObserverManager::GetInstance()->Init(env);
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        if (argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        if (argc == ARGC_TWO) {
            if (!IsParasNullOrUndefined(env, argv[INDEX_ONE]) &&
                (!AppExecFwk::IsTypeForNapiValue(env, argv[INDEX_ONE], napi_object) ||
                !IsJSFunctionExist(env, argv[INDEX_ONE], "onAbilityFirstFrameDrawn"))) {
                TAG_LOGE(AAFwkTag::APPMGR, "Invalid param.");
                ThrowInvalidParamError(env, "Parse param observer failed, must be a AbilityFirstFrameStateObserver.");
                return CreateJsUndefined(env);
            }
        }

        if (argc == ARGC_ONE || (argc == ARGC_TWO && IsParasNullOrUndefined(env, argv[INDEX_ONE]))) {
            JSAbilityFirstFrameStateObserverManager::GetInstance()->RemoveAllJsObserverObjects(abilityManager_);
        } else if (argc == ARGC_TWO) {
            JSAbilityFirstFrameStateObserverManager::GetInstance()->RemoveJsObserverObject(abilityManager_,
                argv[INDEX_ONE]);
        }
        return CreateJsUndefined(env);
    }
#endif
    static void OnOffOldInner(sptr<OHOS::AppExecFwk::IAppMgr> appManager, sptr<JSAppStateObserver> observer,
        int64_t observerId, napi_env env, NapiAsyncTask *task)
    {
        if (observer == nullptr || appManager == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null observer or appManager");
            task->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
            return;
        }
        int32_t ret = appManager->UnregisterApplicationStateObserver(observer);
        if (ret == 0 && observer->RemoveJsObserverObject(observerId)) {
            task->ResolveWithNoError(env, CreateJsUndefined(env));
            TAG_LOGD(AAFwkTag::APPMGR, "success size:%{public}zu", observer->GetJsObserverMapSize());
        } else {
            TAG_LOGE(AAFwkTag::APPMGR, "failed error:%{public}d", ret);
            task->Reject(env, CreateJsErrorByNativeErr(env, ret));
        }
    }

    napi_value OnOffOld(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        if (argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        if (!CheckOnOffType(env, argc, argv)) {
            ThrowInvalidParamError(env, "Parse param type failed, must be a string,"
                "value must be applicationState, appForegroundState or abilityFirstFrameState.");
            return CreateJsUndefined(env);
        }

        int64_t observerId = -1;
        napi_get_value_int64(env, argv[INDEX_ONE], &observerId);
        if (observer_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "observer is nullptr, please register first.");
            ThrowInvalidParamError(env, "observer is nullptr, please register first.");
            return CreateJsUndefined(env);
        }
        if (!observer_->FindObserverByObserverId(observerId)) {
            TAG_LOGE(AAFwkTag::APPMGR, "not find observer, observer:%{public}d", static_cast<int32_t>(observerId));
            ThrowInvalidParamError(env, "not find observerId.");
            return CreateJsUndefined(env);
        }
        TAG_LOGD(AAFwkTag::APPMGR, "find observer exist observer:%{public}d", static_cast<int32_t>(observerId));

        napi_value lastParam = (argc > ARGC_TWO) ? argv[INDEX_TWO] : nullptr;
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [appManager = appManager_, observer = observer_, observerId,
            env, task = napiAsyncTask.get()]() {
            OnOffOldInner(appManager, observer, observerId, env, task);
            delete task;
        };
        if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
            napiAsyncTask->Reject(env, CreateJsErrorByNativeErr(env,
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "send event failed!"));
        } else {
            napiAsyncTask.release();
        }
        return result;
    }

    napi_value OnOffNew(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        if (argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        int32_t observerId = -1;
        if (!ConvertFromJsValue(env, argv[INDEX_ONE], observerId)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Parse observerId failed");
            ThrowInvalidParamError(env, "Parse param observerId failed, must be a number.");
            return CreateJsUndefined(env);
        }

        if (observerSync_ == nullptr || appManager_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "observer or appManager nullptr");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
        if (!observerSync_->FindObserverByObserverId(observerId)) {
            TAG_LOGE(AAFwkTag::APPMGR, "not find observer, observer:%{public}d", static_cast<int32_t>(observerId));
            ThrowInvalidParamError(env, "not find observerId.");
            return CreateJsUndefined(env);
        }
        int32_t ret = appManager_->UnregisterApplicationStateObserver(observerSync_);
        if (ret == 0 && observerSync_->RemoveJsObserverObject(observerId)) {
            TAG_LOGD(AAFwkTag::APPMGR, "success size:%{public}zu", observerSync_->GetJsObserverMapSize());
            return CreateJsUndefined(env);
        } else {
            TAG_LOGE(AAFwkTag::APPMGR, "failed error:%{public}d", ret);
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
    }

    napi_value OnOffForeground(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        if (argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        if (argc == ARGC_TWO && !AppExecFwk::IsTypeForNapiValue(env, argv[INDEX_ONE], napi_object)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Invalid param.");
            ThrowInvalidParamError(env, "Parse param observer failed, must be a AppForegroundStateObserver.");
            return CreateJsUndefined(env);
        }
        if (observerForeground_ == nullptr || appManager_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "Observer or appManager nullptr.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }

        if (argc == ARGC_ONE) {
            observerForeground_->RemoveAllJsObserverObjects();
        } else if (argc == ARGC_TWO) {
            observerForeground_->RemoveJsObserverObject(argv[INDEX_ONE]);
        }
        if (observerForeground_->IsEmpty()) {
            int32_t ret = appManager_->UnregisterAppForegroundStateObserver(observerForeground_);
            if (ret != NO_ERROR) {
                TAG_LOGE(AAFwkTag::APPMGR, "Failed error: %{public}d.", ret);
                ThrowErrorByNativeErr(env, ret);
                return CreateJsUndefined(env);
            }
        }
        return CreateJsUndefined(env);
    }

    napi_value OnGetForegroundApplications(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");

        napi_value lastParam = (argc > ARGC_ZERO) ? argv[INDEX_ZERO] : nullptr;
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [appManager = appManager_, env, task = napiAsyncTask.get()]() {
            if (appManager == nullptr) {
                TAG_LOGE(AAFwkTag::APPMGR, "appManager nullptr");
                task->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                delete task;
                return;
            }
            std::vector<AppExecFwk::AppStateData> list;
            int32_t ret = appManager->GetForegroundApplications(list);
            if (ret == 0) {
                TAG_LOGD(AAFwkTag::APPMGR, "success.");
                task->ResolveWithNoError(env, CreateJsAppStateDataArray(env, list));
            } else {
                TAG_LOGE(AAFwkTag::APPMGR, "failed error:%{public}d", ret);
                task->Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(ret)));
            }
            delete task;
        };
        if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
            napiAsyncTask->Reject(env, CreateJsErrorByNativeErr(env,
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "send event failed!"));
        } else {
            napiAsyncTask.release();
        }
        return result;
    }

    napi_value OnGetRunningProcessInformation(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");

        napi_value lastParam = (argc > ARGC_ZERO) ? argv[INDEX_ZERO] : nullptr;
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [appManager = appManager_, env, task = napiAsyncTask.get()]() {
            if (appManager == nullptr) {
                TAG_LOGW(AAFwkTag::APPMGR, "appManager nullptr");
                task->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                delete task;
                return;
            }
            std::vector<AppExecFwk::RunningProcessInfo> infos;
            auto ret = appManager->GetAllRunningProcesses(infos);
            if (ret == 0) {
                task->ResolveWithNoError(env, CreateJsRunningProcessInfoArray(env, infos));
            } else {
                task->Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(ret)));
            }
            delete task;
        };
        if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
            napiAsyncTask->Reject(env, CreateJsErrorByNativeErr(env,
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "send event failed!"));
        } else {
            napiAsyncTask.release();
        }
        return result;
    }

    napi_value OnGetRunningMultiAppInfo(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
#ifdef SUPPORT_SCREEN
        if (!CheckCallerIsSystemApp()) {
            TAG_LOGE(AAFwkTag::APPMGR, "Current app is not system app");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
            return CreateJsUndefined(env);
        }
#endif
        // only support 1 params
        if (argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        std::string bundleName;
        if (!ConvertFromJsValue(env, argv[0], bundleName) || bundleName.empty()) {
            TAG_LOGE(AAFwkTag::APPMGR, "get bundleName failed!");
            ThrowInvalidParamError(env, "Parse param bundleName failed, must be a string.");
            return CreateJsUndefined(env);
        }
        auto info = std::make_shared<RunningMultiAppInfo>();
        auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute =
            [appManager = appManager_, bundleName, innerErrorCode, info]() {
                if (appManager == nullptr) {
                    TAG_LOGW(AAFwkTag::APPMGR, "appManager nullptr");
                    *innerErrorCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
                    return;
                }
                *innerErrorCode = appManager->GetRunningMultiAppInfoByBundleName(bundleName, *info);
            };
        NapiAsyncTask::CompleteCallback complete =
            [innerErrorCode, info](napi_env env, NapiAsyncTask &task, int32_t status) {
                if (*innerErrorCode == ERR_OK) {
                    task.ResolveWithNoError(env, CreateJsRunningMultiAppInfo(env, *info));
                } else {
                    task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
                }
            };
        napi_value lastParam = nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSAppManager::OnGetRunningMultiAppInfo",
            env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return result;
    }

    napi_value OnGetRunningProcessInformationByBundleType(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        if (argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        int32_t bundleType = -1;
        if (!ConvertFromJsValue(env, argv[INDEX_ZERO], bundleType)) {
            TAG_LOGE(AAFwkTag::APPMGR, "get bundleType error");
            ThrowInvalidParamError(env, "Parse param bundleType failed, must be a BundleType.");
            return CreateJsUndefined(env);
        }
        if (bundleType < 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "Invalid bundle type:%{public}d", bundleType);
            ThrowInvalidParamError(env, "Parse param bundleType failed, must not be less then zero.");
            return CreateJsUndefined(env);
        }

        napi_value lastParam = (argc > ARGC_ONE) ? argv[INDEX_ONE] : nullptr;
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [appManager = appManager_, bundleType, env, task = napiAsyncTask.get()]() {
            if (appManager == nullptr) {
                TAG_LOGW(AAFwkTag::APPMGR, "appManager nullptr");
                task->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                delete task;
                return;
            }
            std::vector<AppExecFwk::RunningProcessInfo> infos;
            auto ret = appManager->GetRunningProcessesByBundleType(
                static_cast<AppExecFwk::BundleType>(bundleType), infos);
            if (ret == 0) {
                task->ResolveWithNoError(env, CreateJsRunningProcessInfoArray(env, infos));
            } else {
                task->Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(ret)));
            }
            delete task;
        };
        if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
            napiAsyncTask->Reject(env, CreateJsErrorByNativeErr(env,
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "send event failed!"));
        } else {
            napiAsyncTask.release();
        }
        return result;
    }

    napi_value OnIsRunningInStabilityTest(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        napi_value lastParam = (argc > ARGC_ZERO) ? argv[INDEX_ZERO] : nullptr;
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [abilityManager = abilityManager_, env, task = napiAsyncTask.get()]() {
            if (abilityManager == nullptr) {
                TAG_LOGW(AAFwkTag::APPMGR, "abilityManager nullptr");
                task->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                delete task;
                return;
            }
            bool ret = abilityManager->IsRunningInStabilityTest();
            TAG_LOGD(AAFwkTag::APPMGR, "result:%{public}d", ret);
            task->ResolveWithNoError(env, CreateJsValue(env, ret));
            delete task;
        };
        if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
            napiAsyncTask->Reject(env, CreateJsErrorByNativeErr(env,
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "send event failed!"));
        } else {
            napiAsyncTask.release();
        }
        return result;
    }

    static void OnKillProcessesByBundleNameInner(std::string bundleName, bool clearPageStack,
        sptr<OHOS::AAFwk::IAbilityManager> abilityManager, napi_env env, NapiAsyncTask *task)
    {
        if (abilityManager == nullptr) {
            TAG_LOGW(AAFwkTag::APPMGR, "abilityManager nullptr");
            task->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
            return;
        }
        auto ret = abilityManager->KillProcess(bundleName, clearPageStack);
        if (ret == 0) {
            task->ResolveWithNoError(env, CreateJsUndefined(env));
        } else {
            task->Reject(env, CreateJsErrorByNativeErr(env, ret, "kill process failed."));
        }
    }
    napi_value OnKillProcessesByBundleName(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        if (argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::APPMGR, "Params not match");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        std::string bundleName;
        if (!ConvertFromJsValue(env, argv[INDEX_ZERO], bundleName)) {
            TAG_LOGE(AAFwkTag::APPMGR, "get bundleName error!");
            ThrowInvalidParamError(env, "Parse param bundleName failed, must be a string.");
            return CreateJsUndefined(env);
        }
        bool clearPageStack = false;
        bool hasClearPageStack = false;
        if (argc > ARGC_ONE && ConvertFromJsValue(env, argv[INDEX_ONE], clearPageStack)) {
            hasClearPageStack = true;
        }
        int32_t appIndex = 0;
        if (hasClearPageStack && argc == ARGC_THREE && !ConvertFromJsValue(env, argv[INDEX_TWO], appIndex)) {
            TAG_LOGE(AAFwkTag::APPMGR, "get appIndex failed!");
            ThrowInvalidParamError(env, "Parse param appIndex failed, must be a number.");
            return CreateJsUndefined(env);
        }
        TAG_LOGI(AAFwkTag::APPMGR,
            "kill [%{public}s], hasClearPageStack [%{public}d], clearPageStack [%{public}d],appIndex [%{public}d]",
            bundleName.c_str(), hasClearPageStack, clearPageStack, appIndex);
        napi_value lastParam = (argc == ARGC_TWO && !hasClearPageStack) ? argv[INDEX_ONE] : nullptr;
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [bundleName, clearPageStack, abilityManager = abilityManager_,
            env, task = napiAsyncTask.get()]() {
            OnKillProcessesByBundleNameInner(bundleName, clearPageStack, abilityManager, env, task);
            delete task;
        };
        if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_immediate)) {
            napiAsyncTask->Reject(env, CreateJsErrorByNativeErr(env,
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "send event failed!"));
        } else {
            napiAsyncTask.release();
        }
        return result;
    }

    napi_value OnClearUpApplicationData(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        if (argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::APPMGR, "arguments not match");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string bundleName;
        if (!ConvertFromJsValue(env, argv[0], bundleName)) {
            TAG_LOGE(AAFwkTag::APPMGR, "get bundleName failed!");
            ThrowInvalidParamError(env, "Parse param bundleName failed, must be a string.");
            return CreateJsUndefined(env);
        }

        napi_value lastParam = (argc == ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [bundleName, appManager = appManager_, env, task = napiAsyncTask.get()]() {
            if (appManager == nullptr) {
                TAG_LOGW(AAFwkTag::APPMGR, "appManager nullptr");
                task->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                delete task;
                return;
            }
            auto ret = appManager->ClearUpApplicationData(bundleName, 0);
            if (ret == 0) {
                task->ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task->Reject(env, CreateJsErrorByNativeErr(env, ret, "clear up application failed."));
            }
            delete task;
        };
        if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
            napiAsyncTask->Reject(env, CreateJsErrorByNativeErr(env,
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "send event failed!"));
        } else {
            napiAsyncTask.release();
        }
        return result;
    }

    napi_value OnClearUpAppData(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        if (argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::APPMGR, "arguments not match");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string bundleName;
        if (!ConvertFromJsValue(env, argv[0], bundleName) || bundleName.empty()) {
            TAG_LOGE(AAFwkTag::APPMGR, "get bundleName failed!");
            ThrowInvalidParamError(env, "Parse param bundleName failed, must be a string");
            return CreateJsUndefined(env);
        }
        int32_t appCloneIndex = 0;
        if (argc > ARGC_ONE && !ConvertFromJsValue(env, argv[1], appCloneIndex)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Get appCloneIndex wrong.");
            ThrowInvalidParamError(env, "Parse param appCloneIndex failed, must be a string");
            return CreateJsUndefined(env);
        }

        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
        auto asyncTask = [bundleName, appCloneIndex, appManager = appManager_, env, task = napiAsyncTask.get()]() {
            if (appManager == nullptr) {
                TAG_LOGW(AAFwkTag::APPMGR, "appManager nullptr");
                task->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                delete task;
                return;
            }
            auto ret = appManager->ClearUpApplicationData(bundleName, appCloneIndex);
            if (ret == 0) {
                task->ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task->Reject(env, CreateJsErrorByNativeErr(env, ret, "clear up application failed."));
            }
            delete task;
        };
        if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
            napiAsyncTask->Reject(env, CreateJsErrorByNativeErr(env,
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "send event failed!"));
        } else {
            napiAsyncTask.release();
        }
        return result;
    }

    napi_value OnTerminateMission(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "OnTerminateMission call.");
        if (argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::APPMGR, "Params not match");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        
        int32_t missionId = 0;
        if (!ConvertFromJsValue(env, argv[INDEX_ZERO], missionId)) {
            TAG_LOGE(AAFwkTag::APPMGR, "get missionId wrong!");
            ThrowInvalidParamError(env, "Parse param missionId failed, must be a number.");
            return CreateJsUndefined(env);
        }

        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, nullptr, &result);
        auto asyncTask = [missionId, env, task = napiAsyncTask.get()]() {
            auto amsClient = AAFwk::AbilityManagerClient::GetInstance();
            if (amsClient == nullptr) {
                TAG_LOGW(AAFwkTag::APPMGR, "amsClient nullptr");
                task->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                delete task;
                return;
            }
            auto ret = amsClient->TerminateMission(missionId);
            (ret == ERR_OK) ? task->ResolveWithNoError(env, CreateJsUndefined(env)) :
                task->Reject(env, CreateJsErrorByNativeErr(env, ret, "Terminate mission failed."));
            delete task;
        };
        if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_high)) {
            napiAsyncTask->Reject(env, CreateJsErrorByNativeErr(env,
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "Terminate mission failed."));
        } else {
            napiAsyncTask.release();
        }
        return result;
    }

    napi_value OnIsSharedBundleRunning(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        if (argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::APPMGR, "Params not match");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string bundleName;
        if (!ConvertFromJsValue(env, argv[0], bundleName)) {
            TAG_LOGE(AAFwkTag::APPMGR, "get bundleName wrong!");
            ThrowInvalidParamError(env, "Parse param bundleName failed, must be a string.");
            return CreateJsUndefined(env);
        }

        uint32_t versionCode = 0;
        if (!ConvertFromJsValue(env, argv[1], versionCode)) {
            TAG_LOGE(AAFwkTag::APPMGR, "get versionCode failed!");
            ThrowInvalidParamError(env, "Parse param versionCode failed, must be a number.");
            return CreateJsUndefined(env);
        }

        napi_value lastParam = (argc == ARGC_THREE) ? argv[INDEX_TWO] : nullptr;
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [bundleName, versionCode, appManager = appManager_, env, task = napiAsyncTask.get()]() {
            if (appManager == nullptr) {
                TAG_LOGW(AAFwkTag::APPMGR, "appManager nullptr");
                task->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                delete task;
                return;
            }
            bool ret = appManager->IsSharedBundleRunning(bundleName, versionCode);
            TAG_LOGI(AAFwkTag::APPMGR, "result:%{public}d", ret);
            task->ResolveWithNoError(env, CreateJsValue(env, ret));
            delete task;
        };
        if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_immediate)) {
            napiAsyncTask->Reject(env, CreateJsErrorByNativeErr(env,
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "send event failed!"));
        } else {
            napiAsyncTask.release();
        }
        return result;
    }

    napi_value OnKillProcessWithAccount(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        if (argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::APPMGR, "Params not match");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        std::string bundleName;
        if (!ConvertFromJsValue(env, argv[INDEX_ZERO], bundleName)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Parse bundleName failed");
            ThrowInvalidParamError(env, "Parse param bundleName failed, must be a string.");
            return CreateJsUndefined(env);
        }
        int32_t accountId = -1;
        if (!ConvertFromJsValue(env, argv[INDEX_ONE], accountId)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Parse userId failed");
            ThrowInvalidParamError(env, "Parse param accountId failed, must be a number.");
            return CreateJsUndefined(env);
        }
        bool clearPageStack = false;
        bool hasClearPageStack = false;
        if (argc > ARGC_TWO && ConvertFromJsValue(env, argv[INDEX_TWO], clearPageStack)) {
            hasClearPageStack = true;
        }
        int32_t appIndex = 0;
        if (hasClearPageStack && argc == ARGC_FOUR && !ConvertFromJsValue(env, argv[INDEX_THREE], appIndex)) {
            ThrowInvalidParamError(env, "Parse param appIndex failed, must be a number.");
            return CreateJsUndefined(env);
        }
        TAG_LOGI(AAFwkTag::APPMGR,
            "kill [%{public}s], hasClearPageStack [%{public}d], clearPageStack [%{public}d],appIndex [%{public}d]",
            bundleName.c_str(), hasClearPageStack, clearPageStack, appIndex);
        napi_value lastParam = (argc == ARGC_THREE && !hasClearPageStack) ? argv[INDEX_TWO] : nullptr;
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [appManager = appManager_, bundleName, accountId, clearPageStack,
            env, task = napiAsyncTask.get()]() {
            if (appManager == nullptr || appManager->GetAmsMgr() == nullptr) {
                TAG_LOGW(AAFwkTag::APPMGR, "appManager is nullptr or amsMgr is nullptr.");
                task->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                delete task;
                return;
            }
            auto ret = appManager->GetAmsMgr()->KillProcessWithAccount(bundleName, accountId, clearPageStack);
            if (ret == 0) {
                task->ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task->Reject(env, CreateJsErrorByNativeErr(env, ret, "Kill processes failed."));
            }
            delete task;
        };
        if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_immediate)) {
            napiAsyncTask->Reject(env, CreateJsErrorByNativeErr(env,
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "send event failed!"));
        } else {
            napiAsyncTask.release();
        }
        return result;
    }

    napi_value OnGetAppMemorySize(napi_env env, size_t argc, napi_value* argv)
    {
        napi_value lastParam = (argc > ARGC_ZERO) ? argv[INDEX_ZERO] : nullptr;
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [abilityManager = abilityManager_, env, task = napiAsyncTask.get()]() {
            if (abilityManager == nullptr) {
                TAG_LOGW(AAFwkTag::APPMGR, "abilityManager nullptr");
                task->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                delete task;
                return;
            }
            int32_t memorySize = abilityManager->GetAppMemorySize();
            TAG_LOGI(AAFwkTag::APPMGR, "memorySize:%{public}d", memorySize);
            task->ResolveWithNoError(env, CreateJsValue(env, memorySize));
            delete task;
        };
        if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_immediate)) {
            napiAsyncTask->Reject(env, CreateJsErrorByNativeErr(env,
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "send event failed!"));
        } else {
            napiAsyncTask.release();
        }
        return result;
    }

    napi_value OnIsRamConstrainedDevice(napi_env env, size_t argc, napi_value* argv)
    {
        napi_value lastParam = (argc > ARGC_ZERO) ? argv[INDEX_ZERO] : nullptr;
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [abilityManager = abilityManager_, env, task = napiAsyncTask.get()]() {
            if (abilityManager == nullptr) {
                TAG_LOGW(AAFwkTag::APPMGR, "abilityManager nullptr");
                task->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                delete task;
                return;
            }
            bool ret = abilityManager->IsRamConstrainedDevice();
            TAG_LOGI(AAFwkTag::APPMGR, "result:%{public}d", ret);
            task->ResolveWithNoError(env, CreateJsValue(env, ret));
            delete task;
        };
        if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_immediate)) {
            napiAsyncTask->Reject(env, CreateJsErrorByNativeErr(env,
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "send event failed!"));
        } else {
            napiAsyncTask.release();
        }
        return result;
    }

    napi_value OnGetProcessMemoryByPid(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        if (argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::APPMGR, "Params not match");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        int32_t pid;
        if (!ConvertFromJsValue(env, argv[0], pid)) {
            TAG_LOGE(AAFwkTag::APPMGR, "get pid failed");
            ThrowInvalidParamError(env, "Parse param pid failed, must be a number.");
            return CreateJsUndefined(env);
        }

        napi_value lastParam = (argc == ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [pid, appManager = appManager_, env, task = napiAsyncTask.get()]() {
            if (appManager == nullptr) {
                TAG_LOGW(AAFwkTag::APPMGR, "appManager is nullptr");
                task->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                delete task;
                return;
            }
            int32_t memSize = 0;
            int32_t ret = appManager->GetProcessMemoryByPid(pid, memSize);
            if (ret == 0) {
                task->ResolveWithNoError(env, CreateJsValue(env, memSize));
            } else {
                task->Reject(env, CreateJsErrorByNativeErr(env, ret));
            }
            delete task;
        };
        if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_immediate)) {
            napiAsyncTask->Reject(env, CreateJsErrorByNativeErr(env,
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "send event failed!"));
        } else {
            napiAsyncTask.release();
        }
        return result;
    }

    static void OnGetRunningProcessInfoByBundleNameInner(std::string bundleName, int userId,
        sptr<OHOS::AppExecFwk::IAppMgr> appManager, napi_env env, NapiAsyncTask *task)
    {
        if (appManager == nullptr) {
            task->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
            return;
        }
        std::vector<AppExecFwk::RunningProcessInfo> infos;
        int32_t ret = appManager->GetRunningProcessInformation(bundleName, userId, infos);
        if (ret == 0) {
            task->ResolveWithNoError(env, CreateJsRunningProcessInfoArray(env, infos));
        } else {
            task->Reject(env, CreateJsErrorByNativeErr(env, ret));
        }
    }

    napi_value OnGetRunningProcessInfoByBundleName(napi_env env, size_t argc, napi_value* argv)
    {
        if (argc < ARGC_ONE) {
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string bundleName;
        int userId = IPCSkeleton::GetCallingUid() / AppExecFwk::Constants::BASE_USER_RANGE;
        bool isPromiseType = false;
        if (!ConvertFromJsValue(env, argv[0], bundleName)) {
            TAG_LOGE(AAFwkTag::APPMGR, "First parameter must be string");
            ThrowInvalidParamError(env, "Parse param bundleName failed, must be a string.");
            return CreateJsUndefined(env);
        }
        if (argc == ARGC_ONE) {
            isPromiseType = true;
        } else if (argc == ARGC_TWO) {
            if (ConvertFromJsValue(env, argv[1], userId)) {
                isPromiseType = true;
            }
        } else if (argc == ARGC_THREE) {
            if (!ConvertFromJsValue(env, argv[1], userId)) {
                TAG_LOGW(AAFwkTag::APPMGR, "Must input userid and use callback when argc is three.");
                ThrowInvalidParamError(env, "Parse param userId failed, must be a number.");
                return CreateJsUndefined(env);
            }
        } else {
            ThrowInvalidParamError(env, "The number of param exceeded.");
            return CreateJsUndefined(env);
        }

        napi_value lastParam = isPromiseType ? nullptr : argv[argc - 1];
        napi_value result = nullptr;
        std::unique_ptr<NapiAsyncTask> napiAsyncTask = CreateEmptyAsyncTask(env, lastParam, &result);
        auto asyncTask = [bundleName, userId, appManager = appManager_, env, task = napiAsyncTask.get()]() {
            OnGetRunningProcessInfoByBundleNameInner(bundleName, userId, appManager, env, task);
            delete task;
        };
        if (napi_status::napi_ok != napi_send_event(env, asyncTask, napi_eprio_immediate)) {
            napiAsyncTask->Reject(env, CreateJsErrorByNativeErr(env,
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "send event failed!"));
        } else {
            napiAsyncTask.release();
        }
        return result;
    }

    napi_value OnIsApplicationRunning(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        if (argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::APPMGR, "Params not match.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string bundleName;
        if (!ConvertFromJsValue(env, argv[0], bundleName)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Get bundle name wrong.");
            ThrowInvalidParamError(env, "Parse param bundleName failed, must be a string.");
            return CreateJsUndefined(env);
        }

        auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
        auto isRunning = std::make_shared<bool>(false);
        wptr<OHOS::AppExecFwk::IAppMgr> appManager = appManager_;
        NapiAsyncTask::ExecuteCallback execute =
            [bundleName, appManager, innerErrorCode, isRunning]() {
            sptr<OHOS::AppExecFwk::IAppMgr> appMgr = appManager.promote();
            if (appMgr == nullptr) {
                TAG_LOGE(AAFwkTag::APPMGR, "null appmgr");
                *innerErrorCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
                return;
            }
            *innerErrorCode = appMgr->IsApplicationRunning(bundleName, *isRunning);
        };
        NapiAsyncTask::CompleteCallback complete =
            [innerErrorCode, isRunning](napi_env env, NapiAsyncTask &task, int32_t status) {
            if (*innerErrorCode == ERR_OK) {
                task.ResolveWithNoError(env, CreateJsValue(env, *isRunning));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
            }
        };

        napi_value lastParam = (argc == ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSAppManager::OnIsApplicationRunning",
            env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return result;
    }

    napi_value OnIsAppRunning(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        if (argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::APPMGR, "Params not match.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string bundleName;
        if (!ConvertFromJsValue(env, argv[0], bundleName)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Get bundle name wrong.");
            ThrowInvalidParamError(env, "Parse param bundleName failed, must be a string.");
            return CreateJsUndefined(env);
        }
        int32_t appCloneIndex = 0;
        if (argc > ARGC_ONE && !ConvertFromJsValue(env, argv[1], appCloneIndex)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Get appCloneIndex wrong.");
            ThrowInvalidParamError(env, "Parse param appCloneIndex failed, must be a string.");
            return CreateJsUndefined(env);
        }

        auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
        auto isRunning = std::make_shared<bool>(false);
        wptr<OHOS::AppExecFwk::IAppMgr> appManager = appManager_;
        NapiAsyncTask::ExecuteCallback execute =
            [bundleName, appCloneIndex, appManager, innerErrorCode, isRunning]() {
            sptr<OHOS::AppExecFwk::IAppMgr> appMgr = appManager.promote();
            if (appMgr == nullptr) {
                TAG_LOGE(AAFwkTag::APPMGR, "App manager is nullptr.");
                *innerErrorCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
                return;
            }
            *innerErrorCode = appMgr->IsAppRunning(bundleName, appCloneIndex, *isRunning);
        };
        NapiAsyncTask::CompleteCallback complete =
            [innerErrorCode, isRunning](napi_env env, NapiAsyncTask &task, int32_t status) {
            if (*innerErrorCode == ERR_OK) {
                task.ResolveWithNoError(env, CreateJsValue(env, *isRunning));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
            }
        };

        napi_value lastParam = (argc == ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSAppManager::IsAppRunning",
            env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return result;
    }

    napi_value OnPreloadApplication(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        if (argc < ARGC_THREE) {
            TAG_LOGE(AAFwkTag::APPMGR, "PreloadApplication Invalid param count.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        PreloadApplicationParam param;
        std::string errorMsg;
        if (!ConvertPreloadApplicationParam(env, argc, argv, param, errorMsg)) {
            ThrowInvalidParamError(env, errorMsg);
            return CreateJsUndefined(env);
        }

        wptr<OHOS::AppExecFwk::IAppMgr> weak = appManager_;
        auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute =
            [param, innerErrorCode, weak]() {
            sptr<OHOS::AppExecFwk::IAppMgr> appMgr = weak.promote();
            if (appMgr == nullptr) {
                TAG_LOGE(AAFwkTag::APPMGR, "null appMgr");
                *innerErrorCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
                return;
            }
            *innerErrorCode = appMgr->PreloadApplication(param.bundleName, param.userId, param.preloadMode,
                param.appIndex);
        };
        NapiAsyncTask::CompleteCallback complete =
            [innerErrorCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            if (*innerErrorCode == ERR_OK) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
            }
        };
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSAppManager::OnPreloadApplication",
            env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
        return result;
    }

    bool CheckOnOffType(napi_env env, size_t argc, napi_value* argv)
    {
        if (argc < ARGC_ONE) {
            return false;
        }

        if (!AppExecFwk::IsTypeForNapiValue(env, argv[0], napi_string)) {
            TAG_LOGE(AAFwkTag::APPMGR, "argv[0] not string");
            return false;
        }

        std::string type;
        if (!ConvertFromJsValue(env, argv[0], type)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Parse on off type failed");
            return false;
        }

        if (type != ON_OFF_TYPE) {
            TAG_LOGE(AAFwkTag::APPMGR, "args[0] should be %{public}s.", ON_OFF_TYPE);
            return false;
        }
        return true;
    }

    std::string ParseParamType(napi_env env, size_t argc, napi_value* argv)
    {
        std::string type;
        if (argc > INDEX_ZERO && ConvertFromJsValue(env, argv[INDEX_ZERO], type)) {
            return type;
        }
        return "";
    }
};
} // namespace

OHOS::sptr<OHOS::AppExecFwk::IAppMgr> GetAppManagerInstance()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::sptr<OHOS::IRemoteObject> appObject = systemAbilityManager->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID);
    return OHOS::iface_cast<OHOS::AppExecFwk::IAppMgr>(appObject);
}

OHOS::sptr<OHOS::AAFwk::IAbilityManager> GetAbilityManagerInstance()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::sptr<OHOS::IRemoteObject> abilityObject =
        systemAbilityManager->GetSystemAbility(OHOS::ABILITY_MGR_SERVICE_ID);
    return OHOS::iface_cast<OHOS::AAFwk::IAbilityManager>(abilityObject);
}

napi_value JsAppManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "env or exportObj null");
        return nullptr;
    }

    std::unique_ptr<JsAppManager> jsAppManager = std::make_unique<JsAppManager>(
        GetAppManagerInstance(), GetAbilityManagerInstance());
    napi_wrap(env, exportObj, jsAppManager.release(), JsAppManager::Finalizer, nullptr, nullptr);

    napi_set_named_property(env, exportObj, "ApplicationState", ApplicationStateInit(env));
    napi_set_named_property(env, exportObj, "ProcessState", ProcessStateInit(env));
    napi_set_named_property(env, exportObj, "PreloadMode", PreloadModeInit(env));

    const char *moduleName = "AppManager";
    BindNativeFunction(env, exportObj, "on", moduleName, JsAppManager::On);
    BindNativeFunction(env, exportObj, "off", moduleName, JsAppManager::Off);
    BindNativeFunction(env, exportObj, "getForegroundApplications", moduleName,
        JsAppManager::GetForegroundApplications);
    BindNativeFunction(env, exportObj, "getProcessRunningInfos", moduleName,
        JsAppManager::GetRunningProcessInformation);
    BindNativeFunction(env, exportObj, "getRunningProcessInformation", moduleName,
        JsAppManager::GetRunningProcessInformation);
    BindNativeFunction(env, exportObj, "isRunningInStabilityTest", moduleName,
        JsAppManager::IsRunningInStabilityTest);
    BindNativeFunction(env, exportObj, "killProcessWithAccount", moduleName, JsAppManager::KillProcessWithAccount);
    BindNativeFunction(env, exportObj, "killProcessesByBundleName", moduleName,
        JsAppManager::KillProcessesByBundleName);
    BindNativeFunction(env, exportObj, "clearUpApplicationData", moduleName, JsAppManager::ClearUpApplicationData);
    BindNativeFunction(env, exportObj, "clearUpAppData", moduleName, JsAppManager::ClearUpAppData);
    BindNativeFunction(env, exportObj, "terminateMission", moduleName, JsAppManager::TerminateMission);
    BindNativeFunction(env, exportObj, "getAppMemorySize", moduleName, JsAppManager::GetAppMemorySize);
    BindNativeFunction(env, exportObj, "isRamConstrainedDevice", moduleName, JsAppManager::IsRamConstrainedDevice);
    BindNativeFunction(env, exportObj, "isSharedBundleRunning", moduleName, JsAppManager::IsSharedBundleRunning);
    BindNativeFunction(env, exportObj, "getProcessMemoryByPid", moduleName, JsAppManager::GetProcessMemoryByPid);
    BindNativeFunction(env, exportObj, "getRunningProcessInfoByBundleName", moduleName,
        JsAppManager::GetRunningProcessInfoByBundleName);
    BindNativeFunction(env, exportObj, "getRunningMultiAppInfo", moduleName, JsAppManager::GetRunningMultiAppInfo);
    BindNativeFunction(env, exportObj, "isApplicationRunning", moduleName, JsAppManager::IsApplicationRunning);
    BindNativeFunction(env, exportObj, "isAppRunning", moduleName,
        JsAppManager::IsAppRunning);
    BindNativeFunction(env, exportObj, "preloadApplication", moduleName, JsAppManager::PreloadApplication);
    BindNativeFunction(env, exportObj, "getRunningProcessInformationByBundleType", moduleName,
        JsAppManager::GetRunningProcessInformationByBundleType);
    TAG_LOGD(AAFwkTag::APPMGR, "end");
    return CreateJsUndefined(env);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
