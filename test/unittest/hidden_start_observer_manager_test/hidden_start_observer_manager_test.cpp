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
#include "hidden_start_observer_manager.h"
#undef private

#include "ability_record.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "token.h"
#include "ihidden_start_observer.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AppExecFwk {

class HiddenStartObserverManagerTest : public testing::Test {
public:
    HiddenStartObserverManagerTest()
    {}
    ~HiddenStartObserverManagerTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<Token> MockToken();
};

void HiddenStartObserverManagerTest::SetUpTestCase(void) {}

void HiddenStartObserverManagerTest::TearDownTestCase(void) {}

void HiddenStartObserverManagerTest::SetUp(void) {}

void HiddenStartObserverManagerTest::TearDown(void)
{}

sptr<Token> HiddenStartObserverManagerTest::MockToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

class IHiddenStartObserverMock : public IHiddenStartObserver {
public:
    IHiddenStartObserverMock() = default;
    virtual ~IHiddenStartObserverMock() = default;
    
    bool IsHiddenStart(int32_t pid) override
    {
        return mockReturnValue;
    }
    
    sptr<IRemoteObject> AsObject() override
    {
        return mockObject;
    }
    
    void SetMockObject(sptr<IRemoteObject> object)
    {
        mockObject = object;
    }
    
    void SetMockReturnValue(bool value)
    {
        mockReturnValue = value;
    }
    
    void UseCustomRemote(bool value)
    {
        useCustomRemote = value;
    }

private:
    sptr<IRemoteObject> mockObject = nullptr;
    bool mockReturnValue = false;
    bool useCustomRemote = false;
};

class CustomRemoteObject : public IRemoteObject {
public:
    CustomRemoteObject(sptr<IRemoteObject> target, sptr<IHiddenStartObserverMock> observer)
        : target_(target), observer_(observer) {}
    
    virtual ~CustomRemoteObject() = default;
    
    sptr<IRemoteObject> promote() const
    {
        return target_;
    }
    template<typename I>
    sptr<I> GetInterfaceByType() const
    {
        return sptr<I>(static_cast<I*>(observer_.GetRefPtr()));
    }
    bool CheckObjectLegality() const override
    {
        return target_->CheckObjectLegality();
    }
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return target_->AddDeathRecipient(recipient);
    }
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        return target_->RemoveDeathRecipient(recipient);
    }
    bool AddRefreshRecipient(const sptr<RefreshRecipient> &recipient) override
    {
        return true;
    }
    bool RemoveRefreshRecipient(const sptr<RefreshRecipient> &recipient) override
    {
        return true;
    }
    bool IsProxyObject() const override
    {
        return target_->IsProxyObject();
    }
    int GetObjectRefCount() override
    {
        return target_->GetObjectRefCount();
    }
    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        return target_->Dump(fd, args);
    }
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return target_->SendRequest(code, data, reply, option);
    }
    sptr<IRemoteObject> target_;
    sptr<IHiddenStartObserverMock> observer_;
};

/**
 * @tc.number: RegisterObserver_001
 * @tc.name: RegisterObserver
 * @tc.desc: Test RegisterObserver with null observer.
 */
