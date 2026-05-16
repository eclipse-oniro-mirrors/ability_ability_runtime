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

#include "cli_tool_event.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t TEST_EXIT_CODE = 7;
constexpr int64_t TEST_TIMESTAMP = 123456;
}

class CliToolEventTest : public testing::Test {};

/**
 * @tc.name: CliToolEvent_Parcelable_0100
 * @tc.desc: Test CliToolEvent marshalling and unmarshalling success and failure paths
 * @tc.type: FUNC
 */
HWTEST_F(CliToolEventTest, CliToolEvent_Parcelable_0100, TestSize.Level1)
{
    CliToolEvent event;
    event.type = "stdout";
    event.eventData = "payload";
    event.exitCode = TEST_EXIT_CODE;
    event.timestamp = TEST_TIMESTAMP;

    Parcel parcel;
    ASSERT_TRUE(event.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<CliToolEvent> unmarshalled(CliToolEvent::Unmarshalling(parcel));
    ASSERT_NE(unmarshalled, nullptr);
    EXPECT_EQ(unmarshalled->type, "stdout");
    EXPECT_EQ(unmarshalled->eventData, "payload");
    EXPECT_EQ(unmarshalled->exitCode, TEST_EXIT_CODE);
    EXPECT_EQ(unmarshalled->timestamp, TEST_TIMESTAMP);

    Parcel partialParcel;
    ASSERT_TRUE(partialParcel.WriteString("exit"));
    partialParcel.RewindRead(0);
    EXPECT_EQ(CliToolEvent::Unmarshalling(partialParcel), nullptr);

    Parcel emptyParcel;
    EXPECT_EQ(CliToolEvent::Unmarshalling(emptyParcel), nullptr);

    Parcel missingExitCodeParcel;
    ASSERT_TRUE(missingExitCodeParcel.WriteString("exit"));
    ASSERT_TRUE(missingExitCodeParcel.WriteString("payload"));
    missingExitCodeParcel.RewindRead(0);
    EXPECT_EQ(CliToolEvent::Unmarshalling(missingExitCodeParcel), nullptr);

    Parcel missingTimestampParcel;
    ASSERT_TRUE(missingTimestampParcel.WriteString("exit"));
    ASSERT_TRUE(missingTimestampParcel.WriteString("payload"));
    ASSERT_TRUE(missingTimestampParcel.WriteInt32(TEST_EXIT_CODE));
    missingTimestampParcel.RewindRead(0);
    EXPECT_EQ(CliToolEvent::Unmarshalling(missingTimestampParcel), nullptr);
}
} // namespace CliTool
} // namespace OHOS
