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

#include "context_impl.h"

#include <cerrno>
#include <regex>

#include "ability_manager_client.h"
#include "app_mgr_client.h"
#include "application_context.h"
#include "bundle_mgr_helper.h"
#include "bundle_mgr_proxy.h"
#include "common_event_manager.h"
#include "configuration_convertor.h"
#include "constants.h"
#include "directory_ex.h"
#include "file_ex.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ipc_object_proxy.h"
#include "ipc_singleton.h"
#include "js_runtime_utils.h"
#ifdef SUPPORT_SCREEN
#include "locale_config.h"
#endif
#include "os_account_manager_wrapper.h"
#include "overlay_event_subscriber.h"
#include "overlay_module_info.h"
#include "parameters.h"
#include "running_process_info.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AbilityBase::Constants;

const std::string PATTERN_VERSION = std::string(FILE_SEPARATOR) + "v\\d+" + FILE_SEPARATOR;

const size_t Context::CONTEXT_TYPE_ID(std::hash<const char*> {} ("Context"));
const int64_t ContextImpl::CONTEXT_CREATE_BY_SYSTEM_APP(0x00000001);
const mode_t MODE = 0770;
const mode_t GROUP_MODE = 02770;
const std::string ContextImpl::CONTEXT_DATA_APP("/data/app/");
const std::string ContextImpl::CONTEXT_BUNDLE("/bundle/");
const std::string ContextImpl::CONTEXT_DISTRIBUTEDFILES_BASE_BEFORE("/mnt/hmdfs/");
const std::string ContextImpl::CONTEXT_DISTRIBUTEDFILES_BASE_MIDDLE("/device_view/local/data/");
const std::string ContextImpl::CONTEXT_DISTRIBUTEDFILES("distributedfiles");
const std::string ContextImpl::CONTEXT_CLOUDFILE("cloud");
const std::string ContextImpl::CONTEXT_FILE_SEPARATOR("/");
const std::string ContextImpl::CONTEXT_DATA("/data/");
const std::string ContextImpl::CONTEXT_DATA_STORAGE("/data/storage/");
const std::string ContextImpl::CONTEXT_BASE("base");
const std::string ContextImpl::CONTEXT_CACHE("cache");
const std::string ContextImpl::CONTEXT_PREFERENCES("preferences");
const std::string ContextImpl::CONTEXT_GROUP("group");
const std::string ContextImpl::CONTEXT_DATABASE("database");
const std::string ContextImpl::CONTEXT_TEMP("/temp");
const std::string ContextImpl::CONTEXT_FILES("/files");
const std::string ContextImpl::CONTEXT_HAPS("/haps");
const std::string ContextImpl::CONTEXT_ELS[] = {"el1", "el2", "el3", "el4", "el5"};
const std::string ContextImpl::CONTEXT_RESOURCE_END = "/resources/resfile";
Global::Resource::DeviceType ContextImpl::deviceType_ = Global::Resource::DeviceType::DEVICE_NOT_SET;
const std::string OVERLAY_STATE_CHANGED = "usual.event.OVERLAY_STATE_CHANGED";
const int32_t TYPE_RESERVE = 1;
const int32_t TYPE_OTHERS = 2;
const int32_t API11 = 11;
const int32_t API_VERSION_MOD = 100;
const int32_t ERR_ABILITY_RUNTIME_EXTERNAL_NOT_SYSTEM_HSP = 16400001;
const int AREA2 = 2;
const int AREA3 = 3;
const int AREA4 = 4;

std::string ContextImpl::GetBundleName() const
{
    if (parentContext_ != nullptr) {
        return parentContext_->GetBundleName();
    }
    return (applicationInfo_ != nullptr) ? applicationInfo_->bundleName : "";
}

std::string ContextImpl::GetBundleNameWithContext(std::shared_ptr<Context> inputContext) const
{
    if (inputContext) {
        return inputContext->GetBundleName();
    }
    return GetBundleName();
}

