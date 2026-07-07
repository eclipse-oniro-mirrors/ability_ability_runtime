/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>
 
#define private public
#include "appcapture_perf.h"
#include "main_thread.h"
#undef private
#include "cpp/mutex.h"
#include "lperf.h"
 
using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
 
namespace OHOS {
namespace AppExecFwk {
class AppCapturePerfTest : public testing::Test {
public:
    AppCapturePerfTest()
    {}
    ~AppCapturePerfTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<MainThread> mainThread_ = nullptr;
};
 
void AppCapturePerfTest::SetUpTestCase(void)
{}
 
void AppCapturePerfTest::TearDownTestCase(void)
{}
 
void AppCapturePerfTest::SetUp(void)
{
    mainThread_ = sptr<MainThread>(new (std::nothrow) MainThread());
}
 
void AppCapturePerfTest::TearDown(void)
{}
 
/**
 * @tc.number: AppCapturePerfTest001
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppCapturePerfTest, AppCapturePerfTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "test AppCapturePerfTest001.\n";
    FaultData faultData;
    faultData.errorObject.name = "testapp";
    faultData.errorObject.message = "test";
    faultData.errorObject.stack = "";
    int32_t ret = AppCapturePerf::GetInstance().CapturePerf(faultData);
    EXPECT_EQ(ret, -1);
}
 
/**
 * @tc.number: AppCapturePerfTest002
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppCapturePerfTest, AppCapturePerfTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "test AppCapturePerfTest002.\n";
    FaultData faultData;
    faultData.errorObject.name = "testapp";
    faultData.errorObject.message = "test";
    faultData.errorObject.stack = "123,,1478";
    faultData.timeoutMarkers = "123456";
    int32_t ret = AppCapturePerf::GetInstance().CapturePerf(faultData);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: AppCapturePerfTest004
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppCapturePerfTest, AppCapturePerfTest004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "test AppCapturePerfTest004.\n";
    std::string input = "";
    char delimiter = ',';
    std::vector<std::string> result = AppCapturePerf::GetInstance().SplitStr(input, delimiter);
    EXPECT_TRUE(result.empty());
}
 
/**
 * @tc.number: AppCapturePerfTest005
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppCapturePerfTest, AppCapturePerfTest005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "test AppCapturePerfTest005.\n";
    std::string input = "single";
    char delimiter = ',';
    std::vector<std::string> result = AppCapturePerf::GetInstance().SplitStr(input, delimiter);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "single");
}
 
/**
 * @tc.number: AppCapturePerfTest006
 * @tc.desc: add testcase codecoverage
 * @tc.type: FUNC
 */
HWTEST_F(AppCapturePerfTest, AppCapturePerfTest006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "test AppCapturePerfTest006.\n";
    std::string input = "apple,banana,cherry";
    char delimiter = ',';
    std::vector<std::string> result = AppCapturePerf::GetInstance().SplitStr(input, delimiter);
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "apple");
    EXPECT_EQ(result[1], "banana");
    EXPECT_EQ(result[2], "cherry");
}
 
/**
 * @tc.name: ScheduleNotifyAppFault_0102
 * @tc.desc: Schedule notify app Fault.
 * @tc.type: FUNC
 * @tc.require: issueI79RY8
 */
HWTEST_F(AppCapturePerfTest, ScheduleNotifyAppFault_0102, TestSize.Level1)
{
    FaultData faultData;
    faultData.faultType = FaultDataType::RESOURCE_CONTROL;
    faultData.errorObject.message = "msgContent";
    faultData.errorObject.stack = "stack";
    faultData.errorObject.name = "eventType";
    const std::shared_ptr<EventRunner> runner;
    const sptr<MainThread> thread;
    mainThread_->mainHandler_ = std::make_shared<MainThread::MainHandler>(runner, thread);
    auto ret = mainThread_->ScheduleNotifyAppFault(faultData);
    EXPECT_EQ(ret, NO_ERROR);
}
}  // namespace AppExecFwk
}  // namespace OHOS