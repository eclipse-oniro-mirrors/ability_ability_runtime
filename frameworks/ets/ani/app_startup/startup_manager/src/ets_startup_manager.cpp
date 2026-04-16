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

#include "ets_startup_manager.h"

#include "ability_stage_context.h"
#include "ani_common_util.h"
#include "ets_error_utils.h"
#include "ets_runtime.h"
#include "ets_startup_config.h"
#include "event_report.h"
#include "hilog_tag_wrapper.h"
#include "interop_js/arkts_interop_js_api.h"
#include "stage_context_transfer.h"
#include "startup_manager.h"
#include "startup_task_utils.h"

namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr const char *ETS_STARTUP_MANAGER_CLASS_NAME = "@ohos.app.appstartup.startupManager.startupManager";
constexpr const char *SIGNATURE_STARTUP_MANAGER_CREATE_STARTUP_TASK_MANAGER =
    "C{std.core.Array}zC{@ohos.app.appstartup.StartupConfig.StartupConfig}"
    "C{application.AbilityStageContext.AbilityStageContext}:i";
constexpr const char *SIGNATURE_STARTUP_MANAGER_RUN_ASYNCCALLBACK =
    "iC{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *SIGNATURE_STARTUP_MANAGER_REMOVE_ALL_STARTUP_TASK_RESULTS =
    ":";
constexpr const char *SIGNATURE_STARTUP_MANAGER_GET_STARTUP_TASK_RESULTS =
    "C{std.core.String}:Y";
constexpr const char *SIGNATURE_STARTUP_MANAGER_IS_STARTUP_TASK_INITIALIZED =
    "C{std.core.String}:z";
constexpr const char *SIGNATURE_STARTUP_MANAGER_REMOVE_STARTUP_TASK_RESULT =
    "C{std.core.String}:";
constexpr int32_t ERR_FAILURE = -1;
}

