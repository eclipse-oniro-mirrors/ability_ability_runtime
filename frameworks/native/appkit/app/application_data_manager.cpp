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

#include "application_data_manager.h"

#include <dfx_signal_handler.h>
#include <sys/xattr.h>
#include "app_recovery.h"
#include "hilog_tag_wrapper.h"
#include "native_engine.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
    constexpr size_t STACK_MAX_SZIE = 1024;
    constexpr size_t AT_SKIP_SZIE = 3;
    constexpr const char* TASK_POOL_THREAD = "Taskpool Thread";
    constexpr const char *RSRC_OBSV_PATH = "/data/storage/el2/base/cache/rawheap";
    constexpr const char *RSRC_OBSV_XATTR = "user.resource_observer";
    constexpr const char *RSRC_OBSV_TRSHD_XATTR_PSS = "user.resource_observer_threshold.pss";
    constexpr const char *RSRC_OBSV_TRSHD_XATTR_GPU = "user.resource_observer_threshold.gpu";
    constexpr const char *RSRC_OBSV_TRSHD_XATTR_FD = "user.resource_observer_threshold.fd";
    constexpr const char *RSRC_OBSV_TRSHD_XATTR_RAT = "user.resource_observer_threshold.rss_ark_ts";
    constexpr const char *RSRC_OBSV_TRSHD_XATTR_RNH = "user.resource_observer_threshold.rss_native_heap";
    enum class InstanceType {
        DEFAULT_TYPE = -1,
        WORKER_THREAD_TYPE = 1,
        TASK_POOL_THREAD_TYPE = 2,
    };
    thread_local bool g_hasNotified = false;
}

std::atomic<bool> ApplicationDataManager::jsErrorHasReport_{false};
std::atomic<napi_env> ApplicationDataManager::jsErrorEnv_{nullptr};
ApplicationDataManager::ApplicationDataManager() {}

ApplicationDataManager::~ApplicationDataManager() {}

ApplicationDataManager &ApplicationDataManager::GetInstance()
{
    static ApplicationDataManager manager;
    return manager;
}

void ApplicationDataManager::AddErrorObserver(const std::shared_ptr<IErrorObserver> &observer)
{
    errorObserver_ = observer;
}

bool ApplicationDataManager::NotifyUnhandledException(const std::string &errMsg)
{
    if (errorObserver_) {
        errorObserver_->OnUnhandledException(errMsg);
        return true;
    }

    // if apprecovery is enabled, we could callback to save current state
    // and restart as developer wants
    return AppRecovery::GetInstance().TryRecoverApp(StateReason::JS_ERROR);
}

bool ApplicationDataManager::NotifyCJUnhandledException(const std::string &errMsg)
{
    if (errorObserver_) {
        errorObserver_->OnUnhandledException(errMsg);
        return true;
    }

    // if apprecovery is enabled, we could callback to save current state
    // and restart as developer wants
    return AppRecovery::GetInstance().TryRecoverApp(StateReason::CJ_ERROR);
}

bool ApplicationDataManager::NotifyETSUnhandledException(const std::string &errMsg)
{
    if (errorObserver_) {
        errorObserver_->OnUnhandledException(errMsg);
        return true;
    }
    return AppRecovery::GetInstance().TryRecoverApp(StateReason::JS_ERROR);
}

void ApplicationDataManager::RemoveErrorObserver()
{
    errorObserver_ = nullptr;
}

bool ApplicationDataManager::NotifyExceptionObject(const AppExecFwk::ErrorObject &errorObj)
{
    if (errorObserver_) {
        errorObserver_->OnExceptionObject(errorObj);
        return true;
    }

    // if apprecovery is enabled, we could callback to save current state
    // and restart as developer wants
    return AppRecovery::GetInstance().TryRecoverApp(StateReason::JS_ERROR);
}

bool ApplicationDataManager::NotifyCJExceptionObject(const AppExecFwk::ErrorObject &errorObj)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Notify Exception error observer come");
    if (errorObserver_) {
        errorObserver_->OnExceptionObject(errorObj);
        return true;
    }

    // if apprecovery is enabled, we could callback to save current state
    // and restart developer wants
    return AppRecovery::GetInstance().TryRecoverApp(StateReason::CJ_ERROR);
}

bool ApplicationDataManager::NotifyETSExceptionObject(const AppExecFwk::ErrorObject &errorObj)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Notify Exception error observer come");
    if (errorObserver_) {
        errorObserver_->OnExceptionObject(errorObj);
        return true;
    }
    return AppRecovery::GetInstance().TryRecoverApp(StateReason::JS_ERROR);
}

bool ApplicationDataManager::NotifyETSErrorObject(const AppExecFwk::ErrorObject &errorObj)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Notify Exception error observer start");
    if (errorCallback_) {
        errorCallback_(errorObj);
        return true;
    }
    return AppRecovery::GetInstance().TryRecoverApp(StateReason::JS_ERROR);
}

