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

#include "js_ability_delegator.h"

#include <mutex>
#include "ability_delegator_registry.h"
#include "hilog_tag_wrapper.h"
#include "js_ability_delegator_utils.h"
#include "js_context_utils.h"
#include "js_error_utils.h"
#include "napi_common_want.h"
#include "napi_common_util.h"
#include "napi_remote_object.h"
#include "shell_cmd_result.h"

namespace OHOS {
namespace AbilityDelegatorJs {
struct AbilityObjectBox {
    std::weak_ptr<NativeReference> object_;
};
struct AbilityStageObjBox {
    std::weak_ptr<NativeReference> object_;
};

struct ShellCmdResultBox {
    std::unique_ptr<ShellCmdResult> shellCmdResult_;
};

constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;
constexpr size_t API20 = 20;
constexpr size_t API_VERSION_MOD = 100;

using namespace OHOS::AbilityRuntime;
std::map<std::shared_ptr<NativeReference>, std::shared_ptr<AbilityMonitor>> g_monitorRecord;
std::map<std::shared_ptr<NativeReference>, std::shared_ptr<AbilityStageMonitor>> g_stageMonitorRecord;
std::map<std::weak_ptr<NativeReference>, sptr<IRemoteObject>, std::owner_less<>> g_abilityRecord;
std::mutex g_mutexAbilityRecord;
std::mutex g_mtxStageMonitorRecord;
std::map<std::shared_ptr<NativeReference>, std::shared_ptr<InteropAbilityMonitor>> g_interopMonitorRecord;

enum ERROR_CODE {
    INCORRECT_PARAMETERS    = 401,
};

std::unordered_map<int32_t, std::string> errorMap = {
    {INCORRECT_PARAMETERS,  "Incorrect parameters."},
};

#ifdef ENABLE_ERRCODE
constexpr int COMMON_FAILED = 16000100;
#else
constexpr int COMMON_FAILED = -1;
#endif

napi_value ThrowJsError(napi_env env, int32_t errCode, std::string errMsg)
{
#ifdef ENABLE_ERRCODE
    HandleScope handleScope(env);
    napi_value error = CreateJsError(env, errCode, errMsg);
    napi_throw(env, error);
#endif
    return CreateJsUndefined(env);
}

void ResolveWithNoError(napi_env env, NapiAsyncTask &task, napi_value value = nullptr)
{
#ifdef ENABLE_ERRCODE
    if (value == nullptr) {
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    } else {
        task.ResolveWithNoError(env, value);
    }
#else
    if (value == nullptr) {
        task.Resolve(env, CreateJsNull(env));
    } else {
        task.Resolve(env, value);
    }
#endif
}

napi_value AttachAppContext(napi_env env, void *value, void *)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");
    HandleEscape handleEscape(env);
    if (value == nullptr) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "null value");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<AbilityRuntime::Context>*>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "null context");
        return nullptr;
    }

    napi_value object = CreateJsBaseContext(env, ptr, true);
    napi_coerce_to_native_binding_object(env, object, DetachCallbackFunc, AttachAppContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::Context>(ptr);
    napi_status status = napi_wrap(env, object, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGI(AAFwkTag::DELEGATOR, "finalizer called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Context> *>(data);
        }, nullptr, nullptr);
    if (status != napi_ok && workContext != nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "napi_wrap Failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }
    return handleEscape.Escape(object);
}

JSAbilityDelegator::JSAbilityDelegator()
{
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        auto clearFunc = [](const std::shared_ptr<BaseDelegatorAbilityProperty> &baseProperty) {
            auto property = std::static_pointer_cast<AppExecFwk::ADelegatorAbilityProperty>(baseProperty);
            TAG_LOGI(AAFwkTag::DELEGATOR, "clearFunc called");
            if (!property) {
                TAG_LOGE(AAFwkTag::DELEGATOR, "null property");
                return;
            }

            std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
            for (auto it = g_abilityRecord.begin(); it != g_abilityRecord.end();) {
                if (it->second == property->token_) {
                    it = g_abilityRecord.erase(it);
                    continue;
                }
                ++it;
            }
        };

        delegator->RegisterClearFunc(clearFunc);
    }
}

void JSAbilityDelegator::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");
    std::unique_ptr<JSAbilityDelegator>(static_cast<JSAbilityDelegator *>(data));
}

napi_value JSAbilityDelegator::AddAbilityMonitor(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnAddAbilityMonitor);
}

napi_value JSAbilityDelegator::AddAbilityMonitorSync(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnAddAbilityMonitorSync);
}

napi_value JSAbilityDelegator::RemoveAbilityMonitor(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnRemoveAbilityMonitor);
}

napi_value JSAbilityDelegator::RemoveAbilityMonitorSync(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnRemoveAbilityMonitorSync);
}

napi_value JSAbilityDelegator::WaitAbilityMonitor(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnWaitAbilityMonitor);
}

napi_value JSAbilityDelegator::AddAbilityStageMonitor(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnAddAbilityStageMonitor);
}

napi_value JSAbilityDelegator::AddAbilityStageMonitorSync(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnAddAbilityStageMonitorSync);
}

napi_value JSAbilityDelegator::RemoveAbilityStageMonitor(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnRemoveAbilityStageMonitor);
}

napi_value JSAbilityDelegator::RemoveAbilityStageMonitorSync(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnRemoveAbilityStageMonitorSync);
}

napi_value JSAbilityDelegator::WaitAbilityStageMonitor(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnWaitAbilityStageMonitor);
}

napi_value JSAbilityDelegator::GetAppContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnGetAppContext);
}

napi_value JSAbilityDelegator::GetAbilityState(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnGetAbilityState);
}

napi_value JSAbilityDelegator::GetCurrentTopAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnGetCurrentTopAbility);
}

napi_value JSAbilityDelegator::StartAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnStartAbility);
}

napi_value JSAbilityDelegator::DoAbilityForeground(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnDoAbilityForeground);
}

napi_value JSAbilityDelegator::DoAbilityBackground(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnDoAbilityBackground);
}

napi_value JSAbilityDelegator::Print(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnPrint);
}

napi_value JSAbilityDelegator::PrintSync(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnPrintSync);
}

