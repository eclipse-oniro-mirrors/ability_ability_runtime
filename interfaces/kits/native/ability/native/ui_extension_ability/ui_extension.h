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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_H

#include <mutex>
#include <unordered_set>

#include "extension_base.h"

namespace OHOS {
namespace AbilityRuntime {
class UIExtensionContext;
class Runtime;
/**
 * @brief Basic ui extension components.
 */
class UIExtension : public ExtensionBase<UIExtensionContext> {
public:
    UIExtension() = default;
    virtual ~UIExtension() = default;

    /**
     * @brief Create and init context.
     *
     * @param record the ui extension record.
     * @param application the application info.
     * @param handler the ui extension handler.
     * @param token the remote token.
     * @return The created context.
     */
    virtual std::shared_ptr<UIExtensionContext> CreateAndInitContext(
        const std::shared_ptr<AbilityLocalRecord> &record,
        const std::shared_ptr<OHOSApplication> &application,
        std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    /**
     * @brief Init the ui extension.
     *
     * @param record the ui extension record.
     * @param application the application info.
     * @param handler the ui extension handler.
     * @param token the remote token.
     */
    virtual void Init(const std::shared_ptr<AbilityLocalRecord> &record,
        const std::shared_ptr<OHOSApplication> &application,
        std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    /**
     * @brief Create ui extension.
     *
     * @param runtime The runtime.
     * @return The ui extension instance.
     */
    static UIExtension* Create(const std::unique_ptr<Runtime>& runtime);

    virtual void OnStopCallBack() override;

    virtual void OnCommand(const AAFwk::Want &want, bool restart, int startId) override;

    virtual void OnCommandWindow(
        const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd) override;

    virtual void OnCommandWindowDone(const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd) override;

    virtual void OnInsightIntentExecuteDone(const sptr<AAFwk::SessionInfo> &sessionInfo,
        const AppExecFwk::InsightIntentExecuteResult &result) override;

    void OnConfigurationUpdated(const AppExecFwk::Configuration& configuration) override;

    virtual void ConfigurationUpdated();

    virtual void OnAbilityConfigurationUpdated(const AppExecFwk::Configuration &configuration);

    void RegisterAbilityConfigUpdateCallback();

    void RegisterUiExtensionDelayResultCallback(uint64_t intentId, const sptr<AAFwk::SessionInfo> &sessionInfo,
        bool isDecorator = false);

    /**
     * @brief Create modal UIExtension.
     * @param want The want of the modal UIExtension to create.
     * @return Returns ERR_OK on success, error code on failure.
     */
    virtual int CreateModalUIExtension(const AAFwk::Want &want);

protected:
    virtual void ForegroundWindow(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo);
    virtual void BackgroundWindow(const sptr<AAFwk::SessionInfo> &sessionInfo);
    virtual void DestroyWindow(const sptr<AAFwk::SessionInfo> &sessionInfo);
    virtual bool ForegroundWindowWithInsightIntent(
        const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, bool needForeground);
    virtual bool HandleSessionCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo);
protected:
    std::mutex uiWindowMutex_;
    std::map<uint64_t, sptr<Rosen::Window>> uiWindowMap_;
    std::set<uint64_t> foregroundWindows_;
    uint64_t intentId_;
    std::shared_ptr<AbilityHandler> handler_ = nullptr;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_UI_EXTENSION_H
