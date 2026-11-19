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

#include "ani_base_context.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
ani_status IsStageContext(ani_env* env, ani_object object, ani_boolean& stageMode)
{
    // arkTS 1.2 only supported stage mode
    stageMode = true;
    return ANI_OK;
}

std::shared_ptr<Context> GetStageModeContext(ani_env* env, ani_object object)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "env is nullptr");
        return nullptr;
    }
    ani_long nativeContextLong;
    ani_status status = env->Object_GetFieldByName_Long(object, "nativeContext", &nativeContextLong);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Object_GetField_Long failed, status: %{public}d", status);
        return nullptr;
    }
    if (nativeContextLong == 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "nativeContext is null");
        return nullptr;
    }
    auto weakContext = reinterpret_cast<std::weak_ptr<Context>*>(nativeContextLong);
    if (weakContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "weakContext is null");
        return nullptr;
    }
    return weakContext->lock();
}
} // namespace AbilityRuntime
} // namespace OHOS
