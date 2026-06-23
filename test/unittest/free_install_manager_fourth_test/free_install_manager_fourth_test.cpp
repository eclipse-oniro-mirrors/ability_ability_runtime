/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "bundle_mgr_helper.h"
#include "distributed_client.h"
#include "free_install_manager.h"
#include "insight_intent_execute_manager.h"
#include "mock_app_mgr_service.h"
#include "mock_bundle_manager_service.h"
#include "task_handler_wrap.h"
#include "utils/app_mgr_util.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AAFwk {
int32_t DistributedClient::StartRemoteFreeInstall(const Want &, int32_t, int32_t, uint32_t,
    const sptr<IRemoteObject> &)
{
    return ERR_OK;
}

TaskHandle TaskHandlerWrap::SubmitTask(const std::function<void()> &task, const std::string &)
{
    if (task) {
        task();
    }
    return TaskHandle();
}

InsightIntentExecuteManager::InsightIntentExecuteManager() = default;

InsightIntentExecuteManager::~InsightIntentExecuteManager() = default;

int32_t InsightIntentExecuteManager::ExecuteIntentDone(uint64_t, int32_t,
    const AppExecFwk::InsightIntentExecuteResult &, int32_t, uint32_t)
{
    return ERR_OK;
}
}  // namespace AAFwk

namespace AppExecFwk {
namespace {
constexpr const char *LOCAL_BUNDLE_NAME = "com.test.demo";
constexpr const char *REMOTE_BUNDLE_NAME = "com.remote.demo";
constexpr const char *AGENT_ABILITY_NAME = "AgentAbility";
constexpr int32_t TEST_USER_ID = 100;

class FreeInstallAgentBundleMgrService : public OHOS::MockBundleManagerService {
public:
    explicit FreeInstallAgentBundleMgrService(const std::weak_ptr<FreeInstallManager> &freeInstallManager)
        : freeInstallManager_(freeInstallManager)
    {}

    ErrCode GetNameForUid(const int, std::string &bundleName) override
    {
        bundleName = LOCAL_BUNDLE_NAME;
        return ERR_OK;
    }

    bool QueryAbilityInfo(const Want &, int32_t, int32_t, AbilityInfo &) override
    {
        return queryAbilityResult_;
    }

    bool QueryExtensionAbilityInfos(const Want &, const int32_t &, const int32_t &,
        std::vector<ExtensionAbilityInfo> &extensionInfos) override
    {
        extensionInfos = extensionInfos_;
        return queryExtensionResult_;
    }

    bool QueryAbilityInfo(const Want &want, int32_t, int32_t, AbilityInfo &, const sptr<IRemoteObject> &) override
    {
        queryAbilityInfoWithCallbackCount_++;
        auto freeInstallManager = freeInstallManager_.lock();
        if (freeInstallManager == nullptr) {
            return false;
        }

        FreeInstallInfo taskInfo;
        if (freeInstallManager->GetFreeInstallTaskInfo(want.GetElement().GetBundleName(),
            want.GetElement().GetAbilityName(), want.GetStringParam(Want::PARAM_RESV_START_TIME), taskInfo)) {
            freeInstallManager->NotifyFreeInstallResult(-1, taskInfo.want, ERR_OK, false);
        } else {
            freeInstallManager->NotifyFreeInstallResult(-1, want, ERR_OK, false);
        }
        return false;
    }

    void SetQueryAbilityResult(bool queryAbilityResult)
    {
        queryAbilityResult_ = queryAbilityResult;
    }

    void SetQueryExtensionResult(bool queryExtensionResult,
        const std::vector<ExtensionAbilityInfo> &extensionInfos = {})
    {
        queryExtensionResult_ = queryExtensionResult;
        extensionInfos_ = extensionInfos;
    }

