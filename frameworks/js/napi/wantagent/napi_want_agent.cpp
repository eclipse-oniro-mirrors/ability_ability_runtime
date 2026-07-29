/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "napi_want_agent.h"

#include <cstdio>
#include <cstring>
#include <pthread.h>
#include <unistd.h>

#include "ability_runtime_error_util.h"
#include "extension_context.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "napi_base_context.h"
#include "napi_common.h"
#include "napi_common_want_agent.h"
#include "start_options.h"
#include "want_agent_helper.h"
#include "tokenid_kit.h"
#include "js_error_utils.h"

using namespace OHOS::AbilityRuntime;
namespace OHOS {
#define NAPI_ASSERT_RETURN_NULL(env, assertion, message)    \
do {                                                        \
    if (!(assertion)) {                                     \
        TAG_LOGI(AAFwkTag::WANTAGENT, message);             \
        return nullptr;                                     \
    }                                                       \
} while (0)
namespace {
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr uint8_t INDEX_ONE = 1;
constexpr uint8_t INDEX_TWO = 2;
constexpr int32_t ERR_NOT_OK = -1;
constexpr int32_t BUSINESS_ERROR_CODE_OK = 0;
constexpr int32_t PARAMETER_ERROR = -1;

bool CheckCallerIsSystemApp()
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Non-system app forbidden to call");
        return false;
    }
    return true;
}
} // namespace


TriggerCompleteCallBack::TriggerCompleteCallBack()
{}

TriggerCompleteCallBack::~TriggerCompleteCallBack()
{}

void TriggerCompleteCallBack::SetCallbackInfo(napi_env env, NativeReference* ref)
{
    triggerCompleteInfo_.env = env;
    triggerCompleteInfo_.nativeRef.reset(ref);
}

void TriggerCompleteCallBack::SetWantAgentInstance(std::shared_ptr<WantAgent> wantAgent)
{
    triggerCompleteInfo_.wantAgent = wantAgent;
}

napi_value RetErrMsg(napi_env env, napi_value lastParam, int32_t errorCode)
{
    HandleEscape handleEscape(env);
    auto complete = [err = errorCode] (napi_env env, NapiAsyncTask &task, int32_t status) {
        task.Reject(env, CreateJsError(env, err, "PARAMETER_ERROR"));
    };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("RetErrMsg",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));

    return handleEscape.Escape(result);
}

napi_value CreateErrorValue(napi_env env, int32_t errCode)
{
    HandleEscape handleEscape(env);
    napi_value error =  NapiGetNull(env);
    if (errCode == NO_ERROR) {
        return handleEscape.Escape(error);
    }

    napi_value code = nullptr;
    napi_create_int32(env, errCode, &code);

    std::string errMsg = AbilityRuntimeErrorUtil::GetErrMessage(errCode);
    napi_value message = nullptr;
    napi_create_string_utf8(env, errMsg.c_str(), NAPI_AUTO_LENGTH, &message);

    napi_create_error(env, nullptr, message, &error);
    napi_set_named_property(env, error, "code", code);
    return handleEscape.Escape(error);
}

void NapiThrow(napi_env env, int32_t errCode)
{
    napi_throw(env, CreateErrorValue(env, errCode));
}

auto OnSendFinishedUvAfterWorkCallback = [](uv_work_t* work, int status) {
    TAG_LOGI(AAFwkTag::WANTAGENT, "trigger callback");
    TriggerReceiveDataWorker* dataWorkerData = static_cast<TriggerReceiveDataWorker *>(work->data);
    if (dataWorkerData == nullptr || dataWorkerData->env == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null dataWorkerData or env");
        delete work;
        return;
    }
    HandleScope handleScope(dataWorkerData->env);
    if (dataWorkerData->resultData == "canceled") {
        TAG_LOGI(AAFwkTag::WANTAGENT, "canceled");
        delete dataWorkerData;
        dataWorkerData = nullptr;
        delete work;
        return;
    }
    napi_value args[ARGC_TWO] = {0};
    napi_value objValueFirst = nullptr;
    napi_create_object(dataWorkerData->env, &objValueFirst);
    if (objValueFirst == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null objValueFirst");
        delete dataWorkerData;
        dataWorkerData = nullptr;
        delete work;
        return;
    }
    napi_value objValueSecond = nullptr;
    napi_create_object(dataWorkerData->env, &objValueSecond);
    if (objValueSecond == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null objValueSecond");
        delete dataWorkerData;
        dataWorkerData = nullptr;
        delete work;
        return;
    }

#ifdef ENABLE_ERRCODE
    objValueFirst = CreateJsUndefined(dataWorkerData->env);
#else
    napi_set_named_property(dataWorkerData->env, objValueFirst, "code",
        CreateJsValue(dataWorkerData->env, BUSINESS_ERROR_CODE_OK));
#endif
    napi_value jsWantAgent = OHOS::AppExecFwk::WrapWantAgent(dataWorkerData->env, dataWorkerData->wantAgent, nullptr);
    if (jsWantAgent == nullptr && dataWorkerData->wantAgent != nullptr) {
        delete dataWorkerData->wantAgent;
        dataWorkerData->wantAgent = nullptr;
    }
    napi_set_named_property(dataWorkerData->env, objValueSecond, "wantAgent", jsWantAgent);
    napi_set_named_property(dataWorkerData->env, objValueSecond, "info", jsWantAgent);
    napi_set_named_property(dataWorkerData->env, objValueSecond, "want",
        CreateJsWant(dataWorkerData->env, dataWorkerData->want));
    napi_set_named_property(dataWorkerData->env, objValueSecond, "finalCode",
        CreateJsValue(dataWorkerData->env, dataWorkerData->resultCode));
    napi_set_named_property(dataWorkerData->env, objValueSecond, "finalData",
        CreateJsValue(dataWorkerData->env, dataWorkerData->resultData));
    napi_set_named_property(dataWorkerData->env, objValueSecond, "extraInfo",
        CreateJsWantParams(dataWorkerData->env, dataWorkerData->resultExtras));
    args[ARGC_ZERO] = objValueFirst;
    args[ARGC_ONE] = objValueSecond;

    napi_value value = dataWorkerData->nativeRef->GetNapiValue();
    napi_value callback = dataWorkerData->nativeRef->GetNapiValue();
    napi_call_function(dataWorkerData->env, value, callback, ARGC_TWO, args, nullptr);
    delete dataWorkerData;
    dataWorkerData = nullptr;
    delete work;
};

void TriggerCompleteCallBack::OnSendFinished(
    const AAFwk::Want &want, int resultCode, const std::string &resultData, const AAFwk::WantParams &resultExtras)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "send finished");
    if (triggerCompleteInfo_.nativeRef == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null callBack");
        return;
    }

    uv_loop_t* loop = nullptr;
#if NAPI_VERSION >= NUMBER_OF_PARAMETERS_TWO
    napi_get_uv_event_loop(triggerCompleteInfo_.env, &loop);
