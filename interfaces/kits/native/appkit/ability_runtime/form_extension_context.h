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

#ifndef OHOS_ABILITY_RUNTIME_FORM_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_FORM_EXTENSION_CONTEXT_H

#include "ability_info.h"
#include "ability_connect_callback.h"
#include "extension_context.h"
#include "form_provider_info.h"
#include "start_options.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * @brief context supply for form extension
 *
 */
class FormExtensionContext : public ExtensionContext {
public:
    FormExtensionContext() = default;
    virtual ~FormExtensionContext() = default;

    /**
     * @brief Updates the content of a specified JS form.
     *
     * @param formId Indicates the ID of the JS form to update.
     *
     * @param formProviderData Indicates the {@code FormProviderData} object used to update the JS form displayed
     *                        on the client.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    int UpdateForm(const int64_t formId, const AppExecFwk::FormProviderData &formProviderData);

    /**
     * @brief Updates the content of a specified JS form with FormProviderInfo.
     *
     * @param formId Indicates the ID of the JS form to update.
     * @param formProviderInfo Indicates the {@code FormProviderInfo} object including isSupportMoreImages.
     * @return Returns ERR_OK on success, others on failure.
     */
    int UpdateForm(const int64_t formId, const AppExecFwk::FormProviderInfo &formProviderInfo);

    /**
     * @brief Starts a new ability.
     * An ability using the AbilityInfo.AbilityType.SERVICE or AbilityInfo.AbilityType.PAGE template uses this method
     * to start a specific ability. The system locates the target ability from installed abilities based on the value
     * of the want parameter and then starts it. You can specify the ability to start using the want parameter.
     *
     * @param want Indicates the Want containing information about the target ability to start.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    ErrCode StartAbility(const AAFwk::Want &want) const;

    /**
     * @brief Obtains information about the current ability.
     * The returned information includes the class name, bundle name, and other information about the current ability.
     *
     * @return Returns the AbilityInfo object for the current ability.
     */
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> GetAbilityInfo() const;

    /**
     * @brief Connects the current ability to an ability using the AbilityInfo.AbilityType.SERVICE template.
     *
     * @param want Indicates the want containing information about the ability to connect
     *
     * @param conn Indicates the callback object when the target ability is connected.
     *
     * @return Returns zero on success, others on failure.
     */
    ErrCode ConnectAbility(
        const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const;

    /**
     * @brief Disconnects the current ability from an ability.
     *
     * @param conn Indicates the IAbilityConnection callback object passed by connectAbility after the connection
     * is set up. The IAbilityConnection object uniquely identifies a connection between two abilities.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    ErrCode DisconnectAbility(
        const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const;

    /**
     * @brief Set AbilityInfo when init.
     *
     */
    void SetAbilityInfo(const std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> &abilityInfo);

    using SelfType = FormExtensionContext;
    static const size_t CONTEXT_TYPE_ID;

protected:
    bool IsContext(size_t contextTypeId) override
    {
        return contextTypeId == CONTEXT_TYPE_ID || ExtensionContext::IsContext(contextTypeId);
    }

private:
    /**
     * @brief Get Current Ability Type
     *
     * @return Current Ability Type
     */
    OHOS::AppExecFwk::AbilityType GetAbilityInfoType() const;

    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_FORM_EXTENSION_CONTEXT_H
