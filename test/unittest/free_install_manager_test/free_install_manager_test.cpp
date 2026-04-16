/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#define protected public
#define private public
#include "ability_manager_service.h"
#include "atomic_service_status_callback_interface.h"
#include "bundle_mgr_helper.h"
#include "permission_constants.h"
#include "support_system_ability_permission.h"
#include "task_handler_wrap.h"
#include "utils/ability_util.h"
#include "utils/app_mgr_util.h"
#undef private
#undef protected

#include "ability_record.h"
#include "mock_app_mgr_service.h"
#include "mock_bundle_manager_service.h"
#include "sa_mgr_client.h"
#include "mock_my_flag.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int VALID_RECORD_ID = 100;
const int INVALID_RECORD_ID = -1;
constexpr const char* KEY_REQUEST_ID = "com.ohos.param.requestId";
constexpr const char* LOCAL_BUNDLE_NAME = "com.test.demo";

class FreeInstallBundleMgrService : public OHOS::MockBundleManagerService {
public:
    explicit FreeInstallBundleMgrService(const std::weak_ptr<OHOS::AAFwk::FreeInstallManager> &freeInstallManager,
        const std::string &localBundleName = "com.test.demo")
        : freeInstallManager_(freeInstallManager), localBundleName_(localBundleName)
    {}

    ErrCode GetNameForUid(const int, std::string &bundleName) override
    {
        bundleName = localBundleName_;
        return ERR_OK;
    }

    bool QueryAbilityInfo(const OHOS::AAFwk::Want &, int32_t, int32_t, AbilityInfo &) override
    {
        return false;
    }

    bool QueryExtensionAbilityInfos(const OHOS::AAFwk::Want &, const int32_t &, const int32_t &,
        std::vector<ExtensionAbilityInfo> &) override
    {
        return false;
    }

    bool QueryAbilityInfo(const OHOS::AAFwk::Want &want, int32_t, int32_t, AbilityInfo &,
        const sptr<IRemoteObject> &) override
    {
        auto freeInstallManager = freeInstallManager_.lock();
        if (freeInstallManager != nullptr) {
            OHOS::AAFwk::FreeInstallInfo taskInfo;
            if (freeInstallManager->GetFreeInstallTaskInfo(want.GetElement().GetBundleName(),
                want.GetElement().GetAbilityName(), want.GetStringParam(Want::PARAM_RESV_START_TIME), taskInfo)) {
                freeInstallManager->NotifyFreeInstallResult(-1, taskInfo.want, ERR_OK, false);
            } else {
                freeInstallManager->NotifyFreeInstallResult(-1, want, ERR_OK, false);
            }
        }
        return false;
    }

private:
    std::weak_ptr<OHOS::AAFwk::FreeInstallManager> freeInstallManager_;
    std::string localBundleName_;
};

class FreeInstallAppMgrService : public MockAppMgrService {
public:
    bool GetAppRunningStateByBundleName(const std::string &) override
    {
        return false;
    }
};
}
class FreeInstallTest : public testing::Test {
public:
    FreeInstallTest()
    {}
    ~FreeInstallTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<Token> MockToken();
    std::shared_ptr<FreeInstallManager> freeInstallManager_ = nullptr;
};

void FreeInstallTest::SetUpTestCase(void) {}

void FreeInstallTest::TearDownTestCase(void) {}

void FreeInstallTest::SetUp(void) {}

void FreeInstallTest::TearDown(void)
{}

sptr<Token> FreeInstallTest::MockToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

/**
 * @tc.number: FreeInstall_StartFreeInstall_001
 * @tc.name: StartFreeInstall
 * @tc.desc: Test StartFreeInstall when callback is success.
 */
