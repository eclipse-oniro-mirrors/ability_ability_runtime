/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#define protected public
#include "ams_mgr_scheduler.h"
#include "mock_ipc_skeleton.h"
#include "mock_permission_verification.h"
#include "mock_my_flag.h"
#undef private
#undef protected

#include "ability_info.h"
#include "ability_manager_errors.h"
#include "application_info.h"
#include "mock_app_mgr_service_inner.h"
#include "mock_native_token.h"
#include "mock_task_handler_wrap.h"
#include "param.h"
#include "perf_profile.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr const char* SCENE_BOARD_BUNDLE_NAME = "com.ohos.sceneboard";
constexpr const char* SCENEBOARD_ABILITY_NAME = "com.ohos.sceneboard.MainAbility";
}

class AmsMgrSchedulerSecondTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        taskHandler_ = MockTaskHandlerWrap::CreateQueueHandler("AmsMgrSchedulerSecondTest");
        appMgrServiceInner_ = std::make_shared<MockAppMgrServiceInner>();
        appMgrServiceInner_->SetSceneBoardAttachFlag(true);
    }

    static void TearDownTestCase() {}

    void SetUp() override {}

    void TearDown() override {}

protected:
    static const std::string GetTestAppName()
    {
        return "test_app_name";
    }

    static const std::string GetTestAbilityName()
    {
        return "test_ability_name";
    }

    static std::shared_ptr<MockAppMgrServiceInner> appMgrServiceInner_;
    static std::shared_ptr<MockTaskHandlerWrap> taskHandler_;
};

std::shared_ptr<MockAppMgrServiceInner> AmsMgrSchedulerSecondTest::appMgrServiceInner_ = nullptr;
std::shared_ptr<MockTaskHandlerWrap> AmsMgrSchedulerSecondTest::taskHandler_ = nullptr;

