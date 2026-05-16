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

#include "exec_options.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int64_t TEST_TIMEOUT = 3000;
constexpr int64_t TEST_YIELD_MS = 50;
}

class ExecOptionsTest : public testing::Test {};

/**
 * @tc.name: ExecOptions_Parcelable_0100
 * @tc.desc: Test ExecOptions marshalling and unmarshalling success path
 * @tc.type: FUNC
 */
HWTEST_F(ExecOptionsTest, ExecOptions_Parcelable_0100, TestSize.Level1)
{
    ExecOptions options;
    options.background = true;
    options.yieldMs = TEST_YIELD_MS;
    options.timeout = TEST_TIMEOUT;

    Parcel parcel;
    ASSERT_TRUE(options.Marshalling(parcel));
    parcel.RewindRead(0);

    std::unique_ptr<ExecOptions> result(ExecOptions::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_TRUE(result->background);
    EXPECT_EQ(result->yieldMs, TEST_YIELD_MS);
    EXPECT_EQ(result->timeout, TEST_TIMEOUT);
}

/**
 * @tc.name: ExecOptions_Unmarshalling_0200
 * @tc.desc: Test ExecOptions unmarshalling failure branches with incomplete parcel data
 * @tc.type: FUNC
 */
HWTEST_F(ExecOptionsTest, ExecOptions_Unmarshalling_0200, TestSize.Level1)
{
    Parcel emptyParcel;
    EXPECT_EQ(ExecOptions::Unmarshalling(emptyParcel), nullptr);

    Parcel partialParcel;
    ASSERT_TRUE(partialParcel.WriteBool(true));
    partialParcel.RewindRead(0);
    EXPECT_EQ(ExecOptions::Unmarshalling(partialParcel), nullptr);

    Parcel missingTimeoutParcel;
    ASSERT_TRUE(missingTimeoutParcel.WriteBool(false));
    ASSERT_TRUE(missingTimeoutParcel.WriteInt64(TEST_YIELD_MS));
    missingTimeoutParcel.RewindRead(0);
    EXPECT_EQ(ExecOptions::Unmarshalling(missingTimeoutParcel), nullptr);
}
} // namespace CliTool
} // namespace OHOS
