/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_EXIT_REASON_HELPER
#define OHOS_ABILITY_RUNTIME_APP_EXIT_REASON_HELPER

#include <memory>
#include <mutex>

#include "bundle_info.h"
#include "exit_reason.h"
#include "sub_managers_helper.h"

namespace OHOS {
namespace AAFwk {
class AppExitReasonHelper {
public:
    explicit AppExitReasonHelper(std::shared_ptr<SubManagersHelper> subManagersHelper);
    ~AppExitReasonHelper() = default;

    int32_t RecordAppExitReason(const ExitReason &exitReason);
    int32_t RecordAppExitReason(const std::string &bundleName, int32_t uid, int32_t appIndex,
        const ExitReason &exitReason);
    int32_t RecordProcessExtensionExitReason(
        const int32_t pid, const std::string &bundleName, const ExitReason &exitReason,
        const AppExecFwk::RunningProcessInfo &processInfo, bool withKillMsg);
    int32_t RecordProcessExitReason(const int32_t pid, const ExitReason &exitReason, bool fromKillWithReason);
    int32_t RecordProcessExitReason(int32_t pid, int32_t uid, const ExitReason &exitReason);
    int32_t RecordUIAbilityExitReason(const pid_t pid, const std::string &abilityName, const ExitReason &exitReason);

private:
    int32_t RecordProcessExitReason(const int32_t pid, const std::string bundleName, const int32_t uid,
        const uint32_t accessTokenId, const ExitReason &exitReason,
        const AppExecFwk::RunningProcessInfo &processInfo, bool fromKillWithReason, bool searchDead);
    void GetActiveAbilityList(int32_t uid, std::vector<std::string> &abilityLists, const int32_t pid);
    int32_t GetActiveAbilityList(int32_t uid, std::vector<std::string> &abilityLists);
    void GetActiveAbilityListFromUIAbilityManager(int32_t uid, std::vector<std::string> &abilityLists,
        const int32_t pid);
    bool IsExitReasonValid(const ExitReason &exitReason);
    int32_t GetActiveAbilityListWithPid(int32_t uid, std::vector<std::string> &abilityList, int32_t pid);

    std::shared_ptr<SubManagersHelper> subManagersHelper_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_EXIT_REASON_HELPER
