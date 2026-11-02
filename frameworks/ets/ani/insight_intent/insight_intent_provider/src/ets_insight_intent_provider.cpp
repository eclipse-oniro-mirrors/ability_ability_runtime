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

#include "ets_insight_intent_provider.h"

#include <memory>
#include <mutex>
#include <vector>

#include "ability_business_error.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_result.h"
#include "ani_common_execute_result.h"
#include "ani_common_util.h"
#include "ets_error_utils.h"
#include "ets_insight_intent_execution_info.h"
#include "insight_intent_delay_result_callback_mgr.h"
#include "insight_intent_will_execute_callback_mgr.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
constexpr const char *INSIGHT_INTENT_PROVIDER_CLASS_NAME =
    "@ohos.app.ability.insightIntentProvider.insightIntentProvider";
}

class EtsInsightIntentProvider {
public:
    EtsInsightIntentProvider() = default;
    ~EtsInsightIntentProvider() = default;

    static void OnSendExecuteResult(ani_env *env, ani_int intentId, ani_object aniResult, ani_object callback)
    {
        TAG_LOGD(AAFwkTag::INTENT, "OnSendExecuteResult called");
        HandleSendExecuteResultCommon(env, intentId, aniResult, callback, false);
    }

    static void OnSendIntentResult(ani_env *env, ani_int intentId, ani_object aniResult, ani_object callback)
    {
        TAG_LOGD(AAFwkTag::INTENT, "OnSendIntentResult called");
        HandleSendExecuteResultCommon(env, intentId, aniResult, callback, true);
    }

private:
    static void HandleSendExecuteResultCommon(
        ani_env *env, ani_int intentId, ani_object aniResult, ani_object callback, bool isDecorator)
    {
        auto nativeResult = std::make_shared<AppExecFwk::InsightIntentExecuteResult>();
        ani_object errorObject = nullptr;
        if (!UnwrapExecuteResult(env, aniResult, *nativeResult, isDecorator)) {
            TAG_LOGE(AAFwkTag::INTENT, " failed to UnwrapExecuteResult");
            errorObject =
                EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER));
            AppExecFwk::AsyncCallback(env, callback, errorObject, nullptr);
            return;
        }
        auto errCode =
            InsightIntentDelayResultCallbackMgr::GetInstance().HandleExecuteDone(intentId, *nativeResult, isDecorator);
        errorObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(errCode));
        AppExecFwk::AsyncCallback(env, callback, errorObject, nullptr);
    }
};

namespace {
constexpr const char *WILL_EXECUTE_METHOD = "onWillInsightIntentExecuteCallBack";
}

class EtsInsightIntentWillExecuteCallbackClient {
public:
    EtsInsightIntentWillExecuteCallbackClient(ani_vm *vm, ani_ref callbackRef)
        : vm_(vm), callbackRef_(callbackRef) {}

    virtual ~EtsInsightIntentWillExecuteCallbackClient()
    {
        bool isAttachThread = false;
        ani_env *env = AppExecFwk::AttachAniEnv(vm_, isAttachThread);
        if (env != nullptr && callbackRef_ != nullptr) {
            env->GlobalReference_Delete(callbackRef_);
            callbackRef_ = nullptr;
        }
        if (isAttachThread) {
            AppExecFwk::DetachAniEnv(vm_, isAttachThread);
        }
    }

    void Process(const AppExecFwk::InsightIntentExecutionInfo &info)
    {
        bool isAttachThread = false;
        ani_env *env = AppExecFwk::AttachAniEnv(vm_, isAttachThread);
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "attach ani env failed");
            return;
        }
        ani_object executionInfo = CreateEtsInsightIntentExecutionInfo(env, info);
        if (executionInfo != nullptr && callbackRef_ != nullptr) {
            ani_status status = env->Object_CallMethodByName_Void(reinterpret_cast<ani_object>(callbackRef_),
                WILL_EXECUTE_METHOD, "C{insightIntent.InsightIntentExecutionInfoInner}:", executionInfo);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::INTENT, "call will execute callback failed, status:%{public}d", status);
            }
        }
        if (isAttachThread) {
            AppExecFwk::DetachAniEnv(vm_, isAttachThread);
        }
    }

    bool IsStrictEquals(ani_env *env, ani_object callback)
    {
        if (env == nullptr || callbackRef_ == nullptr || callback == nullptr) {
            return false;
        }
        ani_boolean isEquals = ANI_FALSE;
        if (env->Reference_StrictEquals(callbackRef_, callback, &isEquals) != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "reference strict equals failed");
            return false;
        }
        return isEquals == ANI_TRUE;
    }

