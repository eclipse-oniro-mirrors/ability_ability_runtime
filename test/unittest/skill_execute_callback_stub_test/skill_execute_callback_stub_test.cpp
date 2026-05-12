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

#include "hilog_tag_wrapper.h"
#include "mock_skill_execute_callback_stub_for_stub_test.h"
#include "skill/skill_execute_callback_proxy.h"
#include "skill_execute_result.h"
#include "want_params.h"

using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace AAFwk {

class SkillExecuteCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    void WriteInterfaceToken(MessageParcel &data);
};

void SkillExecuteCallbackStubTest::SetUpTestCase(void)
{}
void SkillExecuteCallbackStubTest::TearDownTestCase(void)
{}
void SkillExecuteCallbackStubTest::SetUp()
{}
void SkillExecuteCallbackStubTest::TearDown()
{}

void SkillExecuteCallbackStubTest::WriteInterfaceToken(MessageParcel &data)
{
    data.WriteInterfaceToken(ISkillExecuteCallback::GetDescriptor());
}

/**
 * @tc.name: OnRemoteRequest_0100
 * @tc.desc: Test OnRemoteRequest with invalid interface token.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteCallbackStubTest, OnRemoteRequest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    sptr<MockSkillExecuteCallback> mockStub(new MockSkillExecuteCallback());
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    // Write wrong interface token
    data.WriteInterfaceToken(u"wrong.descriptor");

    int res = mockStub->OnRemoteRequest(ISkillExecuteCallback::ON_SKILL_EXECUTE_DONE,
        data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_STATE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: OnRemoteRequest_0200
 * @tc.desc: Test OnRemoteRequest with unknown code.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteCallbackStubTest, OnRemoteRequest_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    sptr<MockSkillExecuteCallback> mockStub(new MockSkillExecuteCallback());
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);

    int res = mockStub->OnRemoteRequest(999, data, reply, option);
    EXPECT_NE(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: OnRemoteRequest_0300
 * @tc.desc: Test OnRemoteRequest with valid OnExecuteDone request but null result.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteCallbackStubTest, OnRemoteRequest_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    sptr<MockSkillExecuteCallback> mockStub(new MockSkillExecuteCallback());
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    data.WriteString("req001");
    data.WriteInt32(0);
    // Write null result parcelable
    data.WriteParcelable(nullptr);

    int res = mockStub->OnRemoteRequest(ISkillExecuteCallback::ON_SKILL_EXECUTE_DONE,
        data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: OnRemoteRequest_0400
 * @tc.desc: Test OnRemoteRequest with valid OnExecuteDone request and valid result.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteCallbackStubTest, OnRemoteRequest_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    sptr<MockSkillExecuteCallback> mockStub(new MockSkillExecuteCallback());
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    data.WriteString("req001");
    data.WriteInt32(0);

    AppExecFwk::SkillExecuteResult result;
    result.code = 0;
    result.result = std::make_shared<AAFwk::WantParams>();
    data.WriteParcelable(&result);

    EXPECT_CALL(*mockStub, OnExecuteDone(_, _, _)).Times(1);
    int res = mockStub->OnRemoteRequest(ISkillExecuteCallback::ON_SKILL_EXECUTE_DONE,
        data, reply, option);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: OnRemoteRequest_0500
 * @tc.desc: Test OnRemoteRequest with valid data including uris in result.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteCallbackStubTest, OnRemoteRequest_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    sptr<MockSkillExecuteCallback> mockStub(new MockSkillExecuteCallback());
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    data.WriteString("req002");
    data.WriteInt32(-1);

    AppExecFwk::SkillExecuteResult result;
    result.code = -1;
    result.result = std::make_shared<AAFwk::WantParams>();
    result.uris = { "file://test.txt" };
    result.flags = 1;
    data.WriteParcelable(&result);

    EXPECT_CALL(*mockStub, OnExecuteDone(_, _, _)).Times(1);
    int res = mockStub->OnRemoteRequest(ISkillExecuteCallback::ON_SKILL_EXECUTE_DONE,
        data, reply, option);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: OnRemoteRequest_0600
 * @tc.desc: Test OnRemoteRequest with error result code.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteCallbackStubTest, OnRemoteRequest_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    sptr<MockSkillExecuteCallback> mockStub(new MockSkillExecuteCallback());
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    data.WriteString("req003");
    data.WriteInt32(12345);

    AppExecFwk::SkillExecuteResult result;
    result.code = 12345;
    result.result = std::make_shared<AAFwk::WantParams>();
    data.WriteParcelable(&result);

    EXPECT_CALL(*mockStub, OnExecuteDone(_, _, _)).Times(1);
    int res = mockStub->OnRemoteRequest(ISkillExecuteCallback::ON_SKILL_EXECUTE_DONE,
        data, reply, option);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: StubInstance_0100
 * @tc.desc: Test stub instance creation.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteCallbackStubTest, StubInstance_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    sptr<MockSkillExecuteCallback> mockStub(new MockSkillExecuteCallback());
    EXPECT_NE(mockStub, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}
} // namespace AAFwk
} // namespace OHOS
