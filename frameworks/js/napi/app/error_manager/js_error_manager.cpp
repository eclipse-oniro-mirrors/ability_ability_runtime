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

#include "js_error_manager.h"

#include <cstdint>
#include <unistd.h>

#include "ability_business_error.h"
#include "application_data_manager.h"
#include "event_runner.h"
#include "hilog_tag_wrapper.h"
#include "js_error_observer.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
struct JsLoopObserver {
    std::shared_ptr<AppExecFwk::EventRunner> mainRunner;
    std::shared_ptr<NativeReference> observerObject;
    napi_env env;
};
static std::shared_ptr<JsLoopObserver> loopObserver_;
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr const char* ON_OFF_TYPE = "error";
constexpr const char* ON_OFF_TYPE_UNHANDLED_REJECTION = "unhandledRejection";
constexpr const char* ON_OFF_TYPE_SYNC = "errorEvent";
constexpr const char* ON_OFF_TYPE_SYNC_LOOP = "loopObserver";
constexpr uint32_t INITITAL_REFCOUNT_ONE = 1;

thread_local std::set<napi_ref> unhandledRejectionObservers;
thread_local std::map<napi_ref, napi_ref> pendingUnHandledRejections;

napi_value AddRejection(napi_env env, napi_value promise, napi_value reason)
{
    napi_ref promiseRef = nullptr;
    NAPI_CALL(env, napi_create_reference(env, promise, INITITAL_REFCOUNT_ONE, &promiseRef));
    napi_ref reasonRef = nullptr;
    NAPI_CALL(env, napi_create_reference(env, reason, INITITAL_REFCOUNT_ONE, &reasonRef));
    pendingUnHandledRejections.insert(std::make_pair(promiseRef, reasonRef));
    return CreateJsUndefined(env);
}

napi_value RemoveRejection(napi_env env, napi_value promise)
{
    napi_value ret = CreateJsUndefined(env);
    auto iter = pendingUnHandledRejections.begin();
    while (iter != pendingUnHandledRejections.end()) {
        napi_value prom = nullptr;
        NAPI_CALL(env, napi_get_reference_value(env, iter->first, &prom));
        bool isEquals = false;
        NAPI_CALL(env, napi_strict_equals(env, promise, prom, &isEquals));
        if (isEquals) {
            NAPI_CALL(env, napi_delete_reference(env, iter->first));
            NAPI_CALL(env, napi_delete_reference(env, iter->second));
            pendingUnHandledRejections.erase(iter);
            return ret;
        }
        ++iter;
    }
    return ret;
}

napi_value UnhandledRejectionHandler(napi_env env, napi_value promise, napi_value reason)
{
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));
    size_t argc = ARGC_TWO;
    napi_value args[] = {reason, promise};
    for (auto& iter : unhandledRejectionObservers) {
        napi_value cb = nullptr;
        NAPI_CALL(env, napi_get_reference_value(env, iter, &cb));
        napi_value result = nullptr;
        NAPI_CALL(env, napi_call_function(env, global, cb, argc, args, &result));
    }
    return CreateJsUndefined(env);
}

static napi_value OnUnhandledRejection(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_THREE; // 3 parameter size
    napi_value argv[ARGC_THREE] = {0}; // 3 array length
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    int32_t event = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &event));
    if (event == static_cast<int32_t>(UnhandledRejectionEvent::REJECT)) {
        return AddRejection(env, argv[INDEX_ONE], argv[INDEX_TWO]); // 2 array index
    }
    if (event == static_cast<int32_t>(UnhandledRejectionEvent::HANDLE)) {
        return RemoveRejection(env, argv[INDEX_ONE]);
    }
    ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    return CreateJsUndefined(env);
}

