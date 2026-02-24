/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <mutex>

#include "ability_manager_interface.h"
#include "ability_manager_errors.h"
#include "app_mgr_interface.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "js_app_manager_utils.h"
#include "event_runner.h"
#include "napi_common_util.h"
#include "js_app_state_observer.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr int32_t ERROR_CODE_ONE = 1;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr int32_t ERR_NOT_OK = -1;

class JsAppManager final {
public:
    JsAppManager(sptr<OHOS::AppExecFwk::IAppMgr> appManager,
        sptr<OHOS::AAFwk::IAbilityManager> abilityManager) : appManager_(appManager),
        abilityManager_(abilityManager) {}
    ~JsAppManager() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        std::unique_ptr<JsAppManager>(static_cast<JsAppManager*>(data));
    }

    static napi_value RegisterApplicationStateObserver(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnRegisterApplicationStateObserver);
    }

    static napi_value UnregisterApplicationStateObserver(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnUnregisterApplicationStateObserver);
    }

    static napi_value GetForegroundApplications(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnGetForegroundApplications);
    }

    static napi_value GetProcessRunningInfos(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnGetProcessRunningInfos);
    }

    static napi_value IsRunningInStabilityTest(napi_env env, napi_callback_info info)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnIsRunningInStabilityTest);
    }

    static napi_value KillProcessWithAccount(napi_env env, napi_callback_info info)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnKillProcessWithAccount);
    }

    static napi_value KillProcessesByBundleName(napi_env env, napi_callback_info info)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnKillProcessByBundleName);
    }

    static napi_value ClearUpApplicationData(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnClearUpApplicationData);
    }

    static napi_value GetAppMemorySize(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnGetAppMemorySize);
    }

    static napi_value IsRamConstrainedDevice(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsAppManager, OnIsRamConstrainedDevice);
    }
