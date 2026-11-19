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

#include "ui_extension_context.h"
#define private public
#define protected public
#include "ui_extension.h"
#undef private
#undef protected

#include "ability_handler.h"
#include "ability_info.h"
#include "ability_local_record.h"
#include "event_runner.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"
#include "ohos_application.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace testing::ext;

class UIExtensionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void UIExtensionTest::SetUpTestCase(void)
{}

void UIExtensionTest::TearDownTestCase(void)
{}

void UIExtensionTest::SetUp()
{}

void UIExtensionTest::TearDown()
{}

/**
 * @tc.number: UIExtension_CreateModalUIExtension_0100
 * @tc.name: UIExtension::CreateModalUIExtension
 * @tc.desc: context is nullptr, return ERR_INVALID_VALUE.
 */
HWTEST_F(UIExtensionTest, UIExtension_CreateModalUIExtension_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIExtension_CreateModalUIExtension_0100 start");
    auto uiExtension = std::make_shared<UIExtension>();
    AAFwk::Want want;
    int ret = uiExtension->CreateModalUIExtension(want);
    EXPECT_EQ(ret, -1);
    TAG_LOGI(AAFwkTag::TEST, "UIExtension_CreateModalUIExtension_0100 end");
}

/**
 * @tc.number: UIExtension_CreateModalUIExtension_0200
 * @tc.name: UIExtension::CreateModalUIExtension
 * @tc.desc: handler_ is nullptr, return ERR_INVALID_VALUE.
 */
HWTEST_F(UIExtensionTest, UIExtension_CreateModalUIExtension_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIExtension_CreateModalUIExtension_0200 start");
    auto uiExtension = std::make_shared<UIExtension>();

    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);

    std::shared_ptr<AppExecFwk::AbilityHandler> handler = nullptr;
    uiExtension->Init(record, application, handler, token);

    AAFwk::Want want;
    int ret = uiExtension->CreateModalUIExtension(want);
    EXPECT_EQ(ret, -1);
    TAG_LOGI(AAFwkTag::TEST, "UIExtension_CreateModalUIExtension_0200 end");
}

/**
 * @tc.number: UIExtension_CreateModalUIExtension_0300
 * @tc.name: UIExtension::CreateModalUIExtension
 * @tc.desc: Init'd but EventRunner is null, PostTask fails, return ERR_INVALID_VALUE.
 */
HWTEST_F(UIExtensionTest, UIExtension_CreateModalUIExtension_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIExtension_CreateModalUIExtension_0300 start");
    auto uiExtension = std::make_shared<UIExtension>();

    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);

    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);
    uiExtension->Init(record, application, handler, token);

    AAFwk::Want want;
    int ret = uiExtension->CreateModalUIExtension(want);
    EXPECT_EQ(ret, -1);
    TAG_LOGI(AAFwkTag::TEST, "UIExtension_CreateModalUIExtension_0300 end");
}

/**
 * @tc.number: UIExtension_CreateModalUIExtension_0400
 * @tc.name: UIExtension::CreateModalUIExtension
 * @tc.desc: Multiple calls with different Want parameters, EventRunner null, PostTask fails.
 */
HWTEST_F(UIExtensionTest, UIExtension_CreateModalUIExtension_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIExtension_CreateModalUIExtension_0400 start");
    auto uiExtension = std::make_shared<UIExtension>();

    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);

    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);
    uiExtension->Init(record, application, handler, token);

    // First call with one Want
    AAFwk::Want want1;
    ElementName element1("device", "com.example.test", "Ability1");
    want1.SetElement(element1);
    int ret1 = uiExtension->CreateModalUIExtension(want1);
    EXPECT_EQ(ret1, -1);

    // Second call with different Want
    AAFwk::Want want2;
    ElementName element2("device", "com.example.test", "Ability2");
    want2.SetElement(element2);
    int ret2 = uiExtension->CreateModalUIExtension(want2);
    EXPECT_EQ(ret2, -1);

    TAG_LOGI(AAFwkTag::TEST, "UIExtension_CreateModalUIExtension_0400 end");
}

/**
 * @tc.number: UIExtension_CreateModalUIExtension_0500
 * @tc.name: UIExtension::CreateModalUIExtension
 * @tc.desc: CreateModalUIExtension with empty Want parameter.
 */
HWTEST_F(UIExtensionTest, UIExtension_CreateModalUIExtension_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIExtension_CreateModalUIExtension_0500 start");
    auto uiExtension = std::make_shared<UIExtension>();

    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);

    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);
    uiExtension->Init(record, application, handler, token);

    AAFwk::Want want;  // Empty Want
    int ret = uiExtension->CreateModalUIExtension(want);
    EXPECT_EQ(ret, -1);

    TAG_LOGI(AAFwkTag::TEST, "UIExtension_CreateModalUIExtension_0500 end");
}

/**
 * @tc.number: UIExtension_CreateModalUIExtension_0600
 * @tc.name: UIExtension::CreateModalUIExtension
 * @tc.desc: CreateModalUIExtension with Want containing parameters.
 */
HWTEST_F(UIExtensionTest, UIExtension_CreateModalUIExtension_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIExtension_CreateModalUIExtension_0600 start");
    auto uiExtension = std::make_shared<UIExtension>();

    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);

    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);
    uiExtension->Init(record, application, handler, token);

    AAFwk::Want want;
    ElementName element("device", "com.example.modal", "ModalExtension");
    want.SetElement(element);
    want.SetAction("action.modal.test");
    want.SetFlags(AAFwk::Want::FLAG_ABILITY_CONTINUATION);

    int ret = uiExtension->CreateModalUIExtension(want);
    EXPECT_EQ(ret, -1);

    TAG_LOGI(AAFwkTag::TEST, "UIExtension_CreateModalUIExtension_0600 end");
}

/**
 * @tc.number: UIExtension_CreateModalUIExtension_0700
 * @tc.name: UIExtension::CreateModalUIExtension
 * @tc.desc: Verify handler_ is used correctly for task posting.
 */
HWTEST_F(UIExtensionTest, UIExtension_CreateModalUIExtension_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIExtension_CreateModalUIExtension_0700 start");
    auto uiExtension = std::make_shared<UIExtension>();

    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);

    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);
    uiExtension->Init(record, application, handler, token);

    // Verify handler_ is set correctly
    EXPECT_TRUE(uiExtension->handler_ != nullptr);

    AAFwk::Want want;
    int ret = uiExtension->CreateModalUIExtension(want);
    EXPECT_EQ(ret, -1);

    TAG_LOGI(AAFwkTag::TEST, "UIExtension_CreateModalUIExtension_0700 end");
}

/**
 * @tc.number: UIExtension_CreateModalUIExtension_0800
 * @tc.name: UIExtension::CreateModalUIExtension
 * @tc.desc: CreateModalUIExtension with different device IDs in Want.
 */
HWTEST_F(UIExtensionTest, UIExtension_CreateModalUIExtension_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UIExtension_CreateModalUIExtension_0800 start");
    auto uiExtension = std::make_shared<UIExtension>();

    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);

    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);
    uiExtension->Init(record, application, handler, token);

    AAFwk::Want want;
    ElementName element("tablet", "com.example.test", "ModalExtension");
    want.SetElement(element);
    want.SetDeviceId("tablet_device_001");

    int ret = uiExtension->CreateModalUIExtension(want);
    EXPECT_EQ(ret, -1);

    TAG_LOGI(AAFwkTag::TEST, "UIExtension_CreateModalUIExtension_0800 end");
}

} // namespace AbilityRuntime
} // namespace OHOS