HWTEST_F(FreeInstallTest, FreeInstall_StartFreeInstall_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 100;
    const int requestCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));
    // mock callerToken
    const sptr<IRemoteObject> callerToken = MockToken();
    int res = 0;

    usleep(100000); // 100000 means us.
    // from freeInstallManager_->freeInstallList_ find startInstallTime
    for (auto it = freeInstallManager_->freeInstallList_.begin(); it != freeInstallManager_->freeInstallList_.end();) {
        std::string bundleName = (*it).want.GetElement().GetBundleName();
        std::string abilityName = (*it).want.GetElement().GetAbilityName();
        if (want.GetElement().GetBundleName().compare(bundleName) != 0 ||
            want.GetElement().GetAbilityName().compare(abilityName) != 0) {
            want.SetParam(Want::PARAM_RESV_START_TIME, (*it).want.GetStringParam(Want::PARAM_RESV_START_TIME));
            break;
        }
    }
    freeInstallManager_->OnInstallFinished(-1, 0, want, userId, false);

    EXPECT_TRUE(freeInstallManager_ != nullptr);
}

/**
 * @tc.number: FreeInstall_StartFreeInstall_002
 * @tc.name: StartFreeInstall
 * @tc.desc: Test StartFreeInstall when token is nullptr.
 */
HWTEST_F(FreeInstallTest, FreeInstall_StartFreeInstall_002, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 100; // 100 means userId
    const int requestCode = 0;
    // token is nullptr, IsTopAbility failed
    const sptr<IRemoteObject> callerToken = nullptr;
    // NotTopAbility
    freeInstallManager_->StartFreeInstall(want, userId, requestCode, callerToken);
    EXPECT_TRUE(freeInstallManager_ != nullptr);
}

/**
 * @tc.number: FreeInstall_StartFreeInstall_003
 * @tc.name: StartFreeInstall
 * @tc.desc: Test StartFreeInstall when callback is failed.
 */
HWTEST_F(FreeInstallTest, FreeInstall_StartFreeInstall_003, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 1;
    const int requestCode = 0;
    // mock callerToken
    const sptr<IRemoteObject> callerToken = MockToken();
    int res = 0;

    usleep(100000); // 100000 means us.
    // from freeInstallManager_->freeInstallList_ find startInstallTime
    for (auto it = freeInstallManager_->freeInstallList_.begin(); it != freeInstallManager_->freeInstallList_.end();) {
        std::string bundleName = (*it).want.GetElement().GetBundleName();
        std::string abilityName = (*it).want.GetElement().GetAbilityName();
        if (want.GetElement().GetBundleName().compare(bundleName) != 0 ||
            want.GetElement().GetAbilityName().compare(abilityName) != 0) {
            want.SetParam(Want::PARAM_RESV_START_TIME, (*it).want.GetStringParam(Want::PARAM_RESV_START_TIME));
            break;
        }
    }
    freeInstallManager_->OnInstallFinished(-1, 1, want, userId, false);

    EXPECT_TRUE(freeInstallManager_ != nullptr);
}

/**
 * @tc.number: FreeInstall_StartFreeInstall_004
 * @tc.name: StartFreeInstall
 * @tc.desc: Test StartFreeInstall when permission check is skipped.
 */
HWTEST_F(FreeInstallTest, FreeInstall_StartFreeInstall_004, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    ASSERT_NE(bundleMgrHelper, nullptr);
    auto bundleMgrBackup = bundleMgrHelper->bundleMgr_;
    auto appMgrBackup = AppMgrUtil::appMgr_;
    sptr<FreeInstallBundleMgrService> mockBundleMgr =
        new (std::nothrow) FreeInstallBundleMgrService(freeInstallManager_);
    sptr<FreeInstallAppMgrService> mockAppMgr = new (std::nothrow) FreeInstallAppMgrService();
    ASSERT_NE(mockBundleMgr, nullptr);
    ASSERT_NE(mockAppMgr, nullptr);
    bundleMgrHelper->bundleMgr_ = mockBundleMgr;
    AppMgrUtil::appMgr_ = mockAppMgr;

    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));
    const int32_t userId = 100;
    const int requestCode = 0;
    auto param = std::make_shared<FreeInstallParams>();
    param->skipStartFreeInstallPermissionCheck = true;
    int result = freeInstallManager_->StartFreeInstall(want, userId, requestCode, nullptr, param);
    bundleMgrHelper->bundleMgr_ = bundleMgrBackup;
    AppMgrUtil::appMgr_ = appMgrBackup;
    EXPECT_NE(result, NOT_TOP_ABILITY);
    EXPECT_TRUE(freeInstallManager_->freeInstallList_.empty());
}

