/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "context_deal.h"

#include <regex>

#include "ability_manager_client.h"
#include "ability_manager_interface.h"
#include "app_context.h"
#include "bundle_mgr_helper.h"
#include "constants.h"
#include "directory_ex.h"
#include "file_ex.h"
#include "hilog_tag_wrapper.h"
#include "iservice_registry.h"
#include "os_account_manager_wrapper.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"

#define MODE 0771
namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::AbilityBase::Constants;

const std::string ContextDeal::CONTEXT_DEAL_FILE_SEPARATOR("/");
const std::string ContextDeal::CONTEXT_DEAL_Files("files");
const int64_t ContextDeal::CONTEXT_CREATE_BY_SYSTEM_APP(0x00000001);
const std::string ContextDeal::CONTEXT_DATA_STORAGE("/data/storage/");
const std::string ContextDeal::CONTEXT_DEAL_DATA_APP("/data/app/");
const std::string ContextDeal::CONTEXT_DEAL_BASE("base");
const std::string ContextDeal::CONTEXT_DEAL_DATABASE("database");
const std::string ContextDeal::CONTEXT_DEAL_PREFERENCES("preferences");
const std::string ContextDeal::CONTEXT_DEAL_DATA("data");

ContextDeal::ContextDeal(bool isCreateBySystemApp) : isCreateBySystemApp_(isCreateBySystemApp)
{}

std::shared_ptr<ApplicationInfo> ContextDeal::GetApplicationInfo() const
{
    return applicationInfo_;
}

void ContextDeal::SetApplicationInfo(const std::shared_ptr<ApplicationInfo> &info)
{
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null info");
        return;
    }
    applicationInfo_ = info;
}

std::shared_ptr<Context> ContextDeal::GetApplicationContext() const
{
    return appContext_;
}

void ContextDeal::SetApplicationContext(const std::shared_ptr<Context> &context)
{
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return;
    }
    appContext_ = context;
}

std::string ContextDeal::GetBundleCodePath()
{
    if (applicationInfo_ == nullptr) {
        return "";
    }

    std::string dir;
    if (isCreateBySystemApp_) {
        dir = std::regex_replace(applicationInfo_->codePath, std::regex(ABS_CODE_PATH), LOCAL_BUNDLES);
    } else {
        dir = LOCAL_CODE_PATH;
    }

    return dir;
}

void ContextDeal::SetBundleCodePath(std::string &path)
{
    path_ = path;
}

const std::shared_ptr<AbilityInfo> ContextDeal::GetAbilityInfo()
{
    return abilityInfo_;
}

void ContextDeal::SetAbilityInfo(const std::shared_ptr<AbilityInfo> &info)
{
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null info");
        return;
    }
    abilityInfo_ = info;
}

std::shared_ptr<Context> ContextDeal::GetContext()
{
    return abilityContext_;
}

void ContextDeal::SetContext(const std::shared_ptr<Context> &context)
{
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return;
    }
    abilityContext_ = context;
}

std::shared_ptr<BundleMgrHelper> ContextDeal::GetBundleManager() const
{
    auto bundleMgrHelper = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null bundleMgrHelper");
        return nullptr;
    }
    return bundleMgrHelper;
}

std::shared_ptr<Global::Resource::ResourceManager> ContextDeal::GetResourceManager() const
{
    return resourceManager_;
}

