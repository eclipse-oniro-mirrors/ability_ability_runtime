/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_UKEY_AUTH_UI_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_UKEY_AUTH_UI_EXTENSION_CONTEXT_H

#include <functional>
#include <memory>

#include "configuration.h"
#include "extension_context.h"
#include "session_info.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class SessionInfo;
}
namespace Rosen {
class Window;
}
namespace AbilityRuntime {

/**
 * @brief context supply for ukeyAuth UIExtension, with only terminate/report/colorMode capabilities.
 */
class UkeyAuthUIExtensionContext : public ExtensionContext {
public:
    using AbilityConfigUpdateCallback = std::function<void(AppExecFwk::Configuration &config)>;

    UkeyAuthUIExtensionContext() = default;
    ~UkeyAuthUIExtensionContext() override = default;

    void SetWindow(const sptr<Rosen::Window> &window);
    sptr<Rosen::Window> GetWindow() const;
    void SetSessionInfo(const sptr<AAFwk::SessionInfo> &sessionInfo);

    /**
     * @brief Destroys the current ukeyAuth extension ability, after reporting to certManager (placeholder).
     */
    ErrCode TerminateSelf();

    /**
     * @brief Destroys the current ukeyAuth extension ability with result, after reporting to certManager.
     */
    ErrCode TerminateSelfWithResult(int32_t resultCode, const AAFwk::Want &want);

    /**
     * @brief Reports drawn completed of the extension ability.
     */
    ErrCode ReportDrawnCompleted();

    /**
     * @brief Sets color mode of the extension ability.
     */
    void SetAbilityColorMode(int32_t colorMode);

    void RegisterAbilityConfigUpdateCallback(AbilityConfigUpdateCallback &&callback);
    std::shared_ptr<AppExecFwk::Configuration> GetAbilityConfiguration() const;
    void SetAbilityConfiguration(const AppExecFwk::Configuration &config);
    void SetAbilityResourceManager(std::shared_ptr<Global::Resource::ResourceManager> abilityResourceMgr);

private:
#ifdef SUPPORT_SCREEN
    sptr<Rosen::Window> uiWindow_ = nullptr;
#endif // SUPPORT_SCREEN
    sptr<AAFwk::SessionInfo> sessionInfo_ = nullptr;
    AbilityConfigUpdateCallback abilityConfigUpdateCallback_ = nullptr;
    std::shared_ptr<AppExecFwk::Configuration> abilityConfiguration_ = nullptr;
    std::shared_ptr<Global::Resource::ResourceManager> abilityResourceMgr_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UKEY_AUTH_UI_EXTENSION_CONTEXT_H