/**
 * @tc.number: FreeInstall_OnInstallFinished_001
 * @tc.name: OnInstallFinished
 * @tc.desc: Test OnInstallFinished succeed.
 */
HWTEST_F(FreeInstallTest, FreeInstall_OnInstallFinished_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 1;
    const int requestCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));

    FreeInstallInfo info = freeInstallManager_->BuildFreeInstallInfo(want, userId, requestCode, nullptr);
    freeInstallManager_->freeInstallList_.resize(0);
    freeInstallManager_->freeInstallList_.emplace_back(info);
    freeInstallManager_->OnInstallFinished(-1, 0, want, userId, false);

    for (auto it = freeInstallManager_->freeInstallList_.begin();
        it != freeInstallManager_->freeInstallList_.end(); it++) {
        std::string bundleName = (*it).want.GetElement().GetBundleName();
        std::string abilityName = (*it).want.GetElement().GetAbilityName();
        std::string startTime = (*it).want.GetStringParam(Want::PARAM_RESV_START_TIME);
        if (want.GetElement().GetBundleName().compare(bundleName) == 0 &&
            want.GetElement().GetAbilityName().compare(abilityName) == 0 &&
            want.GetStringParam(Want::PARAM_RESV_START_TIME).compare(startTime) == 0) {
            EXPECT_EQ((*it).promise->get_future().get(), 0);
        }
    }
}

/**
 * @tc.number: FreeInstall_OnInstallFinished_002
 * @tc.name: OnInstallFinished
 * @tc.desc: Test OnInstallFinished failed.
 */
HWTEST_F(FreeInstallTest, FreeInstall_OnInstallFinished_002, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 1;
    const int requestCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));

    FreeInstallInfo info = freeInstallManager_->BuildFreeInstallInfo(want, userId, requestCode, nullptr);
    freeInstallManager_->freeInstallList_.resize(0);
    freeInstallManager_->freeInstallList_.emplace_back(info);
    freeInstallManager_->OnInstallFinished(-1, 1, want, userId, false);

    for (auto it = freeInstallManager_->freeInstallList_.begin();
        it != freeInstallManager_->freeInstallList_.end(); it++) {
        std::string bundleName = (*it).want.GetElement().GetBundleName();
        std::string abilityName = (*it).want.GetElement().GetAbilityName();
        std::string startTime = (*it).want.GetStringParam(Want::PARAM_RESV_START_TIME);
        if (want.GetElement().GetBundleName().compare(bundleName) == 0 &&
            want.GetElement().GetAbilityName().compare(abilityName) == 0 &&
            want.GetStringParam(Want::PARAM_RESV_START_TIME).compare(startTime) == 0) {
            EXPECT_EQ((*it).promise->get_future().get(), 1);
        }
    }
}

/**
 * @tc.number: FreeInstall_OnInstallFinished_003
 * @tc.name: OnInstallFinished
 * @tc.desc: Test OnInstallFinished failed.
 */
HWTEST_F(FreeInstallTest, FreeInstall_OnInstallFinished_003, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));
    const int32_t userId = 1;
    const int requestCode = 0;

    FreeInstallInfo info = freeInstallManager_->BuildFreeInstallInfo(want, userId, requestCode, nullptr);
    info.isInstalled = true;
    freeInstallManager_->freeInstallList_.resize(0);
    info.promise.reset();
    freeInstallManager_->freeInstallList_.emplace_back(info);
    freeInstallManager_->OnInstallFinished(-1, 0, want, userId, false);

    int size = freeInstallManager_->freeInstallList_.size();
    EXPECT_EQ(size, 1);
}

