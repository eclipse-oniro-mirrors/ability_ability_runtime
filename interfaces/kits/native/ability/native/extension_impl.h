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

#ifndef OHOS_ABILITY_RUNTIME_EXTENSION_IMPL_H
#define OHOS_ABILITY_RUNTIME_EXTENSION_IMPL_H

#include "extension.h"
#include "extension_ability_info.h"
#include "lifecycle_state_info.h"

namespace OHOS {
class IRemoteObject;
namespace AAFwk {
class Want;
}
namespace AppExecFwk {
struct AbilityInfo;
class OHOSApplication;
class AbilityHandler;
class AbilityLocalRecord;
}
namespace AbilityRuntime {
/**
 * @brief Responsible for managing and scheduling the life cycle of extension.
 */
class ExtensionImpl : public std::enable_shared_from_this<ExtensionImpl> {
public:
    ExtensionImpl() = default;
    virtual ~ExtensionImpl();

    /**
     * @brief Init the object.
     *
     * @param application the application info.
     * @param record the extension record.
     * @param extension the extension object.
     * @param handler the extension handler.
     * @param token the remote token.
     */
    void Init(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
        std::shared_ptr<Extension> &extension,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token);

    /**
     * @brief Connect the Extension. and Calling information back to Extension.
     * @param want The Want object to connect to.
     * @param targetState The terget state.
     * @param sessionInfo  Indicates the sessionInfo.
     *
     */
    virtual void HandleExtensionTransaction(const Want &want, const AAFwk::LifeCycleStateInfo &targetState,
        sptr<AAFwk::SessionInfo> sessionInfo = nullptr);

    /**
     * @brief scheduling update configuration of extension.
     *
     * @param config Configuration
     */
    void ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config);

    /**
     * @brief Notify current memory level.
     *
     * @param level Current memory level.
     */
    void NotifyMemoryLevel(int level);

    /**
     * @brief Connect the Extension. and Calling information back to Extension.
     *
     * @param want The Want object to connect to.
     *
     */
    sptr<IRemoteObject> ConnectExtension(const Want &want);

    /**
     * @brief Connect the Extension. and Calling information back to Extension.
     *
     * @param want The Want object to connect to.
     * @param isAsyncCallback Indicates whether it is an asynchronous lifecycle callback
     */
    sptr<IRemoteObject> ConnectExtension(const Want &want, bool &isAsyncCallback);

    /**
     * @brief The callback of connect.
     */
    void ConnectExtensionCallback(sptr<IRemoteObject> &service);

    /**
     * @brief Disconnects the connected object.
     *
     * @param want The Want object to disconnect to.
     */
    void DisconnectExtension(const Want &want);

    /**
     * @brief Disconnects the connected object.
     *
     * @param want The Want object to disconnect to.
     * @param isAsyncCallback Indicates whether it is an asynchronous lifecycle callback
     */
    void DisconnectExtension(const Want &want, bool &isAsyncCallback);

    /**
     * @brief The callback of disconnect.
     */
    void DisconnectExtensionCallback();

    /**
     * @brief Command the Extension. and Calling information back to Extension.
     *
     * @param want The Want object to command to.
     *
     * * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
     * destroyed, and the value false indicates a normal startup.
     *
     * @param startId Indicates the number of times the Service Extension has been started. The startId is incremented
     * by 1 every time the Extension is started. For example, if the Extension has been started for six times, the value
     * of startId is 6.
     */
    void CommandExtension(const Want &want, bool restart, int startId);

    /**
     * @brief Handle insight intent.
     *
     * @param want The Want object with insight intent to handle.
     */
    bool HandleInsightIntent(const Want &want);

    void CommandExtensionWindow(const Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo,
        AAFwk::WindowCommand winCmd);

    /*
     * SendResult, Send result to app when extension ability is terminated with result want.
     *
     * @param requestCode, the requestCode of the extension ability to start.
     * @param resultCode, the resultCode of the extension ability to terminate.
     * @param resultData, the want of the extension  ability to terminate.
     */
    void SendResult(int requestCode, int resultCode, const Want &resultData);

    /**
     * @brief Save information about ability launch.
     *
     * @param launchParam Used to save information about ability launch param.
     */
    void SetLaunchParam(const AAFwk::LaunchParam &launchParam);

    void ScheduleAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message, int32_t resultCode = 0);

    void ScheduleAbilityRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element);

protected:
    /**
     * @brief Toggles the lifecycle status of Extension to AAFwk::ABILITY_STATE_INACTIVE. And notifies the application
     * that it belongs to of the lifecycle status.
     *
     * @param want  The Want object to switch the life cycle.
     * @param sessionInfo  Indicates the sessionInfo.
     */
    void Start(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo = nullptr);

    /**
     * @brief Toggles the lifecycle status of Extension to AAFwk::ABILITY_STATE_INITIAL. And notifies the application
     * that it belongs to of the lifecycle status.
     *
     */
    void Stop();
     /**
     * @brief Toggles the lifecycle status of Extension to AAFwk::ABILITY_STATE_INITIAL. And notifies the application
     * that it belongs to of the lifecycle status.
     * @param isAsyncCallback Indicates whether it is an asynchronous lifecycle callback
     * @param want Indicates want.
     * @param sessionInfo Indicates the sessionInfo, nullptr when not uiextension.
     */
    void Stop(bool &isAsyncCallback, const Want &want, sptr<AAFwk::SessionInfo> sessionInfo);
    void AbilityTransactionCallback(const AAFwk::AbilityLifeCycleState &state);

    /**
     * @brief Toggles the lifecycle status of Extension to AAFwk::ABILITY_STATE_INACTIVE. And notifies the application
     * that it belongs to of the lifecycle status.
     *
     * @param want The Want object to switch the life cycle.
     */
    void Foreground(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo);

    /**
     * @brief Toggles the lifecycle status of Extension to AAFwk::ABILITY_STATE_BACKGROUND. And notifies the
     * application that it belongs to of the lifecycle status.
     * @param want Indicates want.
     * @param sessionInfo Indicates the sessionInfo, nullptr when not uiextension.
     */
    void Background(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo);

private:
    inline bool UIExtensionAbilityExecuteInsightIntent(const Want &want);

    bool skipCommandExtensionWithIntent_ = false;
    int lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    sptr<IRemoteObject> token_;
    std::shared_ptr<Extension> extension_;
    AppExecFwk::ExtensionAbilityType extensionType_ = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;

class ExtensionWindowLifeCycleImpl : public Rosen::IWindowLifeCycle {
public:
    ExtensionWindowLifeCycleImpl(const sptr<IRemoteObject>& token, const std::shared_ptr<ExtensionImpl>& owner)
        : token_(token), owner_(owner) {}
    virtual ~ExtensionWindowLifeCycleImpl() {}
    void AfterForeground() override;
    void AfterBackground() override;
    void AfterActive() override;
    void AfterInactive() override;
private:
    sptr<IRemoteObject> token_ = nullptr;
    std::weak_ptr<ExtensionImpl> owner_;
};
};
}
}
#endif  // OHOS_ABILITY_RUNTIME_EXTENSION_IMPL_H
