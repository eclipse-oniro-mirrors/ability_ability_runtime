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

#include "ets_insight_intent_execution_info.h"

#include "ani_common_util.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *EXECUTION_INFO_CLASS_NAME =
    "@ohos.app.ability.insightIntent.insightIntent.InsightIntentExecutionInfoInner";
}

ani_object CreateEtsInsightIntentExecutionInfo(ani_env *env,
    const AppExecFwk::InsightIntentExecutionInfo &info)
{
    ani_class cls = nullptr;
    if (env->FindClass(EXECUTION_INFO_CLASS_NAME, &cls) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "find execution info class failed");
        return nullptr;
    }
    ani_method ctor = nullptr;
    if (env->Class_FindMethod(cls, "<ctor>", ":", &ctor) != ANI_OK || ctor == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "find execution info ctor failed");
        return nullptr;
    }
    ani_object obj = nullptr;
    if (env->Object_New(cls, ctor, &obj) != ANI_OK || obj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "new execution info failed");
        return nullptr;
    }
    env->Object_SetPropertyByName_Ref(obj, "intentName", AppExecFwk::GetAniString(env, info.intentName));
    env->Object_SetPropertyByName_Ref(obj, "callerBundleName",
        AppExecFwk::GetAniString(env, info.callerBundleName));
    env->Object_SetPropertyByName_Ref(obj, "callerTokenId", AppExecFwk::GetAniString(env, info.callerTokenId));
    env->Object_SetPropertyByName_Ref(obj, "callerUid", AppExecFwk::CreateInt(env, info.callerUid));
    return obj;
}
} // namespace AbilityRuntime
} // namespace OHOS