/**
 * @tc.number: FreeInstall_FreeInstallAbilityFromRemote_001
 * @tc.name: FreeInstallAbilityFromRemote
 * @tc.desc: Test FreeInstallAbilityFromRemote.
 */
HWTEST_F(FreeInstallTest, FreeInstall_FreeInstallAbilityFromRemote_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 1;
    const int requestCode = 0;

    int res = freeInstallManager_->FreeInstallAbilityFromRemote(want, nullptr, userId, requestCode);
    EXPECT_EQ(res, 22);
}

/**
 * @tc.number: FreeInstall_OnRemoteInstallFinished_001
 * @tc.name: OnRemoteInstallFinished
 * @tc.desc: Test OnRemoteInstallFinished.
 */
HWTEST_F(FreeInstallTest, FreeInstall_OnRemoteInstallFinished_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 1;
    const int requestCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));

    FreeInstallInfo info = freeInstallManager_->BuildFreeInstallInfo(want, userId, requestCode, nullptr);
    freeInstallManager_->freeInstallList_.resize(0);
    freeInstallManager_->freeInstallList_.emplace_back(info);
    freeInstallManager_->OnRemoteInstallFinished(-1, 0, want, userId);

    for (auto it = freeInstallManager_->freeInstallList_.begin();
        it != freeInstallManager_->freeInstallList_.end(); it++) {
        std::string bundleName = (*it).want.GetElement().GetBundleName();
        std::string abilityName = (*it).want.GetElement().GetAbilityName();
        std::string startTime = (*it).want.GetStringParam(Want::PARAM_RESV_START_TIME);
        if (want.GetElement().GetBundleName().compare(bundleName) == 0 &&
            want.GetElement().GetAbilityName().compare(abilityName) == 0 &&
            want.GetStringParam(Want::PARAM_RESV_START_TIME).compare(startTime) == 0) {
            EXPECT_EQ((*it).promise->get_future().get(), 0);
        }
    }
}

/**
 * @tc.number: FreeInstall_ConnectFreeInstall_001
 * @tc.name: ConnectFreeInstall
 * @tc.desc: Test ConnectFreeInstall.
 */
HWTEST_F(FreeInstallTest, FreeInstall_ConnectFreeInstall_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 1;

    int res = freeInstallManager_->ConnectFreeInstall(
        want, userId, nullptr, "", AppExecFwk::ExtensionAbilityType::SERVICE);
    EXPECT_NE(res, 0);
}

/**
 * @tc.number: FreeInstall_ConnectFreeInstall_002
 * @tc.name: ConnectFreeInstall
 * @tc.desc: Test ConnectFreeInstall for agent extension type.
 */
HWTEST_F(FreeInstallTest, FreeInstall_ConnectFreeInstall_002, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    ASSERT_NE(bundleMgrHelper, nullptr);
    auto bundleMgrBackup = bundleMgrHelper->bundleMgr_;
    auto appMgrBackup = AppMgrUtil::appMgr_;
    sptr<FreeInstallBundleMgrService> mockBundleMgr =
        new (std::nothrow) FreeInstallBundleMgrService(freeInstallManager_, LOCAL_BUNDLE_NAME);
    sptr<FreeInstallAppMgrService> mockAppMgr = new (std::nothrow) FreeInstallAppMgrService();
    ASSERT_NE(mockBundleMgr, nullptr);
    ASSERT_NE(mockAppMgr, nullptr);
    bundleMgrHelper->bundleMgr_ = mockBundleMgr;
    AppMgrUtil::appMgr_ = mockAppMgr;

    Want want;
    ElementName element("", LOCAL_BUNDLE_NAME, "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));
    const int32_t userId = 1;
    int serviceRes = freeInstallManager_->ConnectFreeInstall(
        want, userId, nullptr, "", AppExecFwk::ExtensionAbilityType::SERVICE);
    int agentRes = freeInstallManager_->ConnectFreeInstall(
        want, userId, nullptr, "", AppExecFwk::ExtensionAbilityType::AGENT);
    bundleMgrHelper->bundleMgr_ = bundleMgrBackup;
    AppMgrUtil::appMgr_ = appMgrBackup;
    bool callerCanBypassPermission = SupportSystemAbilityPermission::IsSupportSaCallPermission() ||
        AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
            PermissionConstants::PERMISSION_START_ABILITIES_FROM_BACKGROUND);
    if (!callerCanBypassPermission) {
        EXPECT_NE(serviceRes, ERR_OK);
        EXPECT_NE(agentRes, NOT_TOP_ABILITY);
    } else {
        EXPECT_EQ(serviceRes, ERR_OK);
        EXPECT_NE(agentRes, INVALID_PARAMETERS_ERR);
    }
    EXPECT_TRUE(freeInstallManager_->freeInstallList_.empty());
}

