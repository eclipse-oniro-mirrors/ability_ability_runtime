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

#ifndef OHOS_ABILITY_RUNTIME_RES_SCHED_UTIL_H
#define OHOS_ABILITY_RUNTIME_RES_SCHED_UTIL_H

#include <gmock/gmock.h>

#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {

class ResSchedUtilBase {
public:
    ResSchedUtilBase() = default;
    virtual ~ResSchedUtilBase() = default;
    DISALLOW_COPY_AND_MOVE(ResSchedUtilBase);

    virtual void ReportUIExtensionProcColdStartToRss(
        int32_t extensionAbilityType, int hostPid, const std::string& hostBundleName, const std::string& bundleName,
        const std::string& abilityName, const std::string& moduleName, bool isPreloadUIExtension) = 0;
    virtual void ReportTemplateProcessReadyToRSS(int32_t pid, int32_t uid, const std::string &bundleName) = 0;
};

class ResSchedUtil : public ResSchedUtilBase {
public:
    ResSchedUtil() = default;
    virtual ~ResSchedUtil() = default;
    DISALLOW_COPY_AND_MOVE(ResSchedUtil);

    static ResSchedUtil &GetInstance();

    MOCK_METHOD(void, ReportUIExtensionProcColdStartToRss,
        (int32_t extensionAbilityType, int hostPid, const std::string& hostBundleName, const std::string& bundleName,
        const std::string& abilityName, const std::string& moduleName, bool isPreloadUIExtension), ());
    MOCK_METHOD(void, ReportTemplateProcessReadyToRSS, (int32_t pid, int32_t uid, const std::string &bundleName), ());
};

} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_RES_SCHED_UTIL_H