static napi_value NotifyUnhandledRejectionHandler(napi_env env, napi_callback_info info)
{
    if (!pendingUnHandledRejections.empty()) {
        auto iter = pendingUnHandledRejections.begin();
        while (iter != pendingUnHandledRejections.end()) {
            napi_value promise = nullptr;
            NAPI_CALL(env, napi_get_reference_value(env, iter->first, &promise));
            napi_value reason = nullptr;
            NAPI_CALL(env, napi_get_reference_value(env, iter->second, &reason));

            UnhandledRejectionHandler(env, promise, reason);

            NAPI_CALL(env, napi_delete_reference(env, iter->first));
            NAPI_CALL(env, napi_delete_reference(env, iter->second));
            iter = pendingUnHandledRejections.erase(iter);
        }
    }
    return CreateJsUndefined(env);
}

class JsErrorManager final {
public:
    JsErrorManager() {}
    ~JsErrorManager() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGI(AAFwkTag::JSNAPI, "finalizer called");
        std::unique_ptr<JsErrorManager>(static_cast<JsErrorManager*>(data));
        ClearReference(env);
    }

    static napi_value On(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsErrorManager, OnOn);
    }

    static napi_value Off(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsErrorManager, OnOff);
    }

    napi_value SetRejectionCallback(napi_env env) const
    {
        napi_value rejectCallback = nullptr;
        std::string rejectCallbackName = "OnUnhandledRejection";
        NAPI_CALL(env, napi_create_function(env,
                                            rejectCallbackName.c_str(),
                                            rejectCallbackName.size(),
                                            OnUnhandledRejection,
                                            nullptr, &rejectCallback));
        napi_ref rejectCallbackRef = nullptr;
        NAPI_CALL(env, napi_create_reference(env, rejectCallback, INITITAL_REFCOUNT_ONE, &rejectCallbackRef));

        napi_value checkCallback = nullptr;
        std::string checkCallbackName = "NotifyUnhandledRejectionHandler";
        NAPI_CALL(env, napi_create_function(env,
                                            checkCallbackName.c_str(),
                                            checkCallbackName.size(),
                                            NotifyUnhandledRejectionHandler,
                                            nullptr, &checkCallback));
        napi_ref checkCallbackRef = nullptr;
        NAPI_CALL(env, napi_create_reference(env, checkCallback, INITITAL_REFCOUNT_ONE, &checkCallbackRef));

        NAPI_CALL(env, napi_set_promise_rejection_callback(env, rejectCallbackRef, checkCallbackRef));

        return CreateJsUndefined(env);
    }

    static void ClearReference(napi_env env)
    {
        for (auto& iter : unhandledRejectionObservers) {
            napi_delete_reference(env, iter);
        }
        unhandledRejectionObservers.clear();

        auto iter = pendingUnHandledRejections.begin();
        while (iter != pendingUnHandledRejections.end()) {
            napi_delete_reference(env, iter->first);
            napi_delete_reference(env, iter->second);
            ++iter;
        }
        pendingUnHandledRejections.clear();
    }