/**
 * @tc.number: FreeInstall_AddFreeInstallObserver_001
 * @tc.name: AddFreeInstallObserver
 * @tc.desc: Test AddFreeInstallObserver.
 */
HWTEST_F(FreeInstallTest, FreeInstall_AddFreeInstallObserver_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();

    auto ret = freeInstallManager_->AddFreeInstallObserver(nullptr, nullptr);
    EXPECT_EQ(CHECK_PERMISSION_FAILED, ret);
}

/**
 * @tc.number: FreeInstall_SetSCBCallStatus_001
 * @tc.name: SetSCBCallStatus
 * @tc.desc: Test SetSCBCallStatus.
 */
HWTEST_F(FreeInstallTest, FreeInstall_SetSCBCallStatus_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();

    freeInstallManager_->SetSCBCallStatus("com.ix.hiservcie", "ServiceAbility", "2024-7-17 00:00:00", false);
    FreeInstallInfo freeInstallInfo;
    freeInstallManager_->GetFreeInstallTaskInfo("com.ix.hiservcie", "ServiceAbility",
        "2024-7-17 00:00:00", freeInstallInfo);
    EXPECT_EQ(freeInstallInfo.isStartUIAbilityBySCBCalled, false);
}

/**
 * @tc.number: FreeInstall_SetPreStartMissionCallStatus_001
 * @tc.name: SetPreStartMissionCallStatus
 * @tc.desc: Test SetPreStartMissionCallStatus.
 */
HWTEST_F(FreeInstallTest, FreeInstall_SetPreStartMissionCallStatus_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();

    freeInstallManager_->SetPreStartMissionCallStatus("com.ix.hiservcie", "ServiceAbility",
        "2024-7-17 00:00:00", false);
    FreeInstallInfo freeInstallInfo;
    freeInstallManager_->GetFreeInstallTaskInfo("com.ix.hiservcie", "ServiceAbility",
        "2024-7-17 00:00:00", freeInstallInfo);
    EXPECT_EQ(freeInstallInfo.isPreStartMissionCalled, false);
}

/**
 * @tc.number: FreeInstall_SetFreeInstallTaskSessionId_001
 * @tc.name: SetFreeInstallTaskSessionId
 * @tc.desc: Test SetFreeInstallTaskSessionId.
 */
HWTEST_F(FreeInstallTest, FreeInstall_SetFreeInstallTaskSessionId_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();

    Want want;
    ElementName element("", "com.ix.hiservcie", "ServiceAbility");
    want.SetElement(element);

    std::string startTime = "2024-7-17 00:00:00";
    want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
    const int32_t userId = 1;
    const int requestCode = 0;
    // mock callerToken
    const sptr<IRemoteObject> callerToken = MockToken();
    FreeInstallInfo info = freeInstallManager_->BuildFreeInstallInfo(want, userId, requestCode, callerToken);
    {
        std::lock_guard<ffrt::mutex> lock(freeInstallManager_->freeInstallListLock_);
        freeInstallManager_->freeInstallList_.push_back(info);
    }

    freeInstallManager_->SetFreeInstallTaskSessionId("com.ix.hiservcie", "ServiceAbility",
        "2024-7-17 00:00:00", "sessionId");
    FreeInstallInfo freeInstallInfo;
    bool ret = freeInstallManager_->GetFreeInstallTaskInfo("sessionId", freeInstallInfo);
    EXPECT_EQ(true, ret);

    freeInstallManager_->RemoveFreeInstallInfo("com.ix.hiservcie", "ServiceAbility", "2024-7-17 00:00:00");
    ret = freeInstallManager_->GetFreeInstallTaskInfo("com.ix.hiservcie", "ServiceAbility",
        "2024-7-17 00:00:00", freeInstallInfo);
    EXPECT_EQ(false, ret);
    freeInstallManager_->OnInstallFinished(-1, 1, want, userId, false);
}

