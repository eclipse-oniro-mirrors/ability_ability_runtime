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

#include "ets_app_process_state_observer.h"

#include "ani_common_util.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *APP_PROCESS_STATE_OBSERVER_INNER =
    "@ohos.app.ability.AppProcessStateObserver.AppProcessStateObserverInner";
constexpr const char *PROCESS_DATA_CLASS_NAME = "application.ProcessData.ProcessData";
constexpr const char *SIGNATURE_PROCESS_DATA = "C{application.ProcessData.ProcessData}:";
constexpr const char *ON_PROCESS_CREATED = "onProcessCreated";
constexpr const char *ON_PROCESS_DIED = "onProcessDied";
} // namespace

EtsAppProcessStateObserver::EtsAppProcessStateObserver(ani_vm *etsVm) : etsVm_(etsVm) {}

EtsAppProcessStateObserver::~EtsAppProcessStateObserver()
{
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetEnv failed");
        return;
    }
    ani_status status = ANI_ERROR;
    {
        std::lock_guard lock(mutex_);
        for (auto &observer : observers_) {
            if (!observer) {
                TAG_LOGE(AAFwkTag::APPKIT, "Invalid aniObserver");
                continue;
            }
            if ((status = env->GlobalReference_Delete(observer)) != ANI_OK) {
                TAG_LOGE(AAFwkTag::APPKIT, "GlobalReference_Delete status: %{public}d", status);
            }
        }
    }
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

ani_object WrapProcessData(ani_env *env, const AppExecFwk::ProcessData &processData)
{
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object object = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(PROCESS_DATA_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindClass status: %{public}d", status);
        return nullptr;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null cls");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Class_FindMethod status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_New status: %{public}d", status);
        return nullptr;
    }
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null object");
        return nullptr;
    }
    env->Object_SetFieldByName_Ref(object, "bundleName", AppExecFwk::GetAniString(env, processData.bundleName));
    env->Object_SetFieldByName_Int(object, "pid", processData.pid);
    env->Object_SetFieldByName_Int(object, "uid", processData.uid);
    env->Object_SetFieldByName_Int(object, "state", static_cast<ani_int>(processData.state));
    env->Object_SetFieldByName_Boolean(object, "isContinuousTask", processData.isContinuousTask);
    env->Object_SetFieldByName_Boolean(object, "isKeepAlive", processData.isKeepAlive);
    return object;
}