napi_value JSAbilityDelegator::ExecuteShellCommand(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnExecuteShellCommand);
}

napi_value JSAbilityDelegator::FinishTest(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnFinishTest);
}

napi_value JSAbilityDelegator::SetMockList(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnSetMockList);
}

napi_value JSAbilityDelegator::AddInteropAbilityMonitorSync(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnAddInteropAbilityMonitorSync);
}

napi_value JSAbilityDelegator::RemoveInteropAbilityMonitorSync(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnRemoveInteropAbilityMonitorSync);
}

napi_value JSAbilityDelegator::OnAddAbilityMonitor(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));
    HandleEscape handleEscape(env);
    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    if (!ParseAbilityMonitorPara(env, info, monitor, false)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return ThrowJsError(env, INCORRECT_PARAMETERS,
            "Parse param monitor failed, monitor must be Monitor.");
    }

    NapiAsyncTask::CompleteCallback complete = [monitor](napi_env env, NapiAsyncTask &task, int32_t status) {
        TAG_LOGI(AAFwkTag::DELEGATOR, "complete called");
        HandleScope handleScope(env);
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "Calling AddAbilityMonitor failed."));
            return;
        }
        delegator->AddAbilityMonitor(monitor);
        ResolveWithNoError(env, task);
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnAddAbilityMonitor",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JSAbilityDelegator::OnAddAbilityMonitorSync(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));
    HandleScope handleScope(env);
    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    if (!ParseAbilityMonitorPara(env, info, monitor, true)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return ThrowJsError(env, INCORRECT_PARAMETERS,
            "Parse param monitor failed, monitor must be Monitor.");
    }
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        delegator->AddAbilityMonitor(monitor);
    } else {
        ThrowError(env, COMMON_FAILED, "Calling AddAbilityMonitorSync failed.");
    }
    return CreateJsUndefined(env);
}

napi_value JSAbilityDelegator::OnAddAbilityStageMonitor(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));

    HandleEscape handleEscape(env);
    bool isExisted = false;
    std::shared_ptr<AbilityStageMonitor> monitor = nullptr;
    if (!ParseAbilityStageMonitorPara(env, info, monitor, isExisted, false)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return ThrowJsError(env, INCORRECT_PARAMETERS,
            "Parse parameters failed, monitor must be Monitor and isExited must be boolean.");
    }

    NapiAsyncTask::CompleteCallback complete = [monitor](napi_env env, NapiAsyncTask &task, int32_t status) {
        TAG_LOGI(AAFwkTag::DELEGATOR, "complete called");
        HandleScope handleScope(env);
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "Calling AddAbilityStageMonitor failed."));
            return;
        }
        delegator->AddAbilityStageMonitor(monitor);
        ResolveWithNoError(env, task);
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnAddAbilityStageMonitor",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));

    if (!isExisted) {
        AddStageMonitorRecord(env, info.argv[INDEX_ZERO], monitor);
    }
    return handleEscape.Escape(result);
}

napi_value JSAbilityDelegator::OnAddAbilityStageMonitorSync(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));

    HandleScope handleScope(env);
    bool isExisted = false;
    std::shared_ptr<AbilityStageMonitor> monitor = nullptr;
    if (!ParseAbilityStageMonitorPara(env, info, monitor, isExisted, true)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return ThrowJsError(env, INCORRECT_PARAMETERS,
            "Parse param monitor failed, monitor must be Monitor.");
    }
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        ThrowError(env, COMMON_FAILED, "Calling AddAbilityStageMonitorSync failed.");
        return CreateJsUndefined(env);
    }
    delegator->AddAbilityStageMonitor(monitor);
    if (!isExisted) {
        AddStageMonitorRecord(env, info.argv[INDEX_ZERO], monitor);
    }
    return CreateJsUndefined(env);
}

napi_value JSAbilityDelegator::OnRemoveAbilityMonitor(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));

    HandleEscape handleEscape(env);
    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    if (!ParseAbilityMonitorPara(env, info, monitor, false)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return ThrowJsError(env, INCORRECT_PARAMETERS,
            "Parse monitor failed, removeAbilityMonitor must be Monitor.");
    }

    NapiAsyncTask::CompleteCallback complete =
        [monitor](napi_env env, NapiAsyncTask &task, int32_t status) mutable {
        TAG_LOGI(AAFwkTag::DELEGATOR, "complete called");
        HandleScope handleScope(env);
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "Calling RemoveAbilityMonitor failed."));
            return;
        }
        delegator->RemoveAbilityMonitor(monitor);
        ResolveWithNoError(env, task);
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnRemoveAbilityMonitor",
        env, CreateAsyncTaskWithLastParam(env,
        lastParam, nullptr, std::move(complete), &result));

    if (AbilityDelegatorRegistry::GetAbilityDelegator()) {
        for (auto iter = g_monitorRecord.begin(); iter != g_monitorRecord.end(); ++iter) {
            std::shared_ptr<NativeReference> jsMonitor = iter->first;
            bool isEquals = false;
            napi_strict_equals(env, (info.argv[INDEX_ZERO]), jsMonitor->GetNapiValue(), &isEquals);
            if (isEquals) {
                g_monitorRecord.erase(iter);
                break;
            }
        }
    }
    return handleEscape.Escape(result);
}


napi_value JSAbilityDelegator::OnRemoveAbilityMonitorSync(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));

    HandleScope handleScope(env);
    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    if (!ParseAbilityMonitorPara(env, info, monitor, true)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return ThrowJsError(env, INCORRECT_PARAMETERS,
            "Parse monitor failed, removeAbilityMonitor must be Monitor.");
    }
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        ThrowError(env, COMMON_FAILED, "Calling RemoveAbilityMonitorSync failed.");
        return CreateJsUndefined(env);
    }
    delegator->RemoveAbilityMonitor(monitor);
    for (auto iter = g_monitorRecord.begin(); iter != g_monitorRecord.end(); ++iter) {
        std::shared_ptr<NativeReference> jsMonitor = iter->first;
        bool isEquals = false;
        napi_strict_equals(env, (info.argv[INDEX_ZERO]), jsMonitor->GetNapiValue(), &isEquals);
        if (isEquals) {
            g_monitorRecord.erase(iter);
            break;
        }
    }
    return CreateJsUndefined(env);
}

