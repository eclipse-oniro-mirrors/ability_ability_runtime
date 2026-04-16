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
#include "query_erms_observer_manager.h"
#undef private
#include "singleton.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
using AtomicServiceStartupRule = OHOS::AbilityRuntime::AtomicServiceStartupRule;
namespace OHOS {
namespace AAFwk {

class MockIRemoteObject : public IRemoteObject {
public:
    static sptr<MockIRemoteObject> instance;
    MockIRemoteObject() : IRemoteObject(u"mock_i_remote_object") {}
    ~MockIRemoteObject() {}
    int32_t GetObjectRefCount() override
    {
        return 0;
    }
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        return ERR_OK;
    }
    bool IsProxyObject() const override
    {
        return true;
    }
    bool CheckObjectLegality() const override
    {
        return true;
    }
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return true;
    }
    bool AddRefreshRecipient(const sptr<RefreshRecipient> &recipient) override
    {
        return true;
    }
    bool RemoveRefreshRecipient(const sptr<RefreshRecipient> &recipient) override
    {
        return true;
    }
    bool Marshalling(Parcel &parcel) const override
    {
        return true;
    }
    sptr<IRemoteBroker> AsInterface() override
    {
        return nullptr;
    }
    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        return 0;
    }
    std::u16string GetObjectDescriptor() const
    {
        std::u16string descriptor = std::u16string();
        return descriptor;
    }
};

class IQueryERMSObserverMock : public IQueryERMSObserver {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.IQueryERMSObserver");

    void OnQueryFinished(const std::string &appId, const std::string &startTime,
                         const AtomicServiceStartupRule &rule, int resultCode)
    {
        return;
    }
    sptr<IRemoteObject> AsObject()
    {
        return nullptr;
    }
    enum {
        ON_QUERY_FINISHED = 1,
    };
};

class MockDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    enum {
        ADD_DEATH_RECIPIENT,
        REMOVE_DEATH_RECIPIENT,
        NOTICE_DEATH_RECIPIENT,
        TEST_SERVICE_DEATH_RECIPIENT,
        TEST_DEVICE_DEATH_RECIPIENT,
    };
    void OnRemoteDied(const wptr<IRemoteObject> &remote)
    {
        return;
    }
};

class QueryERMSObserverManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void QueryERMSObserverManagerTest::SetUpTestCase()
{
}

void QueryERMSObserverManagerTest::TearDownTestCase()
{
}

void QueryERMSObserverManagerTest::SetUp()
{
}

void QueryERMSObserverManagerTest::TearDown()
{
}

/*
 * @tc.number: AddObserver_0100
 * @tc.name: AddObserver
 * @tc.desc: Verify AddObserver
 */
HWTEST_F(QueryERMSObserverManagerTest, AddObserver_0100, TestSize.Level1)
{
    int32_t recordId = 1;
    sptr<IQueryERMSObserver> observer = nullptr;
    auto result = QueryERMSObserverManager::GetInstance().AddObserver(recordId, observer);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    observer = sptr<IQueryERMSObserverMock>::MakeSptr();
    EXPECT_EQ(QueryERMSObserverManager::GetInstance().observerMap_.size(), 0);
    result = QueryERMSObserverManager::GetInstance().AddObserver(recordId, observer);
    EXPECT_EQ(QueryERMSObserverManager::GetInstance().observerMap_.size(), 1);

    QueryERMSObserverManager::GetInstance().deathRecipient_ = nullptr;
    result = QueryERMSObserverManager::GetInstance().AddObserver(recordId, observer);
    EXPECT_NE(QueryERMSObserverManager::GetInstance().deathRecipient_, nullptr);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * @tc.number: OnQueryFinished_0100
 * @tc.name: OnQueryFinished
 * @tc.desc: Verify OnQueryFinished
 */
HWTEST_F(QueryERMSObserverManagerTest, OnQueryFinished_0100, TestSize.Level1)
{
    int32_t recordId = 1;
    std::string appId = "appId";
    std::string startTime = "12:00";
    AbilityRuntime::AtomicServiceStartupRule rule;
    int resultCode = 1;

    DelayedSingleton<OHOS::AAFwk::AbilityManagerService>::GetInstance()->taskHandler_ = nullptr;
    QueryERMSObserverManager::GetInstance().OnQueryFinished(recordId,
        appId, startTime, rule, resultCode);
    EXPECT_NE(&QueryERMSObserverManager::GetInstance(), nullptr);
}

/*
 * @tc.number: HandleOnQueryFinished_0100
 * @tc.name: HandleOnQueryFinished
 * @tc.desc: Verify HandleOnQueryFinished
 */
HWTEST_F(QueryERMSObserverManagerTest, HandleOnQueryFinished_0100, TestSize.Level1)
{
    int32_t recordId = 1;
    std::string appId = "appId";
    std::string startTime = "12:00";
    AbilityRuntime::AtomicServiceStartupRule rule;
    int resultCode = 1;

    sptr<IQueryERMSObserver> observer = sptr<IQueryERMSObserverMock>::MakeSptr();
    QueryERMSObserverManager::GetInstance().observerMap_.emplace(recordId, observer);
    QueryERMSObserverManager::GetInstance().HandleOnQueryFinished(recordId,
        appId, startTime, rule, resultCode);
    EXPECT_NE(&QueryERMSObserverManager::GetInstance(), nullptr);
}

/*
 * @tc.number: OnObserverDied_0100
 * @tc.name: OnObserverDied
 * @tc.desc: Verify OnObserverDied
 */
HWTEST_F(QueryERMSObserverManagerTest, OnObserverDied_0100, TestSize.Level1)
{
    wptr<IRemoteObject> remote = nullptr;
    QueryERMSObserverManager::GetInstance().OnObserverDied(remote);

    remote = sptr<MockIRemoteObject>::MakeSptr();
    sptr<IQueryERMSObserver> observer = sptr<IQueryERMSObserverMock>::MakeSptr();
    QueryERMSObserverManager::GetInstance().observerMap_.emplace(1, observer);
    EXPECT_NE(QueryERMSObserverManager::GetInstance().observerMap_.size(), 0);
    QueryERMSObserverManager::GetInstance().OnObserverDied(remote);
    EXPECT_NE(&QueryERMSObserverManager::GetInstance(), nullptr);
}
} // namespace AAFwk
} // namespace OHOS