#endif  // NAPI_VERSION >= 2
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null loop");
        return;
    }

    uv_work_t* work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null work");
        return;
    }
    TriggerReceiveDataWorker* dataWorker = new (std::nothrow) TriggerReceiveDataWorker();
    if (dataWorker == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null dataWorker");
        delete work;
        work = nullptr;
        return;
    }
    dataWorker->want = want;
    dataWorker->resultCode = resultCode;
    dataWorker->resultData = resultData;
    dataWorker->resultExtras = resultExtras;
    dataWorker->env = triggerCompleteInfo_.env;
    dataWorker->nativeRef = std::move(triggerCompleteInfo_.nativeRef);
    if (triggerCompleteInfo_.wantAgent != nullptr) {
        dataWorker->wantAgent = new WantAgent(triggerCompleteInfo_.wantAgent->GetPendingWant());
    }
    work->data = static_cast<void *>(dataWorker);
    int ret = uv_queue_work(loop, work, [](uv_work_t *work) {}, OnSendFinishedUvAfterWorkCallback);
    if (ret != 0) {
        delete dataWorker;
        dataWorker = nullptr;
        delete work;
        work = nullptr;
    }
}

void JsWantAgent::Finalizer(napi_env env, void* data, void* hint)
{
    std::unique_ptr<JsWantAgent>(static_cast<JsWantAgent*>(data));
}

napi_value JsWantAgent::Equal(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnEqual(env, info) : nullptr;
};

napi_value JsWantAgent::GetWant(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnGetWant(env, info) : nullptr;
};

napi_value JsWantAgent::GetOperationType(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnGetOperationType(env, info) : nullptr;
};

napi_value JsWantAgent::GetBundleName(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnGetBundleName(env, info) : nullptr;
};

napi_value JsWantAgent::GetUid(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnGetUid(env, info) : nullptr;
};

napi_value JsWantAgent::Cancel(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnCancel(env, info) : nullptr;
};

napi_value JsWantAgent::Trigger(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnTrigger(env, info) : nullptr;
};

napi_value JsWantAgent::GetWantAgent(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnGetWantAgent(env, info) : nullptr;
};

napi_value JsWantAgent::NapiGetWant(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnNapiGetWant(env, info) : nullptr;
};

napi_value JsWantAgent::NapiTrigger(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnNapiTrigger(env, info) : nullptr;
};

napi_value JsWantAgent::NapiTriggerAsync(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnNapiTriggerAsync(env, info) : nullptr;
};

napi_value JsWantAgent::NapiGetWantAgent(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnNapiGetWantAgent(env, info) : nullptr;
};

napi_value JsWantAgent::NapiCreateLocalWantAgent(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnNapiCreateLocalWantAgent(env, info) : nullptr;
};

napi_value JsWantAgent::NapiIsLocalWantAgent(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnNapiIsLocalWantAgent(env, info) : nullptr;
};

napi_value JsWantAgent::NapiGetOperationType(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnNapiGetOperationType(env, info) : nullptr;
};

napi_value JsWantAgent::NapiSetWantAgentMultithreading(napi_env env, napi_callback_info info)
{
    JsWantAgent* me = CheckParamsAndGetThis<JsWantAgent>(env, info);
    return (me != nullptr) ? me->OnNapiSetWantAgentMultithreading(env, info) : nullptr;
};

napi_value JsWantAgent::HandleInvalidParam(napi_env env, napi_value lastParam, const std::string &errorMessage)
{
    HandleEscape handleEscape(env);
    #ifdef ENABLE_ERRCODE
        ThrowInvalidParamError(env, errorMessage);
        return CreateJsUndefined(env);
    #else
        return handleEscape.Escape(RetErrMsg(env, lastParam, ERR_NOT_OK));
    #endif
}

void HandleAsyncTaskResult(napi_env env, NapiAsyncTask &task, ErrCode retCode)
{
    HandleScope handleScope(env);
    bool ret = false;
    #ifdef ENABLE_ERRCODE
        if (retCode == ERR_NOT_OK) {
            ret = false;
            task.ResolveWithNoError(env, CreateJsValue(env, ret));
        } else if (retCode == ERR_OK) {
            ret = true;
            task.ResolveWithNoError(env, CreateJsValue(env, ret));
        } else {
            task.Reject(env, CreateJsError(env, retCode, AbilityRuntimeErrorUtil::GetErrMessage(retCode)));
        }
    #else
        if (retCode != ERR_OK) {
            ret = false;
            task.Resolve(env, CreateJsValue(env, ret));
        } else {
            ret = true;
            task.Resolve(env, CreateJsValue(env, ret));
        }
    #endif
}

