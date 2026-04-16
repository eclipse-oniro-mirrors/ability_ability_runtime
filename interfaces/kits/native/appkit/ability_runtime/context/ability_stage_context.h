/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_STAGE_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_ABILITY_STAGE_CONTEXT_H

#include "context.h"
#include "want.h"
#include "context_impl.h"

namespace OHOS {
namespace AbilityRuntime {
class AbilityStageContext : public Context {
public:
    AbilityStageContext();
    virtual ~AbilityStageContext() = default;

    void SetParentContext(const std::shared_ptr<Context> &context);

    void InitHapModuleInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo);
    void InitHapModuleInfo(const AppExecFwk::HapModuleInfo &hapModuleInfo);
    void InitPluginHapModuleInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo,
        const std::string &hostBundleName);
    std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo() const override;

    void SetConfiguration(const std::shared_ptr<AppExecFwk::Configuration> &config);
    std::shared_ptr<AppExecFwk::Configuration> GetConfiguration() const override;

    void SetResourceManager(const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager);
    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override;

    std::string GetBundleName() const override;
    std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const override;

    std::shared_ptr<Context> CreatePluginContext(const std::string &pluginBundleName,
        const std::string &moduleName, std::shared_ptr<Context> inputContext);
    std::shared_ptr<Context> CreateBundleContext(const std::string &bundleName) override;
    std::shared_ptr<Context> CreateModuleContext(const std::string &moduleName) override;
    std::shared_ptr<Context> CreateModuleContext(const std::string &bundleName, const std::string &moduleName) override;
    std::shared_ptr<Global::Resource::ResourceManager> CreateModuleResourceManager(
        const std::string &bundleName, const std::string &moduleName) override;
    int32_t CreateSystemHspModuleResourceManager(const std::string &bundleName,
        const std::string &moduleName, std::shared_ptr<Global::Resource::ResourceManager> &resourceManager) override;
    std::shared_ptr<Context> CreateAreaModeContext(int areaMode) override;
    std::shared_ptr<Context> CreateModuleOrPluginContext(const std::string &bundleName,
        const std::string &moduleName) override;
#ifdef SUPPORT_GRAPHICS
    std::shared_ptr<Context> CreateDisplayContext(uint64_t displayId) override;
#endif

    std::string GetBundleCodePath() const override;
    std::string GetBundleCodeDir() override;
    std::string GetCacheDir() override;
    std::string GetTempDir() override;
    std::string GetFilesDir() override;
    std::string GetResourceDir(const std::string &moduleName = "") override;
    std::string GetDatabaseDir() override;
    std::string GetPreferencesDir() override;
    std::string GetGroupDir(std::string groupId) override;
    std::string GetDistributedFilesDir() override;
    std::string GetCloudFileDir() override;
    std::string GetBaseDir() const override;
    std::string GetLogFileDir() override;
    int32_t GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir) override;
    int32_t GetSystemPreferencesDir(const std::string &groupId, bool checkExist, std::string &preferencesDir) override;

    bool IsUpdatingConfigurations() override;
    bool PrintDrawnCompleted() override;
    void SetIsPlugin(bool isPlugin);

    sptr<IRemoteObject> GetToken() override;
    void SetToken(const sptr<IRemoteObject> &token) override;

    void SwitchArea(int mode) override;
    int GetArea() override;

    std::string GetProcessName() override;
    void SetProcessName(const std::string &processName);

    void SetLaunchElement(const AppExecFwk::ElementName &elementName);
    const AppExecFwk::ElementName &GetLaunchElement() const;

    Global::Resource::DeviceType GetDeviceType() const override;

    using SelfType = AbilityStageContext;
    static const size_t CONTEXT_TYPE_ID;

protected:
    bool IsContext(size_t contextTypeId) override
    {
        return contextTypeId == CONTEXT_TYPE_ID || Context::IsContext(contextTypeId);
    }

private:
    std::shared_ptr<ContextImpl> contextImpl_;
    AppExecFwk::ElementName launchElement_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_STAGE_CONTEXT_H