/**
 * @tc.name: AmsMgrSchedulerSecondTest_LoadAbility_001
 * @tc.desc: Test LoadAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_LoadAbility_001, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize params
     */
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_LoadAbility_001 start");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::shared_ptr<AbilityRuntime::LoadParam> loadParam = std::make_shared<AbilityRuntime::LoadParam>();

    /**
     * @tc.steps: step2. LoadAbility with null abilityInfo
     * @tc.expected: step2. expect GetSceneBoardAttachFlag true
     */
    amsMgrScheduler->LoadAbility(abilityInfo, appInfo, want, loadParam);
    EXPECT_TRUE(appMgrServiceInner_->GetSceneBoardAttachFlag()); // not changed
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_LoadAbility_002
 * @tc.desc: Test LoadAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_LoadAbility_002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize params
     */
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_LoadAbility_002 start");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> appInfo = nullptr;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::shared_ptr<AbilityRuntime::LoadParam> loadParam = std::make_shared<AbilityRuntime::LoadParam>();

    /**
     * @tc.steps: step2. LoadAbility with null appInfo
     * @tc.expected: step2. expect GetSceneBoardAttachFlag true
     */
    amsMgrScheduler->LoadAbility(abilityInfo, appInfo, want, loadParam);
    EXPECT_TRUE(appMgrServiceInner_->GetSceneBoardAttachFlag()); // not changed
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_LoadAbility_003
 * @tc.desc: Test LoadAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_LoadAbility_003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize params
     */
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_LoadAbility_003 start");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::shared_ptr<AbilityRuntime::LoadParam> loadParam = std::make_shared<AbilityRuntime::LoadParam>();

    /**
     * @tc.steps: step2. LoadAbility with error permission
     * @tc.expected: step2. expect GetSceneBoardAttachFlag true
     */
    IPCSkeleton::SetCallingUid(-1);
    amsMgrScheduler->LoadAbility(abilityInfo, appInfo, want, loadParam);
    EXPECT_TRUE(appMgrServiceInner_->GetSceneBoardAttachFlag());
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_LoadAbility_004
 * @tc.desc: Test LoadAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_LoadAbility_004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize params
     */
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_LoadAbility_004 start");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::shared_ptr<AbilityRuntime::LoadParam> loadParam = std::make_shared<AbilityRuntime::LoadParam>();

    /**
     * @tc.steps: step2. LoadAbility with permission FOUNDATION_UID
     * @tc.expected: step2. expect taskhandler executed at least one time
     */
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    amsMgrScheduler->LoadAbility(abilityInfo, appInfo, want, loadParam);
    EXPECT_TRUE(appMgrServiceInner_->GetSceneBoardAttachFlag());
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_LoadAbility_005
 * @tc.desc: Test LoadAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_LoadAbility_005, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize params
     */
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_LoadAbility_005 start");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->bundleName = SCENE_BOARD_BUNDLE_NAME;
    abilityInfo->name = SCENEBOARD_ABILITY_NAME;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::shared_ptr<AbilityRuntime::LoadParam> loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    EXPECT_TRUE(appMgrServiceInner_->GetSceneBoardAttachFlag()); // default true

    /**
     * @tc.steps: step2. LoadAbility with permission FOUNDATION_UID
     * @tc.expected: step2. expect appMgrServiceInner_ GetSceneBoardAttachFlag false
     */
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    amsMgrScheduler->LoadAbility(abilityInfo, appInfo, want, loadParam);
    EXPECT_FALSE(appMgrServiceInner_->GetSceneBoardAttachFlag()); // false
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_LoadAbility_006
 * @tc.desc: Test LoadAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_LoadAbility_006, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize params
     */
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_LoadAbility_006 start");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->bundleName = SCENE_BOARD_BUNDLE_NAME;
    abilityInfo->name = "";
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::shared_ptr<AbilityRuntime::LoadParam> loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    appMgrServiceInner_->SetSceneBoardAttachFlag(true); // set true

    /**
     * @tc.steps: step2. LoadAbility with permission FOUNDATION_UID
     * @tc.expected: step2. expect appMgrServiceInner_ GetSceneBoardAttachFlag true
     */
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    amsMgrScheduler->LoadAbility(abilityInfo, appInfo, want, loadParam);
    EXPECT_TRUE(appMgrServiceInner_->GetSceneBoardAttachFlag());
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_UpdateAbilityState_001
 * @tc.desc: Test UpdateAbilityState
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_UpdateAbilityState_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_UpdateAbilityState_001 start");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, nullptr);

    amsMgrScheduler->UpdateAbilityState(nullptr, AbilityState::ABILITY_STATE_CREATE);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_UpdateAbilityState_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_UpdateAbilityState_002
 * @tc.desc: Test UpdateAbilityState
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_UpdateAbilityState_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_UpdateAbilityState_002 start");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);

    IPCSkeleton::SetCallingUid(-1);
    amsMgrScheduler->UpdateAbilityState(nullptr, AbilityState::ABILITY_STATE_CREATE);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_UpdateAbilityState_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_UpdateAbilityState_003
 * @tc.desc: Test UpdateAbilityState
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_UpdateAbilityState_003, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_UpdateAbilityState_003 start");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);

    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    amsMgrScheduler->UpdateAbilityState(nullptr, AbilityState::ABILITY_STATE_CREATE);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_UpdateAbilityState_003 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_KillProcessesByUserId_001
 * @tc.desc: Test KillProcessesByUserId
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_KillProcessesByUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillProcessesByUserId_001 start");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
    std::make_shared<AmsMgrScheduler>(nullptr, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect taskHandler_ time(0)
     */
    amsMgrScheduler->KillProcessesByUserId(-1, false, nullptr);
    amsMgrScheduler->KillProcessesByUserId(-1, true, nullptr);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillProcessesByUserId_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_KillProcessesByUserId_002
 * @tc.desc: Test KillProcessesByUserId
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_KillProcessesByUserId_002, TestSize.Level1)
{
    /**
     * @tc.steps: step1. amsMgrScheduler permission not ok
     * @tc.expected: step1. expect taskHandler_ time(0)
     */
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillProcessesByUserId_002 start");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
    MyFlag::flag_ = 0;
    IPCSkeleton::SetCallingUid(-1);
    amsMgrScheduler->KillProcessesByUserId(-1, false, nullptr);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillProcessesByUserId_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_KillProcessesByUserId_003
 * @tc.desc: Test KillProcessesByUserId
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_KillProcessesByUserId_003, TestSize.Level1)
{
    /**
     * @tc.steps: step1. amsMgrScheduler
     * @tc.expected: step1. expect taskHandler time(0)
     */
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillProcessesByUserId_003 start");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);

    MyFlag::flag_ = 0;
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    amsMgrScheduler->KillProcessesByUserId(-1, false, nullptr);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillProcessesByUserId_003 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_KillProcessesByUserId_004
 * @tc.desc: Test KillProcessesByUserId
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_KillProcessesByUserId_004, TestSize.Level1)
{
    /**
     * @tc.steps: step1. amsMgrScheduler
     * @tc.expected: step1. expect taskHandler time(AtLeast(1))
     */
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillProcessesByUserId_004 start");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);

    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    MockNativeToken::SetNativeToken();
    MyFlag::flag_ = 1;
    amsMgrScheduler->KillProcessesByUserId(-1, false, nullptr);
    MyFlag::flag_ = 0;
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillProcessesByUserId_004 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_KillProcessesByUserId_001
 * @tc.desc: Test KillProcessesByUserId
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_KillProcessesByPids_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillProcessesByPids_001 start");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
    appMgrServiceInner_ = nullptr; // not ready

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect taskHandler_ time(0)
     */
    std::vector<int32_t> pids;
    amsMgrScheduler->KillProcessesByPids(pids);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillProcessesByPids_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_KillProcessesByPids_002
 * @tc.desc: Test KillProcessesByUserId
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_KillProcessesByPids_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillProcessesByPids_002 start");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect taskHandler_ time(0)
     */
    std::vector<int32_t> pids = { 1 };
    amsMgrScheduler->KillProcessesByPids(pids);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillProcessesByPids_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_AttachPidToParent_001
 * @tc.desc: Test AttachPidToParent
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_AttachPidToParent_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_AttachPidToParent_001 start");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
    appMgrServiceInner_ = nullptr; // not ready

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect taskHandler_ time(0)
     */
    const sptr<IRemoteObject> token = nullptr;
    const sptr<IRemoteObject> callerToken = nullptr;
    amsMgrScheduler->AttachPidToParent(token, callerToken);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_AttachPidToParent_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_AttachPidToParent_002
 * @tc.desc: Test AttachPidToParent
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_AttachPidToParent_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_AttachPidToParent_002 start");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect taskHandler_ time(0)
     */
    const sptr<IRemoteObject> token = nullptr;
    const sptr<IRemoteObject> callerToken = nullptr;
    amsMgrScheduler->AttachPidToParent(token, callerToken);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_AttachPidToParent_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_KillProcessWithAccount_001
 * @tc.desc: Test KillProcessWithAccount
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_KillProcessWithAccount_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillProcessWithAccount_001 start");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner_, taskHandler_);
    appMgrServiceInner_ = nullptr; // not ready

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect taskHandler_ time(0)
     */
    const std::string bundleName = "testBundleName";
    auto ret = amsMgrScheduler->KillProcessWithAccount(bundleName, 1, false);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillProcessWithAccount_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_KillProcessWithAccount_002
 * @tc.desc: Test KillProcessWithAccount
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_KillProcessWithAccount_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillProcessWithAccount_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect ret ERR_OK
     */
    const std::string bundleName = "testBundleName";
    MyFlag::flag_ = MyFlag::IS_SHELL_CALL;
    auto ret = amsMgrScheduler->KillProcessWithAccount(bundleName, 1, false);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillProcessWithAccount_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_AbilityAttachTimeOut_001
 * @tc.desc: Test AbilityAttachTimeOut
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_AbilityAttachTimeOut_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_AbilityAttachTimeOut_001 start");
    const std::shared_ptr<MockAppMgrServiceInner> appMgrServiceInner = nullptr;
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect ret ERR_OK
     */
    const sptr<IRemoteObject> token = nullptr;
    amsMgrScheduler->AbilityAttachTimeOut(token);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_AbilityAttachTimeOut_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_AbilityAttachTimeOut_002
 * @tc.desc: Test AbilityAttachTimeOut
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_AbilityAttachTimeOut_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_AbilityAttachTimeOut_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("AmsMgrSchedulerSecondTest_AbilityAttachTimeOut_002");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect taskHandler times 0
     */
    IPCSkeleton::SetCallingUid(-1);
    const sptr<IRemoteObject> token = nullptr;
    amsMgrScheduler->AbilityAttachTimeOut(token);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_AbilityAttachTimeOut_002 start");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_AbilityAttachTimeOut_003
 * @tc.desc: Test AbilityAttachTimeOut
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_AbilityAttachTimeOut_003, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_AbilityAttachTimeOut_003 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("AmsMgrSchedulerSecondTest_AbilityAttachTimeOut_003");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect ret ERR_OK
     */
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    const sptr<IRemoteObject> token = nullptr;
    amsMgrScheduler->AbilityAttachTimeOut(token);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_AbilityAttachTimeOut_003 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_PrepareTerminate_001
 * @tc.desc: Test AbilityAttachTimeOut
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_PrepareTerminate_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_PrepareTerminate_001 start");
    const std::shared_ptr<MockAppMgrServiceInner> appMgrServiceInner = nullptr;
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect taskHandler_ time 0
     */
    const sptr<IRemoteObject> token = nullptr;
    amsMgrScheduler->PrepareTerminate(token, false);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_PrepareTerminate_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_PrepareTerminate_002
 * @tc.desc: Test PrepareTerminate
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_PrepareTerminate_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_PrepareTerminate_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("AmsMgrSchedulerSecondTest_PrepareTerminate_002");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect taskHandler times 0
     */
    IPCSkeleton::SetCallingUid(-1);
    const sptr<IRemoteObject> token = nullptr;
    amsMgrScheduler->PrepareTerminate(token, false);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_PrepareTerminate_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_PrepareTerminate_003
 * @tc.desc: Test PrepareTerminate
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_PrepareTerminate_003, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_PrepareTerminate_003 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("AmsMgrSchedulerSecondTest_PrepareTerminate_003");

    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect ret ERR_OK
     */
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    const sptr<IRemoteObject> token = nullptr;
    amsMgrScheduler->PrepareTerminate(token, false);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_PrepareTerminate_003 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_UpdateApplicationInfoInstalled_001
 * @tc.desc: Test UpdateApplicationInfoInstalled
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_UpdateApplicationInfoInstalled_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_UpdateApplicationInfoInstalled_001 start");
    const std::shared_ptr<MockAppMgrServiceInner> appMgrServiceInner = nullptr;
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect taskHandler_ time 0
     */
    auto ret = amsMgrScheduler->UpdateApplicationInfoInstalled("", 0, "", false);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_UpdateApplicationInfoInstalled_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_UpdateApplicationInfoInstalled_002
 * @tc.desc: Test UpdateApplicationInfoInstalled
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_UpdateApplicationInfoInstalled_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_UpdateApplicationInfoInstalled_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("UpdateApplicationInfoInstalled_002");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler);
    EXPECT_CALL(*appMgrServiceInner, UpdateApplicationInfoInstalled(_, _, _, _)).WillRepeatedly(Return(ERR_OK));

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect taskHandler times at least 1
     */
    auto ret = amsMgrScheduler->UpdateApplicationInfoInstalled("", 0, "", false);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_UpdateApplicationInfoInstalled_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_KillProcessesByAccessTokenId_001
 * @tc.desc: Test KillProcessesByAccessTokenId
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_KillProcessesByAccessTokenId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillProcessesByAccessTokenId_001 start");
    const std::shared_ptr<MockAppMgrServiceInner> appMgrServiceInner = nullptr;
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect taskHandler_ time 0
     */
    auto ret = amsMgrScheduler->KillProcessesByAccessTokenId(0);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillProcessesByAccessTokenId_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_KillProcessesByAccessTokenId_002
 * @tc.desc: Test UpdateApplicationInfoInstalled
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_KillProcessesByAccessTokenId_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillProcessesByAccessTokenId_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("KillProcessesByAccessTokenId_002");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect ERR_OK
     */
    MyFlag::flag_ = MyFlag::IS_SHELL_CALL;
    auto ret = amsMgrScheduler->KillProcessesByAccessTokenId(0);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillProcessesByAccessTokenId_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_KillApplicationByUid_001
 * @tc.desc: Test KillApplicationByUid
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_KillApplicationByUid_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillApplicationByUid_001 start");
    const std::shared_ptr<MockAppMgrServiceInner> appMgrServiceInner = nullptr;
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect taskHandler_ time 0
     */
    auto ret = amsMgrScheduler->KillApplicationByUid("", 0);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillProcessesByAccessTokenId_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_KillApplicationByUid_002
 * @tc.desc: Test UpdateApplicationInfoInstalled
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_KillApplicationByUid_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillApplicationByUid_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("KillApplicationByUid_002");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect ERR_OK
     */
    auto ret = amsMgrScheduler->KillApplicationByUid("", 0);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_KillApplicationByUid_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_StartSpecifiedAbility_001
 * @tc.desc: Test StartSpecifiedAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_StartSpecifiedAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_StartSpecifiedAbility_001 start");
    const std::shared_ptr<MockAppMgrServiceInner> appMgrServiceInner = nullptr;
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    const AAFwk::Want want;
    const AppExecFwk::AbilityInfo abilityInfo;
    AbilityRuntime::StartSpecifiedParam param;
    amsMgrScheduler->StartSpecifiedAbility(want, abilityInfo, param);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_StartSpecifiedAbility_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_StartSpecifiedAbility_002
 * @tc.desc: Test StartSpecifiedAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_StartSpecifiedAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_StartSpecifiedAbility_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("KillApplicationByUid_002");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect ERR_OK
     */
    IPCSkeleton::SetCallingUid(-1);
    const AAFwk::Want want;
    const AppExecFwk::AbilityInfo abilityInfo;
    AbilityRuntime::StartSpecifiedParam param;
    amsMgrScheduler->StartSpecifiedAbility(want, abilityInfo, param);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_StartSpecifiedAbility_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_StartSpecifiedAbility_003
 * @tc.desc: Test StartSpecifiedAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_StartSpecifiedAbility_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_StartSpecifiedAbility_003 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("StartSpecifiedAbility_003");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect ERR_OK
     */
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    const AAFwk::Want want;
    const AppExecFwk::AbilityInfo abilityInfo;
    AbilityRuntime::StartSpecifiedParam param;
    amsMgrScheduler->StartSpecifiedAbility(want, abilityInfo, param);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_StartSpecifiedAbility_003 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_StartSpecifiedProcess_001
 * @tc.desc: Test StartSpecifiedAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_StartSpecifiedProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_StartSpecifiedProcess_001 start");
    const std::shared_ptr<MockAppMgrServiceInner> appMgrServiceInner = nullptr;
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    const AAFwk::Want want;
    const AppExecFwk::AbilityInfo abilityInfo;
    amsMgrScheduler->StartSpecifiedProcess(want, abilityInfo, 0);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_StartSpecifiedProcess_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_StartSpecifiedProcess_002
 * @tc.desc: Test StartSpecifiedAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_StartSpecifiedProcess_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_StartSpecifiedProcess_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("KillApplicationByUid_002");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect ERR_OK
     */
    IPCSkeleton::SetCallingUid(-1);
    const AAFwk::Want want;
    const AppExecFwk::AbilityInfo abilityInfo;
    amsMgrScheduler->StartSpecifiedProcess(want, abilityInfo, 0);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_StartSpecifiedProcess_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_StartSpecifiedProcess_003
 * @tc.desc: Test StartSpecifiedAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_StartSpecifiedProcess_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_StartSpecifiedProcess_003 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("StartSpecifiedAbility_003");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect ERR_OK
     */
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    const AAFwk::Want want;
    const AppExecFwk::AbilityInfo abilityInfo;
    amsMgrScheduler->StartSpecifiedProcess(want, abilityInfo, 0);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_StartSpecifiedProcess_003 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_RegisterStartSpecifiedAbilityResponse_001
 * @tc.desc: Test StartSpecifiedAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_RegisterStartSpecifiedAbilityResponse_001,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_RegisterStartSpecifiedAbilityResponse_001 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    MyFlag::flag_ = 0;
    const sptr<IStartSpecifiedAbilityResponse> &response = nullptr;
    amsMgrScheduler->RegisterStartSpecifiedAbilityResponse(response);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_RegisterStartSpecifiedAbilityResponse_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_RegisterStartSpecifiedAbilityResponse_002
 * @tc.desc: Test StartSpecifiedAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_RegisterStartSpecifiedAbilityResponse_002,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_RegisterStartSpecifiedAbilityResponse_002 start");
    const std::shared_ptr<MockAppMgrServiceInner> appMgrServiceInner = nullptr;
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    MyFlag::flag_ = 1;
    const sptr<IStartSpecifiedAbilityResponse> &response = nullptr;
    amsMgrScheduler->RegisterStartSpecifiedAbilityResponse(response);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_RegisterStartSpecifiedAbilityResponse_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_RegisterStartSpecifiedAbilityResponse_003
 * @tc.desc: Test StartSpecifiedAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_RegisterStartSpecifiedAbilityResponse_003,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_RegisterStartSpecifiedAbilityResponse_003 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    const sptr<IStartSpecifiedAbilityResponse> &response = nullptr;
    amsMgrScheduler->RegisterStartSpecifiedAbilityResponse(response);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_RegisterStartSpecifiedAbilityResponse_003 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_GetApplicationInfoByProcessID_001
 * @tc.desc: Test KillApplicationSelf
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_GetApplicationInfoByProcessID_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_GetApplicationInfoByProcessID_001 start");
    const std::shared_ptr<MockAppMgrServiceInner> appMgrServiceInner = nullptr;
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    const int pid = 1;
    AppExecFwk::ApplicationInfo application;
    bool debug = false;
    auto ret = amsMgrScheduler->GetApplicationInfoByProcessID(pid, application, debug);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_GetApplicationInfoByProcessID_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_GetApplicationInfoByProcessID_002
 * @tc.desc: Test KillApplicationSelf
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_GetApplicationInfoByProcessID_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_GetApplicationInfoByProcessID_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("GetApplicationInfoByProcessID_002");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect ERR_OK
     */
    AppExecFwk::ApplicationInfo application;
    bool debug = false;
    auto ret = amsMgrScheduler->GetApplicationInfoByProcessID(1, application, debug);
    EXPECT_EQ(ret, ERR_NAME_NOT_FOUND);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_GetApplicationInfoByProcessID_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_NotifyAppMgrRecordExitReason_001
 * @tc.desc: Test NotifyAppMgrRecordExitReason
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_NotifyAppMgrRecordExitReason_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_NotifyAppMgrRecordExitReason_001 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, nullptr);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    auto ret = amsMgrScheduler->NotifyAppMgrRecordExitReason(0, 0, "");
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_NotifyAppMgrRecordExitReason_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_NotifyAppMgrRecordExitReason_002
 * @tc.desc: Test SetAbilityForegroundingFlagToAppRecord
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_NotifyAppMgrRecordExitReason_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_NotifyAppMgrRecordExitReason_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("SetAbilityForegroundingFlagToAppRecord_002");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect ERR_OK
     */
    auto ret = amsMgrScheduler->NotifyAppMgrRecordExitReason(0, 0, "");
    EXPECT_EQ(ret, ERR_NAME_NOT_FOUND); // empty bundlename
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_NotifyAppMgrRecordExitReason_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_SetEnableStartProcessFlagByUserId_001
 * @tc.desc: Test SetEnableStartProcessFlagByUserId
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_SetEnableStartProcessFlagByUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_SetEnableStartProcessFlagByUserId_001 start");
    std::shared_ptr<MockAppMgrServiceInner> appMgrServiceInner = nullptr;
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    amsMgrScheduler->SetEnableStartProcessFlagByUserId(0, false);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_SetEnableStartProcessFlagByUserId_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_SetEnableStartProcessFlagByUserId_002
 * @tc.desc: Test StartSpecifiedAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_SetEnableStartProcessFlagByUserId_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_SetEnableStartProcessFlagByUserId_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    IPCSkeleton::SetCallingUid(-1);
    amsMgrScheduler->SetEnableStartProcessFlagByUserId(0, false);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_SetEnableStartProcessFlagByUserId_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_SetEnableStartProcessFlagByUserId_003
 * @tc.desc: Test StartSpecifiedAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_SetEnableStartProcessFlagByUserId_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_SetEnableStartProcessFlagByUserId_003 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    amsMgrScheduler->SetEnableStartProcessFlagByUserId(0, false);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_SetEnableStartProcessFlagByUserId_003 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_RegisterAppDebugListener_001
 * @tc.desc: Test RegisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_RegisterAppDebugListener_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_RegisterAppDebugListener_001 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, nullptr);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    const sptr<IAppDebugListener> &listener = nullptr;
    auto ret = amsMgrScheduler->RegisterAppDebugListener(listener);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_RegisterAppDebugListener_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_RegisterAppDebugListener_002
 * @tc.desc: Test RegisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_RegisterAppDebugListener_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_RegisterAppDebugListener_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("RegisterAppDebugListener_002");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect ERR_OK
     */
    const sptr<IAppDebugListener> &listener = nullptr;
    auto ret = amsMgrScheduler->RegisterAppDebugListener(listener);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_RegisterAppDebugListener_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_RegisterAppDebugListener_001
 * @tc.desc: Test RegisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_UnregisterAppDebugListener_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_UnregisterAppDebugListener_001 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, nullptr);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    auto ret = amsMgrScheduler->UnregisterAppDebugListener(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_UnregisterAppDebugListener_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_UnregisterAppDebugListener_002
 * @tc.desc: Test UnregisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_UnregisterAppDebugListener_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_UnregisterAppDebugListener_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("UnregisterAppDebugListener_002");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect ERR_OK
     */
    auto ret = amsMgrScheduler->UnregisterAppDebugListener(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_UnregisterAppDebugListener_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_AttachAppDebug_001
 * @tc.desc: Test SetEnableStartProcessFlagByUserId
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_AttachAppDebug_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_AttachAppDebug_001 start");
    std::shared_ptr<MockAppMgrServiceInner> appMgrServiceInner = nullptr;
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    auto ret = amsMgrScheduler->AttachAppDebug("", false);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_AttachAppDebug_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_AttachAppDebug_002
 * @tc.desc: Test StartSpecifiedAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_AttachAppDebug_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_AttachAppDebug_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    MyFlag::flag_ = 0;
    auto ret = amsMgrScheduler->AttachAppDebug("", false);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_AttachAppDebug_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_AttachAppDebug_003
 * @tc.desc: Test AttachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_AttachAppDebug_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_AttachAppDebug_003 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    auto ret = amsMgrScheduler->AttachAppDebug("", false);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_AttachAppDebug_003 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_DetachAppDebug_001
 * @tc.desc: Test SetEnableStartProcessFlagByUserId
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_DetachAppDebug_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_DetachAppDebug_001 start");
    std::shared_ptr<MockAppMgrServiceInner> appMgrServiceInner = nullptr;
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    auto ret = amsMgrScheduler->DetachAppDebug("");
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_DetachAppDebug_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_DetachAppDebug_002
 * @tc.desc: Test StartSpecifiedAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_DetachAppDebug_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_DetachAppDebug_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    MyFlag::flag_ = 0;
    auto ret = amsMgrScheduler->DetachAppDebug("");
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_DetachAppDebug_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_SetEnableStartProcessFlagByUserId_003
 * @tc.desc: Test StartSpecifiedAbility
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_DetachAppDebug_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_DetachAppDebug_003 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    auto ret = amsMgrScheduler->DetachAppDebug("");
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_DetachAppDebug_003 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_SetAppWaitingDebug_001
 * @tc.desc: Test RegisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_SetAppWaitingDebug_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_SetAppWaitingDebug_001 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, nullptr);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    auto ret = amsMgrScheduler->SetAppWaitingDebug("", false);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_SetAppWaitingDebug_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_SetAppWaitingDebug_002
 * @tc.desc: Test UnregisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_SetAppWaitingDebug_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_SetAppWaitingDebug_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("UnregisterAppDebugListener_002");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect ERR_INVALID_VALUE bundleName empty
     */
    MyFlag::flag_ = MyFlag::IS_SHELL_CALL;
    auto ret = amsMgrScheduler->SetAppWaitingDebug("", false);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_SetAppWaitingDebug_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_CancelAppWaitingDebug_001
 * @tc.desc: Test RegisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_CancelAppWaitingDebug_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_CancelAppWaitingDebug_001 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, nullptr);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    auto ret = amsMgrScheduler->CancelAppWaitingDebug();
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_CancelAppWaitingDebug_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_CancelAppWaitingDebug_002
 * @tc.desc: Test UnregisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_CancelAppWaitingDebug_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_CancelAppWaitingDebug_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("UnregisterAppDebugListener_002");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect ERR_OK
     */
    auto ret = amsMgrScheduler->CancelAppWaitingDebug();
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_CancelAppWaitingDebug_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_GetWaitingDebugApp_001
 * @tc.desc: Test GetWaitingDebugApp
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_GetWaitingDebugApp_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_GetWaitingDebugApp_001 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, nullptr);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    std::vector<std::string> debugInfoList;
    auto ret = amsMgrScheduler->GetWaitingDebugApp(debugInfoList);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_GetWaitingDebugApp_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_GetWaitingDebugApp_002
 * @tc.desc: Test GetWaitingDebugApp
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_GetWaitingDebugApp_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_GetWaitingDebugApp_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("GetWaitingDebugApp_002");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect ERR_OK
     */
    MyFlag::flag_ = MyFlag::IS_SHELL_CALL; // 2 means shell call
    std::vector<std::string> debugInfoList;
    auto ret = amsMgrScheduler->GetWaitingDebugApp(debugInfoList);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_GetWaitingDebugApp_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_IsWaitingDebugApp_001
 * @tc.desc: Test IsWaitingDebugApp
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_IsWaitingDebugApp_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsWaitingDebugApp_001 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, nullptr);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    auto ret = amsMgrScheduler->IsWaitingDebugApp("");
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsWaitingDebugApp_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_IsWaitingDebugApp_002
 * @tc.desc: Test UnregisterAppDebugListener
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_IsWaitingDebugApp_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsWaitingDebugApp_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("IsWaitingDebugApp_002");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect ERR_OK
     */
    appMgrServiceInner->waitingDebugBundleList_.emplace("testBundleName", true);
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    auto ret = amsMgrScheduler->IsWaitingDebugApp("testBundleName");
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsWaitingDebugApp_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_RegisterAbilityDebugResponse_001
 * @tc.desc: Test RegisterAbilityDebugResponse
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_RegisterAbilityDebugResponse_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_RegisterAbilityDebugResponse_001 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, nullptr);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    auto ret = amsMgrScheduler->RegisterAbilityDebugResponse(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_RegisterAbilityDebugResponse_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_RegisterAbilityDebugResponse_002
 * @tc.desc: Test RegisterAbilityDebugResponse
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_RegisterAbilityDebugResponse_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_RegisterAbilityDebugResponse_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("RegisterAbilityDebugResponse_002");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect ERR_OK
     */
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    auto ret = amsMgrScheduler->RegisterAbilityDebugResponse(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_RegisterAbilityDebugResponse_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_IsAttachDebug_001
 * @tc.desc: Test IsAttachDebug
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_IsAttachDebug_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsAttachDebug_001 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, nullptr);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    auto ret = amsMgrScheduler->IsAttachDebug("");
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsAttachDebug_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_IsAttachDebug_002
 * @tc.desc: Test IsAttachDebug
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_IsAttachDebug_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsAttachDebug_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("IsAttachDebug_002");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady
     * @tc.expected: step1. expect ERR_OK
     */
    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    auto ret = amsMgrScheduler->IsAttachDebug("testBundleName");
    EXPECT_FALSE(ret); // appDebug return false
    TAG_LOGI(AAFwkTag::TEST, "IsAttachDebug_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_ClearProcessByToken_001
 * @tc.desc: Test ClearProcessByToken
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_ClearProcessByToken_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_ClearProcessByToken_001 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, nullptr);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    sptr<IRemoteObject> token = nullptr;
    amsMgrScheduler->ClearProcessByToken(token);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_ClearProcessByToken_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_ClearProcessByToken_002
 * @tc.desc: Test ClearProcessByToken
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_ClearProcessByToken_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_ClearProcessByToken_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    sptr<IRemoteObject> token = nullptr;
    amsMgrScheduler->ClearProcessByToken(token);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_ClearProcessByToken_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_IsMemorySizeSufficient_001
 * @tc.desc: Test IsMemorySizeSufficient
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_IsMemorySizeSufficient_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsMemorySizeSufficient_001 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, nullptr);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    auto ret = amsMgrScheduler->IsMemorySizeSufficient();
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsMemorySizeSufficient_001 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_IsMemorySizeSufficient_002
 * @tc.desc: Test IsMemorySizeSufficient
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_IsMemorySizeSufficient_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsMemorySizeSufficient_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    IPCSkeleton::SetCallingUid(-1);
    auto ret = amsMgrScheduler->IsMemorySizeSufficient();
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsMemorySizeSufficient_002 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_IsMemorySizeSufficient_003
 * @tc.desc: Test IsMemorySizeSufficient
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_IsMemorySizeSufficient_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsMemorySizeSufficient_003 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    auto ret = amsMgrScheduler->IsMemorySizeSufficient();
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsMemorySizeSufficient_003 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_AttachedToStatusBar_001
 * @tc.desc: Test AttachedToStatusBar
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_AttachedToStatusBar_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_ClearProcessByToken_001 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, nullptr);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    sptr<IRemoteObject> token = nullptr;
    amsMgrScheduler->AttachedToStatusBar(token);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_AttachedToStatusBar_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_AttachedToStatusBar_002
 * @tc.desc: Test AttachedToStatusBar
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_AttachedToStatusBar_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_AttachedToStatusBar_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    sptr<IRemoteObject> token = nullptr;
    amsMgrScheduler->AttachedToStatusBar(token);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_AttachedToStatusBar_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_BlockProcessCacheByPids_001
 * @tc.desc: Test AttachedToStatusBar
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_BlockProcessCacheByPids_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_BlockProcessCacheByPids_001 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, nullptr);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    std::vector<int32_t> pids;
    amsMgrScheduler->BlockProcessCacheByPids(pids);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_BlockProcessCacheByPids_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_BlockProcessCacheByPids_002
 * @tc.desc: Test ClearProcessByToken
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_BlockProcessCacheByPids_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_AttachedToStatusBar_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    std::vector<int32_t> pids;
    amsMgrScheduler->BlockProcessCacheByPids(pids);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_BlockProcessCacheByPids_002 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_CleanAbilityByUserRequest_001
 * @tc.desc: Test CleanAbilityByUserRequest
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_CleanAbilityByUserRequest_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_CleanAbilityByUserRequest_001 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, nullptr);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    amsMgrScheduler->CleanAbilityByUserRequest(nullptr);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_CleanAbilityByUserRequest_001 end");
}

