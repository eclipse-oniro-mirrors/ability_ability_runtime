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

#include <atomic>
#include <gtest/gtest.h>
#include <map>
#include <thread>
#include <vector>

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
// Mock SA interceptor bound to a distinct remote object, used for multi-interceptor scenarios
class MockSAInterceptorWithObj : public ISAInterceptor {
public:
    explicit MockSAInterceptorWithObj(int32_t id) : id_(id), remote_(sptr<MockIRemoteObject>::MakeSptr()) {}
    ~MockSAInterceptorWithObj() = default;
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.AbiilityRuntime.ISAInterceptor");
    int32_t OnCheckStarting(const std::string &params, Rule &rule) override
    {
        checkCount_.fetch_add(1, std::memory_order_relaxed);
        rule.type = allow_ ? RuleType::ALLOW : RuleType::NOT_ALLOW;
        return ERR_OK;
    }
    sptr<IRemoteObject> AsObject() override
    {
        return remote_;
    }

    int32_t id_ = 0;
    bool allow_ = true;
    std::atomic<int32_t> checkCount_ { 0 };
    sptr<MockIRemoteObject> remote_;
};

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
 * @tc.desc: Verify RemoveSAInterceptor with null interceptor
 */
HWTEST_F(SAInterceptorManagerTest, RemoveSAInterceptor_0100, TestSize.Level1)
{
    SAInterceptorManager::GetInstance().saInterceptors_.clear();
    int32_t result = SAInterceptorManager::GetInstance().RemoveSAInterceptor(nullptr);
    EXPECT_EQ(result, ERR_NULL_SA_INTERCEPTOR_EXECUTER);
    sptr<IRemoteObject> remoteObj = new MockIRemoteObject();
    result = SAInterceptorManager::GetInstance().RemoveSAInterceptor(remoteObj);
    EXPECT_EQ(result, ERR_SA_INTERCEPTOR_NOT_EXIST);
}

/*
 * @tc.number: ExecuteSAInterceptor_0100
 * @tc.name: ExecuteSAInterceptor
 * @tc.desc: Verify ExecuteSAInterceptor functionality
 */