void ApplicationDataManager::SetErrorHandlerCallback(EtsErrorCallback errorCallback)
{
    errorCallback_ = errorCallback;
}

void ApplicationDataManager::SetIsUncatchable(bool isUncatchable)
{
    isUncatchable_.store(isUncatchable);
}

bool ApplicationDataManager::GetIsUncatchable()
{
    bool isUncatchable = isUncatchable_.load();
    return isUncatchable;
}

std::string ApplicationDataManager::GetFuncNameFromError(napi_env env, napi_value error)
{
    if (error == nullptr) {
        return TASK_POOL_THREAD;
    }

    napi_value stack;
    if (napi_get_named_property(env, error, "stack", &stack) != napi_ok ||stack == nullptr) {
        return TASK_POOL_THREAD;
    }

    std::string rawStack;
    size_t rawStackSize = 0;
    napi_get_value_string_utf8(env, stack, nullptr, 0, &rawStackSize);
    rawStackSize = std::min(rawStackSize, STACK_MAX_SZIE);
    rawStack.reserve(rawStackSize + 1);
    rawStack.resize(rawStackSize);
    napi_get_value_string_utf8(env, stack, rawStack.data(), rawStack.size() + 1, &rawStackSize);

    size_t pos = rawStack.find("at");
    if (pos == std::string::npos) {
        return TASK_POOL_THREAD;
    }
    size_t endPos = rawStack.find("(", pos);
    if (endPos == std::string::npos) {
        return TASK_POOL_THREAD;
    }
    size_t startPos = pos + AT_SKIP_SZIE;
    if (endPos <= startPos + 1) {
        return TASK_POOL_THREAD;
    }

    std::string funcName = std::string(TASK_POOL_THREAD);
    funcName.append(rawStack.substr(startPos, endPos - startPos - 1));
    return funcName;
}

bool ApplicationDataManager::NotifyUncaughtException(const ExceptionParams &params,
    const AppExecFwk::ErrorObject &errorObj)
{
    if (params.isUncatchable && g_hasNotified) {
        return false;
    }
    g_hasNotified = params.isUncatchable;

    auto napiEnv = params.env ? params.env : params.mainEnv;
    if (params.env == params.mainEnv) {
        TAG_LOGI(AAFwkTag::APPKIT, "main thread");
        if (NapiErrorManager::GetInstance()->NotifyUncaughtException(napiEnv, params.summary,
            errorObj.name, errorObj.message, errorObj.stack)) {
            TAG_LOGI(AAFwkTag::APPKIT, "Complete all callbacks");
            if (!params.isUncatchable) {
                return true;
            }
        }
    } else if (params.isUncatchable) {
        NativeEngine* engine = reinterpret_cast<NativeEngine*>(napiEnv);
        if (engine == nullptr) {
            return false;
        }
        std::string name;
        InstanceType type = InstanceType::DEFAULT_TYPE;
        if (engine->IsWorkerThread()) {
            type = InstanceType::WORKER_THREAD_TYPE;
            napi_value workerGlobalObject = nullptr;
            napi_get_global(napiEnv, &workerGlobalObject);
            napi_value valueStr = nullptr;
            if (napi_get_named_property(napiEnv, workerGlobalObject, "name", &valueStr) != napi_ok) {
                return false;
            }
            napi_valuetype valueType = napi_undefined;
            napi_typeof(napiEnv, valueStr, &valueType);
 
            if (valueType == napi_string) {
                size_t nameSize = 0;
                napi_get_value_string_utf8(napiEnv, valueStr, nullptr, 0, &nameSize);
                name.reserve(nameSize + 1);
                name.resize(nameSize);
                napi_get_value_string_utf8(napiEnv, valueStr, name.data(), name.size() + 1, &nameSize);
            }
            TAG_LOGE(AAFwkTag::APPKIT, "worker thread, instanceType=1, instanceName=%{public}s", name.c_str());
        } else if (engine->IsTaskPoolThread()) {
            type = InstanceType::TASK_POOL_THREAD_TYPE;
            name = GetFuncNameFromError(napiEnv, params.exception);
            TAG_LOGE(AAFwkTag::APPKIT, "task pool thread, instanceType=2, instanceName=%{public}s", name.c_str());
        }
        NapiErrorManager::GetInstance()->NotifyUncaughtException(napiEnv, params.exception, name,
            static_cast<uint32_t>(type));
    }
    return false;
}

bool ApplicationDataManager::NotifyLeakObject(const LeakObject &leakObj)
{
    std::lock_guard<std::mutex> lock(leakObserverMutex_);
    if (leakObserver_) {
        return leakObserver_(leakObj);
    }
    return false;
}

void ApplicationDataManager::SetLeakObserver(LeakObserverFunction leakCallback)
{
    std::lock_guard<std::mutex> lock(leakObserverMutex_);
    leakObserver_ = leakCallback;
}

