/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "mock_bundle_manager.h"

#include "ability_info.h"
#include "application_info.h"
#include "hilog_tag_wrapper.h"
#include "mock_overlay_manager.h"
namespace {
const int32_t HQF_VERSION_CODE = 1000;
}

namespace OHOS {
namespace AppExecFwk {
namespace {
std::string g_CurrentBundleName;
int32_t appUid = 2100;
int32_t appUid1 = 2101;
}

void MockSetCurrentBundleName(const std::string& bundleName)
{
    g_CurrentBundleName = bundleName;
}

namespace {
void ConstructHqfInfo(BundleInfo& bundleInfo)
{
    std::vector<HqfInfo> hqfInfos;
    bundleInfo.applicationInfo.appQuickFix.deployedAppqfInfo.versionCode = HQF_VERSION_CODE;
    bundleInfo.applicationInfo.appQuickFix.deployedAppqfInfo.versionName = "1.0.0";
    bundleInfo.applicationInfo.appQuickFix.deployedAppqfInfo.cpuAbi = "armeabi-v7a";
    bundleInfo.applicationInfo.appQuickFix.deployedAppqfInfo.nativeLibraryPath = "patch_1000/libs/arm";
    bundleInfo.applicationInfo.appQuickFix.deployedAppqfInfo.type = QuickFixType::PATCH;
    HqfInfo hqfInfo1;
    hqfInfo1.moduleName = "entry1";
    hqfInfo1.hapSha256 = "12345678";
    hqfInfo1.hqfFilePath = "/data/app/el1/bundle/public/com.ohos.quickfix/patch_1000/entry1.hqf";
    hqfInfo1.nativeLibraryPath = "patch_1000/libs/arm";
    HqfInfo hqfInfo2;
    hqfInfo2.moduleName = "entry2";
    hqfInfo2.hapSha256 = "12345678";
    hqfInfo2.hqfFilePath = "/data/app/el1/bundle/public/com.ohos.quickfix/patch_1000/entry2.hqf";
    hqfInfo2.nativeLibraryPath = "patch_1000/libs/arm";
    hqfInfos.push_back(hqfInfo1);
    hqfInfos.push_back(hqfInfo2);
    bundleInfo.applicationInfo.appQuickFix.deployedAppqfInfo.hqfInfos = hqfInfos;

    std::vector<HapModuleInfo> hapModuleInfos;
    HapModuleInfo moduleInfo1;
    moduleInfo1.name = "entry1";
    moduleInfo1.moduleName = "entry1";
    moduleInfo1.moduleType = AppExecFwk::ModuleType::ENTRY;
    moduleInfo1.hapPath = "/data/app/el1/bundle/public/com.ohos.hotreload/entry1";
    moduleInfo1.hqfInfo = hqfInfo1;
    moduleInfo1.process = "test_quickfix";
    moduleInfo1.isModuleJson = true;
    moduleInfo1.isStageBasedModel = true;
    moduleInfo1.isLibIsolated = true;
    HapModuleInfo moduleInfo2;
    moduleInfo2.name = "entry2";
    moduleInfo2.moduleName = "entry2";
    moduleInfo2.hapPath = "/data/app/el1/bundle/public/com.ohos.hotreload/entry2";
    moduleInfo2.hqfInfo = hqfInfo2;
    moduleInfo2.process = "test_quickfix";
    moduleInfo2.isModuleJson = true;
    moduleInfo2.isStageBasedModel = true;
    moduleInfo2.isLibIsolated = true;
    hapModuleInfos.push_back(moduleInfo1);
    hapModuleInfos.push_back(moduleInfo2);
    bundleInfo.hapModuleInfos = hapModuleInfos;
    bundleInfo.versionCode = HQF_VERSION_CODE;
    bundleInfo.versionName = "1.0.0";
    bundleInfo.applicationInfo.nativeLibraryPath = "libs/arm";
}
} // namespace

bool BundleMgrProxy::QueryAbilityInfo(const AAFwk::Want& want, AbilityInfo& abilityInfo)
{
    ElementName eleName = want.GetElement();
    if (eleName.GetBundleName().empty()) {
        return false;
    }
    abilityInfo.visible = true;
    abilityInfo.name = eleName.GetAbilityName();
    abilityInfo.bundleName = eleName.GetBundleName();
    abilityInfo.applicationName = "Helloworld";
    return true;
}

bool BundleMgrProxy::QueryAbilityInfoByUri(const std::string& uri, AbilityInfo& abilityInfo)
{
    return false;
}

bool BundleMgrProxy::GetApplicationInfo(
    const std::string& appName, const ApplicationFlag flag, const int userId, ApplicationInfo& appInfo)
{
    if (appName.empty()) {
        return false;
    }
    appInfo.name = "Helloworld";
    appInfo.bundleName = "com.ohos.hiworld";
    return true;
}

std::string BundleMgrProxy::GetAppType(const std::string& bundleName)
{
    return "system";
}

sptr<IOverlayManager> BundleMgrProxy::GetOverlayManagerProxy()
{
    sptr<IOverlayManager> overlayModuleProxy = new (std::nothrow) OverlayManagerProxy(nullptr);
    return overlayModuleProxy;
}

int BundleMgrStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    return 0;
}

BundleMgrService::BundleMgrService()
{
    MakingPackageData();
}

bool BundleMgrService::QueryAbilityInfo(const AAFwk::Want& want, AbilityInfo& abilityInfo)
{
    ElementName elementName = want.GetElement();
    if (elementName.GetBundleName().empty()) {
        return false;
    }
    if (std::string::npos != elementName.GetBundleName().find("service")) {
        abilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    }
    abilityInfo.visible = true;
    abilityInfo.name = elementName.GetAbilityName();
    abilityInfo.bundleName = elementName.GetBundleName();
    abilityInfo.applicationName = elementName.GetBundleName();
    if (want.HasEntity(Want::ENTITY_HOME) && want.GetAction() == Want::ACTION_HOME) {
        abilityInfo.applicationInfo.isLauncherApp = true;
    } else {
        abilityInfo.applicationInfo.isLauncherApp = false;
    }
    return true;
}

bool BundleMgrService::QueryAbilityInfoByUri(const std::string& uri, AbilityInfo& abilityInfo)
{
    return false;
}

bool BundleMgrService::GetApplicationInfo(
    const std::string& appName, const ApplicationFlag flag, const int userId, ApplicationInfo& appInfo)
{
    if (appName.empty()) {
        return false;
    }
    appInfo.name = "Helloworld";
    appInfo.bundleName = "com.ohos.hiworld";
    return true;
}

std::string BundleMgrService::GetAppType(const std::string& bundleName)
{
    return "system";
}

bool BundleMgrService::GetHapModuleInfo(const AbilityInfo& abilityInfo, HapModuleInfo& hapModuleInfo)
{
    hapModuleInfo.name = "Captain";
    return true;
}

bool BundleMgrService::GetHapModuleInfo(const AbilityInfo& abilityInfo, int32_t userId, HapModuleInfo& hapModuleInfo)
{
    hapModuleInfo.name = "Captain";
    return true;
}

bool BundleMgrProxy::GetHapModuleInfo(const AbilityInfo& abilityInfo, HapModuleInfo& hapModuleInfo)
{
    hapModuleInfo.name = "Captain";
    return true;
}

bool BundleMgrProxy::GetHapModuleInfo(const AbilityInfo& abilityInfo, int32_t userId, HapModuleInfo& hapModuleInfo)
{
    hapModuleInfo.name = "Captain";
    return true;
}

bool BundleMgrService::GetBundleInfo(
    const std::string& bundleName, const BundleFlag flag, BundleInfo& bundleInfo, int32_t userId)
{
    int32_t userUid = 10001;
    int32_t userGid = 10001;
    bundleInfo.uid = userUid;
    bundleInfo.gid = userGid;
    int index1 = 101;
    int index2 = 102;
    int index3 = 103;
    int index4 = 104;
    if (bundleName == COM_OHOS_HELLO + std::to_string(index1) ||
        bundleName == COM_OHOS_HELLO + std::to_string(index2)) {
        bundleInfo.appId = bundleName + "_xxx";
    }
    if (bundleName == COM_OHOS_HELLO + std::to_string(index3) ||
        bundleName == COM_OHOS_HELLO + std::to_string(index4)) {
        bundleInfo.appId = bundleName + "_xxx";
    }
    if (bundleName == "KeepAliveApplication") {
        HapModuleInfo hapModuleInfo;
        hapModuleInfo.moduleName = "KeepAliveApplication";
        ApplicationInfo appInfo;
        appInfo.name = "KeepAliveApp";
        appInfo.bundleName = bundleName;
        appInfo.uid = appUid;
        bundleInfo.uid = appUid;
        bundleInfo.name = bundleName;
        bundleInfo.applicationInfo = appInfo;
        bundleInfo.hapModuleInfos.push_back(hapModuleInfo);
    }
    if (bundleName == "KeepAliveApplication1") {
        HapModuleInfo hapModuleInfo;
        hapModuleInfo.moduleName = "KeepAliveApplication1";
        ApplicationInfo appInfo;
        appInfo.name = "KeepAliveApp1";
        appInfo.bundleName = bundleName;
        appInfo.uid = appUid1;
        bundleInfo.uid = appUid1;
        bundleInfo.name = bundleName;
        bundleInfo.applicationInfo = appInfo;
        bundleInfo.hapModuleInfos.push_back(hapModuleInfo);
    }
    if (bundleName == "com.ohos.quickfix") {
        TAG_LOGI(AAFwkTag::TEST, "GetBundleInfo of [com.ohos.quickfix].");
        return false;
    }
    return true;
}
bool BundleMgrService::GetBundleGids(const std::string& bundleName, std::vector<int>& gids)
{
    int32_t userGid1 = 10001;
    int32_t userGid2 = 10002;
    int32_t userGid3 = 10003;
    gids.push_back(userGid1);
    gids.push_back(userGid2);
    gids.push_back(userGid3);
    return true;
}

bool BundleMgrService::GetBundleInfos(
    const BundleFlag flag, std::vector<BundleInfo>& bundleInfos, int32_t userId)
{
    bundleInfos = bundleInfos_;
    return true;
}

void BundleMgrService::PushTestHelloIndexAbility(int index)
{
    AbilityInfo info;
    info.name = "com.ohos.test.helloworld.MainAbility";
    info.bundleName = COM_OHOS_HELLO + std::to_string(index);
    info.applicationInfo.bundleName = COM_OHOS_HELLO + std::to_string(index);
    info.applicationName = "helloworld";
    info.applicationInfo.name = "helloworld";
    info.process = "p1";
    info.applicationInfo.uid = -1;
    info.deviceId = "deviceId";
    info.visible = true;

    BundleInfo bundleInfo;
    bundleInfo.name = COM_OHOS_HELLO + std::to_string(index);
    bundleInfo.uid = info.applicationInfo.uid;
    bundleInfo.abilityInfos.emplace_back(info);
    bundleInfo.applicationInfo = info.applicationInfo;
    bundleInfos_.emplace_back(bundleInfo);
}

void BundleMgrService::PushTestSpecialAbility()
{
    AbilityInfo info;
    info.name = "com.ohos.test.helloworld.MainAbility";
    info.bundleName = COM_OHOS_SPECIAL;
    info.applicationInfo.bundleName = COM_OHOS_SPECIAL;
    info.applicationName = "helloworld";
    info.applicationInfo.name = "helloworld";
    info.process = "p1";
    info.applicationInfo.uid = -1;
    info.deviceId = "deviceId";
    info.visible = true;

    BundleInfo bundleInfo;
    bundleInfo.name = COM_OHOS_SPECIAL;
    bundleInfo.uid = info.applicationInfo.uid;
    bundleInfo.abilityInfos.emplace_back(info);
    bundleInfo.applicationInfo = info.applicationInfo;
    bundleInfos_.emplace_back(bundleInfo);
}

void BundleMgrService::PushTestHelloAbility()
{
    AbilityInfo info;
    info.name = "com.ohos.test.helloworld.MainAbility";
    info.bundleName = COM_OHOS_HELLO;
    info.applicationInfo.bundleName = COM_OHOS_HELLO;
    info.applicationName = "helloworld";
    info.applicationInfo.name = "helloworld";
    info.process = "p1";
    info.applicationInfo.uid = -1;
    info.deviceId = "deviceId";
    info.visible = true;

    BundleInfo bundleInfo;
    bundleInfo.name = COM_OHOS_HELLO;
    bundleInfo.uid = info.applicationInfo.uid;
    bundleInfo.abilityInfos.emplace_back(info);
    bundleInfo.applicationInfo = info.applicationInfo;
    bundleInfos_.emplace_back(bundleInfo);
}

void BundleMgrService::MakingPackageData()
{
    PushTestSpecialAbility();
    PushTestHelloAbility();
    MakingResidentProcData();
    for (int i = 0; i <= APPLICATION_NUMHELLO; i++) {
        PushTestHelloIndexAbility(i);
    }
}

void BundleMgrService::MakingResidentProcData()
{
    int appUid = 2100;
    int appUid1 = 2101;
    BundleInfo bundleInfo;
    bundleInfo.uid = appUid;
    bundleInfo.name = "KeepAliveApplication";

    BundleInfo bundleInfo1;
    bundleInfo1.uid = appUid1;
    bundleInfo1.name = "KeepAliveApplication1";

    bundleInfos_.emplace_back(bundleInfo);
    bundleInfos_.emplace_back(bundleInfo1);
}

sptr<IQuickFixManager> BundleMgrService::GetQuickFixManagerProxy()
{
    if (quickFixManager_ == nullptr) {
        quickFixManager_ = new (std::nothrow) QuickFixManagerHostImpl();
        if (quickFixManager_ == nullptr) {
            GTEST_LOG_(ERROR) << "new quick fix manager failed.";
            return nullptr;
        }
    }
    return quickFixManager_;
}

ErrCode BundleMgrService::GetBundleInfoForSelf(int32_t flags, BundleInfo &bundleInfo)
{
    HapModuleInfo hapModuleInfo;
    bundleInfo.hapModuleInfos.push_back(hapModuleInfo);
    if (g_CurrentBundleName == "com.ohos.quickfix") {
        TAG_LOGI(AAFwkTag::TEST, "GetBundleInfo of [com.ohos.quickfix].");
        ConstructHqfInfo(bundleInfo);
    }
    return ERR_OK;
}

sptr<IOverlayManager> BundleMgrService::GetOverlayManagerProxy()
{
    sptr<IOverlayManager> overlayManagerProxy = new (std::nothrow) OverlayManagerProxy(nullptr);
    return overlayManagerProxy;
}
}  // namespace AppExecFwk
}  // namespace OHOS