std::string ContextImpl::GetBundleCodeDir()
{
    auto appInfo = GetApplicationInfo();
    if (appInfo == nullptr) {
        return "";
    }

    std::string dir;
    if (IsCreateBySystemApp()) {
        dir = std::regex_replace(appInfo->codePath, std::regex(ABS_CODE_PATH), LOCAL_BUNDLES);
    } else {
        dir = LOCAL_CODE_PATH;
    }
    CreateDirIfNotExist(dir, MODE);
    TAG_LOGD(AAFwkTag::APPKIT, "dir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetCacheDir()
{
    std::string dir = GetBaseDir() + CONTEXT_FILE_SEPARATOR + CONTEXT_CACHE;
    CreateDirIfNotExist(dir, MODE);
    TAG_LOGD(AAFwkTag::APPKIT, "dir:%{public}s", dir.c_str());
    return dir;
}

bool ContextImpl::IsUpdatingConfigurations()
{
    return false;
}

bool ContextImpl::PrintDrawnCompleted()
{
    return false;
}

void ContextImpl::CreateDirIfNotExistWithCheck(const std::string &dirPath, const mode_t &mode, bool checkExist)
{
    if (checkExist) {
        CreateDirIfNotExist(dirPath, mode);
        return;
    }
    // Check if the dirPath exists on the first call
    std::lock_guard<std::mutex> lock(checkedDirSetLock_);
    if (checkedDirSet_.find(dirPath) != checkedDirSet_.end()) {
        return;
    }
    checkedDirSet_.emplace(dirPath);
    CreateDirIfNotExist(dirPath, mode);
}

int32_t ContextImpl::GetDatabaseDirWithCheck(bool checkExist, std::string &databaseDir)
{
    if (IsCreateBySystemApp()) {
        databaseDir = CONTEXT_DATA_APP + currArea_ + CONTEXT_FILE_SEPARATOR + std::to_string(GetCurrentAccountId())
                      + CONTEXT_FILE_SEPARATOR + CONTEXT_DATABASE + CONTEXT_FILE_SEPARATOR + GetBundleName();
    } else {
        databaseDir = CONTEXT_DATA_STORAGE + currArea_ + CONTEXT_FILE_SEPARATOR + CONTEXT_DATABASE;
    }
    if (parentContext_ != nullptr) {
        databaseDir = databaseDir + CONTEXT_FILE_SEPARATOR +
                      ((GetHapModuleInfo() == nullptr) ? "" : GetHapModuleInfo()->moduleName);
    }
    CreateDirIfNotExistWithCheck(databaseDir, 0, checkExist);
    return ERR_OK;
}

int32_t ContextImpl::GetGroupDatabaseDirWithCheck(const std::string &groupId, bool checkExist, std::string &databaseDir)
{
    int32_t ret = GetGroupDirWithCheck(groupId, checkExist, databaseDir);
    if (ret != ERR_OK) {
        return ret;
    }
    databaseDir = databaseDir + CONTEXT_FILE_SEPARATOR + CONTEXT_DATABASE;
    CreateDirIfNotExistWithCheck(databaseDir, GROUP_MODE, checkExist);
    return ERR_OK;
}

int32_t ContextImpl::GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir)
{
    int32_t ret;
    if (groupId.empty()) {
        ret = GetDatabaseDirWithCheck(checkExist, databaseDir);
    } else {
        ret = GetGroupDatabaseDirWithCheck(groupId, checkExist, databaseDir);
    }
    TAG_LOGD(AAFwkTag::APPKIT, "databaseDir: %{public}s", databaseDir.c_str());
    return ret;
}

std::string ContextImpl::GetDatabaseDir()
{
    std::string dir;
    GetDatabaseDirWithCheck(true, dir);
    TAG_LOGD(AAFwkTag::APPKIT, "databaseDir: %{public}s", dir.c_str());
    return dir;
}

int32_t ContextImpl::GetPreferencesDirWithCheck(bool checkExist, std::string &preferencesDir)
{
    preferencesDir = GetBaseDir() + CONTEXT_FILE_SEPARATOR + CONTEXT_PREFERENCES;
    CreateDirIfNotExistWithCheck(preferencesDir, MODE, checkExist);
    return ERR_OK;
}

int32_t ContextImpl::GetGroupPreferencesDirWithCheck(const std::string &groupId, bool checkExist,
    std::string &preferencesDir)
{
    int32_t ret = GetGroupDirWithCheck(groupId, checkExist, preferencesDir);
    if (ret != ERR_OK) {
        return ret;
    }
    preferencesDir = preferencesDir + CONTEXT_FILE_SEPARATOR + CONTEXT_PREFERENCES;
    CreateDirIfNotExistWithCheck(preferencesDir, GROUP_MODE, checkExist);
    return ERR_OK;
}

int32_t ContextImpl::GetSystemPreferencesDir(const std::string &groupId, bool checkExist, std::string &preferencesDir)
{
    int32_t ret;
    if (groupId.empty()) {
        ret = GetPreferencesDirWithCheck(checkExist, preferencesDir);
    } else {
        ret = GetGroupPreferencesDirWithCheck(groupId, checkExist, preferencesDir);
    }
    TAG_LOGD(AAFwkTag::APPKIT, "preferencesDir: %{public}s", preferencesDir.c_str());
    return ret;
}

std::string ContextImpl::GetPreferencesDir()
{
    std::string dir;
    GetPreferencesDirWithCheck(true, dir);
    TAG_LOGD(AAFwkTag::APPKIT, "preferencesDir: %{public}s", dir.c_str());
    return dir;
}

int32_t ContextImpl::GetGroupDirWithCheck(const std::string &groupId, bool checkExist, std::string &groupDir)
{
    if (currArea_ == CONTEXT_ELS[0]) {
        TAG_LOGE(AAFwkTag::APPKIT, "GroupDir currently can't supports the el1 level");
        return ERR_INVALID_VALUE;
    }
    int errCode = GetBundleManager();
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed, errCode: %{public}d", errCode);
        return errCode;
    }
    std::string groupDirGet;
    bool ret = bundleMgr_->GetGroupDir(groupId, groupDirGet);
    if (!ret || groupDirGet.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetGroupDir failed or groupDirGet is empty");
        return ERR_INVALID_VALUE;
    }
    std::string uuid = groupDirGet.substr(groupDirGet.rfind('/'));
    groupDir = CONTEXT_DATA_STORAGE + currArea_ + CONTEXT_FILE_SEPARATOR + CONTEXT_GROUP + uuid;
    CreateDirIfNotExistWithCheck(groupDir, MODE, true);
    return ERR_OK;
}

