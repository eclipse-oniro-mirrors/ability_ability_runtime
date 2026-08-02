/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "mock_context_impl.h"
#include "sys_mgr_client.h"
#include "configuration.h"

namespace OHOS {
namespace AbilityRuntime {

std::string MockContextImpl::GetBundleName() const
{
    return "com.test.bundleName";
}

std::string MockContextImpl::GetBundleCodeDir()
{
    return "/code";
}

std::string MockContextImpl::GetCacheDir()
{
    return "/cache";
}

bool MockContextImpl::IsUpdatingConfigurations()
{
    return true;
}

bool MockContextImpl::PrintDrawnCompleted()
{
    return true;
}

std::string MockContextImpl::GetDatabaseDir()
{
    return "/data/app/database";
}

std::string MockContextImpl::GetPreferencesDir()
{
    return "/preferences";
}

std::string MockContextImpl::GetTempDir()
{
    return "/temp";
}

std::string MockContextImpl::GetResourceDir(const std::string &moduleName)
{
    return "/resfile";
}

std::string MockContextImpl::GetFilesDir()
{
    return "/files";
}

std::string MockContextImpl::GetDistributedFilesDir()
{
    return "/mnt/hmdfs/device_view/local/data/bundleName";
}

std::string MockContextImpl::GetCloudFileDir()
{
    return "/cloud";
}

std::string MockContextImpl::GetLogFileDir()
{
    return "/log";
}

std::string MockContextImpl::GetGroupDir(std::string groupId)
{
    return "/group";
}

int32_t MockContextImpl::GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir)
{
    return 0;
}

int32_t MockContextImpl::GetSystemPreferencesDir(const std::string &groupId, bool checkExist,
    std::string &preferencesDir)
{
    return 0;
}

std::shared_ptr<Context> MockContextImpl::CreateModuleContext(const std::string &moduleName)
{
    std::shared_ptr<ContextImpl> appContext = std::make_shared<ContextImpl>();
    return appContext;
}

std::shared_ptr<Context> MockContextImpl::CreateModuleContext(const std::string &bundleName,
    const std::string &moduleName)
{
    std::shared_ptr<ContextImpl> appContext = std::make_shared<ContextImpl>();
    return appContext;
}

int32_t MockContextImpl::CreateSystemHspModuleResourceManager(const std::string &bundleName,
    const std::string &moduleName, std::shared_ptr<Global::Resource::ResourceManager> &resourceManager)
{
    return 0;
}

std::shared_ptr<Global::Resource::ResourceManager> MockContextImpl::CreateModuleResourceManager(
    const std::string &bundleName, const std::string &moduleName)
{
    return nullptr;
}

int MockContextImpl::GetArea()
{
    return 1;
}

std::string MockContextImpl::GetBaseDir() const
{
    return "/data/app/base";
}

std::shared_ptr<Context> MockContextImpl::CreateBundleContext(const std::string &bundleName)
{
    std::shared_ptr<ContextImpl> appContext = std::make_shared<ContextImpl>();
    return appContext;
}

sptr<AppExecFwk::IBundleMgr> MockContextImpl::GetBundleManager() const
{
    auto instance = OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    if (instance == nullptr) {
        return nullptr;
    }
    auto bundleObj = instance->GetSystemAbility(401);
    if (bundleObj == nullptr) {
        return nullptr;
    }
    sptr<AppExecFwk::IBundleMgr> bms = iface_cast<AppExecFwk::IBundleMgr>(bundleObj);
    return bms;
}

std::string MockContextImpl::GetBundleCodePath() const
{
    return "codePath";
}

void MockContextImpl::InitHapModuleInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo)
{}

Global::Resource::DeviceType MockContextImpl::GetDeviceType() const
{
    return Global::Resource::DeviceType::DEVICE_NOT_SET;
}

std::shared_ptr<AppExecFwk::ApplicationInfo> MockContextImpl::GetApplicationInfo() const
{
    std::shared_ptr<AppExecFwk::ApplicationInfo> info = std::make_shared<AppExecFwk::ApplicationInfo>();
    info->name = "ContextTest";
    return info;
}

std::shared_ptr<Global::Resource::ResourceManager> MockContextImpl::GetResourceManager() const
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    return resourceManager;
}

std::shared_ptr<AppExecFwk::Configuration> MockContextImpl::GetConfiguration() const
{
    std::shared_ptr<AppExecFwk::Configuration> config = std::make_shared<AppExecFwk::Configuration>();
    return config;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