private:
    sptr<OHOS::AppExecFwk::IAppMgr> appManager_ = nullptr;
    sptr<OHOS::AAFwk::IAbilityManager> abilityManager_ = nullptr;
    sptr<JSAppStateObserver> observer_ = nullptr;

    napi_value OnRegisterApplicationStateObserver(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        // only support 1 or 2 params
        if (argc != ARGC_ONE && argc != ARGC_TWO) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid argc");
            return CreateJsUndefined(env);
        }
        if (appManager_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null appMgr");
            return CreateJsUndefined(env);
        }
        static int64_t serialNumber = 0;
        std::vector<std::string> bundleNameList;
        // unwrap observer
        if (observer_ == nullptr) {
            observer_ = new JSAppStateObserver(env);
        }
        if (argc == ARGC_TWO) {
            AppExecFwk::UnwrapArrayStringFromJS(env, argv[INDEX_ONE], bundleNameList);
        }
        int32_t ret = appManager_->RegisterApplicationStateObserver(observer_, bundleNameList);
        if (ret == 0) {
            TAG_LOGD(AAFwkTag::APPMGR, "success");
            int64_t observerId = serialNumber;
            observer_->AddJsObserverObject(observerId, argv[INDEX_ZERO]);
            if (serialNumber < INT32_MAX) {
                serialNumber++;
            } else {
                serialNumber = 0;
            }
            return CreateJsValue(env, observerId);
        } else {
            TAG_LOGE(AAFwkTag::APPMGR, "error:%{public}d", ret);
            return CreateJsUndefined(env);
        }
    }

    napi_value OnUnregisterApplicationStateObserver(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        int32_t errCode = 0;
        int64_t observerId = -1;

        // only support 1 or 2 params
        if (argc != ARGC_ONE && argc != ARGC_TWO) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid argc");
            errCode = ERR_NOT_OK;
        } else {
            // unwrap connectId
            napi_get_value_int64(env, argv[INDEX_ZERO], &observerId);
            bool isExist = observer_->FindObserverByObserverId(observerId);
            if (isExist) {
                // match id
                TAG_LOGD(AAFwkTag::APPMGR, "observer exist:%{public}d", static_cast<int32_t>(observerId));
            } else {
                TAG_LOGD(AAFwkTag::APPMGR, "observer not exist:%{public}d", static_cast<int32_t>(observerId));
                errCode = ERR_NOT_OK;
            }
        }

        NapiAsyncTask::CompleteCallback complete =
            [appManager = appManager_, observer = observer_, observerId, errCode](
                napi_env env, NapiAsyncTask& task, int32_t status) {
                if (errCode != 0) {
                    task.Reject(env, CreateJsError(env, errCode, "Invalidate params."));
                    return;
                }
                if (observer == nullptr || appManager == nullptr) {
                    TAG_LOGE(AAFwkTag::APPMGR, "null observer or appMgr");
                    task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "observer or appManager nullptr"));
                    return;
                }
                int32_t ret = appManager->UnregisterApplicationStateObserver(observer);
                if (ret == 0 && observer->RemoveJsObserverObject(observerId)) {
                    task.Resolve(env, CreateJsUndefined(env));
                    TAG_LOGD(AAFwkTag::APPMGR, "success size:%{public}zu", observer->GetJsObserverMapSize());
                } else {
                    TAG_LOGE(AAFwkTag::APPMGR, "error:%{public}d", ret);
                    task.Reject(env, CreateJsError(env, ret, "UnregisterApplicationStateObserver failed"));
                }
            };

        napi_value lastParam = (argc == ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JSAppManager::OnUnregisterApplicationStateObserver",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnGetForegroundApplications(napi_env env, const size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        int32_t errCode = 0;

        // only support 0 or 1 params
        if (argc != ARGC_ZERO && argc != ARGC_ONE) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid argc");
            errCode = ERR_NOT_OK;
        }
        NapiAsyncTask::CompleteCallback complete =
            [appManager = appManager_, errCode](napi_env env, NapiAsyncTask& task, int32_t status) {
                if (errCode != 0) {
                    task.Reject(env, CreateJsError(env, errCode, "Invalidate params."));
                    return;
                }
                if (appManager == nullptr) {
                    TAG_LOGE(AAFwkTag::APPMGR, "null appMgr");
                    task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "appManager nullptr"));
                    return;
                }
                std::vector<AppExecFwk::AppStateData> list;
                int32_t ret = appManager->GetForegroundApplications(list);
                if (ret == 0) {
                    TAG_LOGD(AAFwkTag::APPMGR, "success.");
                    task.Resolve(env, CreateJsAppStateDataArray(env, list));
                } else {
                    TAG_LOGE(AAFwkTag::APPMGR, "error:%{public}d", ret);
                    task.Reject(env, CreateJsError(env, ret, "OnGetForegroundApplications failed"));
                }
            };

        napi_value lastParam = (argc == ARGC_ONE) ? argv[INDEX_ZERO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JSAppManager::OnGetForegroundApplications",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnGetProcessRunningInfos(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        int32_t errCode = 0;

        // only support 0 or 1 params
        if (argc != ARGC_ZERO && argc != ARGC_ONE) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid argc");
            errCode = ERR_NOT_OK;
        }
        NapiAsyncTask::CompleteCallback complete =
            [appManager = appManager_, errCode](napi_env env, NapiAsyncTask &task, int32_t status) {
                if (errCode != 0) {
                    task.Reject(env, CreateJsError(env, errCode, "Invalidate params."));
                    return;
                }
                std::vector<AppExecFwk::RunningProcessInfo> infos;
                auto ret = appManager->GetAllRunningProcesses(infos);
                if (ret == 0) {
                    task.Resolve(env, CreateJsProcessRunningInfoArray(env, infos));
                } else {
                    task.Reject(env, CreateJsError(env, ret, "Get mission infos failed."));
                }
            };

        napi_value lastParam = (argc == ARGC_ONE) ? argv[INDEX_ZERO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JSAppManager::OnGetProcessRunningInfos",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnIsRunningInStabilityTest(napi_env env, const size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        int32_t errCode = 0;

        // only support 0 or 1 params
        if (argc != ARGC_ZERO && argc != ARGC_ONE) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid argc");
            errCode = ERR_NOT_OK;
        }
        NapiAsyncTask::CompleteCallback complete =
            [abilityManager = abilityManager_, errCode](napi_env env, NapiAsyncTask& task, int32_t status) {
                if (errCode != 0) {
                    task.Reject(env, CreateJsError(env, errCode, "Invalidate params."));
                    return;
                }
                if (abilityManager == nullptr) {
                    TAG_LOGW(AAFwkTag::APPMGR, "null abilityMgr");
                    task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "abilityManager nullptr"));
                    return;
                }
                bool ret = abilityManager->IsRunningInStabilityTest();
                TAG_LOGI(AAFwkTag::APPMGR, "result:%{public}d", ret);
                task.Resolve(env, CreateJsValue(env, ret));
            };

        napi_value lastParam = (argc == ARGC_ONE) ? argv[INDEX_ZERO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JSAppManager::OnIsRunningInStabilityTest",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnKillProcessByBundleName(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        int32_t errCode = 0;
        std::string bundleName;
        // only support 1 or 2 params
        if (argc != ARGC_ONE && argc != ARGC_TWO) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid argc");
            errCode = ERR_NOT_OK;
        } else {
            if (!ConvertFromJsValue(env, argv[INDEX_ZERO], bundleName)) {
                TAG_LOGE(AAFwkTag::APPMGR, "get bundleName failed!");
                errCode = ERR_NOT_OK;
            }
        }

        NapiAsyncTask::CompleteCallback complete =
            [bundleName, abilityManager = abilityManager_, errCode](napi_env env, NapiAsyncTask& task,
                int32_t status) {
            if (errCode != 0) {
                task.Reject(env, CreateJsError(env, errCode, "Invalidate params."));
                return;
            }
            if (abilityManager == nullptr) {
                TAG_LOGW(AAFwkTag::APPMGR, "null abilityMgr");
                task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "abilityManager nullptr"));
                return;
            }
            auto ret = abilityManager->KillProcess(bundleName);
            if (ret == 0) {
                task.Resolve(env, CreateJsValue(env, ret));
            } else {
                task.Reject(env, CreateJsError(env, ret, "kill process failed."));
            }
        };
        napi_value lastParam = (argc == ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSAppManager::OnKillProcessByBundleName",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnClearUpApplicationData(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        int32_t errCode = 0;
        std::string bundleName;

        // only support 1 or 2 params
        if (argc != ARGC_ONE && argc != ARGC_TWO) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid argc");
            errCode = ERR_NOT_OK;
        } else {
            if (!ConvertFromJsValue(env, argv[0], bundleName)) {
                TAG_LOGE(AAFwkTag::APPMGR, "get bundleName failed");
                errCode = ERR_NOT_OK;
            } else {
                TAG_LOGI(AAFwkTag::APPMGR, "kill process [%{public}s]", bundleName.c_str());
            }
        }

        NapiAsyncTask::CompleteCallback complete =
            [bundleName, appManager = appManager_, errCode](napi_env env, NapiAsyncTask& task,
                int32_t status) {
            if (errCode != 0) {
                task.Reject(env, CreateJsError(env, errCode, "Invalidate params."));
                return;
            }
            if (appManager == nullptr) {
                TAG_LOGW(AAFwkTag::APPMGR, "null appMgr");
                task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "appManager nullptr"));
                return;
            }
            auto ret = appManager->ClearUpApplicationData(bundleName, 0);
            if (ret == 0) {
                task.Resolve(env, CreateJsValue(env, ret));
            } else {
                task.Reject(env, CreateJsError(env, AAFwk::CLEAR_APPLICATION_DATA_FAIL,
                    "clear up application failed."));
            }
        };

        napi_value lastParam = (argc == ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JSAppManager::OnClearUpApplicationData",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnKillProcessWithAccount(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "called");
        int32_t errCode = 0;
        int32_t accountId = -1;
        std::string bundleName;

        // only support 2 or 3 params
        if (argc != ARGC_TWO && argc != ARGC_THREE) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid argc");
            errCode = ERR_NOT_OK;
        } else {
            if (!ConvertFromJsValue(env, argv[INDEX_ZERO], bundleName)) {
                TAG_LOGE(AAFwkTag::APPMGR, "Parse bundleName failed");
                errCode = ERR_NOT_OK;
            }
            if (!ConvertFromJsValue(env, argv[INDEX_ONE], accountId)) {
                TAG_LOGE(AAFwkTag::APPMGR, "Parse userId failed");
                errCode = ERR_NOT_OK;
            }
        }

        NapiAsyncTask::CompleteCallback complete =
            [appManager = appManager_, bundleName, accountId, errCode](
                napi_env env, NapiAsyncTask &task, int32_t status) {
                if (errCode != 0) {
                    task.Reject(env, CreateJsError(env, errCode, "Invalidate params."));
                    return;
                }
                auto ret = appManager->GetAmsMgr()->KillProcessWithAccount(bundleName, accountId);
                if (ret == 0) {
                    task.Resolve(env, CreateJsUndefined(env));
                } else {
                    TAG_LOGD(AAFwkTag::APPMGR, "failed error:%{public}d", ret);
                    task.Reject(env, CreateJsError(env, ret, "Kill processes failed."));
                }
            };

        napi_value lastParam = (argc == ARGC_THREE) ? argv[INDEX_TWO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSAppManager::OnKillProcessWithAccount",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnGetAppMemorySize(napi_env env, const size_t argc, napi_value* argv)
    {
        int32_t errCode = 0;

        // only support 0 or 1 params
        if (argc != ARGC_ZERO && argc != ARGC_ONE) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid argc");
            errCode = ERR_NOT_OK;
        }
        NapiAsyncTask::CompleteCallback complete =
            [abilityManager = abilityManager_, errCode](napi_env env, NapiAsyncTask& task, int32_t status) {
                if (errCode != 0) {
                    task.Reject(env, CreateJsError(env, errCode, "Invalidate params."));
                    return;
                }
                if (abilityManager == nullptr) {
                    TAG_LOGW(AAFwkTag::APPMGR, "null abilityMgr");
                    task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "abilityManager nullptr"));
                    return;
                }
                int32_t memorySize = abilityManager->GetAppMemorySize();
                TAG_LOGI(AAFwkTag::APPMGR, "memorySize:%{public}d", memorySize);
                task.Resolve(env, CreateJsValue(env, memorySize));
            };

        napi_value lastParam = (argc == ARGC_ONE) ? argv[INDEX_ZERO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSAppManager::OnGetAppMemorySize",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnIsRamConstrainedDevice(napi_env env, const size_t argc, napi_value* argv)
    {
        int32_t errCode = 0;

        // only support 0 or 1 params
        if (argc != ARGC_ZERO && argc != ARGC_ONE) {
            TAG_LOGE(AAFwkTag::APPMGR, "invalid argc");
            errCode = ERR_NOT_OK;
        }
        NapiAsyncTask::CompleteCallback complete =
            [abilityManager = abilityManager_, errCode](napi_env env, NapiAsyncTask& task, int32_t status) {
                if (errCode != 0) {
                    task.Reject(env, CreateJsError(env, errCode, "Invalidate params."));
                    return;
                }
                if (abilityManager == nullptr) {
                    TAG_LOGW(AAFwkTag::APPMGR, "null abilityMgr");
                    task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "abilityManager nullptr"));
                    return;
                }
                bool ret = abilityManager->IsRamConstrainedDevice();
                TAG_LOGI(AAFwkTag::APPMGR, "result:%{public}d", ret);
                task.Resolve(env, CreateJsValue(env, ret));
            };

        napi_value lastParam = (argc == ARGC_ONE) ? argv[INDEX_ZERO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JSAppManager::OnIsRamConstrainedDevice",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
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
        TAG_LOGW(AAFwkTag::APPMGR, "null env or exportObj");
        return nullptr;
    }

    std::unique_ptr<JsAppManager> jsAppManager = std::make_unique<JsAppManager>(
        GetAppManagerInstance(), GetAbilityManagerInstance());
    napi_wrap(env, exportObj, jsAppManager.release(), JsAppManager::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsAppManager";
    BindNativeFunction(env, exportObj, "registerApplicationStateObserver", moduleName,
        JsAppManager::RegisterApplicationStateObserver);
    BindNativeFunction(env, exportObj, "unregisterApplicationStateObserver", moduleName,
        JsAppManager::UnregisterApplicationStateObserver);
    BindNativeFunction(env, exportObj, "getForegroundApplications", moduleName,
        JsAppManager::GetForegroundApplications);
    BindNativeFunction(env, exportObj, "getProcessRunningInfos", moduleName,
        JsAppManager::GetProcessRunningInfos);
    BindNativeFunction(env, exportObj, "getProcessRunningInformation", moduleName,
        JsAppManager::GetProcessRunningInfos);
    BindNativeFunction(env, exportObj, "isRunningInStabilityTest", moduleName,
        JsAppManager::IsRunningInStabilityTest);
    BindNativeFunction(env, exportObj, "killProcessWithAccount", moduleName,
        JsAppManager::KillProcessWithAccount);
    BindNativeFunction(env, exportObj, "killProcessesByBundleName", moduleName,
        JsAppManager::KillProcessesByBundleName);
    BindNativeFunction(env, exportObj, "clearUpApplicationData", moduleName,
        JsAppManager::ClearUpApplicationData);
    BindNativeFunction(env, exportObj, "getAppMemorySize", moduleName,
        JsAppManager::GetAppMemorySize);
    BindNativeFunction(env, exportObj, "isRamConstrainedDevice", moduleName,
        JsAppManager::IsRamConstrainedDevice);
    TAG_LOGD(AAFwkTag::APPMGR, "JsAppManagerInit end");
    return CreateJsUndefined(env);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
