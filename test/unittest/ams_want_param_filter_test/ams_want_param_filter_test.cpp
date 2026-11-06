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
#include <unistd.h>

#include "mock_accesstoken_kit.h"
#include "ams_want_param_filter.h"
#include "mock_ipc_skeleton.h"
#include "parcel.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char *AGENT_TASK_ID_PARAM_KEY = "ohos.aafwk.param.agent.taskId";
// bit 32 set marks a system-app full token (same pattern as task_manager_service_test)
constexpr uint64_t TEST_SYSTEM_APP_FLAG = (1ULL << 32);
}  // namespace

class AMSWantParamFilterTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        // register the filter as the WantParams deserialization observer once
        AMSWantParamFilter::GetInstance()->InstallFilter();
    }

    void SetUp() override
    {
        // cross-process IPC caller with a plain (non-system) token by default
        IPCSkeleton::SetCallingPid(getpid() + 1);
        IPCSkeleton::SetCallingFullTokenID(0);
    }

    void TearDown() override
    {
        IPCSkeleton::SetCallingPid(getpid());
        IPCSkeleton::SetCallingTokenID(0);
        IPCSkeleton::SetCallingFullTokenID(0);
        Security::AccessToken::SetMockTokenType(Security::AccessToken::MockTokenType::INVALID);
    }
};

/**
 * @tc.name: AMSWantParamFilterTest_ThirdPartyTaskIdStripped_001
 * @tc.desc: A third-party IPC caller cannot carry the task-attribution reserved key;
 *           business keys pass through untouched.
 * @tc.type: FUNC
 */
HWTEST_F(AMSWantParamFilterTest, ThirdPartyTaskIdStripped_001, TestSize.Level1)
{
    Want src;
    src.SetParam(AGENT_TASK_ID_PARAM_KEY, std::string("forged_task"));
    src.SetParam("business.key", std::string("kept"));
    Parcel parcel;
    ASSERT_TRUE(src.Marshalling(parcel));

    std::unique_ptr<Want> dst(Want::Unmarshalling(parcel));
    ASSERT_NE(dst, nullptr);
    EXPECT_FALSE(dst->HasParameter(AGENT_TASK_ID_PARAM_KEY));
    EXPECT_EQ(dst->GetStringParam("business.key"), "kept");
}

/**
 * @tc.name: AMSWantParamFilterTest_SystemCallerTaskIdKept_002
 * @tc.desc: A system caller passes through untouched, so the legitimate task-attribution
 *           flow (system apps only) is unaffected.
 * @tc.type: FUNC
 */
HWTEST_F(AMSWantParamFilterTest, SystemCallerTaskIdKept_002, TestSize.Level1)
{
    // system-app full token: IsSystemAppCall() is true, the filter passes through
    IPCSkeleton::SetCallingFullTokenID(TEST_SYSTEM_APP_FLAG | 1);
    Want src;
    src.SetParam(AGENT_TASK_ID_PARAM_KEY, std::string("legit_task"));
    Parcel parcel;
    ASSERT_TRUE(src.Marshalling(parcel));

    std::unique_ptr<Want> dst(Want::Unmarshalling(parcel));
    ASSERT_NE(dst, nullptr);
    EXPECT_EQ(dst->GetStringParam(AGENT_TASK_ID_PARAM_KEY), "legit_task");
}

/**
 * @tc.name: AMSWantParamFilterTest_LocalDeserializationKept_003
 * @tc.desc: A local (same-process) deserialization has no IPC caller and must not be filtered.
 * @tc.type: FUNC
 */
HWTEST_F(AMSWantParamFilterTest, LocalDeserializationKept_003, TestSize.Level1)
{
    IPCSkeleton::SetCallingPid(getpid());
    Want src;
    src.SetParam(AGENT_TASK_ID_PARAM_KEY, std::string("snapshot_task"));
    Parcel parcel;
    ASSERT_TRUE(src.Marshalling(parcel));

    std::unique_ptr<Want> dst(Want::Unmarshalling(parcel));
    ASSERT_NE(dst, nullptr);
    EXPECT_EQ(dst->GetStringParam(AGENT_TASK_ID_PARAM_KEY), "snapshot_task");
}

/**
 * @tc.name: AMSWantParamFilterTest_SaCallerTaskIdKept_004
 * @tc.desc: An SA (native token) IPC caller passes through untouched.
 * @tc.type: FUNC
 */
HWTEST_F(AMSWantParamFilterTest, SaCallerTaskIdKept_004, TestSize.Level1)
{
    Security::AccessToken::SetMockTokenType(Security::AccessToken::MockTokenType::NATIVE);
    IPCSkeleton::SetCallingTokenID(1);
    Want src;
    src.SetParam(AGENT_TASK_ID_PARAM_KEY, std::string("sa_task"));
    Parcel parcel;
    ASSERT_TRUE(src.Marshalling(parcel));

    std::unique_ptr<Want> dst(Want::Unmarshalling(parcel));
    ASSERT_NE(dst, nullptr);
    EXPECT_EQ(dst->GetStringParam(AGENT_TASK_ID_PARAM_KEY), "sa_task");
}

/**
 * @tc.name: AMSWantParamFilterTest_HapTokenStripped_005
 * @tc.desc: A plain HAP token (non-system-app) is still a third-party caller: the
 *           reserved key is stripped even with an empty value.
 * @tc.type: FUNC
 */
HWTEST_F(AMSWantParamFilterTest, HapTokenStripped_005, TestSize.Level1)
{
    Security::AccessToken::SetMockTokenType(Security::AccessToken::MockTokenType::HAP);
    IPCSkeleton::SetCallingTokenID(2);
    Want src;
    src.SetParam(AGENT_TASK_ID_PARAM_KEY, std::string(""));
    src.SetParam("business.key", std::string("kept"));
    Parcel parcel;
    ASSERT_TRUE(src.Marshalling(parcel));

    std::unique_ptr<Want> dst(Want::Unmarshalling(parcel));
    ASSERT_NE(dst, nullptr);
    // the key itself is dropped, so an empty value cannot smuggle the key through
    EXPECT_FALSE(dst->HasParameter(AGENT_TASK_ID_PARAM_KEY));
    EXPECT_EQ(dst->GetStringParam("business.key"), "kept");
}

/**
 * @tc.name: AMSWantParamFilterTest_ReservedKeyLiteralAnchored_007
 * @tc.desc: The ability_base constant must stay in sync with the strip defense.
 * @tc.type: FUNC
 */
HWTEST_F(AMSWantParamFilterTest, ReservedKeyLiteralAnchored_007, TestSize.Level1)
{
    EXPECT_STREQ(Want::PARAM_RESV_AGENT_TASK_ID.c_str(), AGENT_TASK_ID_PARAM_KEY);
}
}  // namespace AAFwk
}  // namespace OHOS
