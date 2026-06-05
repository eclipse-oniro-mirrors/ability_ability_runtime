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

#include "ets_query_erms_observer.h"

#include "ets_ability_manager_utils.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"


namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *ATOMIC_SERVICE_STARTUP_RULE_IMPL_CLASS_NAME =
    "@ohos.app.ability.abilityManager.abilityManager.AtomicServiceStartupRuleImpl";
}
EtsQueryERMSObserver::EtsQueryERMSObserver(ani_vm *etsVm) : etsVm_(etsVm) {}

EtsQueryERMSObserver::~EtsQueryERMSObserver()
{
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::QUERY_ERMS, "null env");
        return;
    }
    for (auto it = etsObserverObjectList_.begin(); it != etsObserverObjectList_.end();) {
        env->GlobalReference_Delete(it->callback);
        it++;
    }
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

void EtsQueryERMSObserver::OnQueryFinished(const std::string &appId, const std::string &startTime,
    const AtomicServiceStartupRule &rule, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::QUERY_ERMS, "OnQueryFinished");
    HandleOnQueryFinished(appId, startTime, rule, resultCode);
}

void EtsQueryERMSObserver::HandleOnQueryFinished(const std::string &appId, const std::string &startTime,
    const AtomicServiceStartupRule &rule, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::QUERY_ERMS, "HandleOnQueryFinished");
    std::vector<ani_object> callbacks;
    {
        std::unique_lock<std::mutex> lock(etsObserverObjectListLock_);
        for (auto it = etsObserverObjectList_.begin(); it != etsObserverObjectList_.end();) {
            if (it->appId != appId || it->startTime != startTime || it->callback == nullptr) {
                it++;
                continue;
            }
            callbacks.emplace_back(it->callback);
            it = etsObserverObjectList_.erase(it);
        }
    }
    for (const auto &callback : callbacks) {
        CallCallback(callback, rule, resultCode);
        FinishAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, "StartQueryERMS", atoi(startTime.c_str()));
    }
}

void EtsQueryERMSObserver::CallCallback(ani_object callback, const AtomicServiceStartupRule &rule, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::QUERY_ERMS, "CallCallback");
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::QUERY_ERMS, "null callback");
        return;
    }
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::QUERY_ERMS, "GetEnv failed");
        return;
    }
    ani_object aniObject = nullptr;
    ani_object ruleObj = nullptr;
    if (resultCode == ERR_OK && WrapAtomicServiceStartupRule(env, rule, ruleObj)) {
        aniObject = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK);
        AppExecFwk::AsyncCallback(env, callback, aniObject, ruleObj);
        AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
        return;
    }
    if (resultCode == ERR_OK) {
        aniObject = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK);
    } else {
        aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, resultCode, "",
            GetInnerErrorMsg(AbilityInnerErrorMsg::QUERY_ATOMIC_SERVICE_STARTUP_RULE_FAILED));
    }
    AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}

void EtsQueryERMSObserver::AddEtsObserverObject(const std::string &appId, const std::string &startTime,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::QUERY_ERMS, "call");
    std::unique_lock<std::mutex> lock(etsObserverObjectListLock_);
    for (auto it = etsObserverObjectList_.begin(); it != etsObserverObjectList_.end(); ++it) {
        if (it->appId == appId && it->startTime == startTime) {
            TAG_LOGW(AAFwkTag::QUERY_ERMS, "The etsObject has been added");
            return;
        }
    }

    int traceId = 0;
    try {
        traceId = std::stoi(startTime);
    } catch (...) {
        TAG_LOGW(AAFwkTag::QUERY_ERMS, "Invalid startTime format: %{public}s", startTime.c_str());
    }
    StartAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, "StartQueryERMS", atoi(startTime.c_str()));
    EtsQueryERMSObserverObject object;
    object.appId = appId;
    object.startTime = startTime;
    object.callback = reinterpret_cast<ani_object>(callback);

    etsObserverObjectList_.emplace_back(object);
}

bool EtsQueryERMSObserver::WrapAtomicServiceStartupRule(ani_env *env,
    const AbilityRuntime::AtomicServiceStartupRule &rule, ani_object &ruleObj)
{
    TAG_LOGD(AAFwkTag::QUERY_ERMS, "WrapAtomicServiceStartupRule");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::QUERY_ERMS, "null env");
        return false;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass(ATOMIC_SERVICE_STARTUP_RULE_IMPL_CLASS_NAME, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::QUERY_ERMS, "Find AtomicServiceStartupRuleImpl Class failed");
        return false;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::QUERY_ERMS, "status : %{public}d", status);
        return false;
    }
    if ((status = env->Object_New(cls, method, &ruleObj)) != ANI_OK || ruleObj == nullptr) {
        TAG_LOGE(AAFwkTag::QUERY_ERMS, "status : %{public}d", status);
        return false;
    }
    if ((status = env->Object_SetPropertyByName_Boolean(ruleObj, "isOpenAllowed", rule.isOpenAllowed)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::QUERY_ERMS, "isOpenAllowed failed status:%{public}d", status);
        return false;
    }
    if ((status = env->Object_SetPropertyByName_Boolean(ruleObj, "isEmbeddedAllowed",
        rule.isEmbeddedAllowed)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::QUERY_ERMS, "isEmbeddedAllowed failed status:%{public}d", status);
        return false;
    }
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
