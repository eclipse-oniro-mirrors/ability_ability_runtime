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

#include "js_runtime_utils.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
#define ARGS_MAX_COUNT 10
namespace {
std::unique_ptr<NapiAsyncTask> CreateAsyncTaskWithLastParam(napi_env env, napi_value lastParam,
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> &&execute,
    std::unique_ptr<NapiAsyncTask::CompleteCallback> &&complete, napi_value* result)
{
    napi_valuetype type = napi_undefined;
    napi_typeof(env, lastParam, &type);
    if (lastParam == nullptr || type != napi_function) {
        napi_deferred nativeDeferred = nullptr;
        napi_create_promise(env, &nativeDeferred, result);
        return std::make_unique<NapiAsyncTask>(nativeDeferred, std::move(execute), std::move(complete));
    } else {
        napi_get_undefined(env, result);
        napi_ref callbackRef = nullptr;
        napi_create_reference(env, lastParam, 1, &callbackRef);
        return std::make_unique<NapiAsyncTask>(callbackRef, std::move(execute), std::move(complete));
    }
}
} // namespace

// Help Functions
napi_value CreateJsError(napi_env env, int32_t errCode, const std::string &message)
{
    napi_value result = nullptr;
    napi_create_error(env, CreateJsValue(env, errCode), CreateJsValue(env, message), &result);
    return result;
}

void BindNativeFunction(napi_env env, napi_value object, const char *name,
    const char *moduleName, napi_callback func)
{
    std::string fullName(moduleName);
    fullName += ".";
    fullName += name;
    napi_value result = nullptr;
    napi_create_function(env, fullName.c_str(), fullName.length(), func, nullptr, &result);
    napi_set_named_property(env, object, name, result);
}

void BindNativeProperty(napi_env env, napi_value object, const char *name, napi_callback getter)
{
    napi_property_descriptor properties[1];
    properties[0].utf8name = name;
    properties[0].name = nullptr;
    properties[0].method = nullptr;
    properties[0].getter = getter;
    properties[0].setter = nullptr;
    properties[0].value = nullptr;
    properties[0].attributes = napi_default;
    properties[0].data = nullptr;
    napi_define_properties(env, object, 1, properties);
}

void *GetNativePointerFromCallbackInfo(const napi_env env, napi_callback_info info, const char *name)
{
    size_t argcAsync = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL_NO_THROW(napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr), nullptr);
    if (name != nullptr) {
        napi_get_named_property(env, thisVar, name, &thisVar);
    }
    void* result = nullptr;
    NAPI_CALL_NO_THROW(napi_unwrap(env, thisVar, &result), nullptr);
    return result;
}

void* GetNapiCallbackInfoAndThis(napi_env env, napi_callback_info info, NapiCallbackInfo& napiInfo, const char* name)
{
    NAPI_CALL_NO_THROW(napi_get_cb_info(
        env, info, &napiInfo.argc, napiInfo.argv, &napiInfo.thisVar, nullptr), nullptr);
    napi_value value = napiInfo.thisVar;
    if (name != nullptr) {
        napi_get_named_property(env, value, name, &value);
    }
    void* result = nullptr;
    NAPI_CALL_NO_THROW(napi_unwrap(env, value, &result), nullptr);
    return result;
}

void SetNamedNativePointer(napi_env env, napi_value object, const char *name, void *ptr, napi_finalize func)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    napi_wrap(env, objValue, ptr, func, nullptr, nullptr);
    napi_set_named_property(env, object, name, objValue);
}

void *GetNamedNativePointer(napi_env env, napi_value object, const char *name)
{
    napi_value proValue = nullptr;
    napi_get_named_property(env, object, name, &proValue);
    void* result = nullptr;
    napi_unwrap(env, proValue, &result);
    return result;
}

bool CheckTypeForNapiValue(napi_env env, napi_value param, napi_valuetype expectType)
{
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, param, &valueType) != napi_ok) {
        return false;
    }
    return valueType == expectType;
}

// Async Task
NapiAsyncTask::NapiAsyncTask(napi_deferred deferred, std::unique_ptr<NapiAsyncTask::ExecuteCallback> &&execute,
    std::unique_ptr<NapiAsyncTask::CompleteCallback> &&complete)
    : deferred_(deferred), execute_(std::move(execute)), complete_(std::move(complete))
{}

