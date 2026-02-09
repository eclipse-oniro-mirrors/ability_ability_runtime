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

#include "ability_stage_context.h"

#include "context.h"
#include "context_impl.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t AbilityStageContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("AbilityStageContext"));

AbilityStageContext::AbilityStageContext()
{
    contextImpl_ = std::make_shared<ContextImpl>();
}

void AbilityStageContext::SetParentContext(const std::shared_ptr<Context> &context)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return;
    }

    contextImpl_->SetParentContext(context);
}

void AbilityStageContext::InitHapModuleInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return;
    }

    contextImpl_->InitHapModuleInfo(abilityInfo);
}

void AbilityStageContext::InitPluginHapModuleInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo,
    const std::string &hostBundleName)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return;
    }

    contextImpl_->InitPluginHapModuleInfo(abilityInfo, hostBundleName);
}

void AbilityStageContext::SetIsPlugin(bool isPlugin)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return;
    }
    contextImpl_->isPlugin_ = isPlugin;
}

void AbilityStageContext::InitHapModuleInfo(const AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return;
    }

    contextImpl_->InitHapModuleInfo(hapModuleInfo);
}

std::shared_ptr<AppExecFwk::HapModuleInfo> AbilityStageContext::GetHapModuleInfo() const
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return nullptr;
    }

    return contextImpl_->GetHapModuleInfo();
}

void AbilityStageContext::SetConfiguration(const std::shared_ptr<AppExecFwk::Configuration> &config)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return;
    }

    contextImpl_->SetConfiguration(config);
}

std::shared_ptr<AppExecFwk::Configuration> AbilityStageContext::GetConfiguration() const
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return nullptr;
    }

    return contextImpl_->GetConfiguration();
}

void AbilityStageContext::SetResourceManager(const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return;
    }

    contextImpl_->SetResourceManager(resourceManager);
}

std::shared_ptr<Global::Resource::ResourceManager> AbilityStageContext::GetResourceManager() const
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return nullptr;
    }

    return contextImpl_->GetResourceManager();
}

std::string AbilityStageContext::GetBundleName() const
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return {};
    }

    return contextImpl_->GetBundleName();
}

std::shared_ptr<AppExecFwk::ApplicationInfo> AbilityStageContext::GetApplicationInfo() const
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return nullptr;
    }

    return contextImpl_->GetApplicationInfo();
}

std::shared_ptr<Context> AbilityStageContext::CreatePluginContext(const std::string &pluginBundleName,
    const std::string &moduleName, std::shared_ptr<Context> inputContext)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return nullptr;
    }

    return contextImpl_->CreatePluginContext(pluginBundleName, moduleName, inputContext);
}

std::shared_ptr<Context> AbilityStageContext::CreateBundleContext(const std::string &bundleName)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return nullptr;
    }

    return contextImpl_->CreateBundleContext(bundleName);
}

std::shared_ptr<Context> AbilityStageContext::CreateModuleContext(const std::string &moduleName)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return nullptr;
    }

    return contextImpl_->CreateModuleContext(moduleName);
}

std::shared_ptr<Context> AbilityStageContext::CreateModuleContext(const std::string &bundleName,
    const std::string &moduleName)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return nullptr;
    }

    return contextImpl_->CreateModuleContext(bundleName, moduleName);
}

std::shared_ptr<Global::Resource::ResourceManager> AbilityStageContext::CreateModuleResourceManager(
    const std::string &bundleName, const std::string &moduleName)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return nullptr;
    }

    return contextImpl_->CreateModuleResourceManager(bundleName, moduleName);
}

int32_t AbilityStageContext::CreateSystemHspModuleResourceManager(const std::string &bundleName,
    const std::string &moduleName, std::shared_ptr<Global::Resource::ResourceManager> &resourceManager)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return ERR_INVALID_VALUE;
    }

    return contextImpl_->CreateSystemHspModuleResourceManager(bundleName, moduleName, resourceManager);
}

std::string AbilityStageContext::GetBundleCodePath() const
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return {};
    }

    return contextImpl_->GetBundleCodePath();
}

std::string AbilityStageContext::GetBundleCodeDir()
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return {};
    }

    return contextImpl_->GetBundleCodeDir();
}

