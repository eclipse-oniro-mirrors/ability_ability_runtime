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

#ifndef OHOS_ABILITY_RUNTIME_PRELOAD_MANAGER_SERVICE_H
#define OHOS_ABILITY_RUNTIME_PRELOAD_MANAGER_SERVICE_H

#include <string>

#include "ability_info.h"
#include "singleton.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class PreloadManagerService
 * PreloadManagerService
 */
class PreloadManagerService {
public:
    /**
     * Get the instance of PreloadManagerService.
     *
     * @return Returns the instance of PreloadManagerService.
     */
    static PreloadManagerService &GetInstance();

    /**
     * Preload application.
     * @param bundleName Name of the application.
     * @param userId user id.
     * @param appIndex app clone index. Currently, only appIndex = 0 is supported.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t PreloadApplication(const std::string &bundleName, int32_t userId, int32_t appIndex);

    int32_t LaunchGameCustomized(const std::string &bundleName, int32_t userId, int32_t appIndex = 0);

    int32_t PreloadApplicationVerification(const std::string &bundleName, int32_t userId, int32_t appIndex, Want &want,
        AppExecFwk::AbilityInfo &abilityInfo);

private:
    PreloadManagerService();
    ~PreloadManagerService();

    DISALLOW_COPY_AND_MOVE(PreloadManagerService);
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_PRELOAD_MANAGER_SERVICE_H