napi_value JsWantAgent::OnEqual(napi_env env, napi_callback_info info)
{
    HandleEscape handleEscape(env);
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    WantAgent* pWantAgentFirst = nullptr;
    WantAgent* pWantAgentSecond = nullptr;
    if (argc < ARGC_TWO || argc > ARGC_THREE) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Not enough params");
#ifdef ENABLE_ERRCODE
        ThrowTooFewParametersError(env);
#endif
        return CreateJsUndefined(env);
    }

    napi_value lastParam = (argc >= ARGC_THREE) ? argv[INDEX_TWO] : nullptr;
    if (!CheckTypeForNapiValue(env, argv[0], napi_object)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Argument not Object");
        return handleEscape.Escape(
            HandleInvalidParam(env, lastParam, "Wrong argument type. Agent must be a WantAgent."));
    }

    UnwrapWantAgent(env, argv[0], reinterpret_cast<void **>(&pWantAgentFirst));
    if (pWantAgentFirst == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Parse first failed");
        return handleEscape.Escape(
            HandleInvalidParam(env, lastParam, "Parse pWantAgentFirst failed. Agent must be a WantAgent."));
    }

    if (!CheckTypeForNapiValue(env, argv[1], napi_object)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Argument type not Object");
        return handleEscape.Escape(
            HandleInvalidParam(env, lastParam, "Wrong argument type. OtherAgent must be a WantAgent."));
    }

    UnwrapWantAgent(env, argv[1], reinterpret_cast<void **>(&pWantAgentSecond));
    if (pWantAgentSecond == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Parse second failed");
        return handleEscape.Escape(
            HandleInvalidParam(env, lastParam, "Parse pWantAgentSceond failed. OtherAgent must be a WantAgent."));
    }

    std::shared_ptr<WantAgent> wantAgentFirst = std::make_shared<WantAgent>(*pWantAgentFirst);
    std::shared_ptr<WantAgent> wantAgentSecond = std::make_shared<WantAgent>(*pWantAgentSecond);
    NapiAsyncTask::CompleteCallback complete =
        [wantAgentFirst, wantAgentSecond](napi_env env, NapiAsyncTask &task, int32_t status) {
            TAG_LOGD(AAFwkTag::WANTAGENT, "called");
            ErrCode retCode = WantAgentHelper::IsEquals(wantAgentFirst, wantAgentSecond);
            HandleAsyncTaskResult(env, task, retCode);
        };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsWantAgent::OnEqual",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JsWantAgent::OnGetWant(napi_env env, napi_callback_info info)
{
    HandleEscape handleEscape(env);
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    int32_t errCode = BUSINESS_ERROR_CODE_OK;
    WantAgent* pWantAgent = nullptr;
    if (argc > ARGC_TWO || argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Not enough arguments");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    napi_value lastParam = (argc >= ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
    if (!CheckTypeForNapiValue(env, argv[0], napi_object)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "parameter type not Object");
        ThrowInvalidParamError(env, "Wrong argument type. Agent must be a WantAgent.");
        errCode = ERR_NOT_OK;
        return handleEscape.Escape(RetErrMsg(env, lastParam, errCode));
    }

    UnwrapWantAgent(env, argv[0], reinterpret_cast<void **>(&pWantAgent));
    if (pWantAgent == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Parse first error");
        ThrowInvalidParamError(env, "Parse pWantAgent error. Agent must be a WantAgent.");
        errCode = ERR_NOT_OK;
        return handleEscape.Escape(RetErrMsg(env, lastParam, errCode));
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    NapiAsyncTask::CompleteCallback complete = [wantAgent](napi_env env, NapiAsyncTask &task, int32_t status) {
        std::shared_ptr<Want> want = WantAgentHelper::GetWant(wantAgent);
        if (want == nullptr) {
            task.Reject(env, CreateJsError(env, ERR_NOT_OK, "WantAgentHelper::GetWant result nullptr."));
            return;
        }
        task.Resolve(env, CreateJsWant(env, *(want)));
    };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsWantAgent::OnGetWant",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JsWantAgent::OnGetOperationType(napi_env env, napi_callback_info info)
{
    HandleEscape handleEscape(env);
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    int32_t errCode = BUSINESS_ERROR_CODE_OK;
    WantAgent* pWantAgent = nullptr;
    if (argc > ARGC_TWO || argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    napi_value lastParam = (argc >= ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
    if (!CheckTypeForNapiValue(env, argv[0], napi_object)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "type not Object");
        ThrowInvalidParamError(env, "Wrong argument type. Agent must be a WantAgent.");
        errCode = ERR_NOT_OK;
        return handleEscape.Escape(RetErrMsg(env, lastParam, errCode));
    }

    UnwrapWantAgent(env, argv[0], reinterpret_cast<void **>(&pWantAgent));
    if (pWantAgent == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Parse pWantAgent failed");
        ThrowInvalidParamError(env, "Parse pWantAgent failed. Agent must be a WantAgent.");
        errCode = ERR_NOT_OK;
        return handleEscape.Escape(RetErrMsg(env, lastParam, errCode));
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    NapiAsyncTask::CompleteCallback complete = [wantAgent](napi_env env, NapiAsyncTask &task, int32_t status) {
        auto ret = WantAgentHelper::GetType(wantAgent);
        task.Resolve(env, CreateJsValue(env, ret));
    };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsWantAgent::OnGetOperationType",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JsWantAgent::OnGetBundleName(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "get bundle name");
    HandleEscape handleEscape(env);
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    WantAgent* pWantAgent = nullptr;
    if (argc > ARGC_TWO || argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Not enough params");
#ifdef ENABLE_ERRCODE
        ThrowTooFewParametersError(env);
#endif
        return CreateJsUndefined(env);
    }

    napi_value lastParam = (argc >= ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
    if (!CheckTypeForNapiValue(env, argv[0], napi_object)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Argument type not Object");
#ifdef ENABLE_ERRCODE
        ThrowInvalidParamError(env, "Wrong argument type. Agent must be a WantAgent.");
        return CreateJsUndefined(env);
#else
        return handleEscape.Escape(RetErrMsg(env, lastParam, ERR_NOT_OK));
#endif
    }

    UnwrapWantAgent(env, argv[0], reinterpret_cast<void **>(&pWantAgent));
    if (pWantAgent == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Parse pWantAgent failed");
#ifdef ENABLE_ERRCODE
        ThrowInvalidParamError(env, "Parse pWantAgent failed. Agent must be a WantAgent.");
        return CreateJsUndefined(env);
#else
        return handleEscape.Escape(RetErrMsg(env, lastParam, ERR_NOT_OK));
#endif
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    NapiAsyncTask::CompleteCallback complete;
    NapiAsyncTask::ExecuteCallback execute;
    SetOnGetBundleNameCallback(wantAgent, complete, execute);

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsWantAgent::OnGetBundleName",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

void JsWantAgent::SetOnGetBundleNameCallback(std::shared_ptr<WantAgent> wantAgent,
    NapiAsyncTask::CompleteCallback &complete, NapiAsyncTask::ExecuteCallback &execute)
{
    auto retCode = std::make_shared<int32_t>(NO_ERROR);
    auto bundleName = std::make_shared<std::string>();
    execute = [wantAgent, retCode, bundleName] () {
        TAG_LOGD(AAFwkTag::WANTAGENT, "get bundle name callback");
        *retCode = WantAgentHelper::GetBundleName(wantAgent, *bundleName);
    };
    complete = [retCode, bundleName](napi_env env, NapiAsyncTask &task, int32_t status) {
        TAG_LOGD(AAFwkTag::WANTAGENT, "called");
        HandleScope handleScope(env);
#ifdef ENABLE_ERRCODE
        if (*retCode != NO_ERROR) {
            task.Reject(env, CreateJsError(env, *retCode, AbilityRuntimeErrorUtil::GetErrMessage(*retCode)));
        } else {
            task.ResolveWithNoError(env, CreateJsValue(env, *bundleName));
        }
#else
        task.Resolve(env, CreateJsValue(env, *bundleName));
#endif
    };
}

napi_value JsWantAgent::OnGetUid(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "get uid");
    HandleEscape handleEscape(env);
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    WantAgent* pWantAgent = nullptr;
    if (argc > ARGC_TWO || argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Not enough params");
#ifdef ENABLE_ERRCODE
        ThrowTooFewParametersError(env);
#endif
        return CreateJsUndefined(env);
    }

    napi_value lastParam = (argc >= ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
    if (!CheckTypeForNapiValue(env, argv[0], napi_object)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Argument type not Object");
#ifdef ENABLE_ERRCODE
        ThrowInvalidParamError(env, "Wrong argument type. Agent must be a WantAgent.");
        return CreateJsUndefined(env);
#else
        return handleEscape.Escape(RetErrMsg(env, lastParam, ERR_NOT_OK));
#endif
    }

    UnwrapWantAgent(env, argv[0], reinterpret_cast<void **>(&pWantAgent));
    if (pWantAgent == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Parse pWantAgent error");
#ifdef ENABLE_ERRCODE
        ThrowInvalidParamError(env, "Parse pWantAgent error. Agent must be a WantAgent.");
        return CreateJsUndefined(env);
#else
        return handleEscape.Escape(RetErrMsg(env, lastParam, ERR_NOT_OK));
#endif
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    NapiAsyncTask::CompleteCallback complete;
    SetOnGetUidCallback(wantAgent, complete);

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsWantAgent::OnGetUid",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return handleEscape.Escape(result);
}

void JsWantAgent::SetOnGetUidCallback(std::shared_ptr<WantAgent> wantAgent,
    NapiAsyncTask::CompleteCallback &complete)
{
    complete = [wantAgent](napi_env env, NapiAsyncTask &task, int32_t status) {
        TAG_LOGD(AAFwkTag::WANTAGENT, "called");
        HandleScope handleScope(env);
        int uid = -1;
#ifdef ENABLE_ERRCODE
        ErrCode result = WantAgentHelper::GetUid(wantAgent, uid);
        if (result != NO_ERROR) {
            task.Reject(env, CreateJsError(env, result, AbilityRuntimeErrorUtil::GetErrMessage(result)));
        } else {
            task.ResolveWithNoError(env, CreateJsValue(env, uid));
        }
#else
        WantAgentHelper::GetUid(wantAgent, uid);
        task.Resolve(env, CreateJsValue(env, uid));
#endif
    };
}

napi_value JsWantAgent::OnCancel(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "on cancel");
    HandleEscape handleEscape(env);
    WantAgent* pWantAgent = nullptr;
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGC_TWO || argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Not enough params");
#ifdef ENABLE_ERRCODE
        ThrowTooFewParametersError(env);
#endif
        return handleEscape.Escape(CreateJsUndefined(env));
    }

    napi_value lastParam = (argc >= ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
    if (!CheckTypeForNapiValue(env, argv[0], napi_object)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Argument not Object");
#ifdef ENABLE_ERRCODE
        ThrowInvalidParamError(env, "Wrong argument type. Agent must be a WantAgent.");
        return CreateJsUndefined(env);
#else
        return handleEscape.Escape(RetErrMsg(env, lastParam, ERR_NOT_OK));
#endif
    }

    UnwrapWantAgent(env, argv[0], reinterpret_cast<void **>(&pWantAgent));
    if (pWantAgent == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Parse pWantAgent failed");
#ifdef ENABLE_ERRCODE
        ThrowInvalidParamError(env, "Parse pWantAgent error. Agent must be a WantAgent.");
        return CreateJsUndefined(env);
#else
        return handleEscape.Escape(RetErrMsg(env, lastParam, ERR_NOT_OK));
#endif
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    NapiAsyncTask::CompleteCallback complete;
    SetOnCancelCallback(wantAgent, complete);

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWantAgent::OnCancel",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return handleEscape.Escape(result);
}

void JsWantAgent::SetOnCancelCallback(std::shared_ptr<WantAgent> wantAgent,
    NapiAsyncTask::CompleteCallback &complete)
{
    complete = [wantAgent](napi_env env, NapiAsyncTask &task, int32_t status) {
            TAG_LOGD(AAFwkTag::WANTAGENT, "called");
            HandleScope handleScope(env);
#ifdef ENABLE_ERRCODE
            ErrCode result = WantAgentHelper::Cancel(wantAgent);
            if (result != NO_ERROR) {
                task.Reject(env, CreateJsError(env, result, AbilityRuntimeErrorUtil::GetErrMessage(result)));
            } else {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            }
#else
            WantAgentHelper::Cancel(wantAgent);
            task.Resolve(env, CreateJsUndefined(env));
#endif
        };
}

napi_value JsWantAgent::OnTrigger(napi_env env, napi_callback_info info)
{
    HandleEscape handleEscape(env);
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_THREE) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::shared_ptr<WantAgent> wantAgent = nullptr;
    TriggerInfo triggerInfo;
    auto triggerObj = std::make_shared<TriggerCompleteCallBack>();
    int32_t errCode = UnWrapTriggerInfoParam(env, info, wantAgent, triggerInfo, triggerObj);
    if (errCode != BUSINESS_ERROR_CODE_OK) {
        return handleEscape.Escape(RetErrMsg(env, argv[ARGC_TWO], errCode));
    }

    // Public api trigger does not support localWantAgent, switch to triggerAsync.
    if (wantAgent->IsLocal()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Tigger does not support localWantAgent");
        ThrowInvalidParamError(env, "Agent can not be local.");
        return CreateJsUndefined(env);
    }

    auto execute = [wantAgent, triggerObj, triggerInfo] () {
        TAG_LOGD(AAFwkTag::WANTAGENT, "called");
        sptr<CompletedDispatcher> completedData;
        WantAgentHelper::TriggerWantAgent(wantAgent, triggerObj, triggerInfo, completedData, nullptr);
    };
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsWantAgent::OnTrigger",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), nullptr, &result));

    return CreateJsNull(env);
}

int32_t JsWantAgent::UnWrapTriggerInfoParam(napi_env env, napi_callback_info info,
    std::shared_ptr<WantAgent> &wantAgent, TriggerInfo &triggerInfo,
    std::shared_ptr<TriggerCompleteCallBack> &triggerObj)
{
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_THREE) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Not enough params");
        ThrowTooFewParametersError(env);
        return ERR_NOT_OK;
    }

    if (!CheckTypeForNapiValue(env, argv[ARGC_ZERO], napi_object)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Argument type not Object");
        ThrowInvalidParamError(env, "Wrong argument type. Agent must be a WantAgent.");
        return ERR_NOT_OK;
    }
    WantAgent* pWantAgent = nullptr;
    UnwrapWantAgent(env, argv[0], reinterpret_cast<void **>(&pWantAgent));

    if (pWantAgent == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Parse pWantAgent failed");
        ThrowInvalidParamError(env, "Parse pWantAgent failed. Agent must be a WantAgent.");
        return ERR_NOT_OK;
    }
    wantAgent = std::make_shared<WantAgent>(*pWantAgent);

    int32_t ret = GetTriggerInfo(env, argv[ARGC_ONE], triggerInfo);
    if (ret != BUSINESS_ERROR_CODE_OK) {
        ThrowInvalidParamError(env, "Get trigger info error. TriggerInfo must be a TriggerInfo.");
        return ret;
    }

    napi_ref ref = nullptr;
    napi_create_reference(env, argv[ARGC_TWO], 1, &ref);
    triggerObj->SetCallbackInfo(env, reinterpret_cast<NativeReference*>(ref));
    triggerObj->SetWantAgentInstance(std::make_shared<WantAgent>(pWantAgent->GetPendingWant()));

    return BUSINESS_ERROR_CODE_OK;
}

int32_t JsWantAgent::GetTriggerWant(napi_env env, napi_value param, std::shared_ptr<AAFwk::Want> &want)
{
    bool hasWant = false;
    napi_has_named_property(env, param, "want", &hasWant);
    if (hasWant) {
        napi_value jsWant = nullptr;
        napi_get_named_property(env, param, "want", &jsWant);
        want = std::make_shared<AAFwk::Want>();
        if (!UnwrapWant(env, jsWant, *want)) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "convert want error");
            return ERR_NOT_OK;
        }
    }

    return BUSINESS_ERROR_CODE_OK;
}

int32_t JsWantAgent::GetTriggerPermission(napi_env env, napi_value param, std::string &permission)
{
    bool hasPermission = false;
    napi_has_named_property(env, param, "permission", &hasPermission);
    if (hasPermission) {
        napi_value jsPermission = nullptr;
        napi_get_named_property(env, param, "permission", &jsPermission);
        if (!ConvertFromJsValue(env, jsPermission, permission)) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "convert permission error");
            return ERR_NOT_OK;
        }
    }

    return BUSINESS_ERROR_CODE_OK;
}

int32_t JsWantAgent::GetTriggerExtraInfo(napi_env env, napi_value param, std::shared_ptr<AAFwk::WantParams> &extraInfo)
{
    bool hasExtraInfo = false;
    napi_value jsExtraInfo = nullptr;
    napi_has_named_property(env, param, "extraInfos", &hasExtraInfo);
    if (hasExtraInfo) {
        napi_get_named_property(env, param, "extraInfos", &jsExtraInfo);
    } else {
        napi_has_named_property(env, param, "extraInfo", &hasExtraInfo);
        if (hasExtraInfo) {
            napi_get_named_property(env, param, "extraInfo", &jsExtraInfo);
        }
    }
    if (hasExtraInfo) {
        extraInfo = std::make_shared<AAFwk::WantParams>();
        if (!UnwrapWantParams(env, (jsExtraInfo),
            *extraInfo)) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "convert extraInfo error");
            return ERR_NOT_OK;
        }
    }

    return BUSINESS_ERROR_CODE_OK;
}

