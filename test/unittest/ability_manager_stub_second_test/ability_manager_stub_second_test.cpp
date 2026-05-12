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
#include "ability_manager_stub_impl_mock.h"
#include "ability_scheduler.h"
#include "app_debug_listener_stub_mock.h"
#include "hilog_tag_wrapper.h"
#include "iremote_proxy.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_token.h"
#include "skill_execute_result.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
namespace {
const int USER_ID = 100;
}  // namespace

class AbilityManagerStubSecondTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void WriteInterfaceToken(MessageParcel& data);
    sptr<AbilityManagerStubImplMock> stub_{ nullptr };
};

void AbilityManagerStubSecondTest::SetUpTestCase(void)
{}
void AbilityManagerStubSecondTest::TearDownTestCase(void)
{}
void AbilityManagerStubSecondTest::TearDown()
{}

void AbilityManagerStubSecondTest::SetUp()
{
    stub_ = new AbilityManagerStubImplMock();
}

void AbilityManagerStubSecondTest::WriteInterfaceToken(MessageParcel& data)
{
    data.WriteInterfaceToken(AbilityManagerStub::GetDescriptor());
}


/**
 * @tc.name: OnRemoteRequestInnerFirst_0100
 * @tc.desc: Test OnRemoteRequestInnerFirst
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, OnRemoteRequestInnerFirst_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerFirst_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::TERMINATE_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::MINIMIZE_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::ATTACH_ABILITY_THREAD),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_TRANSITION_DONE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_WINDOW_CONFIG_TRANSITION_DONE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY_DONE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::DISCONNECT_ABILITY_DONE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::COMMAND_ABILITY_DONE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::COMMAND_ABILITY_WINDOW_DONE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::ACQUIRE_DATA_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::RELEASE_DATA_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::BACK_TO_CALLER_UIABILITY)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->OnRemoteRequestInnerFirst(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->OnRemoteRequestInnerFirst(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerFirst_0100 end");
}

/**
 * @tc.name: OnRemoteRequestInnerSecond_0100
 * @tc.desc: Test OnRemoteRequestInnerSecond
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, OnRemoteRequestInnerSecond_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerSecond_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::KILL_PROCESS),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::UNINSTALL_APP),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::UPGRADE_APP),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_ADD_CALLER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_WITH_SPECIFY_TOKENID),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_AS_CALLER_BY_TOKEN),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_AS_CALLER_FOR_OPTIONS),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_SESSION_ABILITY_ADD_CALLER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_SESSION_ABILITY_FOR_OPTIONS),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_ONLY_UI_ABILITY)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->OnRemoteRequestInnerSecond(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->OnRemoteRequestInnerSecond(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerSecond_0100 end");
}

/**
 * @tc.name: OnRemoteRequestInnerThird_0100
 * @tc.desc: Test OnRemoteRequestInnerThird
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, OnRemoteRequestInnerThird_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerThird_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_BY_INSIGHT_INTENT),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::DISCONNECT_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_SERVICE_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::DUMP_STATE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::DUMPSYS_STATE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_SETTINGS),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::CONTINUE_MISSION),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::CONTINUE_MISSION_OF_BUNDLENAME),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::CONTINUE_ABILITY)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->OnRemoteRequestInnerThird(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->OnRemoteRequestInnerThird(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerThird_0100 end");
}

/**
 * @tc.name: OnRemoteRequestInnerFourth_0100
 * @tc.desc: Test OnRemoteRequestInnerFourth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, OnRemoteRequestInnerFourth_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerFourth_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_CONTINUATION),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_COMPLETE_CONTINUATION),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_CONTINUATION_RESULT),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_RESULT_TO_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_REMOTE_MISSION_LISTENER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_REMOTE_ON_LISTENER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_REMOTE_OFF_LISTENER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_REMOTE_MISSION_LISTENER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_OPTIONS),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SYNC_MISSIONS)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->OnRemoteRequestInnerFourth(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->OnRemoteRequestInnerFourth(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerFourth_0100 end");
}

/**
 * @tc.name: OnRemoteRequestInnerFifth_0100
 * @tc.desc: Test OnRemoteRequestInnerFifth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, OnRemoteRequestInnerFifth_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerFifth_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_SYNC_MISSIONS),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::FORCE_TIMEOUT),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::FREE_INSTALL_ABILITY_FROM_REMOTE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::ADD_FREE_INSTALL_OBSERVER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY_WITH_TYPE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_RECOVERY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_RECOVERY_ENABLE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_RECOVERY_SUBMITINFO),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAR_RECOVERY_PAGE_STACK),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::MINIMIZE_UI_ABILITY_BY_SCB),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::CLOSE_UI_ABILITY_BY_SCB)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->OnRemoteRequestInnerFifth(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->OnRemoteRequestInnerFifth(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerFifth_0100 end");
}

/**
 * @tc.name: OnRemoteRequestInnerSixth_0100
 * @tc.desc: Test OnRemoteRequestInnerSixth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, OnRemoteRequestInnerSixth_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerSixth_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_COLLABORATOR),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_COLLABORATOR),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_APP_DEBUG_LISTENER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_APP_DEBUG_LISTENER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::ATTACH_APP_DEBUG),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::DETACH_APP_DEBUG),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::IS_ABILITY_CONTROLLER_START),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_INTENT),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_INSIGHT_INTENT_DONE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::OPEN_FILE)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->OnRemoteRequestInnerSixth(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->OnRemoteRequestInnerSixth(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerSixth_0100 end");
}

/**
 * @tc.name: OnRemoteRequestInnerSeventh_0100
 * @tc.desc: Test OnRemoteRequestInnerSeventh
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, OnRemoteRequestInnerSeventh_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerSeventh_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_SENDER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_PENDING_WANT_SENDER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::CANCEL_PENDING_WANT_SENDER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_UID),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_USERID),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_BUNDLENAME),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_CODE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_TYPE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_CANCEL_LISTENER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_CANCEL_LISTENER)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->OnRemoteRequestInnerSeventh(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->OnRemoteRequestInnerSeventh(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerSeventh_0100 end");
}

/**
 * @tc.name: OnRemoteRequestInnerEighth_0100
 * @tc.desc: Test OnRemoteRequestInnerEighth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, OnRemoteRequestInnerEighth_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerEighth_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_REQUEST_WANT),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_SENDER_INFO),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_APP_MEMORY_SIZE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::IS_RAM_CONSTRAINED_DEVICE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::LOCK_MISSION_FOR_CLEANUP),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::UNLOCK_MISSION_FOR_CLEANUP),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_SESSION_LOCKED_STATE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_MISSION_LISTENER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_MISSION_LISTENER)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->OnRemoteRequestInnerEighth(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->OnRemoteRequestInnerEighth(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerEighth_0100 end");
}

/**
 * @tc.name: OnRemoteRequestInnerNinth_0100
 * @tc.desc: Test OnRemoteRequestInnerNinth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, OnRemoteRequestInnerNinth_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerNinth_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_MISSION_INFOS),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_MISSION_INFO_BY_ID),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAN_MISSION),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAN_ALL_MISSIONS),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSION_TO_FRONT),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSION_TO_FRONT_BY_OPTIONS),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSIONS_TO_FOREGROUND),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSIONS_TO_BACKGROUND),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_CALL_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::CALL_REQUEST_DONE)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->OnRemoteRequestInnerNinth(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->OnRemoteRequestInnerNinth(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerNinth_0100 end");
}

/**
 * @tc.name: OnRemoteRequestInnerTenth_0100
 * @tc.desc: Test OnRemoteRequestInnerTenth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, OnRemoteRequestInnerTenth_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerTenth_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::RELEASE_CALL_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_USER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_USER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::LOGOUT_USER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ABILITY_RUNNING_INFO),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_EXTENSION_RUNNING_INFO),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PROCESS_RUNNING_INFO),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_ABILITY_CONTROLLER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_MISSION_SNAPSHOT_INFO),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::IS_USER_A_STABILITY_TEST)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->OnRemoteRequestInnerTenth(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->OnRemoteRequestInnerTenth(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerTenth_0100 end");
}

/**
 * @tc.name: OnRemoteRequestInnerEleventh_0100
 * @tc.desc: Test OnRemoteRequestInnerEleventh
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, OnRemoteRequestInnerEleventh_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerEleventh_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::ACQUIRE_SHARE_DATA),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::SHARE_DATA_DONE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ABILITY_TOKEN),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::FORCE_EXIT_APP),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::RECORD_APP_EXIT_REASON),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::RECORD_PROCESS_EXIT_REASON),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_SESSION_HANDLER)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->OnRemoteRequestInnerEleventh(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->OnRemoteRequestInnerEleventh(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerEleventh_0100 end");
}

/**
 * @tc.name: OnRemoteRequestInnerTwelveth_0100
 * @tc.desc: Test OnRemoteRequestInnerTwelveth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, OnRemoteRequestInnerTwelveth_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerTwelveth_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_USER_TEST),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::FINISH_USER_TEST),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_TOP_ABILITY_TOKEN),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::CHECK_UI_EXTENSION_IS_FOCUSED),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::DELEGATOR_DO_ABILITY_FOREGROUND),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::DELEGATOR_DO_ABILITY_BACKGROUND),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::DO_ABILITY_FOREGROUND),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::DO_ABILITY_BACKGROUND),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_MISSION_ID_BY_ABILITY_TOKEN),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_TOP_ABILITY)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->OnRemoteRequestInnerTwelveth(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->OnRemoteRequestInnerTwelveth(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerTwelveth_0100 end");
}

/**
 * @tc.name: OnRemoteRequestInnerThirteenth_0100
 * @tc.desc: Test OnRemoteRequestInnerThirteenth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, OnRemoteRequestInnerThirteenth_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerThirteenth_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ELEMENT_NAME_BY_TOKEN),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::DUMP_ABILITY_INFO_DONE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_EXTENSION_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_EXTENSION_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::UPDATE_MISSION_SNAPSHOT_FROM_WMS),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_CONNECTION_OBSERVER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_CONNECTION_OBSERVER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_DLP_CONNECTION_INFOS),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_ABILITY_TO_BACKGROUND),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_UI_ABILITY_TO_BACKGROUND)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->OnRemoteRequestInnerThirteenth(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->OnRemoteRequestInnerThirteenth(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerThirteenth_0100 end");
}

/**
 * @tc.name: OnRemoteRequestInnerFourteenth_0100
 * @tc.desc: Test OnRemoteRequestInnerFourteenth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, OnRemoteRequestInnerFourteenth_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerFourteenth_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_CONTINUE_STATE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::PREPARE_TERMINATE_ABILITY_BY_SCB),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REQUEST_MODAL_UIEXTENSION),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_UI_EXTENSION_ROOT_HOST_INFO),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_UI_EXTENSION_SESSION_INFO),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::PRELOAD_UIEXTENSION_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::TERMINATE_UI_SERVICE_EXTENSION_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_ABILITIES_IN_SPLIT_WINDOW_MODE)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->OnRemoteRequestInnerFourteenth(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->OnRemoteRequestInnerFourteenth(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerFourteenth_0100 end");
}

/**
 * @tc.name: OnRemoteRequestInnerFifteenth_0100
 * @tc.desc: Test OnRemoteRequestInnerFifteenth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, OnRemoteRequestInnerFifteenth_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerFifteenth_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_LABEL),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_ICON),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_WMS_HANDLER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::COMPLETEFIRSTFRAMEDRAWING),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_EXTENSION_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::MINIMIZE_UI_EXTENSION_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::TERMINATE_UI_EXTENSION_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_UI_EXTENSION_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::PREPARE_TERMINATE_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_DIALOG_SESSION_INFO),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_DIALOG_RESULT),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_ABILITY_FIRST_FRAME_STATE_OBSERVER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_ABILITY_FIRST_FRAME_STATE_OBSERVER)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->OnRemoteRequestInnerFifteenth(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->OnRemoteRequestInnerFifteenth(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerFifteenth_0100 end");
}

/**
 * @tc.name: OnRemoteRequestInnerSixteenth_0100
 * @tc.desc: Test OnRemoteRequestInnerSixteenth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, OnRemoteRequestInnerSixteenth_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerSixteenth_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::COMPLETE_FIRST_FRAME_DRAWING_BY_SCB),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_EXTENSION_ABILITY_EMBEDDED),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_EXTENSION_CONSTRAINED_EMBEDDED),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REQUEST_DIALOG_SERVICE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REPORT_DRAWN_COMPLETED),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_MISSION_VAILD),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::VERIFY_PERMISSION),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_ABILITY_BY_SCB),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_ROOT_SCENE_SESSION),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::CALL_ABILITY_BY_SCB),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SPECIFIED_ABILITY_BY_SCB)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->OnRemoteRequestInnerSixteenth(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->OnRemoteRequestInnerSixteenth(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerSixteenth_0100 end");
}

/**
 * @tc.name: OnRemoteRequestInnerSeventeenth_0100
 * @tc.desc: Test OnRemoteRequestInnerSeventeenth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, OnRemoteRequestInnerSeventeenth_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerSeventeenth_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_SAVE_AS_RESULT),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_SESSIONMANAGERSERVICE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::UPDATE_SESSION_INFO),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_STATUS_BAR_DELEGATE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::KILL_PROCESS_WITH_PREPARE_TERMINATE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_AUTO_STARTUP_SYSTEM_CALLBACK),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_AUTO_STARTUP_SYSTEM_CALLBACK),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_APPLICATION_AUTO_STARTUP),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::CANCEL_APPLICATION_AUTO_STARTUP),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_ALL_AUTO_STARTUP_APPLICATION),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_AUTO_STARTUP_STATUS_FOR_SELF)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->OnRemoteRequestInnerSeventeenth(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->OnRemoteRequestInnerSeventeenth(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerSeventeenth_0100 end");
}

/**
 * @tc.name: OnRemoteRequestInnerEighteenth_0100
 * @tc.desc: Test OnRemoteRequestInnerEighteenth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, OnRemoteRequestInnerEighteenth_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerEighteenth_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_CONNECTION_DATA),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_APPLICATION_AUTO_STARTUP_BY_EDM),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::CANCEL_APPLICATION_AUTO_STARTUP_BY_EDM),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_RESULT_AS_CALLER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_RESULT_AS_CALLER_FOR_OPTIONS),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_FOREGROUND_UI_ABILITIES),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::RESTART_APP),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::OPEN_ATOMIC_SERVICE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::IS_EMBEDDED_OPEN_ALLOWED),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REQUEST_ASSERT_FAULT_DIALOG)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->OnRemoteRequestInnerEighteenth(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->OnRemoteRequestInnerEighteenth(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerEighteenth_0100 end");
}

/**
 * @tc.name: OnRemoteRequestInnerNineteenth_0100
 * @tc.desc: Test OnRemoteRequestInnerNineteenth
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, OnRemoteRequestInnerNineteenth_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerNineteenth_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_DEBUG_ASSERT_RESULT),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::CHANGE_ABILITY_VISIBILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::CHANGE_UI_ABILITY_VISIBILITY_BY_SCB),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SHORTCUT),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_RESIDENT_PROCESS_ENABLE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ABILITY_STATE_BY_PERSISTENT_ID),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::TRANSFER_ABILITY_RESULT),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_FROZEN_PROCESS_BY_RSS),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::PRE_START_MISSION),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAN_UI_ABILITY_BY_SCB),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::OPEN_LINK),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::TERMINATE_MISSION),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::BLOCK_ALL_APP_START)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->OnRemoteRequestInnerNineteenth(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->OnRemoteRequestInnerNineteenth(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerNineteenth_0100 end");
}

/**
 * @tc.name: OnRemoteRequestInnerTwentyFirst_0100
 * @tc.desc: Test OnRemoteRequestInnerTwentyFirst
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, OnRemoteRequestInnerTwentyFirst_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerTwentyFirst_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_APP_SERVICE_EXTENSION_KEEP_ALIVE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_APP_SERVICE_EXTENSIONS_KEEP_ALIVE)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->OnRemoteRequestInnerTwentyFirst(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->OnRemoteRequestInnerTwentyFirst(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInnerTwentyFirst_0100 end");
}

/**
 * @tc.name: OnRemoteRequestInner_0100
 * @tc.desc: Test OnRemoteRequestInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, OnRemoteRequestInner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInner_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(AbilityManagerInterfaceCode::TERMINATE_ABILITY);
    stub_->OnRemoteRequestInner(code, data, reply, option);
    
    code = static_cast<uint32_t>(AbilityManagerInterfaceCode::ACQUIRE_SHARE_DATA);
    stub_->OnRemoteRequestInner(code, data, reply, option);

    code = 0;
    auto ret = stub_->OnRemoteRequestInner(code, data, reply, option);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequestInner_0100 end");
}

/**
 * @tc.name: HandleOnRemoteRequestInnerFirst_0100
 * @tc.desc: Test HandleOnRemoteRequestInnerFirst
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, HandleOnRemoteRequestInnerFirst_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleOnRemoteRequestInnerFirst_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::TERMINATE_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::KILL_PROCESS),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_BY_INSIGHT_INTENT),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_CONTINUATION),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_SYNC_MISSIONS),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_COLLABORATOR),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_SENDER),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_REQUEST_WANT),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_MISSION_INFOS),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::RELEASE_CALL_ABILITY)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->HandleOnRemoteRequestInnerFirst(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->HandleOnRemoteRequestInnerFirst(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "HandleOnRemoteRequestInnerFirst_0100 end");
}

/**
 * @tc.name: HandleOnRemoteRequestInnerSecond_0100
 * @tc.desc: Test HandleOnRemoteRequestInnerSecond
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, HandleOnRemoteRequestInnerSecond_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleOnRemoteRequestInnerSecond_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::ACQUIRE_SHARE_DATA),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_USER_TEST),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ELEMENT_NAME_BY_TOKEN),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_CONTINUE_STATE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_LABEL),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::COMPLETE_FIRST_FRAME_DRAWING_BY_SCB),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_SAVE_AS_RESULT),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_CONNECTION_DATA),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_DEBUG_ASSERT_RESULT)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->HandleOnRemoteRequestInnerSecond(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->HandleOnRemoteRequestInnerSecond(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "HandleOnRemoteRequestInnerSecond_0100 end");
}

/**
 * @tc.name: HandleOnRemoteRequestInnerThird_0100
 * @tc.desc: Test HandleOnRemoteRequestInnerThird
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, HandleOnRemoteRequestInnerThird_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleOnRemoteRequestInnerThird_0100 begin");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code[] = {
        static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_APP_SERVICE_EXTENSION_KEEP_ALIVE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_APP_SERVICE_EXTENSIONS_KEEP_ALIVE),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::SUSPEND_EXTENSION_ABILITY),
        static_cast<uint32_t>(AbilityManagerInterfaceCode::RESUME_EXTENSION_ABILITY)
    };

    int i = 0;
    while (i < sizeof(code) / sizeof(uint32_t)) {
        stub_->HandleOnRemoteRequestInnerThird(code[i++], data, reply, option);
    }
    
    uint32_t code_ = 0;
    auto ret = stub_->HandleOnRemoteRequestInnerThird(code_, data, reply, option);
    EXPECT_EQ(ret, ERR_CODE_NOT_EXIST);

    TAG_LOGI(AAFwkTag::TEST, "HandleOnRemoteRequestInnerThird_0100 end");
}

/*
 * Feature: AbilityManagerService
 * Function: SuspendExtensionAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SuspendExtensionAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SuspendExtensionAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubSecondTest, SuspendExtensionAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    stub_->SuspendExtensionAbilityInner(data, reply);
    EXPECT_TRUE(stub_ != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: SuspendExtensionAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SuspendExtensionAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SuspendExtensionAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubSecondTest, SuspendExtensionAbilityInner_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    WriteInterfaceToken(data);
    data.WriteParcelable(connect->AsObject());
    auto res = stub_->SuspendExtensionAbilityInner(data, reply);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: ResumeExtensionAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ResumeExtensionAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ResumeExtensionAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubSecondTest, ResumeExtensionAbilityInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    stub_->ResumeExtensionAbilityInner(data, reply);
    EXPECT_TRUE(stub_ != nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: ResumeExtensionAbilityInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService ResumeExtensionAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify the function ResumeExtensionAbilityInner is normal flow.
 */
