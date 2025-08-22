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

#include "ability_context.h"

#include <cstring>
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t AbilityContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("AbilityContext"));

std::shared_ptr<AppExecFwk::Configuration> AbilityContext::GetConfiguration()
{
    return stageContext_ ? stageContext_->GetConfiguration() : nullptr;
}

std::shared_ptr<AppExecFwk::ApplicationInfo> AbilityContext::GetApplicationInfo() const
{
    return stageContext_ ? stageContext_->GetApplicationInfo() : nullptr;
}

std::shared_ptr<AppExecFwk::HapModuleInfo> AbilityContext::GetHapModuleInfo() const
{
    return stageContext_ ? stageContext_->GetHapModuleInfo() : nullptr;
}

std::shared_ptr<AppExecFwk::AbilityInfo> AbilityContext::GetAbilityInfo() const
{
    return abilityInfo_;
}

void AbilityContext::SetAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &info)
{
    abilityInfo_ = info;
}

Options AbilityContext::GetOptions()
{
    return options_;
}

void AbilityContext::SetOptions(const Options &options)
{
    options_ = options;

    TAG_LOGD(AAFwkTag::ABILITY_SIM, "bundleName:%{public}s", options.bundleName.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "moduleName:%{public}s", options.moduleName.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "modulePath:%{public}s", options.modulePath.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "resourcePath:%{public}s", options.resourcePath.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "debugPort:%{public}d", options.debugPort);
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "assetPath:%{public}s", options.assetPath.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "systemResourcePath:%{public}s", options.systemResourcePath.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "appResourcePath:%{public}s", options.appResourcePath.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "containerSdkPath:%{public}s", options.containerSdkPath.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "url:%{public}s", options.url.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "language:%{public}s", options.language.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "region:%{public}s", options.region.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "script:%{public}s", options.script.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "themeId:%{public}d", options.themeId);
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "deviceWidth:%{public}d", options.deviceWidth);
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "deviceHeight:%{public}d", options.deviceHeight);
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "isRound:%{public}d", options.isRound);
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "compatibleVersion:%{public}d", options.compatibleVersion);
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "installationFree:%{public}d", options.installationFree);
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "labelId:%{public}d", options.labelId);
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "compileMode:%{public}s", options.compileMode.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "pageProfile:%{public}s", options.pageProfile.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "targetVersion:%{public}d", options.targetVersion);
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "releaseType:%{public}s", options.releaseType.c_str());
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "enablePartialUpdate:%{public}d", options.enablePartialUpdate);
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "isComponentMode:%{public}d", options.isComponentMode);
}

std::string AbilityContext::GetBundleName() const
{
    return stageContext_ ? stageContext_->GetBundleName() : "";
}

std::string AbilityContext::GetBundleCodePath()
{
    return stageContext_ ? stageContext_->GetBundleCodePath() : "";
}

std::string AbilityContext::GetBundleCodeDir()
{
    return stageContext_ ? stageContext_->GetBundleCodeDir() : "";
}

std::string AbilityContext::GetCacheDir()
{
    return stageContext_ ? stageContext_->GetCacheDir() : "";
}

std::string AbilityContext::GetTempDir()
{
    return stageContext_ ? stageContext_->GetTempDir() : "";
}

std::string AbilityContext::GetResourceDir()
{
    return stageContext_ ? stageContext_->GetResourceDir() : "";
}

std::string AbilityContext::GetFilesDir()
{
    return stageContext_ ? stageContext_->GetFilesDir() : "";
}

std::string AbilityContext::GetDatabaseDir()
{
    return stageContext_ ? stageContext_->GetDatabaseDir() : "";
}

std::string AbilityContext::GetPreferencesDir()
{
    return stageContext_ ? stageContext_->GetPreferencesDir() : "";
}

std::string AbilityContext::GetDistributedFilesDir()
{
    return stageContext_ ? stageContext_->GetDistributedFilesDir() : "";
}

std::string AbilityContext::GetCloudFileDir()
{
    return stageContext_ ? stageContext_->GetCloudFileDir() : "";
}

void AbilityContext::SwitchArea(int mode)
{
    if (stageContext_) {
        stageContext_->SwitchArea(mode);
    }
}

int AbilityContext::GetArea()
{
    return stageContext_ ? stageContext_->GetArea() : EL_DEFAULT;
}

std::string AbilityContext::GetBaseDir()
{
    return stageContext_ ? stageContext_->GetBaseDir() : "";
}

std::shared_ptr<Global::Resource::ResourceManager> AbilityContext::GetResourceManager() const
{
    return stageContext_ ? stageContext_->GetResourceManager() : nullptr;
}

void AbilityContext::SetResourceManager(const std::shared_ptr<Global::Resource::ResourceManager> &resMgr)
{
    if (stageContext_ != nullptr) {
        stageContext_->SetResourceManager(resMgr);
    }
}

std::shared_ptr<Context> AbilityContext::CreateModuleContext(const std::string &moduleName)
{
    return stageContext_ ? stageContext_->CreateModuleContext(moduleName) : nullptr;
}

std::shared_ptr<Context> AbilityContext::CreateModuleContext(
    const std::string &bundleName, const std::string &moduleName)
{
    return stageContext_ ? stageContext_->CreateModuleContext(bundleName, moduleName) : nullptr;
}

void AbilityContext::SetAbilityStageContext(const std::shared_ptr<AbilityStageContext> &stageContext)
{
    stageContext_ = stageContext;
}

bool AbilityContext::IsTerminating()
{
    return isTerminating_;
}

void AbilityContext::SetTerminating(const bool &state)
{
    isTerminating_ = state;
}

int32_t AbilityContext::TerminateSelf()
{
    if (simulator_ != nullptr) {
        simulator_->TerminateAbility(0);
    }
    return 0;
}

void AbilityContext::SetSimulator(Simulator *simulator)
{
    simulator_ = simulator;
}
} // namespace AbilityRuntime
} // namespace OHOS