    int32_t GetQueryAbilityInfoWithCallbackCount() const
    {
        return queryAbilityInfoWithCallbackCount_;
    }

private:
    std::weak_ptr<FreeInstallManager> freeInstallManager_;
    bool queryAbilityResult_ = false;
    bool queryExtensionResult_ = false;
    std::vector<ExtensionAbilityInfo> extensionInfos_;
    int32_t queryAbilityInfoWithCallbackCount_ = 0;
};

class FreeInstallAgentAppMgrService : public MockAppMgrService {
public:
    bool GetAppRunningStateByBundleName(const std::string &) override
    {
        return false;
    }
};

class FreeInstallAgentMockScope {
public:
    explicit FreeInstallAgentMockScope(const std::shared_ptr<FreeInstallManager> &freeInstallManager)
    {
        bundleMgrHelper_ = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
        if (bundleMgrHelper_ != nullptr) {
            bundleMgrBackup_ = bundleMgrHelper_->bundleMgr_;
            mockBundleMgr_ = new (std::nothrow) FreeInstallAgentBundleMgrService(freeInstallManager);
            bundleMgrHelper_->bundleMgr_ = mockBundleMgr_;
        }

        appMgrBackup_ = AppMgrUtil::appMgr_;
        mockAppMgr_ = new (std::nothrow) FreeInstallAgentAppMgrService();
        AppMgrUtil::appMgr_ = mockAppMgr_;
    }

    ~FreeInstallAgentMockScope()
    {
        if (bundleMgrHelper_ != nullptr) {
            bundleMgrHelper_->bundleMgr_ = bundleMgrBackup_;
        }
        AppMgrUtil::appMgr_ = appMgrBackup_;
    }

    sptr<FreeInstallAgentBundleMgrService> GetBundleMgr() const
    {
        return mockBundleMgr_;
    }

    sptr<FreeInstallAgentAppMgrService> GetAppMgr() const
    {
        return mockAppMgr_;
    }

private:
    std::shared_ptr<AppExecFwk::BundleMgrHelper> bundleMgrHelper_;
    sptr<IBundleMgr> bundleMgrBackup_;
    sptr<IAppMgr> appMgrBackup_;
    sptr<FreeInstallAgentBundleMgrService> mockBundleMgr_;
    sptr<FreeInstallAgentAppMgrService> mockAppMgr_;
};
}  // namespace

class FreeInstallManagerFourthTest : public testing::Test {
public:
    void SetUp() override
    {
        freeInstallManager_ = std::make_shared<FreeInstallManager>();
    }

