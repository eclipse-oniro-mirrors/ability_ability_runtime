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

#include "errors.h"

#define private public
#define protected public
#include "ability_handler.h"
#include "ability_info.h"
#include "ability_local_record.h"
#include "extension_impl.h"
#undef private
#undef protected
#include "event_runner.h"
#include "mock_ability_token.h"
#include "ohos_application.h"
#include "ui_extension_context.h"
#include "ui_extension.h"
#include "ui_extension_wrapper.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class ExtensionImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ExtensionImplTest::SetUpTestCase(void)
{
}

void ExtensionImplTest::TearDownTestCase(void)
{
}

void ExtensionImplTest::SetUp(void)
{
}

void ExtensionImplTest::TearDown(void)
{
}

/**
 * @tc.number: AaFwk_ExtensionImpl_0100
 * @tc.name: Init
 * @tc.desc: params is nullptr, Validation initialization failed.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_0100 start";
    std::shared_ptr<OHOSApplication> application;
    std::shared_ptr<AbilityLocalRecord> record;
    std::shared_ptr<AbilityRuntime::Extension> extension;
    std::shared_ptr<AppExecFwk::AbilityHandler> handler;
    sptr<IRemoteObject> token = nullptr;

    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->Init(application, record, extension, handler, token);
    EXPECT_TRUE(impl->token_ == nullptr);
    EXPECT_TRUE(impl->extension_ == nullptr);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_0100 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_0200
 * @tc.name: Init
 * @tc.desc: application is not null, others is null, Validation initialization failed.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_0200 start";
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AbilityLocalRecord> record;
    std::shared_ptr<AbilityRuntime::Extension> extension;
    std::shared_ptr<AppExecFwk::AbilityHandler> handler;
    sptr<IRemoteObject> token = nullptr;

    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->Init(application, record, extension, handler, token);
    EXPECT_TRUE(impl->token_ == nullptr);
    EXPECT_TRUE(impl->extension_ == nullptr);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_0200 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_0300
 * @tc.name: Init
 * @tc.desc: record is not null, others is null, Validation initialization failed.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_0300 start";
    std::shared_ptr<OHOSApplication> application;

    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);

    std::shared_ptr<AbilityRuntime::Extension> extension;
    std::shared_ptr<AppExecFwk::AbilityHandler> handler;

    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->Init(application, record, extension, handler, token);
    EXPECT_TRUE(impl->token_ == nullptr);
    EXPECT_TRUE(impl->extension_ == nullptr);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_0300 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_0400
 * @tc.name: Init
 * @tc.desc: token is not null, others is null, Validation initialization failed.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_0400 start";
    std::shared_ptr<OHOSApplication> application;
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    std::shared_ptr<AbilityLocalRecord> record;
    std::shared_ptr<AbilityRuntime::Extension> extension;
    std::shared_ptr<AppExecFwk::AbilityHandler> handler;

    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->Init(application, record, extension, handler, token);
    EXPECT_TRUE(impl->token_ == nullptr);
    EXPECT_TRUE(impl->extension_ == nullptr);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_0400 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_0500
 * @tc.name: Init
 * @tc.desc: handler is not null, others is null, Validation initialization failed.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_0500 start";
    std::shared_ptr<OHOSApplication> application;
    sptr<IRemoteObject> token = nullptr;
    std::shared_ptr<AbilityLocalRecord> record;
    std::shared_ptr<AbilityRuntime::Extension> extension;

    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);

    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->Init(application, record, extension, handler, token);
    EXPECT_TRUE(impl->token_ == nullptr);
    EXPECT_TRUE(impl->extension_ == nullptr);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_0500 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_0600
 * @tc.name: Init
 * @tc.desc: extension is not null, others is null, Validation initialization failed.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_0600 start";
    std::shared_ptr<OHOSApplication> application;
    sptr<IRemoteObject> token = nullptr;
    std::shared_ptr<AbilityLocalRecord> record;
    auto extension = std::make_shared<AbilityRuntime::Extension>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler;

    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->Init(application, record, extension, handler, token);
    EXPECT_TRUE(impl->token_ == nullptr);
    EXPECT_TRUE(impl->extension_ == nullptr);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_0600 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_0700
 * @tc.name: Init
 * @tc.desc: extension is not null, others is null, Validation initialization succeeded.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_0700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_0700 start";
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);
    auto extension = std::make_shared<AbilityRuntime::Extension>();
    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);

    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->Init(application, record, extension, handler, token);
    EXPECT_TRUE(impl->token_ != nullptr);
    EXPECT_TRUE(impl->extension_ != nullptr);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_0700 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_0800
 * @tc.name: NotifyMemoryLevel
 * @tc.desc: Validation NotifyMemoryLevel succeeded.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_0800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_0800 start";
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);
    auto extension = std::make_shared<AbilityRuntime::Extension>();
    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);

    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->Init(application, record, extension, handler, token);
    EXPECT_TRUE(impl->token_ != nullptr);
    EXPECT_TRUE(impl->extension_ != nullptr);

    impl->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    int32_t level = 0;
    impl->NotifyMemoryLevel(level);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_0800 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_0900
 * @tc.name: NotifyMemoryLevel
 * @tc.desc: Validation NotifyMemoryLevel succeeded.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_0900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_0900 start";
    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    int32_t level = 0;
    impl->NotifyMemoryLevel(level);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_0900 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_1000
 * @tc.name: Start
 * @tc.desc: extension is nullptr, Validation Start failed.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_1000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_1000 start";
    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    Want want;
    impl->Start(want);
    EXPECT_NE(impl->lifecycleState_, AAFwk::ABILITY_STATE_INACTIVE);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_1000 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_1100
 * @tc.name: Start
 * @tc.desc: extension is not nullptr, Validation Start succeeded.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_1100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_1100 start";
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);
    auto extension = std::make_shared<AbilityRuntime::Extension>();
    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);

    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->Init(application, record, extension, handler, token);

    Want want;
    impl->Start(want);
    EXPECT_EQ(impl->lifecycleState_, AAFwk::ABILITY_STATE_INACTIVE);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_1100 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_1200
 * @tc.name: Stop
 * @tc.desc: extension is not nullptr, Validation Stop succeeded.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_1200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_1200 start";
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);
    auto extension = std::make_shared<AbilityRuntime::Extension>();
    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);

    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->Init(application, record, extension, handler, token);
    impl->lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
    impl->Stop();
    EXPECT_EQ(impl->lifecycleState_, AAFwk::ABILITY_STATE_INITIAL);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_1200 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_1300
 * @tc.name: Stop
 * @tc.desc: extension is nullptr, Validation Stop failed.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_1300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_1300 start";
    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
    impl->Stop();
    EXPECT_NE(impl->lifecycleState_, AAFwk::ABILITY_STATE_INITIAL);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_1300 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_1400
 * @tc.name: ConnectExtension
 * @tc.desc: extension is nullptr, Validation ConnectExtension failed.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_1400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_1400 start";
    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
    Want want;
    auto object = impl->ConnectExtension(want);
    EXPECT_NE(impl->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    EXPECT_TRUE(object == nullptr);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_1400 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_1500
 * @tc.name: ConnectExtension
 * @tc.desc: extension is not nullptr, Validation ConnectExtension succeeded.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_1500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_1500 start";
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);
    auto extension = std::make_shared<AbilityRuntime::Extension>();
    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);

    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->Init(application, record, extension, handler, token);
    impl->lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
    Want want;
    auto object = impl->ConnectExtension(want);
    EXPECT_EQ(impl->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    EXPECT_TRUE(object == nullptr);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_1500 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_1600
 * @tc.name: DisconnectExtension
 * @tc.desc: extension is not nullptr, Validation DisconnectExtension succeeded.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_1600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_1600 start";
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);
    auto extension = std::make_shared<AbilityRuntime::Extension>();
    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);

    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->Init(application, record, extension, handler, token);
    Want want;
    impl->DisconnectExtension(want);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_1600 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_1700
 * @tc.name: DisconnectExtension
 * @tc.desc: extension is not nullptr, Validation DisconnectExtension failed.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_1700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_1700 start";
    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    Want want;
    impl->DisconnectExtension(want);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_1700 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_1800
 * @tc.name: DisconnectExtension
 * @tc.desc: extension is nullptr, Validation DisconnectExtension failed.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_1800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_1800 start";
    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    Want want;
    bool isAsyncCallback = true;
    impl->DisconnectExtension(want, isAsyncCallback);
    EXPECT_FALSE(isAsyncCallback);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_1800 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_1900
 * @tc.name: DisconnectExtension
 * @tc.desc: extension is nullptr, Validation DisconnectExtension failed.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_1900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_1900 start";
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);
    auto extension = std::make_shared<AbilityRuntime::Extension>();
    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);

    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->Init(application, record, extension, handler, token);

    Want want;
    bool isAsyncCallback = true;
    impl->DisconnectExtension(want, isAsyncCallback);
    EXPECT_FALSE(isAsyncCallback);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_1900 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_2000
 * @tc.name: DisconnectExtensionCallback
 * @tc.desc: Validation DisconnectExtensionCallback succeeded.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_2000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_2000 start";
    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->DisconnectExtensionCallback();
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_2000 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_2100
 * @tc.name: CommandExtension
 * @tc.desc: extension is nullptr, Validation CommandExtension failed.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_2100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_2100 start";
    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
    Want want;
    bool restart = false;
    int32_t startId = 0;
    impl->CommandExtension(want, restart, startId);
    EXPECT_EQ(impl->lifecycleState_, AAFwk::ABILITY_STATE_INACTIVE);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_2100 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_2200
 * @tc.name: CommandExtension
 * @tc.desc: extension is not nullptr, Validation CommandExtension succeeded.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_2200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_2200 start";
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);
    auto extension = std::make_shared<AbilityRuntime::Extension>();
    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);

    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->Init(application, record, extension, handler, token);
    impl->lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
    Want want;
    bool restart = false;
    int32_t startId = 0;
    impl->CommandExtension(want, restart, startId);
    EXPECT_EQ(impl->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_2200 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_2300
 * @tc.name: ScheduleUpdateConfiguration
 * @tc.desc: extension is not nullptr, Validation ScheduleUpdateConfiguration succeeded.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_2300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_2300 start";
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);
    auto extension = std::make_shared<AbilityRuntime::Extension>();
    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);

    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->Init(application, record, extension, handler, token);
    AppExecFwk::Configuration config;
    impl->ScheduleUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_2300 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_2400
 * @tc.name: ScheduleUpdateConfiguration
 * @tc.desc: extension is not nullptr, Validation ScheduleUpdateConfiguration failed.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_2400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_2400 start";
    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    AppExecFwk::Configuration config;
    impl->ScheduleUpdateConfiguration(config);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_2400 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_2600
 * @tc.name: HandleExtensionTransaction
 * @tc.desc: current state is init, target state is inactive, Validation HandleExtensionTransaction succeeded.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_2600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_2600 start";
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);
    auto extension = std::make_shared<AbilityRuntime::Extension>();
    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);

    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->Init(application, record, extension, handler, token);

    impl->lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    AAFwk::LifeCycleStateInfo targetState;
    targetState.state = AAFwk::ABILITY_STATE_INITIAL;
    Want want;
    impl->HandleExtensionTransaction(want, targetState);

    targetState.state = AAFwk::ABILITY_STATE_INACTIVE;
    impl->HandleExtensionTransaction(want, targetState);
    EXPECT_EQ(impl->lifecycleState_, AAFwk::ABILITY_STATE_INACTIVE);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_2600 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_2700
 * @tc.name: HandleExtensionTransaction
 * @tc.desc: current state is active, target state is init, Validation HandleExtensionTransaction succeeded.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_2700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_2700 start";
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);
    auto extension = std::make_shared<AbilityRuntime::Extension>();
    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);

    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->Init(application, record, extension, handler, token);
    impl->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    Want want;
    AAFwk::LifeCycleStateInfo targetState;
    targetState.state = AAFwk::ABILITY_STATE_INITIAL;
    impl->HandleExtensionTransaction(want, targetState);
    EXPECT_EQ(impl->lifecycleState_, AAFwk::ABILITY_STATE_INITIAL);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_2700 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_2800
 * @tc.name: HandleExtensionTransaction
 * @tc.desc: current state is active, target state is inactive, Validation HandleExtensionTransaction failed.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_2800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_2800 start";
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);
    auto extension = std::make_shared<AbilityRuntime::Extension>();
    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);

    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->Init(application, record, extension, handler, token);
    impl->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    Want want;
    AAFwk::LifeCycleStateInfo targetState;
    targetState.state = AAFwk::ABILITY_STATE_INACTIVE;
    impl->HandleExtensionTransaction(want, targetState);
    EXPECT_EQ(impl->lifecycleState_, AAFwk::ABILITY_STATE_ACTIVE);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_2800 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_2900
 * @tc.name: CommandExtensionWindow
 * @tc.desc: extension is nullptr, Validation CommandExtensionWindow failed.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_2900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_2900 start";
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);
    auto extension = std::make_shared<AbilityRuntime::Extension>();
    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);

    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->Init(application, record, extension, handler, token);
    sptr<AAFwk::SessionInfo> session = new (std::nothrow) AAFwk::SessionInfo();
    EXPECT_NE(session, nullptr);
    Want want;
    impl->CommandExtensionWindow(want, session, AAFwk::WIN_CMD_FOREGROUND);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_2900 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_3000
 * @tc.name: HandleInsightIntent
 * @tc.desc: Validation HandleInsightIntent succeeded.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_3000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_3000 start";
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);
    auto extension = std::make_shared<AbilityRuntime::Extension>();
    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);

    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->Init(application, record, extension, handler, token);
    Want want;
    impl->HandleInsightIntent(want);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_3000 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_3100
 * @tc.name: ScheduleAbilityRequestFailure
 * @tc.desc: Verify ScheduleAbilityRequestFailure succeeded.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_3100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_3100 start";
    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    ASSERT_NE(impl, nullptr);
    auto extension = std::make_shared<AbilityRuntime::Extension>();
    ASSERT_NE(extension, nullptr);
    impl->extension_ = extension;
    EXPECT_NE(impl->extension_, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    std::string message = "failure";
    impl->ScheduleAbilityRequestFailure(requestId, element, message);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_3100 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_3200
 * @tc.name: ScheduleAbilityRequestFailure
 * @tc.desc: Verify ScheduleAbilityRequestFailure succeeded.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_3200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_3200 start";
    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    ASSERT_NE(impl, nullptr);
    EXPECT_EQ(impl->extension_, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    std::string message = "failure";
    impl->ScheduleAbilityRequestFailure(requestId, element, message);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_3200 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_3300
 * @tc.name: ScheduleAbilityRequestSuccess
 * @tc.desc: Verify ScheduleAbilityRequestSuccess succeeded.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_3300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_3300 start";
    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    ASSERT_NE(impl, nullptr);
    auto extension = std::make_shared<AbilityRuntime::Extension>();
    ASSERT_NE(extension, nullptr);
    impl->extension_ = extension;
    EXPECT_NE(impl->extension_, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    impl->ScheduleAbilityRequestSuccess(requestId, element);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_3300 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_3400
 * @tc.name: ScheduleAbilityRequestSuccess
 * @tc.desc: Verify ScheduleAbilityRequestSuccess succeeded.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_3400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_3400 start";
    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    ASSERT_NE(impl, nullptr);
    EXPECT_EQ(impl->extension_, nullptr);
    std::string requestId = "1234567890";
    AppExecFwk::ElementName element("", "com.example.com", "MainAbility");
    impl->ScheduleAbilityRequestSuccess(requestId, element);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_3400 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_CreateModalUIExtension_0100
 * @tc.name: CreateModalUIExtension
 * @tc.desc: extensionType_ is not UI extension type, return ERR_INVALID_VALUE.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_CreateModalUIExtension_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_CreateModalUIExtension_0100 start";
    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->extensionType_ = AppExecFwk::ExtensionAbilityType::SERVICE;
    impl->extension_ = nullptr;
    AAFwk::Want want;
    int ret = impl->CreateModalUIExtension(want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_CreateModalUIExtension_0100 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_CreateModalUIExtension_0200
 * @tc.name: CreateModalUIExtension
 * @tc.desc: extension_ is nullptr, return ERR_INVALID_VALUE.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_CreateModalUIExtension_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_CreateModalUIExtension_0200 start";
    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->extensionType_ = AppExecFwk::ExtensionAbilityType::UI;
    impl->extension_ = nullptr;
    AAFwk::Want want;
    int ret = impl->CreateModalUIExtension(want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_CreateModalUIExtension_0200 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_CreateModalUIExtension_0400
 * @tc.name: CreateModalUIExtension
 * @tc.desc: UIExtension Init'd but handler has no EventRunner, PostTask fails, return ERR_INVALID_VALUE.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_CreateModalUIExtension_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_CreateModalUIExtension_0400 start";
    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->extensionType_ = AppExecFwk::ExtensionAbilityType::UI;

    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);
    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);

    auto uiExtension = std::make_shared<AbilityRuntime::UIExtension>();
    uiExtension->Init(record, application, handler, token);
    impl->extension_ = uiExtension;

    AAFwk::Want want;
    int ret = impl->CreateModalUIExtension(want);
    EXPECT_EQ(ret, -1);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_CreateModalUIExtension_0400 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_CreateModalUIExtension_0500
 * @tc.name: CreateModalUIExtension
 * @tc.desc: UIExtension::CreateModalUIExtension call failed, return error code.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_CreateModalUIExtension_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_CreateModalUIExtension_0500 start";
    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->extensionType_ = AppExecFwk::ExtensionAbilityType::UI;

    auto uiExtension = std::make_shared<AbilityRuntime::UIExtension>();
    impl->extension_ = uiExtension;

    AAFwk::Want want;
    int ret = impl->CreateModalUIExtension(want);
    EXPECT_EQ(ret, -1);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_CreateModalUIExtension_0500 end";
}
/**
 * @tc.number: AaFwk_ExtensionImpl_CreateModalUIExtension_0600
 * @tc.name: CreateModalUIExtension
 * @tc.desc: extensionType_ is UNSPECIFIED (not in UIExtension set), return ERR_INVALID_VALUE.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_CreateModalUIExtension_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_CreateModalUIExtension_0600 start";
    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->extensionType_ = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    auto extension = std::make_shared<AbilityRuntime::UIExtension>();
    impl->extension_ = extension;
    AAFwk::Want want;
    int ret = impl->CreateModalUIExtension(want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_CreateModalUIExtension_0600 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_CreateModalUIExtension_0700
 * @tc.name: CreateModalUIExtension
 * @tc.desc: extensionType_ is FORM (not in UIExtension set), extension_ is UIExtension, return ERR_INVALID_VALUE.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_CreateModalUIExtension_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_CreateModalUIExtension_0700 start";
    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->extensionType_ = AppExecFwk::ExtensionAbilityType::FORM;
    auto extension = std::make_shared<AbilityRuntime::UIExtension>();
    impl->extension_ = extension;
    AAFwk::Want want;
    int ret = impl->CreateModalUIExtension(want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_CreateModalUIExtension_0700 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_CreateModalUIExtension_0900
 * @tc.name: CreateModalUIExtension
 * @tc.desc: extensionType_ is EMBEDDED_UI (in UIExtension set), extension_ is nullptr, return ERR_INVALID_VALUE.
 *           This covers line 551-553 with a different UIExtension subtype.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_CreateModalUIExtension_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_CreateModalUIExtension_0900 start";
    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->extensionType_ = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    impl->extension_ = nullptr;
    AAFwk::Want want;
    int ret = impl->CreateModalUIExtension(want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_CreateModalUIExtension_0900 end";
}

/**
 * @tc.number: AaFwk_ExtensionImpl_CreateModalUIExtension_1000
 * @tc.name: CreateModalUIExtension
 * @tc.desc: extensionType_ is SHARE (in UIExtension set), extension_ is nullptr, return ERR_INVALID_VALUE.
 *           This covers line 551-553 with SHARE subtype.
 */
HWTEST_F(ExtensionImplTest, AaFwk_ExtensionImpl_CreateModalUIExtension_1000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_CreateModalUIExtension_1000 start";
    auto impl = std::make_shared<AbilityRuntime::ExtensionImpl>();
    impl->extensionType_ = AppExecFwk::ExtensionAbilityType::SHARE;
    impl->extension_ = nullptr;
    AAFwk::Want want;
    int ret = impl->CreateModalUIExtension(want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AaFwk_ExtensionImpl_CreateModalUIExtension_1000 end";
}

} // namespace AppExecFwk
} // namespace OHOS