std::string ContextImpl::GetGroupDir(std::string groupId)
{
    std::string dir;
    GetGroupDirWithCheck(groupId, true, dir);
    TAG_LOGD(AAFwkTag::APPKIT, "GroupDir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetTempDir()
{
    std::string dir = GetBaseDir() + CONTEXT_TEMP;
    CreateDirIfNotExist(dir, MODE);
    TAG_LOGD(AAFwkTag::APPKIT, "dir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetResourceDir()
{
    std::shared_ptr<AppExecFwk::HapModuleInfo> hapModuleInfoPtr = GetHapModuleInfo();
    if (hapModuleInfoPtr == nullptr || hapModuleInfoPtr->moduleName.empty()) {
        return "";
    }
    std::string dir = std::string(LOCAL_CODE_PATH) + CONTEXT_FILE_SEPARATOR +
                      hapModuleInfoPtr->moduleName + CONTEXT_RESOURCE_END;
    if (OHOS::FileExists(dir)) {
        return dir;
    }
    return "";
}

void ContextImpl::GetAllTempDir(std::vector<std::string> &tempPaths)
{
    // Application temp dir
    auto appTemp = GetTempDir();
    if (OHOS::FileExists(appTemp)) {
        tempPaths.push_back(appTemp);
    }
    // Module dir
    if (applicationInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "The application info is empty");
        return;
    }

    std::string baseDir;
    if (IsCreateBySystemApp()) {
        baseDir = CONTEXT_DATA_APP + currArea_ + CONTEXT_FILE_SEPARATOR + std::to_string(GetCurrentAccountId()) +
            CONTEXT_FILE_SEPARATOR + CONTEXT_BASE + CONTEXT_FILE_SEPARATOR + GetBundleName();
    } else {
        baseDir = CONTEXT_DATA_STORAGE + currArea_ + CONTEXT_FILE_SEPARATOR + CONTEXT_BASE;
    }
    for (const auto &moudleItem: applicationInfo_->moduleInfos) {
        auto moudleTemp = baseDir + CONTEXT_HAPS + CONTEXT_FILE_SEPARATOR + moudleItem.moduleName + CONTEXT_TEMP;
        if (!OHOS::FileExists(moudleTemp)) {
            TAG_LOGW(AAFwkTag::APPKIT, "moudle[%{public}s] temp path not exist,path: %{public}s",
                moudleItem.moduleName.c_str(), moudleTemp.c_str());
            continue;
        }
        tempPaths.push_back(moudleTemp);
    }
}

std::string ContextImpl::GetFilesDir()
{
    std::string dir = GetBaseDir() + CONTEXT_FILES;
    CreateDirIfNotExist(dir, MODE);
    TAG_LOGD(AAFwkTag::APPKIT, "dir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetDistributedFilesDir()
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    std::string dir;
    if (IsCreateBySystemApp()) {
        dir = CONTEXT_DISTRIBUTEDFILES_BASE_BEFORE + std::to_string(GetCurrentAccountId()) +
            CONTEXT_DISTRIBUTEDFILES_BASE_MIDDLE + GetBundleName();
    } else {
        if (currArea_ == CONTEXT_ELS[1] || currArea_ == CONTEXT_ELS[AREA2] || currArea_ == CONTEXT_ELS[AREA3] ||
	    currArea_ == CONTEXT_ELS[AREA4]) {
            // when areamode swith to el3/el4/el5, the distributedfiles dir should be always el2's
            // distributedfilesdir dir
            dir = CONTEXT_DATA_STORAGE + CONTEXT_ELS[1] + CONTEXT_FILE_SEPARATOR + CONTEXT_DISTRIBUTEDFILES;
        } else {
            dir = CONTEXT_DATA_STORAGE + currArea_ + CONTEXT_FILE_SEPARATOR + CONTEXT_DISTRIBUTEDFILES;
        }
    }
    CreateDirIfNotExist(dir, 0);
    TAG_LOGD(AAFwkTag::APPKIT, "dir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetCloudFileDir()
{
    std::string dir = CONTEXT_DATA_STORAGE + CONTEXT_ELS[1] + CONTEXT_FILE_SEPARATOR + CONTEXT_CLOUDFILE;
    CreateDirIfNotExist(dir, MODE);
    return dir;
}

void ContextImpl::SwitchArea(int mode)
{
    TAG_LOGD(AAFwkTag::APPKIT, "mode:%{public}d", mode);
    if (mode < 0 || mode >= (int)(sizeof(CONTEXT_ELS) / sizeof(CONTEXT_ELS[0]))) {
        TAG_LOGE(AAFwkTag::APPKIT, "ContextImpl::SwitchArea, mode is invalid.");
        return;
    }
    currArea_ = CONTEXT_ELS[mode];
    TAG_LOGD(AAFwkTag::APPKIT, "currArea:%{public}s", currArea_.c_str());
}

void ContextImpl::SetMcc(std::string mcc)
{
    TAG_LOGD(AAFwkTag::APPKIT, "mcc:%{public}s", mcc.c_str());
    if (config_) {
        config_->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_MCC, mcc);
    }
}

void ContextImpl::SetMnc(std::string mnc)
{
    TAG_LOGD(AAFwkTag::APPKIT, "mnc:%{public}s", mnc.c_str());
    if (config_) {
        config_->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_MNC, mnc);
    }
}

std::shared_ptr<Context> ContextImpl::CreateModuleContext(const std::string &moduleName,
    std::shared_ptr<Context> inputContext)
{
    return CreateModuleContext(GetBundleNameWithContext(inputContext), moduleName, inputContext);
}

std::shared_ptr<Context> ContextImpl::CreateModuleContext(const std::string &bundleName, const std::string &moduleName,
    std::shared_ptr<Context> inputContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPKIT, "begin");
    if (bundleName.empty() || moduleName.empty()) {
        return nullptr;
    }

    TAG_LOGD(AAFwkTag::APPKIT, "length: %{public}zu, bundleName: %{public}s",
        (size_t)bundleName.length(), bundleName.c_str());

    int accountId = GetCurrentAccountId();
    if (accountId == 0) {
        accountId = GetCurrentActiveAccountId();
    }

    AppExecFwk::BundleInfo bundleInfo;
    GetBundleInfo(bundleName, bundleInfo, accountId, inputContext);
    if (bundleInfo.name.empty() || bundleInfo.applicationInfo.name.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetBundleInfo is error");
        ErrCode ret = bundleMgr_->GetDependentBundleInfo(bundleName, bundleInfo,
            AppExecFwk::GetDependentBundleInfoFlag::GET_ALL_DEPENDENT_BUNDLE_INFO);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "GetDependentBundleInfo failed:%{public}d", ret);
            return nullptr;
        }
    }

    std::shared_ptr<ContextImpl> appContext = std::make_shared<ContextImpl>();
    if (bundleInfo.applicationInfo.codePath != std::to_string(TYPE_RESERVE) &&
        bundleInfo.applicationInfo.codePath != std::to_string(TYPE_OTHERS)) {
        TAG_LOGD(AAFwkTag::APPKIT, "modulename: %{public}s, bundleName: %{public}s",
            moduleName.c_str(), bundleName.c_str());
        auto info = std::find_if(bundleInfo.hapModuleInfos.begin(), bundleInfo.hapModuleInfos.end(),
            [&moduleName](const AppExecFwk::HapModuleInfo &hapModuleInfo) {
                return hapModuleInfo.moduleName == moduleName;
            });
        if (info == bundleInfo.hapModuleInfos.end()) {
            TAG_LOGE(AAFwkTag::APPKIT, "moduleName is error");
            return nullptr;
        }
        appContext->InitHapModuleInfo(*info);
    }

    appContext->SetConfiguration(config_);
    bool self = false;
    if (inputContext) {
        self = (bundleName == inputContext->GetBundleName());
    } else {
        self = bundleName == GetBundleName();
    }
    InitResourceManager(bundleInfo, appContext, self, moduleName, inputContext);
    appContext->SetApplicationInfo(std::make_shared<AppExecFwk::ApplicationInfo>(bundleInfo.applicationInfo));
    return appContext;
}

std::shared_ptr<Context> ContextImpl::CreateModuleContext(const std::string &moduleName)
{
    return CreateModuleContext(GetBundleName(), moduleName, nullptr);
}

std::shared_ptr<Context> ContextImpl::CreateModuleContext(const std::string &bundleName, const std::string &moduleName)
{
    return CreateModuleContext(bundleName, moduleName, nullptr);
}

std::shared_ptr<Global::Resource::ResourceManager> ContextImpl::CreateModuleResourceManager(
    const std::string &bundleName, const std::string &moduleName)
{
    TAG_LOGD(AAFwkTag::APPKIT, "begin, bundleName: %{public}s, moduleName: %{public}s",
        bundleName.c_str(), moduleName.c_str());
    if (bundleName.empty() || moduleName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "bundleName is %{public}s, moduleName is %{public}s",
            bundleName.c_str(), moduleName.c_str());
        return nullptr;
    }

    AppExecFwk::BundleInfo bundleInfo;
    bool currentBundle = false;
    if (GetBundleInfo(bundleName, bundleInfo, currentBundle) != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to GetBundleInfo, bundleName: %{public}s", bundleName.c_str());
        return nullptr;
    }

    if (bundleInfo.applicationInfo.codePath == std::to_string(TYPE_RESERVE) ||
        bundleInfo.applicationInfo.codePath == std::to_string(TYPE_OTHERS)) {
        std::shared_ptr<Global::Resource::ResourceManager> resourceManager = InitOthersResourceManagerInner(
            bundleInfo, currentBundle, moduleName);
        if (resourceManager == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "InitOthersResourceManagerInner create resourceManager failed");
        }
        return resourceManager;
    }

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = InitResourceManagerInner(
        bundleInfo, currentBundle, moduleName);
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "InitResourceManagerInner create resourceManager failed");
        return nullptr;
    }
    UpdateResConfig(resourceManager);
    return resourceManager;
}