std::string ContextDeal::GetDatabaseDir()
{
    std::string dir;
    if (IsCreateBySystemApp()) {
        dir = CONTEXT_DEAL_DATA_APP + currArea_ + CONTEXT_DEAL_FILE_SEPARATOR + std::to_string(GetCurrentAccountId())
            + CONTEXT_DEAL_FILE_SEPARATOR + CONTEXT_DEAL_DATABASE + CONTEXT_DEAL_FILE_SEPARATOR + GetBundleName();
    } else {
        dir = CONTEXT_DATA_STORAGE + currArea_ + CONTEXT_DEAL_FILE_SEPARATOR + CONTEXT_DEAL_DATABASE;
    }
    CreateDirIfNotExist(dir);
    TAG_LOGD(AAFwkTag::APPKIT, "GetDatabaseDir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextDeal::GetDataDir()
{
    std::string dir = GetBaseDir() + CONTEXT_DEAL_FILE_SEPARATOR + CONTEXT_DEAL_DATA;
    CreateDirIfNotExist(dir);
    TAG_LOGD(AAFwkTag::APPKIT, "GetDataDir dir = %{public}s", dir.c_str());
    return dir;
}

std::string ContextDeal::GetDir(const std::string &name, int mode)
{
    if (applicationInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationInfo_");
        return "";
    }
    std::string dir = applicationInfo_->dataDir + CONTEXT_DEAL_FILE_SEPARATOR + name;
    if (!OHOS::FileExists(dir)) {
        TAG_LOGI(AAFwkTag::APPKIT, "GetDir File not exits");
        OHOS::ForceCreateDirectory(dir);
        OHOS::ChangeModeDirectory(dir, mode);
    }
    return dir;
}

std::string ContextDeal::GetFilesDir()
{
    std::string dir = GetBaseDir() + CONTEXT_DEAL_FILE_SEPARATOR + CONTEXT_DEAL_Files;
    CreateDirIfNotExist(dir);
    TAG_LOGD(AAFwkTag::APPKIT, "GetFilesDir dir = %{public}s", dir.c_str());
    return dir;
}

std::string ContextDeal::GetBundleName() const
{
    return (applicationInfo_ != nullptr) ? applicationInfo_->bundleName : "";
}

std::string ContextDeal::GetBundleResourcePath()
{
    if (abilityInfo_ == nullptr) {
        return "";
    }

    std::string dir;
    if (isCreateBySystemApp_) {
        dir = std::regex_replace(abilityInfo_->resourcePath, std::regex(ABS_CODE_PATH), LOCAL_BUNDLES);
    } else {
        std::regex pattern(std::string(ABS_CODE_PATH) + std::string(FILE_SEPARATOR) + abilityInfo_->bundleName);
        dir = std::regex_replace(abilityInfo_->resourcePath, pattern, LOCAL_CODE_PATH);
    }
    return dir;
}

sptr<AAFwk::IAbilityManager> ContextDeal::GetAbilityManager()
{
    auto remoteObject = OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null remoteObject");
        return nullptr;
    }
    sptr<AAFwk::IAbilityManager> ams = iface_cast<AAFwk::IAbilityManager>(remoteObject);
    return ams;
}

std::string ContextDeal::GetAppType()
{
    auto ptr = GetBundleManager();
    if (ptr == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ptr");
        return "";
    }
    std::string retString = ptr->GetAppType(applicationInfo_->bundleName);
    return retString;
}

bool ContextDeal::IsCreateBySystemApp() const
{
    return (static_cast<uint64_t>(flags_) & static_cast<uint64_t>(CONTEXT_CREATE_BY_SYSTEM_APP)) == 1;
}

int ContextDeal::GetCurrentAccountId() const
{
    int userId = 0;
    OsAccountManagerWrapper::GetOsAccountLocalIdFromProcess(userId);
    TAG_LOGD(AAFwkTag::APPKIT, "userId: %{public}d", userId);
    return userId;
}

void ContextDeal::CreateDirIfNotExist(const std::string &dirPath) const
{
    if (!OHOS::FileExists(dirPath)) {
        TAG_LOGD(AAFwkTag::APPKIT, "CreateDirIfNotExist File is not exits");
        bool createDir = OHOS::ForceCreateDirectory(dirPath);
        if (!createDir) {
            TAG_LOGI(AAFwkTag::APPKIT, "CreateDirIfNotExist: create dir %{public}s failed.", dirPath.c_str());
            return;
        }
    }
}

void ContextDeal::SetPattern(int patternId)
{
    if (resourceManager_ != nullptr) {
        if (!pattern_.empty()) {
            pattern_.clear();
        }
        OHOS::Global::Resource::RState errval = resourceManager_->GetPatternById(patternId, pattern_);
        if (errval != OHOS::Global::Resource::RState::SUCCESS) {
            TAG_LOGE(AAFwkTag::APPKIT, "SetPattern GetPatternById(patternId:%{public}d) retval: %{public}u",
                     patternId, errval);
        }
    } else {
        TAG_LOGE(AAFwkTag::APPKIT, "null resourceManager_");
    }
}

std::shared_ptr<HapModuleInfo> ContextDeal::GetHapModuleInfo()
{
    // fix set HapModuleInfoLocal data failed, request only once
    if (hapModuleInfoLocal_ == nullptr) {
        HapModuleInfoRequestInit();
        if (hapModuleInfoLocal_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null hapModuleInfoLocal_");
            return nullptr;
        }
    }
    return hapModuleInfoLocal_;
}

void ContextDeal::initResourceManager(const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager)
{
    resourceManager_ = resourceManager;
}

std::string ContextDeal::GetString(int resId)
{
    if (resourceManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null resourceManager_");
        return "";
    }

    std::string ret;
    OHOS::Global::Resource::RState errval = resourceManager_->GetStringById(resId, ret);
    if (errval == OHOS::Global::Resource::RState::SUCCESS) {
        return ret;
    } else {
        TAG_LOGE(AAFwkTag::APPKIT, "GetStringById(resId:%{public}d) retval: %{public}u", resId, errval);
        return "";
    }
}

std::vector<std::string> ContextDeal::GetStringArray(int resId)
{
    if (resourceManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null resourceManager_");
        return std::vector<std::string>();
    }

    std::vector<std::string> retv;
    OHOS::Global::Resource::RState errval = resourceManager_->GetStringArrayById(resId, retv);
    if (errval == OHOS::Global::Resource::RState::SUCCESS) {
        return retv;
    } else {
        TAG_LOGE(AAFwkTag::APPKIT, "GetStringArrayById(resId:%{public}d) retval: %{public}u", resId, errval);
        return std::vector<std::string>();
    }
}

std::vector<int> ContextDeal::GetIntArray(int resId)
{
    if (resourceManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null resourceManager_");
        return std::vector<int>();
    }

    std::vector<int> retv;
    OHOS::Global::Resource::RState errval = resourceManager_->GetIntArrayById(resId, retv);
    if (errval == OHOS::Global::Resource::RState::SUCCESS) {
        return retv;
    } else {
        TAG_LOGE(AAFwkTag::APPKIT, "GetIntArrayById(resId:%{public}d) retval: %{public}u", resId, errval);
        return std::vector<int>();
    }
}

std::map<std::string, std::string> ContextDeal::GetTheme()
{
    if (theme_.empty()) {
        SetTheme(GetThemeId());
    }
    return theme_;
}

void ContextDeal::SetTheme(int themeId)
{
    if (resourceManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null resourceManager_");
        return;
    }

    auto hapModInfo = GetHapModuleInfo();
    if (hapModInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null hapModInfo");
        return;
    }

    if (!theme_.empty()) {
        theme_.clear();
    }
    OHOS::Global::Resource::RState errval = resourceManager_->GetThemeById(themeId, theme_);
    if (errval != OHOS::Global::Resource::RState::SUCCESS) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetThemeById(themeId:%{public}d) retval: %{public}u", themeId, errval);
    }
}