napi_value JSAbilityDelegator::OnRemoveAbilityStageMonitor(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));

    HandleEscape handleEscape(env);
    bool isExisted = false;
    std::shared_ptr<AbilityStageMonitor> monitor = nullptr;
    if (!ParseAbilityStageMonitorPara(env, info, monitor, isExisted, false)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return ThrowJsError(env, INCORRECT_PARAMETERS,
            "Parse monitor failed, removeAbilityMonitor must be Monitor.");
    }

    NapiAsyncTask::CompleteCallback complete =
        [monitor](napi_env env, NapiAsyncTask &task, int32_t status) {
        TAG_LOGI(AAFwkTag::DELEGATOR, "complete called");
        HandleScope handleScope(env);

        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "Calling RemoveAbilityStageMonitor failed."));
            return;
        }
        delegator->RemoveAbilityStageMonitor(monitor);
        ResolveWithNoError(env, task);
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnRemoveAbilityStageMonitor", env,
        CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));

    if (isExisted) {
        RemoveStageMonitorRecord(env, info.argv[INDEX_ZERO]);
    }
    return handleEscape.Escape(result);
}

napi_value JSAbilityDelegator::OnRemoveAbilityStageMonitorSync(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));

    HandleScope handleScope(env);
    bool isExisted = false;
    std::shared_ptr<AbilityStageMonitor> monitor = nullptr;
    if (!ParseAbilityStageMonitorPara(env, info, monitor, isExisted, true)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return ThrowJsError(env, INCORRECT_PARAMETERS,
            "Parse monitor failed, removeAbilityMonitor must be Monitor.");
    }

    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        ThrowError(env, COMMON_FAILED, "Calling RemoveAbilityStageMonitorSync failed.");
        return CreateJsUndefined(env);
    }
    delegator->RemoveAbilityStageMonitor(monitor);
    if (isExisted) {
        RemoveStageMonitorRecord(env, info.argv[INDEX_ZERO]);
    }
    return CreateJsUndefined(env);
}

napi_value JSAbilityDelegator::OnWaitAbilityMonitor(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));

    HandleEscape handleEscape(env);
    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    TimeoutCallback opt {false, false};
    int64_t timeout = 0;
    if (!ParseWaitAbilityMonitorPara(env, info, monitor, opt, timeout)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return ThrowJsError(env, INCORRECT_PARAMETERS,
            "Parse monitor want failed, removeAbilityMonitor must be Monitor.");
    }

    auto abilityObjectBox = std::make_shared<AbilityObjectBox>();
    NapiAsyncTask::ExecuteCallback execute = [monitor, timeout, opt, abilityObjectBox]() {
        TAG_LOGI(AAFwkTag::DELEGATOR, "execute called");
        if (!abilityObjectBox) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "null abilityObjectBox");
            return;
        }

        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
            return;
        }

        std::shared_ptr<BaseDelegatorAbilityProperty> property = opt.hasTimeoutPara ?
            delegator->WaitAbilityMonitor(monitor, timeout) : delegator->WaitAbilityMonitor(monitor);
        auto jsProperty = std::static_pointer_cast<AppExecFwk::ADelegatorAbilityProperty>(property);
        if (!jsProperty || jsProperty->object_.expired()) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "invalid property");
            return;
        }

        abilityObjectBox->object_ = jsProperty->object_;
        std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
        g_abilityRecord.emplace(jsProperty->object_, jsProperty->token_);
    };

    NapiAsyncTask::CompleteCallback complete = [abilityObjectBox](napi_env env, NapiAsyncTask &task, int32_t status) {
        TAG_LOGI(AAFwkTag::DELEGATOR, "called");
        HandleScope handleScope(env);
        if (abilityObjectBox && !abilityObjectBox->object_.expired()) {
            ResolveWithNoError(env, task, abilityObjectBox->object_.lock()->GetNapiValue());
        } else {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "Calling WaitAbilityMonitor failed."));
        }
    };

    napi_value lastParam = nullptr;
    if (opt.hasCallbackPara) {
        lastParam = opt.hasTimeoutPara ? info.argv[INDEX_TWO] : info.argv[INDEX_ONE];
    }

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnWaitAbilityMonitor",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JSAbilityDelegator::OnWaitAbilityStageMonitor(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));

    HandleEscape handleEscape(env);
    std::shared_ptr<AbilityStageMonitor> monitor = nullptr;
    TimeoutCallback opt {false, false};
    int64_t timeout = 0;
    if (!ParseWaitAbilityStageMonitorPara(env, info, monitor, opt, timeout)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return ThrowJsError(env, INCORRECT_PARAMETERS, "Parse monitor failed, removeAbilityMonitor must be Monitor.");
    }

    auto abilityStageObjBox = std::make_shared<AbilityStageObjBox>();
    NapiAsyncTask::ExecuteCallback execute = [monitor, timeout, opt, abilityStageObjBox]() {
        TAG_LOGI(AAFwkTag::DELEGATOR, "execute called");
        if (!abilityStageObjBox) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "null abilityStageObjBox");
            return;
        }
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
            return;
        }
        std::shared_ptr<BaseDelegatorAbilityStageProperty> result;
        result = opt.hasTimeoutPara ?
            delegator->WaitAbilityStageMonitor(monitor, timeout) : delegator->WaitAbilityStageMonitor(monitor);
        auto jsStageProperty = std::static_pointer_cast<AppExecFwk::DelegatorAbilityStageProperty>(result);
        if (!jsStageProperty || jsStageProperty->object_.expired()) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "waitAbilityStageMonitor failed");
            return;
        }
        abilityStageObjBox->object_ = jsStageProperty->object_;
    };

    NapiAsyncTask::CompleteCallback complete = [abilityStageObjBox](napi_env env, NapiAsyncTask &task, int32_t status) {
        TAG_LOGI(AAFwkTag::DELEGATOR, "complete called");
        HandleScope handleScope(env);
        if (abilityStageObjBox && !abilityStageObjBox->object_.expired()) {
            ResolveWithNoError(env, task, abilityStageObjBox->object_.lock()->GetNapiValue());
        } else {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "Calling WaitAbilityStageMonitor failed."));
        }
    };
    napi_value lastParam = nullptr;
    if (opt.hasCallbackPara) {
        lastParam = opt.hasTimeoutPara ? info.argv[INDEX_TWO] : info.argv[INDEX_ONE];
    }
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnWaitAbilityStageMonitor",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JSAbilityDelegator::OnPrint(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));

    HandleEscape handleEscape(env);
    std::string msg;
    if (!ParsePrintPara(env, info, msg)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return ThrowJsError(env, INCORRECT_PARAMETERS,
            "Parse parameters msg failed, msg must be string.");
    }

    NapiAsyncTask::CompleteCallback complete = [msg](napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        TAG_LOGI(AAFwkTag::DELEGATOR, "complete called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "print failed."));
            return;
        }
        delegator->Print(msg);
        ResolveWithNoError(env, task);
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnPrint",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JSAbilityDelegator::OnPrintSync(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));

    HandleScope handleScope(env);
    std::string msg;
    if (!ParsePrintPara(env, info, msg)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return ThrowJsError(env, INCORRECT_PARAMETERS,
            "Parse msg failed, msg must be string.");
    }

    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        return CreateJsUndefined(env);
    }

    delegator->Print(msg);
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::OnExecuteShellCommand(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));

    HandleEscape handleEscape(env);
    std::string cmd;
    TimeoutCallback opt {false, false};
    int64_t timeoutSecs = 0;
    if (!ParseExecuteShellCommandPara(env, info, cmd, opt, timeoutSecs)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return ThrowJsError(env, INCORRECT_PARAMETERS,
            "Parse parameters failed, cmd must be string and timeout must be number.");
    }

    auto shellCmdResultBox = std::make_shared<ShellCmdResultBox>();
    NapiAsyncTask::ExecuteCallback execute = [cmd, timeoutSecs, shellCmdResultBox]() {
        TAG_LOGI(AAFwkTag::DELEGATOR, "execute called");
        if (!shellCmdResultBox) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "null shellCmdResultBox");
            return;
        }

        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
            return;
        }

        shellCmdResultBox->shellCmdResult_ = delegator->ExecuteShellCommand(cmd, timeoutSecs);
    };

    NapiAsyncTask::CompleteCallback complete = [shellCmdResultBox](napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        TAG_LOGI(AAFwkTag::DELEGATOR, "complete called");
        if (!shellCmdResultBox) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "null shellCmdResultBox");
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "executeShellCommand failed."));
            return;
        }

        napi_value result = CreateJsShellCmdResult(env, shellCmdResultBox->shellCmdResult_);
        if (result) {
            ResolveWithNoError(env, task, result);
        } else {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "executeShellCommand failed."));
        }
    };

    napi_value lastParam = nullptr;
    if (opt.hasCallbackPara) {
        lastParam = opt.hasTimeoutPara ? info.argv[INDEX_TWO] : info.argv[INDEX_ONE];
    }

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnExecuteShellCommand:" + cmd,
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JSAbilityDelegator::OnGetAppContext(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));

    HandleEscape handleEscape(env);
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        return CreateJsNull(env);
    }
    std::shared_ptr<AbilityRuntime::Context> context = delegator->GetAppContext();
    if (!context) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null context");
        return CreateJsNull(env);
    }
    napi_value value = CreateJsBaseContext(env, context, false);
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::Context>(context);
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null workContext");
        return CreateJsNull(env);
    }
    napi_coerce_to_native_binding_object(env, value, DetachCallbackFunc, AttachAppContext, workContext, nullptr);
    napi_status status = napi_wrap(env, value, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGI(AAFwkTag::DELEGATOR, "finalizer called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Context> *>(data);
        }, nullptr, nullptr);
    if (status != napi_ok && workContext != nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "napi_wrap Failed: %{public}d", status);
        delete workContext;
        return CreateJsNull(env);
    }
    return handleEscape.Escape(value);
}

