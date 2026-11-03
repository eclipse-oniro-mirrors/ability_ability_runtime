/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "app_mgr_service_inner.h"
#include "app_utils.h"
#include "hilog_tag_wrapper.h"
#include "mock_my_flag.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppMgrServiceInnerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppMgrServiceInnerTest::SetUpTestCase(void)
{}

void AppMgrServiceInnerTest::TearDownTestCase(void)
{}

void AppMgrServiceInnerTest::SetUp()
{}

void AppMgrServiceInnerTest::TearDown()
{}

/**
 * @tc.name: SetStartMsgCustomSandboxFlag_0100
 * @tc.type: FUNC
 * @tc.Function: SetStartMsgCustomSandboxFlag
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SetStartMsgCustomSandboxFlag_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetStartMsgCustomSandboxFlag_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    AppSpawnStartMsg startMsg = {};
    uint32_t accessTokenId = MOCKTOKENID::TOKENID_TWO;
    appMgrServiceInner->SetStartMsgCustomSandboxFlag(startMsg, accessTokenId);
    EXPECT_FALSE(startMsg.flags & (1ULL << StartFlags::CUSTOM_SANDBOX));

    AAFwk::AppUtils::isStartOptionsWithAnimation_ = true;
    appMgrServiceInner->SetStartMsgCustomSandboxFlag(startMsg, accessTokenId);
    EXPECT_FALSE(startMsg.flags & (1ULL << StartFlags::CUSTOM_SANDBOX));

    accessTokenId = MOCKTOKENID::TOKENID_ONE;
    appMgrServiceInner->SetStartMsgCustomSandboxFlag(startMsg, accessTokenId);
    EXPECT_TRUE(startMsg.flags & (1ULL << StartFlags::CUSTOM_SANDBOX));
    TAG_LOGI(AAFwkTag::TEST, "SetStartMsgCustomSandboxFlag_0100 end");
}
} // namespace AppExecFwk
} // namespace OHOS