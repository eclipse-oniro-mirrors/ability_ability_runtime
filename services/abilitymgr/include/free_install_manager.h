/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_FREE_INSTALL_MANAGER_H
#define OHOS_ABILITY_RUNTIME_FREE_INSTALL_MANAGER_H

#include <future>
#include "cpp/mutex.h"

#include <iremote_object.h>
#include <iremote_stub.h>
#include <memory>

#include "ability_info.h"
#include "free_install_observer_manager.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class AbilityManagerService;

struct FreeInstallInfo {
    Want want;
    int32_t userId = -1;
    int32_t requestCode = -1;
    std::shared_ptr<std::promise<int32_t>> promise;
    bool isInstalled = false;
    std::string identity;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IRemoteObject> dmsCallback = nullptr;
    bool isPreStartMissionCalled = false;
    bool isStartUIAbilityBySCBCalled = false;
    uint32_t specifyTokenId = 0;
    bool isFreeInstallFinished = false;
    int resultCode = 0;
    bool isOpenAtomicServiceShortUrl = false;
    std::shared_ptr<Want> originalWant = nullptr;
};

/**
 * @class FreeInstallManager
 * FreeInstallManager.
 */
class FreeInstallManager : public std::enable_shared_from_this<FreeInstallManager> {
public:
    explicit FreeInstallManager(const std::weak_ptr<AbilityManagerService> &server);
    virtual ~FreeInstallManager() = default;

    /**
     * OnInstallFinished, StartFreeInstall is complete.
     *
     * @param resultCode, ERR_OK on success, others on failure.
     * @param want, installed ability.
     * @param userId, user`s id.
     */
    void OnInstallFinished(int32_t recordId, int resultCode, const Want &want, int32_t userId, bool isAsync = false);

    /**
     * OnRemoteInstallFinished, DMS has finished.
     *
     * @param resultCode, ERR_OK on success, others on failure.
     * @param want, installed ability.
     * @param userId, user`s id.
     */
    void OnRemoteInstallFinished(int32_t recordId, int resultCode, const Want &want, int32_t userId);

    /**
     * Start to free install.
     *
     * @param want, the want of the ability to free install.
     * @param userId, designation User ID.
     * @param requestCode, ability request code.
     * @param callerToken, caller ability token.
     * @param isAsync, the request is async.
     * @param isOpenAtomicServiceShortUrl, the flag of open atomic service short url.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StartFreeInstall(const Want &want, int32_t userId, int requestCode, const sptr<IRemoteObject> &callerToken,
        bool isAsync = false, uint32_t specifyTokenId = 0, bool isOpenAtomicServiceShortUrl = false,
        std::shared_ptr<Want> originalWant = nullptr);

    /**
     * Start to remote free install.
     *
     * @param want, the want of the ability to free install.
     * @param requestCode, ability request code.
     * @param validUserId, designation User ID.
     * @param callerToken, caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StartRemoteFreeInstall(const Want &want, int requestCode, int32_t validUserId,
        const sptr<IRemoteObject> &callerToken);

    /**
     * Start to free install from another devices.
     * The request is send from DMS.
     *
     * @param want, the want of the ability to free install.
     * @param callback, used to notify caller the result of free install.
     * @param userId, designation User ID.
     * @param requestCode, ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    int FreeInstallAbilityFromRemote(const Want &want, const sptr<IRemoteObject> &callback,
        int32_t userId, int requestCode);

    /**
     * Connect if the request is free install.
     * @param want, the want of the ability to free install.
     * @param userId, designation User ID.
     * @param callerToken, caller ability token.
     * @param localDeviceId, the device id of local.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ConnectFreeInstall(const Want &want, int32_t userId, const sptr<IRemoteObject> &callerToken,
        const std::string& localDeviceId);

    /**
     * Add an observer from application into freeInstallObserverManager.
     * @param observer, the observer of the ability to free install.
     * @return Returns ERR_OK on success, others on failure.
     */
    int AddFreeInstallObserver(const sptr<IRemoteObject> &callerToken,
        const sptr<AbilityRuntime::IFreeInstallObserver> &observer);

    /**
     * Get free install task info.
     *
     * @param bundleName, the bundle name of the task.
     * @param abilityName, the ability name of the task.
     * @param startTime, the start time of the task.
     * @param taskInfo, the found task info
     * @return Returns true on success, false on failure.
    */
    bool GetFreeInstallTaskInfo(const std::string& bundleName, const std::string& abilityName,
        const std::string& startTime, FreeInstallInfo& taskInfo);

    /**
     * Get free install task info.
     *
     * @param sessionId, the sessionId of the task.
     * @param taskInfo, the found task info
     * @return Returns true on success, false on failure.
    */
    bool GetFreeInstallTaskInfo(const std::string& sessionId, FreeInstallInfo& taskInfo);