HWTEST_F(HiddenStartObserverManagerTest, RegisterObserver_001, TestSize.Level1)
{
    sptr<IHiddenStartObserver> observer = nullptr;
    int32_t result = HiddenStartObserverManager::GetInstance().RegisterObserver(observer);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.number: RegisterObserver_002
 * @tc.name: RegisterObserver
 * @tc.desc: Test RegisterObserver with valid observer.
 */
HWTEST_F(HiddenStartObserverManagerTest, RegisterObserver_002, TestSize.Level1)
{
    sptr<IHiddenStartObserverMock> observer = new IHiddenStartObserverMock();
    sptr<Token> token = MockToken();
    observer->SetMockObject(token);
    int32_t result = HiddenStartObserverManager::GetInstance().RegisterObserver(observer);
    EXPECT_EQ(result, ERR_OK);
    HiddenStartObserverManager::GetInstance().UnregisterObserver(observer);
}

/**
 * @tc.number: RegisterObserver_003
 * @tc.name: RegisterObserver
 * @tc.desc: Test RegisterObserver with observer that already exists.
 */
HWTEST_F(HiddenStartObserverManagerTest, RegisterObserver_003, TestSize.Level1)
{
    sptr<IHiddenStartObserverMock> observer = new IHiddenStartObserverMock();
    sptr<Token> token = MockToken();
    observer->SetMockObject(token);
    int32_t result1 = HiddenStartObserverManager::GetInstance().RegisterObserver(observer);
    EXPECT_EQ(result1, ERR_OK);
    int32_t result2 = HiddenStartObserverManager::GetInstance().RegisterObserver(observer);
    EXPECT_EQ(result2, ERR_INVALID_VALUE);
    HiddenStartObserverManager::GetInstance().UnregisterObserver(observer);
}

/**
 * @tc.number: UnregisterObserver_001
 * @tc.name: UnregisterObserver
 * @tc.desc: Test UnregisterObserver with null observer.
 */
HWTEST_F(HiddenStartObserverManagerTest, UnregisterObserver_001, TestSize.Level1)
{
    sptr<IHiddenStartObserver> observer = nullptr;
    int32_t result = HiddenStartObserverManager::GetInstance().UnregisterObserver(observer);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.number: UnregisterObserver_002
 * @tc.name: UnregisterObserver
 * @tc.desc: Test UnregisterObserver with observer that doesn't exist.
 */
HWTEST_F(HiddenStartObserverManagerTest, UnregisterObserver_002, TestSize.Level1)
{
    sptr<IHiddenStartObserverMock> observer = new IHiddenStartObserverMock();
    sptr<Token> token = MockToken();
    observer->SetMockObject(token);
    int32_t result = HiddenStartObserverManager::GetInstance().UnregisterObserver(observer);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.number: UnregisterObserver_003
 * @tc.name: UnregisterObserver
 * @tc.desc: Test UnregisterObserver with valid observer.
 */
HWTEST_F(HiddenStartObserverManagerTest, UnregisterObserver_003, TestSize.Level1)
{
    sptr<IHiddenStartObserverMock> observer = new IHiddenStartObserverMock();
    sptr<Token> token = MockToken();
    observer->SetMockObject(token);
    HiddenStartObserverManager::GetInstance().RegisterObserver(observer);
    int32_t result = HiddenStartObserverManager::GetInstance().UnregisterObserver(observer);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: IsHiddenStart_001
 * @tc.name: IsHiddenStart
 * @tc.desc: Test IsHiddenStart with no observers.
 */
HWTEST_F(HiddenStartObserverManagerTest, IsHiddenStart_001, TestSize.Level1)
{
    sptr<IHiddenStartObserverMock> observer = new IHiddenStartObserverMock();
    sptr<Token> token = MockToken();
    observer->SetMockObject(token);
    int32_t result1 = HiddenStartObserverManager::GetInstance().RegisterObserver(observer);
    EXPECT_EQ(result1, ERR_OK);
    HiddenStartObserverSet observers = HiddenStartObserverManager::GetInstance().GetObserversCopy();
    for (auto it = observers.begin(); it != observers.end(); ++it) {
        HiddenStartObserverManager::GetInstance().UnregisterObserver(*it);
    }
    bool result2 = HiddenStartObserverManager::GetInstance().IsHiddenStart(12345);
    EXPECT_FALSE(result2);
}

/**
 * @tc.number: IsHiddenStart_002
 * @tc.name: IsHiddenStart
 * @tc.desc: Test IsHiddenStart with one observer returning false.
 */
HWTEST_F(HiddenStartObserverManagerTest, IsHiddenStart_002, TestSize.Level1)
{
    HiddenStartObserverSet observers = HiddenStartObserverManager::GetInstance().GetObserversCopy();
    for (auto it = observers.begin(); it != observers.end(); ++it) {
        HiddenStartObserverManager::GetInstance().UnregisterObserver(*it);
    }
    sptr<IHiddenStartObserverMock> observer = new IHiddenStartObserverMock();
    sptr<Token> token = MockToken();
    observer->SetMockObject(token);
    observer->SetMockReturnValue(false);
    HiddenStartObserverManager::GetInstance().RegisterObserver(observer);
    bool result = HiddenStartObserverManager::GetInstance().IsHiddenStart(12345);
    EXPECT_FALSE(result);
    HiddenStartObserverManager::GetInstance().UnregisterObserver(observer);
}

/**
 * @tc.number: IsHiddenStart_003
 * @tc.name: IsHiddenStart
 * @tc.desc: Test IsHiddenStart with one observer returning true.
 */
HWTEST_F(HiddenStartObserverManagerTest, IsHiddenStart_003, TestSize.Level1)
{
    HiddenStartObserverSet observers = HiddenStartObserverManager::GetInstance().GetObserversCopy();
    for (auto it = observers.begin(); it != observers.end(); ++it) {
        HiddenStartObserverManager::GetInstance().UnregisterObserver(*it);
    }
    sptr<IHiddenStartObserverMock> observer = new IHiddenStartObserverMock();
    sptr<Token> token = MockToken();
    observer->SetMockObject(token);
    observer->SetMockReturnValue(true);
    HiddenStartObserverManager::GetInstance().RegisterObserver(observer);
    bool result = HiddenStartObserverManager::GetInstance().IsHiddenStart(12345);
    EXPECT_TRUE(result);
    HiddenStartObserverManager::GetInstance().UnregisterObserver(observer);
}

/**
 * @tc.number: ObserverExist_001
 * @tc.name: ObserverExist
 * @tc.desc: Test ObserverExist with null observer.
 */
HWTEST_F(HiddenStartObserverManagerTest, ObserverExist_001, TestSize.Level1)
{
    sptr<IRemoteBroker> observer = nullptr;
    bool result = HiddenStartObserverManager::GetInstance().ObserverExist(observer);
    EXPECT_FALSE(result);
}

/**
 * @tc.number: ObserverExist_002
 * @tc.name: ObserverExist
 * @tc.desc: Test ObserverExist with observer that doesn't exist.
 */
HWTEST_F(HiddenStartObserverManagerTest, ObserverExist_002, TestSize.Level1)
{
    sptr<IHiddenStartObserverMock> observer = new IHiddenStartObserverMock();
    sptr<Token> token = MockToken();
    observer->SetMockObject(token);
    bool result = HiddenStartObserverManager::GetInstance().ObserverExist(observer);
    EXPECT_FALSE(result);
}

/**
 * @tc.number: ObserverExist_003
 * @tc.name: ObserverExist
 * @tc.desc: Test ObserverExist with observer that exists.
 */
HWTEST_F(HiddenStartObserverManagerTest, ObserverExist_003, TestSize.Level1)
{
    sptr<IHiddenStartObserverMock> observer = new IHiddenStartObserverMock();
    sptr<Token> token = MockToken();
    observer->SetMockObject(token);
    HiddenStartObserverManager::GetInstance().RegisterObserver(observer);
    bool result = HiddenStartObserverManager::GetInstance().ObserverExist(observer);
    EXPECT_TRUE(result);
    HiddenStartObserverManager::GetInstance().UnregisterObserver(observer);
}

/**
 * @tc.number: AddObserverDeathRecipient_001
 * @tc.name: AddObserverDeathRecipient
 * @tc.desc: Test AddObserverDeathRecipient with null observer.
 */
HWTEST_F(HiddenStartObserverManagerTest, AddObserverDeathRecipient_001, TestSize.Level1)
{
    sptr<IRemoteBroker> observer = nullptr;
    HiddenStartObserverManager::GetInstance().AddObserverDeathRecipient(observer);
    EXPECT_EQ(HiddenStartObserverManager::GetInstance().recipientMap_.size(), 0);
}

/**
 * @tc.number: AddObserverDeathRecipient_002
 * @tc.name: AddObserverDeathRecipient
 * @tc.desc: Test AddObserverDeathRecipient with valid observer.
 */
HWTEST_F(HiddenStartObserverManagerTest, AddObserverDeathRecipient_002, TestSize.Level1)
{
    sptr<IHiddenStartObserverMock> observer = new IHiddenStartObserverMock();
    sptr<Token> token = MockToken();
    observer->SetMockObject(token);
    HiddenStartObserverManager::GetInstance().recipientMap_.clear();
    HiddenStartObserverManager::GetInstance().AddObserverDeathRecipient(observer);
    EXPECT_EQ(HiddenStartObserverManager::GetInstance().recipientMap_.size(), 1);
    HiddenStartObserverManager::GetInstance().RemoveObserverDeathRecipient(observer);
}

/**
 * @tc.number: RemoveObserverDeathRecipient_001
 * @tc.name: RemoveObserverDeathRecipient
 * @tc.desc: Test RemoveObserverDeathRecipient with observer null.
 */
HWTEST_F(HiddenStartObserverManagerTest, RemoveObserverDeathRecipient_001, TestSize.Level1)
{
    sptr<IHiddenStartObserverMock> observer = new IHiddenStartObserverMock();
    sptr<Token> token = MockToken();
    observer->SetMockObject(token);
    HiddenStartObserverManager::GetInstance().AddObserverDeathRecipient(observer);
    EXPECT_EQ(HiddenStartObserverManager::GetInstance().recipientMap_.size(), 1);
    observer->SetMockObject(nullptr);
    HiddenStartObserverManager::GetInstance().RemoveObserverDeathRecipient(observer);
    EXPECT_EQ(HiddenStartObserverManager::GetInstance().recipientMap_.size(), 1);
    observer->SetMockObject(token);
    HiddenStartObserverManager::GetInstance().RemoveObserverDeathRecipient(observer);
    EXPECT_EQ(HiddenStartObserverManager::GetInstance().recipientMap_.size(), 0);
}

/**
 * @tc.number: RemoveObserverDeathRecipient_002
 * @tc.name: RemoveObserverDeathRecipient
 * @tc.desc: Test RemoveObserverDeathRecipient with valid observer.
 */
HWTEST_F(HiddenStartObserverManagerTest, RemoveObserverDeathRecipient_002, TestSize.Level1)
{
    sptr<IHiddenStartObserverMock> observer = new IHiddenStartObserverMock();
    sptr<Token> token = MockToken();
    observer->SetMockObject(token);
    HiddenStartObserverManager::GetInstance().AddObserverDeathRecipient(observer);
    EXPECT_EQ(HiddenStartObserverManager::GetInstance().recipientMap_.size(), 1);
    HiddenStartObserverManager::GetInstance().RemoveObserverDeathRecipient(observer);
    EXPECT_EQ(HiddenStartObserverManager::GetInstance().recipientMap_.size(), 0);
}

/**
 * @tc.number: OnObserverDied_001
 * @tc.name: OnObserverDied
 * @tc.desc: Test OnObserverDied with null remote object.
 */
HWTEST_F(HiddenStartObserverManagerTest, OnObserverDied_001, TestSize.Level1)
{
    sptr<IHiddenStartObserverMock> observer = new IHiddenStartObserverMock();
    sptr<Token> token = MockToken();
    observer->SetMockObject(token);
    HiddenStartObserverManager::GetInstance().RegisterObserver(observer);
    EXPECT_TRUE(HiddenStartObserverManager::GetInstance().ObserverExist(observer));
    wptr<IRemoteObject> remote = nullptr;
    HiddenStartObserverManager::GetInstance().OnObserverDied(remote);
    EXPECT_TRUE(HiddenStartObserverManager::GetInstance().ObserverExist(observer));
}
}  // namespace AppExecFwk
}  // namespace OHOS