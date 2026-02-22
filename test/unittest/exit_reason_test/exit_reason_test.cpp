/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <memory>

#define private public
#define protected public
#include "exit_reason.h"
#undef private
#undef protected

#include "ability_config.h"
#include "ability_manager_errors.h"
#include "ability_scheduler.h"
#include "ability_util.h"
#include "bundlemgr/mock_bundle_manager.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_connect_callback.h"
#include "mock_sa_call.h"
#include "mock_task_handler_wrap.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include <thread>
#include <chrono>

using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using testing::_;
using testing::Return;

namespace {
    const int32_t SLEEP_TIME = 10000;
}
namespace OHOS {
namespace AAFwk {

class ExitReasonTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ExitReasonTest::SetUpTestCase(void)
{}
void ExitReasonTest::TearDownTestCase(void)
{}

void ExitReasonTest::SetUp()
{}
void ExitReasonTest::TearDown()
{}

/*
 * Feature: ExitReasonTest
 * Function: Unmarshalling
 * SubFunction: NA
 * FunctionPoints: Unmarshalling
 * EnvConditions:NA
 * CaseDescription: Verify the normal process of Unmarshalling
 */
HWTEST_F(ExitReasonTest, Unmarshalling_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "Unmarshalling_001 called. start");
    Reason reason = Reason::REASON_JS_ERROR;
    std::string exitMsg = "JsError";
    ExitReason info(reason, exitMsg);
    Parcel parcel;
    ExitReason * res = info.Unmarshalling(parcel);
    EXPECT_EQ(res, nullptr);
    TAG_LOGD(AAFwkTag::TEST, "Unmarshalling_001 called. end");
}

/*
 * Feature: ExitReasonTest
 * Function: Marshalling
 * SubFunction: NA
 * FunctionPoints: Marshalling
 * EnvConditions:NA
 * CaseDescription: Verify the normal process of Marshalling
 */
HWTEST_F(ExitReasonTest, Marshalling_001, TestSize.Level1)
{
    Reason reason = Reason::REASON_JS_ERROR;
    std::string exitMsg = "JsError";
    ExitReasonCompability info(reason, exitMsg);
    Parcel parcel;
    bool ret = info.Marshalling(parcel);
    EXPECT_TRUE(ret);
    ExitReasonCompability *res = info.Unmarshalling(parcel);
    EXPECT_NE(res, nullptr);
    EXPECT_EQ(res->reason, reason);
    EXPECT_EQ(res->exitMsg, exitMsg);
}
}  // namespace AAFwk
}  // namespace OHOS
