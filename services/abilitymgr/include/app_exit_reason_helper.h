/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
struct RecordExitReasonParams {
    int32_t pid = 0;
    std::string bundleName = "";
    int32_t uid = 0;
    uint32_t accessTokenId = 0;
    ExitReasonCompability exitReason;
    AppExecFwk::RunningProcessInfo processInfo;
    bool fromKillWithReason = false;
    int32_t callerPid = DEFAULT_INVAL_VALUE;
};

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
    int32_t RecordProcessExitReason(const int32_t pid, const ExitReason &exitReason, bool fromKillWithReason,
        int32_t callerPid = DEFAULT_INVAL_VALUE);
    int32_t RecordAppKillId(const std::string &bundleName, int32_t uid, int32_t pid,
        uint32_t accessTokenId, int32_t killId);
    int32_t RecordProcessExitReason(int32_t pid, int32_t uid, const ExitReason &exitReason);
    int32_t RecordUIAbilityExitReason(const pid_t pid, const std::string &abilityName, const ExitReason &exitReason,
        int32_t callerPid = DEFAULT_INVAL_VALUE, bool isLastAbilityOfProcess = true);
    int32_t RecordKillCallerInfo(const pid_t pid, int32_t callerPid);
    int32_t RecordProcessExitReasonForTimeout(const AppExecFwk::AbilityInfo &abilityInfo, const ExitReason &exitReason,
        const std::vector<std::string> &abilityList, const AppExecFwk::RunningProcessInfo &processInfo);
    int32_t AddAppExitReason(const std::string &bundleName, int32_t pid, int32_t uid, int32_t appIndex,
        const ExitReasonCompability &exitReason, int32_t callerPid);
    int32_t AddBundleExitReason(const std::string &bundleName, int32_t userId, int32_t appIndex,
        const ExitReasonCompability &exitReason, int32_t callerPid);
    int32_t RecordAppWithReason(int32_t pid, int32_t uid, const ExitReasonCompability &exitReason, int32_t callerPid);
    void RecordAppsWithReasonByProcessInfoList(const ExitReasonCompability &exitReason,
        const std::vector<AppExecFwk::RunningProcessInfo> &processInfoList);
    void RecordInvalidKillId(int32_t pid, const ExitReasonCompability &params,
        const std::string &bundleName = "", int32_t userId = 0, int32_t callerPid = DEFAULT_INVAL_VALUE);

    /**
     * @brief Report the exit reason of THIS force stop event for every matched process and stamp
     *        kill id/caller info right before the kill, best effort: all failures are logged and
     *        never block the kill. The report scope aligns with the kill scope: bundle + resolved
     *        appIndex (per-pid reverse lookup, covering all multi-instance forms) + users
     *        (originUserId -1 means all users, enumerated per active os account).
     * @param bundleName bundle name of the application to report.
     * @param appIndex resolved application index of the kill scope.
     * @param userId the resolved user id of the caller.
     * @param originUserId the original user id, -1 means the kill crosses all users.
     * @param recordPid force stop stamping scope, >0 means only the specified pid.
     * @param exitReason the declared exit reason, normalized by the caller.
     * @param callerPid the pid of the force stop caller, captured at the transaction boundary.
     */
    void ReportForceStopKillId(const std::string &bundleName, int32_t appIndex, int32_t userId,
        int32_t originUserId, int32_t recordPid, int32_t exitReason, int32_t callerPid);

    /**
     * @brief Enumerate the running process records of the force stop kill scope: per active os
     *        account when originUserId is -1 (the kill crosses all users), the resolved user
     *        otherwise. An empty result means nothing to report, never an error.
     * @param bundleName bundle name of the application to report.
     * @param userId the resolved user id of the caller.
     * @param originUserId the original user id, -1 means the kill crosses all users.
     * @return The running process infos of the kill scope.
     */
    static std::vector<AppExecFwk::RunningProcessInfo> CollectForceStopProcessInfos(
        const std::string &bundleName, int32_t userId, int32_t originUserId);

    /**
     * @brief Report the exit reason of one force stop event for one candidate process and stamp
     *        kill id/caller info, best effort: all failures are logged and never block the kill.
     * @param pid pid of the candidate process record.
     * @param bundleName bundle name of the application to report.
     * @param appIndex resolved application index of the kill scope.
     * @param recordPid force stop stamping scope, >0 means only the specified pid.
     * @param exitReason the declared exit reason, normalized by the caller.
     * @param callerPid the pid of the force stop caller, captured at the transaction boundary.
     */
    void ReportForceStopKillIdForPid(int32_t pid, const std::string &bundleName, int32_t appIndex,
        int32_t recordPid, int32_t exitReason, int32_t callerPid, bool persistKV);

private:
    int32_t RecordProcessExitReason(const int32_t pid, const std::string bundleName, const int32_t uid,
        const uint32_t accessTokenId, const ExitReason &exitReason,
        const AppExecFwk::RunningProcessInfo &processInfo, bool fromKillWithReason,
        int32_t callerPid = DEFAULT_INVAL_VALUE);
    void GetActiveAbilityList(int32_t uid, std::vector<std::string> &abilityLists, const int32_t pid);
    void GetActiveAbilityListFromUIAbilityManager(int32_t uid, std::vector<std::string> &abilityLists,
        const int32_t pid);
    bool IsExitReasonValid(const ExitReason &exitReason);
    int32_t GetActiveAbilityListWithPid(int32_t uid, std::vector<std::string> &abilityList, int32_t pid);
    void GetRunningProcessInfo(int32_t pid, int32_t userId, const std::string &bundleName,
        AppExecFwk::RunningProcessInfo &processInfo, int32_t appIndex = DEFAULT_INVAL_VALUE);
    int32_t AddProcessExitReason(const RecordExitReasonParams &params);
    std::vector<AppExecFwk::RunningProcessInfo> GetRunningProcessInfos(int32_t userId, const std::string &bundleName);

    int32_t RecordAppWithReasonInner(const ExitReasonCompability &exitReasonCompability,
        const AppExecFwk::RunningProcessInfo &processInfo, int32_t callerPid);

    std::shared_ptr<SubManagersHelper> subManagersHelper_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_EXIT_REASON_HELPER
