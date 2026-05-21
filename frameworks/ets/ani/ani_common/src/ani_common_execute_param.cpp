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

#include "ani_common_execute_param.h"

#include "hilog_tag_wrapper.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "ani_enum_convert.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;

bool UnwrapExecuteParam(ani_env *env, ani_object param, AppExecFwk::InsightIntentExecuteParam &executeParam)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return false;
    }
    if (param == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null param");
        return false;
    }

    std::string bundleName {""};
    if (!GetStringProperty(env, param, "bundleName", bundleName)) {
        TAG_LOGE(AAFwkTag::INTENT, "Wrong argument type bundleName");
        return false;
    }
    executeParam.bundleName_ = bundleName;

    std::string moduleName {""};
    if (!GetStringProperty(env, param, "moduleName", moduleName)) {
        TAG_LOGE(AAFwkTag::INTENT, "Wrong argument type moduleName");
        return false;
    }
    executeParam.moduleName_ = moduleName;

    std::string abilityName {""};
    if (!GetStringProperty(env, param, "abilityName", abilityName)) {
        TAG_LOGE(AAFwkTag::INTENT, "Wrong argument type abilityName");
        return false;
    }
    executeParam.abilityName_ = abilityName;

    std::string insightIntentName {""};
    if (!GetStringProperty(env, param, "insightIntentName", insightIntentName)) {
        TAG_LOGE(AAFwkTag::INTENT, "Wrong argument type insightIntentName");
        return false;
    }
    executeParam.insightIntentName_ = insightIntentName;

    if (IsExistsProperty(env, param, "deviceId")) {
        std::string deviceId {""};
        if (!GetStringProperty(env, param, "deviceId", deviceId)) {
            TAG_LOGE(AAFwkTag::INTENT, "Wrong argument type deviceId");
            return false;
        }
        executeParam.deviceId_ = deviceId;
    }

    ani_ref aniIntentParam = nullptr;
    if (!GetRefProperty(env, param, "insightIntentParam", aniIntentParam)) {
        TAG_LOGE(AAFwkTag::INTENT, "null aniIntentParam");
        return false;
    }
    auto wp = std::make_shared<WantParams>();
    if (!UnwrapWantParams(env, aniIntentParam, *wp)) {
        TAG_LOGE(AAFwkTag::INTENT, "unwrap want fail");
        return false;
    }
    executeParam.insightIntentParam_ = wp;

    int32_t executeMode = 0;
    ani_ref executeModeRef = nullptr;
    if (!GetRefProperty(env, param, "executeMode", executeModeRef) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "Wrong argument type executeMode");
        return false;
    }
    AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env,
        static_cast<ani_enum_item>(executeModeRef), executeMode);
    executeParam.executeMode_ = executeMode;

    ani_long displayIdD = 0;
    int32_t displayId = INVALID_DISPLAY_ID;
    if (executeMode == ExecuteMode::UI_ABILITY_FOREGROUND &&
        IsExistsProperty(env, param, "displayId")) {
        if (GetLongPropertyObject(env, param, "displayId", displayIdD)) {
            displayId = static_cast<int32_t>(displayIdD);
            if (displayId < 0) {
                TAG_LOGE(AAFwkTag::INTENT, "invalid displayId");
                return false;
            }
            TAG_LOGI(AAFwkTag::INTENT, "displayId %{public}d", displayId);
            executeParam.displayId_ = displayId;
        }
    }
    if (IsExistsProperty(env, param, "uris")) {
        std::vector<std::string> uris;
        if (!GetStringArrayProperty(env, param, "uris", uris)) {
            TAG_LOGE(AAFwkTag::INTENT, "Wrong argument uris fail");
            return false;
        }
        executeParam.uris_ = uris;
    }
    if (IsExistsProperty(env, param, "flags")) {
        ani_int flags = 0;
        if (!GetIntPropertyObject(env, param, "flags", flags)) {
            TAG_LOGE(AAFwkTag::INTENT, "Wrong argument flags fail");
            return false;
        }
        executeParam.flags_ = flags;
    }
    if (IsExistsProperty(env, param, "userId")) {
        ani_int userId = 0;
        if (!GetIntPropertyObject(env, param, "userId", userId)) {
            TAG_LOGE(AAFwkTag::INTENT, "Wrong argument userId fail");
            return false;
        }
        executeParam.userId_ = userId;
    }

    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