int32_t ContextImpl::CreateSystemHspModuleResourceManager(const std::string &bundleName,
    const std::string &moduleName, std::shared_ptr<Global::Resource::ResourceManager> &resourceManager)
{
    TAG_LOGD(AAFwkTag::APPKIT, "begin, bundleName: %{public}s, moduleName: %{public}s",
        bundleName.c_str(), moduleName.c_str());
    if (bundleName.empty() || moduleName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "bundleName is %{public}s, moduleName is %{public}s",
            bundleName.c_str(), moduleName.c_str());
        return ERR_INVALID_VALUE;
    }

    int accountId = GetCurrentAccountId();
    if (accountId == 0) {
        accountId = GetCurrentActiveAccountId();
    }
    AppExecFwk::BundleInfo bundleInfo;
    GetBundleInfo(bundleName, bundleInfo, accountId);
    if (bundleInfo.name.empty() || bundleInfo.applicationInfo.name.empty()) {
        TAG_LOGW(AAFwkTag::APPKIT, "GetBundleInfo is error");
        ErrCode ret = bundleMgr_->GetDependentBundleInfo(bundleName, bundleInfo,
            AppExecFwk::GetDependentBundleInfoFlag::GET_ALL_DEPENDENT_BUNDLE_INFO);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "GetDependentBundleInfo failed:%{public}d", ret);
            return ERR_INVALID_VALUE;
        }
    }

    if (bundleInfo.applicationInfo.bundleType != AppExecFwk::BundleType::APP_SERVICE_FWK) {
        TAG_LOGE(AAFwkTag::APPKIT, "input bundleName:%{public}s is not system hsp", bundleName.c_str());
        return ERR_ABILITY_RUNTIME_EXTERNAL_NOT_SYSTEM_HSP;
    }

    std::string selfBundleName = GetBundleName();
    if (bundleInfo.applicationInfo.codePath == std::to_string(TYPE_RESERVE) ||
        bundleInfo.applicationInfo.codePath == std::to_string(TYPE_OTHERS)) {
        resourceManager = InitOthersResourceManagerInner(bundleInfo, selfBundleName == bundleName, moduleName);
        if (resourceManager == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "InitOthersResourceManagerInner create resourceManager failed");
        }
        return ERR_INVALID_VALUE;
    }

    resourceManager = InitResourceManagerInner(bundleInfo, selfBundleName == bundleName, moduleName);
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "InitResourceManagerInner create resourceManager failed");
        return ERR_INVALID_VALUE;
    }
    UpdateResConfig(resourceManager);
    return ERR_OK;
}

int32_t ContextImpl::GetBundleInfo(const std::string &bundleName, AppExecFwk::BundleInfo &bundleInfo,
    bool &currentBundle)
{
    std::string currentBundleName;
    auto appContext = ApplicationContext::GetInstance();
    if (appContext != nullptr) {
        currentBundleName = appContext->GetBundleName();
    }
    currentBundle = bundleName == currentBundleName;

    int errCode = GetBundleManager();
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "errCode: %{public}d", errCode);
        return errCode;
    }

    if (currentBundle) {
        bundleMgr_->GetBundleInfoForSelf((
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ABILITY) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_DISABLE) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_SIGNATURE_INFO) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_METADATA)), bundleInfo);
    } else {
        int accountId = GetCurrentAccountId();
        if (accountId == 0) {
            accountId = GetCurrentActiveAccountId();
        }
        bundleMgr_->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, accountId);
    }

    if (bundleInfo.name.empty() || bundleInfo.applicationInfo.name.empty()) {
        TAG_LOGW(AAFwkTag::APPKIT, "bundleInfo is empty");
        ErrCode ret = bundleMgr_->GetUninstalledBundleInfo(bundleName, bundleInfo);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "GetUninstalledBundleInfo failed:%{public}d", ret);
            return ret;
        }
    }
    return ERR_OK;
}

void ContextImpl::GetBundleInfo(const std::string &bundleName, AppExecFwk::BundleInfo &bundleInfo,
    const int &accountId, std::shared_ptr<Context> inputContext)
{
    TAG_LOGD(AAFwkTag::APPKIT, "begin");
    if (bundleMgr_ == nullptr) {
        int errCode = GetBundleManager();
        if (errCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed, errCode: %{public}d", errCode);
            return;
        }
    }

    if (bundleName == GetBundleNameWithContext(inputContext)) {
        bundleMgr_->GetBundleInfoForSelf(
            (static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ABILITY) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_DISABLE) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_SIGNATURE_INFO) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_METADATA)), bundleInfo);
    } else {
        bundleMgr_->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, accountId);
    }
}

int ContextImpl::GetArea()
{
    TAG_LOGD(AAFwkTag::APPKIT, "begin");
    int mode = -1;
    for (int i = 0; i < (int)(sizeof(CONTEXT_ELS) / sizeof(CONTEXT_ELS[0])); i++) {
        if (currArea_ == CONTEXT_ELS[i]) {
            mode = i;
            break;
        }
    }
    if (mode == -1) {
        TAG_LOGE(AAFwkTag::APPKIT, "not find mode");
        return EL_DEFAULT;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "end");
    return mode;
}

std::string ContextImpl::GetBaseDir() const
{
    std::string baseDir;
    if (IsCreateBySystemApp()) {
        baseDir = CONTEXT_DATA_APP + currArea_ + CONTEXT_FILE_SEPARATOR + std::to_string(GetCurrentAccountId()) +
            CONTEXT_FILE_SEPARATOR + CONTEXT_BASE + CONTEXT_FILE_SEPARATOR + GetBundleName();
    } else {
        baseDir = CONTEXT_DATA_STORAGE + currArea_ + CONTEXT_FILE_SEPARATOR + CONTEXT_BASE;
    }
    if (parentContext_ != nullptr) {
        baseDir = baseDir + CONTEXT_HAPS + CONTEXT_FILE_SEPARATOR +
            ((GetHapModuleInfo() == nullptr) ? "" : GetHapModuleInfo()->moduleName);
    }

    TAG_LOGD(AAFwkTag::APPKIT, "Dir:%{public}s", baseDir.c_str());
    return baseDir;
}

int ContextImpl::GetCurrentAccountId() const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int userId = 0;
    auto instance = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance();
    if (instance == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get OsAccountManager instance");
        return userId;
    }
    instance->GetOsAccountLocalIdFromProcess(userId);
    return userId;
}

int ContextImpl::GetCurrentActiveAccountId() const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::vector<int> accountIds;
    auto instance = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance();
    if (instance == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get OsAccountManager instance");
        return 0;
    }
    ErrCode ret = instance->QueryActiveOsAccountIds(accountIds);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "ContextImpl::GetCurrentActiveAccountId error");
        return 0;
    }

    if (accountIds.size() == 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "no accounts");
        return 0;
    }

    if (accountIds.size() > 1) {
        TAG_LOGE(AAFwkTag::APPKIT, "no current now");
        return 0;
    }

    return accountIds[0];
}

int32_t ContextImpl::CreateBundleContext(std::shared_ptr<Context> &context, const std::string &bundleName,
    std::shared_ptr<Context> inputContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "bundleName is empty");
        return ERR_INVALID_VALUE;
    }

    int errCode = GetBundleManager();
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed, errCode: %{public}d", errCode);
        return ERR_INVALID_VALUE;
    }

    AppExecFwk::BundleInfo bundleInfo;
    int accountId = GetCurrentAccountId();
    if (accountId == 0) {
        accountId = GetCurrentActiveAccountId();
    }

    TAG_LOGD(AAFwkTag::APPKIT, "length: %{public}zu, bundleName: %{public}s",
        (size_t)bundleName.length(), bundleName.c_str());
    errCode = bundleMgr_->GetBundleInfoV9(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT,
        bundleInfo, accountId);
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed, errCode: %{public}d", errCode);
        return errCode;
    }

    std::shared_ptr<ContextImpl> appContext = std::make_shared<ContextImpl>();
    appContext->SetFlags(CONTEXT_CREATE_BY_SYSTEM_APP);
    appContext->SetConfiguration(config_);

    // init resourceManager.
    InitResourceManager(bundleInfo, appContext, false, "", inputContext);

    appContext->SetApplicationInfo(std::make_shared<AppExecFwk::ApplicationInfo>(bundleInfo.applicationInfo));
    context = appContext;
    return ERR_OK;
}

