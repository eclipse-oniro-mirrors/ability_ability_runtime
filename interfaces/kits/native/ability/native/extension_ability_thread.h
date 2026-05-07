/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_EXTENSION_ABILITY_THREAD_H
#define OHOS_ABILITY_RUNTIME_EXTENSION_ABILITY_THREAD_H

#include "ability_thread.h"
#include "extension_impl.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
class AbilityLocalRecord;
class OHOSApplication;
} // namespace AppExecFwk
namespace AbilityRuntime {
using LifeCycleStateInfo = OHOS::AAFwk::LifeCycleStateInfo;
class ExtensionAbilityThread : public AppExecFwk::AbilityThread {
public:
    /**
     * @brief Default constructor used to create a ExtensionAbilityThread instance.
     */
    ExtensionAbilityThread();
    ~ExtensionAbilityThread() override;

    /**
     * @brief Attach The ability thread to the main process.
     * @param application Indicates the main process.
     * @param abilityRecord current running ability record
     * @param mainRunner The runner which main_thread holds.
     * @param appContext the AbilityRuntime context
     */
    void Attach(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
        const std::shared_ptr<AppExecFwk::EventRunner> &mainRunner,
        [[maybe_unused]] const std::shared_ptr<Context> &appContext) override;

    /**
     * @brief Attach The ability thread to the main process.
     * @param application Indicates the main process.
     * @param abilityRecord current running ability record
     * @param appContext the AbilityRuntime context
     */
    void Attach(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
        [[maybe_unused]] const std::shared_ptr<Context> &appContext) override;

    /**
     * @brief Provide operating system AbilityTransaction information to the observer
     * @param want Indicates the structure containing Transaction information about the ability.
     * @param targetState Indicates the lifecycle state.
     * @param sessionInfo Indicates the session info.
     */
    bool ScheduleAbilityTransaction(const Want &want, const LifeCycleStateInfo &targetState,
        sptr<AAFwk::SessionInfo> sessionInfo = nullptr) override;

    /**
     * @brief Provide operating system ConnectAbility information to the observer
     * @param want Indicates the structure containing connect information about the ability.
     */
    void ScheduleConnectAbility(const Want &want) override;

    /**
     * @brief Provide operating system DisconnectAbility information to the observer
     * @param want Indicates the structure containing connect information about the ability.
     */
    void ScheduleDisconnectAbility(const Want &want) override;

    /**
     * @brief Provide operating system CommandAbility information to the observer
     * @param want The Want object to command to.
     * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
     * destroyed, and the value false indicates a normal startup.
     * @param startId Indicates the number of times the Service ability has been started. The startId is incremented by
     * 1 every time the ability is started. For example, if the ability has been started for six times, the value of
     * startId is 6.
     */
    void ScheduleCommandAbility(const Want &want, bool restart, int startId) override;

    /**
     * @brief Schedule Command AbilityWindow
     * @param want The Want object to command to.
     * @param sessionInfo Indicates the session info.
     * @param winCmd Indicates the WindowCommand of winCmd
     */
    void ScheduleCommandAbilityWindow(
        const Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd) override;

    /**
     * @brief ScheduleUpdateConfiguration, scheduling update configuration.
     * @param config Indicates the updated configuration information
     */
    void ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config) override;

    /**
     * @brief Send the result code and data to be returned by this Page ability to the caller.
     * When a Page ability is destroyed, the caller overrides the AbilitySlice#onAbilityResult(int32_t, int32_t, Want)
     * method to receive the result set in the current method. This method can be called only after the ability has
     * been initialized.
     * @param requestCode Indicates the request code for send.
     * @param resultCode Indicates the result code returned after the ability is destroyed. You can define the result
     * code to identify an error.
     * @param want Indicates the data returned after the ability is destroyed. You can define the data returned. This
     * parameter can be null.
     */
    void SendResult(int requestCode, int resultCode, const Want &resultData) override;

    /**
     * @brief notify this ability current memory level.
     * @param level Current memory level
     */
    void NotifyMemoryLevel(int32_t level) override;

