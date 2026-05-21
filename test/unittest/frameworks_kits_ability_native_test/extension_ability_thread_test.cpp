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

#include <thread>
#include <functional>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#define private public
#define protected public
#include "ability_loader.h"
#include "ability_thread.h"
#include "extension_ability_thread.h"
#undef private
#undef protected
#include "mock_ability_thread.h"
#include "mock_ability_token.h"
#include "modular_object_worker_manager.h"
#include "ohos_application.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

static const int32_t STARTID = 0;

class ExtensionAbilityThreadTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void ExtensionAbilityThreadTest::SetUpTestCase(void) {}

void ExtensionAbilityThreadTest::TearDownTestCase(void) {}

void ExtensionAbilityThreadTest::SetUp(void) {}

void ExtensionAbilityThreadTest::TearDown(void) {}

/**
 * @tc.name: ExtensionAbilityThread_DumpAbilityInfo_0100
 * @tc.desc: DumpAbilityInfo
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_DumpAbilityInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "DumpAbilityInfo_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
        auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
        extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
        std::vector<std::string> params;
        std::vector<std::string> info;
        extensionabilitythread->DumpAbilityInfo(params, info);
        EXPECT_EQ(info.size(), 0);
        GTEST_LOG_(INFO) << "info:";
        for (auto item : info) {
            GTEST_LOG_(INFO) << item;
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfo_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_DumpAbilityInfo_0200
 * @tc.name: DumpAbilityInfo
 * @tc.desc: Test DumpAbilityInfo function when token_ and abilityHandler_ is
 * not nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_DumpAbilityInfo_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfo_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(extensionabilitythread->token_, nullptr);
    extensionabilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(extensionabilitythread->abilityHandler_, nullptr);
    std::vector<std::string> params;
    std::vector<std::string> info;
    extensionabilitythread->DumpAbilityInfo(params, info);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfo_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_DumpAbilityInfo_0300
 * @tc.name: DumpAbilityInfo
 * @tc.desc: Test DumpAbilityInfo function when abilityHandler_ is not nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_DumpAbilityInfo_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfo_0300 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(extensionabilitythread->token_, nullptr);
    EXPECT_EQ(extensionabilitythread->abilityHandler_, nullptr);
    std::vector<std::string> params;
    std::vector<std::string> info;
    extensionabilitythread->DumpAbilityInfo(params, info);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfo_0300 end";
}

/**
 * @tc.number: ExtensionAbilityThread_Attach_3_Param_0100
 * @tc.name: Attach
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_Attach_3_Param_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_Attach_3_Param_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_Attach_3_Param_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_Attach_3_Param_0200
 * @tc.name: Attach
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_Attach_3_Param_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_Attach_3_Param_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = nullptr;
            auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_Attach_3_Param_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_Attach_2_Param_0100
 * @tc.name: Attach
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_Attach_2_Param_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_Attach_2_Param_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
            extensionabilitythread->Attach(application, abilityRecord, nullptr);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_Attach_2_Param_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_Attach_2_Param_0200
 * @tc.name: Attach
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_Attach_2_Param_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_Attach_2_Param_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = nullptr;
            auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            extensionabilitythread->Attach(application, abilityRecord, nullptr);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_Attach_2_Param_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleAbilityTransaction_0100
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleAbilityTransaction_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionAbilityThread_ScheduleAbilityTransaction_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            Want want;
            LifeCycleStateInfo lifeCycleStateInfo;
            extensionabilitythread->ScheduleAbilityTransaction(want, lifeCycleStateInfo);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilityTransaction_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleAbilityTransaction_0200
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleAbilityTransaction_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilityTransaction_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
            extensionabilitythread->Attach(application, abilityRecord, nullptr);
            Want want;
            LifeCycleStateInfo lifeCycleStateInfo;
            extensionabilitythread->ScheduleAbilityTransaction(want, lifeCycleStateInfo);
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilityTransaction_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleAbilityTransaction_0300
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Test ScheduleAbilityTransaction function when token_ is not nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleAbilityTransaction_0300,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilityTransaction_0300 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(extensionabilitythread->token_, nullptr);
    Want want;
    LifeCycleStateInfo lifeCycleStateInfo;
    extensionabilitythread->ScheduleAbilityTransaction(want, lifeCycleStateInfo);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilityTransaction_0300 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleAbilityTransaction_0400
 * @tc.name: ScheduleAbilityTransaction
 * @tc.desc: Test ScheduleAbilityTransaction function when abilityHandler_ and
 * token_ is not nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleAbilityTransaction_0400,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilityTransaction_0400 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->token_ = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(extensionabilitythread->token_, nullptr);
    extensionabilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(extensionabilitythread->abilityHandler_, nullptr);
    Want want;
    LifeCycleStateInfo lifeCycleStateInfo;
    extensionabilitythread->ScheduleAbilityTransaction(want, lifeCycleStateInfo);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilityTransaction_0400 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleConnectAbility_0100
 * @tc.name: ScheduleConnectAbility
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleConnectAbility_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleConnectAbility_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            Want want;
            extensionabilitythread->ScheduleConnectAbility(want);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleConnectAbility_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleConnectAbility_0200
 * @tc.name: ScheduleConnectAbility
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleConnectAbility_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionAbilityThread_ScheduleConnectAbility_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
        auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
        extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
        Want want;
        extensionabilitythread->ScheduleConnectAbility(want);
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleConnectAbility_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleDisconnectAbility_0100
 * @tc.name: ScheduleDisconnectAbility
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleDisconnectAbility_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleDisconnectAbility_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            Want want;
            extensionabilitythread->ScheduleDisconnectAbility(want);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleDisconnectAbility_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleDisconnectAbility_0200
 * @tc.name: ScheduleDisconnectAbility
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleDisconnectAbility_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleDisconnectAbility_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
        auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
        extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
        Want want;
        extensionabilitythread->ScheduleDisconnectAbility(want);
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleDisconnectAbility_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleCommandAbility_0100
 * @tc.name: ScheduleCommandAbility
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleCommandAbility_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleCommandAbility_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        abilityInfo->isNativeAbility = true;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            Want want;
            bool restart = true;
            int startId = 0;
            extensionabilitythread->ScheduleCommandAbility(want, restart, startId);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleCommandAbility_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleCommandAbility_0200
 * @tc.name: ScheduleCommandAbility
 * @tc.desc: Validate when normally entering a string
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleCommandAbility_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleCommandAbility_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
        auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
        std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
        extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
        Want want;
        bool restart = true;
        int startId = 0;
        extensionabilitythread->ScheduleCommandAbility(want, restart, startId);
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleCommandAbility_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleCommandAbilityWindow_0100
 * @tc.name: ScheduleCommandAbilityWindow
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleCommandAbilityWindow_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleCommandAbilityWindow_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    if (extensionabilitythread != nullptr) {
        std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
        abilityInfo->name = "ExtensionAbility";
        abilityInfo->type = AbilityType::EXTENSION;
        sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
        EXPECT_NE(token, nullptr);
        if (token != nullptr) {
            std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
            auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
            std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
            extensionabilitythread->Attach(application, abilityRecord, mainRunner, nullptr);
            Want want;
            sptr<AAFwk::SessionInfo> session = new (std::nothrow) AAFwk::SessionInfo();
            extensionabilitythread->ScheduleCommandAbilityWindow(want, session, AAFwk::WIN_CMD_FOREGROUND);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleCommandAbilityWindow_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_HandleExtensionTransaction_0100
 * @tc.name: HandleExtensionTransaction
 * @tc.desc: Test HandleExtensionTransaction function when extensionImpl_ is
 * nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_HandleExtensionTransaction_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleExtensionTransaction_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    EXPECT_EQ(extensionabilitythread->extensionImpl_, nullptr);
    Want want;
    LifeCycleStateInfo lifeCycleStateInfo;
    extensionabilitythread->HandleExtensionTransaction(want, lifeCycleStateInfo);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleExtensionTransaction_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_HandleExtensionTransaction_0200
 * @tc.name: HandleExtensionTransaction
 * @tc.desc: Test HandleExtensionTransaction function when extensionImpl_ is not
 * nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_HandleExtensionTransaction_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleExtensionTransaction_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    Want want;
    LifeCycleStateInfo lifeCycleStateInfo;
    extensionabilitythread->HandleExtensionTransaction(want, lifeCycleStateInfo);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleExtensionTransaction_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_HandleConnectExtension_0100
 * @tc.name: HandleConnectExtension
 * @tc.desc: Test HandleConnectExtension function when extensionImpl_ is nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_HandleConnectExtension_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleConnectExtension_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    EXPECT_EQ(extensionabilitythread->extensionImpl_, nullptr);
    Want want;
    extensionabilitythread->HandleConnectExtension(want);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleConnectExtension_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_HandleConnectExtension_0200
 * @tc.name: HandleConnectExtension
 * @tc.desc: Test HandleConnectExtension function when extensionImpl_ is not
 * nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_HandleConnectExtension_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleConnectExtension_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    Want want;
    extensionabilitythread->HandleConnectExtension(want);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleConnectExtension_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_HandleCommandExtension_0100
 * @tc.name: HandleCommandExtension
 * @tc.desc: Test HandleCommandExtension function when extensionImpl_ is nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_HandleCommandExtension_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleCommandExtension_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    EXPECT_EQ(extensionabilitythread->extensionImpl_, nullptr);
    Want want;
    extensionabilitythread->HandleCommandExtension(want, false, STARTID);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleCommandExtension_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_HandleCommandExtension_0200
 * @tc.name: HandleCommandExtension
 * @tc.desc: Test HandleCommandExtension function when extensionImpl_ is not
 * nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_HandleCommandExtension_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleCommandExtension_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    Want want;
    extensionabilitythread->HandleCommandExtension(want, false, STARTID);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleCommandExtension_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_HandleCommandExtensionWindow_0100
 * @tc.name: HandleCommandExtensionWindow
 * @tc.desc: Test HandleCommandExtensionWindow function when extensionImpl_ is
 * nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_HandleCommandExtensionWindow_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleCommandExtensionWindow_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    Want want;
    sptr<AAFwk::SessionInfo> session = new (std::nothrow) AAFwk::SessionInfo();
    extensionabilitythread->HandleCommandExtensionWindow(want, session, AAFwk::WIN_CMD_FOREGROUND);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleCommandExtensionWindow_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleUpdateConfiguration_0100
 * @tc.name: ScheduleUpdateConfiguration
 * @tc.desc: Test ScheduleUpdateConfiguration function when abilityHandler_ is
 * nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleUpdateConfiguration_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleUpdateConfiguration_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    EXPECT_EQ(extensionabilitythread->abilityHandler_, nullptr);
    Configuration config;
    extensionabilitythread->ScheduleUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleUpdateConfiguration_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleUpdateConfiguration_0200
 * @tc.name: ScheduleUpdateConfiguration
 * @tc.desc: Test ScheduleUpdateConfiguration function when abilityHandler_ is
 * not nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleUpdateConfiguration_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleUpdateConfiguration_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(extensionabilitythread->abilityHandler_, nullptr);
    Configuration config;
    extensionabilitythread->ScheduleUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleUpdateConfiguration_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ExtensionUpdateConfiguration_0100
 * @tc.name: HandleExtensionUpdateConfiguration
 * @tc.desc: Test HandleExtensionUpdateConfiguration function when
 * extensionImpl_ is nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ExtensionUpdateConfiguration_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ExtensionUpdateConfiguration_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    EXPECT_EQ(extensionabilitythread->extensionImpl_, nullptr);
    Configuration config;
    extensionabilitythread->HandleExtensionUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ExtensionUpdateConfiguration_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ExtensionUpdateConfiguration_0200
 * @tc.name: HandleExtensionUpdateConfiguration
 * @tc.desc: Test HandleExtensionUpdateConfiguration function when
 * extensionImpl_ is not nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ExtensionUpdateConfiguration_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ExtensionUpdateConfiguration_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    Configuration config;
    extensionabilitythread->HandleExtensionUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ExtensionUpdateConfiguration_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_NotifyMemoryLevel_0100
 * @tc.name: NotifyMemoryLevel
 * @tc.desc: Test NotifyMemoryLevel function when isExtension_ is false and
 * abilityImpl_ is nullptr
 */
