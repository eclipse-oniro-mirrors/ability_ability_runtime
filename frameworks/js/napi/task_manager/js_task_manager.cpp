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

#include "js_task_manager.h"

#include <cstddef>
#include <memory>
#include <string>

#include "ability_manager_errors.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "appmgr/app_mgr_client.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t TASK_ID_MIN_LEN = 1;
constexpr size_t TASK_ID_MAX_LEN = 256;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;

class JsTaskManager final {
public:
    JsTaskManager() = default;
    ~JsTaskManager() = default;

    static void Finalizer(napi_env env, void *data, void *hint)
    {
        TAG_LOGD(AAFwkTag::JSNAPI, "finalizer");
        delete reinterpret_cast<JsTaskManager *>(data);
    }

    static napi_value StartTask(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsTaskManager, OnStartTask);
    }

    static napi_value FinishTask(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsTaskManager, OnFinishTask);
    }

private:
    napi_value OnStartTask(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::JSNAPI, "called");
        HandleEscape handleEscape(env);
        if (info.argc < ARGC_ONE) {
            TAG_LOGW(AAFwkTag::JSNAPI, "not enough params");
            ThrowTooFewParametersError(env);
            return handleEscape.Escape(CreateJsUndefined(env));
        }
        std::string taskId;
        if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], taskId) || taskId.size() < TASK_ID_MIN_LEN ||
            taskId.size() > TASK_ID_MAX_LEN) {
            TAG_LOGW(AAFwkTag::JSNAPI, "taskId parse failed");
            ThrowInvalidParamError(env, "taskId must be a string of 1~256 characters");
            return handleEscape.Escape(CreateJsUndefined(env));
        }
        napi_value lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
        auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute = [taskId, innerErrCode]() {
            auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
            if (appMgrClient == nullptr) {
                *innerErrCode = AAFwk::INNER_ERR;
                return;
            }
            *innerErrCode = appMgrClient->StartTask(taskId);
        };
        NapiAsyncTask::CompleteCallback complete = [innerErrCode](napi_env env, NapiAsyncTask &task,
            int32_t status) {
            HandleScope handleScope(env);
            if (*innerErrCode == ERR_OK) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
                return;
            }
            TAG_LOGE(AAFwkTag::JSNAPI, "startTask failed %{public}d", *innerErrCode);
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
        };
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsTaskManager::OnStartTask", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnFinishTask(napi_env env, NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::JSNAPI, "called");
        HandleEscape handleEscape(env);
        if (info.argc < ARGC_ONE) {
            TAG_LOGW(AAFwkTag::JSNAPI, "not enough params");
            ThrowTooFewParametersError(env);
            return handleEscape.Escape(CreateJsUndefined(env));
        }
        std::string taskId;
        if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], taskId) || taskId.size() < TASK_ID_MIN_LEN ||
            taskId.size() > TASK_ID_MAX_LEN) {
            TAG_LOGW(AAFwkTag::JSNAPI, "taskId parse failed");
            ThrowInvalidParamError(env, "taskId must be a string of 1~256 characters");
            return handleEscape.Escape(CreateJsUndefined(env));
        }
        napi_value lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
        auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute = [taskId, innerErrCode]() {
            auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
            if (appMgrClient == nullptr) {
                *innerErrCode = AAFwk::INNER_ERR;
                return;
            }
            *innerErrCode = appMgrClient->FinishTask(taskId);
        };
        NapiAsyncTask::CompleteCallback complete = [innerErrCode](napi_env env, NapiAsyncTask &task,
            int32_t status) {
            HandleScope handleScope(env);
            if (*innerErrCode == ERR_OK) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
                return;
            }
            TAG_LOGE(AAFwkTag::JSNAPI, "finishTask failed %{public}d", *innerErrCode);
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
        };
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsTaskManager::OnFinishTask", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }
};
}  // namespace

napi_value TaskManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    std::unique_ptr<JsTaskManager> jsTaskManager = std::make_unique<JsTaskManager>();
    napi_wrap(env, exportObj, jsTaskManager.release(), JsTaskManager::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsTaskManager";
    BindNativeFunction(env, exportObj, "startTask", moduleName, JsTaskManager::StartTask);
    BindNativeFunction(env, exportObj, "finishTask", moduleName, JsTaskManager::FinishTask);
    return exportObj;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
