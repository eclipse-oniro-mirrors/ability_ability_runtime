/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_IAPP_STATE_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_IAPP_STATE_CALLBACK_H

#include "iremote_broker.h"
#include "iremote_object.h"

#include "app_mgr_constants.h"
#include "app_process_data.h"
#include "bundle_info.h"
#include "running_process_info.h"

namespace OHOS {
namespace AppExecFwk {
    class Configuration;
}
}

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr pid_t INVALID_DIED_PID = -1;
}
class IAppStateCallback : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.AppStateCallback");

    /**
     * Application state changed callback.
     *
     * @param appProcessData Process data
     */
    virtual void OnAppStateChanged(const AppProcessData &appProcessData) = 0;

    /**
     * AbilityMgr's request is done.
     *
     * @param token Ability token.
     * @param state Application state.
     */
    virtual void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const AbilityState state) = 0;

    /**
     * @brief Notify application update system environment changes.
     * @param config System environment change parameters.
     * @param userId userId Designation User ID.
     */
    virtual void NotifyConfigurationChange(const AppExecFwk::Configuration &config, int32_t userId) {}

    /**
     * @brief Notify abilityms start resident process.
     * @param bundleInfos resident process bundle infos.
     */
    virtual void NotifyStartResidentProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos) {}

    /**
     * @brief Notify abilityms start keep-alive process.
     * @param bundleInfos resident process bundle infos.
     */
    virtual void NotifyStartKeepAliveProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos,
        pid_t diedPid = INVALID_DIED_PID) {}

    /**
     * @brief Notify abilityms app process OnRemoteDied
     * @param abilityTokens abilities in died process.
     */
    virtual void OnAppRemoteDied(const std::vector<sptr<IRemoteObject>> &abilityTokens) {}

    /**
     * @brief Notify abilityms start process failed when load ability
     * @param abilityTokens abilities in died process.
     */
    virtual void OnStartProcessFailed(const std::vector<sptr<IRemoteObject>> &abilityTokens) {}

    /**
     * @brief Notify abilityms app process pre cache
     * @param pid process pid.
     * @param userId userId Designation User ID.
     */
    virtual void NotifyAppPreCache(int32_t pid, int32_t userId) {}

    /**
     * @brief Notify abilityms process info when an app dies
     * @param accessTokenId app accessTokenId.
     * @param exitInfo process running info.
     * @param bundleName app bundleName.
     * @param abilityNames started abilities.
     * @param uiExtensionNames started ui extensions.
     */
    virtual void OnCacheExitInfo(uint32_t accessTokenId, const RunningProcessInfo &exitInfo,
        const std::string &bundleName, const std::vector<std::string> &abilityNames,
        const std::vector<std::string> &uiExtensionNames) {}

    /**
     * @brief Record the signal reason when an application process exits.
     *
     * @param pid Process ID of the exited application process.
     * @param uid User ID of the exited application process.
     * @param signal Signal number that caused the process to exit.
     * @param bundleName Bundle name of the exited application.
     */
    virtual void RecordAppExitSignalReason(int32_t pid, int32_t uid, int32_t signal, std::string &bundleName) {}

    enum class Message {
        TRANSACT_ON_APP_STATE_CHANGED = 0,
        TRANSACT_ON_ABILITY_REQUEST_DONE,
        TRANSACT_ON_NOTIFY_CONFIG_CHANGE,
        TRANSACT_ON_NOTIFY_START_RESIDENT_PROCESS,
        TRANSACT_ON_APP_REMOTE_DIED,
        TRANSACT_ON_APP_PRE_CACHE,
        TRANSACT_ON_NOTIFY_START_KEEP_ALIVE_PROCESS,
        TRANSACT_ON_START_PROCESS_FAILED,
        TRANSACT_ON_CACHE_EXIT_INFO,
        TRANSACT_ON_RECORD_APP_EXIT_SIGNAL_REASON,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_IAPP_STATE_CALLBACK_H