/**
 * @tc.name: AmsMgrSchedulerSecondTest_CleanAbilityByUserRequest_002
 * @tc.desc: Test ClearProcessByToken
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_CleanAbilityByUserRequest_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_CleanAbilityByUserRequest_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    amsMgrScheduler->CleanAbilityByUserRequest(nullptr);
    EXPECT_TRUE(amsMgrScheduler != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_CleanAbilityByUserRequest_002 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_IsKilledForUpgradeWeb_001
 * @tc.desc: Test IsKilledForUpgradeWeb
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_IsKilledForUpgradeWeb_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsKilledForUpgradeWeb_001 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, nullptr);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    auto ret = amsMgrScheduler->IsKilledForUpgradeWeb("");
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsKilledForUpgradeWeb_001 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_IsKilledForUpgradeWeb_002
 * @tc.desc: Test IsKilledForUpgradeWeb
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_IsKilledForUpgradeWeb_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsKilledForUpgradeWeb_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    IPCSkeleton::SetCallingUid(-1);
    auto ret = amsMgrScheduler->IsKilledForUpgradeWeb("");
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsKilledForUpgradeWeb_002 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_IsKilledForUpgradeWeb_003
 * @tc.desc: Test IsKilledForUpgradeWeb
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_IsKilledForUpgradeWeb_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsKilledForUpgradeWeb_003 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    auto ret = amsMgrScheduler->IsKilledForUpgradeWeb("");
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsKilledForUpgradeWeb_003 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_IsProcessContainsOnlyUIAbility_001
 * @tc.desc: Test IsProcessAttached
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_IsProcessContainsOnlyUIAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsProcessContainsOnlyUIAbility_001 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, nullptr);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    auto ret = amsMgrScheduler->IsProcessContainsOnlyUIAbility(0);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsProcessContainsOnlyUIAbility_001 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_IsProcessContainsOnlyUIAbility_002
 * @tc.desc: Test IsProcessAttached
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_IsProcessContainsOnlyUIAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsProcessAttached_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    IPCSkeleton::SetCallingUid(-1);
    auto ret = amsMgrScheduler->IsProcessContainsOnlyUIAbility(0);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsProcessContainsOnlyUIAbility_002 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_IsProcessAttached_001
 * @tc.desc: Test IsProcessAttached
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_IsProcessAttached_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsProcessAttached_001 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, nullptr);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    sptr<IRemoteObject> token = nullptr;
    auto ret = amsMgrScheduler->IsProcessAttached(token);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsProcessAttached_001 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_IsProcessAttached_002
 * @tc.desc: Test IsProcessAttached
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_IsProcessAttached_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsProcessAttached_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    IPCSkeleton::SetCallingUid(-1);
    sptr<IRemoteObject> token = nullptr;
    auto ret = amsMgrScheduler->IsProcessAttached(token);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsProcessAttached_002 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_IsProcessAttached_002
 * @tc.desc: Test IsProcessAttached
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_IsProcessAttached_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsProcessAttached_003 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    sptr<IRemoteObject> token = nullptr;
    auto ret = amsMgrScheduler->IsProcessAttached(token);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsProcessAttached_003 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_IsCallerKilling_001
 * @tc.desc: Test IsCallerKilling
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_IsCallerKilling_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsCallerKilling_001 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, nullptr);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    auto ret = amsMgrScheduler->IsCallerKilling("");
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsCallerKilling_001 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_IsCallerKilling_002
 * @tc.desc: Test IsCallerKilling
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_IsCallerKilling_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsCallerKilling_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    IPCSkeleton::SetCallingUid(-1);
    auto ret = amsMgrScheduler->IsCallerKilling("");
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsCallerKilling_002 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_IsCallerKilling_003
 * @tc.desc: Test IsCallerKilling
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_IsCallerKilling_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsCallerKilling_003 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);

    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect ERR_INVALID_OPERATION
     */
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    auto ret = amsMgrScheduler->IsCallerKilling("");
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_IsCallerKilling_003 end");
}

