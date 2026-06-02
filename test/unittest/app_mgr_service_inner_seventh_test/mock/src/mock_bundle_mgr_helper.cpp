/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "mock_my_status.h"

namespace OHOS {
namespace AppExecFwk {

BundleMgrHelper::BundleMgrHelper()
{
}

BundleMgrHelper::~BundleMgrHelper()
{
}

void BundleMgrHelper::PreConnect()
{
}

void BundleMgrHelper::ConnectTillSuccess()
{
}

void BundleMgrHelper::SetBmsReady(bool bmsReady)
{
}

ErrCode BundleMgrHelper::GetNameForUid(const int32_t uid, std::string& name)
{
    name = AAFwk::MyStatus::GetInstance().getNameForUid_;
    return ERR_OK;
}

ErrCode BundleMgrHelper::GetNameAndIndexForUid(const int32_t uid, std::string& bundleName, int32_t& appIndex)
{
    return ERR_OK;
}

bool BundleMgrHelper::GetBundleInfo(const std::string& bundleName,
    const BundleFlag flag, BundleInfo& bundleInfo, int32_t userId)
{
    return false;
}

ErrCode BundleMgrHelper::InstallSandboxApp(const std::string& bundleName,
    int32_t dlpType, int32_t userId, int32_t& appIndex)
{
    return ERR_OK;
}

ErrCode BundleMgrHelper::UninstallSandboxApp(const std::string& bundleName,
    int32_t appIndex, int32_t userId)
{
    return ERR_OK;
}

ErrCode BundleMgrHelper::GetUninstalledBundleInfo(const std::string bundleName, BundleInfo& bundleInfo)
{
    return ERR_OK;
}

ErrCode BundleMgrHelper::GetSandboxBundleInfo(const std::string& bundleName,
    int32_t appIndex, int32_t userId, BundleInfo& info)
{
    return AAFwk::MyStatus::GetInstance().getSandboxBundleInfo_;
}

ErrCode BundleMgrHelper::GetSandboxAbilityInfo(const Want& want,
    int32_t appIndex, int32_t flags, int32_t userId, AbilityInfo& abilityInfo)
{
    return ERR_OK;
}

ErrCode BundleMgrHelper::GetSandboxExtAbilityInfos(const Want& want, int32_t appIndex, int32_t flags, int32_t userId,
    std::vector<ExtensionAbilityInfo>& extensionInfos)
{
    return ERR_OK;
}

ErrCode BundleMgrHelper::GetSandboxHapModuleInfo(
    const AbilityInfo& abilityInfo, int32_t appIndex, int32_t userId, HapModuleInfo& hapModuleInfo)
{
    return AAFwk::MyStatus::GetInstance().getSandboxHapModuleInfo_;
}

bool BundleMgrHelper::GetBundleInfo(const std::string& bundleName,
    int32_t flags, BundleInfo& bundleInfo, int32_t userId)
{
    return false;
}

std::string BundleMgrHelper::GetAppIdByBundleName(const std::string& bundleName, const int32_t userId)
{
    return "";
}

bool BundleMgrHelper::GetHapModuleInfo(const AbilityInfo& abilityInfo, HapModuleInfo& hapModuleInfo)
{
    return AAFwk::MyStatus::GetInstance().getHapModuleInfo_;
}

std::string BundleMgrHelper::GetAbilityLabel(const std::string& bundleName, const std::string& abilityName)
{
    return "";
}


std::string BundleMgrHelper::GetAppType(const std::string& bundleName)
{
    return "";
}

ErrCode BundleMgrHelper::GetBaseSharedBundleInfos(const std::string& bundleName,
    std::vector<BaseSharedBundleInfo>& baseSharedBundleInfos,
    GetDependentBundleInfoFlag flag)
{
    baseSharedBundleInfos = AAFwk::MyStatus::GetInstance().baseSharedBundleInfos_;
    return AAFwk::MyStatus::GetInstance().getBaseSharedBundleInfos_;
}

ErrCode BundleMgrHelper::GetBundleInfoForSelf(int32_t flags, BundleInfo& bundleInfo)
{
    return ERR_OK;
}

ErrCode BundleMgrHelper::GetDependentBundleInfo(const std::string& sharedBundleName, BundleInfo& sharedBundleInfo,
    GetDependentBundleInfoFlag flag)
{
    return ERR_OK;
}

bool BundleMgrHelper::GetGroupDir(const std::string& dataGroupId, std::string& dir)
{
    return false;
}

sptr<IOverlayManager> BundleMgrHelper::GetOverlayManagerProxy()
{
    AAFwk::MyStatus::GetInstance().getOverlayCall_++;
    return AAFwk::MyStatus::GetInstance().getOverlay_;
}

bool BundleMgrHelper::QueryAbilityInfo(const Want& want, AbilityInfo& abilityInfo)
{
    return false;
}

bool BundleMgrHelper::QueryAbilityInfo(const Want& want, int32_t flags, int32_t userId, AbilityInfo& abilityInfo)
{
    return false;
}

bool BundleMgrHelper::GetBundleInfos(
    int32_t flags, std::vector<BundleInfo>& bundleInfos, int32_t userId)
{
    return false;
}

sptr<IQuickFixManager> BundleMgrHelper::GetQuickFixManagerProxy()
{
    return nullptr;
}

bool BundleMgrHelper::ProcessPreload(const Want& want)
{
    return false;
}

sptr<IAppControlMgr> BundleMgrHelper::GetAppControlProxy()
{
    return nullptr;
}

bool BundleMgrHelper::QueryExtensionAbilityInfos(const Want& want, const int32_t& flag, const int32_t& userId,
    std::vector<ExtensionAbilityInfo>& extensionInfos)
{
    return false;
}

ErrCode BundleMgrHelper::GetBundleInfoV9(const std::string& bundleName,
    int32_t flags, BundleInfo& bundleInfo, int32_t userId)
{
    return AAFwk::MyStatus::GetInstance().getBundleInfoV9_;
}

ErrCode BundleMgrHelper::GetBundleInfosV9(int32_t flags,
    std::vector<BundleInfo> &bundleInfos, int32_t userId)
{
    return ERR_OK;
}

bool BundleMgrHelper::GetApplicationInfo(
    const std::string& appName, const ApplicationFlag flag, const int32_t userId, ApplicationInfo& appInfo)
{
    appInfo = AAFwk::MyStatus::GetInstance().applicationInfo_;
    return AAFwk::MyStatus::GetInstance().getApplicationInfo_;
}

bool BundleMgrHelper::GetApplicationInfo(const std::string& appName,
    int32_t flags, int32_t userId, ApplicationInfo& appInfo)
{
    return false;
}

bool BundleMgrHelper::GetApplicationInfoWithAppIndex(
    const std::string& appName, int32_t appIndex, int32_t userId, ApplicationInfo& appInfo)
{
    return false;
}

ErrCode BundleMgrHelper::GetJsonProfile(ProfileType profileType, const std::string& bundleName,
    const std::string& moduleName, std::string& profile, int32_t userId)
{
    return ERR_OK;
}

bool BundleMgrHelper::UnregisterBundleEventCallback(const sptr<IBundleEventCallback>& bundleEventCallback)
{
    return false;
}

bool BundleMgrHelper::QueryExtensionAbilityInfoByUri(
    const std::string& uri, int32_t userId, ExtensionAbilityInfo& extensionAbilityInfo)
{
    return false;
}

bool BundleMgrHelper::QueryExtensionAbilityInfoByUriOptimal(
    const std::string& uri, int32_t userId, ExtensionAbilityInfo& extensionAbilityInfo)
{
    return false;
}

bool BundleMgrHelper::ImplicitQueryInfoByPriority(
    const Want& want, int32_t flags, int32_t userId, AbilityInfo& abilityInfo, ExtensionAbilityInfo& extensionInfo)
{
    return false;
}

bool BundleMgrHelper::QueryAbilityInfoByUri(const std::string& abilityUri, int32_t userId, AbilityInfo& abilityInfo)
{
    return false;
}

bool BundleMgrHelper::QueryAbilityInfo(
    const Want& want, int32_t flags, int32_t userId, AbilityInfo& abilityInfo, const sptr<IRemoteObject>& callBack)
{
    return false;
}

void BundleMgrHelper::UpgradeAtomicService(const Want& want, int32_t userId) {}

bool BundleMgrHelper::ImplicitQueryInfos(const Want& want, int32_t flags, int32_t userId, bool withDefault,
    std::vector<AbilityInfo>& abilityInfos, std::vector<ExtensionAbilityInfo>& extensionInfos,
    bool& findDefaultApp)
{
    return false;
}

bool BundleMgrHelper::CleanBundleDataFiles(
    const std::string& bundleName, int32_t userId, int32_t appCloneIndex, int32_t callerUid)
{
    return AAFwk::MyStatus::GetInstance().cleanBundleDataFiles_;
}

bool BundleMgrHelper::QueryDataGroupInfos(const std::string& bundleName,
    int32_t userId, std::vector<DataGroupInfo>& infos)
{
    infos = AAFwk::MyStatus::GetInstance().queryData_;
    return AAFwk::MyStatus::GetInstance().queryDataGroupInfos_;
}

bool BundleMgrHelper::RegisterBundleEventCallback(const sptr<IBundleEventCallback>& bundleEventCallback)
{
    return false;
}

bool BundleMgrHelper::GetBundleInfos(
    const BundleFlag flag, std::vector<BundleInfo>& bundleInfos, int32_t userId)
{
    return false;
}

bool BundleMgrHelper::GetHapModuleInfo(const AbilityInfo& abilityInfo, int32_t userId, HapModuleInfo& hapModuleInfo)
{
    return AAFwk::MyStatus::GetInstance().getHapModuleInfo_;
}

bool BundleMgrHelper::QueryAppGalleryBundleName(std::string& bundleName)
{
    return false;
}

ErrCode BundleMgrHelper::GetUidByBundleName(const std::string& bundleName, int32_t userId, int32_t appCloneIndex)
{
    return ERR_OK;
}

ErrCode BundleMgrHelper::QueryExtensionAbilityInfosOnlyWithTypeName(const std::string& extensionTypeName,
    const uint32_t flag, const int32_t userId, std::vector<ExtensionAbilityInfo>& extensionInfos)
{
    return ERR_OK;
}

sptr<IDefaultApp> BundleMgrHelper::GetDefaultAppProxy()
{
    return nullptr;
}

ErrCode BundleMgrHelper::GetLaunchWantForBundle(const std::string& bundleName, Want& want, int32_t userId)
{
    return ERR_OK;
}

ErrCode BundleMgrHelper::QueryCloneAbilityInfo(
    const ElementName& element, int32_t flags, int32_t appCloneIndex, AbilityInfo& abilityInfo, int32_t userId)
{
    return ERR_OK;
}

ErrCode BundleMgrHelper::GetCloneBundleInfo(
    const std::string& bundleName, int32_t flags, int32_t appCloneIndex, BundleInfo& bundleInfo, int32_t userId)
{
    return AAFwk::MyStatus::GetInstance().getCloneBundleInfo_;
}

ErrCode BundleMgrHelper::QueryCloneExtensionAbilityInfoWithAppIndex(const ElementName& element,
    int32_t flags, int32_t appCloneIndex, ExtensionAbilityInfo& extensionInfo, int32_t userId)
{
    return ERR_OK;
}

ErrCode BundleMgrHelper::GetCloneAppIndexes(const std::string& bundleName,
    std::vector<int32_t>& appIndexes, int32_t userId)
{
    return ERR_OK;
}

ErrCode BundleMgrHelper::GetSignatureInfoByBundleName(const std::string& bundleName, SignatureInfo& signatureInfo)
{
    return ERR_OK;
}

std::string BundleMgrHelper::GetStringById(
    const std::string& bundleName, const std::string& moduleName, uint32_t resId, int32_t userId)
{
    return "";
}

std::string BundleMgrHelper::GetDataDir(const std::string& bundleName, const int32_t appIndex)
{
    return "";
}

ErrCode BundleMgrHelper::GetPluginInfosForSelf(std::vector<PluginBundleInfo> &pluginBundleInfos)
{
    return ERR_OK;
}

ErrCode BundleMgrHelper::GetPluginAbilityInfo(const std::string &hostBundleName, const std::string &pluginBundleName,
    const std::string &pluginModuleName, const std::string &pluginAbilityName, int32_t userId,
    AbilityInfo &pluginAbilityInfo)
{
    return ERR_OK;
}

ErrCode BundleMgrHelper::RegisterPluginEventCallback(sptr<IBundleEventCallback> pluginEventCallback)
{
    return ERR_OK;
}

ErrCode BundleMgrHelper::UnregisterPluginEventCallback(sptr<IBundleEventCallback> pluginEventCallback)
{
    return ERR_OK;
}
} // namespace AppExecFwk
} // namespace OHOS