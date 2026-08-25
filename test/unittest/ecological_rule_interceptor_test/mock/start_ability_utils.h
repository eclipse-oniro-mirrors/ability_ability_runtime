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

#ifndef OHOS_ABILITY_RUNTIME_START_ABILITY_UTILS_H
#define OHOS_ABILITY_RUNTIME_START_ABILITY_UTILS_H

#include <memory>
#include <string>

#include "ability_info.h"
#include "extension_ability_info.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {

struct StartAbilityInfo {
    static std::shared_ptr<StartAbilityInfo> CreateStartAbilityInfo(const Want &want, int32_t userId,
        int32_t appIndex, sptr<IRemoteObject> callerToken);

    static std::shared_ptr<StartAbilityInfo> createStartAbilityInfo;
    int32_t status = ERR_OK;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ExtensionProcessMode extensionProcessMode = AppExecFwk::ExtensionProcessMode::UNDEFINED;
    std::string customProcess;
};

struct StartAbilityUtils {
    static bool GetCallerAbilityInfo(const sptr<IRemoteObject> &callerToken,
        AppExecFwk::AbilityInfo &abilityInfo);

    static std::shared_ptr<StartAbilityInfo> startAbilityInfo;
    static std::shared_ptr<StartAbilityInfo> callerAbilityInfo;
    static bool skipCrowTest;
    static bool skipErms;
    static int32_t ermsResultCode;
    static bool isWantWithAppCloneIndex;
    static bool ermsSupportBackToCallerFlag;
    static bool retGetCallerAbilityInfo;
    static AppExecFwk::AbilityInfo callerAbiltyInfo;
};
}
}
#endif // OHOS_ABILITY_RUNTIME_START_ABILITY_UTILS_H