HWTEST_F(
    ExtensionAbilityThreadTest, ExtensionAbilityThread_NotifyMemoryLevel_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_NotifyMemoryLevel_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    EXPECT_EQ(extensionabilitythread->extensionImpl_, nullptr);
    int32_t result = STARTID;
    extensionabilitythread->NotifyMemoryLevel(result);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_NotifyMemoryLevel_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_NotifyMemoryLevel_0200
 * @tc.name: NotifyMemoryLevel
 * @tc.desc: Test NotifyMemoryLevel function when isExtension_ is false and
 * abilityImpl_ is not nullptr
 */
HWTEST_F(
    ExtensionAbilityThreadTest, ExtensionAbilityThread_NotifyMemoryLevel_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_NotifyMemoryLevel_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    int32_t result = STARTID;
    extensionabilitythread->NotifyMemoryLevel(result);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_NotifyMemoryLevel_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_NotifyMemoryLevel_0300
 * @tc.name: NotifyMemoryLevel
 * @tc.desc: Test NotifyMemoryLevel function when isExtension_ is true and
 * extensionImpl_ is nullptr
 */
HWTEST_F(
    ExtensionAbilityThreadTest, ExtensionAbilityThread_NotifyMemoryLevel_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_NotifyMemoryLevel_0300 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    EXPECT_EQ(extensionabilitythread->extensionImpl_, nullptr);
    int32_t result = STARTID;
    extensionabilitythread->NotifyMemoryLevel(result);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_NotifyMemoryLevel_0300 end";
}

/**
 * @tc.number: ExtensionAbilityThread_NotifyMemoryLevel_0400
 * @tc.name: NotifyMemoryLevel
 * @tc.desc: Test NotifyMemoryLevel function when isExtension_ is true and
 * extensionImpl_ is not nullptr
 */
HWTEST_F(
    ExtensionAbilityThreadTest, ExtensionAbilityThread_NotifyMemoryLevel_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_NotifyMemoryLevel_0400 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    int32_t result = STARTID;
    extensionabilitythread->NotifyMemoryLevel(result);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_NotifyMemoryLevel_0400 end";
}

/**
 * @tc.number: ExtensionAbilityThread_DumpAbilityInfoInner_0100
 * @tc.name: DumpAbilityInfoInner
 * @tc.desc: Test DumpAbilityInfoInner function when currentAbility_ and
 * currentExtension_ is not nullptr
 */
HWTEST_F(
    ExtensionAbilityThreadTest, ExtensionAbilityThread_DumpAbilityInfoInner_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfoInner_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->currentExtension_ = std::make_shared<AbilityRuntime::Extension>();
    EXPECT_NE(extensionabilitythread->currentExtension_, nullptr);
    extensionabilitythread->currentExtension_ = std::make_shared<AbilityRuntime::Extension>();
    EXPECT_NE(extensionabilitythread->currentExtension_, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    std::vector<std::string> params;
    std::vector<std::string> info;
    extensionabilitythread->DumpAbilityInfoInner(params, info);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfoInner_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_DumpAbilityInfoInner_0200
 * @tc.name: DumpAbilityInfoInner
 * @tc.desc: Test DumpAbilityInfoInner function when currentAbility_ is nullptr
 */
HWTEST_F(
    ExtensionAbilityThreadTest, ExtensionAbilityThread_DumpAbilityInfoInner_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfoInner_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    EXPECT_EQ(extensionabilitythread->currentExtension_, nullptr);
    extensionabilitythread->currentExtension_ = std::make_shared<AbilityRuntime::Extension>();
    EXPECT_NE(extensionabilitythread->currentExtension_, nullptr);
    std::vector<std::string> params;
    std::vector<std::string> info;
    extensionabilitythread->DumpAbilityInfoInner(params, info);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfoInner_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_DumpAbilityInfoInner_0300
 * @tc.name: DumpAbilityInfoInner
 * @tc.desc: Test DumpAbilityInfoInner function when currentExtension_ is
 * nullptr
 */
HWTEST_F(
    ExtensionAbilityThreadTest, ExtensionAbilityThread_DumpAbilityInfoInner_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfoInner_0300 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->currentExtension_ = std::make_shared<AbilityRuntime::Extension>();
    EXPECT_NE(extensionabilitythread->currentExtension_, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    std::vector<std::string> params;
    std::vector<std::string> info;
    extensionabilitythread->DumpAbilityInfoInner(params, info);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpAbilityInfoInner_0300 end";
}

/**
 * @tc.number: ExtensionAbilityThread_DumpOtherInfo_0100
 * @tc.name: DumpOtherInfo
 * @tc.desc: Test DumpOtherInfo function when abilityHandler_ and
 * currentAbility_ is not nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_DumpOtherInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpOtherInfo_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(extensionabilitythread->abilityHandler_, nullptr);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "ExtensionAbility";
    abilityInfo->type = AbilityType::EXTENSION;
    auto setRunner = EventRunner::Create(abilityInfo->name);
    extensionabilitythread->abilityHandler_->SetEventRunner(setRunner);
    auto getRunner = extensionabilitythread->abilityHandler_->GetEventRunner();
    EXPECT_NE(getRunner, nullptr);
    extensionabilitythread->currentExtension_ = std::make_shared<AbilityRuntime::Extension>();
    EXPECT_NE(extensionabilitythread->currentExtension_, nullptr);
    std::vector<std::string> info;
    extensionabilitythread->DumpOtherInfo(info);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpOtherInfo_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_DumpOtherInfo_0200
 * @tc.name: DumpOtherInfo
 * @tc.desc: Test DumpOtherInfo function when abilityHandler_ is nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_DumpOtherInfo_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpOtherInfo_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    EXPECT_EQ(extensionabilitythread->abilityHandler_, nullptr);
    std::vector<std::string> info;
    extensionabilitythread->DumpOtherInfo(info);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpOtherInfo_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_DumpOtherInfo_0300
 * @tc.name: DumpOtherInfo
 * @tc.desc: Test DumpOtherInfo function when currentAbility_ is nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_DumpOtherInfo_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpOtherInfo_0300 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(extensionabilitythread->abilityHandler_, nullptr);
    EXPECT_EQ(extensionabilitythread->currentExtension_, nullptr);
    std::vector<std::string> info;
    extensionabilitythread->DumpOtherInfo(info);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_DumpOtherInfo_0300 end";
}

/**
 * @tc.number: ExtensionAbilityThread_CreateAbilityName_0100
 * @tc.name: CreateAbilityName
 * @tc.desc: Test CreateAbilityName function when parameters are application,
 * mainRunner and abilityRecord
 */
HWTEST_F(
    ExtensionAbilityThreadTest, ExtensionAbilityThread_CreateAbilityName_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_CreateAbilityName_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "ExtensionAbility";
    abilityInfo->type = AbilityType::EXTENSION;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
    std::string abilityName = extensionabilitythread->CreateAbilityName(abilityRecord, application);
    auto extension = AbilityLoader::GetInstance().GetExtensionByName(abilityName,
        OHOS::AbilityRuntime::CODE_LANGUAGE_ARKTS_1_0);
    EXPECT_EQ(extension, nullptr);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_CreateAbilityName_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_HandleDisconnectExtension_0100
 * @tc.name: HandleDisconnectExtension
 * @tc.desc: Test HandleDisconnectExtension function when abilityImpl_ is not
 * nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_HandleDisconnectExtension_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleDisconnectExtension_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    Want want;
    extensionabilitythread->HandleDisconnectExtension(want);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_HandleDisconnectExtension_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleAbilitiesRequestDone_0100
 * @tc.name: ScheduleAbilitiesRequestDone
 * @tc.desc: Test ScheduleAbilitiesRequestDone function
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleAbilitiesRequestDone_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilitiesRequestDone_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    extensionabilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(extensionabilitythread->abilityHandler_, nullptr);
    std::string requestKey = "1234567890";
    int32_t resultCode = 123;
    extensionabilitythread->ScheduleAbilitiesRequestDone(requestKey, resultCode);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilitiesRequestDone_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleAbilitiesRequestDone_0200
 * @tc.name: ScheduleAbilitiesRequestDone
 * @tc.desc: Test ScheduleAbilitiesRequestDone function
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleAbilitiesRequestDone_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilitiesRequestDone_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    extensionabilitythread->abilityHandler_ = nullptr;
    std::string requestKey = "1234567890";
    int32_t resultCode = 123;
    extensionabilitythread->ScheduleAbilitiesRequestDone(requestKey, resultCode);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilitiesRequestDone_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleAbilityRequestFailure_0100
 * @tc.name: ScheduleAbilityRequestFailure
 * @tc.desc: Test ScheduleAbilityRequestFailure function
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleAbilityRequestFailure_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilityRequestFailure_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    extensionabilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(extensionabilitythread->abilityHandler_, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    std::string message = "failure";
    extensionabilitythread->ScheduleAbilityRequestFailure(requestId, element, message);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilityRequestFailure_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleAbilityRequestFailure_0200
 * @tc.name: ScheduleAbilityRequestFailure
 * @tc.desc: Test ScheduleAbilityRequestFailure function
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleAbilityRequestFailure_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilityRequestFailure_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    EXPECT_EQ(extensionabilitythread->extensionImpl_, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    std::string message = "failure";
    extensionabilitythread->ScheduleAbilityRequestFailure(requestId, element, message);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_ScheduleAbilityRequestFailure_0200 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleAbilityRequestSuccess_0100
 * @tc.name: ScheduleAbilityRequestSuccess
 * @tc.desc: Test ScheduleAbilityRequestSuccess function
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleAbilityRequestSuccess_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityRequestSuccess_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    extensionabilitythread->abilityHandler_ = std::make_shared<AbilityHandler>(nullptr);
    EXPECT_NE(extensionabilitythread->abilityHandler_, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    std::string message = "success";
    extensionabilitythread->ScheduleAbilityRequestSuccess(requestId, element);
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityRequestSuccess_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_ScheduleAbilityRequestSuccess_0200
 * @tc.name: ScheduleAbilityRequestSuccess
 * @tc.desc: Test ScheduleAbilityRequestSuccess function
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_ScheduleAbilityRequestSuccess_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityRequestSuccess_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    EXPECT_EQ(extensionabilitythread->extensionImpl_, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    std::string message = "success";
    extensionabilitythread->ScheduleAbilityRequestSuccess(requestId, element);
    GTEST_LOG_(INFO) << "AbilityRuntime_ScheduleAbilityRequestSuccess_0200 end";
}
/**
 * @tc.name  : ExtensionAbilityThread_HandleNormalExtensionAttach_0100
 * @tc.number: HandleNormalExtensionAttachTest_001
 * @tc.desc  : Test that when mainRunner is nullptr, the HandleNormalExtensionAttach function.
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_HandleNormalExtensionAttach_0100,
    Function | MediumTest | Level1) {
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "ContentEmbedExtensionAbility";
    abilityInfo->type = AbilityType::EXTENSION;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::string abilityName = "TestAbility";
 
    ExtensionAbilityThread thread;
    thread.HandleNormalExtensionAttach(abilityRecord, nullptr, abilityName);
 
    EXPECT_NE(thread.runner_, nullptr);
    EXPECT_NE(thread.abilityHandler_, nullptr);
}
 
/**
 * @tc.name  : ExtensionAbilityThread_HandleNormalExtensionAttach_0200
 * @tc.number: HandleNormalExtensionAttachTest_002
 * @tc.desc  : Test that when mainRunner not nullptr, the HandleNormalExtensionAttach function.
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_HandleNormalExtensionAttach_0200,
    Function | MediumTest | Level1) {
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "ContentEmbedExtensionAbility";
    abilityInfo->type = AbilityType::EXTENSION;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);
    std::string abilityName = "TestAbility";
 
    ExtensionAbilityThread thread;
    thread.HandleNormalExtensionAttach(abilityRecord, mainRunner, abilityName);
 
    EXPECT_NE(thread.abilityHandler_, nullptr);
}
 
/**
 * @tc.name  : ExtensionAbilityThread_CreateExtensionAbilityName_0100
 * @tc.number: CreateExtensionAbilityNameTest_001
 * @tc.desc  : test extensionAbilityType is STATICSUBSCRIBER
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_CreateExtensionAbilityName_0100,
    Function | MediumTest | Level1) {
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->extensionAbilityType = AppExecFwk::ExtensionAbilityType::STATICSUBSCRIBER;
    std::string abilityName;
 
    ExtensionAbilityThread thread;
    thread.CreateExtensionAbilityName(abilityInfo, abilityName);
 
    EXPECT_EQ(abilityName, "StaticSubscriberExtension");
}
 
/**
 * @tc.name  : ExtensionAbilityThread_CreateExtensionAbilityName_0200
 * @tc.number: CreateExtensionAbilityNameTest_002
 * @tc.desc  : test extensionAbilityType is DRIVER
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_CreateExtensionAbilityName_0200,
    Function | MediumTest | Level1) {
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->extensionAbilityType = AppExecFwk::ExtensionAbilityType::DRIVER;
    std::string abilityName;
 
    ExtensionAbilityThread thread;
    thread.CreateExtensionAbilityName(abilityInfo, abilityName);
 
    EXPECT_EQ(abilityName, "DriverExtension");
}
 
/**
 * @tc.name  : ExtensionAbilityThread_CreateExtensionAbilityName_0300
 * @tc.number: CreateExtensionAbilityNameTest_003
 * @tc.desc  : test extensionAbilityType is CONTENT_EMBED
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_CreateExtensionAbilityName_0300,
    Function | MediumTest | Level1) {
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->extensionAbilityType = AppExecFwk::ExtensionAbilityType::CONTENT_EMBED;
    std::string abilityName;
 
    ExtensionAbilityThread thread;
    thread.CreateExtensionAbilityName(abilityInfo, abilityName);
 
    EXPECT_EQ(abilityName, "ContentEmbedExtension");
}

/**
 * @tc.name  : ExtensionAbilityThread_HandleNativeExtensionAttach_0100
 * @tc.number: HandleNativeExtensionAttachTest_001
 * @tc.desc  : test contentEmbedEventRunner_ not nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_HandleNativeExtensionAttach_0100,
    Function | MediumTest | Level1) {
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "ContentEmbedExtensionAbility";
    abilityInfo->type = AbilityType::EXTENSION;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::string abilityName = "TestAbility";
    ExtensionAbilityThread thread;
    thread.HandleNativeExtensionAttach(abilityRecord, abilityName);
    EXPECT_NE(thread.contentEmbedEventRunner_, nullptr);
}

/**
 * @tc.name: GetOrCreateWorkerThread_ShouldReturnNonNullWhenFirstCreated
 * @tc.desc: Test creating a new worker thread returns valid handler
 */
HWTEST_F(ExtensionAbilityThreadTest,
    GetOrCreateWorkerThread_ShouldReturnNonNullWhenFirstCreated, Function | MediumTest | Level1)
{
    auto &mgr = ModularObjectWorkerManager::GetInstance();
    auto handler = mgr.GetOrCreateWorkerThread("test_bundle_TestExt");
    EXPECT_NE(handler, nullptr);
    mgr.ReleaseWorkerThread("test_bundle_TestExt");
}

/**
 * @tc.name: GetOrCreateWorkerThread_ShouldReturnSameHandlerWhenKeyExists
 * @tc.desc: Test reusing existing worker thread returns same handler
 */
HWTEST_F(ExtensionAbilityThreadTest,
    GetOrCreateWorkerThread_ShouldReturnSameHandlerWhenKeyExists, Function | MediumTest | Level1)
{
    auto &mgr = ModularObjectWorkerManager::GetInstance();
    auto handler1 = mgr.GetOrCreateWorkerThread("test_reuse_key");
    ASSERT_NE(handler1, nullptr);
    auto handler2 = mgr.GetOrCreateWorkerThread("test_reuse_key");
    EXPECT_EQ(handler1, handler2);
    mgr.ReleaseWorkerThread("test_reuse_key");
    mgr.ReleaseWorkerThread("test_reuse_key");
}

/**
 * @tc.name: GetOrCreateWorkerThread_ShouldReturnDifferentHandlerWhenKeyDiffers
 * @tc.desc: Test different keys create different handlers
 */
HWTEST_F(ExtensionAbilityThreadTest,
    GetOrCreateWorkerThread_ShouldReturnDifferentHandlerWhenKeyDiffers, Function | MediumTest | Level1)
{
    auto &mgr = ModularObjectWorkerManager::GetInstance();
    auto handler1 = mgr.GetOrCreateWorkerThread("test_key_A");
    auto handler2 = mgr.GetOrCreateWorkerThread("test_key_B");
    ASSERT_NE(handler1, nullptr);
    ASSERT_NE(handler2, nullptr);
    EXPECT_NE(handler1, handler2);
    mgr.ReleaseWorkerThread("test_key_A");
    mgr.ReleaseWorkerThread("test_key_B");
}

/**
 * @tc.name: ReleaseWorkerThread_ShouldRemoveHandlerWhenRefCountReachesZero
 * @tc.desc: Test releasing all refs removes handler, next create gives new instance
 */
HWTEST_F(ExtensionAbilityThreadTest,
    ReleaseWorkerThread_ShouldRemoveHandlerWhenRefCountReachesZero, Function | MediumTest | Level1)
{
    auto &mgr = ModularObjectWorkerManager::GetInstance();
    auto handler1 = mgr.GetOrCreateWorkerThread("test_release_key");
    ASSERT_NE(handler1, nullptr);
    mgr.ReleaseWorkerThread("test_release_key");
    auto handler2 = mgr.GetOrCreateWorkerThread("test_release_key");
    ASSERT_NE(handler2, nullptr);
    EXPECT_NE(handler1, handler2);
    mgr.ReleaseWorkerThread("test_release_key");
}

/**
 * @tc.name: ReleaseWorkerThread_ShouldNotCrashWhenKeyNotFound
 * @tc.desc: Test releasing non-existent key does not crash
 */
HWTEST_F(ExtensionAbilityThreadTest,
    ReleaseWorkerThread_ShouldNotCrashWhenKeyNotFound, Function | MediumTest | Level1)
{
    auto &mgr = ModularObjectWorkerManager::GetInstance();
    mgr.ReleaseWorkerThread("non_existent_key");
    EXPECT_TRUE(true);
}

/**
 * @tc.name: ReleaseWorkerThread_ShouldKeepHandlerWhenPartialRelease
 * @tc.desc: Test ref counting: partial release keeps handler, full release removes it
 */
HWTEST_F(ExtensionAbilityThreadTest,
    ReleaseWorkerThread_ShouldKeepHandlerWhenPartialRelease, Function | MediumTest | Level1)
{
    auto &mgr = ModularObjectWorkerManager::GetInstance();
    auto handler1 = mgr.GetOrCreateWorkerThread("test_refcount_key");
    ASSERT_NE(handler1, nullptr);
    auto handler2 = mgr.GetOrCreateWorkerThread("test_refcount_key");
    EXPECT_EQ(handler1, handler2);
    mgr.ReleaseWorkerThread("test_refcount_key");
    auto handler3 = mgr.GetOrCreateWorkerThread("test_refcount_key");
    EXPECT_EQ(handler1, handler3);
    mgr.ReleaseWorkerThread("test_refcount_key");
    mgr.ReleaseWorkerThread("test_refcount_key");
}

/**
 * @tc.name: GenerateInstanceId_ShouldReturnMonotonicallyIncreasingIds
 * @tc.desc: Test instance IDs are unique and monotonically increasing
 */
HWTEST_F(ExtensionAbilityThreadTest,
    GenerateInstanceId_ShouldReturnMonotonicallyIncreasingIds, Function | MediumTest | Level1)
{
    auto &mgr = ModularObjectWorkerManager::GetInstance();
    uint32_t id1 = mgr.GenerateInstanceId();
    uint32_t id2 = mgr.GenerateInstanceId();
    uint32_t id3 = mgr.GenerateInstanceId();
    EXPECT_LT(id1, id2);
    EXPECT_LT(id2, id3);
}

// Test extension that overrides GetAbilityHandler to provide a custom handler
class TestHandlerExtension : public AbilityRuntime::Extension {
public:
    TestHandlerExtension() = default;
    ~TestHandlerExtension() override = default;

    std::shared_ptr<AppExecFwk::AbilityHandler> GetAbilityHandler(
        const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo) override
    {
        if (abilityInfo == nullptr) {
            return nullptr;
        }
        auto runner = AppExecFwk::EventRunner::Create("TestHandlerThread");
        return std::make_shared<AppExecFwk::AbilityHandler>(runner);
    }
};

/**
 * @tc.name: HandleAttach_ShouldUseExtensionHandlerWhenOverrideReturnsNonNull
 * @tc.desc: Test HandleAttach uses extension-provided handler via GetAbilityHandler
 */
HWTEST_F(ExtensionAbilityThreadTest,
    HandleAttach_ShouldUseExtensionHandlerWhenOverrideReturnsNonNull, Function | MediumTest | Level1)
{
    AppExecFwk::AbilityLoader::GetInstance().RegisterExtension("TestHandlerExtension",
        [](const std::string &) -> AbilityRuntime::Extension * {
            return new (std::nothrow) TestHandlerExtension();
        });

    sptr<AbilityRuntime::ExtensionAbilityThread> thread = sptr<AbilityRuntime::ExtensionAbilityThread>::MakeSptr();
    ASSERT_NE(thread, nullptr);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "TestHandlerExtension";
    abilityInfo->type = AbilityType::EXTENSION;
    abilityInfo->isNativeAbility = true;
    abilityInfo->extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;

    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    ASSERT_NE(token, nullptr);

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);

    thread->Attach(application, abilityRecord, mainRunner, nullptr);
    EXPECT_NE(thread->abilityHandler_, nullptr);
}

/**
 * @tc.name: HandleAttach_ShouldFallbackToDefaultWhenOverrideReturnsNull
 * @tc.desc: Test HandleAttach falls back to default when GetAbilityHandler returns nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest,
    HandleAttach_ShouldFallbackToDefaultWhenOverrideReturnsNull, Function | MediumTest | Level1)
{
    AppExecFwk::AbilityLoader::GetInstance().RegisterExtension("DefaultHandlerExtension",
        [](const std::string &) -> AbilityRuntime::Extension * {
            return new (std::nothrow) AbilityRuntime::Extension();
        });

    sptr<AbilityRuntime::ExtensionAbilityThread> thread = sptr<AbilityRuntime::ExtensionAbilityThread>::MakeSptr();
    ASSERT_NE(thread, nullptr);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "DefaultHandlerExtension";
    abilityInfo->type = AbilityType::EXTENSION;
    abilityInfo->isNativeAbility = true;
    abilityInfo->extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;

    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    ASSERT_NE(token, nullptr);

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(abilityInfo->name);

    thread->Attach(application, abilityRecord, mainRunner, nullptr);
    EXPECT_NE(thread->abilityHandler_, nullptr);
}

/**
 * @tc.number: ExtensionAbilityThread_CreateModalUIExtension_0100
 * @tc.name: CreateModalUIExtension
 * @tc.desc: Test CreateModalUIExtension function when extensionImpl_ is nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_CreateModalUIExtension_0100,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_CreateModalUIExtension_0100 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    EXPECT_EQ(extensionabilitythread->extensionImpl_, nullptr);
    AAFwk::Want want;
    int result = extensionabilitythread->CreateModalUIExtension(want);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_CreateModalUIExtension_0100 end";
}

/**
 * @tc.number: ExtensionAbilityThread_CreateModalUIExtension_0200
 * @tc.name: CreateModalUIExtension
 * @tc.desc: Test CreateModalUIExtension function when extensionImpl_ is not nullptr
 */
HWTEST_F(ExtensionAbilityThreadTest, ExtensionAbilityThread_CreateModalUIExtension_0200,
    Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_CreateModalUIExtension_0200 start";
    AbilityRuntime::ExtensionAbilityThread *extensionabilitythread =
        new (std::nothrow) AbilityRuntime::ExtensionAbilityThread();
    EXPECT_NE(extensionabilitythread, nullptr);
    extensionabilitythread->extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    EXPECT_NE(extensionabilitythread->extensionImpl_, nullptr);
    AAFwk::Want want;
    int result = extensionabilitythread->CreateModalUIExtension(want);
    // ExtensionImpl::CreateModalUIExtension returns ERR_INVALID_VALUE when extension is not UIExtension
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "ExtensionAbilityThread_CreateModalUIExtension_0200 end";
}
} // namespace AbilityRuntime
} // namespace OHOS