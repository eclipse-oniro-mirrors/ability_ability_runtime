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

#ifndef OHOS_ABILITY_RUNTIME_UI_SERVICE_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_UI_SERVICE_EXTENSION_CONTEXT_H

#include "extension_context.h"

#include "ability_connect_callback.h"
#include "connection_manager.h"
#include "local_call_container.h"
#include "start_options.h"
#include "iability_callback.h"
#include "want.h"
#include "wm/window.h"
#include <atomic>
#include <memory>
#ifdef SUPPORT_SCREEN
#include "scene_board_judgement.h"
#include "ui_content.h"
#endif // SUPPORT_SCREEN

namespace OHOS {
namespace Ace {
struct ModalUIExtensionCallbacks;
}
namespace AbilityRuntime {
/**
 * @brief context supply for ui_service
 *
 */
class UIServiceExtensionContext : public ExtensionContext {
public:
    UIServiceExtensionContext() = default;
    virtual ~UIServiceExtensionContext() = default;

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
    ErrCode StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions) const;

    /**
     * @brief Destroys the current ability.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    ErrCode TerminateSelf();

    void SetWindow(sptr<Rosen::Window> window);

    sptr<Rosen::Window> GetWindow();

    /**
     * @brief Start a new ability using type;
     * @return errCode ERR_OK on success, others on failure.
    */
    ErrCode StartAbilityByType(const std::string &type, AAFwk::WantParams &wantParam,
        std::shared_ptr<UIExtensionCallback> uiExtensionCallback);

    /**
     * @brief Connects the current ServiceExtensionAbility to an ability using
     * the AbilityInfo.AbilityType.SERVICE template.
     *
     * @param want Indicates the want containing information about the ability to connect
     *
     * @param conn Indicates the callback object when the target ability is connected.
     *
     * @return Returns zero on success, others on failure.
     */
    ErrCode ConnectServiceExtensionAbility(
        const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const;

            /**
     * @brief Disconnects the current ServiceExtensionAbility from an ability.
     *
     * @param conn Indicates the IAbilityConnection callback object passed by connectAbility after the connection
     * is set up. The IAbilityConnection object uniquely identifies a connection between two abilities.
     *
     * @return errCode ERR_OK on success, others on failure.
     */
    ErrCode DisConnectServiceExtensionAbility(const AAFwk::Want &want,
        const sptr<AbilityConnectCallback> &connectCallback, int32_t accountId = -1) const;

    /**
     * @brief Get ui content object.
     *
     * @return UIContent object of ACE.
     */
    Ace::UIContent *GetUIContent();
    using SelfType = UIServiceExtensionContext;
    static const size_t CONTEXT_TYPE_ID;

protected:
    bool IsContext(size_t contextTypeId) override
    {
        return contextTypeId == CONTEXT_TYPE_ID || ExtensionContext::IsContext(contextTypeId);
    }

private:
    static int ILLEGAL_REQUEST_CODE;
    sptr<Rosen::Window> window_ = nullptr;
    Ace::ModalUIExtensionCallbacks SetupModalUIExtensionCallbacks(
        std::shared_ptr<UIExtensionCallback> uiExtensionCallback,
        std::shared_ptr<std::atomic<bool>> errorFired);
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_UI_SERVICE_EXTENSION_CONTEXT_H
