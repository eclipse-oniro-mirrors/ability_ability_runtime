/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <singleton.h>
#include <cstdint>
#include <cstring>

#include "ability_manager_service.h"
#include "ability_record.h"
#include "wm_common.h"
#define private public
#define protected public
#include "window_focus_changed_listener.h"
#include "permission_verification.h"
#include "bundle_mgr_helper.h"
#include "ui_extension_record_factory.h"
#undef private
#undef protected

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

class WindowFocusChangedListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void WindowFocusChangedListenerTest::SetUpTestCase(void)
{}

void WindowFocusChangedListenerTest::TearDownTestCase(void)
{}

void WindowFocusChangedListenerTest::SetUp(void)
{}

void WindowFocusChangedListenerTest::TearDown(void)
{}

/**
 * @tc.number: DumpFfrtHelperTest_001
 * @tc.name: DumpFfrt
 * @tc.desc: Test whether GetBundleName is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(WindowFocusChangedListenerTest, OnFocused_001, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityManagerService> owner;
    std::shared_ptr<AAFwk::TaskHandlerWrap> handler;
    auto info = std::make_shared<WindowFocusChangedListener>(owner, handler);
    sptr<Rosen::FocusChangeInfo> focusChangeInfo;
    info->OnFocused(focusChangeInfo);
    EXPECT_EQ(focusChangeInfo, nullptr);
}

/**
 * @tc.number: DumpFfrtHelperTest_001
 * @tc.name: DumpFfrt
 * @tc.desc: Test whether GetBundleName is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(WindowFocusChangedListenerTest, OnFocused_002, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityManagerService> owner;
    std::shared_ptr<AAFwk::TaskHandlerWrap> handler;
    auto info = std::make_shared<WindowFocusChangedListener>(owner, handler);
    sptr<Rosen::FocusChangeInfo> focusChangeInfo;
    pid_t pid = 1;
    focusChangeInfo = new Rosen::FocusChangeInfo();
    focusChangeInfo->pid_ = pid;
    info->OnFocused(focusChangeInfo);
    EXPECT_NE(focusChangeInfo, nullptr);
}

/**
 * @tc.number: DumpFfrtHelperTest_001
 * @tc.name: DumpFfrt
 * @tc.desc: Test whether GetBundleName is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(WindowFocusChangedListenerTest, OnFocused_003, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityManagerService> owner;
    std::string queueName = "queueName";
    std::shared_ptr<AAFwk::TaskHandlerWrap> handler = AAFwk::TaskHandlerWrap::CreateQueueHandler(queueName);
    EXPECT_NE(handler, nullptr);
    auto info = std::make_shared<WindowFocusChangedListener>(owner, handler);
    sptr<Rosen::FocusChangeInfo> focusChangeInfo;
    pid_t pid = 1;
    focusChangeInfo = new Rosen::FocusChangeInfo();
    focusChangeInfo->pid_ = pid;
    info->OnFocused(focusChangeInfo);
    EXPECT_NE(focusChangeInfo, nullptr);
}

/**
 * @tc.number: DumpFfrtHelperTest_001
 * @tc.name: DumpFfrt
 * @tc.desc: Test whether GetBundleName is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(WindowFocusChangedListenerTest, OnUnfocused_001, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityManagerService> owner;
    std::shared_ptr<AAFwk::TaskHandlerWrap> handler;
    auto info = std::make_shared<WindowFocusChangedListener>(owner, handler);
    sptr<Rosen::FocusChangeInfo> focusChangeInfo;
    info->OnUnfocused(focusChangeInfo);
    EXPECT_EQ(focusChangeInfo, nullptr);
}

/**
 * @tc.number: DumpFfrtHelperTest_001
 * @tc.name: DumpFfrt
 * @tc.desc: Test whether GetBundleName is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(WindowFocusChangedListenerTest, OnUnfocused_002, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityManagerService> owner;
    std::shared_ptr<AAFwk::TaskHandlerWrap> handler;
    auto info = std::make_shared<WindowFocusChangedListener>(owner, handler);
    sptr<Rosen::FocusChangeInfo> focusChangeInfo;
    pid_t pid = 1;
    focusChangeInfo = new Rosen::FocusChangeInfo();
    focusChangeInfo->pid_ = pid;
    info->OnUnfocused(focusChangeInfo);
    EXPECT_NE(focusChangeInfo, nullptr);
}

/**
 * @tc.number: DumpFfrtHelperTest_001
 * @tc.name: DumpFfrt
 * @tc.desc: Test whether GetBundleName is called normally.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HL
 */
