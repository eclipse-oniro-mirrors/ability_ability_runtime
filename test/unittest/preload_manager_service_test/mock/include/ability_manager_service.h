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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_SERVICE_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_SERVICE_H

#include <memory>
#include <singleton.h>

#include "start_ability_utils.h"
#include "start_options.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class AbilityManagerService
 * AbilityManagerService provides a facility for managing ability life cycle.
 */
class AbilityManagerService : public std::enable_shared_from_this<AbilityManagerService> {
    DECLARE_DELAYED_SINGLETON(AbilityManagerService)
public:
    int32_t GetValidUserId(int32_t userId);

    bool JudgeMultiUserConcurrency(int32_t userId);

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
    int32_t StartAbility(const Want &want, const StartOptions &startOptions,
        sptr<IRemoteObject> callerToken, int32_t userId = -1, int requestCode = -1);
    int StartAbilityInner(StartAbilityWrapParam &param);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_SERVICE_H
