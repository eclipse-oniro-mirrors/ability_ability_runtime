/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "bundle_mgr_helper.h"

#include "bundle_mgr_service_death_recipient.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
BundleMgrHelper::BundleMgrHelper() {}

BundleMgrHelper::~BundleMgrHelper()
{
    if (bundleMgr_ != nullptr && bundleMgr_->AsObject() != nullptr && deathRecipient_ != nullptr) {
        bundleMgr_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
}

ErrCode BundleMgrHelper::GetNameForUid(const int32_t uid, std::string &name)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect.");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetNameForUid(uid, name);
}

ErrCode BundleMgrHelper::GetNameAndIndexForUid(const int32_t uid, std::string &bundleName, int32_t &appIndex)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect.");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetNameAndIndexForUid(uid, bundleName, appIndex);
}

bool BundleMgrHelper::GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo,
    int32_t userId)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect.");
        return false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetBundleInfo(bundleName, flag, bundleInfo, userId);
}

ErrCode BundleMgrHelper::InstallSandboxApp(const std::string &bundleName, int32_t dlpType, int32_t userId,
    int32_t &appIndex)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "The bundleName is empty.");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR;
    }
    auto bundleInstaller = ConnectBundleInstaller();
    if (bundleInstaller == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect.");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_INTERNAL_ERROR;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleInstaller->InstallSandboxApp(bundleName, dlpType, userId, appIndex);
}

ErrCode BundleMgrHelper::UninstallSandboxApp(const std::string &bundleName, int32_t appIndex, int32_t userId)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    if (bundleName.empty() || appIndex <= AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "The params are invalid.");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR;
    }
    auto bundleInstaller = ConnectBundleInstaller();
    if (bundleInstaller == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect.");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_INTERNAL_ERROR;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleInstaller->UninstallSandboxApp(bundleName, appIndex, userId);
}

ErrCode BundleMgrHelper::GetUninstalledBundleInfo(const std::string bundleName, BundleInfo &bundleInfo)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect.");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetUninstalledBundleInfo(bundleName, bundleInfo);
}

ErrCode BundleMgrHelper::GetSandboxBundleInfo(
    const std::string &bundleName, int32_t appIndex, int32_t userId, BundleInfo &info)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    if (bundleName.empty() || appIndex <= AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "The params are invalid.");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR;
    }
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_INTERNAL_ERROR;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetSandboxBundleInfo(bundleName, appIndex, userId, info);
}

ErrCode BundleMgrHelper::GetSandboxAbilityInfo(const Want &want, int32_t appIndex, int32_t flags, int32_t userId,
    AbilityInfo &abilityInfo)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    if (appIndex <= AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "The params are invalid");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR;
    }
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_INTERNAL_ERROR;
    }

    AAFwk::Want newWant = want;
    newWant.RemoveAllFd();

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetSandboxAbilityInfo(newWant, appIndex, flags, userId, abilityInfo);
}

ErrCode BundleMgrHelper::GetSandboxExtAbilityInfos(const Want &want, int32_t appIndex, int32_t flags,
    int32_t userId, std::vector<ExtensionAbilityInfo> &extensionInfos)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    if (appIndex <= AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "The params are invalid");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR;
    }
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_INTERNAL_ERROR;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetSandboxExtAbilityInfos(want, appIndex, flags, userId, extensionInfos);
}

ErrCode BundleMgrHelper::GetSandboxHapModuleInfo(const AbilityInfo &abilityInfo, int32_t appIndex, int32_t userId,
    HapModuleInfo &hapModuleInfo)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    if (appIndex <= AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "The params are invalid");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR;
    }
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return ERR_APPEXECFWK_SANDBOX_INSTALL_INTERNAL_ERROR;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetSandboxHapModuleInfo(abilityInfo, appIndex, userId, hapModuleInfo);
}

