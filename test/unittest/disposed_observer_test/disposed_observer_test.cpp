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

} // namespace AAFwk
} // namespace OHOS