/*
 * Feature: AmsMgrScheduler
 * Function: KillProcessesInBatch
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler KillProcessesInBatch
 * EnvConditions: NA
 * CaseDescription: not initial scheduler
 */
HWTEST_F(AmsMgrSchedulerSecondTest, KillProcessesInBatch_001, TestSize.Level1)
{
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler = std::make_shared<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    std::vector<int32_t> pids = {0};
    int32_t ret = amsMgrScheduler->KillProcessesInBatch(pids);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/*
 * Feature: AmsMgrScheduler
 * Function: KillProcessesInBatch
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler KillProcessesInBatch
 * EnvConditions: NA
 * CaseDescription: initial scheduler call KillProcessesInBatch success
 */
HWTEST_F(AmsMgrSchedulerSecondTest, KillProcessesInBatch_002, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler>  amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);
    ASSERT_NE(amsMgrScheduler, nullptr);
    std::vector<int32_t> pids = {0};
    int32_t ret = amsMgrScheduler->KillProcessesInBatch(pids);
    EXPECT_NE(ret, ERR_INVALID_OPERATION);
}

/*
 * Feature: AmsMgrScheduler
 * Function: ForceKillApplication
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler ForceKillApplication
 * EnvConditions: NA
 * CaseDescription: not initial scheduler
 */