int32_t ETSStartupManager::NativeCreateStartupTaskManager(ani_env *env, ani_object startupTasks,
    ani_boolean isDefaultContext, ani_object startupConfig, ani_object abilityStageContext)
{
    TAG_LOGD(AAFwkTag::STARTUP, "NativeCreateStartupTaskManager");
    uint32_t startupTaskManagerId = 0;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "env is null");
        return ERR_FAILURE;
    }
    std::shared_ptr<StartupTaskManager> startupTaskManager = nullptr;
    int32_t result = ETSStartupManager::GetStartupTaskManager(env, startupTasks, isDefaultContext, startupConfig,
        abilityStageContext, startupTaskManager);
    if (result != ERR_OK || startupTaskManager == nullptr) {
        EtsErrorUtil::ThrowError(env, result, StartupUtils::GetErrorMessage(result));
        TAG_LOGE(AAFwkTag::STARTUP, "GetStartupTaskManager failed");
        return result;
    }
    startupTaskManagerId = startupTaskManager->GetStartupTaskManagerId();
    return static_cast<int32_t>(startupTaskManagerId);
}
void ETSStartupManager::NativeRun(ani_env *env, ani_int startupTaskManagerId, ani_object callback)
{
    TAG_LOGD(AAFwkTag::STARTUP, "NativeRun");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "env is null");
        return;
    }
    std::shared_ptr<StartupTaskManager> startupTaskManager = nullptr;
    int32_t result = DelayedSingleton<StartupManager>::GetInstance()->GetStartupTaskManagerById(
        static_cast<uint32_t>(startupTaskManagerId), startupTaskManager);
    if (result != ERR_OK || startupTaskManager == nullptr) {
        EtsErrorUtil::ThrowError(env, result, StartupUtils::GetErrorMessage(result));
        TAG_LOGE(AAFwkTag::STARTUP, "GetStartupTaskManagerById failed");
        return;
    }
    ani_ref gl = nullptr;
    env->GlobalReference_Create(callback, &gl);
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "GetVM failed");
        EtsErrorUtil::ThrowError(env, ERR_STARTUP_INTERNAL_ERROR,
            StartupUtils::GetErrorMessage(ERR_STARTUP_INTERNAL_ERROR));
        return;
    }
    auto onCompletedCallback = std::make_shared<OnCompletedCallback>(
        [etsVm = aniVM, callbackRef = gl](const std::shared_ptr<StartupTaskResult> &result) {
            if (etsVm == nullptr || callbackRef == nullptr) {
                TAG_LOGE(AAFwkTag::STARTUP, "etsVm or callbackRef is null");
                return;
            }
            ani_object callback = reinterpret_cast<ani_object>(callbackRef);
            bool isAttachedThread = false;
            ani_env *env = AppExecFwk::AttachAniEnv(etsVm, isAttachedThread);
            if (env == nullptr) {
                TAG_LOGE(AAFwkTag::STARTUP, "AttachAniEnv failed");
                return;
            }
            if (result == nullptr || result->GetResultCode() != ERR_OK) {
                ani_object resultValue = ETSStartupConfig::BuildResult(env, result);
                AppExecFwk::AsyncCallback(env, callback, resultValue, nullptr);
                AppExecFwk::DetachAniEnv(etsVm, isAttachedThread);
                return;
            }
            ani_object resultValue = ETSStartupConfig::BuildResult(env, result);
            AppExecFwk::AsyncCallback(env, callback, resultValue, nullptr);
            AppExecFwk::DetachAniEnv(etsVm, isAttachedThread);
        }
    );
    const auto timeoutCallback = []() {
        auto startupManager = DelayedSingleton<StartupManager>::GetInstance();
        if (startupManager == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "null startupManager");
            return;
        }
        AAFwk::EventInfo eventInfo;
        eventInfo.errCode = ERR_STARTUP_TIMEOUT;
        eventInfo.errMsg = "Manual task timeout.";
        eventInfo.bundleName = startupManager->GetBundleName();
        eventInfo.appIndex = startupManager->GetAppIndex();
        eventInfo.userId = startupManager->GetUid() / AppExecFwk::Constants::BASE_USER_RANGE;
        AAFwk::EventReport::SendAppStartupErrorEvent(
            AAFwk::EventName::APP_STARTUP_ERROR, HISYSEVENT_FAULT, eventInfo);
    };
    startupTaskManager->SetTimeoutCallback(timeoutCallback);
    result = startupTaskManager->Run(onCompletedCallback);
    if (result != ERR_OK) {
        if (!onCompletedCallback->IsCalled()) {
            EtsErrorUtil::ThrowError(env, result, StartupUtils::GetErrorMessage(result));
            return;
        }
    }
}

ani_object ETSStartupManager::NativeGetStartupTaskResult(ani_env *env, ani_string startupTask)
{
    TAG_LOGD(AAFwkTag::STARTUP, "NativeGetStartupTaskResult");
    std::string strStartupTask;
    if (!AppExecFwk::GetStdString(env, startupTask, strStartupTask)) {
        TAG_LOGE(AAFwkTag::STARTUP, "GetStdString failed.");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param startupTask failed.");
        return nullptr;
    }
    std::shared_ptr<StartupTaskResult> result;
    int32_t res = DelayedSingleton<StartupManager>::GetInstance()->GetResult(strStartupTask, result);
    if (res != ERR_OK || result == nullptr || result->GetResultCode() != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "get %{public}s result failed", strStartupTask.c_str());
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error: Failed to get result");
        return nullptr;
    }
    ani_ref etsResultRef = StartupTaskUtils::GetDependencyResult(env, result);
    if (etsResultRef == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "ets result is null");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error: ets result is null");
        return nullptr;
    }
    return reinterpret_cast<ani_object>(etsResultRef);
}