int32_t JsWantAgent::GetTriggerInfo(napi_env env, napi_value param, TriggerInfo &triggerInfo)
{
    TAG_LOGD(AAFwkTag::WANTAGENT, "called");
    if (!CheckTypeForNapiValue(env, param, napi_object)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "param type mismatch");
        return ERR_NOT_OK;
    }

    int32_t code = -1;
    napi_value jsCode = nullptr;
    napi_get_named_property(env, param, "code", &jsCode);
    if (!ConvertFromJsValue(env, jsCode, code)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "convert code error");
        return ERR_NOT_OK;
    }

    std::shared_ptr<AAFwk::Want> want = nullptr;
    if (GetTriggerWant(env, param, want) == ERR_NOT_OK) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "convert code error");
        return ERR_NOT_OK;
    }

    std::string permission = "";
    if (GetTriggerPermission(env, param, permission) == ERR_NOT_OK) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "convert code error");
        return ERR_NOT_OK;
    }

    std::shared_ptr<AAFwk::WantParams> extraInfo = nullptr;
    if (GetTriggerExtraInfo(env, param, extraInfo) == ERR_NOT_OK) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "convert code error");
        return ERR_NOT_OK;
    }

    std::shared_ptr<AAFwk::StartOptions> startOptions = nullptr;
    bool hasStartOptions = false;
    napi_value jsStartOptions = nullptr;
    napi_has_named_property(env, param, "startOptions", &hasStartOptions);
    if (hasStartOptions) {
        if (!CheckCallerIsSystemApp()) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "Non-system app");
            AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
            return ERR_NOT_OK;
        }
        startOptions = std::make_shared<AAFwk::StartOptions>();
        napi_get_named_property(env, param, "startOptions", &jsStartOptions);
        if (!UnwrapStartOptions(env, jsStartOptions, *startOptions)) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "convert startOptions error");
            return ERR_NOT_OK;
        }
    }
    TriggerInfo triggerInfoData(permission, extraInfo, want, startOptions, code);
    triggerInfo = triggerInfoData;
    return BUSINESS_ERROR_CODE_OK;
}

