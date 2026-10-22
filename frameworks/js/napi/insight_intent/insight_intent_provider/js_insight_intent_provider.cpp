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

#include "js_insight_intent_provider.h"

#include <memory>
#include <mutex>
#include <vector>

#include "hilog_tag_wrapper.h"
#include "js_runtime_utils.h"
#include "js_error_utils.h"
#include "napi/native_node_api.h"

#include "insight_intent_will_execute_callback_mgr.h"
#include "js_insight_intent_execution_info.h"
#include "napi_common_execute_result.h"
#include "insight_intent_delay_result_callback_mgr.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const char *JS_INSIGHT_INTENT_PROVIDER_NAME = "JsInsightIntentProvider";
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_ONE = 1;
const char *WILL_EXECUTE_METHOD = "onWillInsightIntentExecuteCallBack";
} // namespace

class JsInsightIntentWillExecuteCallbackClient
    : public std::enable_shared_from_this<JsInsightIntentWillExecuteCallbackClient> {
public:
    JsInsightIntentWillExecuteCallbackClient(napi_env env, napi_ref callbackRef)
        : env_(env), callbackRef_(callbackRef) {}

    ~JsInsightIntentWillExecuteCallbackClient()
    {
        if (env_ == nullptr || callbackRef_ == nullptr) {
            return;
        }
        napi_env env = env_;
        napi_ref ref = callbackRef_;
        callbackRef_ = nullptr;
        auto deleteTask = [env, ref]() {
            napi_status status = napi_delete_reference(env, ref);
            if (status != napi_ok) {
                TAG_LOGE(AAFwkTag::INTENT, "napi_delete_reference failed: %{public}d", status);
            }
        };
        if (napi_send_event(env, deleteTask, napi_eprio_high) != napi_ok) {
            TAG_LOGE(AAFwkTag::INTENT, "napi_send_event failed, skip cross-thread reference release");
        }
    }

    void Process(const AppExecFwk::InsightIntentExecutionInfo &info)
    {
        if (env_ == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null env");
            return;
        }
        std::shared_ptr<JsInsightIntentWillExecuteCallbackClient> client = shared_from_this();
        napi_env env = env_;
        auto asyncTask = [client, env, info]() {
            client->CallJsWillExecute(env, info);
        };
        if (napi_send_event(env, asyncTask, napi_eprio_immediate) != napi_ok) {
            TAG_LOGE(AAFwkTag::INTENT, "send will execute event failed");
        }
    }

    bool IsStrictEquals(napi_env env, napi_value callback)
    {
        bool isEquals = false;
        napi_value refValue = nullptr;
        napi_get_reference_value(env_, callbackRef_, &refValue);
        napi_strict_equals(env, refValue, callback, &isEquals);
        return isEquals;
    }

private:
    void CallJsWillExecute(napi_env env, const AppExecFwk::InsightIntentExecutionInfo &info)
    {
        HandleScope handleScope(env);
        napi_value callback = nullptr;
        napi_get_reference_value(env, callbackRef_, &callback);
        napi_value executionInfo = CreateJsInsightIntentExecutionInfo(env, info);
        napi_value method = nullptr;
        napi_get_named_property(env, callback, WILL_EXECUTE_METHOD, &method);
        napi_value result = nullptr;
        napi_call_function(env, callback, method, 1, &executionInfo, &result);
    }

    napi_env env_ = nullptr;
    napi_ref callbackRef_ = nullptr;
};

class JsInsightIntentWillExecuteCallback {
public:
    struct CallbackEntry {
        uint32_t callbackId = 0;
        std::shared_ptr<JsInsightIntentWillExecuteCallbackClient> client = nullptr;
    };

    static napi_value OnWillInsightIntentExecute(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentWillExecuteCallback, OnWillInsightIntentExecute);
    }

    static napi_value OffWillInsightIntentExecute(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentWillExecuteCallback, OffWillInsightIntentExecute);
    }

private:
    static napi_value OnWillInsightIntentExecute(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::INTENT, "on will insight intent execute napi called");
        if (info.argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::INTENT, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        napi_ref callbackRef = nullptr;
        napi_create_reference(env, info.argv[0], 1, &callbackRef);
        auto client = std::make_shared<JsInsightIntentWillExecuteCallbackClient>(env, callbackRef);
        uint32_t callbackId = 0;
        auto ret = InsightIntentWillExecuteCallbackMgr::GetInstance()->Register(
            [client](const AppExecFwk::InsightIntentExecutionInfo &executionInfo) {
                client->Process(executionInfo);
            },
            callbackId);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "register will execute callback failed, ret:%{public}d", ret);
            napi_delete_reference(env, callbackRef);
            return CreateJsUndefined(env);
        }
        std::lock_guard<std::mutex> lock(entriesMutex_);
        entries_.push_back({callbackId, client});
        return CreateJsUndefined(env);
    }

    static napi_value OffWillInsightIntentExecute(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::INTENT, "off will insight intent execute napi called");
        if (info.argc < ARGC_ONE) {
            {
                std::lock_guard<std::mutex> lock(entriesMutex_);
                entries_.clear();
            }
            InsightIntentWillExecuteCallbackMgr::GetInstance()->UnregisterAll();
            return CreateJsUndefined(env);
        }
        uint32_t callbackId = 0;
        bool found = false;
        {
            std::lock_guard<std::mutex> lock(entriesMutex_);
            for (auto it = entries_.begin(); it != entries_.end(); ++it) {
                if (it->client != nullptr && it->client->IsStrictEquals(env, info.argv[0])) {
                    callbackId = it->callbackId;
                    entries_.erase(it);
                    found = true;
                    break;
                }
            }
        }
        if (found) {
            InsightIntentWillExecuteCallbackMgr::GetInstance()->Unregister(callbackId);
        }
        return CreateJsUndefined(env);
    }

    static std::vector<CallbackEntry> entries_;
    static std::mutex entriesMutex_;
};

