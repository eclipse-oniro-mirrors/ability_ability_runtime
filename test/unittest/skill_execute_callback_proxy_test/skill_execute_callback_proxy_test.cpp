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
#include "mock_skill_execute_callback_stub.h"
#include "skill/skill_execute_callback_proxy.h"
#include "skill_execute_result.h"

using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace AAFwk {

class SkillExecuteCallbackProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SkillExecuteCallbackProxyTest::SetUpTestCase(void)
{}
void SkillExecuteCallbackProxyTest::TearDownTestCase(void)
{}
void SkillExecuteCallbackProxyTest::SetUp()
{}
void SkillExecuteCallbackProxyTest::TearDown()
{}

/**
 * @tc.name: OnExecuteDone_0100
 * @tc.desc: Test OnExecuteDone IPC between proxy and stub.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteCallbackProxyTest, OnExecuteDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    sptr<MockSkillExecuteCallbackStub> mockStub(new MockSkillExecuteCallbackStub());
    sptr<SkillExecuteCallbackProxy> proxy(new SkillExecuteCallbackProxy(mockStub));

    AppExecFwk::SkillExecuteResult result;
    result.code = 0;
    result.result = std::make_shared<AAFwk::WantParams>();

    EXPECT_CALL(*mockStub, OnExecuteDone(_, _, _))
        .Times(1)
        .WillOnce(InvokeWithoutArgs(mockStub.GetRefPtr(), &MockSkillExecuteCallbackStub::PostVoid));

    proxy->OnExecuteDone("req001", 0, result);
    mockStub->Wait();
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: OnExecuteDone_0200
 * @tc.desc: Test OnExecuteDone with error result code.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteCallbackProxyTest, OnExecuteDone_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    sptr<MockSkillExecuteCallbackStub> mockStub(new MockSkillExecuteCallbackStub());
    sptr<SkillExecuteCallbackProxy> proxy(new SkillExecuteCallbackProxy(mockStub));

    AppExecFwk::SkillExecuteResult result;
    result.code = -1;
    result.result = std::make_shared<AAFwk::WantParams>();

    EXPECT_CALL(*mockStub, OnExecuteDone(_, _, _))
        .Times(1)
        .WillOnce(InvokeWithoutArgs(mockStub.GetRefPtr(), &MockSkillExecuteCallbackStub::PostVoid));

    proxy->OnExecuteDone("req002", -1, result);
    mockStub->Wait();
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: OnExecuteDone_0300
 * @tc.desc: Test OnExecuteDone with uris in result.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteCallbackProxyTest, OnExecuteDone_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    sptr<MockSkillExecuteCallbackStub> mockStub(new MockSkillExecuteCallbackStub());
    sptr<SkillExecuteCallbackProxy> proxy(new SkillExecuteCallbackProxy(mockStub));

    AppExecFwk::SkillExecuteResult result;
    result.code = 0;
    result.result = std::make_shared<AAFwk::WantParams>();
    result.uris = { "file://docs/storage/test.txt" };
    result.flags = 1;

    EXPECT_CALL(*mockStub, OnExecuteDone(_, _, _))
        .Times(1)
        .WillOnce(InvokeWithoutArgs(mockStub.GetRefPtr(), &MockSkillExecuteCallbackStub::PostVoid));

    proxy->OnExecuteDone("req003", 0, result);
    mockStub->Wait();
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ProxyInstance_0100
 * @tc.desc: Test proxy instance creation is successful.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteCallbackProxyTest, ProxyInstance_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    sptr<MockSkillExecuteCallbackStub> mockStub(new MockSkillExecuteCallbackStub());
    sptr<SkillExecuteCallbackProxy> proxy(new SkillExecuteCallbackProxy(mockStub));
    EXPECT_NE(proxy, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}
} // namespace AAFwk
} // namespace OHOS
