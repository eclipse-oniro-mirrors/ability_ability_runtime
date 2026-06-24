/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_NATIVE_ABILITY_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_NATIVE_ABILITY_CONTEXT_H

#include <map>

#include "context_container.h"
#include "data_ability_helper.h"

namespace OHOS {
namespace DataShare {
class DataShareHelper;
}
namespace AppExecFwk {
class AbilityContext : public ContextContainer {
public:
    AbilityContext() = default;
    virtual ~AbilityContext() = default;

    /**
     * @brief Starts a new ability.
     * An ability using the AbilityInfo.AbilityType.SERVICE or AbilityInfo.AbilityType.PAGE template uses this method
     * to start a specific ability. The system locates the target ability from installed abilities based on the value
     * of the want parameter and then starts it. You can specify the ability to start using the want parameter.
     *
     * @param want Indicates the Want containing information about the target ability to start.
     *
     * @param requestCode Indicates the request code returned after the ability using the AbilityInfo.AbilityType.PAGE
     * template is started. You can define the request code to identify the results returned by abilities. The value
     * ranges from 0 to 65535. This parameter takes effect only on abilities using the AbilityInfo.AbilityType.PAGE
     * template.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    using ContextContainer::StartAbility;
    ErrCode StartAbility(const AAFwk::Want &want, int requestCode) override;

    /**
     * @brief Starts a new ability with special ability start setting.
     *
     * @param want Indicates the Want containing information about the target ability to start.
     * @param requestCode Indicates the request code returned after the ability is started. You can define the request
     * code to identify the results returned by abilities. The value ranges from 0 to 65535.
     * @param abilityStartSetting Indicates the special start setting used in starting ability.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    ErrCode StartAbility(const Want &want, int requestCode, const AbilityStartSetting &abilityStartSetting) override;

    ErrCode AddFreeInstallObserver(const sptr<AbilityRuntime::IFreeInstallObserver> &observer);

    /**
     * @brief Destroys the current ability.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    ErrCode TerminateAbility() override;

    /**
     * @brief Obtains the bundle name of the ability that called the current ability.
     * You can use the obtained bundle name to check whether the calling ability is allowed to receive the data you will
     * send. If you did not use Ability.startAbilityForResult(ohos.aafwk.content.Want, int,
     * ohos.aafwk.ability.startsetting.AbilityStartSetting) to start the calling ability, null is returned.
     *
     * @return Returns the bundle name of the calling ability; returns null if no calling ability is available.
     */
    std::string GetCallingBundle() override;

    /**
     * @brief Obtains the ohos.bundle.ElementName object of the current ability.
     *
     * @return Returns the ohos.bundle.ElementName object of the current ability.
     */
    std::shared_ptr<ElementName> GetElementName();

    /**
     * @brief Obtains the ElementName of the ability that called the current ability.
     *
     * @return Returns the ElementName of the calling ability; returns null if no calling ability is available.
     */
    std::shared_ptr<ElementName> GetCallingAbility();

    /**
     * @brief Connects the current ability to an ability using the AbilityInfo.AbilityType.SERVICE template.
     *
     * @param want Indicates the want containing information about the ability to connect
     *
     * @param conn Indicates the callback object when the target ability is connected.
     *
     * @return True means success and false means failure
     */
    bool ConnectAbility(const Want &want, const sptr<AAFwk::IAbilityConnection> &conn) override;

    /**
     * @brief Disconnects the current ability from an ability
     *
     * @param conn Indicates the IAbilityConnection callback object passed by connectAbility after the connection
     *              is set up. The IAbilityConnection object uniquely identifies a connection between two abilities.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    ErrCode DisconnectAbility(const sptr<AAFwk::IAbilityConnection> &conn) override;

    /**
     * @brief Destroys another ability that uses the AbilityInfo.AbilityType.SERVICE template.
     * The current ability using either the AbilityInfo.AbilityType.SERVICE or AbilityInfo.AbilityType.PAGE
     * template can call this method to destroy another ability that uses the AbilityInfo.AbilityType.SERVICE
     * template. The current ability itself can be destroyed by calling the terminateAbility() method.
     *
     * @param want Indicates the Want containing information about the ability to destroy.
     *
     * @return Returns true if the ability is destroyed successfully; returns false otherwise.
     */
    virtual bool StopAbility(const AAFwk::Want &want) override;

    /**
     * @brief Obtains a resource manager.
     *
     * @return Returns a ResourceManager object.
     */
    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override;

    /**
     * @brief Query whether the application of the specified PID and UID has been granted a certain permission
     *
     * @param permissions Indicates the list of permissions to be requested. This parameter cannot be null.
     * @param pid Process id
     * @param uid
     * @return Returns 0 (IBundleManager.PERMISSION_GRANTED) if the current process has the permission;
     * returns -1 (IBundleManager.PERMISSION_DENIED) otherwise.
     */
    virtual int VerifyPermission(const std::string &permission, int pid, int uid) override;

    /**
     * @brief Requests certain permissions from the system.
     * This method is called for permission request. This is an asynchronous method. When it is executed,
     * the task will be called back.
     *
     * @param permissions Indicates the list of permissions to be requested. This parameter cannot be null.
     * @param permissionsState Indicates the list of permissions' state to be requested. This parameter cannot be null.
     * @param task The callback or promise fo js interface.
     */
    virtual void RequestPermissionsFromUser(std::vector<std::string> &permissions, std::vector<int> &permissionsState,
        PermissionRequestTask &&task) override;

    /**
     * @brief Set deviceId/bundleName/abilityName of the calling ability
     *
     * @param deviceId deviceId of the calling ability
     *
     * @param bundleName bundleName of the calling ability
     *
     * @param abilityName abilityName of the calling ability
     */
    void SetCallingContext(const std::string &deviceId, const std::string &bundleName,
        const std::string &abilityName, const std::string &moduleName = "");

    /**
     * @brief Starts multiple abilities.
     *
     * @param wants Indicates the Want containing information array about the target ability to start.
     */
    void StartAbilities(const std::vector<AAFwk::Want> &wants) override;

    void SetAbilityRecordId(int32_t abilityRecordId)
    {
        abilityRecordId_ = abilityRecordId;
    }
    int32_t GetAbilityRecordId() const
    {
        return abilityRecordId_;
    }

    friend DataAbilityHelper;
    friend OHOS::DataShare::DataShareHelper;
    static int ABILITY_CONTEXT_DEFAULT_REQUEST_CODE;

protected:
    sptr<IRemoteObject> GetToken() override;
    sptr<IRemoteObject> GetSessionToken();

    int resultCode_ = -1;
    int32_t abilityRecordId_ = 0;
    sptr<IRemoteObject> token_;
    sptr<IRemoteObject> sessionToken_;
    std::string callingDeviceId_;
    std::string callingBundleName_;
    std::string callingAbilityName_;
    std::string callingModuleName_;
    std::map<sptr<AAFwk::IAbilityConnection>, sptr<IRemoteObject>> abilityConnectionMap_;
    std::mutex sessionTokenMutex_;
    AAFwk::Want resultWant_;

private:
    /**
     * @brief Get Current Ability Type
     *
     * @return Current Ability Type
     */
    AppExecFwk::AbilityType GetAbilityInfoType();
    void GetPermissionDes(const std::string &permissionName, std::string &des);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NATIVE_ABILITY_CONTEXT_H