HWTEST_F(AmsMgrSchedulerSecondTest, ForceKillApplication_001, TestSize.Level1)
{
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler = std::make_shared<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    std::string bundleName = "bundleName";
    int userId = -1;
    int appIndex = 0;
    int32_t ret = amsMgrScheduler->ForceKillApplication(bundleName, userId, appIndex);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/*
 * Feature: AmsMgrScheduler
 * Function: ForceKillApplication
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler ForceKillApplication
 * EnvConditions: NA
 * CaseDescription: initial scheduler call ForceKillApplication success
 */
HWTEST_F(AmsMgrSchedulerSecondTest, ForceKillApplication_002, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);
    ASSERT_NE(amsMgrScheduler, nullptr);
    std::string bundleName = "bundleName";
    int userId = -1;
    int appIndex = 0;
    int32_t ret = amsMgrScheduler->ForceKillApplication(bundleName, userId, appIndex);
    EXPECT_NE(ret, ERR_INVALID_OPERATION);
}

/*
 * Feature: AmsMgrScheduler
 * Function: KillApplicationWithUserId
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler KillApplicationWithUserId
 * EnvConditions: NA
 * CaseDescription: not initial scheduler
 */
HWTEST_F(AmsMgrSchedulerSecondTest, KillApplicationWithUserId_001, TestSize.Level1)
{
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler = std::make_shared<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    std::string bundleName = "bundleName";
    int userId = -1;
    int appIndex = 0;
    int32_t ret = amsMgrScheduler->KillApplicationWithUserId(bundleName, userId, appIndex);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/*
 * Feature: AmsMgrScheduler
 * Function: KillApplicationWithUserId
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler KillApplicationWithUserId
 * EnvConditions: NA
 * CaseDescription: initial scheduler call KillApplicationWithUserId success
 */
HWTEST_F(AmsMgrSchedulerSecondTest, KillApplicationWithUserId_002, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);
    ASSERT_NE(amsMgrScheduler, nullptr);
    std::string bundleName = "bundleName";
    int userId = -1;
    int appIndex = 0;
    int32_t ret = amsMgrScheduler->KillApplicationWithUserId(bundleName, userId, appIndex);
    EXPECT_NE(ret, ERR_INVALID_OPERATION);
}