HWTEST_F(AbilityManagerStubSecondTest, ResumeExtensionAbilityInner_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    WriteInterfaceToken(data);
    data.WriteParcelable(connect->AsObject());
    auto res = stub_->ResumeExtensionAbilityInner(data, reply);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: SetOnNewWantSkipScenariosInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetOnNewWantSkipScenariosInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SetOnNewWantSkipScenariosInner is normal flow.
 */
HWTEST_F(AbilityManagerStubSecondTest, SetOnNewWantSkipScenariosInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto ret = stub_->SetOnNewWantSkipScenariosInner(data, reply);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/*
 * Feature: AbilityManagerService
 * Function: SetOnNewWantSkipScenariosInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetOnNewWantSkipScenariosInner
 * EnvConditions: NA
 * CaseDescription: Verify the function SetOnNewWantSkipScenariosInner is normal flow.
 */
HWTEST_F(AbilityManagerStubSecondTest, SetOnNewWantSkipScenariosInner_002, TestSize.Level1)
{
    MessageParcel data;
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    WriteInterfaceToken(data);
    data.WriteRemoteObject(connect->AsObject());
    int32_t scenarios = 1;
    data.WriteInt32(scenarios);
    MessageParcel reply;
    MessageOption option;
    auto ret = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_ON_NEW_WANT_SKIP_SCENARIOS), data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: NotifyStartupExceptionBySCBInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService NotifyStartupExceptionBySCBInner
 * EnvConditions: NA
 * CaseDescription: Verify the function NotifyStartupExceptionBySCBInner is normal flow.
 */
HWTEST_F(AbilityManagerStubSecondTest, NotifyStartupExceptionBySCBInner_001, TestSize.Level1)
{
    MessageParcel data;
    sptr<IAbilityConnection> connect = new AbilityConnectCallback();
    WriteInterfaceToken(data);
    data.WriteRemoteObject(connect->AsObject());
    int32_t requestId = 1;
    data.WriteInt32(requestId);
    MessageParcel reply;
    MessageOption option;
    auto ret = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_STARTUP_EXCEPTION_BY_SCB), data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
}

/*
 * Feature: AbilityManagerService
 * Function: PreloadApplicationInner
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService PreloadApplicationInner
 * EnvConditions: NA
 * CaseDescription: Verify the function PreloadApplicationInner is normal flow.
 */
HWTEST_F(AbilityManagerStubSecondTest, PreloadApplicationInner_001, TestSize.Level1)
{
    MessageParcel data;
    WriteInterfaceToken(data);
    std::string bundleName = "bundleName";
    data.WriteString(bundleName);
    int32_t userId = -1;
    data.WriteInt32(userId);
    int32_t appIndex = 0;
    data.WriteInt32(appIndex);
    MessageParcel reply;
    MessageOption option;
    auto ret = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::PRELOAD_APPLICATION), data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
}