NapiAsyncTask::NapiAsyncTask(napi_ref callbackRef, std::unique_ptr<NapiAsyncTask::ExecuteCallback> &&execute,
    std::unique_ptr<NapiAsyncTask::CompleteCallback> &&complete)
    : callbackRef_(callbackRef), execute_(std::move(execute)), complete_(std::move(complete))
{}

NapiAsyncTask::~NapiAsyncTask() = default;

void NapiAsyncTask::Schedule(const std::string &name, napi_env env, std::unique_ptr<NapiAsyncTask> &&task)
{
    if (task && task->Start(name, env)) {
        task.release();
    }
}

void NapiAsyncTask::Resolve(napi_env env, napi_value value)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "called");
    if (deferred_) {
        napi_resolve_deferred(env, deferred_, value);
        deferred_ = nullptr;
    }
    if (callbackRef_) {
        napi_value argv[] = {
            CreateJsError(env, 0),
            value,
        };
        napi_value func = nullptr;
        napi_get_reference_value(env, callbackRef_, &func);
        napi_call_function(env, CreateJsUndefined(env), func, ArraySize(argv), argv, nullptr);
        napi_delete_reference(env, callbackRef_);
        callbackRef_ = nullptr;
    }
}

void NapiAsyncTask::ResolveWithNoError(napi_env env, napi_value value)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "called");
    if (deferred_) {
        napi_resolve_deferred(env, deferred_, value);
        deferred_ = nullptr;
    }
    if (callbackRef_) {
        napi_value argv[] = {
            CreateJsNull(env),
            value,
        };
        napi_value func = nullptr;
        napi_get_reference_value(env, callbackRef_, &func);
        napi_call_function(env, CreateJsUndefined(env), func, ArraySize(argv), argv, nullptr);
        napi_delete_reference(env, callbackRef_);
        callbackRef_ = nullptr;
    }
}

void NapiAsyncTask::Reject(napi_env env, napi_value error)
{
    if (deferred_) {
        napi_reject_deferred(env, deferred_, error);
        deferred_ = nullptr;
    }
    if (callbackRef_) {
        napi_value argv[] = {
            error,
            CreateJsUndefined(env),
        };
        napi_value func = nullptr;
        napi_get_reference_value(env, callbackRef_, &func);
        napi_call_function(env, CreateJsUndefined(env), func, ArraySize(argv), argv, nullptr);
        napi_delete_reference(env, callbackRef_);
        callbackRef_ = nullptr;
    }
}

void NapiAsyncTask::ResolveWithCustomize(napi_env env, napi_value error, napi_value value)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "called");
    if (deferred_) {
        napi_resolve_deferred(env, deferred_, value);
        deferred_ = nullptr;
    }
    if (callbackRef_) {
        napi_value argv[] = {
            error,
            value,
        };
        napi_value func = nullptr;
        napi_get_reference_value(env, callbackRef_, &func);
        napi_call_function(env, CreateJsUndefined(env), func, ArraySize(argv), argv, nullptr);
        napi_delete_reference(env, callbackRef_);
        callbackRef_ = nullptr;
    }
}

void NapiAsyncTask::RejectWithCustomize(napi_env env, napi_value error, napi_value value)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "called");
    if (deferred_) {
        napi_reject_deferred(env, deferred_, error);
        deferred_ = nullptr;
    }
    if (callbackRef_) {
        napi_value argv[] = {
            error,
            value,
        };
        napi_value func = nullptr;
        napi_get_reference_value(env, callbackRef_, &func);
        napi_call_function(env, CreateJsUndefined(env), func, ArraySize(argv), argv, nullptr);
        napi_delete_reference(env, callbackRef_);
        callbackRef_ = nullptr;
    }
}

void NapiAsyncTask::Execute(napi_env env, void *data)
{
    if (env == nullptr || data == nullptr) {
        return;
    }
    auto me = static_cast<NapiAsyncTask*>(data);
    if (me->execute_ && *(me->execute_)) {
        (*me->execute_)();
    }
}