sptr<IBundleMgr> BundleMgrHelper::Connect()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    std::lock_guard<std::mutex> lock(mutex_);
    if (bundleMgr_ == nullptr) {
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemAbilityManager == nullptr) {
            TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to get system ability manager");
            return nullptr;
        }

        sptr<IRemoteObject> remoteObject_ = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (remoteObject_ == nullptr || (bundleMgr_ = iface_cast<IBundleMgr>(remoteObject_)) == nullptr) {
            TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to get bundle mgr service remote object");
            return nullptr;
        }
        std::weak_ptr<BundleMgrHelper> weakPtr = shared_from_this();
        auto deathCallback = [weakPtr](const wptr<IRemoteObject>& object) {
            auto sharedPtr = weakPtr.lock();
            if (sharedPtr == nullptr) {
                TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Bundle helper instance is nullptr");
                return;
            }
            sharedPtr->OnDeath();
        };
        deathRecipient_ = new (std::nothrow) BundleMgrServiceDeathRecipient(deathCallback);
        if (deathRecipient_ == nullptr) {
            TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to create death recipient");
            return nullptr;
        }
        if (bundleMgr_->AsObject() != nullptr) {
            bundleMgr_->AsObject()->AddDeathRecipient(deathRecipient_);
        }
    }

    return bundleMgr_;
}

sptr<IBundleInstaller> BundleMgrHelper::ConnectBundleInstaller()
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (bundleInstaller_ != nullptr) {
            return bundleInstaller_;
        }
    }

    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return nullptr;
    }
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<std::mutex> lock(mutex_);
    bundleInstaller_ = bundleMgr->GetBundleInstaller();
    if ((bundleInstaller_ == nullptr) || (bundleInstaller_->AsObject() == nullptr)) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to get bundle installer proxy");
        return nullptr;
    }

    return bundleInstaller_;
}

void BundleMgrHelper::OnDeath()
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    std::lock_guard<std::mutex> lock(mutex_);
    if (bundleMgr_ == nullptr || bundleMgr_->AsObject() == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "bundleMgr_ is nullptr");
        return;
    }
    bundleMgr_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    bundleMgr_ = nullptr;
    bundleInstaller_ = nullptr;
}

bool BundleMgrHelper::GetBundleInfo(const std::string &bundleName, int32_t flags,
    BundleInfo &bundleInfo, int32_t userId)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetBundleInfo(bundleName, flags, bundleInfo, userId);
}

bool BundleMgrHelper::GetHapModuleInfo(const AbilityInfo &abilityInfo, HapModuleInfo &hapModuleInfo)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetHapModuleInfo(abilityInfo, hapModuleInfo);
}

std::string BundleMgrHelper::GetAbilityLabel(const std::string &bundleName, const std::string &abilityName)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return "";
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetAbilityLabel(bundleName, abilityName);
}

std::string BundleMgrHelper::GetAppType(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return "";
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetAppType(bundleName);
}

ErrCode BundleMgrHelper::GetBaseSharedBundleInfos(
    const std::string &bundleName, std::vector<BaseSharedBundleInfo> &baseSharedBundleInfos,
    GetDependentBundleInfoFlag flag)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetBaseSharedBundleInfos(bundleName, baseSharedBundleInfos, flag);
}

ErrCode BundleMgrHelper::GetBundleInfoForSelf(int32_t flags, BundleInfo &bundleInfo)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetBundleInfoForSelf(flags, bundleInfo);
}

ErrCode BundleMgrHelper::GetDependentBundleInfo(const std::string &sharedBundleName, BundleInfo &sharedBundleInfo,
    GetDependentBundleInfoFlag flag)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetDependentBundleInfo(sharedBundleName, sharedBundleInfo, flag);
}

bool BundleMgrHelper::GetGroupDir(const std::string &dataGroupId, std::string &dir)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetGroupDir(dataGroupId, dir);
}

sptr<IOverlayManager> BundleMgrHelper::GetOverlayManagerProxy()
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return nullptr;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetOverlayManagerProxy();
}

bool BundleMgrHelper::QueryAbilityInfo(const Want &want, AbilityInfo &abilityInfo)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    AAFwk::Want newWant = want;
    newWant.RemoveAllFd();
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->QueryAbilityInfo(newWant, abilityInfo);
}

bool BundleMgrHelper::QueryAbilityInfo(const Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    AAFwk::Want newWant = want;
    newWant.RemoveAllFd();
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->QueryAbilityInfo(newWant, flags, userId, abilityInfo);
}

bool BundleMgrHelper::GetBundleInfos(int32_t flags, std::vector<BundleInfo> &bundleInfos, int32_t userId)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetBundleInfos(flags, bundleInfos, userId);
}

bool BundleMgrHelper::GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo> &bundleInfos, int32_t userId)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetBundleInfos(flag, bundleInfos, userId);
}

