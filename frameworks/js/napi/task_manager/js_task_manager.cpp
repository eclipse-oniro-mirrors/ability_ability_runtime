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
// Aligned with the TaskPriority enum of @ohos.app.agent.taskManager and the limits checked
// again on the service side (services/appmgr task_mgr); kept local because the service-side
// common.h is not exported to the NAPI layer.
constexpr int32_t TASK_PRIORITY_LOW = 100;
constexpr int32_t TASK_PRIORITY_MEDIUM = 200;
constexpr int32_t TASK_PRIORITY_HIGH = 300;
constexpr size_t TASK_ID_MIN_LEN = 1;
constexpr size_t TASK_ID_MAX_LEN = 256;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;

napi_status SetEnumItem(napi_env env, napi_value object, const char *name, int32_t value)
{
    napi_status status;
    napi_value itemName = nullptr;
    napi_value itemValue = nullptr;
    NAPI_CALL_BASE(env, status = napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &itemName), status);
    NAPI_CALL_BASE(env, status = napi_create_int32(env, value, &itemValue), status);
    NAPI_CALL_BASE(env, status = napi_set_property(env, object, itemName, itemValue), status);
    NAPI_CALL_BASE(env, status = napi_set_property(env, object, itemValue, itemName), status);
    return napi_ok;
}

napi_value TaskPriorityInit(napi_env env)
{
    napi_value taskPriority = nullptr;
    NAPI_CALL(env, napi_create_object(env, &taskPriority));
    NAPI_CALL(env, SetEnumItem(env, taskPriority, "LOW", TASK_PRIORITY_LOW));
    NAPI_CALL(env, SetEnumItem(env, taskPriority, "MEDIUM", TASK_PRIORITY_MEDIUM));
    NAPI_CALL(env, SetEnumItem(env, taskPriority, "HIGH", TASK_PRIORITY_HIGH));
    return taskPriority;
}

bool ParsePriorityParam(napi_env env, napi_value value, int32_t &priority)
{
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, value, &valueType) != napi_ok) {
        return false;
    }
    if (valueType == napi_undefined) {
        priority = TASK_PRIORITY_MEDIUM;
        return true;
    }
    if (valueType != napi_number || !ConvertFromJsValue(env, value, priority) ||
        (priority != TASK_PRIORITY_LOW && priority != TASK_PRIORITY_MEDIUM &&
        priority != TASK_PRIORITY_HIGH)) {
        return false;
    }
    return true;
}

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
        int32_t priority = TASK_PRIORITY_MEDIUM;
        if (info.argc >= ARGC_TWO && !ParsePriorityParam(env, info.argv[INDEX_ONE], priority)) {
            TAG_LOGW(AAFwkTag::JSNAPI, "priority parse failed");
            ThrowInvalidParamError(env, "priority must be a value of TaskPriority");
            return handleEscape.Escape(CreateJsUndefined(env));
        }
        napi_value lastParam = (info.argc >= ARGC_THREE) ? info.argv[INDEX_TWO] : nullptr;
        auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute = [taskId, priority, innerErrCode]() {
            auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
            if (appMgrClient == nullptr) {
                *innerErrCode = AAFwk::INNER_ERR;
                return;
            }
            *innerErrCode = appMgrClient->StartTask(taskId, priority);
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

    napi_set_named_property(env, exportObj, "TaskPriority", TaskPriorityInit(env));

    const char *moduleName = "JsTaskManager";
    BindNativeFunction(env, exportObj, "startTask", moduleName, JsTaskManager::StartTask);
    BindNativeFunction(env, exportObj, "finishTask", moduleName, JsTaskManager::FinishTask);
    return exportObj;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