std::shared_ptr<Context> ContextImpl::CreateBundleContext(const std::string &bundleName)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPKIT, "begin");
    if (parentContext_ != nullptr) {
        return parentContext_->CreateBundleContext(bundleName);
    }

    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "bundleName is empty");
        return nullptr;
    }

    int errCode = GetBundleManager();
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed, errCode: %{public}d", errCode);
        return nullptr;
    }

    AppExecFwk::BundleInfo bundleInfo;
    int accountId = GetCurrentAccountId();
    if (accountId == 0) {
        accountId = GetCurrentActiveAccountId();
    }
    TAG_LOGD(AAFwkTag::APPKIT, "length: %{public}zu, bundleName: %{public}s",
        (size_t)bundleName.length(), bundleName.c_str());
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "bundleMgr_->GetBundleInfo");
    bundleMgr_->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, accountId);

    if (bundleInfo.name.empty() || bundleInfo.applicationInfo.name.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "bundleInfo is empty");
        return nullptr;
    }

    std::shared_ptr<ContextImpl> appContext = std::make_shared<ContextImpl>();
    appContext->SetFlags(CONTEXT_CREATE_BY_SYSTEM_APP);
    appContext->SetConfiguration(config_);

    // init resourceManager.
    InitResourceManager(bundleInfo, appContext);
    appContext->SetApplicationInfo(std::make_shared<AppExecFwk::ApplicationInfo>(bundleInfo.applicationInfo));
    return appContext;
}

void ContextImpl::InitResourceManager(const AppExecFwk::BundleInfo &bundleInfo,
    const std::shared_ptr<ContextImpl> &appContext, bool currentBundle, const std::string& moduleName,
    std::shared_ptr<Context> inputContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPKIT, "begin, bundleName:%{public}s, moduleName:%{public}s",
        bundleInfo.name.c_str(), moduleName.c_str());

    if (appContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "InitResourceManager appContext is nullptr");
        return;
    }

    if (bundleInfo.applicationInfo.codePath == std::to_string(TYPE_RESERVE) ||
        bundleInfo.applicationInfo.codePath == std::to_string(TYPE_OTHERS)) {
        std::shared_ptr<Global::Resource::ResourceManager> resourceManager = InitOthersResourceManagerInner(
            bundleInfo, currentBundle, moduleName);
        if (resourceManager == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "create resourceManager failed");
            return;
        }
        appContext->SetResourceManager(resourceManager);
        return;
    }

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = InitResourceManagerInner(
        bundleInfo, currentBundle, moduleName, inputContext);
    if (resourceManager == nullptr) {
        return;
    }
    UpdateResConfig(resourceManager, inputContext);
    appContext->SetResourceManager(resourceManager);
}

std::shared_ptr<Global::Resource::ResourceManager> ContextImpl::InitOthersResourceManagerInner(
    const AppExecFwk::BundleInfo &bundleInfo, bool currentBundle, const std::string& moduleName)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    std::string hapPath;
    std::vector<std::string> overlayPaths;
    int32_t appType;
    if (bundleInfo.applicationInfo.codePath == std::to_string(TYPE_RESERVE)) {
        appType = TYPE_RESERVE;
    } else if (bundleInfo.applicationInfo.codePath == std::to_string(TYPE_OTHERS)) {
        appType = TYPE_OTHERS;
    } else {
        appType = 0;
    }
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager(
        bundleInfo.name, moduleName, hapPath, overlayPaths, *resConfig, appType));
    return resourceManager;
}

std::shared_ptr<Global::Resource::ResourceManager> ContextImpl::InitResourceManagerInner(
    const AppExecFwk::BundleInfo &bundleInfo, bool currentBundle, const std::string& moduleName,
    std::shared_ptr<Context> inputContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = InitOthersResourceManagerInner(
        bundleInfo, currentBundle, moduleName);
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Create resourceManager failed");
        return resourceManager;
    }
    if (!moduleName.empty() || !bundleInfo.applicationInfo.multiProjects) {
        TAG_LOGD(AAFwkTag::APPKIT, "hapModuleInfos count: %{public}zu", bundleInfo.hapModuleInfos.size());
        std::regex inner_pattern(std::string(ABS_CODE_PATH) + std::string(FILE_SEPARATOR)
            + GetBundleNameWithContext(inputContext));
        std::regex outer_pattern(ABS_CODE_PATH);
        std::regex hsp_pattern(std::string(ABS_CODE_PATH) + FILE_SEPARATOR + bundleInfo.name + PATTERN_VERSION);
        std::string hsp_sandbox = std::string(LOCAL_CODE_PATH) + FILE_SEPARATOR + bundleInfo.name + FILE_SEPARATOR;
        {
            HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "for (auto hapModuleInfo : bundleInfo.hapModuleInfos)");
            for (auto hapModuleInfo : bundleInfo.hapModuleInfos) {
                TAG_LOGD(AAFwkTag::APPKIT, "hapModuleInfo abilityInfo size: %{public}zu",
                    hapModuleInfo.abilityInfos.size());
                if (!moduleName.empty() && hapModuleInfo.moduleName != moduleName) {
                    continue;
                }
                std::string loadPath =
                    hapModuleInfo.hapPath.empty() ? hapModuleInfo.resourcePath : hapModuleInfo.hapPath;
                if (loadPath.empty()) {
                    TAG_LOGD(AAFwkTag::APPKIT, "loadPath is empty");
                    continue;
                }
                if (currentBundle) {
                    loadPath = std::regex_replace(loadPath, inner_pattern, LOCAL_CODE_PATH);
                } else if (bundleInfo.applicationInfo.bundleType == AppExecFwk::BundleType::SHARED) {
                    loadPath = std::regex_replace(loadPath, hsp_pattern, hsp_sandbox);
                } else if (bundleInfo.applicationInfo.bundleType == AppExecFwk::BundleType::APP_SERVICE_FWK) {
                    TAG_LOGD(AAFwkTag::APPKIT, "System hsp path, not need translate");
                } else {
                    loadPath = std::regex_replace(loadPath, outer_pattern, LOCAL_BUNDLES);
                }

                TAG_LOGD(AAFwkTag::APPKIT, "loadPath: %{private}s", loadPath.c_str());
                GetOverlayPath(resourceManager, bundleInfo.name, hapModuleInfo.moduleName, loadPath, currentBundle,
                    inputContext);
                AddPatchResource(resourceManager, loadPath, hapModuleInfo.hqfInfo.hqfFilePath,
                    bundleInfo.applicationInfo.debug, inputContext);
            }
        }
    }
    return resourceManager;
}

