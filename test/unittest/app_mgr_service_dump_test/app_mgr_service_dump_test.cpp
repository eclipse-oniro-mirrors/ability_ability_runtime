/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "app_mgr_service.h"
#undef private
#include "hilog_tag_wrapper.h"
#include "mock_native_token.h"
#include "mock_sa_call.h"
#include "app_mgr_service_dump_error_code.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t USER_ID = 100;
} // namespace

class AppMgrServiceDumpTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppMgrServiceDumpTest::SetUpTestCase(void)
{
    MockNativeToken::SetNativeToken();
}

void AppMgrServiceDumpTest::TearDownTestCase(void)
{}

void AppMgrServiceDumpTest::SetUp()
{}

void AppMgrServiceDumpTest::TearDown()
{}

/**
 * @tc.name: AppMgrServiceDump_GetProcessRunningInfosByUserId_0100
 * @tc.desc: GetProcessRunningInfosByUserId
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppMgrServiceDumpTest, AppMgrServiceDump_GetProcessRunningInfosByUserId_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDump_GetProcessRunningInfosByUserId_0100 start");

    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::vector<RunningProcessInfo> info;
    auto result = appMgrServiceInner->GetProcessRunningInfosByUserId(info, USER_ID);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDump_GetProcessRunningInfosByUserId_0100 end");
}

/**
 * @tc.name: AppMgrServiceDump_GetProcessRunningInfosByUserId_0200
 * @tc.desc: GetProcessRunningInfosByUserId
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppMgrServiceDumpTest, AppMgrServiceDump_GetProcessRunningInfosByUserId_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDump_GetProcessRunningInfosByUserId_0200 start");

    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);

    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(
        appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);

    std::vector<RunningProcessInfo> info;
    auto result = appMgrService->GetProcessRunningInfosByUserId(info, USER_ID);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDump_GetProcessRunningInfosByUserId_0200 end");
}

/*
 * @tc.number    : AppMgrServiceDump_0100
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test dump interface
 */
HWTEST_F(AppMgrServiceDumpTest, AppMgrServiceDump_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDump_0100 start");

    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    EXPECT_NE(appMgrService->taskHandler_, nullptr);
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(
        appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg = Str8ToStr16("-h");
    args.emplace_back(arg);
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDump_0100 end");
}

/*
 * @tc.number    : AppMgrServiceDump_0200
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test dump interface
 */
HWTEST_F(AppMgrServiceDumpTest, AppMgrServiceDump_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDump_0200 start");

    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);

    constexpr int fd(0);
    std::vector<std::u16string> args;
    auto arg = Str8ToStr16("-h");
    args.emplace_back(arg);
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, ERR_APPEXECFWK_HIDUMP_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDump_0200 end");
}

/*
 * @tc.number    : AppMgrServiceDump_0300
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test dump interface
 */
HWTEST_F(AppMgrServiceDumpTest, AppMgrServiceDump_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDump_0300 start");

    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    EXPECT_NE(appMgrService->taskHandler_, nullptr);
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(
        appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDump_0300 end");
}

/*
 * @tc.number    : AppMgrServiceDump_0400
 * @tc.name      : AppMgrService dump
 * @tc.desc      : 1.Test dump interface
 */
HWTEST_F(AppMgrServiceDumpTest, AppMgrServiceDump_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDump_0400 start");

    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    EXPECT_NE(appMgrService->taskHandler_, nullptr);
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(
        appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);

    constexpr int fd(1);
    std::vector<std::u16string> args;
    auto arg = Str8ToStr16("-i");
    args.emplace_back(arg);
    auto result = appMgrService->Dump(fd, args);
    EXPECT_EQ(result, DumpErrorCode::ERR_UNKNOWN_OPTION_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceDump_0400 end");
}
}  // namespace AppExecFwk
}  // namespace OHOS