void EtsAppProcessStateObserver::CallEtsFunction(ani_env *env, ani_object etsObserverObject,
    const char *methodName, const char *signature, ani_object processDataObj)
{
    TAG_LOGD(AAFwkTag::APPKIT, "call method:%{public}s", methodName);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_OK;
    status = env->FindClass(APP_PROCESS_STATE_OBSERVER_INNER, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindClass status: %{public}d", status);
        return;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null cls");
        return;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, methodName, signature, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Class_FindMethod status: %{public}d", status);
        return;
    }
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null method");
        return;
    }
    env->ResetError();
    if ((status = env->Object_CallMethod_Void(etsObserverObject, method, processDataObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_CallMethod_Void status: %{public}d", status);
    }
}

void EtsAppProcessStateObserver::HandleOnProcessCreated(const AppExecFwk::ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetEnv failed");
        return;
    }
    auto processDataObj = WrapProcessData(env, processData);
    if (processDataObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null processDataObj");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    std::set<ani_ref> observersCopy;
    {
        std::lock_guard lock(mutex_);
        observersCopy = observers_;
    }
    for (auto &observer : observersCopy) {
        if (!observer) {
            TAG_LOGE(AAFwkTag::APPKIT, "Invalid aniObserver");
            continue;
        }
        CallEtsFunction(env, reinterpret_cast<ani_object>(observer),
            ON_PROCESS_CREATED, SIGNATURE_PROCESS_DATA, processDataObj);
    }
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

void EtsAppProcessStateObserver::HandleOnProcessDied(const AppExecFwk::ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetEnv failed");
        return;
    }
    auto processDataObj = WrapProcessData(env, processData);
    if (processDataObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null processDataObj");
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    std::set<ani_ref> observersCopy;
    {
        std::lock_guard lock(mutex_);
        observersCopy = observers_;
    }
    for (auto &observer : observersCopy) {
        if (!observer) {
            TAG_LOGE(AAFwkTag::APPKIT, "Invalid aniObserver");
            continue;
        }
        CallEtsFunction(env, reinterpret_cast<ani_object>(observer),
            ON_PROCESS_DIED, SIGNATURE_PROCESS_DATA, processDataObj);
    }
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

void EtsAppProcessStateObserver::OnProcessCreated(const AppExecFwk::ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    wptr<EtsAppProcessStateObserver> self(this);
    auto task = [self, processData]() {
        sptr<EtsAppProcessStateObserver> observer = self.promote();
        if (observer == nullptr) {
            TAG_LOGW(AAFwkTag::APPKIT, "null observer");
            return;
        }
        observer->HandleOnProcessCreated(processData);
    };
    if (AniTask::AniSendEvent(task) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to aniSendEvent");
    }
}

void EtsAppProcessStateObserver::OnProcessDied(const AppExecFwk::ProcessData &processData)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    wptr<EtsAppProcessStateObserver> self(this);
    auto task = [self, processData]() {
        sptr<EtsAppProcessStateObserver> observer = self.promote();
        if (observer == nullptr) {
            TAG_LOGW(AAFwkTag::APPKIT, "null observer");
            return;
        }
        observer->HandleOnProcessDied(processData);
    };
    if (AniTask::AniSendEvent(task) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to aniSendEvent");
    }
}

void EtsAppProcessStateObserver::Register(ani_object aniCallback)
{
    if (aniCallback == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null aniCallback");
        return;
    }
    ani_ref aniCallbackRef = nullptr;
    ani_status status = ANI_ERROR;
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetEnv failed");
        return;
    }
    if ((status = env->GlobalReference_Create(aniCallback, &aniCallbackRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "GlobalReference_Create status: %{public}d", status);
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    {
        std::lock_guard lock(mutex_);
        observers_.emplace(aniCallbackRef);
    }
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

bool EtsAppProcessStateObserver::UnRegister(ani_object aniCallback)
{
    ani_status status = ANI_ERROR;
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetEnv failed");
        return false;
    }
    if (aniCallback == nullptr) {
        TAG_LOGI(AAFwkTag::APPKIT, "null aniCallback, unregister all");
        std::lock_guard lock(mutex_);
        for (auto &observer : observers_) {
            if (!observer) {
                TAG_LOGE(AAFwkTag::APPKIT, "Invalid aniObserver");
                continue;
            }
            if ((status = env->GlobalReference_Delete(observer)) != ANI_OK) {
                TAG_LOGE(AAFwkTag::APPKIT, "GlobalReference_Delete status: %{public}d", status);
            }
        }
        observers_.clear();
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return true;
    }
    std::lock_guard lock(mutex_);
    for (auto &observer : observers_) {
        if (!observer) {
            TAG_LOGE(AAFwkTag::APPKIT, "Invalid aniObserver");
            continue;
        }
        ani_boolean isEqual = false;
        env->Reference_StrictEquals(aniCallback, observer, &isEqual);
        if (isEqual) {
            if ((status = env->GlobalReference_Delete(observer)) != ANI_OK) {
                TAG_LOGE(AAFwkTag::APPKIT, "GlobalReference_Delete status: %{public}d", status);
                AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
                return false;
            }
            AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
            return observers_.erase(observer) == 1;
        }
    }
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
    return false;
}

bool EtsAppProcessStateObserver::IsEmpty() const
{
    std::lock_guard lock(mutex_);
    return observers_.empty();
}
} // namespace AbilityRuntime
} // namespace OHOS