bool ETSStartupManager::NativeIsStartupTaskInitialized(ani_env *env, ani_string startupTask)
{
    bool isInitialized = false;
    std::string strStartupTask;
    if (!AppExecFwk::GetStdString(env, startupTask, strStartupTask)) {
        TAG_LOGE(AAFwkTag::STARTUP, "GetStdString failed.");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param startupTask failed.");
        return false;
    }
    int32_t res = DelayedSingleton<StartupManager>::GetInstance()->IsInitialized(strStartupTask, isInitialized);
    if (res != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "get %{public}s result failed", strStartupTask.c_str());
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error: Failed to get result");
        return false;
    }
    return isInitialized;
}

void ETSStartupManager::NativeRemoveStartupTaskResult(ani_env *env, ani_string startupTask)
{
    TAG_LOGD(AAFwkTag::STARTUP, "NativeRemoveStartupTaskResult");
    std::string strStartupTask;
    if (!AppExecFwk::GetStdString(env, startupTask, strStartupTask)) {
        TAG_LOGE(AAFwkTag::STARTUP, "GetStdString failed.");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param startupTask failed.");
        return;
    }
    int32_t res = DelayedSingleton<StartupManager>::GetInstance()->RemoveResult(strStartupTask);
    if (res != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "remove %{public}s result failed", strStartupTask.c_str());
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error: Failed to remove result");
        return;
    }
    return;
}

int32_t ETSStartupManager::GetStartupTaskManager(ani_env *env, ani_object startupTasks, ani_boolean isDefaultContext,
    ani_object startupConfig, ani_object abilityStageContext, std::shared_ptr<StartupTaskManager> &startupTaskManager)
{
    std::vector<std::string> dependencies;
    if (!AppExecFwk::UnwrapArrayString(env, reinterpret_cast<ani_object>(startupTasks), dependencies)) {
        TAG_LOGE(AAFwkTag::STARTUP, "get dependencies failed");
        return ERR_STARTUP_INVALID_VALUE;
    }
    std::shared_ptr<StartupConfig> config;
    ani_boolean isConfigUndefined = ANI_TRUE;
    env->Reference_IsUndefined(reinterpret_cast<ani_ref>(startupConfig), &isConfigUndefined);
    int32_t result = ERR_OK;
    if (!isConfigUndefined) {
        result = GetConfig(env, startupConfig, config);
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::STARTUP, "get config failed");
            return result;
        }
    }
    bool supportFeatureModule = isDefaultContext != ANI_TRUE;
    result = DelayedSingleton<StartupManager>::GetInstance()->BuildAppStartupTaskManager(dependencies,
        startupTaskManager, supportFeatureModule);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "build startup task manager failed");
        return result;
    }
    if (startupTaskManager == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "startup task manager is null");
        return ERR_STARTUP_INVALID_VALUE;
    }
    auto tasks = startupTaskManager->GetStartupTasks();
    if (supportFeatureModule) {
        TAG_LOGI(AAFwkTag::STARTUP, "supportFeatureModule");
        ani_ref contextRef = nullptr;
        env->GlobalReference_Create(abilityStageContext, &contextRef);
        UpdateStartupTasks(env, tasks, contextRef);
    }
    result = startupTaskManager->Prepare();
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "prepare startup task manager failed");
        return result;
    }
    if (config != nullptr) {
        startupTaskManager->SetConfig(config);
    }
    return ERR_OK;
}

int32_t ETSStartupManager::GetConfig(ani_env *env, ani_object configObj, std::shared_ptr<StartupConfig> &config)
{
    ani_vm *aniVM = nullptr;
    ani_status status = env->GetVM(&aniVM);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "get vm failed");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    std::shared_ptr<ETSStartupConfig> startupConfig = std::make_shared<ETSStartupConfig>(aniVM);
    if (startupConfig == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "create startup config failed");
        return ERR_STARTUP_INVALID_VALUE;
    }
    if (startupConfig->Init(configObj) != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "init startup config failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "init startup config failed");
        return ERR_STARTUP_INVALID_VALUE;
    }
    config = startupConfig;
    return ERR_OK;
}

