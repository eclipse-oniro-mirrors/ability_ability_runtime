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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_PERMISSION_UTIL_H
#define OHOS_ABILITY_RUNTIME_ABILITY_PERMISSION_UTIL_H

#include <memory>

#include "ability_record.h"
#include "ffrt.h"
#include "iremote_object.h"
#include "nocopyable.h"
#include "permission_verification.h"

namespace OHOS {
namespace AppExecFwk {
struct RunningProcessInfo;
}
namespace AAFwk {
struct AbilityRequest;

struct access_token_hap_query_op {
    uint32_t tokenId;
    uint64_t hapTokenId;
};

#define ACCESS_TOKENID_IOCTL_BASE 'A'
#define GET_CLOSEST_HAP_TOKENID 9
#define ACCESS_TOKENID_GET_CLOSEST_HAP_TOKENID \
    _IOW(ACCESS_TOKENID_IOCTL_BASE, GET_CLOSEST_HAP_TOKENID, struct access_token_hap_query_op)

/**
 * @class Want
 * the struct to open abilities.
 */
class Want;

class StartSelfUIAbilityRecordGuard {
public:
    StartSelfUIAbilityRecordGuard() = delete;

    StartSelfUIAbilityRecordGuard(pid_t pid, int32_t tokenId);

    ~StartSelfUIAbilityRecordGuard();

private:
    pid_t pid_;
};

/**
 * @class AbilityPermissionUtil
 * provides ability permission utilities.
 */
class AbilityPermissionUtil {
public:
    /**
     * GetInstance, get an instance of AbilityPermissionUtil.
     *
     * @return An instance of AbilityPermissionUtil.
     */
    static AbilityPermissionUtil &GetInstance();

    /**
     * IsDelegatorCall, check caller is delegator.
     *
     * @param processInfo The process information.
     * @param abilityRequest The ability request.
     * @return Whether the caller is delegator.
     */
    bool IsDelegatorCall(const AppExecFwk::RunningProcessInfo &processInfo, const AbilityRequest &abilityRequest) const;

    int32_t GetClosestHapTokenId(uint32_t tokenId, uint32_t &hapTokenId);

    /**
     * IsDominateScreen, check dominate screen.
     *
     * @param want The want.
     * @param isPendingWantCaller Flag of whether it is the pending want caller.
     * @return Whether it is dominate screen.
     */
    bool IsDominateScreen(const Want &want, bool isPendingWantCaller);

    /**
     * CheckMultiInstanceAndAppClone, check if the app is either multi-instance or app-clone.
     *
     * @param want The want.
     * @param userId The user id.
     * @param appIndex The app index.
     * @param callerToken The caller token.
     * @param isScbCall The scb call.
     * @return Whether the app is either multi-instance or app-clone.
     */
    int32_t CheckMultiInstanceAndAppClone(Want &want, int32_t userId, int32_t appIndex,
        sptr<IRemoteObject> callerToken, bool isScbCall);

    /**
     * CheckMultiInstanceKeyForExtension, check multi-instance key for extension.
     *
     * @param abilityRequest The ability request.
     * @return Whether the key is multi-instance key.
     */
    int32_t CheckMultiInstanceKeyForExtension(const AbilityRequest &abilityRequest);

    int32_t CheckStartRecentAbility(const Want &want, AbilityRequest &request);

    /**
     * Check HasFloatingWindow.
     * @param callerToken The caller token.
     * @return Whether the caller has floatingWindow.
     */
    int32_t CheckStartCallHasFloatingWindow(const sptr<IRemoteObject> &callerToken);

    bool IsStartSelfUIAbility();

    int32_t CheckPrepareTerminateEnable(std::shared_ptr<AbilityRecord> abilityRecord);

    bool NeedCheckStatusBar(std::shared_ptr<AbilityRecord> abilityRecord, const AbilityRequest &abilityRequest);

    /**
     * CheckStartUIAbilityByUserLockStatus, check if UIAbility start is allowed based on user lock status.
     * When status is locked, only system apps are allowed to start.
     *
     * @param bundleName The bundle name of the target application.
     * @return Whether the ability start is allowed (true) or should be blocked (false).
     */
    bool CheckStartUIAbilityByUserLockStatus(const std::string &bundleName);

private:
    /**
     * AbilityPermissionUtil, the private constructor.
     *
     */
    AbilityPermissionUtil() = default;

    /**
     * AbilityPermissionUtil, the private destructor.
     *
     */
    ~AbilityPermissionUtil() = default;

    std::optional<int32_t> GetAccessTokenIdFd();

    /**
     * CheckMultiInstance, check multi-instance.
     *
     * @param want The want.
     * @param callerToken The caller token.
     * @param maxCount The max number of instances.
     * @param isScbCall The scb call.
     * @return Whether it is a valid multi-instance instance.
     */
    int32_t CheckMultiInstance(Want &want, sptr<IRemoteObject> callerToken, int32_t maxCount, bool isScbCall);

    /**
     * UpdateInstanceKey, update instance key.
     *
     * @param want The want.
     * @param originInstanceKey The original instance key.
     * @param instanceKeyArray Candidate instance keys.
     * @param instanceKey The new instance key.
     * @return Whether the update is successful.
     */
    int32_t UpdateInstanceKey(Want &want, const std::string &originInstanceKey,
        const std::vector<std::string> &instanceKeyArray, const std::string &instanceKey);

    void AddStartSelfUIAbilityRecord(pid_t pid, int32_t tokenId);

    void RemoveStartSelfUIAbilityRecord(pid_t pid);

    int GetTokenIdByPid(pid_t pid);

    std::vector<std::vector<int32_t>> startSelfUIAbilityRecords_;
    ffrt::mutex startSelfUIAbilityRecordsMutex_;

    friend class StartSelfUIAbilityRecordGuard;

    DISALLOW_COPY_AND_MOVE(AbilityPermissionUtil);
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_PERMISSION_UTIL_H