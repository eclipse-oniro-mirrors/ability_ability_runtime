/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "ability_manager_service.h"
#include "sa_interceptor_manager.h"
#undef private
#include "mock_sa_interceptor_manager.h"
#include "singleton.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
namespace OHOS {
namespace AbilityRuntime {
class SAInterceptorManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void SAInterceptorManagerTest::SetUpTestCase()
{
}

void SAInterceptorManagerTest::TearDownTestCase()
{
}

void SAInterceptorManagerTest::SetUp()
{
}

void SAInterceptorManagerTest::TearDown()
{
}

/*
 * @tc.number: GetInstance_0100
 * @tc.name: GetInstance
 * @tc.desc: Verify GetInstance returns the singleton instance
 */
HWTEST_F(SAInterceptorManagerTest, GetInstance_0100, TestSize.Level1)
{
    SAInterceptorManager& instance1 = SAInterceptorManager::GetInstance();
    SAInterceptorManager& instance2 = SAInterceptorManager::GetInstance();
    
    // Verify we're getting the same instance
    EXPECT_EQ(&instance1, &instance2);
}

/*
 * @tc.number: AddSAInterceptor_0100
 * @tc.name: AddSAInterceptor
 * @tc.desc: Verify AddSAInterceptor with null interceptor
 */
HWTEST_F(SAInterceptorManagerTest, AddSAInterceptor_0100, TestSize.Level1)
{
    int32_t result = SAInterceptorManager::GetInstance().AddSAInterceptor(nullptr);
    EXPECT_EQ(result, ERR_NULL_SA_INTERCEPTOR_EXECUTER);
    sptr<ISAInterceptor> interceptor = new MockSAInterceptor();
    EXPECT_EQ(SAInterceptorManager::GetInstance().saInterceptors_.size(), 0);
    result = SAInterceptorManager::GetInstance().AddSAInterceptor(interceptor);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(SAInterceptorManager::GetInstance().saInterceptors_.size(), 1);
    result = SAInterceptorManager::GetInstance().AddSAInterceptor(interceptor);
    EXPECT_EQ(SAInterceptorManager::GetInstance().saInterceptors_.size(), 1);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * @tc.number: RemoveSAInterceptor_0100
 * @tc.name: RemoveSAInterceptor
 * @tc.desc: Verify RemoveSAInterceptor with null interceptor and non-exist interceptor
 */
HWTEST_F(SAInterceptorManagerTest, RemoveSAInterceptor_0100, TestSize.Level1)
{
    SAInterceptorManager::GetInstance().saInterceptors_.clear();
    int32_t result = SAInterceptorManager::GetInstance().RemoveSAInterceptor(nullptr);
    EXPECT_EQ(result, ERR_NULL_SA_INTERCEPTOR_EXECUTER);
    sptr<ISAInterceptor> interceptor = new MockSAInterceptor();
    result = SAInterceptorManager::GetInstance().RemoveSAInterceptor(interceptor->AsObject());
    EXPECT_EQ(result, ERR_SA_INTERCEPTOR_NOT_EXIST);
    result = SAInterceptorManager::GetInstance().AddSAInterceptor(interceptor);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(SAInterceptorManager::GetInstance().saInterceptors_.size(), 1);
    result = SAInterceptorManager::GetInstance().RemoveSAInterceptor(interceptor->AsObject());
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(SAInterceptorManager::GetInstance().saInterceptors_.size(), 0);
}

/*
 * @tc.number: ExecuteSAInterceptor_0100
 * @tc.name: ExecuteSAInterceptor
 * @tc.desc: Verify ExecuteSAInterceptor functionality
 */
HWTEST_F(SAInterceptorManagerTest, ExecuteSAInterceptor_0100, TestSize.Level1)
{
    std::string params = "";
    OHOS::AAFwk::Rule rule;
    int32_t result = SAInterceptorManager::GetInstance().ExecuteSAInterceptor(params, rule);
    EXPECT_EQ(result, ERR_OK);
    sptr<ISAInterceptor> interceptor = new MockSAInterceptorRetFalse();
    SAInterceptorManager::GetInstance().saInterceptors_.emplace_back(interceptor);
    result = SAInterceptorManager::GetInstance().ExecuteSAInterceptor(params, rule);
    EXPECT_NE(result, ERR_OK);
}

/*
 * @tc.number: SAInterceptorListIsEmpty_0100
 * @tc.name: SAInterceptorListIsEmpty
 * @tc.desc: Verify SAInterceptorListIsEmpty functionality
 */
HWTEST_F(SAInterceptorManagerTest, SAInterceptorListIsEmpty_0100, TestSize.Level1)
{
    SAInterceptorManager::GetInstance().saInterceptors_.clear();
    bool result = SAInterceptorManager::GetInstance().SAInterceptorListIsEmpty();
    EXPECT_EQ(result, true);
    sptr<MockSAInterceptor> interceptor = new MockSAInterceptor();
    SAInterceptorManager::GetInstance().saInterceptors_.emplace_back(interceptor);
    result = SAInterceptorManager::GetInstance().SAInterceptorListIsEmpty();
    EXPECT_EQ(result, false);
}

/*
 * @tc.number: ObserverExist_0100
 * @tc.name: ObserverExist
 * @tc.desc: Verify ObserverExist functionality
 */
HWTEST_F(SAInterceptorManagerTest, ObserverExist_0100, TestSize.Level1)
{
    SAInterceptorManager::GetInstance().saInterceptors_.clear();
    sptr<MockSAInterceptor> interceptor = new MockSAInterceptor();
    bool result = SAInterceptorManager::GetInstance().ObserverExist(interceptor);
    EXPECT_EQ(result, false);
    SAInterceptorManager::GetInstance().saInterceptors_.emplace_back(interceptor);
    result = SAInterceptorManager::GetInstance().ObserverExist(interceptor);
    EXPECT_EQ(result, true);
}

/*
 * @tc.number: GenerateSAInterceptorParams_0100
 * @tc.name: GenerateSAInterceptorParams
 * @tc.desc: Verify GenerateSAInterceptorParams functionality
 */
HWTEST_F(SAInterceptorManagerTest, GenerateSAInterceptorParams_0100, TestSize.Level1)
{
    std::string dialogSessionId = "10001";
    AppExecFwk::AbilityInfo abilityInfo;
    Want want;
    auto result = SAInterceptorManager::GetInstance().GenerateSAInterceptorParams(want, nullptr,
        abilityInfo, dialogSessionId);
    EXPECT_NE(result, "");
}

/*
 * @tc.number: OnObserverDied_0100
 * @tc.name: OnObserverDied
 * @tc.desc: Verify OnObserverDied
 */
HWTEST_F(SAInterceptorManagerTest, OnObserverDied_0100, TestSize.Level1)
{
    wptr<IRemoteObject> remote = nullptr;
    SAInterceptorManager::GetInstance().OnObserverDied(remote);

    remote = sptr<MockIRemoteObject>::MakeSptr();
    sptr<ISAInterceptor> interceptor = sptr<MockSAInterceptor>::MakeSptr();
    SAInterceptorManager::GetInstance().saInterceptors_.emplace_back(interceptor);
    EXPECT_NE(SAInterceptorManager::GetInstance().saInterceptors_.size(), 0);
    SAInterceptorManager::GetInstance().OnObserverDied(remote);
    EXPECT_NE(&SAInterceptorManager::GetInstance(), nullptr);
}
} // namespace AbilityRuntime
} // namespace OHOS