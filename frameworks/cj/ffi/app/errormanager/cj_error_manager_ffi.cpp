/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstring>
#include <mutex>
#include "cj_error_manager_ffi.h"
#include "cj_error_observer.h"
#include "application_data_manager.h"
#include "hilog_tag_wrapper.h"
#include "event_runner.h"

using namespace OHOS::AbilityRuntime;
using namespace OHOS::FFI;

namespace {
    constexpr int ERR_PARAM = 401;
    constexpr int ERROR_CODE_INVALID_ID = 16000003;
    constexpr int ERROR_CODE_INVALID_CALLER = 16200001;
    constexpr const char* ON_OFF_TYPE = "error";
    int32_t g_serialNumber = 0;
    std::shared_ptr<ErrorObserver> g_observer;
    std::mutex g_errorMtx;
    std::mutex g_loopMtx;

    struct CJLoopObserver {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> mainRunner;
        std::function<void(int64_t)> callbackOnLoopTimeout = nullptr;
    };
    static std::shared_ptr<CJLoopObserver> g_loopObserver;
}

extern "C" {
RetDataI32 FfiOHOSErrorManagerOn(char* onType, CErrorObserver observer)
{
    TAG_LOGI(AAFwkTag::APPKIT, "FfiOHOSErrorManagerOn.");
    RetDataI32 ret = { .code = ERR_PARAM, .data = 0 };
    if (strcmp(onType, ON_OFF_TYPE) != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "FfiOHOSErrorManagerOn failed.");
        return ret;
    }
    std::lock_guard<std::mutex> lock(g_errorMtx);
    int32_t observerId = g_serialNumber;
    if (g_serialNumber < INT32_MAX) {
        g_serialNumber++;
    } else {
        g_serialNumber = 0;
    }

    if (g_observer == nullptr) {
        TAG_LOGI(AAFwkTag::APPKIT, "g_observer is null.");
        g_observer = std::make_shared<ErrorObserver>();
        OHOS::AppExecFwk::ApplicationDataManager::GetInstance().AddErrorObserver(g_observer);
    }

    g_observer->AddObserverObject(observerId, observer);
    TAG_LOGI(AAFwkTag::APPKIT, "FfiOHOSErrorManagerOn success.");
    ret.code = SUCCESS_CODE;
    ret.data = observerId;
    return ret;
}

int FfiOHOSErrorManagerOff(char* offType, int observerId)
{
    TAG_LOGI(AAFwkTag::APPKIT, "FfiOHOSErrorManagerOff.");
    int ret = ERR_PARAM;
    TAG_LOGI(AAFwkTag::APPKIT, "unregister errorObserver called, observer:%{public}d", observerId);
    if (strcmp(offType, ON_OFF_TYPE) != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "FfiOHOSErrorManagerOff failed.");
        return ret;
    }

    std::lock_guard<std::mutex> lock(g_errorMtx);
    TAG_LOGI(AAFwkTag::APPKIT, "Unregister errorObserver called.");
    if (g_observer == nullptr || !g_observer->RemoveObserverObject(observerId)) {
        ret = ERROR_CODE_INVALID_ID;
        return ret;
    }

    if (g_observer && g_observer->IsEmpty()) {
        OHOS::AppExecFwk::ApplicationDataManager::GetInstance().RemoveErrorObserver();
        g_observer = nullptr;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "FfiOHOSErrorManagerOff success.");
    ret = SUCCESS_CODE;
    return ret;
}

static void CallbackTimeout(int64_t timeout)
{
    std::shared_ptr<CJLoopObserver> observer;
    {
        std::lock_guard<std::mutex> lock(g_loopMtx);
        observer = g_loopObserver;
    }
    if (observer != nullptr && observer->callbackOnLoopTimeout != nullptr) {
        observer->callbackOnLoopTimeout(timeout);
    }
}

int32_t FfiOHOSErrorManagerLoopObserverOn(int64_t timeout, CLoopObserver observer)
{
    TAG_LOGD(AAFwkTag::APPKIT, "FfiOHOSErrorManagerLoopObserverOn.");
    if (!OHOS::AppExecFwk::EventRunner::IsAppMainThread()) {
        TAG_LOGE(AAFwkTag::APPKIT, "not mainThread");
        return ERROR_CODE_INVALID_CALLER;
    }
    if (timeout <= 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "Param invalid, timeout<=0");
        return ERR_PARAM;
    }
    {
        std::lock_guard<std::mutex> lock(g_loopMtx);
        if (g_loopObserver == nullptr) {
            g_loopObserver = std::make_shared<CJLoopObserver>();
        }
        if (observer.callbackOnLoopTimeout != nullptr) {
            g_loopObserver->callbackOnLoopTimeout = CJLambda::Create(observer.callbackOnLoopTimeout);
        }
        g_loopObserver->mainRunner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        if (g_loopObserver->mainRunner == nullptr) {
            return ERR_PARAM;
        }
        g_loopObserver->mainRunner->SetTimeout(timeout);
        g_loopObserver->mainRunner->SetTimeoutCallback(CallbackTimeout);
    }
    return SUCCESS_CODE;
}

int32_t FfiOHOSErrorManagerLoopObserverOff()
{
    TAG_LOGD(AAFwkTag::APPKIT, "FfiOHOSErrorManagerLoopObserverOff.");
    if (!OHOS::AppExecFwk::EventRunner::IsAppMainThread()) {
        TAG_LOGE(AAFwkTag::APPKIT, "not mainThread");
        return ERROR_CODE_INVALID_CALLER;
    }
    {
        std::lock_guard<std::mutex> lock(g_loopMtx);
        if (g_loopObserver) {
            g_loopObserver.reset();
            g_loopObserver = nullptr;
            TAG_LOGI(AAFwkTag::APPKIT, "LoopObserverOff success");
        }
    }
    return SUCCESS_CODE;
}
}