/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#define private public
#include "ability_manager_service.h"
#include "task_handler_wrap.h"
#undef private

#include "ability_record.h"
#include "sa_mgr_client.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int BUNDLE_MGR_SERVICE_SYS_ABILITY_ID = 401;
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
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 100;
    const int requestCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));
    // mock callerToken
    const sptr<IRemoteObject> callerToken = MockToken();
    int res = 0;

    usleep(100000);
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

    EXPECT_EQ(res, 0);
}

/**
 * @tc.number: FreeInstall_StartFreeInstall_002
 * @tc.name: StartFreeInstall
 * @tc.desc: Test StartFreeInstall when token is nullptr.
 */
HWTEST_F(FreeInstallTest, FreeInstall_StartFreeInstall_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 100;
    const int requestCode = 0;
    // token is nullptr, IsTopAbility failed
    const sptr<IRemoteObject> callerToken = nullptr;
    // NotTopAbility
    int res = freeInstallManager_->StartFreeInstall(want, userId, requestCode, callerToken);
    EXPECT_EQ(res, UNKNOWN_EXCEPTION);
}

/**
 * @tc.number: FreeInstall_StartFreeInstall_003
 * @tc.name: StartFreeInstall
 * @tc.desc: Test StartFreeInstall when callback is failed.
 */
HWTEST_F(FreeInstallTest, FreeInstall_StartFreeInstall_003, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 1;
    const int requestCode = 0;
    // mock callerToken
    const sptr<IRemoteObject> callerToken = MockToken();
    int res = 0;

    usleep(100000);
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

    EXPECT_EQ(res, 0);
}

/**
 * @tc.number: FreeInstall_OnInstallFinished_001
 * @tc.name: OnInstallFinished
 * @tc.desc: Test OnInstallFinished succeed.
 */
HWTEST_F(FreeInstallTest, FreeInstall_OnInstallFinished_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 1;
    const int requestCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));

    FreeInstallInfo info = freeInstallManager_->BuildFreeInstallInfo(want, userId, requestCode, nullptr, false);
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
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 1;
    const int requestCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));

    FreeInstallInfo info = freeInstallManager_->BuildFreeInstallInfo(want, userId, requestCode, nullptr, false);
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
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));
    const int32_t userId = 1;
    const int requestCode = 0;

    FreeInstallInfo info = freeInstallManager_->BuildFreeInstallInfo(want, userId, requestCode, nullptr, false);
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
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);
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
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 1;
    const int requestCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));

    FreeInstallInfo info = freeInstallManager_->BuildFreeInstallInfo(want, userId, requestCode, nullptr, false);
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
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 1;

    int res = freeInstallManager_->ConnectFreeInstall(want, userId, nullptr, "");
    EXPECT_NE(res, 0);
}


/**
 * @tc.number: FreeInstall_UpdateElementName_001
 * @tc.name: UpdateElementName
 * @tc.desc: Test UpdateElementName.
 */
HWTEST_F(FreeInstallTest, FreeInstall_UpdateElementName_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);
    Want want;
    const int32_t userId = 1;
    freeInstallManager_->UpdateElementName(want, userId);
    freeInstallManager_->GetTimeStamp();
}

/**
 * @tc.number: FreeInstall_AddFreeInstallObserver_001
 * @tc.name: AddFreeInstallObserver
 * @tc.desc: Test AddFreeInstallObserver.
 */
HWTEST_F(FreeInstallTest, FreeInstall_AddFreeInstallObserver_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);

    const sptr<IRemoteObject> callerToken = MockToken();
    freeInstallManager_->AddFreeInstallObserver(nullptr, nullptr);

    freeInstallManager_->AddFreeInstallObserver(callerToken, nullptr);
}

/**
 * @tc.number: FreeInstall_RemoveFreeInstallInfo_001
 * @tc.name: RemoveFreeInstallInfo
 * @tc.desc: Test RemoveFreeInstallInfo.
 */
HWTEST_F(FreeInstallTest, FreeInstall_RemoveFreeInstallInfo_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);

    freeInstallManager_->RemoveFreeInstallInfo("com.ix.hiservcie", "ServiceAbility", "2024-7-17 00:00:00");
}

