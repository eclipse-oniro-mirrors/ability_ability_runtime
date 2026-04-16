/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APPLICATION_DATA_MANAGER_H
#define OHOS_ABILITY_RUNTIME_APPLICATION_DATA_MANAGER_H

#include <string>
#include <mutex>

#include "napi/native_api.h"

#include "ierror_observer.h"
#include "nocopyable.h"
#include "fault_data.h"

namespace OHOS {
namespace AppExecFwk {
struct RegisterResourceParams {
    uint64_t appTelemetryLeakType { 0 };
    int thresholdPss { INT_MAX };
    int thresholdGpu { INT_MAX };
    int thresholdFd { INT_MAX };
    int thresholdRAT { INT_MAX };  // rss_ark_ts
    int thresholdRNH { INT_MAX };  // rss_native_heap
};
typedef void (*EtsErrorCallback)(const AppExecFwk::ErrorObject &errorObj);
class ApplicationDataManager {
using LeakObserverFunction = std::function<bool(const LeakObject &obj)>;
using HasOnErrorCallback = std::function<bool()>;
using ResourceOverlimitCB = std::function<void(const AppTelemetryObject& atObj)>;
public:
    struct ExceptionParams {
        napi_env env;
        napi_env mainEnv;
        napi_value exception;
        std::string summary;
        bool isUncatchable;
    };

    static std::atomic<bool> jsErrorHasReport_;
    static std::atomic<napi_env> jsErrorEnv_;
    static ApplicationDataManager &GetInstance();
    void AddErrorObserver(const std::shared_ptr<IErrorObserver> &observer);
    bool NotifyUnhandledException(const std::string &errMsg);
    bool NotifyCJUnhandledException(const std::string &errMsg);
    bool NotifyETSUnhandledException(const std::string &errMsg);
    void RemoveErrorObserver();
    bool NotifyExceptionObject(const AppExecFwk::ErrorObject &errorObj);
    bool NotifyCJExceptionObject(const AppExecFwk::ErrorObject &errorObj);
    bool NotifyETSExceptionObject(const AppExecFwk::ErrorObject &errorObj);
    void SetIsUncatchable(bool isUncatchable);
    bool GetIsUncatchable();
    static bool NotifyUncaughtException(const ExceptionParams &params, const AppExecFwk::ErrorObject &errorObj);
    void SetLeakObserver(LeakObserverFunction leakCallback);
    bool RegisterResourceObserver(RegisterResourceParams params, ResourceOverlimitCB cb);
    void SetErrorHandlerCallback(EtsErrorCallback errorCallback);
    bool NotifyETSErrorObject(const AppExecFwk::ErrorObject &errorObj);
    void RegisterHasOnErrorCallback(HasOnErrorCallback hasOnErrorCallback);
    bool GetHasOnErrorCallback();
    void NotifyAppFault(const FaultData &faultData);
private:
    ApplicationDataManager();
    ~ApplicationDataManager();
    static std::string GetFuncNameFromError(napi_env env, napi_value error);
    DISALLOW_COPY_AND_MOVE(ApplicationDataManager);
    bool WriteSandBoxXattr(RegisterResourceParams params);
    void NotifyAppTelemetry(AppTelemetryLeakType atLeakType);
    bool NotifyLeakObject(const LeakObject &leakObj);
    std::shared_ptr<IErrorObserver> errorObserver_;
    std::atomic_bool isUncatchable_;
    LeakObserverFunction leakObserver_ = nullptr;
    std::mutex leakObserverMutex_;
    std::mutex hasOnErrorCallbackMutex_;
    EtsErrorCallback errorCallback_ = nullptr;
    HasOnErrorCallback hasOnErrorCallback_ = nullptr;
    ResourceOverlimitCB resourceOverlimitCB_ = nullptr;
    std::mutex resourceMutex_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APPLICATION_DATA_MANAGER_H