void NapiAsyncTask::Complete(napi_env env, napi_status status, void *data)
{
    if (data == nullptr) {
        return;
    }
    std::unique_ptr<NapiAsyncTask> me(static_cast<NapiAsyncTask*>(data));
    if (me->complete_ && *(me->complete_)) {
        (*me->complete_)(env, *me, static_cast<int32_t>(status));
    }
}

bool NapiAsyncTask::Start(const std::string &name, napi_env env)
{
    if (work_) {
        napi_delete_async_work(env, work_);
        work_ = nullptr;
    }
    if (env == nullptr) {
        return false;
    }
    NativeEngine* engine = reinterpret_cast<NativeEngine*>(env);
    work_ = reinterpret_cast<napi_async_work>(engine->CreateAsyncWork(name,
        reinterpret_cast<NativeAsyncExecuteCallback>(Execute),
        reinterpret_cast<NativeAsyncCompleteCallback>(Complete), this));
    napi_queue_async_work(env, work_);
    return true;
}

std::unique_ptr<NapiAsyncTask> CreateAsyncTaskWithLastParam(napi_env env, napi_value lastParam,
    NapiAsyncTask::ExecuteCallback &&execute, NapiAsyncTask::CompleteCallback &&complete, napi_value *result)
{
    return CreateAsyncTaskWithLastParam(env, lastParam,
        std::make_unique<NapiAsyncTask::ExecuteCallback>(std::move(execute)),
        std::make_unique<NapiAsyncTask::CompleteCallback>(std::move(complete)), result);
}

std::unique_ptr<NapiAsyncTask> CreateAsyncTaskWithLastParam(napi_env env, napi_value lastParam,
    NapiAsyncTask::ExecuteCallback &&execute, std::nullptr_t, napi_value *result)
{
    return CreateAsyncTaskWithLastParam(
        env, lastParam, std::make_unique<NapiAsyncTask::ExecuteCallback>(std::move(execute)), nullptr, result);
}

std::unique_ptr<NapiAsyncTask> CreateAsyncTaskWithLastParam(napi_env env, napi_value lastParam,
    std::nullptr_t, NapiAsyncTask::CompleteCallback &&complete, napi_value *result)
{
    return CreateAsyncTaskWithLastParam(
        env, lastParam, nullptr, std::make_unique<NapiAsyncTask::CompleteCallback>(std::move(complete)), result);
}

std::unique_ptr<NapiAsyncTask> CreateAsyncTaskWithLastParam(napi_env env, napi_value lastParam,
    std::nullptr_t, std::nullptr_t, napi_value *result)
{
    return CreateAsyncTaskWithLastParam(env, lastParam, std::unique_ptr<NapiAsyncTask::ExecuteCallback>(),
        std::unique_ptr<NapiAsyncTask::CompleteCallback>(), result);
}

bool NapiAsyncTask::SendNapiEvent(napi_env env, napi_event_priority eventPriority)
{
    if (napi_send_event(
        env,
        [env, this]() {
            napi_status status = napi_ok;
            Complete(env, status, this);
        },
        eventPriority) != napi_ok) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "SendNapiEvent failed");
        return false;
    }
    return true;
}

void NapiAsyncTask::ScheduleHighQos(const std::string &name, napi_env env, std::unique_ptr<NapiAsyncTask> &&task)
{
    if (task && task->StartHighQos(name, env)) {
        task.release();
    }
}

bool NapiAsyncTask::StartHighQos(const std::string &name, napi_env env)
{
    if (work_) {
        napi_delete_async_work(env, work_);
        work_ = nullptr;
    }
    if (env == nullptr) {
        return false;
    }
    if (execute_ == nullptr) {
        return SendNapiEvent(env, napi_eprio_immediate);
    }
    NativeEngine* engine = reinterpret_cast<NativeEngine*>(env);
    work_ = reinterpret_cast<napi_async_work>(engine->CreateAsyncWork(name,
        reinterpret_cast<NativeAsyncExecuteCallback>(Execute),
        reinterpret_cast<NativeAsyncCompleteCallback>(Complete), this));
    napi_queue_async_work_with_qos(env, work_, napi_qos_user_initiated);
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS