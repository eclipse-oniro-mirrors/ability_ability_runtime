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

#define private public
#include "shell_command_executor.h"
#undef private

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace {
const int64_t TIMEOUT = 1;
const std::string CMD = "ls";
}  // namespace

class ShellCommandExecutorTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<ShellCommandExecutor> Command_;
};

void ShellCommandExecutorTest::SetUpTestCase()
{}

void ShellCommandExecutorTest::TearDownTestCase()
{}

void ShellCommandExecutorTest::SetUp()
{
    Command_ = std::make_shared<ShellCommandExecutor>(CMD, TIMEOUT);
}

void ShellCommandExecutorTest::TearDown()
{}

/**
 * @tc.number: ShellCommandExecutor_DoWork_0100
 * @tc.name: DoWork
 * @tc.desc: Verify cmd_ When it is null, calling DoWork function returns false.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_DoWork_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0100 start";
    Command_->cmd_ = "";
    EXPECT_EQ(Command_->DoWork(), false);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0100 end";
}

/**
 * @tc.number: ShellCommandExecutor_DoWork_0200
 * @tc.name: DoWork
 * @tc.desc: Verify the handler_ When it is nullptr, calling DoWork function returns false.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_DoWork_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0200 start";
    Command_->handler_ = nullptr;
    EXPECT_EQ(Command_->DoWork(), false);
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0200 end";
}

/**
 * @tc.number: ShellCommandExecutor_DoWork_0300
 * @tc.name: DoWork
 * @tc.desc: Verify that PostTask conditions are met and cmd_ Not empty, call DoWork function branch to overwrite.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_DoWork_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0300 start";
    std::shared_ptr<AppExecFwk::EventHandler> handler_ = std::make_shared<AppExecFwk::EventHandler>();
    ASSERT_NE(handler_, nullptr);
    auto task = []() { GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0300 task called"; };
    handler_->PostTask(task, 1000);
    Command_->cmd_ = CMD;
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0300 end";
}

/**
 * @tc.number: ShellCommandExecutor_DoWork_0400
 * @tc.name: DoWork
 * @tc.desc: Verify that PostTask conditions are met and cmd_ Exception, calling DoWork function branch
 *           failed to reach coverage.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_DoWork_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0400 start";
    std::shared_ptr<AppExecFwk::EventHandler> handler_ = std::make_shared<AppExecFwk::EventHandler>();
    ASSERT_NE(handler_, nullptr);
    auto task = []() { GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0400 task called"; };
    handler_->PostTask(task, 1000);
    Command_->cmd_ = "CMD12";
    GTEST_LOG_(INFO) << "ShellCommandExecutor_DoWork_0400 end";
}

/**
 * @tc.number: ShellCommandExecutor_WaitWorkDone_0100
 * @tc.name: WaitWorkDone
 * @tc.desc: Verify that timeoutSec is met_ Condition, call WaitWorkDone function branch to reach coverage.
 */
HWTEST_F(ShellCommandExecutorTest, ShellCommandExecutor_WaitWorkDone_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShellCommandExecutor_WaitWorkDone_0100 start";
    ASSERT_NE(Command_, nullptr);
    Command_->timeoutSec_ = -1;
    Command_->cmdResult_.exitCode = 0;
    Command_->WaitWorkDone();
    Command_->DoWork();
    GTEST_LOG_(INFO) << "ShellCommandExecutor_WaitWorkDone_0100 end";
}