int32_t JsWantAgent::GetWantAgentParam(napi_env env, napi_callback_info info, WantAgentWantsParas &paras)
{
    TAG_LOGD(AAFwkTag::WANTAGENT, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (!CheckTypeForNapiValue(env, argv[0], napi_object)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "param type mismatch");
        return PARAMETER_ERROR;
    }

    napi_value jsWants = nullptr;
    napi_get_named_property(env, argv[0], "wants", &jsWants);

    bool isArray = false;
    napi_is_array(env, jsWants, &isArray);
    if (!isArray) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "wants is not array");
        return PARAMETER_ERROR;
    }

    uint32_t length = 0;
    napi_get_array_length(env, jsWants, &length);
    for (uint32_t i = 0; i < length; i++) {
        std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
        napi_value jsWant = nullptr;
        napi_get_element(env, jsWants, i, &jsWant);
        if (!UnwrapWant(env, jsWant, *want)) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "UnwrapWant failed");
            return PARAMETER_ERROR;
        }
        paras.wants.emplace_back(want);
    }

    bool hasActionType = false;
    napi_has_named_property(env, argv[0], "actionType", &hasActionType);
    if (hasActionType) {
        napi_value jsActionType = nullptr;
        napi_get_named_property(env, argv[0], "actionType", &jsActionType);
        if (!ConvertFromJsValue(env, jsActionType, paras.operationType)) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "Convert actionType failed");
            return PARAMETER_ERROR;
        }
    }

    bool hasOperationType = false;
    napi_has_named_property(env, argv[0], "operationType", &hasOperationType);
    if (!hasActionType && hasOperationType) {
        napi_value jsOperationType = nullptr;
        napi_get_named_property(env, argv[0], "operationType", &jsOperationType);
        if (!ConvertFromJsValue(env, jsOperationType, paras.operationType)) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "Convert operationType failed");
            return PARAMETER_ERROR;
        }
    }

    napi_value jsRequestCode = nullptr;
    napi_get_named_property(env, argv[0], "requestCode", &jsRequestCode);
    if (!ConvertFromJsValue(env, jsRequestCode, paras.requestCode)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Convert requestCode failed");
        return PARAMETER_ERROR;
    }

    bool hasUserId = false;
    napi_has_named_property(env, argv[0], "userId", &hasUserId);
    if (hasUserId) {
        napi_value jsUserId = nullptr;
        napi_get_named_property(env, argv[0], "userId", &jsUserId);
        if (!ConvertFromJsValue(env, jsUserId, paras.userId)) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "Convert userId failed");
            return PARAMETER_ERROR;
        }
    }
    bool hasActionFlags = false;
    napi_has_named_property(env, argv[0], "actionFlags", &hasActionFlags);
    if (hasActionFlags) {
        napi_value jsActionFlags = nullptr;
        napi_get_named_property(env, argv[0], "actionFlags", &jsActionFlags);
        bool jsActionFlagsIsArray = false;
        napi_is_array(env, jsActionFlags, &jsActionFlagsIsArray);
        if (!jsActionFlagsIsArray) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "actionFlags is not array");
            return PARAMETER_ERROR;
        }

        uint32_t jsActionFlagsLen = 0;
        napi_get_array_length(env, jsActionFlags, &jsActionFlagsLen);
        for (uint32_t i = 0; i < jsActionFlagsLen; i++) {
            napi_value jsActionFlag = nullptr;
            napi_get_element(env, jsActionFlags, i, &jsActionFlag);
            if (!CheckTypeForNapiValue(env, jsActionFlag, napi_number)) {
                TAG_LOGE(AAFwkTag::WANTAGENT, "ActionFlag type error");
                return PARAMETER_ERROR;
            }
            int32_t actionFlag = 0;
            if (!ConvertFromJsValue(env, jsActionFlag, actionFlag)) {
                TAG_LOGE(AAFwkTag::WANTAGENT, "Convert actionFlag failed");
                return PARAMETER_ERROR;
            }
            paras.wantAgentFlags.emplace_back(static_cast<WantAgentConstant::Flags>(actionFlag));
        }
    }

    bool hasWantAgentFlags = false;
    napi_has_named_property(env, argv[0], "wantAgentFlags", &hasWantAgentFlags);
    if (!hasActionFlags && hasWantAgentFlags) {
        napi_value jsWantAgentFlags = nullptr;
        napi_get_named_property(env, argv[0], "wantAgentFlags", &jsWantAgentFlags);
        bool jsWantAgentFlagsIsArray = false;
        napi_is_array(env, jsWantAgentFlags, &jsWantAgentFlagsIsArray);
        if (!jsWantAgentFlagsIsArray) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "wantAgentFlags is not array");
            return PARAMETER_ERROR;
        }

        uint32_t jsWantAgentFlagsLen = 0;
        napi_get_array_length(env, jsWantAgentFlags, &jsWantAgentFlagsLen);
        for (uint32_t i = 0; i < jsWantAgentFlagsLen; i++) {
            napi_value jsWantAgentFlag = nullptr;
            napi_get_element(env, jsWantAgentFlags, i, &jsWantAgentFlag);
            if (!CheckTypeForNapiValue(env, jsWantAgentFlag, napi_number)) {
                TAG_LOGE(AAFwkTag::WANTAGENT, "WantAgentFlag type failed");
                return PARAMETER_ERROR;
            }
            int32_t wantAgentFlag = 0;
            if (!ConvertFromJsValue(env, jsWantAgentFlag, wantAgentFlag)) {
                TAG_LOGE(AAFwkTag::WANTAGENT, "Convert WantAgentFlag failed");
                return PARAMETER_ERROR;
            }
            paras.wantAgentFlags.emplace_back(static_cast<WantAgentConstant::Flags>(wantAgentFlag));
        }
    }

    bool hasExtraInfo = false;
    napi_value jsExtraInfo = nullptr;
    napi_has_named_property(env, argv[0], "extraInfos", &hasExtraInfo);
    if (hasExtraInfo) {
        napi_get_named_property(env, argv[0], "extraInfos", &jsExtraInfo);
    } else {
        napi_has_named_property(env, argv[0], "extraInfo", &hasExtraInfo);
        if (hasExtraInfo) {
            napi_get_named_property(env, argv[0], "extraInfo", &jsExtraInfo);
        }
    }
    if (hasExtraInfo) {
        if (!CheckTypeForNapiValue(env, jsExtraInfo, napi_object)) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "ExtraInfo type error");
            return PARAMETER_ERROR;
        }
        if (!UnwrapWantParams(env, (jsExtraInfo),
            paras.extraInfo)) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "Convert extraInfo failed");
            return PARAMETER_ERROR;
        }
    }
    return BUSINESS_ERROR_CODE_OK;
}