/**
 * @tc.number: FreeInstall_VerifyStartFreeInstallPermission_001
 * @tc.name: VerifyStartFreeInstallPermission
 * @tc.desc: Test VerifyStartFreeInstallPermission.
 */
HWTEST_F(FreeInstallTest, FreeInstall_VerifyStartFreeInstallPermission_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();

    const sptr<IRemoteObject> callerToken = MockToken();
    freeInstallManager_->VerifyStartFreeInstallPermission(callerToken);
    freeInstallManager_->GetRecordIdByToken(callerToken);
    EXPECT_TRUE(freeInstallManager_ != nullptr);
}

/**
 * @tc.number: FreeInstall_SetAppRunningState_001
 * @tc.name: SetAppRunningState
 * @tc.desc: Test SetAppRunningState.
 */
HWTEST_F(FreeInstallTest, FreeInstall_SetAppRunningState_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();

    Want want;
    freeInstallManager_->SetAppRunningState(want);
    EXPECT_TRUE(freeInstallManager_ != nullptr);
}

/**
 * @tc.number: FreeInstall_PostUpgradeAtomicServiceTask_001
 * @tc.name: PostUpgradeAtomicServiceTask
 * @tc.desc: Test PostUpgradeAtomicServiceTask.
 */
HWTEST_F(FreeInstallTest, FreeInstall_PostUpgradeAtomicServiceTask_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();

    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 100;
    int resultCode = 0;
    freeInstallManager_->PostUpgradeAtomicServiceTask(resultCode, want, userId);
    EXPECT_TRUE(freeInstallManager_ != nullptr);
}

/**
 * @tc.number: FreeInstall_StartAbilityByOriginalWant_001
 * @tc.name: StartAbilityByOriginalWant
 * @tc.desc: Test StartAbilityByOriginalWant.
 */
HWTEST_F(FreeInstallTest, FreeInstall_StartAbilityByOriginalWant_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();

    FreeInstallInfo freeInstallInfo;
    freeInstallManager_->StartAbilityByOriginalWant(freeInstallInfo, "2024-07-17 00:00:00");
    EXPECT_TRUE(freeInstallManager_ != nullptr);
}

/**
 * @tc.number: FreeInstall_StartAbilityByConvertedWant_001
 * @tc.name: StartAbilityByConvertedWant
 * @tc.desc: Test StartAbilityByConvertedWant.
 */
HWTEST_F(FreeInstallTest, FreeInstall_StartAbilityByConvertedWant_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();

    FreeInstallInfo freeInstallInfo;
    freeInstallManager_->StartAbilityByConvertedWant(freeInstallInfo, "2024-07-17 00:00:00");
    EXPECT_TRUE(freeInstallManager_ != nullptr);
}

/**
 * @tc.number: IsTopAbility_001
 * @tc.name: IsTopAbility
 * @tc.desc: Test IsTopAbility.
 */
HWTEST_F(FreeInstallTest, FreeInstall_IsTopAbility_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    EXPECT_NE(freeInstallManager_, nullptr);
    sptr<IRemoteObject> callerToken = nullptr;
    bool result = freeInstallManager_->IsTopAbility(callerToken);
    EXPECT_FALSE(result);
}