private:
    napi_value OnOn(napi_env env, const size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::JSNAPI, "called");
        std::string type = ParseParamType(env, argc, argv);
        if (type == ON_OFF_TYPE_SYNC) {
            return OnOnNew(env, argc, argv);
        }
        if (type == ON_OFF_TYPE_SYNC_LOOP) {
            if (!AppExecFwk::EventRunner::IsAppMainThread()) {
                TAG_LOGE(AAFwkTag::JSNAPI, "not mainThread");
                ThrowInvalidCallerError(env);
                return CreateJsUndefined(env);
            }
            return OnSetLoopWatch(env, argc, argv);
        }
        if (type == ON_OFF_TYPE_UNHANDLED_REJECTION) {
            if (argc != ARGC_TWO) {
                TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
                ThrowInvalidNumParametersError(env);
                return CreateJsUndefined(env);
            }
            return OnOnUnhandledRejection(env, argv[INDEX_ONE]);
        }
        return OnOnOld(env, argc, argv);
    }

    napi_value OnOnOld(napi_env env, const size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::JSNAPI, "called");
        if (argc != ARGC_TWO) {
            TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string type;
        if (!ConvertFromJsValue(env, argv[INDEX_ZERO], type) || type != ON_OFF_TYPE) {
            ThrowInvalidParamError(env, "Parameter error: Parse type failed, must be a string error.");
            TAG_LOGE(AAFwkTag::JSNAPI, "parse type failed");
            return CreateJsUndefined(env);
        }
        int32_t observerId = serialNumber_;
        if (serialNumber_ < INT32_MAX) {
            serialNumber_++;
        } else {
            serialNumber_ = 0;
        }

        if (observer_ == nullptr) {
            TAG_LOGD(AAFwkTag::JSNAPI, "null observer_");
            // create observer
            observer_ = std::make_shared<JsErrorObserver>(env);
            AppExecFwk::ApplicationDataManager::GetInstance().AddErrorObserver(observer_);
        }
        observer_->AddJsObserverObject(observerId, argv[INDEX_ONE]);
        return CreateJsValue(env, observerId);
    }

    napi_value OnOnUnhandledRejection(napi_env env, napi_value function)
    {
        if (!ValidateFunction(env, function)) {
            return nullptr;
        }
        for (auto& iter : unhandledRejectionObservers) {
            napi_value observer = nullptr;
            NAPI_CALL(env, napi_get_reference_value(env, iter, &observer));
            bool equals = false;
            NAPI_CALL(env, napi_strict_equals(env, observer, function, &equals));
            if (equals) {
                NAPI_CALL(env, napi_delete_reference(env, iter));
                unhandledRejectionObservers.erase(iter);
                break;
            }
        }
        napi_ref myCallRef = nullptr;
        NAPI_CALL(env, napi_create_reference(env, function, INITITAL_REFCOUNT_ONE, &myCallRef));
        unhandledRejectionObservers.insert(myCallRef);
        return nullptr;
    }

    napi_value OnOnNew(napi_env env, const size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::JSNAPI, "called");
        if (argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        if (!CheckTypeForNapiValue(env, argv[INDEX_ONE], napi_object)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "invalid param");
            ThrowInvalidParamError(env, "Parameter error: Parse observer failed, must be a ErrorObserver.");
            return CreateJsUndefined(env);
        }

        int32_t observerId = serialNumber_;
        if (serialNumber_ < INT32_MAX) {
            serialNumber_++;
        } else {
            serialNumber_ = 0;
        }

        if (observer_ == nullptr) {
            // create observer
            observer_ = std::make_shared<JsErrorObserver>(env);
            AppExecFwk::ApplicationDataManager::GetInstance().AddErrorObserver(observer_);
        }
        observer_->AddJsObserverObject(observerId, argv[INDEX_ONE], true);
        return CreateJsValue(env, observerId);
    }

    napi_value OnOff(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::JSNAPI, "called");
        std::string type = ParseParamType(env, argc, argv);
        if (type == ON_OFF_TYPE_SYNC) {
            return OnOffNew(env, argc, argv);
        }
        if (type == ON_OFF_TYPE_SYNC_LOOP) {
            if (!AppExecFwk::EventRunner::IsAppMainThread()) {
                TAG_LOGE(AAFwkTag::JSNAPI, "not mainThread");
                ThrowInvalidCallerError(env);
                return CreateJsUndefined(env);
            }
            return OnRemoveLoopWatch(env, argc, argv);
        }
        if (type == ON_OFF_TYPE_UNHANDLED_REJECTION) {
            if (argc != ARGC_TWO && argc != ARGC_ONE) {
                TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
                ThrowInvalidNumParametersError(env);
                return CreateJsUndefined(env);
            }
            return OnOffUnhandledRejection(env, argc, argv);
        }
        return OnOffOld(env, argc, argv);
    }

    napi_value OnOffOld(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::JSNAPI, "called");
        int32_t observerId = -1;
        if (argc != ARGC_TWO && argc != ARGC_THREE) {
            ThrowTooFewParametersError(env);
            TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
        } else {
            napi_get_value_int32(env, argv[INDEX_ONE], &observerId);
            TAG_LOGI(AAFwkTag::JSNAPI, "observer:%{public}d", observerId);
        }

        std::string type;
        if (!ConvertFromJsValue(env, argv[INDEX_ZERO], type) || type != ON_OFF_TYPE) {
            TAG_LOGE(AAFwkTag::JSNAPI, "parse type failed");
            ThrowInvalidParamError(env, "Parameter error: Parse type failed, must be a string error.");
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [&observer = observer_, observerId](
                napi_env env, NapiAsyncTask& task, int32_t status) {
            TAG_LOGI(AAFwkTag::JSNAPI, "complete called");
                if (observerId == -1) {
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
                    return;
                }
                if (observer && observer->RemoveJsObserverObject(observerId)) {
                    task.ResolveWithNoError(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_ID));
                }
                if (observer && observer->IsEmpty()) {
                    AppExecFwk::ApplicationDataManager::GetInstance().RemoveErrorObserver();
                    observer = nullptr;
                }
            };

        napi_value lastParam = (argc <= ARGC_TWO) ? nullptr : argv[INDEX_TWO];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JSErrorManager::OnUnregisterErrorObserver",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnOffUnhandledRejection(napi_env env, size_t argc, napi_value* argv)
    {
        auto res = CreateJsUndefined(env);
        if (argc == ARGC_ONE) {
            for (auto& iter : unhandledRejectionObservers) {
                napi_delete_reference(env, iter);
            }
            unhandledRejectionObservers.clear();
            return res;
        }
        napi_value function = argv[INDEX_ONE];
        if (!ValidateFunction(env, function)) {
            return res;
        }
        for (auto& iter : unhandledRejectionObservers) {
            napi_value observer = nullptr;
            NAPI_CALL(env, napi_get_reference_value(env, iter, &observer));
            bool equals = false;
            NAPI_CALL(env, napi_strict_equals(env, observer, function, &equals));
            if (equals) {
                NAPI_CALL(env, napi_delete_reference(env, iter));
                unhandledRejectionObservers.erase(iter);
                return res;
            }
        }
        TAG_LOGE(AAFwkTag::JSNAPI, "remove observer failed");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_OBSERVER_NOT_FOUND);
        return res;
    }

    napi_value OnOffNew(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::JSNAPI, "called");
        if (argc < ARGC_TWO) {
            ThrowTooFewParametersError(env);
            TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
            return CreateJsUndefined(env);
        }
        int32_t observerId = -1;
        if (!ConvertFromJsValue(env, argv[INDEX_ONE], observerId)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "parse observerId failed");
            ThrowInvalidParamError(env, "Parameter error: Parse observerId failed, must be a number.");
            return CreateJsUndefined(env);
        }
        if (observer_ == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "null observer");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
        if (observer_->RemoveJsObserverObject(observerId, true)) {
            TAG_LOGD(AAFwkTag::JSNAPI, "success");
        } else {
            TAG_LOGE(AAFwkTag::JSNAPI, "failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_ID);
        }
        if (observer_->IsEmpty()) {
            AppExecFwk::ApplicationDataManager::GetInstance().RemoveErrorObserver();
            observer_ = nullptr;
        }
        return CreateJsUndefined(env);
    }

    static void CallJsFunction(napi_env env, napi_value obj, const char* methodName,
        napi_value const* argv, size_t argc)
    {
        TAG_LOGI(AAFwkTag::JSNAPI, "call func: %{public}s", methodName);
        if (obj == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "null obj");
            return;
        }

        napi_value method = nullptr;
        napi_get_named_property(env, obj, methodName, &method);
        if (method == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "null method");
            return;
        }
        napi_value callResult = nullptr;
        napi_call_function(env, obj, method, argc, argv, &callResult);
    }

    static void CallbackTimeout(int64_t number)
    {
        std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
            ([number](napi_env env, NapiAsyncTask &task, int32_t status) {
                if (loopObserver_ == nullptr) {
                    TAG_LOGE(AAFwkTag::JSNAPI, "null loopObserver_");
                    return;
                }
                if (loopObserver_->env == nullptr) {
                    TAG_LOGE(AAFwkTag::JSNAPI, "null env");
                    return;
                }
                if (loopObserver_->observerObject == nullptr) {
                    TAG_LOGE(AAFwkTag::JSNAPI, "null observer");
                    return;
                }
                napi_value jsValue[] = { CreateJsValue(loopObserver_->env, number) };
                CallJsFunction(loopObserver_->env, loopObserver_->observerObject->GetNapiValue(), "onLoopTimeOut",
                    jsValue, ARGC_ONE);
            });
        napi_ref callback = nullptr;
        std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
        if (loopObserver_ && loopObserver_->env) {
            NapiAsyncTask::Schedule("JsErrorObserver::CallbackTimeout",
                loopObserver_->env, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
        }
    }

    napi_value OnSetLoopWatch(napi_env env, size_t argc, napi_value* argv)
    {
        if (argc != ARGC_THREE) {
            TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        if (!CheckTypeForNapiValue(env, argv[INDEX_ONE], napi_number)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "invalid param");
            ThrowInvalidParamError(env, "Parameter error: Failed to parse timeout, must be a number.");
            return CreateJsUndefined(env);
        }
        if (!CheckTypeForNapiValue(env, argv[INDEX_TWO], napi_object)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "invalid param");
            ThrowInvalidParamError(env, "Parameter error: Failed to parse observer, must be a LoopObserver.");
            return CreateJsUndefined(env);
        }
        int64_t number;
        if (!ConvertFromJsNumber(env, argv[INDEX_ONE], number)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "parse timeout failed");
            ThrowInvalidParamError(env, "Parameter error: Failed to parse timeout, must be a number.");
            return CreateJsUndefined(env);
        }
        if (number <= 0) {
            TAG_LOGE(AAFwkTag::JSNAPI, "timeout<=0");
            ThrowInvalidParamError(env, "Parameter error: The timeout cannot be less than 0.");
            return CreateJsUndefined(env);
        }

        if (loopObserver_ == nullptr) {
            loopObserver_ = std::make_shared<JsLoopObserver>();
        }
        loopObserver_->mainRunner = AppExecFwk::EventRunner::GetMainEventRunner();
        napi_ref ref = nullptr;
        napi_create_reference(env, argv[INDEX_TWO], INITITAL_REFCOUNT_ONE, &ref);
        loopObserver_->observerObject = std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
        loopObserver_->env = env;
        loopObserver_->mainRunner->SetTimeout(number);
        loopObserver_->mainRunner->SetTimeoutCallback(CallbackTimeout);
        return nullptr;
    }

    napi_value OnRemoveLoopWatch(napi_env env, size_t argc, napi_value* argv)
    {
        if (loopObserver_) {
            loopObserver_.reset();
            loopObserver_ = nullptr;
            TAG_LOGI(AAFwkTag::JSNAPI, "success");
        } else {
            TAG_LOGI(AAFwkTag::JSNAPI, "called");
        }
        return nullptr;
    }

    std::string ParseParamType(napi_env env, const size_t argc, napi_value* argv)
    {
        std::string type;
        if (argc > INDEX_ZERO && ConvertFromJsValue(env, argv[INDEX_ZERO], type)) {
            return type;
        }
        return "";
    }

    bool ValidateFunction(napi_env env, napi_value function)
    {
        if (function == nullptr ||
            CheckTypeForNapiValue(env, function, napi_null) ||
            CheckTypeForNapiValue(env, function, napi_undefined)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "invalid func");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return false;
        }
        return true;
    }

    int32_t serialNumber_ = 0;
    std::shared_ptr<JsErrorObserver> observer_;
};
} // namespace

napi_value JsErrorManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "null env or exportObj");
        return nullptr;
    }
    std::unique_ptr<JsErrorManager> jsErrorManager = std::make_unique<JsErrorManager>();
    jsErrorManager->SetRejectionCallback(env);
    napi_wrap(env, exportObj, jsErrorManager.release(), JsErrorManager::Finalizer, nullptr, nullptr);

    TAG_LOGD(AAFwkTag::JSNAPI, "bind func ready");
    const char *moduleName = "JsErrorManager";
    BindNativeFunction(env, exportObj, "on", moduleName, JsErrorManager::On);
    BindNativeFunction(env, exportObj, "off", moduleName, JsErrorManager::Off);
    return CreateJsUndefined(env);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