HWTEST_F(SAInterceptorManagerTest, ExecuteSAInterceptor_0100, TestSize.Level1)
{
    std::string params = "";
    Rule rule;
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

/*
 * @tc.number: ExecuteSAInterceptor_0200
 * @tc.name: ExecuteSAInterceptor
 * @tc.desc: Verify multiple interceptors with distinct remote objects are all registered,
 *           executed in order, and the execution is short-circuited once a rule is violated
 */
HWTEST_F(SAInterceptorManagerTest, ExecuteSAInterceptor_0200, TestSize.Level1)
{
    SAInterceptorManager::GetInstance().saInterceptors_.clear();
    auto first = sptr<MockSAInterceptorWithObj>::MakeSptr(1);
    auto blocker = sptr<MockSAInterceptorWithObj>::MakeSptr(2);
    blocker->allow_ = false;
    auto last = sptr<MockSAInterceptorWithObj>::MakeSptr(3);

    EXPECT_EQ(SAInterceptorManager::GetInstance().AddSAInterceptor(first), ERR_OK);
    EXPECT_EQ(SAInterceptorManager::GetInstance().AddSAInterceptor(blocker), ERR_OK);
    EXPECT_EQ(SAInterceptorManager::GetInstance().AddSAInterceptor(last), ERR_OK);
    // interceptors with distinct remote objects are all kept, no dedup miss
    EXPECT_EQ(SAInterceptorManager::GetInstance().saInterceptors_.size(), 3);

    std::string params = "{}";
    Rule rule;
    int32_t result = SAInterceptorManager::GetInstance().ExecuteSAInterceptor(params, rule);
    EXPECT_EQ(result, ERR_OK);
    // interceptors before and including the violated one are executed
    EXPECT_GT(first->checkCount_.load(), 0);
    EXPECT_GT(blocker->checkCount_.load(), 0);
    // interceptors after the violated one are short-circuited
    EXPECT_EQ(last->checkCount_.load(), 0);
    EXPECT_EQ(rule.type, RuleType::NOT_ALLOW);

    EXPECT_EQ(SAInterceptorManager::GetInstance().RemoveSAInterceptor(first->AsObject()), ERR_OK);
    EXPECT_EQ(SAInterceptorManager::GetInstance().RemoveSAInterceptor(blocker->AsObject()), ERR_OK);
    EXPECT_EQ(SAInterceptorManager::GetInstance().RemoveSAInterceptor(last->AsObject()), ERR_OK);
    EXPECT_TRUE(SAInterceptorManager::GetInstance().SAInterceptorListIsEmpty());
}

/*
 * @tc.number: SAInterceptorMultiThreadRace_0100
 * @tc.name: SAInterceptorMultiThreadRace
 * @tc.desc: Verify add/remove/execute of multiple SA interceptors from concurrent threads
 *           keeps the interceptor list consistent: no duplicated interceptor, no crash,
 *           and the manager stays functional after the race
 */
HWTEST_F(SAInterceptorManagerTest, SAInterceptorMultiThreadRace_0100, TestSize.Level1)
{
    SAInterceptorManager::GetInstance().saInterceptors_.clear();
    constexpr int32_t INTERCEPTOR_NUM = 8;
    constexpr int32_t ADDER_THREAD_NUM = 4;
    constexpr int32_t LOOP_TIMES = 200;
    std::vector<sptr<MockSAInterceptorWithObj>> interceptors;
    for (int32_t i = 0; i < INTERCEPTOR_NUM; i++) {
        interceptors.push_back(sptr<MockSAInterceptorWithObj>::MakeSptr(i));
    }

    std::atomic<bool> startFlag(false);
    std::atomic<int32_t> executeFailCount(0);
    // each interceptor is only added by its owner thread, removers and executors race with it
    auto addWorker = [&interceptors, &startFlag](int32_t from, int32_t to) {
        while (!startFlag.load()) {
            std::this_thread::yield();
        }
        for (int32_t loop = 0; loop < LOOP_TIMES; loop++) {
            for (int32_t i = from; i < to; i++) {
                SAInterceptorManager::GetInstance().AddSAInterceptor(interceptors[i]);
            }
        }
    };
    auto removeWorker = [&interceptors, &startFlag]() {
        while (!startFlag.load()) {
            std::this_thread::yield();
        }
        for (int32_t loop = 0; loop < LOOP_TIMES; loop++) {
            for (int32_t i = 0; i < INTERCEPTOR_NUM; i++) {
                SAInterceptorManager::GetInstance().RemoveSAInterceptor(interceptors[i]->AsObject());
            }
        }
    };
    auto executeWorker = [&startFlag, &executeFailCount]() {
        while (!startFlag.load()) {
            std::this_thread::yield();
        }
        std::string params = "{}";
        for (int32_t loop = 0; loop < LOOP_TIMES; loop++) {
            Rule rule;
            if (SAInterceptorManager::GetInstance().ExecuteSAInterceptor(params, rule) != ERR_OK) {
                executeFailCount.fetch_add(1);
            }
        }
    };

    std::vector<std::thread> threads;
    for (int32_t t = 0; t < ADDER_THREAD_NUM; t++) {
        int32_t from = t * INTERCEPTOR_NUM / ADDER_THREAD_NUM;
        int32_t to = (t + 1) * INTERCEPTOR_NUM / ADDER_THREAD_NUM;
        threads.emplace_back(addWorker, from, to);
    }
    threads.emplace_back(removeWorker);
    threads.emplace_back(removeWorker);
    threads.emplace_back(executeWorker);
    threads.emplace_back(executeWorker);

    startFlag.store(true);
    for (auto &threadItem : threads) {
        threadItem.join();
    }

    // consistency check: each interceptor appears at most once after the race
    std::map<const IRemoteObject *, int32_t> counter;
    {
        std::lock_guard<std::mutex> lock(SAInterceptorManager::GetInstance().saInterceptorLock_);
        EXPECT_LE(SAInterceptorManager::GetInstance().saInterceptors_.size(),
            static_cast<size_t>(INTERCEPTOR_NUM));
        for (auto &item : SAInterceptorManager::GetInstance().saInterceptors_) {
            counter[item->AsObject().GetRefPtr()]++;
        }
    }
    for (auto &entry : counter) {
        EXPECT_EQ(entry.second, 1);
    }
    EXPECT_EQ(executeFailCount.load(), 0);

    // manager stays functional after the race: remove remaining, then add/execute/remove again
    for (auto &item : interceptors) {
        SAInterceptorManager::GetInstance().RemoveSAInterceptor(item->AsObject());
    }
    EXPECT_TRUE(SAInterceptorManager::GetInstance().SAInterceptorListIsEmpty());
    EXPECT_EQ(SAInterceptorManager::GetInstance().AddSAInterceptor(interceptors[0]), ERR_OK);
    Rule rule;
    EXPECT_EQ(SAInterceptorManager::GetInstance().ExecuteSAInterceptor(std::string("{}"), rule), ERR_OK);
    EXPECT_EQ(SAInterceptorManager::GetInstance().RemoveSAInterceptor(interceptors[0]->AsObject()), ERR_OK);
    EXPECT_TRUE(SAInterceptorManager::GetInstance().SAInterceptorListIsEmpty());
}
} // namespace AbilityRuntime
} // namespace OHOS