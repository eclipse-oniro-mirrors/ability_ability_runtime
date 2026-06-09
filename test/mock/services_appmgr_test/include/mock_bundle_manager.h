/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MANAGER_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MANAGER_H

#include <vector>
#include "gmock/gmock.h"
#include "ability_info.h"
#include "application_info.h"
#include "want.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "bundle_mgr_interface.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string COM_OHOS_HELLO = "com.ohos.test.helloworld";
const int32_t APPLICATION_NUMHELLO = 104;
const std::string COM_OHOS_SPECIAL = "com.ohos.test.special";
}  // namespace
class BundleMgrProxy : public IRemoteProxy<IBundleMgr> {
public:
    explicit BundleMgrProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<IBundleMgr>(impl)
    {}
    virtual ~BundleMgrProxy()
    {}
    bool QueryAbilityInfo(const AAFwk::Want& want, AbilityInfo& abilityInfo) override;
    bool QueryAbilityInfoByUri(const std::string& uri, AbilityInfo& abilityInfo) override;
    std::string GetAppType(const std::string& bundleName) override;

    virtual bool GetApplicationInfo(
        const std::string& appName, const ApplicationFlag flag, const int userId, ApplicationInfo& appInfo) override;
    virtual bool GetHapModuleInfo(const AbilityInfo& abilityInfo, HapModuleInfo& hapModuleInfo);
    virtual bool GetHapModuleInfo(
        const AbilityInfo& abilityInfo, int32_t userId, HapModuleInfo& hapModuleInfo) override;
    virtual bool QueryKeepAliveBundleInfos(std::vector<BundleInfo>& bundleInfos) override
    {
        int appUid = 2100;
        GTEST_LOG_(INFO) << "QueryKeepAliveBundleInfos()";
        ApplicationInfo info;
        info.name = "KeepAliveApp";
        info.bundleName = "KeepAliveApplication";
        info.uid = appUid;

        BundleInfo bundleInfo;
        bundleInfo.applicationInfo = info;
        HapModuleInfo hapModuleInfo;
        hapModuleInfo.name = "Module";
        HapModuleInfo hapModuleInfo1;
        hapModuleInfo1.name = "Module1";
        bundleInfo.hapModuleInfos.push_back(hapModuleInfo);
        bundleInfo.hapModuleInfos.push_back(hapModuleInfo1);

        bundleInfos.push_back(bundleInfo);
        GTEST_LOG_(INFO) << "bundleInfos size : " << bundleInfos.size();
        return true;
    };

    ErrCode GetBaseSharedBundleInfos(const std::string &bundleName,
        std::vector<BaseSharedBundleInfo> &baseSharedBundleInfos,
        AppExecFwk::GetDependentBundleInfoFlag flag) override
    {
        return ERR_OK;
    }

    sptr<IOverlayManager> GetOverlayManagerProxy() override;
};

class BundleMgrStub : public IRemoteStub<IBundleMgr> {
public:
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
};

class BundleMgrService : public BundleMgrStub {
public:
    MOCK_METHOD0(GetBundleInstaller, sptr<AppExecFwk::IBundleInstaller>());
    bool QueryAbilityInfo(const AAFwk::Want& want, AbilityInfo& abilityInfo) override;
    bool QueryAbilityInfoByUri(const std::string& uri, AbilityInfo& abilityInfo) override;

    std::string GetAppType(const std::string& bundleName) override;