napi_value JSAbilityDelegator::OnGetAbilityState(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));

    HandleEscape handleEscape(env);
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid argc");
        return CreateJsUndefined(env);
    }

    sptr<OHOS::IRemoteObject> remoteObject = nullptr;
    if (!ParseAbilityPara(env, info.argv[INDEX_ZERO], remoteObject)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return CreateJsUndefined(env);
    }

    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        return CreateJsNull(env);
    }
    AbilityDelegator::AbilityState lifeState = delegator->GetAbilityState(remoteObject);
    AbilityLifecycleState abilityLifeState = AbilityLifecycleState::UNINITIALIZED;
    AbilityLifecycleStateToJs(lifeState, abilityLifeState);
    return handleEscape.Escape(CreateJsValue(env, static_cast<int>(abilityLifeState)));
}

napi_value JSAbilityDelegator::OnGetCurrentTopAbility(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));

    HandleEscape handleEscape(env);
    if (info.argc >= ARGC_ONE && !AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ZERO], napi_function)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return ThrowJsError(env, INCORRECT_PARAMETERS, "Parse callback failed, callback must be function");
    }

    NapiAsyncTask::CompleteCallback complete = [](napi_env env, NapiAsyncTask &task, int32_t status) {
        TAG_LOGI(AAFwkTag::DELEGATOR, "complete called");
        HandleScope handleScope(env);
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "Calling GetCurrentTopAbility failed."));
            return;
        }

        auto property = delegator->GetCurrentTopAbility();
        auto jsProperty = std::static_pointer_cast<AppExecFwk::ADelegatorAbilityProperty>(property);
        if (!jsProperty || jsProperty->object_.expired()) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "invalid jsProperty");
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "Calling GetCurrentTopAbility failed."));
        } else {
            {
                std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
                g_abilityRecord.emplace(jsProperty->object_, jsProperty->token_);
            }
            ResolveWithNoError(env, task, jsProperty->object_.lock()->GetNapiValue());
        }
    };

    napi_value lastParam = (info.argc >= ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSAbilityDelegator::OnGetCurrentTopAbility",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JSAbilityDelegator::OnStartAbility(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));

    HandleEscape handleEscape(env);
    AAFwk::Want want;
    if (!ParseStartAbilityPara(env, info, want)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "parse startAbility param failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS,
            "Parse want failed, want must be Want.");
    }

    NapiAsyncTask::CompleteCallback complete = [want](napi_env env, NapiAsyncTask &task, int32_t status) {
        TAG_LOGI(AAFwkTag::DELEGATOR, "complete called");
        HandleScope handleScope(env);
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "startAbility failed."));
            return;
        }
        int result = delegator->StartAbility(want);
        if (result) {
            uint32_t apiTargetVersion = delegator->GetApiTargetVersion() % API_VERSION_MOD;
            if (apiTargetVersion >= API20) {
                task.Reject(env,
                    CreateJsError(env, static_cast<int>(GetJsErrorCodeByNativeError(result)), "startAbility failed."));
            } else {
                task.Reject(env, CreateJsError(env, result, "startAbility failed."));
            }
        } else {
            ResolveWithNoError(env, task);
        }
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSAbilityDelegator::OnStartAbility",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JSAbilityDelegator::OnDoAbilityForeground(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));

    HandleEscape handleEscape(env);
    sptr<OHOS::IRemoteObject> remoteObject = nullptr;
    if (!ParseAbilityCommonPara(env, info, remoteObject)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return ThrowJsError(env, INCORRECT_PARAMETERS,
            "Parse remoteObject failed, remoteObject must be RemoteObject.");
    }

    NapiAsyncTask::CompleteCallback complete = [remoteObject](napi_env env, NapiAsyncTask &task, int32_t status) {
        TAG_LOGI(AAFwkTag::DELEGATOR, "complete called");
        HandleScope handleScope(env);
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "Calling DoAbilityForeground failed."));
            return;
        }
        if (delegator->DoAbilityForeground(remoteObject)) {
            ResolveWithNoError(env, task, CreateJsNull(env));
        } else {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "Calling DoAbilityForeground failed."));
        }
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSAbilityDelegator::OnDoAbilityForeground",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JSAbilityDelegator::OnDoAbilityBackground(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));

    HandleEscape handleEscape(env);
    sptr<OHOS::IRemoteObject> remoteObject = nullptr;
    if (!ParseAbilityCommonPara(env, info, remoteObject)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return ThrowJsError(env, INCORRECT_PARAMETERS,
            "Parse remoteObject failed, remoteObject must be RemoteObject.");
    }

    NapiAsyncTask::CompleteCallback complete = [remoteObject](napi_env env, NapiAsyncTask &task, int32_t status) {
        TAG_LOGI(AAFwkTag::DELEGATOR, "complete called");
        HandleScope handleScope(env);
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "Calling DoAbilityBackground failed."));
            return;
        }
        if (delegator->DoAbilityBackground(remoteObject)) {
            ResolveWithNoError(env, task, CreateJsNull(env));
        } else {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "Calling DoAbilityBackground failed."));
        }
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSAbilityDelegator::OnDoAbilityBackground",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JSAbilityDelegator::OnFinishTest(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));

    HandleEscape handleEscape(env);
    std::string msg;
    int64_t code = 0;
    if (!ParseFinishTestPara(env, info, msg, code)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return ThrowJsError(env, INCORRECT_PARAMETERS,
            "Parse parameters failed, msg must be string and code must be number.");
    }

    NapiAsyncTask::CompleteCallback complete = [msg, code](napi_env env, NapiAsyncTask &task, int32_t status) {
        TAG_LOGI(AAFwkTag::DELEGATOR, "complete called");
        HandleScope handleScope(env);
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "Calling FinishTest failed."));
            return;
        }
        delegator->FinishUserTest(msg, code);
        ResolveWithNoError(env, task);
    };
    napi_value lastParam = (info.argc > ARGC_TWO) ? info.argv[INDEX_TWO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnFinishTest",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JSAbilityDelegator::OnSetMockList(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));

    HandleScope handleScope(env);
    std::map<std::string, std::string> mockList;
    if (!ParseMockListPara(env, info, mockList)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return ThrowJsError(env, INCORRECT_PARAMETERS,
            "Parse mockList failed, mockList must be MockList.");
    }
    auto engine = reinterpret_cast<NativeEngine*>(env);
    engine->SetMockModuleList(mockList);
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseMonitorPara(
    napi_env env, napi_value value, std::shared_ptr<AbilityMonitor> &monitor)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "monitorRecord size: %{public}zu", g_monitorRecord.size());

    HandleScope handleScope(env);
    for (auto iter = g_monitorRecord.begin(); iter != g_monitorRecord.end(); ++iter) {
        std::shared_ptr<NativeReference> jsMonitor = iter->first;
        bool isEquals = false;
        napi_strict_equals(env, value, jsMonitor->GetNapiValue(), &isEquals);
        if (isEquals) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "monitor exist");
            monitor = iter->second;
            return monitor ? CreateJsNull(env) : nullptr;
        }
    }

    napi_value abilityNameValue = nullptr;
    napi_get_named_property(env, value, "abilityName", &abilityNameValue);
    if (abilityNameValue == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null abilityNameValue");
        return nullptr;
    }

    std::string abilityName;
    if (!ConvertFromJsValue(env, abilityNameValue, abilityName)) {
        return nullptr;
    }

    std::string moduleName = "";
    napi_value moduleNameValue = nullptr;
    napi_get_named_property(env, value, "moduleName", &moduleNameValue);
    if (moduleNameValue != nullptr && !ConvertFromJsValue(env, moduleNameValue, moduleName)) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "get property moduleName failed");
        moduleName = "";
    }

    std::shared_ptr<JSAbilityMonitor> abilityMonitor = nullptr;
    if (moduleName.empty()) {
        abilityMonitor = std::make_shared<JSAbilityMonitor>(abilityName);
        abilityMonitor->SetJsAbilityMonitorEnv(env);
        abilityMonitor->SetJsAbilityMonitor(value);
        monitor = std::make_shared<AbilityMonitor>(abilityName, abilityMonitor);
    } else {
        abilityMonitor = std::make_shared<JSAbilityMonitor>(abilityName, moduleName);
        abilityMonitor->SetJsAbilityMonitorEnv(env);
        abilityMonitor->SetJsAbilityMonitor(value);
        monitor = std::make_shared<AbilityMonitor>(abilityName, moduleName, abilityMonitor);
    }

    std::shared_ptr<NativeReference> reference = nullptr;
    napi_ref ref = nullptr;
    napi_create_reference(env, value, 1, &ref);
    reference.reset(reinterpret_cast<NativeReference*>(ref));
    g_monitorRecord.emplace(reference, monitor);

    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseStageMonitorPara(
    napi_env env, napi_value value, std::shared_ptr<AbilityStageMonitor> &monitor, bool &isExisted)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "stageMonitorRecord size: %{public}zu", g_stageMonitorRecord.size());

    HandleScope handleScope(env);
    isExisted = false;
    for (auto iter = g_stageMonitorRecord.begin(); iter != g_stageMonitorRecord.end(); ++iter) {
        std::shared_ptr<NativeReference> jsMonitor = iter->first;
        bool isEquals = false;
        napi_strict_equals(env, value, jsMonitor->GetNapiValue(), &isEquals);
        if (isEquals) {
            TAG_LOGW(AAFwkTag::DELEGATOR, "abilityStage monitor exist");
            isExisted = true;
            monitor = iter->second;
            return monitor ? CreateJsNull(env) : nullptr;
        }
    }

    napi_value moduleNameValue = nullptr;
    napi_get_named_property(env, value, "moduleName", &moduleNameValue);
    if (moduleNameValue == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null moduleName");
        return nullptr;
    }
    std::string moduleName;
    if (!ConvertFromJsValue(env, moduleNameValue, moduleName)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "convert moduleName failed");
        return nullptr;
    }
    napi_value srcEntranceValue = nullptr;
    napi_get_named_property(env, value, "srcEntrance", &srcEntranceValue);
    if (srcEntranceValue == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null srcEntrance");
        return nullptr;
    }
    std::string srcEntrance;
    if (!ConvertFromJsValue(env, srcEntranceValue, srcEntrance)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "convert srcEntrance failed");
        return nullptr;
    }

    monitor = std::make_shared<AbilityStageMonitor>(moduleName, srcEntrance);
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseAbilityPara(
    napi_env env, napi_value value, sptr<OHOS::IRemoteObject> &remoteObject)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    HandleScope handleScope(env);
    std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
    for (auto iter = g_abilityRecord.begin(); iter != g_abilityRecord.end();) {
        if (iter->first.expired()) {
            iter = g_abilityRecord.erase(iter);
            continue;
        }

        bool isEquals = false;
        napi_strict_equals(env, value, iter->first.lock()->GetNapiValue(), &isEquals);
        if (isEquals) {
            remoteObject = iter->second;
            TAG_LOGI(AAFwkTag::DELEGATOR, "ability exist");
            return remoteObject ? CreateJsNull(env) : nullptr;
        }

        ++iter;
    }

    TAG_LOGE(AAFwkTag::DELEGATOR, "ability not exist");
    remoteObject = nullptr;
    return nullptr;
}