void ETSStartupManager::NativeRemoveAllStartupTaskResults(ani_env *env)
{
    TAG_LOGD(AAFwkTag::STARTUP, "called");
    DelayedSingleton<StartupManager>::GetInstance()->RemoveAllResult();
}

void ETSStartupManager::UpdateStartupTasks(ani_env *env, std::map<std::string, std::shared_ptr<StartupTask>> &tasks,
    ani_ref stageContextRef)
{
    napi_env napiEnv = {};
    if (!arkts_napi_scope_open(env, &napiEnv)) {
        TAG_LOGE(AAFwkTag::STARTUP, "arkts_napi_scope_open failed");
        return;
    }
    for (auto &iter : tasks) {
        if (iter.second == nullptr) {
            continue;
        }
        if (iter.second->GetType() != AppStartupTask::TASK_TYPE_JS &&
            iter.second->GetType() != AppStartupTask::TASK_TYPE_ETS) {
            continue;
        }
        std::shared_ptr<AppStartupTask> appStartupTask = std::static_pointer_cast<AppStartupTask>(iter.second);
        auto context = StageContextTransfer::UnwrapContext(env, stageContextRef);
        StartupTaskUtils::UpdateStartupTaskContextRef(napiEnv, appStartupTask, context, stageContextRef);
    }
    arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
}

void ETSStartupManagerInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::STARTUP, "Init startup manager");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "env is null");
        return;
    }
    ani_namespace ns;
    auto status = env->FindNamespace(ETS_STARTUP_MANAGER_CLASS_NAME, &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "find namespace failed, status: %{public}d", status);
        return;
    }
    if (ns == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "ns null");
        return;
    }
    std::array nativeFuncs = {
        ani_native_function { "nativeCreateStartupTaskManager", SIGNATURE_STARTUP_MANAGER_CREATE_STARTUP_TASK_MANAGER,
            reinterpret_cast<void*>(ETSStartupManager::NativeCreateStartupTaskManager) },
        ani_native_function { "nativeRun", SIGNATURE_STARTUP_MANAGER_RUN_ASYNCCALLBACK,
            reinterpret_cast<void*>(ETSStartupManager::NativeRun) },
        ani_native_function { "nativeRemoveAllStartupTaskResults",
            SIGNATURE_STARTUP_MANAGER_REMOVE_ALL_STARTUP_TASK_RESULTS,
            reinterpret_cast<void*>(ETSStartupManager::NativeRemoveAllStartupTaskResults) },
        ani_native_function { "nativeGetStartupTaskResult", SIGNATURE_STARTUP_MANAGER_GET_STARTUP_TASK_RESULTS,
            reinterpret_cast<void*>(ETSStartupManager::NativeGetStartupTaskResult) },
        ani_native_function { "nativeIsStartupTaskInitialized", SIGNATURE_STARTUP_MANAGER_IS_STARTUP_TASK_INITIALIZED,
            reinterpret_cast<void*>(ETSStartupManager::NativeIsStartupTaskInitialized) },
        ani_native_function { "nativeRemoveStartupTaskResult", SIGNATURE_STARTUP_MANAGER_REMOVE_STARTUP_TASK_RESULT,
            reinterpret_cast<void*>(ETSStartupManager::NativeRemoveStartupTaskResult) },
    };
    status = env->Namespace_BindNativeFunctions(ns, nativeFuncs.data(), static_cast<int32_t>(nativeFuncs.size()));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "bind native methods failed, status: %{public}d", status);
        return;
    }
    TAG_LOGD(AAFwkTag::STARTUP, "Init startup manager success");
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::STARTUP, "ANI_Constructor");
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if (vm == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null vm");
        return ANI_ERROR;
    }
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "GetEnv failed status: %{public}d", status);
        return ANI_NOT_FOUND;
    }

    ETSStartupManagerInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::STARTUP, "ANI_Constructor finish");
    return ANI_OK;
}
}
}
}