napi_value JsWantAgent::OnGetWantAgent(napi_env env, napi_callback_info info)
{
    HandleEscape handleEscape(env);
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGC_TWO || argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    napi_value lastParam = (argc >= ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
    std::shared_ptr<WantAgentWantsParas> spParas = std::make_shared<WantAgentWantsParas>();
    int32_t ret = GetWantAgentParam(env, info, *spParas);
    if (ret != 0) {
        ThrowInvalidParamError(env, "Failed to get wantAgent parameter. Agent must be a WantAgent.");
        return handleEscape.Escape(RetErrMsg(env, lastParam, ret));
    }

    NapiAsyncTask::CompleteCallback complete = [weak = weak_from_this(), parasobj = spParas](napi_env env,
        NapiAsyncTask &task, int32_t status) {
        TAG_LOGD(AAFwkTag::WANTAGENT, "called");
        auto self = weak.lock();
        std::shared_ptr<AAFwk::WantParams> extraInfo = std::make_shared<AAFwk::WantParams>(parasobj->extraInfo);
        WantAgentInfo wantAgentInfo(parasobj->requestCode,
                                    static_cast<WantAgentConstant::OperationType>(parasobj->operationType),
                                    parasobj->wantAgentFlags,
                                    parasobj->wants,
                                    extraInfo);

        auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
        std::shared_ptr<WantAgent> wantAgent = nullptr;
        WantAgentHelper::GetWantAgent(context, wantAgentInfo, wantAgent);
        WantAgent* pWantAgent = nullptr;
        napi_value jsWantAgent = nullptr;
        if (wantAgent) {
            pWantAgent = new WantAgent(wantAgent->GetPendingWant());
        }
        jsWantAgent = OHOS::AppExecFwk::WrapWantAgent(env, pWantAgent, nullptr);
        if (jsWantAgent == nullptr && pWantAgent != nullptr) {
            delete pWantAgent;
            pWantAgent = nullptr;
        }

        task.Resolve(env, jsWantAgent);
    };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsWantAgent::OnGetWantAgent",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JsWantAgent::OnNapiGetWant(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "on get want");
    HandleEscape handleEscape(env);
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGC_TWO || argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    WantAgent* pWantAgent = nullptr;
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(IPCSkeleton::GetSelfTokenID())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Non-system app forbidden to call");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }

    if (!CheckTypeForNapiValue(env, argv[0], napi_object)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Type not Object");
        ThrowInvalidParamError(env, "Parameter error! Agent must be a WantAgent!");
        return CreateJsUndefined(env);
    }

    UnwrapWantAgent(env, argv[0], reinterpret_cast<void **>(&pWantAgent));
    if (pWantAgent == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Parse pWantAgent failed");
        ThrowInvalidParamError(env, "Parse wantAgent failed! Agent must be a WantAgent!");
        return CreateJsUndefined(env);
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    std::shared_ptr<Want> want = std::make_shared<Want>();
    auto retCode = std::make_shared<int32_t>(NO_ERROR);
    NapiAsyncTask::ExecuteCallback execute = [wantAgent, retCode, want] () {
        TAG_LOGI(AAFwkTag::WANTAGENT, "start");
        std::shared_ptr<Want> retWant = std::make_shared<Want>();
        *retCode = WantAgentHelper::GetWant(wantAgent, retWant);
        *want = *retWant;
    };

    NapiAsyncTask::CompleteCallback complete = [retCode, want](napi_env env, NapiAsyncTask &task, int32_t status) {
        TAG_LOGD(AAFwkTag::WANTAGENT, "start");
        HandleScope handleScope(env);
        if (*retCode != NO_ERROR) {
            task.Reject(env, CreateJsError(env, *retCode, AbilityRuntimeErrorUtil::GetErrMessage(*retCode)));
            return;
        }
        task.ResolveWithNoError(env, CreateJsWant(env, *(want)));
    };
    napi_value lastParam = (argc >= ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsWantAgent::OnNapiGetWant",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JsWantAgent::OnNapiTrigger(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "on trigger");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_THREE) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::shared_ptr<WantAgent> wantAgent = nullptr;
    TriggerInfo triggerInfo;
    auto triggerObj = std::make_shared<TriggerCompleteCallBack>();
    int32_t errCode = UnWrapTriggerInfoParam(env, info, wantAgent, triggerInfo, triggerObj);
    if (errCode != NO_ERROR) {
        ThrowInvalidParamError(env, "Parameter error!");
        return CreateJsUndefined(env);
    }

    // Public api trigger does not support localWantAgent, switch to triggerAsync.
    if (wantAgent->IsLocal()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Tigger does not support localWantAgent");
        ThrowInvalidParamError(env, "Agent can not be local.");
        return CreateJsUndefined(env);
    }

    auto execute = [wantAgent, triggerObj, triggerInfo] () {
        TAG_LOGD(AAFwkTag::WANTAGENT, "called");
        sptr<CompletedDispatcher> completedData;
        WantAgentHelper::TriggerWantAgent(wantAgent, triggerObj, triggerInfo, completedData, nullptr);
    };
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsWantAgent::OnNapiTrigger",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), nullptr, &result));

    return CreateJsNull(env);
}

napi_value JsWantAgent::OnNapiTriggerAsync(napi_env env, napi_callback_info info)
{
    HandleEscape handleEscape(env);
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_THREE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    bool stageMode = false;
    napi_status status = OHOS::AbilityRuntime::IsStageContext(env, argv[ARGC_TWO], stageMode);
    if (status != napi_ok || !stageMode) {
        ThrowInvalidParamError(env, "Parse param context failed, must be a context of stageMode.");
        return CreateJsUndefined(env);
    }

    auto context = OHOS::AbilityRuntime::GetStageModeContext(env, argv[ARGC_TWO]);
    if (context == nullptr) {
        ThrowInvalidParamError(env, "Parse param context failed, must not be nullptr.");
        return CreateJsUndefined(env);
    }
    
    auto inputContextPtr = ConvertToContext(context);
    if (inputContextPtr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "the context is not ability context");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_NOT_ABILITY_CONTEXT);
        return CreateJsUndefined(env);
    }

    std::shared_ptr<WantAgent> wantAgent = nullptr;
    TriggerInfo triggerInfo;
    auto triggerObj = std::make_shared<TriggerCompleteCallBack>();
    int32_t errCode = UnWrapTriggerInfoParam(env, info, wantAgent, triggerInfo, triggerObj);
    if (errCode != NO_ERROR) {
        ThrowInvalidParamError(env, "Parameter error!");
        return CreateJsUndefined(env);
    }
    auto retCode = std::make_shared<int32_t>(NO_ERROR);
    if (!CheckCallerIsSystemApp()) {
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }
    std::shared_ptr<CompletedDispatcher> data = std::make_shared<CompletedDispatcher>();
    auto execute = [wantAgent, triggerObj, triggerInfo, inputContextPtr, data, retCode] () {
        sptr<CompletedDispatcher> completedData;
        *retCode = WantAgentHelper::TriggerWantAgent(wantAgent, triggerObj, triggerInfo, completedData,
            inputContextPtr->GetToken());
        *data = *reinterpret_cast<CompletedDispatcher*>(completedData.GetRefPtr());
    };
    NapiAsyncTask::CompleteCallback complete = [retCode, data](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (*retCode == NO_ERROR) {
            task.ResolveWithNoError(env, CreateJsCompletedData(env, *data));
        } else {
            task.Reject(env, CreateJsError(env, *retCode, AbilityRuntimeErrorUtil::GetErrMessage(*retCode)));
        }
    };
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsWantAgent::OnNapiTriggerAsync",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));

    return handleEscape.Escape(result);
}

std::shared_ptr<AbilityRuntime::Context> JsWantAgent::ConvertToContext(std::shared_ptr<AbilityRuntime::Context> context)
{
    auto uiAbilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
    auto extensionContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::ExtensionContext>(context);
    if (uiAbilityContext == nullptr && extensionContext == nullptr) {
        return nullptr;
    }
    
    return OHOS::AbilityRuntime::Context::ConvertTo<AbilityRuntime::Context>(context);
}

napi_value JsWantAgent::CreateJsCompletedData(napi_env env, const CompletedDispatcher &data)
{
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    WantAgent *pWantAgent = new (std::nothrow) WantAgent(data.GetPendingWant());
    if (pWantAgent == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null pWantAgent");
        return handleEscape.Escape(objValue);
    }

    napi_value jsWantAgent = OHOS::AppExecFwk::WrapWantAgent(env, pWantAgent, nullptr);
    if (jsWantAgent == nullptr) {
        delete pWantAgent;
        pWantAgent = nullptr;
    }
    napi_set_named_property(env, objValue, "info", jsWantAgent);
    napi_set_named_property(env, objValue, "want", CreateJsWant(env, data.GetWant()));
    napi_set_named_property(env, objValue, "finalCode", CreateJsValue(env, data.GetResultCode()));
    napi_set_named_property(env, objValue, "finalData", CreateJsValue(env, data.GetResultData()));
    napi_set_named_property(env, objValue, "extraInfo", CreateJsWantParams(env, data.GetResultExtras()));
    return handleEscape.Escape(objValue);
}

napi_value JsWantAgent::OnNapiGetWantAgent(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "on get wantAgent");
    HandleEscape handleEscape(env);
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGC_TWO || argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::shared_ptr<WantAgentWantsParas> spParas = std::make_shared<WantAgentWantsParas>();
    int32_t ret = GetWantAgentParam(env, info, *spParas);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Failed to get wantAgent param");
        ThrowInvalidParamError(env, "Parameter error! Info must be a WantAgentInfo.");
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete;
    SetOnNapiGetWantAgentCallback(spParas, complete);

    napi_value lastParam = (argc >= ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsWantAgent::OnNapiGetWantAgent",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return handleEscape.Escape(result);
}

void JsWantAgent::SetOnNapiGetWantAgentCallback(std::shared_ptr<WantAgentWantsParas> spParas,
    AbilityRuntime::NapiAsyncTask::CompleteCallback &complete)
{
    complete = [weak = weak_from_this(), parasobj = spParas](napi_env env,
        NapiAsyncTask &task, int32_t status) {
        TAG_LOGD(AAFwkTag::WANTAGENT, "called");
        HandleScope handleScope(env);
        auto self = weak.lock();
        std::shared_ptr<AAFwk::WantParams> extraInfo = std::make_shared<AAFwk::WantParams>(parasobj->extraInfo);
        WantAgentInfo wantAgentInfo(parasobj->requestCode,
                                    static_cast<WantAgentConstant::OperationType>(parasobj->operationType),
                                    parasobj->wantAgentFlags,
                                    parasobj->wants,
                                    extraInfo,
                                    parasobj->userId);

        auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
        std::shared_ptr<WantAgent> wantAgent = nullptr;
        ErrCode result = WantAgentHelper::GetWantAgent(context, wantAgentInfo, wantAgent);
        if (result != NO_ERROR) {
            task.Reject(env, CreateJsError(env, result, AbilityRuntimeErrorUtil::GetErrMessage(result)));
            return;
        }
        if (wantAgent == nullptr) {
            result = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
            task.Reject(env, CreateJsError(env, result, AbilityRuntimeErrorUtil::GetErrMessage(result)));
            return;
        }
        WantAgent *pWantAgent = new (std::nothrow) WantAgent(wantAgent->GetPendingWant());
        if (pWantAgent == nullptr) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "null pWantAgent");
            result = ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
            task.Reject(env, CreateJsError(env, result, AbilityRuntimeErrorUtil::GetErrMessage(result)));
        } else {
            napi_value jsWantAgent = OHOS::AppExecFwk::WrapWantAgent(env, pWantAgent, nullptr);
            if (jsWantAgent == nullptr) {
                delete pWantAgent;
                pWantAgent = nullptr;
            }
            task.ResolveWithNoError(env, jsWantAgent);
        }
    };
}