    virtual bool GetApplicationInfo(
        const std::string& appName, const ApplicationFlag flag, const int userId, ApplicationInfo& appInfo) override;
    virtual bool GetBundleInfo(
        const std::string& bundleName, const BundleFlag flag, BundleInfo& bundleInfo, int32_t userId) override;
    virtual bool GetBundleInfos(
        const BundleFlag flag, std::vector<BundleInfo>& bundleInfos, int32_t userId) override;
    virtual bool GetBundleGids(const std::string& bundleName, std::vector<int>& gids) override;
    virtual bool GetHapModuleInfo(const AbilityInfo& abilityInfo, HapModuleInfo& hapModuleInfo);
    virtual bool GetHapModuleInfo(
        const AbilityInfo& abilityInfo, int32_t userId, HapModuleInfo& hapModuleInfo) override;
    virtual bool QueryKeepAliveBundleInfos(std::vector<BundleInfo>& bundleInfos) override
    {
        int appUid = 2100;
        GTEST_LOG_(INFO) << "QueryKeepAliveBundleInfos()";
        ApplicationInfo info;
        info.name = "KeepAliveApp";
        info.bundleName = "KeepAliveApplication";
        info.uid = appUid;

        BundleInfo bundleInfo;
        bundleInfo.applicationInfo = info;
        HapModuleInfo hapModuleInfo;
        hapModuleInfo.name = "Module";
        HapModuleInfo hapModuleInfo1;
        hapModuleInfo1.name = "Module1";
        bundleInfo.hapModuleInfos.push_back(hapModuleInfo);
        bundleInfo.hapModuleInfos.push_back(hapModuleInfo1);

        bundleInfos.push_back(bundleInfo);
        GTEST_LOG_(INFO) << "bundleInfos size : " << bundleInfos.size();
        return true;
    };
    virtual bool ImplicitQueryInfoByPriority(const Want& want, int32_t flags, int32_t userId,
        AbilityInfo& abilityInfo, ExtensionAbilityInfo& extensionInfo) override
    {
        abilityInfo.name = "MainAbility";
        abilityInfo.bundleName = "com.ohos.launcher";
        return true;
    }
    virtual bool GetGroupDir(const std::string &dataGroupId, std::string &dir) override
    {
        dir = "group";
        return true;
    }

    sptr<IQuickFixManager> GetQuickFixManagerProxy() override;

    BundleMgrService();
    virtual ~BundleMgrService() {}
    void MakingPackageData();
    void PushTestHelloIndexAbility(int index);
    void PushTestSpecialAbility();
    void PushTestHelloAbility();
    void MakingResidentProcData();
    ErrCode GetBundleInfoForSelf(int32_t flags, BundleInfo &bundleInfo);
    ErrCode GetBaseSharedBundleInfos(const std::string &bundleName,
        std::vector<BaseSharedBundleInfo> &baseSharedBundleInfos,
        AppExecFwk::GetDependentBundleInfoFlag flag) override
    {
        return ERR_OK;
    }
    sptr<IOverlayManager> GetOverlayManagerProxy() override;
private:
    std::vector<BundleInfo> bundleInfos_;
    sptr<IQuickFixManager> quickFixManager_ = nullptr;
};

class QuickFixManagerHost : public IRemoteStub<IQuickFixManager> {
public:
    int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
};

class QuickFixManagerHostImpl : public QuickFixManagerHost {
public:
    MOCK_METHOD6(DeployQuickFix, ErrCode(const std::vector<std::string>& bundleFilePaths,
        const sptr<IQuickFixStatusCallback>& statusCallback, bool isDebug, const std::string& targetPath,
        bool isReplace, bool isCheckDebugApp));
    MOCK_METHOD3(SwitchQuickFix, ErrCode(const std::string& bundleName, bool enable,
        const sptr<IQuickFixStatusCallback>& statusCallback));
    MOCK_METHOD2(DeleteQuickFix, ErrCode(const std::string& bundleName,
        const sptr<IQuickFixStatusCallback>& statusCallback));
    MOCK_METHOD3(CreateFd, ErrCode(const std::string& fileName, int32_t& fd, std::string& path));

    virtual ErrCode CopyFiles(const std::vector<std::string>& sourceFiles, std::vector<std::string>& destFiles) override
    {
        destFiles = sourceFiles;
        return 0;
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MANAGER_H
