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

#ifndef OHOS_ABILITY_RUNTIME_DAEMON_SERVICE_INFO_H
#define OHOS_ABILITY_RUNTIME_DAEMON_SERVICE_INFO_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace AAFwk {
struct LaunchServiceDescriptor {
    std::string bundleName;
    std::string moduleName;
    std::string serviceName;
    int32_t userId = -1;
    int32_t appIndex = 0;

    static bool IsValidIdentity(const std::string &bundleName, const std::string &moduleName,
        const std::string &serviceName, int32_t userId, int32_t appIndex)
    {
        constexpr size_t maxFieldLength = 512;
        return !bundleName.empty() && bundleName.size() <= maxFieldLength && !moduleName.empty() &&
            moduleName.size() <= maxFieldLength && !serviceName.empty() && serviceName.size() <= maxFieldLength &&
            userId >= -1 && appIndex >= 0;
    }

    bool IsValid() const
    {
        return IsValidIdentity(bundleName, moduleName, serviceName, userId, appIndex);
    }
};
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_DAEMON_SERVICE_INFO_H