std::vector<JsInsightIntentWillExecuteCallback::CallbackEntry> JsInsightIntentWillExecuteCallback::entries_;
std::mutex JsInsightIntentWillExecuteCallback::entriesMutex_;

class JsInsightIntentProvider {
public:
    JsInsightIntentProvider() = default;
    ~JsInsightIntentProvider() = default;

    static void Finalizer(napi_env env, void *data, void *hint)
    {
        TAG_LOGD(AAFwkTag::INTENT, "called");
        std::unique_ptr<JsInsightIntentProvider>(static_cast<JsInsightIntentProvider*>(data));
    }

    static napi_value SendExecuteResult(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentProvider, OnSendExecuteResult);
    }

    static napi_value SendIntentResult(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentProvider, OnSendIntentResult);
    }

private:
    napi_value OnSendExecuteResult(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::INTENT, "called");
        return OnSendExecuteResultCommon(env, info, false);
    }

    napi_value OnSendIntentResult(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::INTENT, "called");
        return OnSendExecuteResultCommon(env, info, true);
    }

    napi_value OnSendExecuteResultCommon(napi_env env, NapiCallbackInfo& info, bool isDecorator)
    {
        TAG_LOGD(AAFwkTag::INTENT, "called");
        HandleEscape handleEscape(env);
        if (info.argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::INTENT, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        int32_t insightIntentId = -1;
        if (!ConvertFromJsValue(env, info.argv[0], insightIntentId)) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse insightIntentId failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        auto nativeResult = std::make_shared<AppExecFwk::InsightIntentExecuteResult>();
        if (!UnwrapExecuteResult(env, info.argv[1], *nativeResult, isDecorator)) {
            TAG_LOGE(AAFwkTag::INTENT, " failed to UnwrapExecuteResult");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
        auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute = [innerErrorCode, insightIntentId, nativeResult, isDecorator]() {
            *innerErrorCode = InsightIntentDelayResultCallbackMgr::GetInstance().HandleExecuteDone(
                insightIntentId, *nativeResult, isDecorator);
        };

        NapiAsyncTask::CompleteCallback complete = [innerErrorCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            if (*innerErrorCode != ERR_OK) {
                TAG_LOGE(AAFwkTag::INTENT, "error: %{public}d",
                    *innerErrorCode);
                task.Reject(env, CreateJsError(env, static_cast<AbilityErrorCode>(*innerErrorCode)));
                return;
            }
            task.ResolveWithNoError(env, CreateJsUndefined(env));
        };

        napi_value lastParam = (info.argc > ARGC_TWO) ? info. argv[ARGC_TWO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsInsightIntentProvider::OnSendIntentResult", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }
};

napi_value CreateJsInsightIntentProvider(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env or exportObj");
        return nullptr;
    }

    HandleScope handleScope(env);
    std::unique_ptr<JsInsightIntentProvider> provider = std::make_unique<JsInsightIntentProvider>();
    napi_wrap(env, exportObj, provider.release(), JsInsightIntentProvider::Finalizer, nullptr, nullptr);
    BindNativeFunction(env, exportObj, "sendExecuteResult", JS_INSIGHT_INTENT_PROVIDER_NAME,
        JsInsightIntentProvider::SendExecuteResult);
    BindNativeFunction(env, exportObj, "sendIntentResult", JS_INSIGHT_INTENT_PROVIDER_NAME,
        JsInsightIntentProvider::SendIntentResult);
    BindNativeFunction(env, exportObj, "onWillInsightIntentExecute", JS_INSIGHT_INTENT_PROVIDER_NAME,
        JsInsightIntentWillExecuteCallback::OnWillInsightIntentExecute);
    BindNativeFunction(env, exportObj, "offWillInsightIntentExecute", JS_INSIGHT_INTENT_PROVIDER_NAME,
        JsInsightIntentWillExecuteCallback::OffWillInsightIntentExecute);

    return CreateJsUndefined(env);
}
} // namespace AbilityRuntime
} // namespace OHOS
