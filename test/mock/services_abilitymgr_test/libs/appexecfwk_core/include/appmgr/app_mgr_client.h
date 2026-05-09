/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_MGR_CLIENT_H
#define OHOS_ABILITY_RUNTIME_APP_MGR_CLIENT_H

#include "iremote_object.h"
#include "refbase.h"

#include "ability_info.h"
#include "application_info.h"
#include "iapp_state_callback.h"
#include "render_process_info.h"
#include "running_process_info.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
class AppServiceManager;
class AppMgrClient {
public:
    AppMgrClient();
    virtual ~AppMgrClient();

    /**
     * Load ability.
     *
     * @param token, Ability identify.
     * @param abilityInfo, Ability information.
     * @param appInfo, Application information.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode LoadAbility(sptr<IRemoteObject>& token, sptr<IRemoteObject> preToken,
        const AbilityInfo& abilityInfo, const ApplicationInfo& appInfo, const AAFwk::Want& want, int32_t);

    /**
     * Terminate ability.
     *
     * @param token Ability identify.
     *
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode TerminateAbility(const sptr<IRemoteObject>& token);

    /**
     * Update ability state.
     *
     * @param token Ability identify.
     * @param state Ability running state.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode UpdateAbilityState(const sptr<IRemoteObject>& token, const AbilityState state,
        bool isFromScreenOffBackground = false);

    /**
     * Register Application state callback.
     *
     * @param callback IAppStateCallback
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode RegisterAppStateCallback(const sptr<IAppStateCallback>& callback);

    /**
     * Connect service.
     *
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode ConnectAppMgrService();

    /**
     * Ability manager resst.
     *
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode Reset();

    /**
     * KillProcessByAbilityToken, call KillProcessByAbilityToken() through proxy object,
     * kill the process by ability token.
     *
     * @param token, the unique identification to the ability.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode KillProcessByAbilityToken(const sptr<IRemoteObject>& token);

    /**
     * KillProcessesByUserId, call KillProcessesByUserId() through proxy object,
     * kill the processes by user id.
     * Send appSpawn uninstall debug hap message.
     *
     * @param userId, the user id.
     * @param isNeedSendAppSpawnMsg, true send appSpawn message otherwise not send.
     * @return Returns RESULT_OK on success, others on failure.
     */
    virtual AppMgrResultCode KillProcessesByUserId(int32_t userId, bool isNeedSendAppSpawnMsg = false,
        sptr<AAFwk::IUserCallback> callback = nullptr);

    /**
     * KillProcessesByPids, call KillProcessesByPids() through proxy object,
     * kill processes by pid list.
     *
     * @param pids, pid list of processes are going to be killed.
     * @param reason, reason to kill processes.
     * @param subProcess, kill SubProcess or not.
     */
    virtual AppMgrResultCode KillProcessesByPids(const std::vector<int32_t> &pids,
        const std::string &reason = "KillProcessesByPids", bool subProcess = false, bool isKillPrecedeStart = false);

    /**
     * KillApplication, call KillApplication() through proxy object, kill the application.
     *
     * @param  bundleName, bundle name in Application record.
     * @return ERR_OK, return back success, others fail.
     */
    virtual AppMgrResultCode KillApplication(const std::string& bundleName, bool clearPageStack = false,
        int32_t appIndex = 0, const std::string& reason = "KillApplication");

    /**
     * ClearUpApplicationData, call ClearUpApplicationData() through proxy project,
     * clear the application data.
     *
     * @param bundleName, bundle name in Application record.
     * @return
     */
    virtual AppMgrResultCode ClearUpApplicationData(const std::string& bundleName);

    /**
     * GetAllRunningProcesses, call GetAllRunningProcesses() through proxy project.
     * Obtains information about application processes that are running on the device.
     *
     * @param info, app name in Application record.
     * @return ERR_OK ,return back success��others fail.
     */
    virtual AppMgrResultCode GetAllRunningProcesses(std::vector<RunningProcessInfo>& info);

    /**
     * GetAllRunningInstanceKeysBySelf, call GetAllRunningInstanceKeysBySelf() through proxy project.
     * Obtains running instance keys of multi-instance app that are running on the device.
     *
     * @param bundlename, bundle name in Application record.
     * @param instanceKeys, output instance keys of the multi-instance app.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual AppMgrResultCode GetAllRunningInstanceKeysBySelf(std::vector<std::string> &instanceKeys);

    /**
     * GetAllRunningInstanceKeysByBundleName, call GetAllRunningInstanceKeysByBundleName() through proxy project.
     * Obtains running instance keys of multi-instance app that are running on the device.
     *
     * @param bundlename, bundle name in Application record.
     * @param instanceKeys, output instance keys of the multi-instance app.
     * @param userId, user id.
     * @return ERR_OK ,return back success，others fail.
     */
    virtual AppMgrResultCode GetAllRunningInstanceKeysByBundleName(const std::string &bundleName,
        std::vector<std::string> &instanceKeys, int32_t userId = -1);

    /**
     * GetAllRenderProcesses, call GetAllRenderProcesses() through proxy project.
     * Obtains information about render processes that are running on the device.
     *
     * @param info, render process info.
     * @return ERR_OK, return back success, others fail.
     */
    virtual AppMgrResultCode GetAllRenderProcesses(std::vector<RenderProcessInfo> &info);

    /**
     * SetAppSuspendTimes, Setting the Freezing Time of APP Background.
     *
     * @param time, The timeout recorded when the application enters the background .
     *
     * @return Success or Failure .
     */
    virtual AppMgrResultCode SetAppFreezingTime(int time);

    /**
     * GetAppFreezingTime, Getting the Freezing Time of APP Background.
     *
     * @param time, The timeout recorded when the application enters the background .
     *
     * @return Success or Failure .
     */
    virtual AppMgrResultCode GetAppFreezingTime(int& time);
    virtual void AbilityAttachTimeOut(const sptr<IRemoteObject>& token);

    virtual void PrepareTerminate(const sptr<IRemoteObject>& token, bool clearMissionFlag = false);

    void GetRunningProcessInfoByToken(const sptr<IRemoteObject>& token, AppExecFwk::RunningProcessInfo& info);

    void GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo& info) const;

private:
    void SetServiceManager(std::unique_ptr<AppServiceManager> serviceMgr);

private:
    std::unique_ptr<AppServiceManager> serviceManager_;
    sptr<IRemoteObject> remote_;
    sptr<IAppStateCallback> callback_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_MGR_CLIENT_H