/**
 * @tc.number: IsTopAbility_002
 * @tc.name: IsTopAbility
 * @tc.desc: Test IsTopAbility.
 */
HWTEST_F(FreeInstallTest, FreeInstall_IsTopAbility_002, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    EXPECT_NE(freeInstallManager_, nullptr);
    sptr<IRemoteObject> callerToken = MockToken();
    bool result = freeInstallManager_->IsTopAbility(callerToken);
    EXPECT_FALSE(result);
}

/**
 * @tc.number: IsTopAbility_003
 * @tc.name: IsTopAbility
 * @tc.desc: Test IsTopAbility.
 */
HWTEST_F(FreeInstallTest, FreeInstall_IsTopAbility_003, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    EXPECT_NE(freeInstallManager_, nullptr);
    sptr<IRemoteObject> callerToken = MockToken();
    MyFlag::GetInstance().isScbEnabled_ = true;
    bool result = freeInstallManager_->IsTopAbility(callerToken);
    MyFlag::GetInstance().isScbEnabled_ = false;
    result = freeInstallManager_->IsTopAbility(callerToken);
    EXPECT_FALSE(result);
}

/**
 * @tc.number: FreeInstall_StartRemoteFreeInstall_001
 * @tc.name: StartRemoteFreeInstall
 * @tc.desc: Test StartRemoteFreeInstall.
 */
HWTEST_F(FreeInstallTest, FreeInstall_StartRemoteFreeInstall_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->missionId_ = -1;
    sptr<IRemoteObject> callerToken = nullptr;
    int result = freeInstallManager_->StartRemoteFreeInstall(want, 0, 0, callerToken);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.number: FreeInstall_AddFreeInstallObserver_002
 * @tc.name: AddFreeInstallObserver
 * @tc.desc: Test StartRemoteFreeInstall.
 */
HWTEST_F(FreeInstallTest, FreeInstall_AddFreeInstallObserver_002, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->missionId_ = -1;
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    int result = freeInstallManager_->AddFreeInstallObserver(callerToken, nullptr);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.number: FreeInstall_GetRecordIdByToken_001
 * @tc.name: GetRecordIdByToken
 * @tc.desc: Test StartRemoteFreeInstall.
 */
HWTEST_F(FreeInstallTest, FreeInstall_GetRecordIdByToken_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    EXPECT_NE(freeInstallManager_, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->recordId_ = VALID_RECORD_ID;
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    int32_t result = freeInstallManager_->GetRecordIdByToken(callerToken);
    EXPECT_NE(result, INVALID_RECORD_ID);
}

/**
 * @tc.number: FreeInstall_GetRecordIdByToken_002
 * @tc.name: GetRecordIdByToken
 * @tc.desc: Test StartRemoteFreeInstall.
 */
HWTEST_F(FreeInstallTest, FreeInstall_GetRecordIdByToken_002, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    EXPECT_NE(freeInstallManager_, nullptr);
    sptr<IRemoteObject> callerToken = nullptr;
    int32_t result = freeInstallManager_->GetRecordIdByToken(callerToken);
    EXPECT_EQ(result, INVALID_RECORD_ID);
}

/**
 * @tc.number: FreeInstall_BuildFreeInstallInfo_002
 * @tc.name: BuildFreeInstallInfo
 * @tc.desc: Test BuildFreeInstallInfo.
 */
HWTEST_F(FreeInstallTest, FreeInstall_BuildFreeInstallInfo_002, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 1;
    const int requestCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));
    auto param = std::make_shared<FreeInstallParams>();
    AAFwk::StartOptions startOptions;
    startOptions.requestId_ = "test";
    param->startOptions = std::make_shared<AAFwk::StartOptions>(startOptions);

    FreeInstallInfo info = freeInstallManager_->BuildFreeInstallInfo(want, userId, requestCode, nullptr, param);
    EXPECT_EQ(info.want.GetStringParam(KEY_REQUEST_ID), "test");
}
}  // namespace AppExecFwk
}  // namespace OHOS