std::string AbilityStageContext::GetCacheDir()
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return {};
    }

    return contextImpl_->GetCacheDir();
}

std::string AbilityStageContext::GetTempDir()
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return {};
    }

    return contextImpl_->GetTempDir();
}

std::string AbilityStageContext::GetFilesDir()
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return {};
    }

    return contextImpl_->GetFilesDir();
}

std::string AbilityStageContext::GetResourceDir(const std::string &moduleName)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return {};
    }

    return contextImpl_->GetResourceDir(moduleName);
}

std::string AbilityStageContext::GetDatabaseDir()
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return {};
    }

    return contextImpl_->GetDatabaseDir();
}

std::string AbilityStageContext::GetPreferencesDir()
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return {};
    }

    return contextImpl_->GetPreferencesDir();
}

std::string AbilityStageContext::GetGroupDir(std::string groupId)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return {};
    }

    return contextImpl_->GetGroupDir(groupId);
}

int32_t AbilityStageContext::GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return ERR_INVALID_VALUE;
    }

    return contextImpl_->GetSystemDatabaseDir(groupId, checkExist, databaseDir);
}

int32_t AbilityStageContext::GetSystemPreferencesDir(const std::string &groupId, bool checkExist,
    std::string &preferencesDir)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return ERR_INVALID_VALUE;
    }

    return contextImpl_->GetSystemPreferencesDir(groupId, checkExist, preferencesDir);
}

std::string AbilityStageContext::GetDistributedFilesDir()
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return {};
    }

    return contextImpl_->GetDistributedFilesDir();
}

std::string AbilityStageContext::GetCloudFileDir()
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return {};
    }

    return contextImpl_->GetCloudFileDir();
}

std::string AbilityStageContext::GetLogFileDir()
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return {};
    }

    return contextImpl_->GetLogFileDir();
}

std::string AbilityStageContext::GetBaseDir() const
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return {};
    }

    return contextImpl_->GetBaseDir();
}

bool AbilityStageContext::IsUpdatingConfigurations()
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return false;
    }

    return contextImpl_->IsUpdatingConfigurations();
}

bool AbilityStageContext::PrintDrawnCompleted()
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return false;
    }

    return contextImpl_->PrintDrawnCompleted();
}

sptr<IRemoteObject> AbilityStageContext::GetToken()
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return nullptr;
    }

    return contextImpl_->GetToken();
}

void AbilityStageContext::SetToken(const sptr<IRemoteObject> &token)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return;
    }

    contextImpl_->SetToken(token);
}

void AbilityStageContext::SwitchArea(int mode)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return;
    }

    contextImpl_->SwitchArea(mode);
}

int AbilityStageContext::GetArea()
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return ContextImpl::EL_DEFAULT;
    }

    return contextImpl_->GetArea();
}

std::string AbilityStageContext::GetProcessName()
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Invalid contextImpl");
        return {};
    }

    return contextImpl_->GetProcessName();
}

void AbilityStageContext::SetProcessName(const std::string &processName)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Invalid contextImpl");
        return;
    }

    contextImpl_->SetProcessName(processName);
}

Global::Resource::DeviceType AbilityStageContext::GetDeviceType() const
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null contextImpl");
        return Global::Resource::DeviceType::DEVICE_PHONE;
    }

    return contextImpl_->GetDeviceType();
}

std::shared_ptr<Context> AbilityStageContext::CreateAreaModeContext(int areaMode)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid contextImpl");
        return nullptr;
    }

    return contextImpl_->CreateAreaModeContext(areaMode);
}

std::shared_ptr<Context> AbilityStageContext::CreateModuleOrPluginContext(const std::string &bundleName,
    const std::string &moduleName)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid contextImpl");
        return nullptr;
    }

    return contextImpl_->CreateModuleOrPluginContext(bundleName, moduleName);
}

#ifdef SUPPORT_GRAPHICS
std::shared_ptr<Context> AbilityStageContext::CreateDisplayContext(uint64_t displayId)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid contextImpl");
        return nullptr;
    }

    return contextImpl_->CreateDisplayContext(displayId);
}
#endif
} // namespace AbilityRuntime
} // namespace OHOS