sptr<IQuickFixManager> BundleMgrHelper::GetQuickFixManagerProxy()
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return nullptr;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetQuickFixManagerProxy();
}

bool BundleMgrHelper::ProcessPreload(const Want &want)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    AAFwk::Want newWant = want;
    newWant.RemoveAllFd();
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->ProcessPreload(newWant);
}

sptr<IAppControlMgr> BundleMgrHelper::GetAppControlProxy()
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return nullptr;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetAppControlProxy();
}

bool BundleMgrHelper::QueryExtensionAbilityInfos(const Want &want, const int32_t &flag, const int32_t &userId,
    std::vector<ExtensionAbilityInfo> &extensionInfos)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    AAFwk::Want newWant = want;
    newWant.RemoveAllFd();
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->QueryExtensionAbilityInfos(newWant, flag, userId, extensionInfos);
}

ErrCode BundleMgrHelper::GetBundleInfoV9(
    const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo, int32_t userId)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetBundleInfoV9(bundleName, flags, bundleInfo, userId);
}

bool BundleMgrHelper::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int32_t userId, ApplicationInfo &appInfo)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetApplicationInfo(appName, flag, userId, appInfo);
}

bool BundleMgrHelper::GetApplicationInfo(
    const std::string &appName, int32_t flags, int32_t userId, ApplicationInfo &appInfo)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetApplicationInfo(appName, flags, userId, appInfo);
}

bool BundleMgrHelper::GetApplicationInfoWithAppIndex(
    const std::string &appName, int32_t appIndex, int32_t userId, ApplicationInfo &appInfo)
{
    TAG_LOGI(AAFwkTag::BUNDLEMGRHELPER, "appName: %{public}s, appIndex: %{public}d", appName.c_str(), appIndex);
    if (appIndex < 0) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Invalid appIndex");
        return false;
    }
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    BundleInfo bundleInfo;
    if (appIndex == 0) {
        if (bundleMgr->GetApplicationInfo(appName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, userId, appInfo)) {
            return true;
        }
    } else if (appIndex <= AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        if (bundleMgr->GetCloneBundleInfo(appName,
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION),
            appIndex, bundleInfo, userId) == ERR_OK) {
            appInfo = bundleInfo.applicationInfo;
            return true;
        }
    } else {
        if (bundleMgr->GetSandboxBundleInfo(appName, appIndex, userId, bundleInfo) == ERR_OK) {
            appInfo = bundleInfo.applicationInfo;
            return true;
        }
    }
    TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "GetApplicationInfo failed");
    return false;
}

bool BundleMgrHelper::UnregisterBundleEventCallback(const sptr<IBundleEventCallback> &bundleEventCallback)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    if (bundleEventCallback == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "The bundleEventCallback is nullptr");
        return false;
    }

    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->UnregisterBundleEventCallback(bundleEventCallback);
}

bool BundleMgrHelper::QueryExtensionAbilityInfoByUri(
    const std::string &uri, int32_t userId, ExtensionAbilityInfo &extensionAbilityInfo)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->QueryExtensionAbilityInfoByUri(uri, userId, extensionAbilityInfo);
}

bool BundleMgrHelper::ImplicitQueryInfoByPriority(
    const Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo, ExtensionAbilityInfo &extensionInfo)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }
    AAFwk::Want newWant = want;
    newWant.RemoveAllFd();
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->ImplicitQueryInfoByPriority(newWant, flags, userId, abilityInfo, extensionInfo);
}

bool BundleMgrHelper::QueryAbilityInfoByUri(const std::string &abilityUri, int32_t userId, AbilityInfo &abilityInfo)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->QueryAbilityInfoByUri(abilityUri, userId, abilityInfo);
}

bool BundleMgrHelper::QueryAbilityInfo(
    const Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo, const sptr<IRemoteObject> &callBack)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    AAFwk::Want newWant = want;
    newWant.RemoveAllFd();
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->QueryAbilityInfo(newWant, flags, userId, abilityInfo, callBack);
}

void BundleMgrHelper::UpgradeAtomicService(const Want &want, int32_t userId)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return;
    }

    AAFwk::Want newWant = want;
    newWant.RemoveAllFd();
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    bundleMgr->UpgradeAtomicService(newWant, userId);
}