std::map<std::string, std::string> ContextDeal::GetPattern()
{
    if (!pattern_.empty()) {
        return pattern_;
    } else {
        TAG_LOGE(AAFwkTag::APPKIT, "pattern_ empty");
        return std::map<std::string, std::string>();
    }
}

int ContextDeal::GetColor(int resId)
{
    if (resourceManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null resourceManager_");
        return INVALID_RESOURCE_VALUE;
    }

    uint32_t ret = INVALID_RESOURCE_VALUE;
    OHOS::Global::Resource::RState errval = resourceManager_->GetColorById(resId, ret);
    if (errval == OHOS::Global::Resource::RState::SUCCESS) {
        return ret;
    } else {
        TAG_LOGE(AAFwkTag::APPKIT, "GetColorById(resId:%{public}d) retval: %{public}u", resId, errval);
        return INVALID_RESOURCE_VALUE;
    }
}

int ContextDeal::GetThemeId()
{
    auto hapModInfo = GetHapModuleInfo();
    if (hapModInfo != nullptr) {
        return -1;
    } else {
        TAG_LOGE(AAFwkTag::APPKIT, "null hapModInfo");
        return -1;
    }
}

int ContextDeal::GetDisplayOrientation()
{
    if (abilityInfo_ != nullptr) {
        return static_cast<int>(abilityInfo_->orientation);
    } else {
        TAG_LOGE(AAFwkTag::APPKIT, "null abilityInfo_");
        return static_cast<int>(DisplayOrientation::UNSPECIFIED);
    }
}