napi_value JSAbilityDelegator::CreateAbilityObject(napi_env env, const sptr<IRemoteObject> &remoteObject)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    HandleEscape handleEscape(env);
    if (!remoteObject) {
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null obj");
        return nullptr;
    }

    std::shared_ptr<NativeReference> reference = nullptr;
    napi_ref ref = nullptr;
    napi_create_reference(env, objValue, 1, &ref);
    reference.reset(reinterpret_cast<NativeReference*>(ref));

    std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
    g_abilityRecord[reference] = remoteObject;
    return handleEscape.Escape(objValue);
}

void JSAbilityDelegator::AbilityLifecycleStateToJs(
    const AbilityDelegator::AbilityState &lifeState, AbilityLifecycleState &abilityLifeState)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "lifeState: %{public}d", static_cast<int32_t>(lifeState));
    switch (lifeState) {
        case AbilityDelegator::AbilityState::STARTED:
            abilityLifeState = AbilityLifecycleState::CREATE;
            break;
        case AbilityDelegator::AbilityState::FOREGROUND:
            abilityLifeState = AbilityLifecycleState::FOREGROUND;
            break;
        case AbilityDelegator::AbilityState::BACKGROUND:
            abilityLifeState = AbilityLifecycleState::BACKGROUND;
            break;
        case AbilityDelegator::AbilityState::STOPPED:
            abilityLifeState = AbilityLifecycleState::DESTROY;
            break;
        default:
            abilityLifeState = AbilityLifecycleState::UNINITIALIZED;
            break;
    }
}