void ContextImpl::AddPatchResource(std::shared_ptr<Global::Resource::ResourceManager> &resourceManager,
    const std::string &loadPath, const std::string &hqfPath, bool isDebug, std::shared_ptr<Context> inputContext)
{
    std::regex pattern(std::string(ABS_CODE_PATH) + std::string(FILE_SEPARATOR)
        + GetBundleNameWithContext(inputContext));
    if (!hqfPath.empty() && isDebug) {
        std::string realHqfPath = std::regex_replace(hqfPath, pattern, LOCAL_CODE_PATH);
        TAG_LOGI(AAFwkTag::APPKIT, "AddPatchResource hapPath:%{public}s, patchPath:%{public}s",
            loadPath.c_str(), realHqfPath.c_str());
        if (!resourceManager->AddPatchResource(loadPath.c_str(), realHqfPath.c_str())) {
            TAG_LOGE(AAFwkTag::APPKIT, "AddPatchResource failed");
        }
    }
}

void ContextImpl::GetOverlayPath(std::shared_ptr<Global::Resource::ResourceManager> &resourceManager,
    const std::string &bundleName, const std::string &moduleName, std::string &loadPath, bool currentBundle,
    std::shared_ptr<Context> inputContext)
{
    // getOverlayPath
    std::vector<AppExecFwk::OverlayModuleInfo> overlayModuleInfos;
    auto res = GetOverlayModuleInfos(bundleName, moduleName, overlayModuleInfos);
    if (res != ERR_OK) {
        TAG_LOGD(AAFwkTag::APPKIT, "Get overlay paths from bms failed.");
    }
    if (overlayModuleInfos.size() == 0) {
        if (!resourceManager->AddResource(loadPath.c_str())) {
            TAG_LOGE(AAFwkTag::APPKIT, "AddResource fail, moduleResPath: %{private}s", loadPath.c_str());
        }
    } else {
        std::vector<std::string> overlayPaths;
        for (auto it : overlayModuleInfos) {
            if (std::regex_search(it.hapPath, std::regex(GetBundleNameWithContext(inputContext)))) {
                it.hapPath = std::regex_replace(it.hapPath, std::regex(std::string(ABS_CODE_PATH) +
        std::string(FILE_SEPARATOR) + GetBundleNameWithContext(inputContext)), LOCAL_CODE_PATH);
            } else {
                it.hapPath = std::regex_replace(it.hapPath, std::regex(ABS_CODE_PATH), LOCAL_BUNDLES);
            }
            if (it.state == AppExecFwk::OverlayState::OVERLAY_ENABLE) {
                TAG_LOGD(AAFwkTag::APPKIT, "hapPath: %{private}s", it.hapPath.c_str());
                overlayPaths.emplace_back(it.hapPath);
            }
        }
        TAG_LOGD(AAFwkTag::APPKIT, "OverlayPaths size:%{public}zu.", overlayPaths.size());
        if (!resourceManager->AddResource(loadPath, overlayPaths)) {
            TAG_LOGE(AAFwkTag::APPKIT, "AddResource failed");
        }

        if (currentBundle) {
            SubscribeToOverlayEvents(resourceManager, bundleName, moduleName, loadPath, overlayModuleInfos);
        }
    }
}

void ContextImpl::SubscribeToOverlayEvents(std::shared_ptr<Global::Resource::ResourceManager> &resourceManager,
    const std::string &name, const std::string &hapModuleName, std::string &loadPath,
    std::vector<AppExecFwk::OverlayModuleInfo> overlayModuleInfos)
{
    // add listen overlay change
    overlayModuleInfos_ = overlayModuleInfos;
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(OVERLAY_STATE_CHANGED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    auto callback = [this, resourceManager, bundleName = name, moduleName =
    hapModuleName, loadPath](const EventFwk::CommonEventData &data) {
        TAG_LOGI(AAFwkTag::APPKIT, "On overlay changed.");
        this->OnOverlayChanged(data, resourceManager, bundleName, moduleName, loadPath);
    };
    auto subscriber = std::make_shared<AppExecFwk::OverlayEventSubscriber>(subscribeInfo, callback);
    bool subResult = EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber);
    TAG_LOGI(AAFwkTag::APPKIT, "Overlay event subscriber register result is %{public}d", subResult);
}

void ContextImpl::UpdateResConfig(std::shared_ptr<Global::Resource::ResourceManager> &resourceManager,
    std::shared_ptr<Context> inputContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "create ResConfig failed");
        return;
    }

    if (inputContext) {
        std::shared_ptr<Global::Resource::ResourceManager> currentResMgr = inputContext->GetResourceManager();
        if (currentResMgr != nullptr) {
            currentResMgr->GetResConfig(*resConfig);
        }
        resourceManager->UpdateResConfig(*resConfig);
        return;
    }
    if (GetHapModuleInfo() != nullptr && GetApplicationInfo() != nullptr) {
        std::vector<AppExecFwk::Metadata> metadata = GetHapModuleInfo()->metadata;
        bool load = std::any_of(metadata.begin(), metadata.end(), [](const auto &metadataItem) {
            return metadataItem.name == "ContextResourceConfigLoadFromParentTemp" && metadataItem.value == "true";
        });
        if (load && GetApplicationInfo()->apiTargetVersion % API_VERSION_MOD >= API11) {
            std::shared_ptr<Global::Resource::ResourceManager> currentResMgr = GetResourceManager();
            if (currentResMgr != nullptr) {
                TAG_LOGD(AAFwkTag::APPKIT, "apiVersion: %{public}d, load parent config",
                    GetApplicationInfo()->apiTargetVersion);
                currentResMgr->GetResConfig(*resConfig);
            }
        }
    }
#ifdef SUPPORT_SCREEN
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(Global::I18n::LocaleConfig::GetSystemLanguage(), status);
    resConfig->SetLocaleInfo(locale);
    if (resConfig->GetLocaleInfo() != nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT,
            "ContextImpl::InitResourceManager language: %{public}s, script: %{public}s, region: %{public}s,",
            resConfig->GetLocaleInfo()->getLanguage(), resConfig->GetLocaleInfo()->getScript(),
            resConfig->GetLocaleInfo()->getCountry());
    } else {
        TAG_LOGE(AAFwkTag::APPKIT, "LocaleInfo is null");
    }
#endif
    resConfig->SetDeviceType(GetDeviceType());
    std::string mcc = config_->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_MCC);
    std::string mnc = config_->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_MNC);
    try {
        resConfig->SetMcc(static_cast<uint32_t>(std::stoi(mcc)));
        resConfig->SetMnc(static_cast<uint32_t>(std::stoi(mnc)));
    } catch (...) {
        TAG_LOGD(AAFwkTag::APPKIT, "Set mcc,mnc failed mcc:%{public}s mnc:%{public}s", mcc.c_str(), mnc.c_str());
    }
    resourceManager->UpdateResConfig(*resConfig);
}

ErrCode ContextImpl::GetBundleManager()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<std::mutex> lock(bundleManagerMutex_);
    if (bundleMgr_ != nullptr && !resetFlag_) {
        return ERR_OK;
    }

    bundleMgr_ = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "The bundleMgr_ is nullptr");
        return ERR_NULL_OBJECT;
    }

    TAG_LOGD(AAFwkTag::APPKIT, "Success");
    return ERR_OK;
}

