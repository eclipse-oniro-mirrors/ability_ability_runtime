/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_SERVICE_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_SERVICE_H

#include <cstdint>
#include <memory>
#include <singleton.h>

#include "ability_keep_alive_service.h"
#include "ability_manager_constants.h"
#include "keep_alive_info.h"
#include "start_options.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
using KeepAliveInfo = AbilityRuntime::KeepAliveInfo;
using KeepAliveSetter = AbilityRuntime::KeepAliveSetter;
using KeepAliveAppType = AbilityRuntime::KeepAliveAppType;
using KeepAliveStatus = AbilityRuntime::KeepAliveStatus;
using KeepAlivePolicy = AbilityRuntime::KeepAlivePolicy;

constexpr int32_t BASE_USER_RANGE = 200000;
const int DEFAULT_INVAL_VALUE = -1;

/**
 * @class AbilityManagerService
 * AbilityManagerService provides a facility for managing ability life cycle.
 */
class AbilityManagerService : public std::enable_shared_from_this<AbilityManagerService> {
    DECLARE_DELAYED_SINGLETON(AbilityManagerService)
public:
    bool IsInStatusBar(uint32_t accessTokenId, int32_t uid, bool isMultiInstance);

    bool IsSupportStatusBar(int32_t uid);

    bool IsSceneBoardReady(int32_t userId);

    /**
     * get the user id.
     *
     */
    int32_t GetUserId() const;

    /**
     * Starts a new ability with specific start options.
     *
     * @param want the want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @param callerToken caller ability token.
     * @param userId Designation User ID.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t StartAbility(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = -1,
        int requestCode = -1);
    
    /**
     * Start extension ability with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @param extensionType If an ExtensionAbilityType is set, only extension of that type can be started.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t StartExtensionAbility(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED);

public:
    static bool isInStatusBarResult;
    static bool isSupportStatusBarResult;
    static bool isSceneBoardReadyResult;
    static int32_t userId_;
    static int32_t startAbilityResult;
    static int32_t startExtensionAbilityResult;
    static int32_t usedSupportStatusBarTimes;
    static int32_t usedStartAbilityTimes;
    static int32_t usedIsInStatusBar;
    static int32_t usedStartExtensionAbilityTimes;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_SERVICE_H