/*
 * Feature: AmsMgrScheduler
 * Function: IsNoRequireBigMemory
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler IsNoRequireBigMemory
 * EnvConditions: NA
 * CaseDescription: not initial scheduler
 */
HWTEST_F(AmsMgrSchedulerSecondTest, IsNoRequireBigMemory_001, TestSize.Level1)
{
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler = std::make_shared<AmsMgrScheduler>(nullptr, nullptr);
    ASSERT_NE(amsMgrScheduler, nullptr);
    bool ret = amsMgrScheduler->IsNoRequireBigMemory();
    EXPECT_TRUE(ret);
}

/*
 * Feature: AmsMgrScheduler
 * Function: IsNoRequireBigMemory
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler IsNoRequireBigMemory
 * EnvConditions: NA
 * CaseDescription: have no permission
 */
HWTEST_F(AmsMgrSchedulerSecondTest, IsNoRequireBigMemory_002, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);
    ASSERT_NE(amsMgrScheduler, nullptr);
    IPCSkeleton::SetCallingUid(-1);
    bool ret = amsMgrScheduler->IsNoRequireBigMemory();
    EXPECT_TRUE(ret);
}

/*
 * Feature: AmsMgrScheduler
 * Function: IsNoRequireBigMemory
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler IsNoRequireBigMemory
 * EnvConditions: NA
 * CaseDescription: initial scheduler call IsNoRequireBigMemory success
 */