private:
    ani_vm *vm_ = nullptr;
    ani_ref callbackRef_ = nullptr;
};

class EtsInsightIntentWillExecuteCallback {
public:
    struct CallbackEntry {
        uint32_t callbackId = 0;
        std::shared_ptr<EtsInsightIntentWillExecuteCallbackClient> client = nullptr;
    };

    static void OnWillInsightIntentExecute(ani_env *env, ani_object callback)
    {
        TAG_LOGD(AAFwkTag::INTENT, "on will insight intent execute ani called");
        ani_vm *vm = nullptr;
        if (env->GetVM(&vm) != ANI_OK || vm == nullptr || callback == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "get vm failed or null callback");
            return;
        }
        ani_ref callbackRef = nullptr;
        if (env->GlobalReference_Create(callback, &callbackRef) != ANI_OK || callbackRef == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "create global ref failed");
            return;
        }
        auto client = std::make_shared<EtsInsightIntentWillExecuteCallbackClient>(vm, callbackRef);
        uint32_t callbackId = 0;
        auto ret = AbilityRuntime::InsightIntentWillExecuteCallbackMgr::GetInstance()->Register(
            [client](const AppExecFwk::InsightIntentExecutionInfo &info) {
                client->Process(info);
            },
            callbackId);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "register will execute callback failed, ret:%{public}d", ret);
            return;
        }
        std::lock_guard<std::mutex> lock(entriesMutex_);
        entries_.push_back({callbackId, client});
    }

    static void OffWillInsightIntentExecute(ani_env *env, ani_object callback)
    {
        TAG_LOGD(AAFwkTag::INTENT, "off will insight intent execute ani called");
        if (callback == nullptr) {
            {
                std::lock_guard<std::mutex> lock(entriesMutex_);
                entries_.clear();
            }
            AbilityRuntime::InsightIntentWillExecuteCallbackMgr::GetInstance()->UnregisterAll();
            return;
        }
        uint32_t callbackId = 0;
        bool found = false;
        {
            std::lock_guard<std::mutex> lock(entriesMutex_);
            for (auto it = entries_.begin(); it != entries_.end(); ++it) {
                if (it->client != nullptr && it->client->IsStrictEquals(env, callback)) {
                    callbackId = it->callbackId;
                    entries_.erase(it);
                    found = true;
                    break;
                }
            }
        }
        if (found) {
            AbilityRuntime::InsightIntentWillExecuteCallbackMgr::GetInstance()->Unregister(callbackId);
        }
    }

private:
    static std::vector<CallbackEntry> entries_;
    static std::mutex entriesMutex_;
};

std::vector<EtsInsightIntentWillExecuteCallback::CallbackEntry> EtsInsightIntentWillExecuteCallback::entries_;
std::mutex EtsInsightIntentWillExecuteCallback::entriesMutex_;

void EtsInsightIntentProviderInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::INTENT, "EtsInsightIntentProviderInit called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return;
    }

    ani_namespace ns;
    ani_status status = env->FindNamespace(INSIGHT_INTENT_PROVIDER_CLASS_NAME, &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "FindNamespace insightIntentProvider failed status: %{public}d", status);
        return;
    }

    std::array nativeFunctions = {
        ani_native_function { "nativeSendExecuteResult", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentProvider::OnSendExecuteResult) },
        ani_native_function {
            "nativeSendIntentResult", nullptr, reinterpret_cast<void *>(EtsInsightIntentProvider::OnSendIntentResult) },
        ani_native_function { "nativeOnWillInsightIntentExecute", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentWillExecuteCallback::OnWillInsightIntentExecute) },
        ani_native_function { "nativeOffWillInsightIntentExecute", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentWillExecuteCallback::OffWillInsightIntentExecute) },
    };

    status = env->Namespace_BindNativeFunctions(ns, nativeFunctions.data(), nativeFunctions.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "bind nativeExecuteSync failed status: %{public}d", status);
    }
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::INTENT, "ANI_Constructor");
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGD(AAFwkTag::INTENT, "GetEnv failed status: %{public}d", status);
        return ANI_NOT_FOUND;
    }

    EtsInsightIntentProviderInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::INTENT, "ANI_Constructor finish");
    return ANI_OK;
}
}
} // namespace AbilityRuntime
} // namespace OHOS
