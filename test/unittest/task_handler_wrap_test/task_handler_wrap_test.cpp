/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "task_handler_wrap.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
constexpr int32_t LONG_TIME_TASK_TIME = 2 * 500000 + 100000; // us
constexpr int32_t SCHEDULE_TIMEOUT = 500; // ms
class TaskHandlerWrapTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<TaskHandlerWrap> queueHandler_;
    std::shared_ptr<TaskHandlerWrap> ffrtHandler_;
};

void TaskHandlerWrapTest::SetUpTestCase(void)
{}
void TaskHandlerWrapTest::TearDownTestCase(void)
{}
void TaskHandlerWrapTest::SetUp()
{
    queueHandler_ = TaskHandlerWrap::CreateQueueHandler("TaskHandlerWrapTest");
    ffrtHandler_ = TaskHandlerWrap::GetFfrtHandler();
}
void TaskHandlerWrapTest::TearDown()
{
    queueHandler_.reset();
    ffrtHandler_.reset();
}

/**
 * @tc.name: QueueTest_0010
 * @tc.desc: SubmitTask Test
 * @tc.type: FUNC
 */
HWTEST_F(TaskHandlerWrapTest, QueueTest_0010, TestSize.Level0)
{
    int input = 0;
    auto taskHandle = queueHandler_->SubmitTask([&input]() {
        input = 1;
        });
    EXPECT_TRUE(taskHandle);
    taskHandle.Sync();
    EXPECT_TRUE(input == 1);
}

/**
 * @tc.name: QueueTest_0020
 * @tc.desc: SubmitTask with delay Test
 * @tc.type: FUNC
 */
HWTEST_F(TaskHandlerWrapTest, QueueTest_0020, TestSize.Level0)
{
    int input = 0;
    auto taskHandle = queueHandler_->SubmitTask([&input]() {
        input = 1;
        }, 100);
    EXPECT_TRUE(taskHandle);
    taskHandle.Sync();
    EXPECT_TRUE(input == 1);
}

/**
 * @tc.name: QueueTest_0030
 * @tc.desc: SubmitTask Test
 * @tc.type: FUNC
 */
HWTEST_F(TaskHandlerWrapTest, QueueTest_0030, TestSize.Level0)
{
    auto taskHandle = queueHandler_->SubmitTask([]() {});
    EXPECT_TRUE(taskHandle);
    auto result = taskHandle.Cancel();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: QueueTest_0040
 * @tc.desc: SubmitTask with delay Test
 * @tc.type: FUNC
 */
HWTEST_F(TaskHandlerWrapTest, QueueTest_0040, TestSize.Level0)
{
    auto taskHandle = queueHandler_->SubmitTask([]() {}, 100);
    EXPECT_TRUE(taskHandle);
    auto result = taskHandle.Cancel();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: QueueTest_0050
 * @tc.desc: SubmitTask time task test
 * @tc.type: FUNC
 */
HWTEST_F(TaskHandlerWrapTest, QueueTest_0050, TestSize.Level0)
{
    int input = 0;
    auto taskHandle = queueHandler_->SubmitTask([&input]() {
            usleep(LONG_TIME_TASK_TIME);
            input = 1;
        }, 100);
    EXPECT_TRUE(taskHandle);
    taskHandle.Sync();
    EXPECT_TRUE(input == 1);
}

/**
 * @tc.name: QueueTest_0060
 * @tc.desc: Insert task test
 * @tc.type: FUNC
 */
HWTEST_F(TaskHandlerWrapTest, QueueTest_0060, TestSize.Level0)
{
    queueHandler_->SetPrintTaskLog(true);
    int input = 0;
    auto task1 = [&input]() {
        usleep(LONG_TIME_TASK_TIME);
        input = 1;
    };
    auto handle1 = queueHandler_->SubmitTask(task1, "task1");

    // this task will trigger scheduling timeout
    auto handle2 = queueHandler_->SubmitTask([](){}, TaskAttribute{
        .taskName_ = "task2",
        .timeoutMillis_ = SCHEDULE_TIMEOUT
    });

    int result3 = 0;
    int result4 = 0;
    auto handle3 = queueHandler_->SubmitTask([&input, &result3]() {
            result3 = ++input;
        }, "task3");
    auto handle4 = queueHandler_->SubmitTask([&input, &result4]() {
            result4 = ++input;
        }, TaskAttribute{
            .taskName_ = "task4",
            .insertHead_ = true
        });
    handle1.Sync();
    handle2.Sync();
    handle3.Sync();
    handle4.Sync();
    EXPECT_TRUE(result3 == result4 + 1);
}

/**
 * @tc.name: FfrtTest_0010
 * @tc.desc: SubmitTask Test
 * @tc.type: FUNC
 */
HWTEST_F(TaskHandlerWrapTest, FfrtTest_0010, TestSize.Level0)
{
    int input = 0;
    auto taskHandle = ffrtHandler_->SubmitTask([&input]() {
        input = 1;
        });
    EXPECT_TRUE(taskHandle);
    taskHandle.Sync();
    EXPECT_TRUE(input == 1);
}

/**
 * @tc.name: FfrtTest_0020
 * @tc.desc: SubmitTask with delay Test
 * @tc.type: FUNC
 */
HWTEST_F(TaskHandlerWrapTest, FfrtTest_0020, TestSize.Level0)
{
    int input = 0;
    auto taskHandle = ffrtHandler_->SubmitTask([&input]() {
        input = 1;
        }, 100);
    EXPECT_TRUE(taskHandle);
    taskHandle.Sync();
    EXPECT_TRUE(input == 1);
}

/**
 * @tc.name: FfrtTest_0030
 * @tc.desc: SubmitTask Test
 * @tc.type: FUNC
 */
HWTEST_F(TaskHandlerWrapTest, FfrtTest_0030, TestSize.Level0)
{
    auto taskHandle = ffrtHandler_->SubmitTask([]() {});
    EXPECT_TRUE(taskHandle);
    auto result = taskHandle.Cancel();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: FfrtTest_0040
 * @tc.desc: SubmitTask with delay Test
 * @tc.type: FUNC
 */
HWTEST_F(TaskHandlerWrapTest, FfrtTest_0040, TestSize.Level0)
{
    auto taskHandle = ffrtHandler_->SubmitTask([]() {}, 100);
    EXPECT_TRUE(taskHandle);
    auto result = taskHandle.Cancel();
    EXPECT_TRUE(result);
}

}  // namespace AAFwk
}  // namespace OHOS
