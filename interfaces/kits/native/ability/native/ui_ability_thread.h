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

#ifndef OHOS_ABILITY_RUNTIME_UI_ABILITY_THREAD_H
#define OHOS_ABILITY_RUNTIME_UI_ABILITY_THREAD_H

#include "ability_thread.h"
#include "context.h"
#include "ui_ability.h"
#include "ui_ability_impl.h"

namespace OHOS {
namespace AppExecFwk {
class AbilityLocalRecord;
class OHOSApplication;
} // namespace AppExecFwk
namespace AbilityRuntime {
using LifeCycleStateInfo = OHOS::AAFwk::LifeCycleStateInfo;
class UIAbilityThread : public AppExecFwk::AbilityThread {
public:
    /**
     * @brief Default constructor used to create a UIAbilityThread instance.
     */
    UIAbilityThread();
    ~UIAbilityThread() override;

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
        const std::shared_ptr<Context> &appContext) override;

    /**
     * @brief Attach The ability thread to the main process.
     * @param application Indicates the main process.
     * @param abilityRecord current running ability record
     * @param appContext the AbilityRuntime context
     */
    void Attach(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
        const std::shared_ptr<Context> &appContext) override;

    /**
     * @brief ScheduleUpdateConfiguration, scheduling update configuration.
     * @param config Indicates the updated configuration information.
     */
    void ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config) override;

    /**
     * @brief notify this ability current memory level.
     * @param level Current memory level
     */
    void NotifyMemoryLevel(int32_t level) override;

    /**
     * @brief Provide operating system AbilityTransaction information to the observer
     * @param want Indicates the structure containing Transaction information about the ability.
     * @param targetState Indicates the lifecycle state.
     * @param sessionInfo Indicates the session info.
     */
    bool ScheduleAbilityTransaction(const Want &want, const LifeCycleStateInfo &targetState,
        sptr<AAFwk::SessionInfo> sessionInfo = nullptr) override;

    /**
     * @brief Provide operating system ShareData information to the observer
     * @param requestCode Indicates the Ability request code.
     */
    void ScheduleShareData(const int32_t &requestCode) override;

    /**
     * @brief Provide operating system PrepareTerminateAbility information to the observer
     */
    bool SchedulePrepareTerminateAbility() override;

    /**
     * @brief Provide operating system SaveabilityState information to the observer
     */
    void ScheduleSaveAbilityState() override;

    /**
     * @brief Provide operating system RestoreAbilityState information to the observer
     * @param state Indicates resotre ability state used to dispatchRestoreAbilityState.
     */
    void ScheduleRestoreAbilityState(const AppExecFwk::PacMap &state) override;

    /**
     * @brief Send the result code and data to be returned by this Page ability to the caller.
     * When a Page ability is destroyed, the caller overrides the AbilitySlice#onAbilityResult(int, int, Want) method to
     * receive the result set in the current method. This method can be called only after the ability has been
     * initialized.
     * @param requestCode Indicates the request code for send.
     * @param resultCode Indicates the result code returned after the ability is destroyed. You can define the result
     * code to identify an error.
     * @param want Indicates the data returned after the ability is destroyed. You can define the data returned. This
     * parameter can be null.
     */
    void SendResult(int requestCode, int resultCode, const Want &resultData) override;

    /**
     * @brief continue ability to target device.
     * @param deviceId target deviceId
     * @param versionCode Target bundle version.
     */
    void ContinueAbility(const std::string &deviceId, uint32_t versionCode) override;

    /**
     * @brief notify this ability continuation result.
     * @param result Continuation result
     */
    void NotifyContinuationResult(int32_t result) override;

    /**
     * @brief Dump ability runner info.
     * @param runnerInfo ability runner info.
     */
    void DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info) override;

    /**
     * @brief Call Request
     */
    void CallRequest() override;

    void OnExecuteIntent(const Want &want) override;

    void ExecuteSkill(const Want &want) override;

    /**
     * @brief create modal UIExtension.
     * @param want Create modal UIExtension with want object.
     */
    int CreateModalUIExtension(const Want &want) override;

    /**
     * @brief Update sessionToken.
     * @param sessionToken The token of session.
     */
    void UpdateSessionToken(sptr<IRemoteObject> sessionToken) override;

    void ScheduleCollaborate(const Want &want) override;

    void ScheduleAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message, int32_t resultCode = 0) override;

    void ScheduleAbilityRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element) override;

#ifdef SUPPORT_SCREEN
    /**
     * @brief Get the UIability of the current thread.
     * @return Returns a null pointer if the UIAbility has not been created or has been destroyed.
     */
    std::shared_ptr<UIAbility> GetUIAbility() override;
#endif

private:
    void DumpAbilityInfoInner(const std::vector<std::string> &params, std::vector<std::string> &info);
    void DumpOtherInfo(std::vector<std::string> &info);
    bool InitNativeThread(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo);
    void AttachInner(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
        const std::shared_ptr<Context> &stageContext);
    std::string CreateAbilityName(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord);
    std::shared_ptr<AppExecFwk::ContextDeal> CreateAndInitContextDeal(
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
        const std::shared_ptr<AppExecFwk::AbilityContext> &abilityObject);
    std::shared_ptr<AbilityContext> BuildAbilityContext(
        const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application, const sptr<IRemoteObject> &token,
        const std::shared_ptr<AbilityRuntime::Context> &stageContext, int32_t abilityRecordId);
    void HandleAbilityTransaction(const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo,
        sptr<AAFwk::SessionInfo> sessionInfo = nullptr);
    void HandleShareData(const int32_t &requestCode);
    bool HandlePrepareTerminateAbility();
    void HandleUpdateConfiguration(const AppExecFwk::Configuration &config);
    void AddLifecycleEvent(uint32_t state, std::string &methodName) const;

    std::shared_ptr<UIAbilityImpl> abilityImpl_ = nullptr;
    std::shared_ptr<UIAbility> currentAbility_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UI_ABILITY_THREAD_H