napi_value JSAbilityDelegator::ParseAbilityMonitorPara(
    napi_env env, NapiCallbackInfo& info, std::shared_ptr<AbilityMonitor> &monitor, bool isSync)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid argc");
        return nullptr;
    }

    if (!ParseMonitorPara(env, info.argv[INDEX_ZERO], monitor)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return nullptr;
    }

    if (isSync) {
        return CreateJsNull(env);
    }

    if (info.argc > ARGC_ONE) {
        if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ONE], napi_function)) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "parse callback param failed");
            return nullptr;
        }
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseAbilityStageMonitorPara(napi_env env, NapiCallbackInfo& info,
    std::shared_ptr<AbilityStageMonitor> &monitor, bool &isExisted, bool isSync)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid argc");
        return nullptr;
    }

    if (!ParseStageMonitorPara(env, info.argv[INDEX_ZERO], monitor, isExisted)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return nullptr;
    }

    if (isSync) {
        return CreateJsNull(env);
    }

    if (info.argc > ARGC_ONE) {
        if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ONE], napi_function)) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "invalid callback ");
            return nullptr;
        }
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseWaitAbilityMonitorPara(napi_env env, NapiCallbackInfo& info,
    std::shared_ptr<AbilityMonitor> &monitor, TimeoutCallback &opt, int64_t &timeout)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid argc");
        return nullptr;
    }

    if (!ParseMonitorPara(env, info.argv[INDEX_ZERO], monitor)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return nullptr;
    }

    if (!ParseTimeoutCallbackPara(env, info, opt, timeout)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid callback");
        return nullptr;
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseWaitAbilityStageMonitorPara(napi_env env, NapiCallbackInfo& info,
    std::shared_ptr<AbilityStageMonitor> &monitor, TimeoutCallback &opt, int64_t &timeout)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid argc");
        return nullptr;
    }

    bool isExisted = false;
    if (!ParseStageMonitorPara(env, info.argv[INDEX_ZERO], monitor, isExisted)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return nullptr;
    }
    if (!ParseTimeoutCallbackPara(env, info, opt, timeout)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid callback");
        return nullptr;
    }
    if (!isExisted) {
        AddStageMonitorRecord(env, info.argv[INDEX_ZERO], monitor);
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseTimeoutCallbackPara(
    napi_env env, NapiCallbackInfo& info, TimeoutCallback &opt, int64_t &timeout)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    opt.hasCallbackPara = false;
    opt.hasTimeoutPara = false;

    if (info.argc >= ARGC_TWO) {
        if (ConvertFromJsValue(env, info.argv[INDEX_ONE], timeout)) {
            opt.hasTimeoutPara = true;
        } else {
            if (info.argv[INDEX_ONE] == nullptr) {
                TAG_LOGW(AAFwkTag::DELEGATOR, "null info.argv[1]");
            } else if (AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ONE], napi_function)) {
                opt.hasCallbackPara = true;
                return CreateJsNull(env);
            } else {
                return nullptr;
            }
        }

        if (info.argc > ARGC_TWO) {
            if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_TWO], napi_function)) {
                if (info.argv[INDEX_TWO] == nullptr) {
                    TAG_LOGW(AAFwkTag::DELEGATOR, "null info.argv[2]");
                    return CreateJsNull(env);
                }
                return nullptr;
            }
            opt.hasCallbackPara = true;
        }
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParsePrintPara(napi_env env, NapiCallbackInfo& info, std::string &msg)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "enter");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid argc");
        return nullptr;
    }

    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], msg)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return nullptr;
    }

    if (info.argc > ARGC_ONE) {
        if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ONE], napi_function)) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "invalid callback");
            return nullptr;
        }
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseExecuteShellCommandPara(
    napi_env env, NapiCallbackInfo& info, std::string &cmd, TimeoutCallback &opt, int64_t &timeout)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid argc");
        return nullptr;
    }

    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], cmd)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return nullptr;
    }
    if (!ParseTimeoutCallbackPara(env, info, opt, timeout)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid callback");
        return nullptr;
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseAbilityCommonPara(
    napi_env env, NapiCallbackInfo& info, sptr<OHOS::IRemoteObject> &remoteObject)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid argc");
        return nullptr;
    }

    if (!ParseAbilityPara(env, info.argv[INDEX_ZERO], remoteObject)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return nullptr;
    }

    if (info.argc > ARGC_ONE) {
        if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ONE], napi_function)) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "invalid callback");
            return nullptr;
        }
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseStartAbilityPara(
    napi_env env, NapiCallbackInfo& info, AAFwk::Want &want)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid argc");
        return nullptr;
    }

    if (!OHOS::AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return nullptr;
    }

    if (info.argc > ARGC_ONE) {
        if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ONE], napi_function)) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "invalid callback");
            return nullptr;
        }
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseFinishTestPara(
    napi_env env, NapiCallbackInfo& info, std::string &msg, int64_t &code)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");
    if (info.argc < ARGC_TWO) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid argc");
        return nullptr;
    }

    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], msg)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid msg");
        return nullptr;
    }

    if (!ConvertFromJsValue(env, info.argv[INDEX_ONE], code)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid code");
        return nullptr;
    }

    if (info.argc > ARGC_TWO) {
        if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_TWO], napi_function)) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "invalid callback");
            return nullptr;
        }
    }
    return CreateJsNull(env);
}

