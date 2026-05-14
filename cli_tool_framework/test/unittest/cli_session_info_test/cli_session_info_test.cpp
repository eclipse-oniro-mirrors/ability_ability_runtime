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

#include "cli_session_info.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t TEST_EXIT_CODE = 7;
}

class CliSessionInfoTest : public testing::Test {};

/**
 * @tc.name: CliSessionInfo_Parcelable_0100
 * @tc.desc: Test CliSessionInfo optional result marshalling branches
 * @tc.type: FUNC
 */
HWTEST_F(CliSessionInfoTest, CliSessionInfo_Parcelable_0100, TestSize.Level1)
{
    CliSessionInfo runningInfo;
    runningInfo.sessionId = "session-running";
    runningInfo.toolName = "tool";
    runningInfo.status = "running";

    Parcel runningParcel;
    ASSERT_TRUE(runningInfo.Marshalling(runningParcel));
    runningParcel.RewindRead(0);
    std::unique_ptr<CliSessionInfo> runningResult(CliSessionInfo::Unmarshalling(runningParcel));
    ASSERT_NE(runningResult, nullptr);
    EXPECT_EQ(runningResult->sessionId, "session-running");
    EXPECT_EQ(runningResult->toolName, "tool");
    EXPECT_EQ(runningResult->status, "running");
    EXPECT_EQ(runningResult->result, nullptr);

    CliSessionInfo completedInfo;
    completedInfo.sessionId = "session-completed";
    completedInfo.toolName = "tool";
    completedInfo.status = "completed";
    completedInfo.result = std::make_shared<ExecResult>();
    completedInfo.result->exitCode = TEST_EXIT_CODE;
    completedInfo.result->outputText = "ok";

    Parcel completedParcel;
    ASSERT_TRUE(completedInfo.Marshalling(completedParcel));
    completedParcel.RewindRead(0);
    std::unique_ptr<CliSessionInfo> completedResult(CliSessionInfo::Unmarshalling(completedParcel));
    ASSERT_NE(completedResult, nullptr);
    ASSERT_NE(completedResult->result, nullptr);
    EXPECT_EQ(completedResult->status, "completed");
    EXPECT_EQ(completedResult->result->exitCode, TEST_EXIT_CODE);
    EXPECT_EQ(completedResult->result->outputText, "ok");
}

/**
 * @tc.name: CliSessionInfo_Unmarshalling_0200
 * @tc.desc: Test CliSessionInfo unmarshalling failure branches
 * @tc.type: FUNC
 */
HWTEST_F(CliSessionInfoTest, CliSessionInfo_Unmarshalling_0200, TestSize.Level1)
{
    Parcel emptyParcel;
    EXPECT_EQ(CliSessionInfo::Unmarshalling(emptyParcel), nullptr);

    Parcel missingStatusParcel;
    ASSERT_TRUE(missingStatusParcel.WriteString("session"));
    ASSERT_TRUE(missingStatusParcel.WriteString("tool"));
    missingStatusParcel.RewindRead(0);
    EXPECT_EQ(CliSessionInfo::Unmarshalling(missingStatusParcel), nullptr);

    Parcel missingResultParcel;
    ASSERT_TRUE(missingResultParcel.WriteString("session"));
    ASSERT_TRUE(missingResultParcel.WriteString("tool"));
    ASSERT_TRUE(missingResultParcel.WriteString("completed"));
    ASSERT_TRUE(missingResultParcel.WriteBool(true));
    missingResultParcel.RewindRead(0);
    EXPECT_EQ(CliSessionInfo::Unmarshalling(missingResultParcel), nullptr);
}
} // namespace CliTool
} // namespace OHOS
