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
#include <memory>

#include "parcel.h"

#include "ui_extension/ui_extension_session_info.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class UIExtensionSessionInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UIExtensionSessionInfoTest::SetUpTestCase(void)
{}

void UIExtensionSessionInfoTest::TearDownTestCase(void)
{}

void UIExtensionSessionInfoTest::SetUp()
{}

void UIExtensionSessionInfoTest::TearDown()
{}

/**
 * @tc.name: Marshalling_0100
 * @tc.desc: Marshalling
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionSessionInfoTest, Marshalling_0100, TestSize.Level1)
{
    Parcel parcel;
    UIExtensionSessionInfo* parcelable1 = new UIExtensionSessionInfo();
    parcelable1->persistentId = 1;
    parcelable1->hostWindowId = 1;
    parcelable1->uiExtensionUsage = AAFwk::UIExtensionUsage::MODAL;
    std::string deviceId;
    std::string bundleName = "ohos.test.bundle";
    std::string abilityName = "TestAbility";
    std::string moduleName = "entry";
    AppExecFwk::ElementName elementName;
    elementName.SetDeviceID(deviceId);
    elementName.SetBundleName(bundleName);
    elementName.SetAbilityName(abilityName);
    elementName.SetModuleName(moduleName);
    parcelable1->elementName = elementName;
    parcelable1->extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYSDIALOG_COMMON;
    std::string hostDeviceId;
    std::string hostBundleName = "ohos.test.bundle";
    std::string hostAbilityName = "TestAbility";
    std::string hostModuleName = "entry";
    AppExecFwk::ElementName hostElementName;
    hostElementName.SetDeviceID(deviceId);
    hostElementName.SetBundleName(hostBundleName);
    hostElementName.SetAbilityName(hostAbilityName);
    hostElementName.SetModuleName(hostModuleName);
    parcelable1->hostElementName = hostElementName;
    EXPECT_EQ(true, parcelable1->Marshalling(parcel));
}

/**
 * @tc.name: Unmarshalling_0100
 * @tc.desc: Unmarshalling with null hostElement
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionSessionInfoTest, Unmarshalling_0100, TestSize.Level1)
{
    Parcel parcel;
    parcel.WriteInt32(1);
    parcel.WriteUint32(1);
    parcel.WriteUint32(1);
    parcel.WriteParcelable(new AppExecFwk::ElementName());
    parcel.WriteInt32(1);
    parcel.WriteParcelable(nullptr);
    UIExtensionSessionInfo *reuslt = UIExtensionSessionInfo::Unmarshalling(parcel);
    EXPECT_EQ(reuslt, nullptr);
}

/**
 * @tc.name: Unmarshalling_0200
 * @tc.desc: Full round-trip Marshalling and Unmarshalling, verify all fields
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionSessionInfoTest, Unmarshalling_0200, TestSize.Level1)
{
    UIExtensionSessionInfo expected;
    expected.persistentId = 42;
    expected.hostWindowId = 100;
    expected.uiExtensionUsage = AAFwk::UIExtensionUsage::EMBEDDED;
    AppExecFwk::ElementName element;
    element.SetDeviceID("device123");
    element.SetBundleName("ohos.test.bundle");
    element.SetAbilityName("TestAbility");
    expected.elementName = element;
    expected.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    AppExecFwk::ElementName hostElement;
    hostElement.SetDeviceID("hostDevice");
    hostElement.SetBundleName("ohos.host.bundle");
    hostElement.SetAbilityName("HostAbility");
    expected.hostElementName = hostElement;
    expected.isBlockSubwindow = true;

    Parcel parcel;
    EXPECT_EQ(true, expected.Marshalling(parcel));

    auto result = std::unique_ptr<UIExtensionSessionInfo>(UIExtensionSessionInfo::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->persistentId, expected.persistentId);
    EXPECT_EQ(result->hostWindowId, expected.hostWindowId);
    EXPECT_EQ(result->uiExtensionUsage, expected.uiExtensionUsage);
    EXPECT_EQ(result->elementName.GetBundleName(), expected.elementName.GetBundleName());
    EXPECT_EQ(result->elementName.GetAbilityName(), expected.elementName.GetAbilityName());
    EXPECT_EQ(result->elementName.GetDeviceID(), expected.elementName.GetDeviceID());
    EXPECT_EQ(result->extensionAbilityType, expected.extensionAbilityType);
    EXPECT_EQ(result->hostElementName.GetBundleName(), expected.hostElementName.GetBundleName());
    EXPECT_EQ(result->hostElementName.GetAbilityName(), expected.hostElementName.GetAbilityName());
    EXPECT_EQ(result->hostElementName.GetDeviceID(), expected.hostElementName.GetDeviceID());
    EXPECT_EQ(result->isBlockSubwindow, expected.isBlockSubwindow);
}

/**
 * @tc.name: Unmarshalling_0300
 * @tc.desc: Unmarshalling with empty parcel, persistentId read fails
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionSessionInfoTest, Unmarshalling_0300, TestSize.Level1)
{
    Parcel parcel;
    EXPECT_EQ(nullptr, UIExtensionSessionInfo::Unmarshalling(parcel));
}

/**
 * @tc.name: Unmarshalling_0400
 * @tc.desc: Unmarshalling with parcel truncated before uiExtensionUsage
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionSessionInfoTest, Unmarshalling_0400, TestSize.Level1)
{
    Parcel parcel;
    parcel.WriteInt32(1);
    parcel.WriteUint32(1);
    EXPECT_EQ(nullptr, UIExtensionSessionInfo::Unmarshalling(parcel));
}

/**
 * @tc.name: Unmarshalling_0500
 * @tc.desc: Unmarshalling with null element parcelable
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionSessionInfoTest, Unmarshalling_0500, TestSize.Level1)
{
    Parcel parcel;
    parcel.WriteInt32(1);
    parcel.WriteUint32(1);
    parcel.WriteUint32(1);
    parcel.WriteParcelable(nullptr);
    EXPECT_EQ(nullptr, UIExtensionSessionInfo::Unmarshalling(parcel));
}

/**
 * @tc.name: Unmarshalling_0600
 * @tc.desc: Unmarshalling with parcel truncated before extensionAbilityType
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionSessionInfoTest, Unmarshalling_0600, TestSize.Level1)
{
    Parcel parcel;
    parcel.WriteInt32(1);
    parcel.WriteUint32(1);
    parcel.WriteUint32(1);
    parcel.WriteParcelable(new AppExecFwk::ElementName());
    EXPECT_EQ(nullptr, UIExtensionSessionInfo::Unmarshalling(parcel));
}

/**
 * @tc.name: Unmarshalling_0700
 * @tc.desc: Unmarshalling with parcel truncated before isBlockSubwindow
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionSessionInfoTest, Unmarshalling_0700, TestSize.Level1)
{
    Parcel parcel;
    parcel.WriteInt32(1);
    parcel.WriteUint32(1);
    parcel.WriteUint32(1);
    parcel.WriteParcelable(new AppExecFwk::ElementName());
    parcel.WriteInt32(static_cast<int32_t>(AppExecFwk::ExtensionAbilityType::SERVICE));
    parcel.WriteParcelable(new AppExecFwk::ElementName());
    EXPECT_EQ(nullptr, UIExtensionSessionInfo::Unmarshalling(parcel));
}

/**
 * @tc.name: Unmarshalling_0800
 * @tc.desc: Round-trip with boundary values (PRE_VIEW_EMBEDDED, persistentId=-1,
 *           hostWindowId=0, isBlockSubwindow=true)
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionSessionInfoTest, Unmarshalling_0800, TestSize.Level1)
{
    UIExtensionSessionInfo expected;
    expected.persistentId = -1;
    expected.hostWindowId = 0;
    expected.uiExtensionUsage = AAFwk::UIExtensionUsage::PRE_VIEW_EMBEDDED;
    expected.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    expected.isBlockSubwindow = true;

    Parcel parcel;
    EXPECT_EQ(true, expected.Marshalling(parcel));

    auto result = std::unique_ptr<UIExtensionSessionInfo>(UIExtensionSessionInfo::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->persistentId, -1);
    EXPECT_EQ(result->hostWindowId, 0u);
    EXPECT_EQ(result->uiExtensionUsage, AAFwk::UIExtensionUsage::PRE_VIEW_EMBEDDED);
    EXPECT_EQ(result->extensionAbilityType, AppExecFwk::ExtensionAbilityType::UNSPECIFIED);
    EXPECT_EQ(result->isBlockSubwindow, true);
}

/**
 * @tc.name: Unmarshalling_0900
 * @tc.desc: Round-trip with MODAL (default value 0) and isBlockSubwindow=false
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionSessionInfoTest, Unmarshalling_0900, TestSize.Level1)
{
    UIExtensionSessionInfo expected;
    expected.persistentId = 0;
    expected.hostWindowId = 0;
    expected.uiExtensionUsage = AAFwk::UIExtensionUsage::MODAL;
    expected.isBlockSubwindow = false;

    Parcel parcel;
    EXPECT_EQ(true, expected.Marshalling(parcel));

    auto result = std::unique_ptr<UIExtensionSessionInfo>(UIExtensionSessionInfo::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->persistentId, 0);
    EXPECT_EQ(result->hostWindowId, 0u);
    EXPECT_EQ(result->uiExtensionUsage, AAFwk::UIExtensionUsage::MODAL);
    EXPECT_EQ(result->isBlockSubwindow, false);
}
} // namespace AbilityRuntime
} // namespace OHOS