    /**
     * @brief Dump ability runner info.
     * @param params the params need to be Dumped
     * @param info ability runner info.
     */
    void DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info) override;

    void ScheduleAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message, int32_t resultCode = 0) override;

    void ScheduleAbilityRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element) override;

    void ScheduleAbilitiesRequestDone(const std::string &requestKey, int32_t resultCode) override;

    void ExecuteSkill(const Want &want) override;

    void HandleNativeExtensionAttach(
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
        const std::string &abilityName);
        
    void HandleNormalExtensionAttach(
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
        const std::shared_ptr<AppExecFwk::EventRunner> &mainRunner,
        const std::string &abilityName);

private:

    /**
     * @brief Dump Ability Info Inner
     * @param params the params need to be Dumped
     * @param info ability runner info
     */
    void DumpAbilityInfoInner(const std::vector<std::string> &params, std::vector<std::string> &info);

    /**
     * @brief Dump Other Info
     * @param info Indicates the info to be Dumped
     */
    void DumpOtherInfo(std::vector<std::string> &info);

    /**
     * @brief Create the abilityname.
     * @param abilityRecord current running ability record.
     * @param application Indicates the application.
     * @return Returns the abilityname.
     */
    std::string CreateAbilityName(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application);

    /**
     * @brief Create the extension abilityname.
     * @param abilityInfo abilityInfo Indicates the parameter about abilityInfo.
     * @param abilityName abilityName Indicates the parameter about abilityName.
     */
    void CreateExtensionAbilityName(
        const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo, std::string &abilityName);

    /**
     * @brief Handle the life cycle of Extension.
     * @param want Indicates the structure containing lifecycle information about the extension.
     * @param lifeCycleStateInfo  Indicates the lifecycle state.
     * @param sessionInfo Indicates the session info.
     */
    void HandleExtensionTransaction(const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo,
        sptr<AAFwk::SessionInfo> sessionInfo = nullptr);

    /**
     * @brief Handle the current connection of Extension.
     * @param want Indicates the structure containing connection information about the extension.
     */
    void HandleConnectExtension(const Want &want);

    /**
     * @brief Handle the current disconnection of Extension.
     * @param want Indicates the structure containing connection information about the extension.
     */
    void HandleDisconnectExtension(const Want &want);

    /**
     * @brief Handle the current command of Extension.
     * @param want The Want object to command to.
     * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
     * destroyed, and the value false indicates a normal startup.
     * @param startId Indicates the number of times the Service extension has been started. The startId is incremented
     * by 1 every time the extension is started. For example, if the extension has been started for six times, the
     * value of startId is 6.
     */
    void HandleCommandExtension(const Want &want, bool restart, int32_t startId);

    /**
     * @brief Handle Command Extension Window.
     * @param want The Want object to command to.
     * @param sessionInfo Indicates the sessionInfo.
     * @param winCmd Indicates the winCmd.
     */
    void HandleCommandExtensionWindow(
        const Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd);

    /**
     * @brief Handle the scheduling update configuration of exabilityInfotension.
     * @param config Indicates the updated configuration information
     */
    void HandleExtensionUpdateConfiguration(const AppExecFwk::Configuration &config);

    void HandleAttach(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
        const std::shared_ptr<AppExecFwk::EventRunner> &mainRunner);

    void HandleAttachInner(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord);

    /**
     * @brief Handle the current command of Extension.
     * @param want The Want object to command to.
     */
    void HandleInsightIntent(const Want &want);

    void HandleExecuteSkill(const Want &want);

    void ScheduleCommandAbilityInner(const Want &want, bool restart, int32_t startId);

    void ScheduleInsightIntentInner(const Want &want);

    void ScheduleSkillExecuteInner(const Want &want);

    std::shared_ptr<ExtensionImpl> extensionImpl_ = nullptr;
    std::shared_ptr<Extension> currentExtension_ = nullptr;
    static std::shared_ptr<AppExecFwk::EventRunner> contentEmbedEventRunner_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_EXTENSION_ABILITY_THREAD_H
