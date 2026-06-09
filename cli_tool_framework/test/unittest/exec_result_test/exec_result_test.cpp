/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <parcel.h>

#include "exec_result.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t TEST_EXIT_CODE = 7;
constexpr int64_t TEST_TIMEOUT = 3000;
}

class ExecResultTest : public testing::Test {};

/**
 * @tc.name: ExecResult_Parcelable_0100
 * @tc.desc: Test ExecResult marshalling and unmarshalling success path
 * @tc.type: FUNC
 */
HWTEST_F(ExecResultTest, ExecResult_Parcelable_0100, TestSize.Level1)
{
    ExecResult result;
    result.exitCode = TEST_EXIT_CODE;
    result.outputText = "stdout text";
    result.errorText = "stderr text";
    result.signalNumber = 9;
    result.timeout = true;
    result.executionTime = TEST_TIMEOUT;

    Parcel parcel;
    ASSERT_TRUE(result.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<ExecResult> unmarshalled(ExecResult::Unmarshalling(parcel));
    ASSERT_NE(unmarshalled, nullptr);
    EXPECT_EQ(unmarshalled->exitCode, TEST_EXIT_CODE);
    EXPECT_EQ(unmarshalled->outputText, "stdout text");
    EXPECT_EQ(unmarshalled->errorText, "stderr text");
    EXPECT_EQ(unmarshalled->signalNumber, 9);
    EXPECT_TRUE(unmarshalled->timeout);
    EXPECT_EQ(unmarshalled->executionTime, TEST_TIMEOUT);
}

/**
 * @tc.name: ExecResult_Unmarshalling_0200
 * @tc.desc: Test ExecResult unmarshalling failure branches with incomplete parcel data
 * @tc.type: FUNC
 */
HWTEST_F(ExecResultTest, ExecResult_Unmarshalling_0200, TestSize.Level1)
{
    Parcel emptyParcel;
    EXPECT_EQ(ExecResult::Unmarshalling(emptyParcel), nullptr);

    Parcel missingOutputParcel;
    ASSERT_TRUE(missingOutputParcel.WriteInt32(TEST_EXIT_CODE));
    missingOutputParcel.RewindRead(0);
    EXPECT_EQ(ExecResult::Unmarshalling(missingOutputParcel), nullptr);

    Parcel missingErrorParcel;
    ASSERT_TRUE(missingErrorParcel.WriteInt32(TEST_EXIT_CODE));
    ASSERT_TRUE(missingErrorParcel.WriteString("stdout"));
    missingErrorParcel.RewindRead(0);
    EXPECT_EQ(ExecResult::Unmarshalling(missingErrorParcel), nullptr);

    Parcel missingSignalParcel;
    ASSERT_TRUE(missingSignalParcel.WriteInt32(TEST_EXIT_CODE));
    ASSERT_TRUE(missingSignalParcel.WriteString("stdout"));
    ASSERT_TRUE(missingSignalParcel.WriteString("stderr"));
    missingSignalParcel.RewindRead(0);
    EXPECT_EQ(ExecResult::Unmarshalling(missingSignalParcel), nullptr);

    Parcel missingTimeoutParcel;
    ASSERT_TRUE(missingTimeoutParcel.WriteInt32(TEST_EXIT_CODE));
    ASSERT_TRUE(missingTimeoutParcel.WriteString("stdout"));
    ASSERT_TRUE(missingTimeoutParcel.WriteString("stderr"));
    ASSERT_TRUE(missingTimeoutParcel.WriteInt32(0));
    missingTimeoutParcel.RewindRead(0);
    EXPECT_EQ(ExecResult::Unmarshalling(missingTimeoutParcel), nullptr);

    Parcel missingExecutionTimeParcel;
    ASSERT_TRUE(missingExecutionTimeParcel.WriteInt32(TEST_EXIT_CODE));
    ASSERT_TRUE(missingExecutionTimeParcel.WriteString("stdout"));
    ASSERT_TRUE(missingExecutionTimeParcel.WriteString("stderr"));
    ASSERT_TRUE(missingExecutionTimeParcel.WriteInt32(0));
    ASSERT_TRUE(missingExecutionTimeParcel.WriteBool(false));
    missingExecutionTimeParcel.RewindRead(0);
    EXPECT_EQ(ExecResult::Unmarshalling(missingExecutionTimeParcel), nullptr);
}
} // namespace CliTool
} // namespace OHOS
