/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_MOCK_CONTEXT_IMPL_H
#define OHOS_ABILITY_RUNTIME_MOCK_CONTEXT_IMPL_H

#include "bundle_mgr_interface.h"
#include "context_impl.h"

namespace OHOS {
namespace AbilityRuntime {
class MockContextImpl : public ContextImpl {
public:
    MockContextImpl() = default;
    virtual ~MockContextImpl() = default;

    std::string GetBundleName() const override;

    std::string GetBundleCodeDir() override;

    std::string GetCacheDir() override;

    bool IsUpdatingConfigurations() override;

    bool PrintDrawnCompleted() override;

    std::string GetTempDir() override;

    std::string GetResourceDir(const std::string &moduleName = "") override;

    std::string GetFilesDir() override;

    std::string GetDatabaseDir() override;

    std::string GetPreferencesDir() override;

    int32_t GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir) override;

    int32_t GetSystemPreferencesDir(const std::string &groupId, bool checkExist, std::string &preferencesDir) override;

    std::string GetDistributedFilesDir() override;

    std::string GetCloudFileDir() override;

    std::string GetLogFileDir() override;

    std::shared_ptr<Context> CreateModuleContext(const std::string &moduleName) override;

    std::shared_ptr<Context> CreateModuleContext(const std::string &bundleName, const std::string &moduleName) override;

    int32_t CreateSystemHspModuleResourceManager(const std::string &bundleName,
        const std::string &moduleName, std::shared_ptr<Global::Resource::ResourceManager> &resourceManager) override;

    int GetArea() override;

    std::shared_ptr<Context> CreateBundleContext(const std::string &bundleName) override;

    std::shared_ptr<Global::Resource::ResourceManager> CreateModuleResourceManager(
        const std::string &bundleName, const std::string &moduleName) override;

    sptr<AppExecFwk::IBundleMgr> GetBundleManager() const;

    void SetApplicationInfo(const std::shared_ptr<AppExecFwk::ApplicationInfo> &info);

    void SetParentContext(const std::shared_ptr<Context> &context);

    std::string GetBundleCodePath() const override;

    void InitHapModuleInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo);

    std::string GetBaseDir() const override;

    Global::Resource::DeviceType GetDeviceType() const override;

    std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const override;

    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override;

    std::shared_ptr<AppExecFwk::Configuration> GetConfiguration() const override;

    std::string GetGroupDir(std::string groupId) override;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MOCK_CONTEXT_IMPL_H
