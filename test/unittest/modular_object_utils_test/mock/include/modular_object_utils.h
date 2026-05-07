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

#ifndef OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_UTILS_H
#define OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_UTILS_H

#include <string>
#include <vector>

#include "ability_record/ability_request.h"
#include "iremote_object.h"
#include "modular_object_extension_info.h"

namespace OHOS {
namespace AAFwk {

class ModularObjectUtils {
public:
    ModularObjectUtils() = delete;

    static int32_t CheckPermission(const AbilityRequest &abilityRequest);
    static int32_t CheckRateLimit();
    static bool GetPidToCheckByCallerToken(sptr<IRemoteObject> callerToken, pid_t &outPid);

    static int32_t CheckExtensionEnabled(const ModularObjectExtensionInfo &info,
        const AbilityRequest &abilityRequest);
    static int32_t CheckCallerForeground();
    static int32_t CheckAppDistributionType(const std::string &callerAppDistributionType,
        const std::string &targetAppDistributionType);
    static bool HasRunningUIAbilityOrExtension(int32_t targetUid, int32_t userId);
    static int32_t CheckTargetHasRunningAbility(int32_t targetUid, int32_t userId,
        const std::string &targetBundleName);
    static int32_t GetTargetExtensionInfoFromDb(const std::string &bundleName,
        const std::string &abilityName, int32_t appIndex, int32_t validUserId,
        ModularObjectExtensionInfo &targetExtensionInfo);
    static int32_t GetCallerAppInfo(AppExecFwk::ApplicationInfo &callerAppInfo);
    static int32_t VerifyExported(const AbilityRequest &abilityRequest);
};

}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_UTILS_H