bool JSAbilityDelegator::ParseMockListPara(
    napi_env env, NapiCallbackInfo& info, std::map<std::string, std::string> &mockList)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    if (info.argc != ARGC_ONE) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid argc");
        return false;
    }

    napi_value value = info.argv[INDEX_ZERO];
    if (value == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null arg[0]");
        return false;
    }

    if (!CheckTypeForNapiValue(env, value, napi_object)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "arg[0] not napi_object");
        return false;
    }

    std::vector<std::string> propNames;
    napi_value array = nullptr;
    napi_get_property_names(env, value, &array);
    if (!ParseArrayStringValue(env, array, propNames)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid propNames");
        return false;
    }

    for (const auto &propName : propNames) {
        napi_value prop = nullptr;
        napi_get_named_property(env, value, propName.c_str(), &prop);
        if (prop == nullptr) {
            TAG_LOGW(AAFwkTag::DELEGATOR, "null Prop: %{public}s", propName.c_str());
            continue;
        }
        if (!CheckTypeForNapiValue(env, prop, napi_string)) {
            TAG_LOGW(AAFwkTag::DELEGATOR, "prop not string: %{public}s", propName.c_str());
            continue;
        }
        std::string valName;
        if (!ConvertFromJsValue(env, prop, valName)) {
            TAG_LOGW(AAFwkTag::DELEGATOR, "convert valName failed: %{public}s", propName.c_str());
            continue;
        }
        TAG_LOGD(AAFwkTag::DELEGATOR, "key: %{public}s, value: %{public}s", propName.c_str(),
            valName.c_str());
        mockList.emplace(propName, valName);
    }
    return true;
}

bool JSAbilityDelegator::ParseArrayStringValue(
    napi_env env, napi_value array, std::vector<std::string> &vector)
{
    if (array == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null array");
        return false;
    }
    bool isArray = false;
    if (napi_is_array(env, array, &isArray) != napi_ok || isArray == false) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "not array");
        return false;
    }

    uint32_t arrayLen = 0;
    napi_get_array_length(env, array, &arrayLen);
    if (arrayLen == 0) {
        return true;
    }
    vector.reserve(arrayLen);
    for (uint32_t i = 0; i < arrayLen; i++) {
        std::string strItem;
        napi_value jsValue = nullptr;
        napi_get_element(env, array, i, &jsValue);
        if (!ConvertFromJsValue(env, jsValue, strItem)) {
            TAG_LOGW(AAFwkTag::DELEGATOR, "convert jsValue failed, i: %{public}u", i);
            continue;
        }
        vector.emplace_back(std::move(strItem));
    }
    return true;
}