HWTEST_F(WindowFocusChangedListenerTest, OnUnfocused_003, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityManagerService> owner;
    std::string queueName = "queueName";
    std::shared_ptr<AAFwk::TaskHandlerWrap> handler = AAFwk::TaskHandlerWrap::CreateQueueHandler(queueName);
    EXPECT_NE(handler, nullptr);
    auto info = std::make_shared<WindowFocusChangedListener>(owner, handler);
    sptr<Rosen::FocusChangeInfo> focusChangeInfo;
    pid_t pid = 1;
    focusChangeInfo = new Rosen::FocusChangeInfo();
    focusChangeInfo->pid_ = pid;
    info->OnUnfocused(focusChangeInfo);
    EXPECT_NE(focusChangeInfo, nullptr);
}

/**
 * @tc.number: IsInAllowList_0100
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with empty extensionInfos
 */
HWTEST_F(WindowFocusChangedListenerTest, IsInAllowList_0100, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    std::string extensionName = "TestExtension";
    std::string callerAppIdentifier = "com.example.caller";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: IsInAllowList_0200
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with matching extension and caller
 */
HWTEST_F(WindowFocusChangedListenerTest, IsInAllowList_0200, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.name = "TestExtension";
    extensionInfo.appIdentifierAllowList.push_back("com.example.caller");
    extensionInfos.push_back(extensionInfo);

    std::string extensionName = "TestExtension";
    std::string callerAppIdentifier = "com.example.caller";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number: IsInAllowList_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with allow_all wildcard
 */
HWTEST_F(WindowFocusChangedListenerTest, IsInAllowList_0300, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.name = "TestExtension";
    extensionInfo.appIdentifierAllowList.push_back("allow_all");
    extensionInfos.push_back(extensionInfo);

    std::string extensionName = "TestExtension";
    std::string callerAppIdentifier = "com.example.any_caller";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number: IsInAllowList_0400
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with non-matching caller
 */
HWTEST_F(WindowFocusChangedListenerTest, IsInAllowList_0400, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.name = "TestExtension";
    extensionInfo.appIdentifierAllowList.push_back("com.example.different_caller");
    extensionInfos.push_back(extensionInfo);

    std::string extensionName = "TestExtension";
    std::string callerAppIdentifier = "com.example.caller";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: IsInAllowList_0500
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with wrong extension type
 */
HWTEST_F(WindowFocusChangedListenerTest, IsInAllowList_0500, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::SERVICE;  // Wrong type
    extensionInfo.name = "TestExtension";
    extensionInfo.appIdentifierAllowList.push_back("com.example.caller");
    extensionInfos.push_back(extensionInfo);

    std::string extensionName = "TestExtension";
    std::string callerAppIdentifier = "com.example.caller";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: IsInAllowList_0600
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with multiple extensions
 */
HWTEST_F(WindowFocusChangedListenerTest, IsInAllowList_0600, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo1;
    extensionInfo1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo1.name = "ServiceExtension";
    extensionInfos.push_back(extensionInfo1);

    AppExecFwk::ExtensionAbilityInfo extensionInfo2;
    extensionInfo2.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo2.name = "TargetExtension";
    extensionInfo2.appIdentifierAllowList.push_back("com.example.caller");
    extensionInfos.push_back(extensionInfo2);

    std::string extensionName = "TargetExtension";
    std::string callerAppIdentifier = "com.example.caller";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number: IsInAllowList_0600
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with multiple extensions
 */
HWTEST_F(WindowFocusChangedListenerTest, IsInAllowList_0700, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo1;
    extensionInfo1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo1.name = "ServiceExtension";
    extensionInfos.push_back(extensionInfo1);

    AppExecFwk::ExtensionAbilityInfo extensionInfo2;
    extensionInfo2.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo2.name = "TargetExtension";
    extensionInfo2.appIdentifierAllowList.push_back("com.example.caller");
    extensionInfos.push_back(extensionInfo2);

    AppExecFwk::ExtensionAbilityInfo extensionInfo3;
    extensionInfo3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo3.name = "TargetExtension1";
    extensionInfo3.appIdentifierAllowList.push_back("com.example.caller");
    extensionInfos.push_back(extensionInfo3);

    std::string extensionName = "TargetExtension";
    std::string callerAppIdentifier = "com.example.caller";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, true);
}

/**
 * @tc.number: IsInAllowList_0600
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with multiple extensions
 */
HWTEST_F(WindowFocusChangedListenerTest, IsInAllowList_0800, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo1;
    extensionInfo1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo1.name = "ServiceExtension";
    extensionInfos.push_back(extensionInfo1);

    AppExecFwk::ExtensionAbilityInfo extensionInfo2;
    extensionInfo2.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo2.name = "TargetExtension";
    extensionInfo2.appIdentifierAllowList.push_back("com.example.caller");
    extensionInfos.push_back(extensionInfo2);

    AppExecFwk::ExtensionAbilityInfo extensionInfo3;
    extensionInfo3.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo3.name = "TargetExtension1";
    extensionInfo3.appIdentifierAllowList.push_back("com.example.caller");
    extensionInfos.push_back(extensionInfo3);

    std::string extensionName = "TargetExtension";
    std::string callerAppIdentifier = "com.example.caller";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: IsInAllowList_0600
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with multiple extensions
 */
HWTEST_F(WindowFocusChangedListenerTest, IsInAllowList_0900, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo1;
    extensionInfo1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo1.name = "ServiceExtension";
    extensionInfos.push_back(extensionInfo1);

    AppExecFwk::ExtensionAbilityInfo extensionInfo2;
    extensionInfo2.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo2.name = "TargetExtension";
    extensionInfo2.appIdentifierAllowList.push_back("com.example.caller11");
    extensionInfos.push_back(extensionInfo2);

    AppExecFwk::ExtensionAbilityInfo extensionInfo3;
    extensionInfo3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo3.name = "TargetExtension1";
    extensionInfo3.appIdentifierAllowList.push_back("com.example.caller22");
    extensionInfos.push_back(extensionInfo3);

    std::string extensionName = "TargetExtension";
    std::string callerAppIdentifier = "com.example.caller";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: IsInAllowList_0600
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with multiple extensions
 */
HWTEST_F(WindowFocusChangedListenerTest, IsInAllowList_1000, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo1;
    extensionInfo1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo1.name = "ServiceExtension";
    extensionInfos.push_back(extensionInfo1);

    AppExecFwk::ExtensionAbilityInfo extensionInfo2;
    extensionInfo2.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo2.name = "TargetExtension";
    extensionInfo2.appIdentifierAllowList.push_back("com.example.caller11");
    extensionInfos.push_back(extensionInfo2);

    AppExecFwk::ExtensionAbilityInfo extensionInfo3;
    extensionInfo3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo3.name = "TargetExtension1";
    extensionInfo3.appIdentifierAllowList.push_back("com.example.caller22");
    extensionInfos.push_back(extensionInfo3);

    std::string extensionName = "TargetExtension";
    std::string callerAppIdentifier = "com.example.caller123";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: IsInAllowList_0600
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with multiple extensions
 */
HWTEST_F(WindowFocusChangedListenerTest, IsInAllowList_1100, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo1;
    extensionInfo1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo1.name = "ServiceExtension";
    extensionInfos.push_back(extensionInfo1);

    AppExecFwk::ExtensionAbilityInfo extensionInfo2;
    extensionInfo2.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo2.name = "TargetExtension";
    extensionInfo2.appIdentifierAllowList.push_back("com.example.caller11");
    extensionInfos.push_back(extensionInfo2);

    AppExecFwk::ExtensionAbilityInfo extensionInfo3;
    extensionInfo3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo3.name = "TargetExtension1";
    extensionInfo3.appIdentifierAllowList.push_back("com.example.caller22");
    extensionInfos.push_back(extensionInfo3);

    std::string extensionName = "TargetExtension88";
    std::string callerAppIdentifier = "com.example.caller123";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: IsInAllowList_0600
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with multiple extensions
 */
HWTEST_F(WindowFocusChangedListenerTest, IsInAllowList_1200, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo1;
    extensionInfo1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo1.name = "ServiceExtension";
    extensionInfos.push_back(extensionInfo1);

    AppExecFwk::ExtensionAbilityInfo extensionInfo2;
    extensionInfo2.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo2.name = "TargetExtension";
    extensionInfo2.appIdentifierAllowList.push_back("com.example.caller11");
    extensionInfos.push_back(extensionInfo2);

    AppExecFwk::ExtensionAbilityInfo extensionInfo3;
    extensionInfo3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo3.name = "TargetExtension1";
    extensionInfo3.appIdentifierAllowList.push_back("com.example.caller22");
    extensionInfos.push_back(extensionInfo3);

    std::string extensionName = "ServiceExtension";
    std::string callerAppIdentifier = "com.example.caller123";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: IsInAllowList_0600
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test IsInAllowList with multiple extensions
 */
HWTEST_F(WindowFocusChangedListenerTest, IsInAllowList_1300, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;

    AppExecFwk::ExtensionAbilityInfo extensionInfo1;
    extensionInfo1.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo1.name = "ServiceExtension";
    extensionInfos.push_back(extensionInfo1);

    AppExecFwk::ExtensionAbilityInfo extensionInfo2;
    extensionInfo2.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo2.name = "TargetExtension";
    extensionInfo2.appIdentifierAllowList.push_back("com.example.caller11");
    extensionInfos.push_back(extensionInfo2);

    AppExecFwk::ExtensionAbilityInfo extensionInfo3;
    extensionInfo3.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo3.name = "TargetExtension1";
    extensionInfo3.appIdentifierAllowList.push_back("com.example.caller22");
    extensionInfos.push_back(extensionInfo3);

    std::string extensionName = "ServiceExtension123";
    std::string callerAppIdentifier = "";

    bool result = factory.IsInAllowList(extensionInfos, extensionName, callerAppIdentifier);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: CheckAllowCrossUserEmbeddedUI_0100
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckAllowCrossUserEmbeddedUI without permission
 */
HWTEST_F(WindowFocusChangedListenerTest, CheckAllowCrossUserEmbeddedUI_0100, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";

    bool result = factory.CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: CheckAllowCrossUserEmbeddedUI_0200
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckAllowCrossUserEmbeddedUI with permission and caller app identifier
 */
HWTEST_F(WindowFocusChangedListenerTest, CheckAllowCrossUserEmbeddedUI_0200, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string targetBundleName = "com.example.target";
    std::string extensionName = "TestExtension";
    std::string callerAppIdentifier = "com.example.caller";

    // 设置 Want 中的 caller app identifier
    abilityRequest.want.SetParam(AAFwk::Want::PARAM_RESV_CALLER_APP_IDENTIFIER, callerAppIdentifier);
    abilityRequest.abilityInfo.bundleName = targetBundleName;
    abilityRequest.abilityInfo.name = extensionName;
    abilityRequest.userId = 100;

    // 设置 mock BundleMgr
    auto bundleMgr = AppExecFwk::BundleMgrHelper::GetBundleMgrHelper();

    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.name = extensionName;
    extensionInfo.appIdentifierAllowList.push_back(callerAppIdentifier);
    bundleInfo.extensionInfos.push_back(extensionInfo);

    bundleMgr->SetMockBundleInfo(bundleInfo);
    bundleMgr->SetGetBundleInfoResult(true);

    bool result = factory.CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: CheckAllowCrossUserEmbeddedUI_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckAllowCrossUserEmbeddedUI with permission and signature info
 */
HWTEST_F(WindowFocusChangedListenerTest, CheckAllowCrossUserEmbeddedUI_0300, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string targetBundleName = "com.example.target";
    std::string extensionName = "TestExtension";
    std::string appIdentifier = "com.example.appidentifier";

    // 不设置 Want 中的 caller app identifier，将通过 signatureInfo 获取
    abilityRequest.abilityInfo.bundleName = targetBundleName;
    abilityRequest.abilityInfo.name = extensionName;
    abilityRequest.userId = 100;

    // 设置 mock BundleMgr
    auto bundleMgr = AppExecFwk::BundleMgrHelper::GetBundleMgrHelper();

    // 设置 SignatureInfo
    AppExecFwk::SignatureInfo signatureInfo;
    signatureInfo.appIdentifier = appIdentifier;
    bundleMgr->SetMockSignatureInfo(signatureInfo);
    bundleMgr->SetGetSignatureInfoResult(ERR_OK);

    // 设置 BundleInfo
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.name = extensionName;
    extensionInfo.appIdentifierAllowList.push_back(appIdentifier);
    bundleInfo.extensionInfos.push_back(extensionInfo);

    bundleMgr->SetMockBundleInfo(bundleInfo);
    bundleMgr->SetGetBundleInfoResult(true);

    bool result = factory.CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: CheckAllowCrossUserEmbeddedUI_0400
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckAllowCrossUserEmbeddedUI when not in allow list
 */
HWTEST_F(WindowFocusChangedListenerTest, CheckAllowCrossUserEmbeddedUI_0400, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string targetBundleName = "com.example.target";
    std::string extensionName = "TestExtension";
    std::string callerAppIdentifier = "com.example.caller";

    abilityRequest.want.SetParam(AAFwk::Want::PARAM_RESV_CALLER_APP_IDENTIFIER, callerAppIdentifier);
    abilityRequest.abilityInfo.bundleName = targetBundleName;
    abilityRequest.abilityInfo.name = extensionName;
    abilityRequest.userId = 100;

    // 设置 mock BundleMgr
    auto bundleMgr = AppExecFwk::BundleMgrHelper::GetBundleMgrHelper();

    // 不包含 caller 的 allow list
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.name = extensionName;
    extensionInfo.appIdentifierAllowList.push_back("com.example.different_caller");
    bundleInfo.extensionInfos.push_back(extensionInfo);

    bundleMgr->SetMockBundleInfo(bundleInfo);
    bundleMgr->SetGetBundleInfoResult(true);

    bool result = factory.CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: CheckAllowCrossUserEmbeddedUI_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckAllowCrossUserEmbeddedUI with permission and signature info
 */
HWTEST_F(WindowFocusChangedListenerTest, CheckAllowCrossUserEmbeddedUI_0500, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string targetBundleName = "com.example.target";
    std::string extensionName = "TestExtension";
    std::string appIdentifier = "com.example.appidentifier";

    // 不设置 Want 中的 caller app identifier，将通过 signatureInfo 获取
    abilityRequest.abilityInfo.bundleName = targetBundleName;
    abilityRequest.abilityInfo.name = extensionName;
    abilityRequest.userId = 100;

    // 设置 mock BundleMgr
    auto bundleMgr = AppExecFwk::BundleMgrHelper::GetBundleMgrHelper();

    // 设置 SignatureInfo
    AppExecFwk::SignatureInfo signatureInfo;
    signatureInfo.appIdentifier = appIdentifier;
    bundleMgr->SetMockSignatureInfo(signatureInfo);
    bundleMgr->SetGetSignatureInfoResult(ERR_OK);

    // 设置 BundleInfo
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.name = extensionName;
    extensionInfo.appIdentifierAllowList.push_back(appIdentifier);
    bundleInfo.extensionInfos.push_back(extensionInfo);

    bundleMgr->SetMockBundleInfo(bundleInfo);
    bundleMgr->SetGetBundleInfoResult(true);

    bool result = factory.CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: CheckAllowCrossUserEmbeddedUI_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckAllowCrossUserEmbeddedUI with permission and signature info
 */
HWTEST_F(WindowFocusChangedListenerTest, CheckAllowCrossUserEmbeddedUI_0600, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string targetBundleName = "com.example.target";
    std::string extensionName = "TestExtension";
    std::string appIdentifier = "com.example.appidentifier";

    // 不设置 Want 中的 caller app identifier，将通过 signatureInfo 获取
    abilityRequest.abilityInfo.bundleName = targetBundleName;
    abilityRequest.abilityInfo.name = extensionName;
    abilityRequest.userId = 100;

    // 设置 mock BundleMgr
    auto bundleMgr = AppExecFwk::BundleMgrHelper::GetBundleMgrHelper();

    // 设置 SignatureInfo
    AppExecFwk::SignatureInfo signatureInfo;
    signatureInfo.appIdentifier = appIdentifier;
    bundleMgr->SetMockSignatureInfo(signatureInfo);
    bundleMgr->SetGetSignatureInfoResult(ERR_OK);

    // 设置 BundleInfo
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::SERVICE;
    extensionInfo.name = extensionName;
    extensionInfo.appIdentifierAllowList.push_back(appIdentifier);
    bundleInfo.extensionInfos.push_back(extensionInfo);

    bundleMgr->SetMockBundleInfo(bundleInfo);
    bundleMgr->SetGetBundleInfoResult(true);

    bool result = factory.CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: CheckAllowCrossUserEmbeddedUI_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckAllowCrossUserEmbeddedUI with permission and signature info
 */
HWTEST_F(WindowFocusChangedListenerTest, CheckAllowCrossUserEmbeddedUI_0700, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string targetBundleName = "com.example.target";
    std::string extensionName = "TestExtension";
    std::string appIdentifier = "com.example.appidentifier";

    // 不设置 Want 中的 caller app identifier，将通过 signatureInfo 获取
    abilityRequest.abilityInfo.bundleName = targetBundleName;
    abilityRequest.abilityInfo.name = extensionName;
    abilityRequest.userId = 100;

    // 设置 mock BundleMgr
    auto bundleMgr = AppExecFwk::BundleMgrHelper::GetBundleMgrHelper();

    // 设置 SignatureInfo
    AppExecFwk::SignatureInfo signatureInfo;
    signatureInfo.appIdentifier = appIdentifier;
    bundleMgr->SetMockSignatureInfo(signatureInfo);
    bundleMgr->SetGetSignatureInfoResult(ERR_OK);

    // 设置 BundleInfo
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.name = extensionName;
    bundleInfo.extensionInfos.push_back(extensionInfo);

    bundleMgr->SetMockBundleInfo(bundleInfo);
    bundleMgr->SetGetBundleInfoResult(true);

    bool result = factory.CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: CheckAllowCrossUserEmbeddedUI_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckAllowCrossUserEmbeddedUI with permission and signature info
 */
HWTEST_F(WindowFocusChangedListenerTest, CheckAllowCrossUserEmbeddedUI_0800, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string targetBundleName = "com.example.target";
    std::string extensionName = "TestExtension";
    std::string appIdentifier = "com.example.appidentifier";

    // 不设置 Want 中的 caller app identifier，将通过 signatureInfo 获取
    abilityRequest.abilityInfo.bundleName = targetBundleName;
    abilityRequest.abilityInfo.name = extensionName;
    abilityRequest.userId = 100;

    // 设置 mock BundleMgr
    auto bundleMgr = AppExecFwk::BundleMgrHelper::GetBundleMgrHelper();

    // 设置 SignatureInfo
    AppExecFwk::SignatureInfo signatureInfo;
    signatureInfo.appIdentifier = appIdentifier;
    bundleMgr->SetMockSignatureInfo(signatureInfo);
    bundleMgr->SetGetSignatureInfoResult(ERR_OK);

    // 设置 BundleInfo
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.name = extensionName;
    extensionInfo.appIdentifierAllowList.push_back(appIdentifier);
    bundleInfo.extensionInfos.push_back(extensionInfo);

    bundleMgr->SetMockBundleInfo(bundleInfo);
    bundleMgr->SetGetBundleInfoResult(false);

    bool result = factory.CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: CheckAllowCrossUserEmbeddedUI_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckAllowCrossUserEmbeddedUI with permission and signature info
 */
HWTEST_F(WindowFocusChangedListenerTest, CheckAllowCrossUserEmbeddedUI_0900, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string targetBundleName = "com.example.target";
    std::string extensionName = "TestExtension";
    std::string appIdentifier = "com.example.appidentifier";

    // 不设置 Want 中的 caller app identifier，将通过 signatureInfo 获取
    abilityRequest.abilityInfo.bundleName = targetBundleName;
    abilityRequest.abilityInfo.name = extensionName;
    abilityRequest.userId = 100;

    // 设置 mock BundleMgr
    auto bundleMgr = AppExecFwk::BundleMgrHelper::GetBundleMgrHelper();

    // 设置 SignatureInfo
    AppExecFwk::SignatureInfo signatureInfo;
    signatureInfo.appIdentifier = appIdentifier;
    bundleMgr->SetMockSignatureInfo(signatureInfo);
    bundleMgr->SetGetSignatureInfoResult(ERR_OK);

    // 设置 BundleInfo
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.appIdentifierAllowList.push_back(appIdentifier);
    bundleInfo.extensionInfos.push_back(extensionInfo);

    bundleMgr->SetMockBundleInfo(bundleInfo);
    bundleMgr->SetGetBundleInfoResult(true);

    bool result = factory.CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: CheckAllowCrossUserEmbeddedUI_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckAllowCrossUserEmbeddedUI with permission and signature info
 */
HWTEST_F(WindowFocusChangedListenerTest, CheckAllowCrossUserEmbeddedUI_1000, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string targetBundleName = "com.example.target";
    std::string extensionName = "TestExtension";
    std::string appIdentifier = "com.example.appidentifier";

    // 不设置 Want 中的 caller app identifier，将通过 signatureInfo 获取
    abilityRequest.abilityInfo.bundleName = targetBundleName;
    abilityRequest.abilityInfo.name = extensionName;
    abilityRequest.userId = 100;

    // 设置 mock BundleMgr
    auto bundleMgr = AppExecFwk::BundleMgrHelper::GetBundleMgrHelper();

    // 设置 SignatureInfo
    AppExecFwk::SignatureInfo signatureInfo;
    signatureInfo.appIdentifier = appIdentifier;
    bundleMgr->SetMockSignatureInfo(signatureInfo);
    bundleMgr->SetGetSignatureInfoResult(ERR_OK);

    // 设置 BundleInfo
    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.name = extensionName;
    extensionInfo.appIdentifierAllowList.push_back(appIdentifier);
    bundleInfo.extensionInfos.push_back(extensionInfo);

    bundleMgr->SetMockBundleInfo(bundleInfo);
    bundleMgr->SetGetBundleInfoResult(true);

    bool result = factory.CheckAllowCrossUserEmbeddedUI(abilityRequest, hostBundleName);
    EXPECT_EQ(result, false);
}

/**
 * @tc.number: CheckHostBundleName_0100
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckHostBundleName with matching applicationName
 */
HWTEST_F(WindowFocusChangedListenerTest, CheckHostBundleName_0100, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string bundleName = "com.example.app";

    abilityRequest.abilityInfo.applicationName = bundleName;

    int32_t result = factory.CheckHostBundleName(abilityRequest, bundleName);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: CheckHostBundleName_0200
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckHostBundleName with non-matching bundleName
 */
HWTEST_F(WindowFocusChangedListenerTest, CheckHostBundleName_0200, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string appBundleName = "com.example.app";

    abilityRequest.abilityInfo.applicationName = appBundleName;
    abilityRequest.isTargetPlugin = false;

    int32_t result = factory.CheckHostBundleName(abilityRequest, hostBundleName);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.number: CheckHostBundleName_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckHostBundleName with cross-user permission allowed
 */
HWTEST_F(WindowFocusChangedListenerTest, CheckHostBundleName_0300, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string targetBundleName = "com.example.target";
    std::string extensionName = "TestExtension";
    std::string callerAppIdentifier = "com.example.caller";

    abilityRequest.abilityInfo.applicationName = targetBundleName;
    abilityRequest.abilityInfo.bundleName = targetBundleName;
    abilityRequest.abilityInfo.name = extensionName;
    abilityRequest.userId = 100;

    abilityRequest.want.SetParam(AAFwk::Want::PARAM_RESV_CALLER_APP_IDENTIFIER, callerAppIdentifier);

    // 设置 mock BundleMgr
    auto bundleMgr = AppExecFwk::BundleMgrHelper::GetBundleMgrHelper();

    AppExecFwk::BundleInfo bundleInfo;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = AppExecFwk::ExtensionAbilityType::EMBEDDED_UI;
    extensionInfo.name = extensionName;
    extensionInfo.appIdentifierAllowList.push_back(callerAppIdentifier);
    bundleInfo.extensionInfos.push_back(extensionInfo);

    bundleMgr->SetMockBundleInfo(bundleInfo);
    bundleMgr->SetGetBundleInfoResult(true);

    int32_t result = factory.CheckHostBundleName(abilityRequest, hostBundleName);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.number: CheckHostBundleName_0400
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test CheckHostBundleName with plugin mode
 */
HWTEST_F(WindowFocusChangedListenerTest, CheckHostBundleName_0400, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";

    abilityRequest.abilityInfo.applicationName = "com.example.different";
    abilityRequest.isTargetPlugin = true;
    abilityRequest.hostBundleName = hostBundleName;

    int32_t result = factory.CheckHostBundleName(abilityRequest, hostBundleName);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: EmbeddedUIPreCheck_0100
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test EmbeddedUIPreCheck with SceneBoard
 */
HWTEST_F(WindowFocusChangedListenerTest, EmbeddedUIPreCheck_0100, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string sceneBoardName = "com.ohos.sceneboard";

    int32_t result = factory.EmbeddedUIPreCheck(abilityRequest, sceneBoardName);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: EmbeddedUIPreCheck_0200
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test EmbeddedUIPreCheck with invalid host bundle name
 */
HWTEST_F(WindowFocusChangedListenerTest, EmbeddedUIPreCheck_0200, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.host";
    std::string appBundleName = "com.example.app";

    abilityRequest.abilityInfo.applicationName = appBundleName;
    abilityRequest.isTargetPlugin = false;

    int32_t result = factory.EmbeddedUIPreCheck(abilityRequest, hostBundleName);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.number: EmbeddedUIPreCheck_0300
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test EmbeddedUIPreCheck with valid host and null sessionInfo
 */
HWTEST_F(WindowFocusChangedListenerTest, EmbeddedUIPreCheck_0300, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string bundleName = "com.example.app";

    abilityRequest.abilityInfo.applicationName = bundleName;
    abilityRequest.sessionInfo = nullptr;

    int32_t result = factory.EmbeddedUIPreCheck(abilityRequest, bundleName);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: EmbeddedUIPreCheck_0400
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test EmbeddedUIPreCheck with non-PAGE caller
 */
HWTEST_F(WindowFocusChangedListenerTest, EmbeddedUIPreCheck_0400, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string bundleName = "com.example.app";

    abilityRequest.abilityInfo.applicationName = bundleName;
    abilityRequest.sessionInfo = new (std::nothrow) AAFwk::SessionInfo();

    // 创建一个 SERVICE 类型的 caller ability
    AAFwk::AbilityRequest callerRequest;
    callerRequest.abilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    callerRequest.abilityInfo.applicationName = bundleName;
    std::shared_ptr<AbilityRecord> callerAbilityRecord = AbilityRecord::CreateAbilityRecord(callerRequest);
    auto callerToken = callerAbilityRecord->GetToken();
    abilityRequest.sessionInfo->callerToken = callerToken;

    int32_t result = factory.EmbeddedUIPreCheck(abilityRequest, bundleName);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.number: EmbeddedUIPreCheck_0500
 * @tc.name: UIExtensionRecordFactoryTest
 * @tc.desc: Test EmbeddedUIPreCheck with valid PAGE caller
 */
HWTEST_F(WindowFocusChangedListenerTest, EmbeddedUIPreCheck_0500, TestSize.Level1)
{
    AbilityRuntime::UIExtensionRecordFactory factory;
    AAFwk::AbilityRequest abilityRequest;
    std::string bundleName = "com.example.app";

    abilityRequest.abilityInfo.applicationName = bundleName;
    abilityRequest.sessionInfo = new (std::nothrow) AAFwk::SessionInfo();

    // 创建一个 PAGE 类型的 caller ability
    AAFwk::AbilityRequest callerRequest;
    callerRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    callerRequest.abilityInfo.applicationName = bundleName;
    std::shared_ptr<AbilityRecord> callerAbilityRecord = AbilityRecord::CreateAbilityRecord(callerRequest);
    auto callerToken = callerAbilityRecord->GetToken();
    abilityRequest.sessionInfo->callerToken = callerToken;

    int32_t result = factory.EmbeddedUIPreCheck(abilityRequest, bundleName);
    EXPECT_EQ(result, ERR_OK);
}
}
}
