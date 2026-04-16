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

#ifndef OHOS_ABILITY_RUNTIME_IERROR_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_IERROR_OBSERVER_H

#include <string>

namespace OHOS {
namespace AppExecFwk {
struct ErrorObject {
    std::string name;
    std::string message;
    std::string stack;
    std::string mainStack;
};

enum class LeakType {
    PSS_MEMORY = 1,
    ION_MEMORY = 2,
    ASHMEM_MEMORY = 3,
    GPU_MEMORY = 4,
    FD = 5,
    THREAD = 6,
    RSS_ARK_TS = 7,
    RSS_NATIVE_HEAP = 8,
};
 
enum AppTelemetryLeakType {
    ATLT_PSS = 1 << static_cast<int>(LeakType::PSS_MEMORY),
    ATLT_GPU = 1 << static_cast<int>(LeakType::GPU_MEMORY),
    ATLT_FD = 1 << static_cast<int>(LeakType::FD),
    ATLT_RSS_ARK_TS = 1 << static_cast<int>(LeakType::RSS_ARK_TS),
    ATLT_RSS_NATIVE_HEAP = 1 << static_cast<int>(LeakType::RSS_NATIVE_HEAP),
};

struct LeakDetailInfo {
    unsigned long arktsSize = 0;
    unsigned long nativeSize = 0;
    unsigned long ionSize = 0;
    unsigned long gpuSize = 0;
    unsigned long ashmemSize = 0;
    unsigned long otherSize = 0;
};

struct LeakObject {
    LeakType leakType;
    unsigned long leakSize = 0;
    LeakDetailInfo detailInfo;
};

struct AppTelemetryObject {
    AppTelemetryLeakType atLeakType;
    std::string runningId;
};

class IErrorObserver {
public:
    IErrorObserver() = default;
    virtual ~IErrorObserver() = default;
    /**
     * Will be called when the js runtime throws an exception which doesn't caught by user.
     *
     * @param errMsg the message and error stacktrace about the exception.
     */
    virtual void OnUnhandledException(std::string errMsg) = 0;

    /**
     * When an abnormal event occurs in the native layer and the JS layer needs to be notified, it will be called.
     *
     * @param errorObj the errorObj about the exception.
     */
    virtual void OnExceptionObject(const AppExecFwk::ErrorObject &errorObj) = 0;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_IERROR_OBSERVER_H