void JSAbilityDelegator::AddStageMonitorRecord(
    napi_env env, napi_value value, const std::shared_ptr<AbilityStageMonitor> &monitor)
{
    if (!value) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null value");
        return;
    }
    if (!AbilityDelegatorRegistry::GetAbilityDelegator()) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        return;
    }
    std::shared_ptr<NativeReference> reference = nullptr;
    napi_ref ref = nullptr;
    napi_create_reference(env, value, 1, &ref);
    reference.reset(reinterpret_cast<NativeReference*>(ref));
    {
        std::unique_lock<std::mutex> lck(g_mtxStageMonitorRecord);
        g_stageMonitorRecord.emplace(reference, monitor);
    }
    TAG_LOGI(AAFwkTag::DELEGATOR, "end, size: %{public}zu", g_stageMonitorRecord.size());
}

void JSAbilityDelegator::RemoveStageMonitorRecord(napi_env env, napi_value value)
{
    if (!value) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null value");
        return;
    }
    if (!AbilityDelegatorRegistry::GetAbilityDelegator()) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        return;
    }
    std::unique_lock<std::mutex> lck(g_mtxStageMonitorRecord);
    for (auto iter = g_stageMonitorRecord.begin(); iter != g_stageMonitorRecord.end(); ++iter) {
        std::shared_ptr<NativeReference> jsMonitor = iter->first;
        bool isEquals = false;
        napi_strict_equals(env, value, jsMonitor->GetNapiValue(), &isEquals);
        if (isEquals) {
            g_stageMonitorRecord.erase(iter);
            TAG_LOGI(AAFwkTag::DELEGATOR, "end, size: %{public}zu", g_stageMonitorRecord.size());
            break;
        }
    }
}

napi_value JSAbilityDelegator::OnAddInteropAbilityMonitorSync(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));
    HandleScope handleScope(env);
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid argc");
        return ThrowJsError(env, INCORRECT_PARAMETERS,
            "Parse param monitor failed, monitor must be InteropAbilityMonitor.");
    }
    std::shared_ptr<InteropAbilityMonitor> monitor = nullptr;
    if (!ParseInteropMonitorPara(env, info.argv[INDEX_ZERO], monitor)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return ThrowJsError(env, INCORRECT_PARAMETERS,
            "Parse param monitor failed, monitor must be InteropAbilityMonitor.");
    }
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::JS);
    if (delegator) {
        monitor->SetLanguage(AbilityRuntime::Runtime::Language::JS);
        delegator->AddInteropAbilityMonitor(monitor);
    } else {
        ThrowError(env, COMMON_FAILED, "Calling AddInteropAbilityMonitorSync failed.");
    }
    return CreateJsUndefined(env);
}

napi_value JSAbilityDelegator::OnRemoveInteropAbilityMonitorSync(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "argc: %{public}d", static_cast<int32_t>(info.argc));
    HandleScope handleScope(env);
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid argc");
        return ThrowJsError(env, INCORRECT_PARAMETERS,
            "Parse monitor failed, removeInteropAbilityMonitorSync must be InteropAbilityMonitor.");
    }
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::JS);
    if (!delegator) {
        ThrowError(env, COMMON_FAILED, "Calling RemoveInteropAbilityMonitorSync failed.");
        return CreateJsUndefined(env);
    }
    for (auto iter = g_interopMonitorRecord.begin(); iter != g_interopMonitorRecord.end(); ++iter) {
        std::shared_ptr<NativeReference> jsMonitor = iter->first;
        bool isEquals = false;
        napi_strict_equals(env, info.argv[INDEX_ZERO], jsMonitor->GetNapiValue(), &isEquals);
        if (isEquals) {
            delegator->RemoveInteropAbilityMonitor(iter->second);
            g_interopMonitorRecord.erase(iter);
            break;
        }
    }
    return CreateJsUndefined(env);
}

napi_value JSAbilityDelegator::ParseInteropMonitorPara(napi_env env, napi_value value,
    std::shared_ptr<InteropAbilityMonitor> &monitor)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "interopMonitorRecord size: %{public}zu", g_interopMonitorRecord.size());

    HandleScope handleScope(env);
    for (auto iter = g_interopMonitorRecord.begin(); iter != g_interopMonitorRecord.end(); ++iter) {
        std::shared_ptr<NativeReference> jsMonitor = iter->first;
        bool isEquals = false;
        napi_strict_equals(env, value, jsMonitor->GetNapiValue(), &isEquals);
        if (isEquals) {
            TAG_LOGW(AAFwkTag::DELEGATOR, "interop monitor exist");
            monitor = iter->second;
            return monitor ? CreateJsNull(env) : nullptr;
        }
    }

    napi_value abilityNameValue = nullptr;
    napi_get_named_property(env, value, "abilityName", &abilityNameValue);
    if (abilityNameValue == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null abilityNameValue");
        return nullptr;
    }

    std::string abilityName;
    if (!ConvertFromJsValue(env, abilityNameValue, abilityName)) {
        return nullptr;
    }

    std::string moduleName = "";
    napi_value moduleNameValue = nullptr;
    napi_get_named_property(env, value, "moduleName", &moduleNameValue);
    if (moduleNameValue != nullptr && !ConvertFromJsValue(env, moduleNameValue, moduleName)) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "get property moduleName failed");
        moduleName = "";
    }

    auto jsInteropMonitor = std::make_shared<JsInteropAbilityMonitor>(abilityName, moduleName);
    jsInteropMonitor->SetJsInteropAbilityMonitor(env, value);
    auto *aniEnvVoid = AppExecFwk::AbilityDelegatorRegistry::GetAniEnv();
    if (aniEnvVoid != nullptr) {
        jsInteropMonitor->SetAniEnv(aniEnvVoid);
    }
    monitor = std::make_shared<InteropAbilityMonitor>(abilityName, moduleName, jsInteropMonitor);

    std::shared_ptr<NativeReference> reference = nullptr;
    napi_ref ref = nullptr;
    napi_create_reference(env, value, 1, &ref);
    reference.reset(reinterpret_cast<NativeReference*>(ref));
    g_interopMonitorRecord.emplace(reference, monitor);

    return CreateJsNull(env);
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
