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

#include "ets_ability_auto_startup_callback.h"

#include "ets_ability_auto_startup_manager_utils.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *METHOD_ON = "onAutoStartupOn";
constexpr const char *METHOD_OFF = "onAutoStartupOff";
constexpr const char *SIGNATURE_AUTO_STARTUP_INFO = "C{application.AutoStartupInfo.AutoStartupInfo}:";
} // namespace
EtsAbilityAutoStartupCallback::EtsAbilityAutoStartupCallback(ani_vm *etsVm) : etsVm_(etsVm) {}

EtsAbilityAutoStartupCallback::~EtsAbilityAutoStartupCallback() {}

ani_status EtsAbilityAutoStartupCallback::AniSendEvent(const std::function<void()> task)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "AniSendEvent");
    if (task == nullptr) {
        TAG_LOGD(AAFwkTag::AUTO_STARTUP, "null task");
        return ani_status::ANI_INVALID_ARGS;
    }
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        if (!runner) {
            TAG_LOGD(AAFwkTag::AUTO_STARTUP, "null EventRunner");
            return ani_status::ANI_NOT_FOUND;
        }
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    }
    if (mainHandler_ == nullptr) {
        TAG_LOGD(AAFwkTag::AUTO_STARTUP, "null mainHandler");
        return ani_status::ANI_NOT_FOUND;
    }
    mainHandler_->PostTask(std::move(task));
    return ani_status::ANI_OK;
}

void EtsAbilityAutoStartupCallback::OnAutoStartupOn(const AutoStartupInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called OnAutoStartupOn");
    EtsCallFunction(info, METHOD_ON);
}

void EtsAbilityAutoStartupCallback::OnAutoStartupOff(const AutoStartupInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called OnAutoStartupOff");
    EtsCallFunction(info, METHOD_OFF);
}

void EtsAbilityAutoStartupCallback::Register(ani_object value)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called Register");
    std::vector<ani_ref> callbacks;
    GetCallbackVector(callbacks);
    for (const auto &callback : callbacks) {
        if (IsEtsCallbackEquals(callback, value)) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "callback exist");
            return;
        }
    }

    ani_ref ref = nullptr;
    ani_status status = ANI_ERROR;
    ani_env *env = GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null env");
        return;
    }
    if ((status = env->GlobalReference_Create(value, &ref)) != ANI_OK || ref == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "status : %{public}d or null ref", status);
        return;
    }

    std::lock_guard<std::mutex> lock(mutexlock_);
    callbacks_.emplace_back(ref);
}

void EtsAbilityAutoStartupCallback::Unregister(ani_object value)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called Unregister");
    std::lock_guard<std::mutex> lock(mutexlock_);
    ani_boolean isUndefined = false;
    ani_env *env = GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null env");
        return;
    }
    if (env->Reference_IsUndefined(value, &isUndefined) != ANI_OK || isUndefined) {
        TAG_LOGD(AAFwkTag::AUTO_STARTUP, "invalid callback, clear all callback");
        for (auto &callback : callbacks_) {
            env->GlobalReference_Delete(callback);
        }
        callbacks_.clear();
        return;
    }

    for (auto it = callbacks_.begin(); it != callbacks_.end();) {
        if (IsEtsCallbackEquals(*it, value)) {
            env->GlobalReference_Delete(*it);
            it = callbacks_.erase(it);
            break;
        }
        it++;
    }
}

void EtsAbilityAutoStartupCallback::GetCallbackVector(std::vector<ani_ref>& callbacks)
{
    std::lock_guard<std::mutex> lock(mutexlock_);
    callbacks = callbacks_;
}

bool EtsAbilityAutoStartupCallback::IsCallbacksEmpty()
{
    std::lock_guard<std::mutex> lock(mutexlock_);
    return callbacks_.empty();
}

void EtsAbilityAutoStartupCallback::EtsCallFunction(const AutoStartupInfo &info, const char *methodName)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called EtsCallFunction");
    wptr<EtsAbilityAutoStartupCallback> weakPtr = this;
    auto task = [info, methodName, weakPtr] () {
        auto obj = weakPtr.promote();
        if (obj == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null obj");
            return;
        }
        obj->EtsCallFunctionWorker(info, methodName);
    };
    if (AniSendEvent(task) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to aniSendEvent");
    }
}

void EtsAbilityAutoStartupCallback::EtsCallFunctionWorker(const AutoStartupInfo &info, const char *methodName)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called EtsCallFunctionWorker");
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    bool isAttachThread = false;
    env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null env");
        return;
    }
    std::vector<ani_ref> callbacks;
    GetCallbackVector(callbacks);
    for (auto callback : callbacks) {
        if (callback == nullptr) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null obj");
            continue;
        }

        auto autoStartupInfoObj = CreateAniAutoStartupInfo(env, info);
        if (autoStartupInfoObj == nullptr) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null autoStartupInfoObj");
            continue;
        }
        if ((status = env->Object_CallMethodByName_Void(reinterpret_cast<ani_object>(callback),
            methodName, SIGNATURE_AUTO_STARTUP_INFO, autoStartupInfoObj)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "status: %{public}d", status);
            continue;
        }
    }
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

ani_env* EtsAbilityAutoStartupCallback::GetAniEnv()
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called GetAniEnv");
    ani_env *env = nullptr;
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "etsVm nullptr");
        return nullptr;
    }
    ani_status status = etsVm_->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "GetEnv failed");
        return nullptr;
    }
    return env;
}

bool EtsAbilityAutoStartupCallback::IsEtsCallbackEquals(ani_ref callback, ani_object value)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called IsEtsCallbackEquals");
    if (callback == nullptr || value == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "callback or value null");
        return false;
    }

    ani_boolean result = ANI_FALSE;
    ani_env *env = GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "null env");
        return false;
    }
    if (env->Reference_StrictEquals(callback, value, &result) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "objects not match");
        return false;
    }

    return result == ANI_TRUE;
}
} // namespace AbilityRuntime
} // namespace OHOS