HWTEST_F(AmsMgrSchedulerSecondTest, IsNoRequireBigMemory_003, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);
    ASSERT_NE(amsMgrScheduler, nullptr);
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    bool ret = amsMgrScheduler->IsNoRequireBigMemory();
    EXPECT_TRUE(ret);
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_SetKeepAliveAppService_001
 * @tc.desc: Test SetKeepAliveAppService
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, AmsMgrSchedulerSecondTest_SetKeepAliveAppService_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_SetKeepAliveAppService_001 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    ASSERT_NE(appMgrServiceInner, nullptr);
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);
    ASSERT_NE(amsMgrScheduler, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    ASSERT_NE(appInfo, nullptr);
    appInfo->bundleName = SCENE_BOARD_BUNDLE_NAME;
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, 0, "foundation");
    ASSERT_NE(appRecord, nullptr);
    appRecord->SetUid(0);
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(1, appRecord);
    /**
     * @tc.steps: step1. amsMgrScheduler isReady false
     * @tc.expected: step1. expect isKeepAliveAppService_ is false
     */
    amsMgrScheduler->amsHandler_ = nullptr;
    amsMgrScheduler->SetKeepAliveAppService(SCENE_BOARD_BUNDLE_NAME, true, 0);
    EXPECT_FALSE(appRecord->isKeepAliveAppService_);

    /**
     * @tc.steps: step2. set appService keepAlive success
     * @tc.expected: step2. expect isKeepAliveAppService_ is true
     */
    amsMgrScheduler->amsHandler_ = taskHandler_;
    IPCSkeleton::SetCallingUid(Constants::FOUNDATION_UID);
    amsMgrScheduler->SetKeepAliveAppService(SCENE_BOARD_BUNDLE_NAME, true, 0);
    EXPECT_TRUE(appRecord->isKeepAliveAppService_);

    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_SetKeepAliveAppService_001 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_PreloadApplicationByPhase_001
 * @tc.desc: Test PreloadApplicationByPhase when service not ready
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, PreloadApplicationByPhase_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_PreloadApplicationByPhase_001 start");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(nullptr, nullptr);

    int32_t ret = amsMgrScheduler->PreloadApplicationByPhase("", 100, 0, PreloadPhase::WINDOW_STAGE_CREATED);
    EXPECT_EQ(ret, AAFwk::ERR_APP_MGR_SERVICE_NOT_READY);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_PreloadApplicationByPhase_001 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_PreloadApplicationByPhase_002
 * @tc.desc: Test PreloadApplicationByPhase when not foundation call
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, PreloadApplicationByPhase_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_PreloadApplicationByPhase_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    EXPECT_CALL(*appMgrServiceInner, IsFoundationCall()).WillOnce(Return(false));

    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);
    int32_t ret = amsMgrScheduler->PreloadApplicationByPhase("", 100, 0, PreloadPhase::WINDOW_STAGE_CREATED);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_PreloadApplicationByPhase_002 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_PreloadApplicationByPhase_003
 * @tc.desc: Test PreloadApplicationByPhase success
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, PreloadApplicationByPhase_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_PreloadApplicationByPhase_003 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    EXPECT_CALL(*appMgrServiceInner, IsFoundationCall()).WillOnce(Return(true));
    EXPECT_CALL(*appMgrServiceInner, PreloadApplicationByPhase(_, _, _, _)).WillOnce(Return(ERR_OK));

    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);
    int32_t ret = amsMgrScheduler->PreloadApplicationByPhase("bundleName", 100, 0, PreloadPhase::WINDOW_STAGE_CREATED);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_PreloadApplicationByPhase_003 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_NotifyPreloadAbilityStateChanged_001
 * @tc.desc: Test NotifyPreloadAbilityStateChanged when service not ready
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, NotifyPreloadAbilityStateChanged_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_NotifyPreloadAbilityStateChanged_001 start");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(nullptr, nullptr);

    int32_t ret = amsMgrScheduler->NotifyPreloadAbilityStateChanged(nullptr, true);
    EXPECT_EQ(ret, AAFwk::ERR_APP_MGR_SERVICE_NOT_READY);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_NotifyPreloadAbilityStateChanged_001 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_NotifyPreloadAbilityStateChanged_002
 * @tc.desc: Test NotifyPreloadAbilityStateChanged when not foundation call
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, NotifyPreloadAbilityStateChanged_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_NotifyPreloadAbilityStateChanged_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    EXPECT_CALL(*appMgrServiceInner, IsFoundationCall()).WillOnce(Return(false));

    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);
    int32_t ret = amsMgrScheduler->NotifyPreloadAbilityStateChanged(nullptr, true);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_NotifyPreloadAbilityStateChanged_002 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_NotifyPreloadAbilityStateChanged_003
 * @tc.desc: Test NotifyPreloadAbilityStateChanged success
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, NotifyPreloadAbilityStateChanged_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_NotifyPreloadAbilityStateChanged_003 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    EXPECT_CALL(*appMgrServiceInner, IsFoundationCall()).WillOnce(Return(true));
    EXPECT_CALL(*appMgrServiceInner, NotifyPreloadAbilityStateChanged(_, _)).WillOnce(Return(ERR_OK));

    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);
    int32_t ret = amsMgrScheduler->NotifyPreloadAbilityStateChanged(nullptr, true);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_NotifyPreloadAbilityStateChanged_003 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_CheckPreloadAppRecordExist_001
 * @tc.desc: Test CheckPreloadAppRecordExist when service not ready
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, CheckPreloadAppRecordExist_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_CheckPreloadAppRecordExist_001 start");
    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(nullptr, nullptr);

    bool isExist = false;
    int32_t ret = amsMgrScheduler->CheckPreloadAppRecordExist("", 0, 0, isExist);
    EXPECT_EQ(ret, AAFwk::ERR_APP_MGR_SERVICE_NOT_READY);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_CheckPreloadAppRecordExist_001 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_CheckPreloadAppRecordExist_002
 * @tc.desc: Test CheckPreloadAppRecordExist when not foundation call
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, CheckPreloadAppRecordExist_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_CheckPreloadAppRecordExist_002 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    EXPECT_CALL(*appMgrServiceInner, IsFoundationCall()).WillOnce(Return(false));

    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);
    bool isExist = false;
    int32_t ret = amsMgrScheduler->CheckPreloadAppRecordExist("", 0, 0, isExist);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_CheckPreloadAppRecordExist_002 end");
}

/*
 * @tc.name: AmsMgrSchedulerSecondTest_CheckPreloadAppRecordExist_003
 * @tc.desc: Test CheckPreloadAppRecordExist success
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrSchedulerSecondTest, CheckPreloadAppRecordExist_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_CheckPreloadAppRecordExist_003 start");
    auto appMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    EXPECT_CALL(*appMgrServiceInner, IsFoundationCall()).WillOnce(Return(true));
    EXPECT_CALL(*appMgrServiceInner, CheckPreloadAppRecordExist(_, _, _)).WillOnce(Return(true));

    std::shared_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_shared<AmsMgrScheduler>(appMgrServiceInner, taskHandler_);
    bool isExist = false;
    int32_t ret = amsMgrScheduler->CheckPreloadAppRecordExist("bundleName", 0, 0, isExist);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(isExist);
    TAG_LOGI(AAFwkTag::TEST, "AmsMgrSchedulerSecondTest_CheckPreloadAppRecordExist_003 end");
}
} // AppExecFwk
} // OHOS

