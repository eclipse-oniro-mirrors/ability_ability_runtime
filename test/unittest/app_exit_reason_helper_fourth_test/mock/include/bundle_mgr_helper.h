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

#ifndef OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H
#define OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H

#include "ability_keep_alive_service.h"
#include "app_control_interface.h"
#include "bundle_info.h"

namespace OHOS {
namespace AppExecFwk {
class BundleMgrHelper : public std::enable_shared_from_this<BundleMgrHelper> {
public:
    BundleMgrHelper();

    ~BundleMgrHelper();

    ErrCode GetNameAndIndexForUid(const int32_t uid, std::string& bundleName, int32_t& appIndex);

    void PreConnect() {}

    ErrCode GetNameForUid(const int32_t uid, std::string& name)
    {
        return -1;
    }

    bool GetBundleInfo(const std::string& bundleName, const BundleFlag flag, BundleInfo& bundleInfo, int32_t userId)
    {
        return false;
    }

    ErrCode InstallSandboxApp(const std::string& bundleName, int32_t dlpType, int32_t userId, int32_t& appIndex)
    {
        return -1;
    }

    ErrCode UninstallSandboxApp(const std::string& bundleName, int32_t appIndex, int32_t userId)
    {
        return -1;
    }

    ErrCode GetUninstalledBundleInfo(const std::string bundleName, BundleInfo& bundleInfo)
    {
        return -1;
    }

    ErrCode GetSandboxBundleInfo(const std::string& bundleName, int32_t appIndex, int32_t userId, BundleInfo& info)
    {
        return -1;
    }

    ErrCode GetSandboxAbilityInfo(
        const AAFwk::Want& want, int32_t appIndex, int32_t flags, int32_t userId, AbilityInfo& abilityInfo)
    {
        return -1;
    }

    ErrCode GetSandboxExtAbilityInfos(const AAFwk::Want& want, int32_t appIndex, int32_t flags, int32_t userId,
        std::vector<ExtensionAbilityInfo>& extensionInfos)
    {
        return -1;
    }

    ErrCode GetSandboxHapModuleInfo(
        const AbilityInfo& abilityInfo, int32_t appIndex, int32_t userId, HapModuleInfo& hapModuleInfo)
    {
        return -1;
    }

    std::string GetAppIdByBundleName(const std::string& bundleName, const int32_t userId)
    {
        return "";
    }

    void ConnectTillSuccess() {}

    void SetBmsReady(bool bmsReady) {}

    void OnDeath() {}

    bool GetBundleInfo(const std::string& bundleName, int32_t flags, BundleInfo& bundleInfo, int32_t userId)
    {
        return false;
    }

    bool GetHapModuleInfo(const AbilityInfo& abilityInfo, HapModuleInfo& hapModuleInfo)
    {
        return false;
    }

    std::string GetAbilityLabel(const std::string& bundleName, const std::string& abilityName)
    {
        return "";
    }

    std::string GetAppType(const std::string& bundleName)
    {
        return "";
    }

    ErrCode GetBundleInfoForSelf(int32_t flags, BundleInfo& bundleInfo)
    {
        return -1;
    }

    ErrCode GetDependentBundleInfo(
        const std::string& sharedBundleName, BundleInfo& sharedBundleInfo, GetDependentBundleInfoFlag flag)
    {
        return -1;
    }

    bool GetGroupDir(const std::string& dataGroupId, std::string& dir)
    {
        return false;
    }

    bool QueryAbilityInfo(const AAFwk::Want& want, AbilityInfo& abilityInfo)
    {
        return false;
    }

    bool QueryAbilityInfo(const AAFwk::Want& want, int32_t flags, int32_t userId, AbilityInfo& abilityInfo)
    {
        return false;
    }

    bool GetBundleInfos(int32_t flags, std::vector<BundleInfo>& bundleInfos, int32_t userId)
    {
        return false;
    }

    bool GetBundleInfos(const BundleFlag flag, std::vector<BundleInfo>& bundleInfos, int32_t userId)
    {
        return false;
    }

    bool ProcessPreload(const AAFwk::Want& want)
    {
        return false;
    }

    sptr<IAppControlMgr> GetAppControlProxy()
    {
        return nullptr;
    }

    bool QueryExtensionAbilityInfos(const AAFwk::Want& want, const int32_t& flag, const int32_t& userId,
        std::vector<ExtensionAbilityInfo>& extensionInfos)
    {
        return false;
    }