void ContextImpl::SetApplicationInfo(const std::shared_ptr<AppExecFwk::ApplicationInfo> &info)
{
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "info is empty");
        return;
    }
    applicationInfo_ = info;
}

void ContextImpl::SetResourceManager(const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager)
{
    resourceManager_ = resourceManager;
}

std::shared_ptr<Global::Resource::ResourceManager> ContextImpl::GetResourceManager() const
{
    if (resourceManager_) {
        return resourceManager_;
    }

    return parentContext_ != nullptr ? parentContext_->GetResourceManager() : nullptr;
}

std::shared_ptr<AppExecFwk::ApplicationInfo> ContextImpl::GetApplicationInfo() const
{
    if (parentContext_ != nullptr) {
        return parentContext_->GetApplicationInfo();
    }

    return applicationInfo_;
}

void ContextImpl::SetParentContext(const std::shared_ptr<Context> &context)
{
    parentContext_ = context;
}

std::string ContextImpl::GetBundleCodePath() const
{
    if (parentContext_ != nullptr) {
        return parentContext_->GetBundleCodePath();
    }
    return (applicationInfo_ != nullptr) ? applicationInfo_->codePath : "";
}

void ContextImpl::InitHapModuleInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo)
{
    if (hapModuleInfo_ != nullptr || abilityInfo == nullptr) {
        return;
    }
    int errCode = GetBundleManager();
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed, errCode: %{public}d", errCode);
        return ;
    }

    hapModuleInfo_ = std::make_shared<AppExecFwk::HapModuleInfo>();
    if (!bundleMgr_->GetHapModuleInfo(*abilityInfo.get(), *hapModuleInfo_)) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetHapModuleInfo failed, will retval false value");
    }
}

void ContextImpl::InitHapModuleInfo(const AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    hapModuleInfo_ = std::make_shared<AppExecFwk::HapModuleInfo>(hapModuleInfo);
}

std::shared_ptr<AppExecFwk::HapModuleInfo> ContextImpl::GetHapModuleInfo() const
{
    if (hapModuleInfo_ == nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "hapModuleInfo is empty");
    }
    return hapModuleInfo_;
}

std::shared_ptr<AppExecFwk::HapModuleInfo> ContextImpl::GetHapModuleInfoWithContext(
    std::shared_ptr<Context> inputContext) const
{
    if (inputContext) {
        return inputContext->GetHapModuleInfo();
    }
    return GetHapModuleInfo();
}

void ContextImpl::SetFlags(int64_t flags)
{
    flags_ = static_cast<uint64_t>(flags_) | static_cast<uint64_t>(CONTEXT_CREATE_BY_SYSTEM_APP);
}

bool ContextImpl::IsCreateBySystemApp() const
{
    return (static_cast<uint64_t>(flags_) & static_cast<uint64_t>(CONTEXT_CREATE_BY_SYSTEM_APP)) == 1;
}

std::shared_ptr<ApplicationContext> Context::applicationContext_ = nullptr;
std::mutex Context::contextMutex_;

std::shared_ptr<ApplicationContext> Context::GetApplicationContext()
{
    std::lock_guard<std::mutex> lock(contextMutex_);
    return applicationContext_;
}

void ContextImpl::SetToken(const sptr<IRemoteObject> &token)
{
    if (token == nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "application is nullptr");
        return;
    }
    token_ = token;
    if (GetBundleName() == "com.ohos.callui") {
        PrintTokenInfo();
    }
}

sptr<IRemoteObject> ContextImpl::GetToken()
{
    return token_;
}

void ContextImpl::CreateDirIfNotExist(const std::string& dirPath, const mode_t& mode) const
{
    if (!OHOS::FileExists(dirPath)) {
        TAG_LOGD(AAFwkTag::APPKIT, "ForceCreateDirectory, dir: %{public}s", dirPath.c_str());
        bool createDir = OHOS::ForceCreateDirectory(dirPath);
        if (!createDir) {
            TAG_LOGE(AAFwkTag::APPKIT, "create dir %{public}s failed, errno is %{public}d", dirPath.c_str(), errno);
            return;
        }
        if (mode != 0) {
            chmod(dirPath.c_str(), mode);
        }
    }
}

void ContextImpl::SetConfiguration(const std::shared_ptr<AppExecFwk::Configuration> &config)
{
    config_ = config;
}

void ContextImpl::AppHasDarkRes(bool &darkRes)
{
    std::shared_ptr<Global::Resource::ResourceManager> currentResMgr = GetResourceManager();
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (currentResMgr == nullptr || resConfig == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "resMgr or resConfig is null");
        return;
    }
    currentResMgr->GetResConfig(*resConfig);
    darkRes = resConfig->GetAppDarkRes();
    TAG_LOGD(AAFwkTag::APPKIT, "darkRes %{public}d", darkRes);
}

void ContextImpl::KillProcessBySelf()
{
    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    appMgrClient->KillApplicationSelf();
}

int32_t ContextImpl::GetProcessRunningInformation(AppExecFwk::RunningProcessInfo &info)
{
    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    auto result = appMgrClient->GetProcessRunningInformation(info);
    TAG_LOGD(AAFwkTag::APPKIT, "result is %{public}d", result);
    return result;
}

int32_t ContextImpl::RestartApp(const AAFwk::Want& want)
{
    auto result = OHOS::AAFwk::AbilityManagerClient::GetInstance()->RestartApp(want);
    TAG_LOGD(AAFwkTag::APPKIT, "result is %{public}d", result);
    return result;
}

std::shared_ptr<AppExecFwk::Configuration> ContextImpl::GetConfiguration() const
{
    return config_;
}

Global::Resource::DeviceType ContextImpl::GetDeviceType() const
{
    if (deviceType_ != Global::Resource::DeviceType::DEVICE_NOT_SET) {
        return deviceType_;
    }

    auto config = GetConfiguration();
    if (config != nullptr) {
        auto deviceType = config->GetItem(AAFwk::GlobalConfigurationKey::DEVICE_TYPE);
        TAG_LOGD(AAFwkTag::APPKIT, "deviceType is %{public}s", deviceType.c_str());
        deviceType_ = AppExecFwk::ConvertDeviceType(deviceType);
    }

    if (deviceType_ == Global::Resource::DeviceType::DEVICE_NOT_SET) {
        deviceType_ = Global::Resource::DeviceType::DEVICE_PHONE;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "deviceType is %{public}d", deviceType_);
    return deviceType_;
}

ErrCode ContextImpl::GetOverlayMgrProxy()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int errCode = GetBundleManager();
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed, errCode: %{public}d", errCode);
        return errCode;
    }

    std::lock_guard<std::mutex> lock(overlayMgrProxyMutex_);
    if (overlayMgrProxy_ != nullptr) {
        return ERR_OK;
    }

    overlayMgrProxy_ = bundleMgr_->GetOverlayManagerProxy();
    if (overlayMgrProxy_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "GetOverlayManagerProxy failed");
        return ERR_NULL_OBJECT;
    }

    TAG_LOGD(AAFwkTag::APPKIT, "Success.");
    return ERR_OK;
}