/**
 * @tc.number: FreeInstall_GetFreeInstallTaskInfo_001
 * @tc.name: GetFreeInstallTaskInfo
 * @tc.desc: Test GetFreeInstallTaskInfo.
 */
HWTEST_F(FreeInstallTest, FreeInstall_GetFreeInstallTaskInfo_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);

    FreeInstallInfo  freeInstallInfo;
    freeInstallManager_->GetFreeInstallTaskInfo("sessionId", freeInstallInfo);
}

/**
 * @tc.number: FreeInstall_SetSCBCallStatus_001
 * @tc.name: SetSCBCallStatus
 * @tc.desc: Test SetSCBCallStatus.
 */
HWTEST_F(FreeInstallTest, FreeInstall_SetSCBCallStatus_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);

    freeInstallManager_->SetSCBCallStatus("com.ix.hiservcie", "ServiceAbility", "2024-7-17 00:00:00", false);
}

/**
 * @tc.number: FreeInstall_SetPreStartMissionCallStatus_001
 * @tc.name: SetPreStartMissionCallStatus
 * @tc.desc: Test SetPreStartMissionCallStatus.
 */
HWTEST_F(FreeInstallTest, FreeInstall_SetPreStartMissionCallStatus_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);

    freeInstallManager_->SetPreStartMissionCallStatus("com.ix.hiservcie", "ServiceAbility",
        "2024-7-17 00:00:00", false);
}

/**
 * @tc.number: FreeInstall_SetFreeInstallTaskSessionId_001
 * @tc.name: SetFreeInstallTaskSessionId
 * @tc.desc: Test SetFreeInstallTaskSessionId.
 */
HWTEST_F(FreeInstallTest, FreeInstall_SetFreeInstallTaskSessionId_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);

    freeInstallManager_->SetFreeInstallTaskSessionId("com.ix.hiservcie", "ServiceAbility",
        "2024-7-17 00:00:00", "sessionId");
}

/**
 * @tc.number: FreeInstall_VerifyStartFreeInstallPermission_001
 * @tc.name: VerifyStartFreeInstallPermission
 * @tc.desc: Test VerifyStartFreeInstallPermission.
 */
HWTEST_F(FreeInstallTest, FreeInstall_VerifyStartFreeInstallPermission_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);

    const sptr<IRemoteObject> callerToken = MockToken();
    freeInstallManager_->VerifyStartFreeInstallPermission(callerToken);
    freeInstallManager_->GetRecordIdByToken(callerToken);
}

/**
 * @tc.number: FreeInstall_SetAppRunningState_001
 * @tc.name: SetAppRunningState
 * @tc.desc: Test SetAppRunningState.
 */
HWTEST_F(FreeInstallTest, FreeInstall_SetAppRunningState_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);

    Want want;
    freeInstallManager_->SetAppRunningState(want);
}

/**
 * @tc.number: FreeInstall_PostUpgradeAtomicServiceTask_001
 * @tc.name: PostUpgradeAtomicServiceTask
 * @tc.desc: Test PostUpgradeAtomicServiceTask.
 */
HWTEST_F(FreeInstallTest, FreeInstall_PostUpgradeAtomicServiceTask_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);

    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 100;
    int resultCode = 0;
    freeInstallManager_->PostUpgradeAtomicServiceTask(resultCode, want, userId);
}

/**
 * @tc.number: FreeInstall_StartAbilityByOriginalWant_001
 * @tc.name: StartAbilityByOriginalWant
 * @tc.desc: Test StartAbilityByOriginalWant.
 */
HWTEST_F(FreeInstallTest, FreeInstall_StartAbilityByOriginalWant_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);

    FreeInstallInfo freeInstallInfo;
    freeInstallManager_->StartAbilityByOriginalWant(freeInstallInfo, "2024-07-17 00:00:00");
}

/**
 * @tc.number: FreeInstall_StartAbilityByConvertedWant_001
 * @tc.name: StartAbilityByConvertedWant
 * @tc.desc: Test StartAbilityByConvertedWant.
 */
HWTEST_F(FreeInstallTest, FreeInstall_StartAbilityByConvertedWant_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);

    FreeInstallInfo freeInstallInfo;
    freeInstallManager_->StartAbilityByConvertedWant(freeInstallInfo, "2024-07-17 00:00:00");
}

}  // namespace AppExecFwk
}  // namespace OHOS