std::string ContextDeal::GetPreferencesDir()
{
    std::string dir = GetBaseDir() + CONTEXT_DEAL_FILE_SEPARATOR + CONTEXT_DEAL_PREFERENCES;
    CreateDirIfNotExist(dir);
    TAG_LOGD(AAFwkTag::APPKIT, "GetPreferencesDir:%{public}s", dir.c_str());
    return dir;
}

void ContextDeal::SetColorMode(int mode)
{
    auto hapModInfo = GetHapModuleInfo();
    if (hapModInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null hapModInfo");
        return;
    }

    if (mode == static_cast<int>(ModuleColorMode::DARK)) {
        hapModInfo->colorMode = ModuleColorMode::DARK;
    } else if (mode == static_cast<int>(ModuleColorMode::LIGHT)) {
        hapModInfo->colorMode = ModuleColorMode::LIGHT;
    } else {  // default use AUTO
        hapModInfo->colorMode = ModuleColorMode::AUTO;
    }
}

int ContextDeal::GetColorMode()
{
    auto hapModInfo = GetHapModuleInfo();
    if (hapModInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null hapModInfo");
        return -1;
    }
    return static_cast<int>(hapModInfo->colorMode);
}


bool ContextDeal::HapModuleInfoRequestInit()
{
    auto ptr = GetBundleManager();
    if (ptr == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ptr");
        return false;
    }

    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null abilityInfo_");
        return false;
    }

    hapModuleInfoLocal_ = std::make_shared<HapModuleInfo>();
    if (!ptr->GetHapModuleInfo(*abilityInfo_.get(), *hapModuleInfoLocal_)) {
        TAG_LOGE(AAFwkTag::APPKIT, "will retval false");
        return false;
    }
    return true;
}

std::string ContextDeal::GetBaseDir() const
{
    std::string baseDir;
    if (IsCreateBySystemApp()) {
        baseDir = CONTEXT_DEAL_DATA_APP + currArea_ + CONTEXT_DEAL_FILE_SEPARATOR +
            std::to_string(GetCurrentAccountId()) + CONTEXT_DEAL_FILE_SEPARATOR + CONTEXT_DEAL_BASE +
            CONTEXT_DEAL_FILE_SEPARATOR + GetBundleName();
    } else {
        baseDir = CONTEXT_DATA_STORAGE + currArea_ + CONTEXT_DEAL_FILE_SEPARATOR + CONTEXT_DEAL_BASE;
    }

    TAG_LOGD(AAFwkTag::APPKIT, "GetBaseDir:%{public}s", baseDir.c_str());
    return baseDir;
}
}  // namespace AppExecFwk
}  // namespace OHOS