void ApplicationDataManager::RegisterHasOnErrorCallback(HasOnErrorCallback hasOnErrorCallback)
{
    std::lock_guard<std::mutex> lock(hasOnErrorCallbackMutex_);
    hasOnErrorCallback_ = hasOnErrorCallback;
}


bool ApplicationDataManager::GetHasOnErrorCallback()
{
    std::lock_guard<std::mutex> lock(hasOnErrorCallbackMutex_);
    if (hasOnErrorCallback_) {
        return hasOnErrorCallback_();
    }
    return false;
}

bool ApplicationDataManager::RegisterResourceObserver(RegisterResourceParams params, ResourceOverlimitCB cb)
{
    std::lock_guard<std::mutex> lock(resourceMutex_);
    if (cb == nullptr || params.appTelemetryLeakType == 0) {
        cb = nullptr;
        params.appTelemetryLeakType = 0;
        TAG_LOGI(AAFwkTag::APPKIT, "will unregister");
    }

    resourceOverlimitCB_ = cb;
    bool ret = WriteSandBoxXattr(params);
    TAG_LOGI(AAFwkTag::APPKIT, "RegisterResourceObserver execute result: %{public}d", ret);
    return ret;
}

bool ApplicationDataManager::WriteSandBoxXattr(RegisterResourceParams params)
{
    std::string xattrValue = DFX_GetAppRunningUniqueId();
    xattrValue += "," + std::to_string(params.appTelemetryLeakType);
    bool ret = (setxattr(RSRC_OBSV_PATH, RSRC_OBSV_XATTR, xattrValue.c_str(), strlen(xattrValue.c_str()), 0) == 0);
    if (!ret) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed write atlt, path: %{public}s, xattr: %{public}s, err: %{public}s",
                 RSRC_OBSV_PATH, RSRC_OBSV_XATTR, strerror(errno));
        return false;
    }
    string threshold = std::to_string(params.thresholdPss);
    ret = (setxattr(RSRC_OBSV_PATH, RSRC_OBSV_TRSHD_XATTR_PSS, threshold.c_str(), threshold.length(), 0) == 0);
    threshold = std::to_string(params.thresholdGpu);
    ret = ret && (setxattr(RSRC_OBSV_PATH, RSRC_OBSV_TRSHD_XATTR_GPU, threshold.c_str(), threshold.length(), 0) == 0);
    threshold = std::to_string(params.thresholdFd);
    ret = ret && (setxattr(RSRC_OBSV_PATH, RSRC_OBSV_TRSHD_XATTR_FD, threshold.c_str(), threshold.length(), 0) == 0);
    threshold = std::to_string(params.thresholdRAT);
    ret = ret && (setxattr(RSRC_OBSV_PATH, RSRC_OBSV_TRSHD_XATTR_RAT, threshold.c_str(), threshold.length(), 0) == 0);
    threshold = std::to_string(params.thresholdRNH);
    ret = ret && (setxattr(RSRC_OBSV_PATH, RSRC_OBSV_TRSHD_XATTR_RNH, threshold.c_str(), threshold.length(), 0) == 0);
    if (!ret) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed write threshold, path: %{public}s, err: %{public}s",
                 RSRC_OBSV_PATH, strerror(errno));
    }
    return ret;
}
 
void ApplicationDataManager::NotifyAppTelemetry(AppTelemetryLeakType atLeakType)
{
    std::lock_guard<std::mutex> lock(resourceMutex_);
    if (resourceOverlimitCB_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "resourceOverlimitCB_ is nullptr");
        return;
    }
    AppTelemetryObject atObj {
        .atLeakType = atLeakType,
        .runningId = DFX_GetAppRunningUniqueId(),
    };
    TAG_LOGI(AAFwkTag::APPKIT, "start callback resource overlimit");
    resourceOverlimitCB_(atObj);
}

void ApplicationDataManager::NotifyAppFault(const FaultData &faultData)
{
    if (!faultData.notifyApp) {
        return;
    }
    if (faultData.faultType == FaultDataType::APP_TELEMETRY) {
        NotifyAppTelemetry(faultData.atLeakType);
        return;
    }
    if (faultData.faultType == FaultDataType::RESOURCE_CONTROL) {
        LeakObject leakObj = {
            .leakType = faultData.leakObject.leakType,
            .leakSize = faultData.leakObject.leakSize,
            .detailInfo = faultData.leakObject.detailInfo,
        };
        NotifyLeakObject(leakObj);
        return;
    }
    ErrorObject faultErrorObj = {
        .name = faultData.errorObject.name,
        .message = faultData.errorObject.message,
        .stack = faultData.errorObject.stack
    };
    NotifyExceptionObject(faultErrorObj);
}
}  // namespace AppExecFwk
}  // namespace OHOS