/*
 * Feature: AbilityManagerStub
 * Function: StartAbilityByOEExtInner
 * SubFunction: Error case with null want
 * FunctionPoints: Test error handling when want is null
 * EnvConditions: Invalid want parameter
 * CaseDescription: Verify that StartAbilityByOEExtInner returns ERR_INVALID_VALUE when want is null
 */
HWTEST_F(AbilityManagerStubSecondTest, StartAbilityByOEExtInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByOEExtInner_002 begin");

    MessageParcel data;
    WriteInterfaceToken(data);

    MessageParcel reply;
    MessageOption option;

    EXPECT_CALL(*stub_, StartAbilityByOEExt(_, _, _, _))
        .Times(0);

    auto ret = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_BY_OE_EXT), data, reply, option);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByOEExtInner_002 end");
}

/*
 * Feature: AbilityManagerStub
 * Function: StartAbilityByOEExtInner
 * SubFunction: Normal case with valid parameters
 * FunctionPoints: Test normal execution with valid want, caller token, hostPid and specifiedFlag
 * EnvConditions: Normal conditions
 * CaseDescription: Verify that StartAbilityByOEExtInner executes correctly with valid parameters
 */
HWTEST_F(AbilityManagerStubSecondTest, StartAbilityByOEExtInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByOEExtInner_001 begin");

    MessageParcel data;
    WriteInterfaceToken(data);

    Want want;
    want.SetElementName("com.test.bundle", "com.test.TestAbility", "test");
    data.WriteParcelable(&want);

    EXPECT_TRUE(data.WriteBool(true));
    auto token = sptr<IRemoteObject>(new AbilityScheduler());
    EXPECT_TRUE(data.WriteRemoteObject(token));
    
    EXPECT_TRUE(data.WriteInt32(12345));
    EXPECT_TRUE(data.WriteString("test_flag"));

    MessageParcel reply;
    MessageOption option;

    EXPECT_CALL(*stub_, StartAbilityByOEExt(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    auto ret = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_BY_OE_EXT), data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByOEExtInner_001 end");
}

/**
 * @tc.name: ExecuteInAppSkillInner_001
 * @tc.desc: Test ExecuteInAppSkillInner with normal parameters
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, ExecuteInAppSkillInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteInAppSkillInner_001 begin");

    MessageParcel data;
    WriteInterfaceToken(data);

    data.WriteString16(Str8ToStr16("bundleName"));
    data.WriteString16(Str8ToStr16("moduleName"));
    data.WriteString16(Str8ToStr16("skillName"));
    data.WriteString16(Str8ToStr16("scriptPath"));
    data.WriteString16(Str8ToStr16("functionName"));

    AAFwk::WantParams params;
    data.WriteParcelable(&params);

    data.WriteBool(false);

    MessageParcel reply;
    MessageOption option;

    EXPECT_CALL(*stub_, ExecuteInAppSkill(_, _, _, _, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    auto ret = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_IN_APP_SKILL), data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    EXPECT_EQ(reply.ReadInt32(), ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteInAppSkillInner_001 end");
}

/**
 * @tc.name: ExecuteInAppSkillInner_002
 * @tc.desc: Test ExecuteInAppSkillInner with null skillArgs (parcel returns null)
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, ExecuteInAppSkillInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteInAppSkillInner_002 begin");

    MessageParcel data;
    WriteInterfaceToken(data);

    data.WriteString16(Str8ToStr16("bundleName"));
    data.WriteString16(Str8ToStr16("moduleName"));
    data.WriteString16(Str8ToStr16("skillName"));
    data.WriteString16(Str8ToStr16("scriptPath"));
    data.WriteString16(Str8ToStr16("functionName"));

    data.WriteBool(false);

    MessageParcel reply;
    MessageOption option;

    EXPECT_CALL(*stub_, ExecuteInAppSkill(_, _, _, _, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    auto ret = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_IN_APP_SKILL), data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteInAppSkillInner_002 end");
}

/**
 * @tc.name: ExecuteSkillDoneWithTokenInner_001
 * @tc.desc: Test ExecuteSkillDoneWithTokenInner with null token
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, ExecuteSkillDoneWithTokenInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDoneWithTokenInner_001 begin");

    MessageParcel data;
    WriteInterfaceToken(data);

    data.WriteRemoteObject(nullptr);

    MessageParcel reply;
    MessageOption option;

    auto ret = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_SKILL_DONE_WITH_TOKEN), data, reply, option);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDoneWithTokenInner_001 end");
}

/**
 * @tc.name: ExecuteSkillDoneWithTokenInner_002
 * @tc.desc: Test ExecuteSkillDoneWithTokenInner with normal parameters
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, ExecuteSkillDoneWithTokenInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDoneWithTokenInner_002 begin");

    MessageParcel data;
    WriteInterfaceToken(data);

    auto token = sptr<IRemoteObject>(new AbilityScheduler());
    data.WriteRemoteObject(token);
    data.WriteString("requestCode");
    data.WriteInt32(0);

    AppExecFwk::SkillExecuteResult result;
    data.WriteParcelable(&result);

    MessageParcel reply;
    MessageOption option;

    EXPECT_CALL(*stub_, ExecuteSkillDone(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    auto ret = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_SKILL_DONE_WITH_TOKEN), data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    EXPECT_EQ(reply.ReadInt32(), ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDoneWithTokenInner_002 end");
}

/**
 * @tc.name: QuerySkillTypeInner_001
 * @tc.desc: Test QuerySkillTypeInner with normal parameters
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSecondTest, QuerySkillTypeInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QuerySkillTypeInner_001 begin");

    MessageParcel data;
    WriteInterfaceToken(data);

    data.WriteString16(Str8ToStr16("bundleName"));
    data.WriteString16(Str8ToStr16("moduleName"));
    data.WriteString16(Str8ToStr16("skillName"));

    MessageParcel reply;
    MessageOption option;

    EXPECT_CALL(*stub_, QuerySkillType(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    auto ret = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_SKILL_TYPE), data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
    EXPECT_EQ(reply.ReadInt32(), ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "QuerySkillTypeInner_001 end");
}
} // namespace AAFwk
} // namespace OHOS