napi_value JsWantAgent::OnNapiCreateLocalWantAgent(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "on create localWantAgent");
    HandleEscape handleEscape(env);
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Non-system app");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }

    std::shared_ptr<WantAgentWantsParas> spParas = std::make_shared<WantAgentWantsParas>();
    int32_t ret = GetWantAgentParam(env, info, *spParas);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Failed to get wantAgent param");
        ThrowInvalidParamError(env, "Parameter error! Info must be a WantAgentInfo.");
        return CreateJsUndefined(env);
    }

    LocalWantAgentInfo localWantAgentInfo(spParas->requestCode,
        static_cast<WantAgentConstant::OperationType>(spParas->operationType),
        spParas->wants);
    const auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    std::shared_ptr<WantAgent> wantAgent = nullptr;
    ErrCode result = WantAgentHelper::CreateLocalWantAgent(context, localWantAgentInfo, wantAgent);
    if (result != ERR_OK || wantAgent == nullptr) {
        ThrowInvalidParamError(env, "Parameter error! CreateLocalWantAgent failed.");
        return CreateJsUndefined(env);
    }

    WantAgent *pWantAgent = new (std::nothrow) WantAgent(wantAgent->GetLocalPendingWant());
    if (pWantAgent == nullptr) {
        ThrowInvalidParamError(env, "Parameter error! New wantAgent failed.");
        return CreateJsUndefined(env);
    }
    napi_value jsWantAgent = OHOS::AppExecFwk::WrapWantAgent(env, pWantAgent, nullptr);
    if (jsWantAgent == nullptr) {
        delete pWantAgent;
        pWantAgent = nullptr;
        return CreateJsUndefined(env);
    }
    return handleEscape.Escape(jsWantAgent);
}