bool BundleMgrHelper::ImplicitQueryInfos(const Want &want, int32_t flags, int32_t userId, bool withDefault,
    std::vector<AbilityInfo> &abilityInfos, std::vector<ExtensionAbilityInfo> &extensionInfos, bool &findDefaultApp)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    AAFwk::Want newWant = want;
    newWant.RemoveAllFd();
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    bool ret = bundleMgr->ImplicitQueryInfos(newWant, flags, userId, withDefault, abilityInfos,
        extensionInfos, findDefaultApp);
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "findDefaultApp is %{public}d.", findDefaultApp);
    return ret;
}

bool BundleMgrHelper::CleanBundleDataFiles(const std::string &bundleName, int32_t userId, int32_t appCloneIndex)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->CleanBundleDataFiles(bundleName, userId, appCloneIndex);
}

bool BundleMgrHelper::QueryDataGroupInfos(
    const std::string &bundleName, int32_t userId, std::vector<DataGroupInfo> &infos)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->QueryDataGroupInfos(bundleName, userId, infos);
}

bool BundleMgrHelper::RegisterBundleEventCallback(const sptr<IBundleEventCallback> &bundleEventCallback)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    if (bundleEventCallback == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "The bundleEventCallback is nullptr");
        return false;
    }

    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->RegisterBundleEventCallback(bundleEventCallback);
}

bool BundleMgrHelper::GetHapModuleInfo(const AbilityInfo &abilityInfo, int32_t userId, HapModuleInfo &hapModuleInfo)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetHapModuleInfo(abilityInfo, userId, hapModuleInfo);
}

bool BundleMgrHelper::QueryAppGalleryBundleName(std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->QueryAppGalleryBundleName(bundleName);
}

ErrCode BundleMgrHelper::GetUidByBundleName(const std::string &bundleName, int32_t userId, int32_t appCloneIndex)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetUidByBundleName(bundleName, userId, appCloneIndex);
}

ErrCode BundleMgrHelper::QueryExtensionAbilityInfosOnlyWithTypeName(const std::string &extensionTypeName,
    const uint32_t flag, const int32_t userId, std::vector<ExtensionAbilityInfo> &extensionInfos)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->QueryExtensionAbilityInfosOnlyWithTypeName(extensionTypeName, flag, userId, extensionInfos);
}

sptr<IDefaultApp> BundleMgrHelper::GetDefaultAppProxy()
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return nullptr;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetDefaultAppProxy();
}

ErrCode BundleMgrHelper::GetJsonProfile(ProfileType profileType, const std::string &bundleName,
    const std::string &moduleName, std::string &profile, int32_t userId)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetJsonProfile(profileType, bundleName, moduleName, profile, userId);
}

ErrCode BundleMgrHelper::GetLaunchWantForBundle(const std::string &bundleName, Want &want, int32_t userId)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    want.RemoveAllFd();
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetLaunchWantForBundle(bundleName, want, userId);
}

ErrCode BundleMgrHelper::QueryCloneAbilityInfo(const ElementName &element, int32_t flags, int32_t appCloneIndex,
    AbilityInfo &abilityInfo, int32_t userId)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->QueryCloneAbilityInfo(element, flags, appCloneIndex, abilityInfo, userId);
}

ErrCode BundleMgrHelper::GetCloneBundleInfo(const std::string &bundleName, int32_t flags, int32_t appCloneIndex,
    BundleInfo &bundleInfo, int32_t userId)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetCloneBundleInfo(bundleName, flags, appCloneIndex, bundleInfo, userId);
}

ErrCode BundleMgrHelper::QueryCloneExtensionAbilityInfoWithAppIndex(const ElementName &element, int32_t flags,
    int32_t appCloneIndex, ExtensionAbilityInfo &extensionInfo, int32_t userId)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->QueryCloneExtensionAbilityInfoWithAppIndex(element, flags, appCloneIndex, extensionInfo, userId);
}

ErrCode BundleMgrHelper::GetCloneAppIndexes(const std::string &bundleName, std::vector<int32_t> &appIndexes,
    int32_t userId)
{
    TAG_LOGD(AAFwkTag::BUNDLEMGRHELPER, "Called");
    auto bundleMgr = Connect();
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::BUNDLEMGRHELPER, "Failed to connect");
        return ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    return bundleMgr->GetCloneAppIndexes(bundleName, appIndexes, userId);
}

}  // namespace AppExecFwk
}  // namespace OHOS