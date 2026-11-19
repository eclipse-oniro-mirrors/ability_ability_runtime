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

#include "exec_tool_param.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int64_t TEST_TIMEOUT = 3000;
constexpr int64_t TEST_YIELD_MS = 50;
}

class ExecToolParamTest : public testing::Test {};

/**
 * @tc.name: ExecToolParam_Parcelable_0100
 * @tc.desc: Test ExecToolParam nested parcelable marshalling branches
 * @tc.type: FUNC
 */
HWTEST_F(ExecToolParamTest, ExecToolParam_Parcelable_0100, TestSize.Level1)
{
    ExecToolParam param;
    param.toolName = "tool";
    param.subcommand = "run";
    param.challenge = "challenge";
    param.options.background = true;
    param.options.yieldMs = TEST_YIELD_MS;
    param.options.timeout = TEST_TIMEOUT;

    Parcel parcel;
    ASSERT_TRUE(param.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<ExecToolParam> result(ExecToolParam::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->toolName, "tool");
    EXPECT_EQ(result->subcommand, "run");
    EXPECT_EQ(result->challenge, "challenge");
    EXPECT_TRUE(result->options.background);
    EXPECT_EQ(result->options.timeout, TEST_TIMEOUT);

    Parcel missingOptionsParcel;
    ASSERT_TRUE(missingOptionsParcel.WriteString("tool"));
    ASSERT_TRUE(missingOptionsParcel.WriteString("run"));
    ASSERT_TRUE(missingOptionsParcel.WriteString("challenge"));
    missingOptionsParcel.RewindRead(0);
    EXPECT_EQ(ExecToolParam::Unmarshalling(missingOptionsParcel), nullptr);

    Parcel emptyParcel;
    EXPECT_EQ(ExecToolParam::Unmarshalling(emptyParcel), nullptr);

    Parcel missingSubcommandParcel;
    ASSERT_TRUE(missingSubcommandParcel.WriteString("tool"));
    missingSubcommandParcel.RewindRead(0);
    EXPECT_EQ(ExecToolParam::Unmarshalling(missingSubcommandParcel), nullptr);

    Parcel missingChallengeParcel;
    ASSERT_TRUE(missingChallengeParcel.WriteString("tool"));
    ASSERT_TRUE(missingChallengeParcel.WriteString("run"));
    missingChallengeParcel.RewindRead(0);
    EXPECT_EQ(ExecToolParam::Unmarshalling(missingChallengeParcel), nullptr);

    Parcel missingArgsParcel;
    ASSERT_TRUE(missingArgsParcel.WriteString("tool"));
    ASSERT_TRUE(missingArgsParcel.WriteString("run"));
    ASSERT_TRUE(missingArgsParcel.WriteString("challenge"));
    ASSERT_TRUE(missingArgsParcel.WriteParcelable(&param.options));
    missingArgsParcel.RewindRead(0);
    EXPECT_EQ(ExecToolParam::Unmarshalling(missingArgsParcel), nullptr);
}
} // namespace CliTool
} // namespace OHOS