    std::shared_ptr<FreeInstallManager> freeInstallManager_;
};

/**
 * @tc.number: FreeInstall_ConnectFreeInstall_003
 * @tc.name: ConnectFreeInstall
 * @tc.desc: Test ConnectFreeInstall returns directly when AGENT extension is already installed.
 */
HWTEST_F(FreeInstallManagerFourthTest, FreeInstall_ConnectFreeInstall_003, TestSize.Level1)
{
    FreeInstallAgentMockScope mockScope(freeInstallManager_);
    ASSERT_NE(mockScope.GetBundleMgr(), nullptr);
    ASSERT_NE(mockScope.GetAppMgr(), nullptr);

    ExtensionAbilityInfo agentExtensionInfo;
    agentExtensionInfo.type = ExtensionAbilityType::AGENT;
    agentExtensionInfo.applicationInfo.bundleType = BundleType::ATOMIC_SERVICE;
    mockScope.GetBundleMgr()->SetQueryExtensionResult(true, { agentExtensionInfo });

    Want want;
    ElementName element("", REMOTE_BUNDLE_NAME, AGENT_ABILITY_NAME);
    want.SetElement(element);

    int agentRes = freeInstallManager_->ConnectFreeInstall(
        want, TEST_USER_ID, nullptr, "", ExtensionAbilityType::AGENT);

    EXPECT_EQ(agentRes, ERR_OK);
    EXPECT_EQ(mockScope.GetBundleMgr()->GetQueryAbilityInfoWithCallbackCount(), 0);
    EXPECT_TRUE(freeInstallManager_->freeInstallList_.empty());
}

/**
 * @tc.number: FreeInstall_ConnectFreeInstall_004
 * @tc.name: ConnectFreeInstall
 * @tc.desc: Test ConnectFreeInstall rejects AGENT connect when an ability target is resolved.
 */
HWTEST_F(FreeInstallManagerFourthTest, FreeInstall_ConnectFreeInstall_004, TestSize.Level1)
{
    FreeInstallAgentMockScope mockScope(freeInstallManager_);
    ASSERT_NE(mockScope.GetBundleMgr(), nullptr);
    ASSERT_NE(mockScope.GetAppMgr(), nullptr);
    mockScope.GetBundleMgr()->SetQueryAbilityResult(true);

    Want want;
    ElementName element("", REMOTE_BUNDLE_NAME, "MainAbility");
    want.SetElement(element);

    int agentRes = freeInstallManager_->ConnectFreeInstall(
        want, TEST_USER_ID, nullptr, "", ExtensionAbilityType::AGENT);

    EXPECT_EQ(agentRes, ERR_WRONG_INTERFACE_CALL);
    EXPECT_EQ(mockScope.GetBundleMgr()->GetQueryAbilityInfoWithCallbackCount(), 0);
    EXPECT_TRUE(freeInstallManager_->freeInstallList_.empty());
}

/**
 * @tc.number: FreeInstall_ConnectFreeInstall_005
 * @tc.name: ConnectFreeInstall
 * @tc.desc: Test ConnectFreeInstall rejects AGENT connect when a non-AGENT extension target is resolved.
 */
HWTEST_F(FreeInstallManagerFourthTest, FreeInstall_ConnectFreeInstall_005, TestSize.Level1)
{
    FreeInstallAgentMockScope mockScope(freeInstallManager_);
    ASSERT_NE(mockScope.GetBundleMgr(), nullptr);
    ASSERT_NE(mockScope.GetAppMgr(), nullptr);

    ExtensionAbilityInfo serviceExtensionInfo;
    serviceExtensionInfo.type = ExtensionAbilityType::SERVICE;
    mockScope.GetBundleMgr()->SetQueryExtensionResult(true, { serviceExtensionInfo });

    Want want;
    ElementName element("", REMOTE_BUNDLE_NAME, "ServiceAbility");
    want.SetElement(element);

    int agentRes = freeInstallManager_->ConnectFreeInstall(
        want, TEST_USER_ID, nullptr, "", ExtensionAbilityType::AGENT);

    EXPECT_EQ(agentRes, ERR_WRONG_INTERFACE_CALL);
    EXPECT_EQ(mockScope.GetBundleMgr()->GetQueryAbilityInfoWithCallbackCount(), 0);
    EXPECT_TRUE(freeInstallManager_->freeInstallList_.empty());
}

/**
 * @tc.number: FreeInstall_ConnectFreeInstall_006
 * @tc.name: ConnectFreeInstall
 * @tc.desc: Test ConnectFreeInstall starts free install for AGENT connect when no target is resolved.
 */
HWTEST_F(FreeInstallManagerFourthTest, FreeInstall_ConnectFreeInstall_006, TestSize.Level1)
{
    FreeInstallAgentMockScope mockScope(freeInstallManager_);
    ASSERT_NE(mockScope.GetBundleMgr(), nullptr);
    ASSERT_NE(mockScope.GetAppMgr(), nullptr);

    Want want;
    ElementName element("", REMOTE_BUNDLE_NAME, AGENT_ABILITY_NAME);
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));

    int agentRes = freeInstallManager_->ConnectFreeInstall(
        want, TEST_USER_ID, nullptr, "", ExtensionAbilityType::AGENT);

    EXPECT_EQ(agentRes, ERR_OK);
    EXPECT_EQ(mockScope.GetBundleMgr()->GetQueryAbilityInfoWithCallbackCount(), 1);
    EXPECT_TRUE(freeInstallManager_->freeInstallList_.empty());
}
}  // namespace AppExecFwk
}  // namespace OHOS
