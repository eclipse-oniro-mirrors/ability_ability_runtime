/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <functional>
#include <gtest/gtest.h>
#define private public
#define protected public
#include "ability_loader.h"
#include "ui_ability_thread.h"
#undef private
#undef protected
#include "ability_context.h"
#include "ability_handler.h"
#include "ability_local_record.h"
#include "mock_ability_token.h"
#include "ohos_application.h"
#include "ui_ability_impl.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using Want = OHOS::AAFwk::Want;

static const int32_t STARTID = 0;
static const std::string DEVICE_ID = "deviceId";
static const std::string TEST = "test";
const unsigned int ZEROTAG = 0;
static const int32_t CODE1 = -1;

class UIAbilityThreadTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void UIAbilityThreadTest::SetUpTestCase(void) {}

void UIAbilityThreadTest::TearDownTestCase(void) {}

void UIAbilityThreadTest::SetUp(void) {}

void UIAbilityThreadTest::TearDown(void) {}

/**
 * @tc.number: AbilityRuntime_DumpAbilityInfo_0100
 * @tc.name: DumpAbilityInfo
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_DumpAbilityInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_DumpAbilityInfo_0100 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);

            std::vector<std::string> params;
            std::vector<std::string> info;
            abilitythread->DumpAbilityInfo(params, info);
            EXPECT_EQ(info.size(), ZEROTAG);
            GTEST_LOG_(INFO) << "info:";
            for (auto item : info) {
                GTEST_LOG_(INFO) << item;
            }
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_DumpAbilityInfo_0100 end";
}

/**
 * @tc.number: AbilityRuntime_DumpAbilityInfo_0200
 * @tc.name: DumpAbilityInfo
 * @tc.desc: Test DumpAbilityInfo function when token_ and abilityHandler_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_DumpAbilityInfo_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_DumpAbilityInfo_0200 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(abilitythread->token_, nullptr);
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    std::vector<std::string> params;
    std::vector<std::string> info;
    abilitythread->DumpAbilityInfo(params, info);
    GTEST_LOG_(INFO) << "AbilityRuntime_DumpAbilityInfo_0200 end";
}

/**
 * @tc.number: AbilityRuntime_DumpAbilityInfo_0300
 * @tc.name: DumpAbilityInfo
 * @tc.desc: Test DumpAbilityInfo function when abilityHandler_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_DumpAbilityInfo_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_DumpAbilityInfo_0300 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(abilitythread->token_, nullptr);
    EXPECT_EQ(abilitythread->abilityHandler_, nullptr);
    std::vector<std::string> params;
    std::vector<std::string> info;
    abilitythread->DumpAbilityInfo(params, info);
    GTEST_LOG_(INFO) << "AbilityRuntime_DumpAbilityInfo_0300 end";
}

/**
 * @tc.number: AbilityRuntime_DumpAbilityInfo_0400
 * @tc.name: DumpAbilityInfo
 * @tc.desc: Test DumpAbilityInfo function when token_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_DumpAbilityInfo_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_DumpAbilityInfo_0400 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->token_ = nullptr;
    EXPECT_EQ(abilitythread->token_, nullptr);
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    std::vector<std::string> params;
    std::vector<std::string> info;
    abilitythread->DumpAbilityInfo(params, info);
    GTEST_LOG_(INFO) << "AbilityRuntime_DumpAbilityInfo_0400 end";
}

/**
 * @tc.number: AbilityRuntime_ScheduleSaveAbilityState_0100
 * @tc.name: ScheduleSaveAbilityState
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_ScheduleSaveAbilityState_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleSaveAbilityState_0100 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            abilitythread->ScheduleSaveAbilityState();
            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleSaveAbilityState_0100 end";
}

/**
 * @tc.number: AbilityRuntime_ScheduleSaveAbilityState_0200
 * @tc.name: ScheduleSaveAbilityState
 * @tc.desc: Test ScheduleSaveAbilityState function when abilityImpl_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_ScheduleSaveAbilityState_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleSaveAbilityState_0200 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        abilitythread->ScheduleSaveAbilityState();
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleSaveAbilityState_0200 end";
}

/**
 * @tc.number: AbilityRuntime_ScheduleRestoreAbilityState_0100
 * @tc.name: ScheduleRestoreAbilityState
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_ScheduleRestoreAbilityState_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleRestoreAbilityState_0100 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            PacMap state;
            abilitythread->ScheduleRestoreAbilityState(state);
            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleRestoreAbilityState_0100 end";
}

/**
 * @tc.number: AbilityRuntime_ScheduleRestoreAbilityState_0200
 * @tc.name: ScheduleRestoreAbilityState
 * @tc.desc: Test Attach_3_Param function when abilityImpl_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_ScheduleRestoreAbilityState_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleRestoreAbilityState_0200 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        abilitythread->ScheduleSaveAbilityState();
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleRestoreAbilityState_0200 end";
}

/**
 * @tc.number: AbilityRuntime_Attach_3_Param_0100
 * @tc.name: Attach
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_Attach_3_Param_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Attach_3_Param_0100 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_Attach_3_Param_0100 end";
}

/**
 * @tc.number: AbilityRuntime_Attach_3_Param_0200
 * @tc.name: Attach
 * @tc.desc: Test Attach_3_Param function when application is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_Attach_3_Param_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Attach_3_Param_0200 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = nullptr;
            auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_Attach_3_Param_0200 end";
}

/**
 * @tc.number: AbilityRuntime_Attach_2_Param_0100
 * @tc.name: Attach
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_Attach_2_Param_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Attach_2_Param_0100 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
            abilitythread->Attach(application, abilityRecord, nullptr);
            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_Attach_2_Param_0100 end";
}

/**
 * @tc.number: AbilityRuntime_Attach_2_Param_0200
 * @tc.name: Attach
 * @tc.desc: Test Attach_2_Param function when application is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_Attach_2_Param_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Attach_2_Param_0200 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = nullptr;
            auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            abilitythread->Attach(application, abilityRecord, nullptr);
            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_Attach_2_Param_0200 end";
}

/**
 * @tc.number: AbilityRuntime_ScheduleAbilityTransaction_0100
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_ScheduleAbilityTransaction_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityTransaction_0100 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            Want want;
            LifeCycleStateInfo lifeCycleStateInfo;
            abilitythread->ScheduleAbilityTransaction(want, lifeCycleStateInfo);
            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityTransaction_0100 end";
}

/**
 * @tc.number: AbilityRuntime_ScheduleAbilityTransaction_0200
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_ScheduleAbilityTransaction_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityTransaction_0200 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
            abilitythread->Attach(application, abilityRecord, nullptr);
            Want want;
            LifeCycleStateInfo lifeCycleStateInfo;
            abilitythread->ScheduleAbilityTransaction(want, lifeCycleStateInfo);
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityTransaction_0200 end";
}

/*
 * @tc.number: AbilityRuntime_ScheduleAbilityTransaction_0300
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Test ScheduleAbilityTransaction function when token_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_ScheduleAbilityTransaction_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityTransaction_0100 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        abilitythread->token_ = nullptr;
        EXPECT_EQ(abilitythread->token_, nullptr);
        abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
        EXPECT_NE(abilitythread->abilityHandler_, nullptr);
        Want want;
        LifeCycleStateInfo lifeCycleStateInfo;
        abilitythread->ScheduleAbilityTransaction(want, lifeCycleStateInfo);
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityTransaction_0300 end";
}

/**
 * @tc.number: AbilityRuntime_ScheduleAbilityTransaction_0300
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Test ScheduleAbilityTransaction function when token_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_ScheduleAbilityTransaction_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityTransaction_0400 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(abilitythread->token_, nullptr);
    Want want;
    LifeCycleStateInfo lifeCycleStateInfo;
    abilitythread->ScheduleAbilityTransaction(want, lifeCycleStateInfo);
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityTransaction_0400 end";
}

/**
 * @tc.number: AbilityRuntime_ScheduleAbilityTransaction_0400
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Test ScheduleAbilityTransaction function when abilityHandler_ and token_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_ScheduleAbilityTransaction_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityTransaction_0500 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(abilitythread->token_, nullptr);
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    Want want;
    LifeCycleStateInfo lifeCycleStateInfo;
    abilitythread->ScheduleAbilityTransaction(want, lifeCycleStateInfo);
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityTransaction_0500 end";
}

/**
 * @tc.number: AbilityRuntime_SendResult_0100
 * @tc.name: SendResult
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_SendResult_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_SendResult_0100 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            std::shared_ptr<AbilityRuntime::UIAbilityImpl> abilityimpl =
                std::make_shared<AbilityRuntime::UIAbilityImpl>();
            abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            int requestCode = 0;
            int resultCode = 0;
            Want want;
            abilitythread->SendResult(requestCode, resultCode, want);
            sleep(1);
        }
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_SendResult_0100 end";
}

/**
 * @tc.number: AbilityRuntime_SendResult_0200
 * @tc.name: SendResult
 * @tc.desc: Test SendResult function when abilityImpl_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_SendResult_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_SendResult_0200 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    if (abilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "MockUIAbility";
        abilityInfo->type = AbilityType::PAGE;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
        auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
        abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
        int requestCode = 0;
        int resultCode = 0;
        Want want;
        abilitythread->SendResult(requestCode, resultCode, want);
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_SendResult_0200 end";
}

/**
 * @tc.number: AbilityRuntime_SendResult_0300
 * @tc.name: SendResult
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_SendResult_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_SendResult_0300 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockUIAbility";
    abilityInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto application = std::make_shared<OHOSApplication>();
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
    abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    int requestCode = STARTID;
    int resultCode = STARTID;
    Want want;
    abilitythread->SendResult(requestCode, resultCode, want);
    GTEST_LOG_(INFO) << "AbilityRuntime_SendResult_0300 end";
}

/**
 * @tc.number: AbilityRuntime_HandleAbilityTransaction_0100
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Test HandleAbilityTransaction function when abilityImpl_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_HandleAbilityTransaction_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleAbilityTransaction_0100 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    Want want;
    LifeCycleStateInfo lifeCycleStateInfo;
    abilitythread->HandleAbilityTransaction(want, lifeCycleStateInfo);
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleAbilityTransaction_0100 end";
}

/**
 * @tc.number: AbilityRuntime_HandleAbilityTransaction_0200
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Test HandleAbilityTransaction function when abilityImpl_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_HandleAbilityTransaction_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleAbilityTransaction_0200 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    Want want;
    LifeCycleStateInfo lifeCycleStateInfo;
    abilitythread->HandleAbilityTransaction(want, lifeCycleStateInfo);
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleAbilityTransaction_0200 end";
}

/**
 * @tc.number: AbilityRuntime_ScheduleUpdateConfiguration_0100
 * @tc.name: ScheduleUpdateConfiguration
 * @tc.desc: Test ScheduleUpdateConfiguration function when abilityHandler_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_ScheduleUpdateConfiguration_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleUpdateConfiguration_0100 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    EXPECT_EQ(abilitythread->abilityHandler_, nullptr);
    Configuration config;
    abilitythread->ScheduleUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleUpdateConfiguration_0100 end";
}

/**
 * @tc.number: AbilityRuntime_ScheduleUpdateConfiguration_0200
 * @tc.name: ScheduleUpdateConfiguration
 * @tc.desc: Test ScheduleUpdateConfiguration function when abilityHandler_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_ScheduleUpdateConfiguration_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleUpdateConfiguration_0200 start";
    AbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    Configuration config;
    abilitythread->ScheduleUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleUpdateConfiguration_0200 end";
}

/**
 * @tc.number: AbilityRuntime_HandleUpdateConfiguration_0100
 * @tc.name: HandleUpdateConfiguration
 * @tc.desc: Test HandleUpdateConfiguration function when abilityImpl_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_HandleUpdateConfiguration_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleUpdateConfiguration_0100 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    Configuration config;
    abilitythread->HandleUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleUpdateConfiguration_0100 end";
}

/**
 * @tc.number: AbilityRuntime_HandleUpdateConfiguration_0200
 * @tc.name: HandleUpdateConfiguration
 * @tc.desc: Test HandleUpdateConfiguration function when abilityImpl_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_HandleUpdateConfiguration_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleUpdateConfiguration_0200 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    Configuration config;
    abilitythread->HandleUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleUpdateConfiguration_0200 end";
}

/**
 * @tc.number: AbilityRuntime_ContinueAbility_0100
 * @tc.name: ContinueAbility
 * @tc.desc: Test ContinueAbility function when abilityImpl_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_ContinueAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ContinueAbility_0100 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    std::string deviceId = DEVICE_ID;
    uint32_t versionCode = STARTID;
    abilitythread->ContinueAbility(deviceId, versionCode);
    GTEST_LOG_(INFO) << "AbilityRuntime_ContinueAbility_0100 end";
}

/**
 * @tc.number: AbilityRuntime_ContinueAbility_0200
 * @tc.name: ContinueAbility
 * @tc.desc: Test ContinueAbility function when abilityImpl_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_ContinueAbility_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ContinueAbility_0200 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    std::string deviceId = DEVICE_ID;
    uint32_t versionCode = STARTID;
    abilitythread->ContinueAbility(deviceId, versionCode);
    GTEST_LOG_(INFO) << "AbilityRuntime_ContinueAbility_0200 end";
}

/**
 * @tc.number: AbilityRuntime_NotifyContinuationResult_0100
 * @tc.name: NotifyContinuationResult
 * @tc.desc: Test NotifyContinuationResult function when abilityImpl_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_NotifyContinuationResult_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_NotifyContinuationResult_0100 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    int32_t result = STARTID;
    abilitythread->NotifyContinuationResult(result);
    GTEST_LOG_(INFO) << "AbilityRuntime_NotifyContinuationResult_0100 end";
}

/**
 * @tc.number: AbilityRuntime_NotifyContinuationResult_0200
 * @tc.name: NotifyContinuationResult
 * @tc.desc: Test NotifyContinuationResult function when abilityImpl_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_NotifyContinuationResult_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_NotifyContinuationResult_0200 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    int32_t result = STARTID;
    abilitythread->NotifyContinuationResult(result);
    GTEST_LOG_(INFO) << "AbilityRuntime_NotifyContinuationResult_0200 end";
}

/**
 * @tc.number: AbilityRuntime_NotifyMemoryLevel_0100
 * @tc.name: NotifyMemoryLevel
 * @tc.desc: Test NotifyMemoryLevel function when abilityImpl_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_NotifyMemoryLevel_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_NotifyMemoryLevel_0100 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    int32_t result = STARTID;
    abilitythread->NotifyMemoryLevel(result);
    GTEST_LOG_(INFO) << "AbilityRuntime_NotifyMemoryLevel_0100 end";
}

/**
 * @tc.number: AbilityRuntime_NotifyMemoryLevel_0200
 * @tc.name: NotifyMemoryLevel
 * @tc.desc: Test NotifyMemoryLevel function when abilityImpl_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_NotifyMemoryLevel_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_NotifyMemoryLevel_0200 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    int32_t result = STARTID;
    abilitythread->NotifyMemoryLevel(result);
    GTEST_LOG_(INFO) << "AbilityRuntime_NotifyMemoryLevel_0200 end";
}

/**
 * @tc.number: AbilityRuntime_BuildAbilityContext_0100
 * @tc.name: BuildAbilityContext
 * @tc.desc: Test BuildAbilityContext function when Parameters is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_BuildAbilityContext_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_BuildAbilityContext_0100 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockUIAbility";
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(abilityInfo, nullptr);
    EXPECT_NE(application, nullptr);
    EXPECT_NE(token, nullptr);
    abilitythread->BuildAbilityContext(abilityInfo, application, token, nullptr, 0);
    GTEST_LOG_(INFO) << "AbilityRuntime_BuildAbilityContext_0100 end";
}

/**
 * @tc.number: AbilityRuntime_DumpAbilityInfoInner_0100
 * @tc.name: DumpAbilityInfoInner
 * @tc.desc: Test DumpAbilityInfoInner function when currentAbility_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_DumpAbilityInfoInner_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_DumpAbilityInfoInner_0100 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->currentAbility_ = std::make_shared<AbilityRuntime::UIAbility>();
    EXPECT_NE(abilitythread->currentAbility_, nullptr);
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    std::vector<std::string> params;
    std::vector<std::string> info;
    abilitythread->DumpAbilityInfoInner(params, info);
    GTEST_LOG_(INFO) << "AbilityRuntime_DumpAbilityInfoInner_0100 end";
}

/**
 * @tc.number: AbilityRuntime_DumpAbilityInfoInner_0200
 * @tc.name: DumpAbilityInfoInner
 * @tc.desc: Test DumpAbilityInfoInner function when currentAbility_ is nulllptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_DumpAbilityInfoInner_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_DumpAbilityInfoInner_0200 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    EXPECT_EQ(abilitythread->currentAbility_, nullptr);
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    std::vector<std::string> params;
    std::vector<std::string> info;
    abilitythread->DumpAbilityInfoInner(params, info);
    GTEST_LOG_(INFO) << "AbilityRuntime_DumpAbilityInfoInner_0200 end";
}

/**
 * @tc.number: AbilityRuntime_DumpOtherInfo_0100
 * @tc.name: DumpOtherInfo
 * @tc.desc: Test DumpOtherInfo function when abilityHandler_ and currentAbility_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_DumpOtherInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_DumpOtherInfo_0100 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockUIAbility";
    abilityInfo->type = AbilityType::PAGE;
    auto setRunner = EventRunner::Create(abilityInfo->name);
    abilitythread->abilityHandler_->SetEventRunner(setRunner);
    auto getRunner = abilitythread->abilityHandler_->GetEventRunner();
    EXPECT_NE(getRunner, nullptr);
    abilitythread->currentAbility_ = std::make_shared<AbilityRuntime::UIAbility>();
    EXPECT_NE(abilitythread->currentAbility_, nullptr);
    std::vector<std::string> info;
    abilitythread->DumpOtherInfo(info);
    GTEST_LOG_(INFO) << "AbilityRuntime_DumpOtherInfo_0100 end";
}

/**
 * @tc.number: AbilityRuntime_DumpOtherInfo_0200
 * @tc.name: DumpOtherInfo
 * @tc.desc: Test DumpOtherInfo function when abilityHandler_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_DumpOtherInfo_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_DumpOtherInfo_0200 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    EXPECT_EQ(abilitythread->abilityHandler_, nullptr);
    std::vector<std::string> info;
    abilitythread->DumpOtherInfo(info);
    GTEST_LOG_(INFO) << "AbilityRuntime_DumpOtherInfo_0200 end";
}

/**
 * @tc.number: AbilityRuntime_DumpOtherInfo_0300
 * @tc.name: DumpOtherInfo
 * @tc.desc: Test DumpOtherInfo function when currentAbility_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_DumpOtherInfo_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_DumpOtherInfo_0300 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    EXPECT_EQ(abilitythread->currentAbility_, nullptr);
    std::vector<std::string> info;
    abilitythread->DumpOtherInfo(info);
    GTEST_LOG_(INFO) << "AbilityRuntime_DumpOtherInfo_0300 end";
}

/**
 * @tc.number: AbilityRuntime_CallRequest_0100
 * @tc.name: CallRequest
 * @tc.desc: Test CallRequest function when abilityHandler_ and currentAbility_ is not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_CallRequest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_CallRequest_0100 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    std::vector<std::string> info;
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    abilitythread->currentAbility_ = std::make_shared<AbilityRuntime::UIAbility>();
    EXPECT_NE(abilitythread->currentAbility_, nullptr);
    abilitythread->CallRequest();
    GTEST_LOG_(INFO) << "AbilityRuntime_CallRequest_0100 end";
}

/**
 * @tc.number: AbilityRuntime_CallRequest_0200
 * @tc.name: CallRequest
 * @tc.desc: Test CallRequest function when abilityHandler_ and currentAbility_ is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_CallRequest_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_CallRequest_0200 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    std::vector<std::string> info;
    EXPECT_EQ(abilitythread->abilityHandler_, nullptr);
    EXPECT_EQ(abilitythread->currentAbility_, nullptr);
    abilitythread->CallRequest();
    GTEST_LOG_(INFO) << "AbilityRuntime_CallRequest_0200 end";
}

/**
 * @tc.number: AbilityRuntime_CreateAndInitContextDeal_0100
 * @tc.name: CreateAndInitContextDeal
 * @tc.desc: Test CreateAndInitContextDeal function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_CreateAndInitContextDeal_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_CreateAndInitContextDeal_0100 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockPageAbility";
    abilityInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    auto ret = abilitythread->CreateAndInitContextDeal(application, abilityRecord, nullptr);
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "AbilityRuntime_CreateAndInitContextDeal_0100 end";
}

/**
 * @tc.number: AbilityRuntime_CreateAndInitContextDeal_0200
 * @tc.name: CreateAndInitContextDeal
 * @tc.desc: Test CreateAndInitContextDeal function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_CreateAndInitContextDeal_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_CreateAndInitContextDeal_0200 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockPageAbility";
    abilityInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<AppExecFwk::AbilityContext> abilityObject = std::make_shared<AppExecFwk::AbilityContext>();
    auto ret = abilitythread->CreateAndInitContextDeal(application, abilityRecord, abilityObject);
    EXPECT_NE(ret, nullptr);
    GTEST_LOG_(INFO) << "AbilityRuntime_CreateAndInitContextDeal_0200 end";
}

/**
 * @tc.number: AbilityRuntime_CreateAndInitContextDeal_0300
 * @tc.name: CreateAndInitContextDeal
 * @tc.desc: Test CreateAndInitContextDeal function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_CreateAndInitContextDeal_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_CreateAndInitContextDeal_0300 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockPageAbility";
    abilityInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<AppExecFwk::AbilityContext> abilityObject = std::make_shared<AppExecFwk::AbilityContext>();
    auto ret = abilitythread->CreateAndInitContextDeal(nullptr, abilityRecord, abilityObject);
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "AbilityRuntime_CreateAndInitContextDeal_0300 end";
}

/**
 * @tc.number: AbilityRuntime_CreateAndInitContextDeal_0400
 * @tc.name: CreateAndInitContextDeal
 * @tc.desc: Test CreateAndInitContextDeal function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_CreateAndInitContextDeal_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_CreateAndInitContextDeal_0400 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockPageAbility";
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<AppExecFwk::AbilityContext> abilityObject = std::make_shared<AppExecFwk::AbilityContext>();
    auto ret = abilitythread->CreateAndInitContextDeal(application, nullptr, abilityObject);
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "AbilityRuntime_CreateAndInitContextDeal_0400 end";
}

/**
 * @tc.number: AbilityRuntime_Attach_0100
 * @tc.name: Attach
 * @tc.desc: Test Attach function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_Attach_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Attach_0100 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "";
    abilityInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
    abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
    auto ret = abilitythread->CreateAbilityName(abilityRecord);
    EXPECT_EQ(ret, "UIAbility");
    GTEST_LOG_(INFO) << "AbilityRuntime_Attach_0100 end";
}

/**
 * @tc.number: AbilityRuntime_HandleShareData_0100
 * @tc.name: HandleShareData
 * @tc.desc: Test HandleShareData function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_HandleShareData_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleShareData_0100 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->abilityHandler_ = nullptr;
    auto ret = abilitythread->SchedulePrepareTerminateAbility();
    int32_t uniqueId = 1;
    abilitythread->HandleShareData(uniqueId);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "AbilityRuntime_HandleShareData_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AddLifecycleEvent_0100
 * @tc.name: AddLifecycleEvent
 * @tc.desc: Test AddLifecycleEvent function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_AddLifecycleEvent_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AddLifecycleEvent_0100 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    uint32_t state = AAFwk::ABILITY_STATE_FOREGROUND_NEW;
    std::string methodName = "methodName";
    abilitythread->AddLifecycleEvent(state, methodName);
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    abilitythread->abilityHandler_ = nullptr;
    auto ret = abilitythread->SchedulePrepareTerminateAbility();
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "AbilityRuntime_AddLifecycleEvent_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AddLifecycleEvent_0200
 * @tc.name: AddLifecycleEvent
 * @tc.desc: Test AddLifecycleEvent function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_AddLifecycleEvent_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AddLifecycleEvent_0200 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    uint32_t state = AAFwk::ABILITY_STATE_BACKGROUND_NEW;
    std::string methodName = "methodName";
    abilitythread->AddLifecycleEvent(state, methodName);
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    abilitythread->abilityHandler_ = nullptr;
    int32_t uniqueId = 1;
    abilitythread->HandleShareData(uniqueId);
    auto ret = abilitythread->SchedulePrepareTerminateAbility();
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "AbilityRuntime_AddLifecycleEvent_0200 end";
}

/**
 * @tc.number: AbilityRuntime_ScheduleShareData_0100
 * @tc.name: ScheduleShareData
 * @tc.desc: Test ScheduleShareData function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_ScheduleShareData_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleShareData_0100 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->token_ = nullptr;
    EXPECT_EQ(abilitythread->token_, nullptr);
    int32_t uniqueId = 1;
    abilitythread->ScheduleShareData(uniqueId);
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    auto ret = abilitythread->SchedulePrepareTerminateAbility();
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleShareData_0100 end";
}

/**
 * @tc.number: AbilityRuntime_ScheduleShareData_0200
 * @tc.name: ScheduleShareData
 * @tc.desc: Test ScheduleShareData function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_ScheduleShareData_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleShareData_0200 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(abilitythread->token_, nullptr);
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    abilitythread->abilityHandler_ = nullptr;
    int32_t uniqueId = 1;
    abilitythread->ScheduleShareData(uniqueId);
    auto ret = abilitythread->SchedulePrepareTerminateAbility();
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleShareData_0200 end";
}

/**
 * @tc.number: AbilityRuntime_SendResult_0400
 * @tc.name: SendResult
 * @tc.desc: Test SendResult function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_SendResult_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_SendResult_0400 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->abilityHandler_ = nullptr;
    int requestCode = 0;
    int resultCode = 1;
    Want want;
    EXPECT_EQ(abilitythread->abilityHandler_, nullptr);
    abilitythread->SendResult(requestCode, resultCode, want);
    GTEST_LOG_(INFO) << "AbilityRuntime_SendResult_0400 end";
}

/**
 * @tc.number: AbilityRuntime_SendResult_0500
 * @tc.name: SendResult
 * @tc.desc: Test SendResult function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_SendResult_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_SendResult_0500 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    int requestCode = -1;
    int resultCode = 1;
    Want want;
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    abilitythread->SendResult(requestCode, resultCode, want);
    GTEST_LOG_(INFO) << "AbilityRuntime_SendResult_0500 end";
}

/**
 * @tc.number: AbilityRuntime_CallRequest_0400
 * @tc.name: CallRequest
 * @tc.desc: Test CallRequest function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_CallRequest_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_CallRequest_0400 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->currentAbility_ = std::make_shared<AbilityRuntime::UIAbility>();
    EXPECT_NE(abilitythread->currentAbility_, nullptr);
    EXPECT_EQ(abilitythread->abilityHandler_, nullptr);
    abilitythread->CallRequest();
    GTEST_LOG_(INFO) << "AbilityRuntime_CallRequest_0400 end";
}

/**
 * @tc.number: AbilityRuntime_OnExecuteIntent_0100
 * @tc.name: OnExecuteIntent
 * @tc.desc: Test OnExecuteIntent function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_OnExecuteIntent_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnExecuteIntent_0100 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    Want want;
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    EXPECT_EQ(abilitythread->abilityHandler_, nullptr);
    abilitythread->OnExecuteIntent(want);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnExecuteIntent_0100 end";
}

/**
 * @tc.number: AbilityRuntime_OnExecuteIntent_0300
 * @tc.name: OnExecuteIntent
 * @tc.desc: Test OnExecuteIntent function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_OnExecuteIntent_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnExecuteIntent_0300 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    Want want;
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    EXPECT_EQ(abilitythread->abilityHandler_, nullptr);
    abilitythread->OnExecuteIntent(want);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnExecuteIntent_0300 end";
}

/**
 * @tc.number: AbilityRuntime_OnExecuteIntent_0400
 * @tc.name: OnExecuteIntent
 * @tc.desc: Test OnExecuteIntent function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_OnExecuteIntent_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnExecuteIntent_0400 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    Want want;
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    abilitythread->OnExecuteIntent(want);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnExecuteIntent_0400 end";
}

/**
 * @tc.number: AbilityRuntime_HandlePrepareTerminateAbility_0100
 * @tc.name: HandlePrepareTerminateAbility
 * @tc.desc: Test HandlePrepareTerminateAbility function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_HandlePrepareTerminateAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_HandlePrepareTerminateAbility_0100 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    abilitythread->HandlePrepareTerminateAbility();
    GTEST_LOG_(INFO) << "AbilityRuntime_HandlePrepareTerminateAbility_0100 end";
}

/**
 * @tc.number: AbilityRuntime_HandlePrepareTerminateAbility_0200
 * @tc.name: HandlePrepareTerminateAbility
 * @tc.desc: Test HandlePrepareTerminateAbility function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_HandlePrepareTerminateAbility_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_HandlePrepareTerminateAbility_0200 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->HandlePrepareTerminateAbility();
    GTEST_LOG_(INFO) << "AbilityRuntime_HandlePrepareTerminateAbility_0200 end";
}

/**
 * @tc.number: AbilityRuntime_CreateModalUIExtension_0100
 * @tc.name: CreateModalUIExtension
 * @tc.desc: Test CreateModalUIExtension function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_CreateModalUIExtension_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_CreateModalUIExtension_0100 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    Want want;
    abilitythread->currentAbility_ = std::make_shared<AbilityRuntime::UIAbility>();
    EXPECT_NE(abilitythread->currentAbility_, nullptr);
    int ret = abilitythread->CreateModalUIExtension(want);
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    abilitythread->UpdateSessionToken(token);
    EXPECT_EQ(ret, CODE1);
    GTEST_LOG_(INFO) << "AbilityRuntime_CreateModalUIExtension_0100 end";
}

/**
 * @tc.number: AbilityRuntime_CreateModalUIExtension_0200
 * @tc.name: CreateModalUIExtension
 * @tc.desc: Test CreateModalUIExtension function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_CreateModalUIExtension_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_CreateModalUIExtension_0200 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    Want want;
    EXPECT_EQ(abilitythread->currentAbility_, nullptr);
    int ret = abilitythread->CreateModalUIExtension(want);
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    abilitythread->UpdateSessionToken(token);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AbilityRuntime_CreateModalUIExtension_0200 end";
}

/**
 * @tc.number: AbilityRuntime_ScheduleAbilityRequestFailure_0100
 * @tc.name: ScheduleAbilityRequestFailure
 * @tc.desc: Test ScheduleAbilityRequestFailure function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_ScheduleAbilityRequestFailure_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityRequestFailure_0100 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    std::string message = "failure";
    abilitythread->ScheduleAbilityRequestFailure(requestId, element, message);
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityRequestFailure_0100 end";
}

/**
 * @tc.number: AbilityRuntime_ScheduleAbilityRequestFailure_0200
 * @tc.name: ScheduleAbilityRequestFailure
 * @tc.desc: Test ScheduleAbilityRequestFailure function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_ScheduleAbilityRequestFailure_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityRequestFailure_0200 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    std::string message = "failure";
    abilitythread->ScheduleAbilityRequestFailure(requestId, element, message);
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityRequestFailure_0200 end";
}

/**
 * @tc.number: AbilityRuntime_ScheduleAbilityRequestSuccess_0100
 * @tc.name: ScheduleAbilityRequestSuccess
 * @tc.desc: Test ScheduleAbilityRequestSuccess function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_ScheduleAbilityRequestSuccess_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityRequestSuccess_0100 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    abilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(abilitythread->abilityHandler_, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    abilitythread->ScheduleAbilityRequestSuccess(requestId, element);
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityRequestSuccess_0100 end";
}

/**
 * @tc.number: AbilityRuntime_ScheduleAbilityRequestSuccess_0200
 * @tc.name: ScheduleAbilityRequestSuccess
 * @tc.desc: Test ScheduleAbilityRequestSuccess function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_ScheduleAbilityRequestSuccess_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityRequestSuccess_0200 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    EXPECT_EQ(abilitythread->abilityImpl_, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    abilitythread->ScheduleAbilityRequestSuccess(requestId, element);
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityRequestSuccess_0200 end";
}

/**
 * @tc.number: AbilityRuntime_InitNativeThread_0100
 * @tc.name: InitNativeThread
 * @tc.desc: Test InitNativeThread function when abilityInfo is nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_InitNativeThread_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_InitNativeThread_0100 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr;
    auto ret = abilitythread->InitNativeThread(abilityInfo);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "AbilityRuntime_InitNativeThread_0100 end";
}

/**
 * @tc.number: AbilityRuntime_InitNativeThread_0200
 * @tc.name: InitNativeThread
 * @tc.desc: Test InitNativeThread function when appContext is nullptr (no application context instance)
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_InitNativeThread_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_InitNativeThread_0200 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockUIAbility";
    abilityInfo->type = AbilityType::PAGE;
    auto ret = abilitythread->InitNativeThread(abilityInfo);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "AbilityRuntime_InitNativeThread_0200 end";
}

/**
 * @tc.number: AbilityRuntime_InitNativeThread_0300
 * @tc.name: InitNativeThread
 * @tc.desc: Test InitNativeThread function when abilityInfo has no native module (withNativeModule is false)
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_InitNativeThread_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_InitNativeThread_0300 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockUIAbility";
    abilityInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
    abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
    auto ret = abilitythread->InitNativeThread(abilityInfo);
    EXPECT_EQ(ret, true);
    GTEST_LOG_(INFO) << "AbilityRuntime_InitNativeThread_0300 end";
}

/**
 * @tc.number: AbilityRuntime_InitNativeThread_0400
 * @tc.name: InitNativeThread
 * @tc.desc: Test InitNativeThread function when currentAbility_ and abilityImpl_ are both nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_InitNativeThread_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_InitNativeThread_0400 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockUIAbility";
    abilityInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
    abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
    abilitythread->currentAbility_ = nullptr;
    abilitythread->abilityImpl_ = nullptr;
    auto ret = abilitythread->InitNativeThread(abilityInfo);
    EXPECT_EQ(ret, true);
    GTEST_LOG_(INFO) << "AbilityRuntime_InitNativeThread_0400 end";
}

/**
 * @tc.number: AbilityRuntime_InitNativeThread_0500
 * @tc.name: InitNativeThread
 * @tc.desc: Test InitNativeThread function when currentAbility_ and abilityImpl_ are not nullptr
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_InitNativeThread_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_InitNativeThread_0500 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    EXPECT_NE(abilitythread, nullptr);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockUIAbility";
    abilityInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
    abilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
    abilitythread->currentAbility_ = std::make_shared<AbilityRuntime::UIAbility>();
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    EXPECT_NE(abilitythread->currentAbility_, nullptr);
    EXPECT_NE(abilitythread->abilityImpl_, nullptr);
    auto ret = abilitythread->InitNativeThread(abilityInfo);
    EXPECT_EQ(ret, true);
    GTEST_LOG_(INFO) << "AbilityRuntime_InitNativeThread_0500 end";
}

#ifdef SUPPORT_SCREEN
/**
 * @tc.number: AbilityRuntime_GetUIAbility_0100
 * @tc.name: GetUIAbility
 * @tc.desc: Test GetUIAbility function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_GetUIAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_GetUIAbility_0100 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    ASSERT_NE(abilitythread, nullptr);
    ASSERT_EQ(abilitythread->abilityImpl_, nullptr);
    ASSERT_EQ(abilitythread->GetUIAbility(), nullptr);
    GTEST_LOG_(INFO) << "AbilityRuntime_GetUIAbility_0100 end";
}

/**
 * @tc.number: AbilityRuntime_GetUIAbility_0200
 * @tc.name: GetUIAbility
 * @tc.desc: Test GetUIAbility function
 */
HWTEST_F(UIAbilityThreadTest, AbilityRuntime_GetUIAbility_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_GetUIAbility_0200 start";
    AbilityRuntime::UIAbilityThread *abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    ASSERT_NE(abilitythread, nullptr);
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    ASSERT_EQ(abilitythread->GetUIAbility(), nullptr);
    GTEST_LOG_(INFO) << "AbilityRuntime_GetUIAbility_0200 end";
}
#endif
} // namespace AppExecFwk
} // namespace OHOS