int ContextImpl::GetOverlayModuleInfos(const std::string &bundleName, const std::string &moduleName,
    std::vector<AppExecFwk::OverlayModuleInfo> &overlayModuleInfos)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int errCode = GetOverlayMgrProxy();
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed, errCode: %{public}d", errCode);
        return errCode;
    }
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "overlayMgrProxy_->GetTargetOverlayModuleInfo");
        auto ret = overlayMgrProxy_->GetTargetOverlayModuleInfo(moduleName, overlayModuleInfos);
        if (ret != ERR_OK) {
            TAG_LOGD(AAFwkTag::APPKIT, "GetOverlayModuleInfo form bms failed");
            return ret;
        }
    }
    std::sort(overlayModuleInfos.begin(), overlayModuleInfos.end(),
        [](const AppExecFwk::OverlayModuleInfo& lhs, const AppExecFwk::OverlayModuleInfo& rhs) -> bool {
        return lhs.priority > rhs.priority;
    });
    TAG_LOGD(AAFwkTag::APPKIT, "the size of overlay is: %{public}zu", overlayModuleInfos.size());
    return ERR_OK;
}

std::vector<std::string> ContextImpl::GetAddOverlayPaths(
    const std::vector<AppExecFwk::OverlayModuleInfo> &overlayModuleInfos)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::vector<std::string> addPaths;
    for (auto it : overlayModuleInfos) {
        auto iter = std::find_if(
            overlayModuleInfos_.begin(), overlayModuleInfos_.end(), [it](AppExecFwk::OverlayModuleInfo item) {
                return it.moduleName == item.moduleName;
            });
        if ((iter != overlayModuleInfos_.end()) && (it.state == AppExecFwk::OverlayState::OVERLAY_ENABLE)) {
            iter->state = it.state;
            ChangeToLocalPath(iter->bundleName, iter->hapPath, iter->hapPath);
            TAG_LOGD(AAFwkTag::APPKIT, "add path:%{private}s", iter->hapPath.c_str());
            addPaths.emplace_back(iter->hapPath);
        }
    }

    return addPaths;
}

std::vector<std::string> ContextImpl::GetRemoveOverlayPaths(
    const std::vector<AppExecFwk::OverlayModuleInfo> &overlayModuleInfos)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::vector<std::string> removePaths;
    for (auto it : overlayModuleInfos) {
        auto iter = std::find_if(
            overlayModuleInfos_.begin(), overlayModuleInfos_.end(), [it](AppExecFwk::OverlayModuleInfo item) {
                return it.moduleName == item.moduleName;
            });
        if ((iter != overlayModuleInfos_.end()) && (it.state != AppExecFwk::OverlayState::OVERLAY_ENABLE)) {
            iter->state = it.state;
            ChangeToLocalPath(iter->bundleName, iter->hapPath, iter->hapPath);
            TAG_LOGD(AAFwkTag::APPKIT, "remove path:%{private}s", iter->hapPath.c_str());
            removePaths.emplace_back(iter->hapPath);
        }
    }

    return removePaths;
}

void ContextImpl::OnOverlayChanged(const EventFwk::CommonEventData &data,
    const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager, const std::string &bundleName,
    const std::string &moduleName, const std::string &loadPath)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPKIT, "begin");
    auto want = data.GetWant();
    std::string action = want.GetAction();
    if (action != OVERLAY_STATE_CHANGED) {
        TAG_LOGD(AAFwkTag::APPKIT, "Not this subscribe, action: %{public}s", action.c_str());
        return;
    }
    if (GetBundleName() != bundleName) {
        TAG_LOGD(AAFwkTag::APPKIT, "Not this app, bundleName: %{public}s", bundleName.c_str());
        return;
    }
    bool isEnable = data.GetWant().GetBoolParam(AppExecFwk::Constants::OVERLAY_STATE, false);
    // 1.get overlay hapPath
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "resourceManager is nullptr");
        return;
    }
    if (overlayModuleInfos_.size() == 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "overlayModuleInfos is empty");
        return;
    }
    std::vector<AppExecFwk::OverlayModuleInfo> overlayModuleInfos;
    auto res = GetOverlayModuleInfos(bundleName, moduleName, overlayModuleInfos);
    if (res != ERR_OK) {
        return;
    }

    // 2.add/remove overlay hapPath
    if (loadPath.empty() || overlayModuleInfos.size() == 0) {
        TAG_LOGW(AAFwkTag::APPKIT, "There is not any hapPath in overlayModuleInfo");
    } else {
        if (isEnable) {
            std::vector<std::string> overlayPaths = GetAddOverlayPaths(overlayModuleInfos);
            if (!resourceManager->AddResource(loadPath, overlayPaths)) {
                TAG_LOGE(AAFwkTag::APPKIT, "AddResource error");
            }
        } else {
            std::vector<std::string> overlayPaths = GetRemoveOverlayPaths(overlayModuleInfos);
            if (!resourceManager->RemoveResource(loadPath, overlayPaths)) {
                TAG_LOGE(AAFwkTag::APPKIT, "RemoveResource error");
            }
        }
    }
}

void ContextImpl::ChangeToLocalPath(const std::string &bundleName,
    const std::string &sourceDir, std::string &localPath)
{
    std::regex pattern(std::string(ABS_CODE_PATH) + std::string(FILE_SEPARATOR) + bundleName);
    if (sourceDir.empty()) {
        return;
    }
    if (std::regex_search(localPath, std::regex(bundleName))) {
        localPath = std::regex_replace(localPath, pattern, std::string(LOCAL_CODE_PATH));
    } else {
        localPath = std::regex_replace(localPath, std::regex(ABS_CODE_PATH), LOCAL_BUNDLES);
    }
}

void ContextImpl::ClearUpApplicationData()
{
    int errCode = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->ClearUpApplicationDataBySelf();
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Delete bundle side user data by self is fail.");
        return;
    }
}

int32_t ContextImpl::SetSupportedProcessCacheSelf(bool isSupport)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Called");
    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (appMgrClient == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "appMgrClient is nullptr");
        return ERR_INVALID_VALUE;
    }
    return appMgrClient->SetSupportedProcessCacheSelf(isSupport);
}

void ContextImpl::PrintTokenInfo() const
{
    if (token_ == nullptr) {
        TAG_LOGI(AAFwkTag::EXT, "null token");
        return;
    }
    if (!token_->IsProxyObject()) {
        TAG_LOGI(AAFwkTag::EXT, "token not proxy");
        return;
    }
    IPCObjectProxy *tokenProxyObject = reinterpret_cast<IPCObjectProxy *>(token_.GetRefPtr());
    if (tokenProxyObject != nullptr) {
        std::string remoteDescriptor = Str16ToStr8(tokenProxyObject->GetInterfaceDescriptor());
        TAG_LOGI(AAFwkTag::EXT, "handle: %{public}d, descriptor: %{public}s",
            tokenProxyObject->GetHandle(), remoteDescriptor.c_str());
    }
}
}  // namespace AbilityRuntime
}  // namespace OHOS