    /**
     * Set the isStartUIAbilityBySCBCalled flag of the given free install task.
     *
     * @param bundleName, the bundle name of the task.
     * @param abilityName, the abilitu name of the task.
     * @param startTime, the start time of the task.
     * @param scbCallStatus, the status of whether StartUIAbilityBySCB is called.
    */
    void SetSCBCallStatus(const std::string& bundleName, const std::string& abilityName,
        const std::string& startTime, bool scbCallStatus);

    /**
     * Set the isPreStartMissionCalled flag of the given free install task.
     *
     * @param bundleName, the bundle name of the task.
     * @param abilityName, the abilitu name of the task.
     * @param startTime, the start time of the task.
     * @param preStartMissionCallStatus, the status of whether PreStartMission is called.
    */
    void SetPreStartMissionCallStatus(const std::string& bundleName, const std::string& abilityName,
        const std::string& startTime, bool preStartMissionCallStatus);

    /**
     * Set the sessionId of the given free install task.
     *
     * @param bundleName, the bundle name of the task.
     * @param abilityName, the abilitu name of the task.
     * @param startTime, the start time of the task.
     * @param sessionId, the sessionId of the free install task.
    */
    void SetFreeInstallTaskSessionId(const std::string& bundleName, const std::string& abilityName,
        const std::string& startTime, const std::string& sessionId);

private:
    std::weak_ptr<AbilityManagerService> server_;
    std::vector<FreeInstallInfo> freeInstallList_;
    std::vector<FreeInstallInfo> dmsFreeInstallCbs_;
    std::map<std::string, std::time_t> timeStampMap_;
    ffrt::mutex distributedFreeInstallLock_;
    ffrt::mutex freeInstallListLock_;
    ffrt::mutex freeInstallObserverLock_;

    int SetAppRunningState(Want &want);

    /**
     * Start remote free install.
     *
     * @param want, the want of the ability to remote free install.
     * @param userId, designation User ID.
     * @param requestCode, ability request code.
     * @param callerToken, caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int RemoteFreeInstall(const Want &want, int32_t userId, int requestCode, const sptr<IRemoteObject> &callerToken);

    int NotifyDmsCallback(const Want &want, int resultCode);
    bool IsTopAbility(const sptr<IRemoteObject> &callerToken);
    void NotifyFreeInstallResult(int32_t recordId, const Want &want, int resultCode, bool isAsync = false);
    FreeInstallInfo BuildFreeInstallInfo(const Want &want, int32_t userId, int requestCode,
        const sptr<IRemoteObject> &callerToken, bool isAsync, uint32_t specifyTokenId = 0,
        bool isOpenAtomicServiceShortUrl = false, std::shared_ptr<Want> originalWant = nullptr);
    std::time_t GetTimeStamp();

    void RemoveFreeInstallInfo(const std::string &bundleName, const std::string &abilityName,
        const std::string &startTime);

    void PostUpgradeAtomicServiceTask(int resultCode, const Want &want, int32_t userId);

    void RemoveTimeoutTask(const std::string &bundleName, const std::string &abilityName, const std::string &startTime);

    void StartAbilityByFreeInstall(FreeInstallInfo &info, std::string &bundleName, std::string &abilityName,
        std::string &startTime);
    void StartAbilityByPreInstall(int32_t recordId, FreeInstallInfo &info, std::string &bundleName,
        std::string &abilityName, std::string &startTime);
    int32_t UpdateElementName(Want &want, int32_t userId) const;
    void HandleFreeInstallResult(int32_t recordId, FreeInstallInfo &freeInstallInfo, int resultCode, bool isAsync);
    void HandleOnFreeInstallSuccess(int32_t recordId, FreeInstallInfo &freeInstallInfo, bool isAsync);
    void HandleOnFreeInstallFail(int32_t recordId, FreeInstallInfo &freeInstallInfo, int resultCode, bool isAsync);
    void NotifySCBToHandleException(const FreeInstallInfo &info, int resultCode);
    void StartAbilityByConvertedWant(FreeInstallInfo &info, const std::string &startTime);
    void StartAbilityByOriginalWant(FreeInstallInfo &info, const std::string &startTime);
    bool VerifyStartFreeInstallPermission(const sptr<IRemoteObject> &callerToken);
    int32_t GetRecordIdByToken(const sptr<IRemoteObject> &callerToken);
    void NotifyInsightIntentFreeInstallResult(const Want &want, int resultCode);
    void NotifyInsightIntentExecuteDone(const Want &want, int resultCode);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_FREE_INSTALL_MANAGER_H