napi_value JsWantAgent::OnNapiIsLocalWantAgent(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "on is localWantAgent");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Non-system app");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }

    if (!CheckTypeForNapiValue(env, argv[0], napi_object)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Type not Object");
        ThrowInvalidParamError(env, "Parameter error! Agent must be a WantAgent.");
        return CreateJsUndefined(env);
    }

    WantAgent* pWantAgent = nullptr;
    UnwrapWantAgent(env, argv[0], reinterpret_cast<void **>(&pWantAgent));
    if (pWantAgent == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Parse pWantAgent failed");
        ThrowInvalidParamError(env, "Parse wantAgent failed! Agent must be a WantAgent.");
        return CreateJsUndefined(env);
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    napi_value result = nullptr;
    bool isLocal = wantAgent->IsLocal();
    napi_get_boolean(env, isLocal, &result);
    return result;
}

napi_value JsWantAgent::OnNapiGetOperationType(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "called");
    HandleEscape handleEscape(env);
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    WantAgent* pWantAgent = nullptr;
    if (argc > ARGC_TWO || argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    if (!CheckTypeForNapiValue(env, argv[0], napi_object)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Type not Object");
        ThrowInvalidParamError(env, "Parameter error! Agent must be a WantAgent.");
        return CreateJsUndefined(env);
    }

    UnwrapWantAgent(env, argv[0], reinterpret_cast<void **>(&pWantAgent));

    if (pWantAgent == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Parse pWantAgent failed");
        ThrowInvalidParamError(env, "Parse wantAgent failed! Agent must be a WantAgent.");
        return CreateJsUndefined(env);
    }

    std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(*pWantAgent);
    auto operType = std::make_shared<int32_t>(NO_ERROR);
    auto retCode = std::make_shared<int32_t>(NO_ERROR);
    NapiAsyncTask::ExecuteCallback execute = [wantAgent, retCode, operType] () {
        TAG_LOGI(AAFwkTag::WANTAGENT, "start");
        *retCode = WantAgentHelper::GetType(wantAgent, *operType);
    };
    NapiAsyncTask::CompleteCallback complete = [retCode, operType](napi_env env, NapiAsyncTask &task, int32_t status) {
        TAG_LOGD(AAFwkTag::WANTAGENT, "called");
        HandleScope handleScope(env);
        if (*retCode != NO_ERROR) {
            task.Reject(env, CreateJsError(env, *retCode, AbilityRuntimeErrorUtil::GetErrMessage(*retCode)));
        } else {
            task.ResolveWithNoError(env, CreateJsValue(env, *operType));
        }
    };

    napi_value lastParam = (argc >= ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsWantAgent::OnNapiGetOperationType",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JsWantAgent::OnNapiSetWantAgentMultithreading(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "set want agent multithreading");
    bool isMultithreadingSupported = false;
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGC_ONE) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid argc");
        ThrowInvalidNumParametersError(env);
        return CreateJsUndefined(env);
    }

    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Non-system app forbidden to call");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }

    if (!CheckTypeForNapiValue(env, argv[0], napi_boolean)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Type not boolean");
        ThrowInvalidParamError(env, "Parameter error! isMultithreadingSupported must be a boolean.");
        return CreateJsUndefined(env);
    }

    if (!ConvertFromJsValue(env, argv[0], isMultithreadingSupported)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Convert isMultithreadingSupported failed");
        ThrowInvalidParamError(env,
            "Convert isMultithreadingSupported failed! isMultithreadingSupported must be a boolean.");
        return CreateJsUndefined(env);
    }

    WantAgent::SetIsMultithreadingSupported(isMultithreadingSupported);
    return CreateJsNull(env);
}

napi_value WantAgentFlagsInit(napi_env env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null env");
        return nullptr;
    }

    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    napi_set_named_property(env, objValue, "ONE_TIME_FLAG",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_ZERO)));
    napi_set_named_property(env, objValue, "NO_BUILD_FLAG",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_ONE)));
    napi_set_named_property(env, objValue, "CANCEL_PRESENT_FLAG",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_TWO)));
    napi_set_named_property(env, objValue, "UPDATE_PRESENT_FLAG",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_THREE)));
    napi_set_named_property(env, objValue, "CONSTANT_FLAG",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_FOUR)));
    napi_set_named_property(env, objValue, "REPLACE_ELEMENT",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_FIVE)));
    napi_set_named_property(env, objValue, "REPLACE_ACTION",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_SIX)));
    napi_set_named_property(env, objValue, "REPLACE_URI",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_SEVEN)));
    napi_set_named_property(env, objValue, "REPLACE_ENTITIES",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_EIGHT)));
    napi_set_named_property(env, objValue, "REPLACE_BUNDLE",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_NINE)));
    napi_set_named_property(env, objValue, "ALLOW_CANCEL_FLAG",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_TEN)));

    return handleEscape.Escape(objValue);
}

napi_value WantAgentOperationTypeInit(napi_env env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null env");
        return nullptr;
    }

    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    napi_set_named_property(env, objValue, "UNKNOWN_TYPE",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_ZERO)));
    napi_set_named_property(env, objValue, "START_ABILITY",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_ONE)));
    napi_set_named_property(env, objValue, "START_ABILITIES",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_TWO)));
    napi_set_named_property(env, objValue, "START_SERVICE",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_THREE)));
    napi_set_named_property(env, objValue, "SEND_COMMON_EVENT",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_FOUR)));
    napi_set_named_property(env, objValue, "START_FOREGROUND_SERVICE",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_FIVE)));
    napi_set_named_property(env, objValue, "START_SERVICE_EXTENSION",
        CreateJsValue(env, static_cast<int32_t>(NUMBER_OF_PARAMETERS_SIX)));

    return handleEscape.Escape(objValue);
}

napi_value JsWantAgentInit(napi_env env, napi_value exportObj)
{
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null env or exportObj");
        return nullptr;
    }

    HandleScope handleScope(env);
    std::unique_ptr<JsWantAgent> jsWantAgent = std::make_unique<JsWantAgent>();
    napi_wrap(env, exportObj, jsWantAgent.release(), JsWantAgent::Finalizer, nullptr, nullptr);

    napi_set_named_property(env, exportObj, "WantAgentFlags", WantAgentFlagsInit(env));
    napi_set_named_property(env, exportObj, "OperationType", WantAgentOperationTypeInit(env));

    const char* moduleName = "JsWantAgent";
    BindNativeFunction(env, exportObj, "equal", moduleName, JsWantAgent::Equal);
    BindNativeFunction(env, exportObj, "getWant", moduleName, JsWantAgent::GetWant);
    BindNativeFunction(env, exportObj, "getOperationType", moduleName, JsWantAgent::GetOperationType);
    BindNativeFunction(env, exportObj, "getBundleName", moduleName, JsWantAgent::GetBundleName);
    BindNativeFunction(env, exportObj, "getUid", moduleName, JsWantAgent::GetUid);
    BindNativeFunction(env, exportObj, "cancel", moduleName, JsWantAgent::Cancel);
    BindNativeFunction(env, exportObj, "trigger", moduleName, JsWantAgent::Trigger);
    BindNativeFunction(env, exportObj, "getWantAgent", moduleName, JsWantAgent::GetWantAgent);
    return CreateJsUndefined(env);
}

napi_value NapiGetNull(napi_env env)
{
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}
}  // namespace OHOS
