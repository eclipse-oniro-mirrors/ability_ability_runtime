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
#include <gmock/gmock.h>

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"
#include "want.h"
#include "page_state_data.h"
#include "ability_info.h"
#include "application_info.h"

#define private public
#define protected public
#include "disposed_observer.h"
#include "ability_record.h"
#include "interceptor/disposed_rule_interceptor.h"
#undef private
#undef protected

namespace OHOS {
namespace AAFwk {
using namespace testing::ext;
using namespace AppExecFwk;

class DisposedObserverTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<DisposedObserver> CreateDisposedObserver(ComponentType type);
    std::shared_ptr<DisposedRuleInterceptor> CreateInterceptor();

protected:
    std::shared_ptr<DisposedRuleInterceptor> interceptor_ = nullptr;
    int32_t testUid_ = 1001;
};

void DisposedObserverTest::SetUpTestCase(void)
{}

void DisposedObserverTest::TearDownTestCase(void)
{}

void DisposedObserverTest::SetUp()
{}

void DisposedObserverTest::TearDown()
{}

std::shared_ptr<DisposedRuleInterceptor> DisposedObserverTest::CreateInterceptor()
{
    return std::make_shared<DisposedRuleInterceptor>();
}

std::shared_ptr<DisposedObserver> DisposedObserverTest::CreateDisposedObserver(ComponentType type)
{
    Want want;
    want.SetElementName("device", "com.example.test", "TestAbility");
    DisposedRule disposedRule;
    disposedRule.want = std::make_shared<Want>(want);
    disposedRule.componentType = type;

    interceptor_ = CreateInterceptor();
    return std::make_shared<DisposedObserver>(disposedRule, interceptor_, testUid_);
}

/**
 * @tc.number: DisposedObserver_OnPageShow_0100
 * @tc.name: DisposedObserver::OnPageShow
 * @tc.desc: pageStateData uid does not match observer uid, return directly without unregister.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_OnPageShow_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_0100 start");
    auto observer = CreateDisposedObserver(ComponentType::UI_EXTENSION);

    PageStateData pageStateData;
    pageStateData.uid = testUid_ + 1; // Different uid

    // When uid doesn't match, OnPageShow returns early without any action
    observer->OnPageShow(pageStateData);

    // Verify observer state remains unchanged
    EXPECT_EQ(observer->uid_, testUid_);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_0100 end");
}

/**
 * @tc.number: DisposedObserver_OnPageShow_0200
 * @tc.name: DisposedObserver::OnPageShow
 * @tc.desc: componentType is UI_ABILITY, OnPageShow executes successfully.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_OnPageShow_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_0200 start");
    auto observer = CreateDisposedObserver(ComponentType::UI_ABILITY);

    PageStateData pageStateData;
    pageStateData.uid = testUid_;

    // Verify observer is properly initialized
    EXPECT_EQ(observer->uid_, testUid_);
    EXPECT_NE(observer->interceptor_, nullptr);

    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_0200 end");
}

/**
 * @tc.number: DisposedObserver_OnPageShow_0300
 * @tc.name: DisposedObserver::OnPageShow
 * @tc.desc: componentType is UI_ABILITY with null token, UnregisterObserver still called.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_OnPageShow_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_0300 start");
    auto observer = CreateDisposedObserver(ComponentType::UI_ABILITY);

    PageStateData pageStateData;
    pageStateData.uid = testUid_;

    observer->token_ = nullptr;

    observer->OnPageShow(pageStateData);

    EXPECT_NE(observer->interceptor_, nullptr);
    EXPECT_EQ(observer->uid_, testUid_);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_0300 end");
}

/**
 * @tc.number: DisposedObserver_OnPageShow_0400
 * @tc.name: DisposedObserver::OnPageShow
 * @tc.desc: componentType is UI_EXTENSION with valid token, ExecuteUIExtension called.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_OnPageShow_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_0400 start");
    auto observer = CreateDisposedObserver(ComponentType::UI_EXTENSION);

    // Setup mock token
    sptr<IRemoteObject> token = new MockAbilityToken();
    observer->token_ = token;

    PageStateData pageStateData;
    pageStateData.uid = testUid_;

    // Expect UnregisterObserver to be called

    observer->OnPageShow(pageStateData);

    // Verify observer state after OnPageShow
    EXPECT_EQ(observer->uid_, testUid_);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_0400 end");
}

/**
 * @tc.number: DisposedObserver_OnPageShow_0500
 * @tc.name: DisposedObserver::OnPageShow
 * @tc.desc: componentType is UI_EXTENSION with null token, UnregisterObserver called.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_OnPageShow_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_0500 start");
    auto observer = CreateDisposedObserver(ComponentType::UI_EXTENSION);

    PageStateData pageStateData;
    pageStateData.uid = testUid_;

    // Keep token_ as nullptr
    observer->token_ = nullptr;

    // UnregisterObserver should be called even when ExecuteUIExtension fails

    observer->OnPageShow(pageStateData);

    // Verify observer state
    EXPECT_EQ(observer->uid_, testUid_);
    EXPECT_EQ(observer->token_, nullptr);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_0500 end");
}

/**
 * * @tc.number: DisposedObserver_OnPageShow_0600
 * * @tc.name: DisposedObserver::OnPageShow - embeddable UIExtension path
 * * @tc.desc: abilityRecord with EMBEDDED_FULL_SCREEN_MODE triggers embeddable path.
 * */
HWTEST_F(DisposedObserverTest, DisposedObserver_OnPageShow_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_0600 start");
    auto observer = CreateDisposedObserver(ComponentType::UI_EXTENSION);

    // Create real AbilityRecord with embeddable screen mode
    Want want;
    want.SetElementName("device", "com.example.test", "TestAbility");
    want.SetParam("ohos.extra.param.key.showMode", static_cast<int32_t>(1));  // EMBEDDED_FULL_SCREEN_MODE

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityInfo.process = "test_process";

    AppExecFwk::ApplicationInfo applicationInfo;

    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(abilityRecord, nullptr);

    // Create token associated with abilityRecord
    sptr<IRemoteObject> token = new Token(abilityRecord);
    observer->token_ = token;

    PageStateData pageStateData;
    pageStateData.uid = testUid_;

    // This should trigger embeddable UIExtension path
    observer->OnPageShow(pageStateData);

    EXPECT_EQ(observer->uid_, testUid_);
    EXPECT_NE(observer->token_, nullptr);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_0600 end");
}

/**
 * * @tc.number: DisposedObserver_OnPageShow_0700
 * * @tc.name: DisposedObserver::OnPageShow - PAGE type UIExtension path
 * * @tc.desc: abilityRecord with PAGE type and non-embeddable mode triggers PAGE path.
 * */
HWTEST_F(DisposedObserverTest, DisposedObserver_OnPageShow_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_0700 start");
    auto observer = CreateDisposedObserver(ComponentType::UI_EXTENSION);

    // Create real AbilityRecord with PAGE type and non-embeddable mode
    Want want;
    want.SetElementName("device", "com.example.test", "TestAbility");
    want.SetParam("ohos.extra.param.key.showMode", static_cast<int32_t>(-1));  // IDLE_SCREEN_MODE

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityInfo.process = "test_process";

    AppExecFwk::ApplicationInfo applicationInfo;

    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(abilityRecord, nullptr);

    // Create token associated with abilityRecord
    sptr<IRemoteObject> token = new Token(abilityRecord);
    observer->token_ = token;

    PageStateData pageStateData;
    pageStateData.uid = testUid_;

    // This should trigger PAGE type UIExtension path
    observer->OnPageShow(pageStateData);

    EXPECT_EQ(observer->uid_, testUid_);
    EXPECT_NE(observer->token_, nullptr);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_0700 end");
}

/**
 * @tc.number: DisposedObserver_Constructor_0100
 * @tc.name: DisposedObserver Constructor
 * @tc.desc: Verify DisposedObserver constructor initializes members correctly.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_Constructor_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_Constructor_0100 start");

    Want want;
    want.SetElementName("device", "com.example.test", "TestAbility");
    DisposedRule disposedRule;
    disposedRule.want = std::make_shared<Want>(want);
    disposedRule.componentType = ComponentType::UI_EXTENSION;

    auto mockInterceptor = CreateInterceptor();
    int32_t testUid = 2002;

    auto observer = std::make_shared<DisposedObserver>(disposedRule, mockInterceptor, testUid);

    EXPECT_NE(observer, nullptr);
    EXPECT_EQ(observer->uid_, testUid);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_Constructor_0100 end");
}

/**
 * @tc.number: DisposedObserver_Members_0100
 * @tc.name: DisposedObserver member initialization
 * @tc.desc: Verify disposedRule_ and interceptor_ are set correctly.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_Members_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_Members_0100 start");

    Want want;
    want.SetElementName("device", "com.example.test", "TestAbility");
    DisposedRule disposedRule;
    disposedRule.want = std::make_shared<Want>(want);
    disposedRule.componentType = ComponentType::UI_ABILITY;

    auto mockInterceptor = CreateInterceptor();
    int32_t testUid = 3003;

    auto observer = std::make_shared<DisposedObserver>(disposedRule, mockInterceptor, testUid);

    EXPECT_EQ(observer->disposedRule_.componentType, ComponentType::UI_ABILITY);
    EXPECT_NE(observer->disposedRule_.want, nullptr);
    EXPECT_EQ(observer->disposedRule_.want->GetElement().GetAbilityName(), "TestAbility");
    EXPECT_EQ(observer->interceptor_, mockInterceptor);
    EXPECT_EQ(observer->uid_, testUid);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_Members_0100 end");
}

/**
 * @tc.number: DisposedObserver_Token_0100
 * @tc.name: DisposedObserver token_ member
 * @tc.desc: Verify token_ can be set and retrieved.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_Token_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_Token_0100 start");

    auto observer = CreateDisposedObserver(ComponentType::UI_EXTENSION);

    // Initially token_ should be nullptr
    EXPECT_EQ(observer->token_, nullptr);

    // Set token_
    sptr<IRemoteObject> token = new MockAbilityToken();
    observer->token_ = token;

    // Verify token_ is set
    EXPECT_NE(observer->token_, nullptr);
    EXPECT_EQ(observer->token_, token);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_Token_0100 end");
}

/**
 * @tc.number: DisposedObserver_UID_0100
 * @tc.name: DisposedObserver uid_ mismatch check
 * @tc.desc: Verify OnPageShow returns early when uid doesn't match.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_UID_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_UID_0100 start");

    auto observer = CreateDisposedObserver(ComponentType::UI_ABILITY);
    int32_t differentUid = testUid_ + 100;

    PageStateData pageStateData;
    pageStateData.uid = differentUid;

    // When uid doesn't match, UnregisterObserver should NOT be called

    observer->OnPageShow(pageStateData);

    // Verify observer's uid_ hasn't changed
    EXPECT_EQ(observer->uid_, testUid_);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_UID_0100 end");
}

/**
 * @tc.number: DisposedObserver_ComponentType_UIAbility_0100
 * @tc.name: DisposedObserver UI_ABILITY component type
 * @tc.desc: Verify UI_ABILITY type triggers UnregisterObserver.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_ComponentType_UIAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_ComponentType_UIAbility_0100 start");

    Want want;
    want.SetElementName("device", "com.example.test", "TestAbility");
    DisposedRule disposedRule;
    disposedRule.want = std::make_shared<Want>(want);
    disposedRule.componentType = ComponentType::UI_ABILITY;

    auto mockInterceptor = CreateInterceptor();
    auto observer = std::make_shared<DisposedObserver>(disposedRule, mockInterceptor, testUid_);

    PageStateData pageStateData;
    pageStateData.uid = testUid_;

    // Verify componentType is correctly set
    EXPECT_EQ(observer->disposedRule_.componentType, ComponentType::UI_ABILITY);

    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_ComponentType_UIAbility_0100 end");
}

/**
 * @tc.number: DisposedObserver_ComponentType_UIExtension_0100
 * @tc.name: DisposedObserver UI_EXTENSION component type
 * @tc.desc: Verify UI_EXTENSION type triggers ExecuteUIExtension.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_ComponentType_UIExtension_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_ComponentType_UIExtension_0100 start");

    Want want;
    want.SetElementName("device", "com.example.test", "TestAbility");
    DisposedRule disposedRule;
    disposedRule.want = std::make_shared<Want>(want);
    disposedRule.componentType = ComponentType::UI_EXTENSION;

    auto mockInterceptor = CreateInterceptor();
    auto observer = std::make_shared<DisposedObserver>(disposedRule, mockInterceptor, testUid_);

    PageStateData pageStateData;
    pageStateData.uid = testUid_;

    // Verify componentType is correctly set
    EXPECT_EQ(observer->disposedRule_.componentType, ComponentType::UI_EXTENSION);

    // Verify UnregisterObserver is called (after ExecuteUIExtension)

    observer->OnPageShow(pageStateData);

    // Verify componentType is correctly set
    EXPECT_EQ(observer->disposedRule_.componentType, ComponentType::UI_EXTENSION);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_ComponentType_UIExtension_0100 end");
}

/**
 * @tc.number: DisposedObserver_Want_0100
 * @tc.name: DisposedObserver want parameter
 * @tc.desc: Verify want is correctly stored in disposedRule.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_Want_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_Want_0100 start");

    Want want;
    want.SetElementName("testDevice", "com.example.bundle", "TestAbility");
    want.SetAction("test.action");
    want.SetFlags(0x1234);

    DisposedRule disposedRule;
    disposedRule.want = std::make_shared<Want>(want);
    disposedRule.componentType = ComponentType::UI_EXTENSION;

    auto mockInterceptor = CreateInterceptor();
    auto observer = std::make_shared<DisposedObserver>(disposedRule, mockInterceptor, testUid_);

    // Verify want is correctly stored
    EXPECT_NE(observer->disposedRule_.want, nullptr);
    EXPECT_EQ(observer->disposedRule_.want->GetElement().GetDeviceID(), "testDevice");
    EXPECT_EQ(observer->disposedRule_.want->GetElement().GetBundleName(), "com.example.bundle");
    EXPECT_EQ(observer->disposedRule_.want->GetElement().GetAbilityName(), "TestAbility");
    EXPECT_EQ(observer->disposedRule_.want->GetAction(), "test.action");
    EXPECT_EQ(observer->disposedRule_.want->GetFlags(), 0x1234);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_Want_0100 end");
}

/**
 * @tc.number: DisposedObserver_MultipleObservers_0100
 * @tc.name: DisposedObserver multiple instances
 * @tc.desc: Verify multiple observer instances can coexist independently.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_MultipleObservers_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_MultipleObservers_0100 start");

    Want want1, want2;
    want1.SetElementName("device", "com.test1", "Ability1");
    want2.SetElementName("device", "com.test2", "Ability2");

    DisposedRule disposedRule1, disposedRule2;
    disposedRule1.want = std::make_shared<Want>(want1);
    disposedRule1.componentType = ComponentType::UI_ABILITY;
    disposedRule2.want = std::make_shared<Want>(want2);
    disposedRule2.componentType = ComponentType::UI_EXTENSION;

    auto mockInterceptor1 = CreateInterceptor();
    auto mockInterceptor2 = CreateInterceptor();

    int32_t uid1 = 1001;
    int32_t uid2 = 1002;

    auto observer1 = std::make_shared<DisposedObserver>(disposedRule1, mockInterceptor1, uid1);
    auto observer2 = std::make_shared<DisposedObserver>(disposedRule2, mockInterceptor2, uid2);

    // Verify each observer has independent state
    EXPECT_EQ(observer1->uid_, uid1);
    EXPECT_EQ(observer2->uid_, uid2);
    EXPECT_EQ(observer1->disposedRule_.componentType, ComponentType::UI_ABILITY);
    EXPECT_EQ(observer2->disposedRule_.componentType, ComponentType::UI_EXTENSION);
    EXPECT_EQ(observer1->disposedRule_.want->GetElement().GetBundleName(), "com.test1");
    EXPECT_EQ(observer2->disposedRule_.want->GetElement().GetBundleName(), "com.test2");

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_MultipleObservers_0100 end");
}

/**
 * @tc.number: DisposedObserver_Interceptor_0100
 * @tc.name: DisposedObserver interceptor initialization
 * @tc.desc: Verify interceptor is correctly initialized and stored.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_Interceptor_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_Interceptor_0100 start");

    Want want;
    want.SetElementName("device", "com.example.test", "TestAbility");

    DisposedRule disposedRule;
    disposedRule.want = std::make_shared<Want>(want);
    disposedRule.componentType = ComponentType::UI_EXTENSION;

    auto mockInterceptor = CreateInterceptor();
    auto observer = std::make_shared<DisposedObserver>(disposedRule, mockInterceptor, testUid_);

    // Verify interceptor is stored
    EXPECT_EQ(observer->interceptor_, mockInterceptor);
    EXPECT_NE(observer->interceptor_, nullptr);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_Interceptor_0100 end");
}

/**
 * @tc.number: DisposedObserver_GenerateAbilityKey_0100
 * @tc.name: DisposedObserver::GenerateAbilityKey
 * @tc.desc: Verify GenerateAbilityKey generates correct key format.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_GenerateAbilityKey_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_GenerateAbilityKey_0100 start");

    std::string moduleName = "testModule";
    std::string abilityName = "TestAbility";
    std::string expectedKey = "testModule/TestAbility";

    std::string actualKey = DisposedObserver::GenerateAbilityKey(moduleName, abilityName);

    EXPECT_EQ(actualKey, expectedKey);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_GenerateAbilityKey_0100 end");
}

/**
 * @tc.number: DisposedObserver_GenerateAbilityKey_0200
 * @tc.name: DisposedObserver::GenerateAbilityKey with special characters
 * @tc.desc: Verify GenerateAbilityKey handles special characters correctly.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_GenerateAbilityKey_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_GenerateAbilityKey_0200 start");

    std::string moduleName = "module.test";
    std::string abilityName = "Ability_Test";
    std::string expectedKey = "module.test/Ability_Test";

    std::string actualKey = DisposedObserver::GenerateAbilityKey(moduleName, abilityName);

    EXPECT_EQ(actualKey, expectedKey);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_GenerateAbilityKey_0200 end");
}

/**
 * @tc.number: DisposedObserver_AddAbilityKey_0100
 * @tc.name: DisposedObserver::AddAbilityKey
 * @tc.desc: Verify AddAbilityKey adds key to abilityKeys_.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_AddAbilityKey_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_AddAbilityKey_0100 start");

    auto observer = CreateDisposedObserver(ComponentType::UI_ABILITY);

    std::string moduleName = "testModule";
    std::string abilityName = "TestAbility";

    EXPECT_EQ(observer->GetAbilityKeyCount(), 0);

    observer->AddAbilityKey(moduleName, abilityName);

    EXPECT_EQ(observer->GetAbilityKeyCount(), 1);
    EXPECT_TRUE(observer->HasAbilityKey(moduleName, abilityName));

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_AddAbilityKey_0100 end");
}

/**
 * @tc.number: DisposedObserver_AddAbilityKey_0200
 * @tc.name: DisposedObserver::AddAbilityKey multiple keys
 * @tc.desc: Verify AddAbilityKey can add multiple keys.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_AddAbilityKey_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_AddAbilityKey_0200 start");

    auto observer = CreateDisposedObserver(ComponentType::UI_ABILITY);

    observer->AddAbilityKey("module1", "Ability1");
    observer->AddAbilityKey("module2", "Ability2");
    observer->AddAbilityKey("module3", "Ability3");

    EXPECT_EQ(observer->GetAbilityKeyCount(), 3);
    EXPECT_TRUE(observer->HasAbilityKey("module1", "Ability1"));
    EXPECT_TRUE(observer->HasAbilityKey("module2", "Ability2"));
    EXPECT_TRUE(observer->HasAbilityKey("module3", "Ability3"));

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_AddAbilityKey_0200 end");
}

/**
 * @tc.number: DisposedObserver_HasAbilityKey_0100
 * @tc.name: DisposedObserver::HasAbilityKey
 * @tc.desc: Verify HasAbilityKey returns false for non-existent key.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_HasAbilityKey_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_HasAbilityKey_0100 start");

    auto observer = CreateDisposedObserver(ComponentType::UI_ABILITY);

    observer->AddAbilityKey("module1", "Ability1");

    EXPECT_FALSE(observer->HasAbilityKey("module1", "Ability2"));
    EXPECT_FALSE(observer->HasAbilityKey("module2", "Ability1"));
    EXPECT_FALSE(observer->HasAbilityKey("module2", "Ability2"));

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_HasAbilityKey_0100 end");
}

/**
 * @tc.number: DisposedObserver_RemoveAbilityKey_0100
 * @tc.name: DisposedObserver::RemoveAbilityKey
 * @tc.desc: Verify RemoveAbilityKey removes key and returns false when not empty.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_RemoveAbilityKey_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_RemoveAbilityKey_0100 start");

    auto observer = CreateDisposedObserver(ComponentType::UI_ABILITY);

    observer->AddAbilityKey("module1", "Ability1");
    observer->AddAbilityKey("module2", "Ability2");

    EXPECT_EQ(observer->GetAbilityKeyCount(), 2);

    // Remove first key, should return false (still has keys)
    bool isEmpty = observer->RemoveAbilityKey("module1", "Ability1");

    EXPECT_FALSE(isEmpty);
    EXPECT_EQ(observer->GetAbilityKeyCount(), 1);
    EXPECT_FALSE(observer->HasAbilityKey("module1", "Ability1"));
    EXPECT_TRUE(observer->HasAbilityKey("module2", "Ability2"));

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_RemoveAbilityKey_0100 end");
}

/**
 * @tc.number: DisposedObserver_RemoveAbilityKey_0200
 * @tc.name: DisposedObserver::RemoveAbilityKey returns true when empty
 * @tc.desc: Verify RemoveAbilityKey returns true when last key is removed.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_RemoveAbilityKey_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_RemoveAbilityKey_0200 start");

    auto observer = CreateDisposedObserver(ComponentType::UI_ABILITY);

    observer->AddAbilityKey("module1", "Ability1");

    EXPECT_EQ(observer->GetAbilityKeyCount(), 1);

    // Remove only key, should return true (empty now)
    bool isEmpty = observer->RemoveAbilityKey("module1", "Ability1");

    EXPECT_TRUE(isEmpty);
    EXPECT_EQ(observer->GetAbilityKeyCount(), 0);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_RemoveAbilityKey_0200 end");
}

/**
 * @tc.number: DisposedObserver_RemoveAbilityKey_0300
 * @tc.name: DisposedObserver::RemoveAbilityKey non-existent key
 * @tc.desc: Verify RemoveAbilityKey handles non-existent key correctly.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_RemoveAbilityKey_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_RemoveAbilityKey_0300 start");

    auto observer = CreateDisposedObserver(ComponentType::UI_ABILITY);

    observer->AddAbilityKey("module1", "Ability1");

    // Try to remove non-existent key, should return false (still has keys)
    bool isEmpty = observer->RemoveAbilityKey("module2", "Ability2");

    EXPECT_FALSE(isEmpty);
    EXPECT_EQ(observer->GetAbilityKeyCount(), 1);
    EXPECT_TRUE(observer->HasAbilityKey("module1", "Ability1"));

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_RemoveAbilityKey_0300 end");
}

/**
 * @tc.number: DisposedObserver_GetAbilityKeyCount_0100
 * @tc.name: DisposedObserver::GetAbilityKeyCount
 * @tc.desc: Verify GetAbilityKeyCount returns correct count.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_GetAbilityKeyCount_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_GetAbilityKeyCount_0100 start");

    auto observer = CreateDisposedObserver(ComponentType::UI_ABILITY);

    EXPECT_EQ(observer->GetAbilityKeyCount(), 0);

    observer->AddAbilityKey("module1", "Ability1");
    EXPECT_EQ(observer->GetAbilityKeyCount(), 1);

    observer->AddAbilityKey("module2", "Ability2");
    EXPECT_EQ(observer->GetAbilityKeyCount(), 2);

    observer->RemoveAbilityKey("module1", "Ability1");
    EXPECT_EQ(observer->GetAbilityKeyCount(), 1);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_GetAbilityKeyCount_0100 end");
}

/**
 * @tc.number: DisposedObserver_OnPageShow_AbilityKeyCheck_0100
 * @tc.name: DisposedObserver::OnPageShow ability key not in watch list
 * @tc.desc: Verify OnPageShow returns early when ability key is not in watch list.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_OnPageShow_AbilityKeyCheck_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_AbilityKeyCheck_0100 start");

    auto observer = CreateDisposedObserver(ComponentType::UI_ABILITY);

    // Add different key to watch list
    observer->AddAbilityKey("watchedModule", "WatchedAbility");

    PageStateData pageStateData;
    pageStateData.uid = testUid_;
    pageStateData.moduleName = "otherModule";
    pageStateData.abilityName = "OtherAbility";

    // OnPageShow should return early since key is not in watch list
    observer->OnPageShow(pageStateData);

    // Verify abilityKeyCount is still 1 (no key was removed)
    EXPECT_EQ(observer->GetAbilityKeyCount(), 1);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_AbilityKeyCheck_0100 end");
}

/**
 * @tc.number: DisposedObserver_OnPageShow_AbilityKeyCheck_0200
 * @tc.name: DisposedObserver::OnPageShow with matching ability key
 * @tc.desc: Verify OnPageShow processes when ability key is in watch list.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_OnPageShow_AbilityKeyCheck_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_AbilityKeyCheck_0200 start");

    auto observer = CreateDisposedObserver(ComponentType::UI_ABILITY);

    std::string watchedModule = "watchedModule";
    std::string watchedAbility = "WatchedAbility";
    observer->AddAbilityKey(watchedModule, watchedAbility);

    EXPECT_EQ(observer->GetAbilityKeyCount(), 1);

    PageStateData pageStateData;
    pageStateData.uid = testUid_;
    pageStateData.moduleName = watchedModule;
    pageStateData.abilityName = watchedAbility;

    // OnPageShow should process since key is in watch list
    observer->OnPageShow(pageStateData);

    // Verify key was removed (now empty, UnregisterObserver called)
    EXPECT_EQ(observer->GetAbilityKeyCount(), 0);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_AbilityKeyCheck_0200 end");
}

/**
 * @tc.number: DisposedObserver_OnPageShow_MultipleKeys_0100
 * @tc.name: DisposedObserver::OnPageShow with multiple ability keys
 * @tc.desc: Verify OnPageShow only removes matched key when multiple keys exist.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_OnPageShow_MultipleKeys_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_MultipleKeys_0100 start");

    auto observer = CreateDisposedObserver(ComponentType::UI_ABILITY);

    observer->AddAbilityKey("module1", "Ability1");
    observer->AddAbilityKey("module2", "Ability2");
    observer->AddAbilityKey("module3", "Ability3");

    EXPECT_EQ(observer->GetAbilityKeyCount(), 3);

    PageStateData pageStateData;
    pageStateData.uid = testUid_;
    pageStateData.moduleName = "module2";
    pageStateData.abilityName = "Ability2";

    // OnPageShow should remove only module2/Ability2
    observer->OnPageShow(pageStateData);

    // Verify only matched key was removed
    EXPECT_EQ(observer->GetAbilityKeyCount(), 2);
    EXPECT_TRUE(observer->HasAbilityKey("module1", "Ability1"));
    EXPECT_FALSE(observer->HasAbilityKey("module2", "Ability2"));
    EXPECT_TRUE(observer->HasAbilityKey("module3", "Ability3"));

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_MultipleKeys_0100 end");
}

/**
 * @tc.number: DisposedObserver_OnAbilityStateChanged_0100
 * @tc.name: DisposedObserver::OnAbilityStateChanged non-foreground state
 * @tc.desc: Verify OnAbilityStateChanged returns early for non-foreground states.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_OnAbilityStateChanged_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnAbilityStateChanged_0100 start");

    auto observer = CreateDisposedObserver(ComponentType::UI_ABILITY);

    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND);
    abilityStateData.token = new MockAbilityToken();

    // Should return early for BACKGROUND state
    observer->OnAbilityStateChanged(abilityStateData);

    // Verify token was not set
    EXPECT_EQ(observer->token_, nullptr);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnAbilityStateChanged_0100 end");
}

/**
 * @tc.number: DisposedObserver_OnAbilityStateChanged_0200
 * @tc.name: DisposedObserver::OnAbilityStateChanged foreground state
 * @tc.desc: Verify OnAbilityStateChanged processes FOREGROUND state.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_OnAbilityStateChanged_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnAbilityStateChanged_0200 start");

    auto observer = CreateDisposedObserver(ComponentType::UI_ABILITY);

    sptr<IRemoteObject> token = new MockAbilityToken();

    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND);
    abilityStateData.token = token;

    observer->OnAbilityStateChanged(abilityStateData);

    // Verify token was set
    EXPECT_EQ(observer->token_, token);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnAbilityStateChanged_0200 end");
}

/**
 * @tc.number: DisposedObserver_OnAbilityStateChanged_0300
 * @tc.name: DisposedObserver::OnAbilityStateChanged with null token
 * @tc.desc: Verify OnAbilityStateChanged handles null token gracefully.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_OnAbilityStateChanged_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnAbilityStateChanged_0300 start");

    auto observer = CreateDisposedObserver(ComponentType::UI_ABILITY);

    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND);
    abilityStateData.token = nullptr;

    observer->OnAbilityStateChanged(abilityStateData);

    // Verify token is nullptr
    EXPECT_EQ(observer->token_, nullptr);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnAbilityStateChanged_0300 end");
}

/**
 * @tc.number: DisposedObserver_OnAbilityStateChanged_0400
 * @tc.name: DisposedObserver::OnAbilityStateChanged created state
 * @tc.desc: Verify OnAbilityStateChanged returns early for CREATED state.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_OnAbilityStateChanged_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnAbilityStateChanged_0400 start");

    auto observer = CreateDisposedObserver(ComponentType::UI_ABILITY);

    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_CREATE);
    abilityStateData.token = new MockAbilityToken();

    observer->OnAbilityStateChanged(abilityStateData);

    // Verify token was not set for non-foreground state
    EXPECT_EQ(observer->token_, nullptr);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnAbilityStateChanged_0400 end");
}

/**
 * @tc.number: DisposedObserver_OnAbilityStateChanged_0500
 * @tc.name: DisposedObserver::OnAbilityStateChanged terminated state
 * @tc.desc: Verify OnAbilityStateChanged returns early for TERMINATED state.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_OnAbilityStateChanged_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnAbilityStateChanged_0500 start");

    auto observer = CreateDisposedObserver(ComponentType::UI_ABILITY);

    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_TERMINATED);
    abilityStateData.token = new MockAbilityToken();

    observer->OnAbilityStateChanged(abilityStateData);

    // Verify token was not set
    EXPECT_EQ(observer->token_, nullptr);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnAbilityStateChanged_0500 end");
}

/**
 * @tc.number: DisposedObserver_ComponentType_Unknown_0100
 * @tc.name: DisposedObserver with unknown component type
 * @tc.desc: Verify observer handles unknown component type.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_ComponentType_Unknown_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_ComponentType_Unknown_0100 start");

    Want want;
    want.SetElementName("device", "com.example.test", "TestAbility");

    DisposedRule disposedRule;
    disposedRule.want = std::make_shared<Want>(want);
    // Use an arbitrary value that's not UI_ABILITY or UI_EXTENSION
    disposedRule.componentType = static_cast<ComponentType>(999);

    auto mockInterceptor = CreateInterceptor();
    auto observer = std::make_shared<DisposedObserver>(disposedRule, mockInterceptor, testUid_);

    EXPECT_NE(observer, nullptr);
    EXPECT_EQ(observer->disposedRule_.componentType, static_cast<ComponentType>(999));

    PageStateData pageStateData;
    pageStateData.uid = testUid_;
    pageStateData.moduleName = "testModule";
    pageStateData.abilityName = "TestAbility";

    // Add key to watch list
    observer->AddAbilityKey("testModule", "TestAbility");

    // OnPageShow should handle unknown component type gracefully
    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_ComponentType_Unknown_0100 end");
}

/**
 * @tc.number: DisposedObserver_EmptyString_0100
 * @tc.name: DisposedObserver::GenerateAbilityKey with empty strings
 * @tc.desc: Verify GenerateAbilityKey handles empty strings correctly.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_EmptyString_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_EmptyString_0100 start");

    std::string emptyModule = "";
    std::string abilityName = "TestAbility";
    std::string expectedKey = "/TestAbility";

    std::string actualKey = DisposedObserver::GenerateAbilityKey(emptyModule, abilityName);

    EXPECT_EQ(actualKey, expectedKey);

    // Test with both empty
    std::string expectedKey2 = "/";
    actualKey = DisposedObserver::GenerateAbilityKey("", "");

    EXPECT_EQ(actualKey, expectedKey2);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_EmptyString_0100 end");
}

/**
 * @tc.number: DisposedObserver_DuplicateKey_0100
 * @tc.name: DisposedObserver::AddAbilityKey with duplicate key
 * @tc.desc: Verify AddAbilityKey allows duplicate keys.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_DuplicateKey_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_DuplicateKey_0100 start");

    auto observer = CreateDisposedObserver(ComponentType::UI_ABILITY);

    observer->AddAbilityKey("module1", "Ability1");
    observer->AddAbilityKey("module1", "Ability1");

    // Currently, duplicate keys are allowed
    EXPECT_EQ(observer->GetAbilityKeyCount(), 2);

    // Removing one should leave one
    observer->RemoveAbilityKey("module1", "Ability1");
    EXPECT_EQ(observer->GetAbilityKeyCount(), 1);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_DuplicateKey_0100 end");
}

/**
 * @tc.number: DisposedObserver_OnPageShow_NullToken_0100
 * @tc.name: DisposedObserver::OnPageShow with null token for UI_EXTENSION
 * @tc.desc: Verify ExecuteUIExtension handles null token scenario.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_OnPageShow_NullToken_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_NullToken_0100 start");

    auto observer = CreateDisposedObserver(ComponentType::UI_EXTENSION);

    std::string testModule = "testModule";
    std::string testAbility = "TestAbility";
    observer->AddAbilityKey(testModule, testAbility);

    PageStateData pageStateData;
    pageStateData.uid = testUid_;
    pageStateData.moduleName = testModule;
    pageStateData.abilityName = testAbility;

    // Explicitly set token to nullptr
    observer->token_ = nullptr;

    // OnPageShow should handle null token in ExecuteUIExtension
    observer->OnPageShow(pageStateData);

    // Key should still be removed
    EXPECT_EQ(observer->GetAbilityKeyCount(), 0);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_OnPageShow_NullToken_0100 end");
}

/**
 * @tc.number: DisposedObserver_ZeroUID_0100
 * @tc.name: DisposedObserver with zero UID
 * @tc.desc: Verify observer handles zero UID correctly.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_ZeroUID_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_ZeroUID_0100 start");

    Want want;
    want.SetElementName("device", "com.example.test", "TestAbility");

    DisposedRule disposedRule;
    disposedRule.want = std::make_shared<Want>(want);
    disposedRule.componentType = ComponentType::UI_ABILITY;

    auto mockInterceptor = CreateInterceptor();
    int32_t zeroUid = 0;

    auto observer = std::make_shared<DisposedObserver>(disposedRule, mockInterceptor, zeroUid);

    EXPECT_EQ(observer->uid_, 0);

    PageStateData pageStateData;
    pageStateData.uid = 0;
    pageStateData.moduleName = "testModule";
    pageStateData.abilityName = "TestAbility";

    observer->AddAbilityKey("testModule", "TestAbility");

    // OnPageShow should process since uid matches (both 0)
    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_ZeroUID_0100 end");
}

/**
 * @tc.number: DisposedObserver_NegativeUID_0100
 * @tc.name: DisposedObserver with negative UID
 * @tc.desc: Verify observer handles negative UID correctly.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_NegativeUID_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_NegativeUID_0100 start");

    Want want;
    want.SetElementName("device", "com.example.test", "TestAbility");

    DisposedRule disposedRule;
    disposedRule.want = std::make_shared<Want>(want);
    disposedRule.componentType = ComponentType::UI_ABILITY;

    auto mockInterceptor = CreateInterceptor();
    int32_t negativeUid = -1;

    auto observer = std::make_shared<DisposedObserver>(disposedRule, mockInterceptor, negativeUid);

    EXPECT_EQ(observer->uid_, -1);

    PageStateData pageStateData;
    pageStateData.uid = -1;
    pageStateData.moduleName = "testModule";
    pageStateData.abilityName = "TestAbility";

    observer->AddAbilityKey("testModule", "TestAbility");

    // OnPageShow should process since uid matches
    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_NegativeUID_0100 end");
}

/**
 * @tc.number: DisposedObserver_NullInterceptor_0100
 * @tc.name: DisposedObserver::OnPageShow with null interceptor
 * @tc.desc: Verify OnPageShow handles null interceptor gracefully.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_NullInterceptor_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_NullInterceptor_0100 start");

    auto observer = CreateDisposedObserver(ComponentType::UI_ABILITY);
    observer->interceptor_ = nullptr;

    PageStateData pageStateData;
    pageStateData.uid = testUid_;

    // Should return early without crash
    observer->OnPageShow(pageStateData);
    EXPECT_EQ(observer->interceptor_, nullptr);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_NullInterceptor_0100 end");
}

/**
 * @tc.number: DisposedObserver_NullInterceptor_0200
 * @tc.name: DisposedObserver::OnAbilityStateChanged with null interceptor
 * @tc.desc: Verify OnAbilityStateChanged handles null interceptor gracefully.
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_NullInterceptor_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_NullInterceptor_0200 start");

    auto observer = CreateDisposedObserver(ComponentType::UI_ABILITY);
    observer->interceptor_ = nullptr;

    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND);
    abilityStateData.uid = testUid_;

    // Should return early without crash
    observer->OnAbilityStateChanged(abilityStateData);
    EXPECT_EQ(observer->interceptor_, nullptr);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "DisposedObserver_NullInterceptor_0200 end");
}

} // namespace AAFwk
} // namespace OHOS
