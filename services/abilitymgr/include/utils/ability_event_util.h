/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_EVENT_UTIL_H
#define OHOS_ABILITY_RUNTIME_ABILITY_EVENT_UTIL_H

#include <string>

#include "application_info.h"
#include "bundle_pack_info.h"
#include "event_report.h"
#include "ffrt.h"

namespace OHOS {
namespace AAFwk {

class AbilityEventUtil {
public:
    static void HandleModuleInfoUpdated(const std::string &bundleName, const int uid, const std::string &moduleName,
        bool isPlugin);
    static void SendStartAbilityErrorEvent(EventInfo &eventInfo, int32_t errCode, const std::string errMsg,
        bool isSystemError = false);
    static void SendKillProcessWithReasonEvent(int32_t errCode, const std::string &errMsg, EventInfo &eventInfo);

    /**
     * @brief Handle bundle first launch event publishing and status setting.
     * @param appInfo The application info containing bundle information.
     * @param userId The user ID.
     * @param callerBundleName The caller bundle name (optional, empty string if none).
     * @return Returns true if the event was published successfully; returns false otherwise.
     */
    static bool HandleBundleFirstLaunch(const AppExecFwk::ApplicationInfo &appInfo, int32_t userId,
        const std::string &callerBundleName = "");
};

} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_EVENT_UTIL_H
