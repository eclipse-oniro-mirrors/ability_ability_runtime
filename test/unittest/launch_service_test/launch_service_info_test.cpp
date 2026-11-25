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
#include "launch_service_info.h"
#include "daemon_service_info.h"

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class LaunchServiceInfoTest : public testing::Test {
};

namespace {
LaunchServiceDescriptor CreateValidDescriptor()
{
    LaunchServiceDescriptor value;
    value.bundleName = "com.example.daemon";
    value.moduleName = "entry";
    value.serviceName = "worker";
    return value;
}
} // namespace

/**
 * @tc.name: StateDataMarshalling_001
 * @tc.desc: Verify all registration states round trip and invalid states fail.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInfoTest, StateDataMarshalling_001, TestSize.Level1)
{
    for (auto state : {LaunchServiceState::NOT_REGISTERED, LaunchServiceState::ENABLED, LaunchServiceState::DISABLED}) {
        LaunchServiceStateData value;
        value.descriptor = CreateValidDescriptor();
        value.state = state;
        Parcel parcel;
        ASSERT_TRUE(value.Marshalling(parcel));
        std::unique_ptr<LaunchServiceStateData> result(LaunchServiceStateData::Unmarshalling(parcel));
        ASSERT_NE(result, nullptr);
        EXPECT_EQ(result->state, state);
        EXPECT_EQ(result->descriptor.bundleName, value.descriptor.bundleName);
    }
    LaunchServiceStateData invalid;
    invalid.descriptor = CreateValidDescriptor();
    invalid.state = static_cast<LaunchServiceState>(3);
    Parcel parcel;
    EXPECT_FALSE(invalid.Marshalling(parcel));
    Parcel empty;
    EXPECT_EQ(LaunchServiceStateData::Unmarshalling(empty), nullptr);
}

/**
 * @tc.name: DescriptorMarshalling_001
 * @tc.desc: Verify descriptor round trip preserves resolved default identity fields.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInfoTest, DescriptorMarshalling_001, TestSize.Level1)
{
    auto value = CreateValidDescriptor();
    Parcel parcel;
    ASSERT_TRUE(value.Marshalling(parcel));
    std::unique_ptr<LaunchServiceDescriptor> result(LaunchServiceDescriptor::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->bundleName, value.bundleName);
    EXPECT_EQ(result->moduleName, value.moduleName);
    EXPECT_EQ(result->serviceName, value.serviceName);
    EXPECT_EQ(result->userId, 0);
    EXPECT_EQ(result->appIndex, 0);
}

/**
 * @tc.name: DescriptorMarshalling_002
 * @tc.desc: Verify descriptor round trip preserves explicit app indexes 0, 1 and 1000.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInfoTest, DescriptorMarshalling_002, TestSize.Level1)
{
    for (int32_t index : {0, 1, 1000}) {
        auto value = CreateValidDescriptor();
        value.userId = 100;
        value.appIndex = index;
        Parcel parcel;
        ASSERT_TRUE(value.Marshalling(parcel));
        std::unique_ptr<LaunchServiceDescriptor> result(LaunchServiceDescriptor::Unmarshalling(parcel));
        ASSERT_NE(result, nullptr);
        EXPECT_EQ(result->userId, 100);
        EXPECT_EQ(result->appIndex, index);
    }
}

/**
 * @tc.name: DescriptorMarshalling_003
 * @tc.desc: Verify marshalling rejects invalid descriptor identity fields.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInfoTest, DescriptorMarshalling_003, TestSize.Level1)
{
    for (int field = 0; field < 5; ++field) {
        auto value = CreateValidDescriptor();
        if (field == 0) {
            value.bundleName.clear();
        }
        if (field == 1) {
            value.moduleName.clear();
        }
        if (field == 2) {
            value.serviceName.clear();
        }
        if (field == 3) {
            value.userId = -1;
        }
        if (field == 4) {
            value.appIndex = -1;
        }
        Parcel parcel;
        EXPECT_FALSE(value.Marshalling(parcel)) << field;
    }
}

/**
 * @tc.name: DescriptorUnmarshalling_001
 * @tc.desc: Verify unmarshalling rejects empty and truncated descriptors.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInfoTest, DescriptorUnmarshalling_001, TestSize.Level1)
{
    Parcel empty;
    EXPECT_EQ(LaunchServiceDescriptor::Unmarshalling(empty), nullptr);
    Parcel truncated;
    ASSERT_TRUE(truncated.WriteString("com.example.daemon"));
    EXPECT_EQ(LaunchServiceDescriptor::Unmarshalling(truncated), nullptr);
}

/**
 * @tc.name: DescriptorUnmarshalling_002
 * @tc.desc: Verify unmarshalling rejects missing userId or appIndex.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInfoTest, DescriptorUnmarshalling_002, TestSize.Level1)
{
    for (bool includeUserId : {false, true}) {
        Parcel parcel;
        ASSERT_TRUE(parcel.WriteString("com.example.daemon"));
        ASSERT_TRUE(parcel.WriteString("entry"));
        ASSERT_TRUE(parcel.WriteString("worker"));
        if (includeUserId) {
            ASSERT_TRUE(parcel.WriteInt32(100));
        }
        EXPECT_EQ(LaunchServiceDescriptor::Unmarshalling(parcel), nullptr);
    }
}

/**
 * @tc.name: InformationMarshalling_001
 * @tc.desc: Verify information round trip preserves each launch service state.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInfoTest, InformationMarshalling_001, TestSize.Level1)
{
    for (auto state : {LaunchServiceState::NOT_REGISTERED, LaunchServiceState::ENABLED, LaunchServiceState::DISABLED}) {
        LaunchServiceInformation value;
        value.descriptor = CreateValidDescriptor();
        value.state = state;
        value.pid = state == LaunchServiceState::NOT_REGISTERED ? 0 : 123;
        Parcel parcel;
        ASSERT_TRUE(value.Marshalling(parcel));
        std::unique_ptr<LaunchServiceInformation> result(LaunchServiceInformation::Unmarshalling(parcel));
        ASSERT_NE(result, nullptr);
        EXPECT_EQ(result->state, state);
        EXPECT_EQ(result->pid, value.pid);
    }
}

/**
 * @tc.name: InformationMarshalling_002
 * @tc.desc: Verify marshalling rejects invalid states, negative PIDs and inconsistent information.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInfoTest, InformationMarshalling_002, TestSize.Level1)
{
    for (int scenario = 0; scenario < 3; ++scenario) {
        LaunchServiceInformation value;
        value.descriptor = CreateValidDescriptor();
        value.state = LaunchServiceState::ENABLED;
        if (scenario == 0) {
            value.state = static_cast<LaunchServiceState>(99);
        }
        if (scenario == 1) {
            value.pid = -1;
        }
        if (scenario == 2) {
            value.state = LaunchServiceState::NOT_REGISTERED;
            value.pid = 123;
        }
        Parcel parcel;
        EXPECT_FALSE(value.Marshalling(parcel)) << scenario;
    }
}

/**
 * @tc.name: InformationUnmarshalling_001
 * @tc.desc: Verify unmarshalling rejects invalid state values from a parcel.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInfoTest, InformationUnmarshalling_001, TestSize.Level1)
{
    auto descriptor = CreateValidDescriptor();
    for (int state : {-1, 3, 100}) {
        Parcel parcel;
        ASSERT_TRUE(parcel.WriteParcelable(&descriptor));
        ASSERT_TRUE(parcel.WriteInt32(state));
        ASSERT_TRUE(parcel.WriteInt32(0));
        EXPECT_EQ(LaunchServiceInformation::Unmarshalling(parcel), nullptr);
    }
}

/**
 * @tc.name: ProcessStateDataMarshalling_001
 * @tc.desc: Verify process state round trip preserves CREATED and TERMINATED events.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInfoTest, ProcessStateDataMarshalling_001, TestSize.Level1)
{
    for (auto state : {LaunchServiceProcessState::CREATED, LaunchServiceProcessState::TERMINATED}) {
        LaunchServiceProcessStateData value;
        value.descriptor = CreateValidDescriptor();
        value.pid = 123;
        value.state = state;
        Parcel parcel;
        ASSERT_TRUE(value.Marshalling(parcel));
        std::unique_ptr<LaunchServiceProcessStateData> result(LaunchServiceProcessStateData::Unmarshalling(parcel));
        ASSERT_NE(result, nullptr);
        EXPECT_EQ(result->pid, 123);
        EXPECT_EQ(result->state, state);
    }
}

/**
 * @tc.name: ProcessStateDataMarshalling_002
 * @tc.desc: Verify marshalling rejects negative PIDs and invalid process states.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInfoTest, ProcessStateDataMarshalling_002, TestSize.Level1)
{
    LaunchServiceProcessStateData value;
    value.descriptor = CreateValidDescriptor();
    value.pid = -1;
    Parcel negativePid;
    EXPECT_FALSE(value.Marshalling(negativePid));
    value.pid = 123;
    value.state = static_cast<LaunchServiceProcessState>(2);
    Parcel invalidState;
    EXPECT_FALSE(value.Marshalling(invalidState));
}

/**
 * @tc.name: DaemonDescriptorIdentity_001
 * @tc.desc: Accept boundary-length fields and default user; reject empty, oversized or negative identity fields.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceInfoTest, DaemonDescriptorIdentity_001, TestSize.Level1)
{
    AAFwk::LaunchServiceDescriptor descriptor {"bundle", "entry", "worker", -1, 0};
    EXPECT_TRUE(descriptor.IsValid());
    EXPECT_TRUE(AAFwk::LaunchServiceDescriptor::IsValidIdentity(std::string(512, 'b'), "m", "s", 0, 0));
    EXPECT_FALSE(AAFwk::LaunchServiceDescriptor::IsValidIdentity(std::string(513, 'b'), "m", "s", 0, 0));
    EXPECT_FALSE(AAFwk::LaunchServiceDescriptor::IsValidIdentity("b", "", "s", 0, 0));
    EXPECT_FALSE(AAFwk::LaunchServiceDescriptor::IsValidIdentity("b", "m", "", 0, 0));
    EXPECT_FALSE(AAFwk::LaunchServiceDescriptor::IsValidIdentity("b", "m", "s", -2, 0));
    EXPECT_FALSE(AAFwk::LaunchServiceDescriptor::IsValidIdentity("b", "m", "s", 0, -1));
}
} // namespace AbilityRuntime
} // namespace OHOS
