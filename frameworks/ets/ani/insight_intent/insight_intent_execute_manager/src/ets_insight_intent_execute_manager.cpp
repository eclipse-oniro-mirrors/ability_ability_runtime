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

#include "ets_insight_intent_execute_manager.h"

#include <array>
#include <string>

#include "ability_business_error.h"
#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "ani_common_util.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
constexpr const char *INSIGHT_INTENT_EXECUTE_MANAGER_CLASS_NAME =
    "@ohos.app.ability.insightIntentExecuteManager.insightIntentExecuteManager";
}

class EtsInsightIntentExecuteManager {
public:
    EtsInsightIntentExecuteManager() = default;
    ~EtsInsightIntentExecuteManager() = default;

    static void OnGetInsightIntentCallerBundleName(ani_env *env, ani_int instanceId, ani_object callback)
    {
        TAG_LOGD(AAFwkTag::INTENT, "OnGetInsightIntentCallerBundleName called");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null env");
            return;
        }
        auto amsClient = AAFwk::AbilityManagerClient::GetInstance();
        if (amsClient == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null amsClient");
            AppExecFwk::AsyncCallback(env, callback,
                EtsErrorUtil::CreateErrorByNativeErr(env, AAFwk::INNER_ERR), nullptr);
            return;
        }
        std::string callerBundleName;
        int32_t errCode = amsClient->GetInsightIntentCallerBundleName(
            static_cast<int32_t>(instanceId), callerBundleName);
        if (errCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "get caller bundle name failed: %{public}d", errCode);
            AppExecFwk::AsyncCallback(env, callback,
                EtsErrorUtil::CreateErrorByNativeErr(env, errCode), nullptr);
            return;
        }
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),
            reinterpret_cast<ani_object>(AppExecFwk::GetAniString(env, callerBundleName)));
    }
};

void EtsInsightIntentExecuteManagerInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::INTENT, "EtsInsightIntentExecuteManagerInit called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return;
    }

    ani_namespace ns;
    ani_status status = env->FindNamespace(INSIGHT_INTENT_EXECUTE_MANAGER_CLASS_NAME, &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "FindNamespace insightIntentExecuteManager failed status: %{public}d", status);
        return;
    }

    std::array nativeFunctions = {
        ani_native_function { "nativeGetInsightIntentCallerBundleName", nullptr,
            reinterpret_cast<void *>(EtsInsightIntentExecuteManager::OnGetInsightIntentCallerBundleName) },
    };

    status = env->Namespace_BindNativeFunctions(ns, nativeFunctions.data(), nativeFunctions.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "bind native function failed status: %{public}d", status);
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

    EtsInsightIntentExecuteManagerInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::INTENT, "ANI_Constructor finish");
    return ANI_OK;
}
}
} // namespace AbilityRuntime
} // namespace OHOS