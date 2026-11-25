/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "js_insight_intent_execute_manager.h"

#include <memory>

#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const char *JS_INSIGHT_INTENT_EXECUTE_MANAGER_NAME = "JsInsightIntentExecuteManager";
constexpr size_t ARGC_ONE = 1;
} // namespace

class JsInsightIntentExecuteManager {
public:
    JsInsightIntentExecuteManager() = default;
    ~JsInsightIntentExecuteManager() = default;

    static void Finalizer(napi_env env, void *data, void *hint)
    {
        TAG_LOGD(AAFwkTag::INTENT, "called");
        std::unique_ptr<JsInsightIntentExecuteManager>(static_cast<JsInsightIntentExecuteManager *>(data));
    }

    static napi_value GetInsightIntentCallerBundleName(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentExecuteManager, OnGetInsightIntentCallerBundleName);
    }

private:
    napi_value OnGetInsightIntentCallerBundleName(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::INTENT, "called");
        HandleEscape handleEscape(env);
        int32_t instanceId = 0;
        if (info.argc >= ARGC_ONE) {
            ConvertFromJsValue(env, info.argv[0], instanceId);
        }

        auto sharedResult = std::make_shared<std::string>();
        NapiAsyncTask::ExecuteCallback execute = [sharedResult, instanceId]() {
            *sharedResult = AAFwk::AbilityManagerClient::GetInstance()->GetInsightIntentCallerBundleName(
                static_cast<uint64_t>(instanceId));
        };

        NapiAsyncTask::CompleteCallback complete = [sharedResult](napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            task.Resolve(env, CreateJsValue(env, *sharedResult));
        };

        napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[ARGC_ONE] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsInsightIntentExecuteManager::OnGetInsightIntentCallerBundleName", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }
};

napi_value CreateJsInsightIntentExecuteManager(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env or exportObj");
        return nullptr;
    }

    HandleScope handleScope(env);
    std::unique_ptr<JsInsightIntentExecuteManager> manager = std::make_unique<JsInsightIntentExecuteManager>();
    napi_wrap(env, exportObj, manager.release(), JsInsightIntentExecuteManager::Finalizer, nullptr, nullptr);
    BindNativeFunction(env, exportObj, "getInsightIntentCallerBundleName", JS_INSIGHT_INTENT_EXECUTE_MANAGER_NAME,
        JsInsightIntentExecuteManager::GetInsightIntentCallerBundleName);

    return CreateJsUndefined(env);
}
} // namespace AbilityRuntime
} // namespace OHOS