    ErrCode GetBundleInfoV9(const std::string& bundleName, int32_t flags, BundleInfo& bundleInfo, int32_t userId)
    {
        return -1;
    }

    bool GetApplicationInfo(
        const std::string& appName, const ApplicationFlag flag, const int32_t userId, ApplicationInfo& appInfo)
    {
        return false;
    }

    bool GetApplicationInfo(const std::string& appName, int32_t flags, int32_t userId, ApplicationInfo& appInfo)
    {
        return false;
    }

    bool GetApplicationInfoWithAppIndex(
        const std::string& appName, int32_t appIndex, int32_t userId, ApplicationInfo& appInfo)
    {
        return false;
    }

    bool QueryExtensionAbilityInfoByUri(
        const std::string& uri, int32_t userId, ExtensionAbilityInfo& extensionAbilityInfo)
    {
        return false;
    }

    bool QueryExtensionAbilityInfoByUriOptimal(
        const std::string& uri, int32_t userId, ExtensionAbilityInfo& extensionAbilityInfo)
    {
        return false;
    }

    bool ImplicitQueryInfoByPriority(const AAFwk::Want& want, int32_t flags, int32_t userId, AbilityInfo& abilityInfo,
        ExtensionAbilityInfo& extensionInfo)
    {
        return false;
    }

    bool QueryAbilityInfoByUri(const std::string& abilityUri, int32_t userId, AbilityInfo& abilityInfo)
    {
        return false;
    }

    bool QueryAbilityInfo(const AAFwk::Want& want, int32_t flags, int32_t userId, AbilityInfo& abilityInfo,
        const sptr<IRemoteObject>& callBack)
    {
        return false;
    }

    void UpgradeAtomicService(const AAFwk::Want& want, int32_t userId) {}

    bool ImplicitQueryInfos(const AAFwk::Want& want, int32_t flags, int32_t userId, bool withDefault,
        std::vector<AbilityInfo>& abilityInfos, std::vector<ExtensionAbilityInfo>& extensionInfos, bool& findDefaultApp)
    {
        return false;
    }

    bool CleanBundleDataFiles(
        const std::string& bundleName, int32_t userId, int32_t appCloneIndex, int32_t callerUid)
    {
        return false;
    }

    bool GetHapModuleInfo(const AbilityInfo& abilityInfo, int32_t userId, HapModuleInfo& hapModuleInfo)
    {
        return false;
    }

    bool QueryAppGalleryBundleName(std::string& bundleName)
    {
        return false;
    }

    ErrCode GetUidByBundleName(const std::string& bundleName, int32_t userId, int32_t appCloneIndex)
    {
        return -1;
    }

    ErrCode QueryExtensionAbilityInfosOnlyWithTypeName(const std::string& extensionTypeName, const uint32_t flag,
        const int32_t userId, std::vector<ExtensionAbilityInfo>& extensionInfos)
    {
        return -1;
    }

    ErrCode GetJsonProfile(ProfileType profileType, const std::string& bundleName, const std::string& moduleName,
        std::string& profile, int32_t userId)
    {
        return -1;
    }

    ErrCode GetLaunchWantForBundle(const std::string& bundleName, AAFwk::Want& want, int32_t userId)
    {
        return -1;
    }

    ErrCode QueryCloneAbilityInfo(
        const ElementName& element, int32_t flags, int32_t appCloneIndex, AbilityInfo& abilityInfo, int32_t userId)
    {
        return -1;
    }

    ErrCode GetCloneBundleInfo(
        const std::string& bundleName, int32_t flags, int32_t appCloneIndex, BundleInfo& bundleInfo, int32_t userId)
    {
        return -1;
    }

    ErrCode QueryCloneExtensionAbilityInfoWithAppIndex(const ElementName& element, int32_t flags, int32_t appCloneIndex,
        ExtensionAbilityInfo& extensionInfo, int32_t userId)
    {
        return -1;
    }

    ErrCode GetCloneAppIndexes(const std::string& bundleName, std::vector<int32_t>& appIndexes, int32_t userId)
    {
        return -1;
    }

    ErrCode GetSignatureInfoByBundleName(const std::string& bundleName, SignatureInfo& signatureInfo)
    {
        return -1;
    }

    std::string GetStringById(
        const std::string& bundleName, const std::string& moduleName, uint32_t resId, int32_t userId)
    {
        return "";
    }

    std::string GetDataDir(const std::string& bundleName, const int32_t appIndex)
    {
        return "";
    }

public:
    static bool getNameAndIndexForUid_;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H