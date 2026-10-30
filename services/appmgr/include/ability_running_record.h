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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_RUNNING_RECORD_H
#define OHOS_ABILITY_RUNTIME_ABILITY_RUNNING_RECORD_H

#include <atomic>
#include <string>

#include "ability_info.h"
#include "application_info.h"
#include "app_mgr_constants.h"
#include "iremote_object.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
class AbilityRunningRecord {
public:
    AbilityRunningRecord(std::shared_ptr<AbilityInfo> info, sptr<IRemoteObject> token,
        int32_t abilityRecordId);
    virtual ~AbilityRunningRecord();

    /**
     * @brief Obtains the name of the ability.
     *
     * @return Returns the ability name.
     */
    const std::string &GetName() const;

    /**
     * @brief Obtains the bundle name of the ability.
     *
     * @return Returns the bundle name.
     */
    const std::string &GetBundleName() const;

    /**
     * @brief Obtains the module name of the ability.
     *
     * @return Returns the module name.
     */
    const std::string &GetModuleName() const;

    /**
     * @brief Obtains the info of the ability.
     *
     * @return Returns the ability info.
     */
    const std::shared_ptr<AbilityInfo> &GetAbilityInfo() const;

    /**
     * @brief Obtains the info of the ability.
     *
     * @return Returns the ability want.
     */
    const std::shared_ptr<AAFwk::Want> &GetWant() const;

    void SetWant(const std::shared_ptr<AAFwk::Want> &want);

    /**
     * @brief Obtains the token of the ability.
     *
     * @return Returns the ability token.
     */
    const sptr<IRemoteObject> &GetToken() const;

    /**
     * @brief Obtains the record id of the ability.
     *
     * @return Returns the ability record id.
     */
    int32_t GetAbilityRecordId() const
    {
        return abilityRecordId_;
    }

    /**
     * @brief Setting id for ability record.
     *
     * @param appId, the ability record id.
     */
    void SetAppRunningRecordId(const int32_t appId);

    /**
     * @brief Setting state for ability record.
     *
     * @param state, the ability record state.
     */
    void SetState(const AbilityState state);

    /**
     * @brief Obtains the state of the ability.
     *
     * @return Returns the ability state.
     */
    AbilityState GetState() const;

    /**
     * @brief Set the Terminating object.
     */
    void SetTerminating();

    /**
     * @brief Whether the ability is terminating.
     *
     * @return Returns whether the ability is terminating.
     */
    bool IsTerminating() const;

    void SetEventId(const int64_t eventId);
    int64_t GetEventId() const;

    /**
     * @brief SetOwnerUserId set the owner of the ability.
     */
    void SetOwnerUserId(int32_t ownerUserId);

    /**
     * @brief GetOwnerUserId get the owner of the ability.
     *
     * @return Return the owner's userId.
     */
    int32_t GetOwnerUserId() const;
    void SetIsSingleUser(bool flag);
    bool IsSingleUser() const;
    void UpdateFocusState(bool isFocus);
    bool GetFocusFlag() const;
    void SetUIExtensionAbilityId(const int32_t uiExtensionAbilityId);
    int32_t GetUIExtensionAbilityId() const;
    void SetUserRequestCleaningStatus();
    bool IsUserRequestCleaning() const;

    /**
     * @brief Whether the ability is scene board.
     *
     * @return Returns whether the ability is scene board.
     */
    bool IsSceneBoard() const;

    bool IsHook() const;

    void SetUIExtensionBindAbilityId(const int32_t uiExtensionBindAbilityId);
    int32_t GetUIExtensionBindAbilityId() const;
private:
    int32_t abilityRecordId_ = 0;
    int32_t uiExtensionAbilityId_ = 0;
    std::shared_ptr<AbilityInfo> info_;
    sptr<IRemoteObject> token_;
    sptr<IRemoteObject> preToken_;
    std::shared_ptr<AAFwk::Want> want_ = nullptr;
    int64_t eventId_ = 0;
    int32_t ownerUserId_ = -1;
    int32_t uiExtensionBindAbilityId_ = 0;
    AbilityState state_ = AbilityState::ABILITY_STATE_CREATE;
    std::atomic<bool> isTerminating_ = false;
    bool isFocused_ = false;
    bool isSingleUser_ = false;
    bool isUserRequestCleaning_ = false;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_RUNNING_RECORD_H
