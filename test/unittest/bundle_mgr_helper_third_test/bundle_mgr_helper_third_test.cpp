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

#include "bundle_mgr_helper.h"

#include "mock_bundle_mgr_stub.h"
#include "want.h"

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t DEFAULT_USERID = 100;
}  // namespace

class BundleMgrHelperThirdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static std::shared_ptr<BundleMgrHelper> bundleMgrHelper;
};

std::shared_ptr<BundleMgrHelper> BundleMgrHelperThirdTest::bundleMgrHelper =
    DelayedSingleton<BundleMgrHelper>::GetInstance();

void BundleMgrHelperThirdTest::SetUpTestCase() {}

void BundleMgrHelperThirdTest::TearDownTestCase() {}

void BundleMgrHelperThirdTest::SetUp()
{
    MockBundleMgrStub::mockQueryAbilityInfosV9Ret = ERR_OK;
    MockBundleMgrStub::mockAbilityInfos.clear();
    MockBundleMgrStub::mockQueryExtensionAbilityInfosV9Ret = ERR_OK;
    MockBundleMgrStub::mockExtensionInfos.clear();
    bundleMgrHelper->bundleMgr_ = new MockBundleMgrStub();
}

void BundleMgrHelperThirdTest::TearDown()
{
    bundleMgrHelper->bundleMgr_ = nullptr;
}

// ========== QueryAbilityInfos tests ==========

/**
 * @tc.name: QueryAbilityInfos_001
 * @tc.desc: Test QueryAbilityInfos returns success with results
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryAbilityInfos_001, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");

    AbilityInfo info;
    info.bundleName = "com.test.bundle";
    info.appIndex = 0;
    info.applicationInfo.enabled = true;
    MockBundleMgrStub::mockAbilityInfos = {info};
    MockBundleMgrStub::mockQueryAbilityInfosV9Ret = ERR_OK;

    std::vector<AbilityInfo> abilityInfos;
    auto ret = bundleMgrHelper->QueryAbilityInfos(want, DEFAULT_USERID, abilityInfos);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(abilityInfos.size(), 1u);
    EXPECT_EQ(abilityInfos[0].bundleName, "com.test.bundle");
}

/**
 * @tc.name: QueryAbilityInfos_002
 * @tc.desc: Test QueryAbilityInfos returns multiple results
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryAbilityInfos_002, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");

    AbilityInfo info1;
    info1.bundleName = "com.test.bundle";
    info1.appIndex = 0;
    info1.applicationInfo.enabled = true;

    AbilityInfo info2;
    info2.bundleName = "com.test.bundle";
    info2.appIndex = 1;
    info2.applicationInfo.enabled = true;

    MockBundleMgrStub::mockAbilityInfos = {info1, info2};
    MockBundleMgrStub::mockQueryAbilityInfosV9Ret = ERR_OK;

    std::vector<AbilityInfo> abilityInfos;
    auto ret = bundleMgrHelper->QueryAbilityInfos(want, DEFAULT_USERID, abilityInfos);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(abilityInfos.size(), 2u);
}

/**
 * @tc.name: QueryAbilityInfos_003
 * @tc.desc: Test QueryAbilityInfos with IPC error
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryAbilityInfos_003, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");

    MockBundleMgrStub::mockQueryAbilityInfosV9Ret = ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;

    std::vector<AbilityInfo> abilityInfos;
    auto ret = bundleMgrHelper->QueryAbilityInfos(want, DEFAULT_USERID, abilityInfos);

    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: QueryAbilityInfos_004
 * @tc.desc: Test QueryAbilityInfos with null bundleMgr
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryAbilityInfos_004, TestSize.Level1)
{
    bundleMgrHelper->bundleMgr_ = nullptr;
    bundleMgrHelper->bmsReady_ = false;

    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");

    std::vector<AbilityInfo> abilityInfos;
    auto ret = bundleMgrHelper->QueryAbilityInfos(want, DEFAULT_USERID, abilityInfos);

    EXPECT_EQ(ret, ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR);
}

// ========== QueryEnabledAbilityInfo tests ==========

/**
 * @tc.name: QueryEnabledAbilityInfo_001
 * @tc.desc: Test QueryEnabledAbilityInfo success with single enabled result
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryEnabledAbilityInfo_001, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");

    AbilityInfo info;
    info.bundleName = "com.test.bundle";
    info.appIndex = 0;
    info.applicationInfo.enabled = true;
    MockBundleMgrStub::mockAbilityInfos = {info};
    MockBundleMgrStub::mockQueryAbilityInfosV9Ret = ERR_OK;

    AbilityInfo result;
    EXPECT_TRUE(bundleMgrHelper->QueryEnabledAbilityInfo(want, DEFAULT_USERID, result));
    EXPECT_EQ(result.bundleName, "com.test.bundle");
    EXPECT_EQ(result.appIndex, 0);
}

/**
 * @tc.name: QueryEnabledAbilityInfo_002
 * @tc.desc: Test QueryEnabledAbilityInfo returns false when disabled
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryEnabledAbilityInfo_002, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");

    AbilityInfo info;
    info.bundleName = "com.test.bundle";
    info.appIndex = 0;
    info.applicationInfo.enabled = false;
    MockBundleMgrStub::mockAbilityInfos = {info};
    MockBundleMgrStub::mockQueryAbilityInfosV9Ret = ERR_OK;

    AbilityInfo result;
    EXPECT_FALSE(bundleMgrHelper->QueryEnabledAbilityInfo(want, DEFAULT_USERID, result));
}

/**
 * @tc.name: QueryEnabledAbilityInfo_003
 * @tc.desc: Test QueryEnabledAbilityInfo returns false when QueryAbilityInfos fails
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryEnabledAbilityInfo_003, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");

    MockBundleMgrStub::mockQueryAbilityInfosV9Ret = ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;

    AbilityInfo result;
    EXPECT_FALSE(bundleMgrHelper->QueryEnabledAbilityInfo(want, DEFAULT_USERID, result));
}

/**
 * @tc.name: QueryEnabledAbilityInfo_004
 * @tc.desc: Test QueryEnabledAbilityInfo returns false with empty results
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryEnabledAbilityInfo_004, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");

    MockBundleMgrStub::mockAbilityInfos.clear();
    MockBundleMgrStub::mockQueryAbilityInfosV9Ret = ERR_OK;

    AbilityInfo result;
    EXPECT_FALSE(bundleMgrHelper->QueryEnabledAbilityInfo(want, DEFAULT_USERID, result));
}

/**
 * @tc.name: QueryEnabledAbilityInfo_005
 * @tc.desc: Test QueryEnabledAbilityInfo returns false with multiple results (not exactly 1)
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryEnabledAbilityInfo_005, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");

    AbilityInfo info1;
    info1.bundleName = "com.test.bundle";
    info1.appIndex = 0;
    info1.applicationInfo.enabled = true;

    AbilityInfo info2;
    info2.bundleName = "com.test.bundle";
    info2.appIndex = 1;
    info2.applicationInfo.enabled = true;

    MockBundleMgrStub::mockAbilityInfos = {info1, info2};
    MockBundleMgrStub::mockQueryAbilityInfosV9Ret = ERR_OK;

    AbilityInfo result;
    EXPECT_FALSE(bundleMgrHelper->QueryEnabledAbilityInfo(want, DEFAULT_USERID, result));
}

// ========== QueryExtensionAbilityInfosV9 tests ==========

/**
 * @tc.name: QueryExtensionAbilityInfosV9_001
 * @tc.desc: Test QueryExtensionAbilityInfosV9 returns success with results
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryExtensionAbilityInfosV9_001, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "ServiceExtAbility");

    ExtensionAbilityInfo info;
    info.bundleName = "com.test.bundle";
    info.name = "ServiceExtAbility";
    info.appIndex = 0;
    MockBundleMgrStub::mockExtensionInfos = {info};
    MockBundleMgrStub::mockQueryExtensionAbilityInfosV9Ret = ERR_OK;

    std::vector<ExtensionAbilityInfo> extensionInfos;
    auto ret = bundleMgrHelper->QueryExtensionAbilityInfosV9(want, DEFAULT_USERID, extensionInfos);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(extensionInfos.size(), 1u);
    EXPECT_EQ(extensionInfos[0].bundleName, "com.test.bundle");
}

/**
 * @tc.name: QueryExtensionAbilityInfosV9_002
 * @tc.desc: Test QueryExtensionAbilityInfosV9 with IPC error
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryExtensionAbilityInfosV9_002, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "ServiceExtAbility");

    MockBundleMgrStub::mockQueryExtensionAbilityInfosV9Ret = ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;

    std::vector<ExtensionAbilityInfo> extensionInfos;
    auto ret = bundleMgrHelper->QueryExtensionAbilityInfosV9(want, DEFAULT_USERID, extensionInfos);

    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: QueryExtensionAbilityInfosV9_003
 * @tc.desc: Test QueryExtensionAbilityInfosV9 with null bundleMgr
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryExtensionAbilityInfosV9_003, TestSize.Level1)
{
    bundleMgrHelper->bundleMgr_ = nullptr;
    bundleMgrHelper->bmsReady_ = false;

    Want want;
    want.SetElementName("com.test.bundle", "ServiceExtAbility");

    std::vector<ExtensionAbilityInfo> extensionInfos;
    auto ret = bundleMgrHelper->QueryExtensionAbilityInfosV9(want, DEFAULT_USERID, extensionInfos);

    EXPECT_EQ(ret, ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR);
}

// ========== QueryEnabledExtensionAbilityInfo tests ==========

/**
 * @tc.name: QueryEnabledExtensionAbilityInfo_001
 * @tc.desc: Test QueryEnabledExtensionAbilityInfo success with single enabled result
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryEnabledExtensionAbilityInfo_001, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "ServiceExtAbility");

    ExtensionAbilityInfo info;
    info.bundleName = "com.test.bundle";
    info.name = "ServiceExtAbility";
    info.appIndex = 2;
    info.applicationInfo.enabled = true;
    MockBundleMgrStub::mockExtensionInfos = {info};
    MockBundleMgrStub::mockQueryExtensionAbilityInfosV9Ret = ERR_OK;

    ExtensionAbilityInfo result;
    EXPECT_TRUE(bundleMgrHelper->QueryEnabledExtensionAbilityInfo(want, DEFAULT_USERID, result));
    EXPECT_EQ(result.bundleName, "com.test.bundle");
    EXPECT_EQ(result.appIndex, 2);
}

/**
 * @tc.name: QueryEnabledExtensionAbilityInfo_002
 * @tc.desc: Test QueryEnabledExtensionAbilityInfo returns false when disabled
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryEnabledExtensionAbilityInfo_002, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "ServiceExtAbility");

    ExtensionAbilityInfo info;
    info.bundleName = "com.test.bundle";
    info.applicationInfo.enabled = false;
    MockBundleMgrStub::mockExtensionInfos = {info};
    MockBundleMgrStub::mockQueryExtensionAbilityInfosV9Ret = ERR_OK;

    ExtensionAbilityInfo result;
    EXPECT_FALSE(bundleMgrHelper->QueryEnabledExtensionAbilityInfo(want, DEFAULT_USERID, result));
}

/**
 * @tc.name: QueryEnabledExtensionAbilityInfo_003
 * @tc.desc: Test QueryEnabledExtensionAbilityInfo returns false when query fails
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryEnabledExtensionAbilityInfo_003, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "ServiceExtAbility");

    MockBundleMgrStub::mockQueryExtensionAbilityInfosV9Ret = ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;

    ExtensionAbilityInfo result;
    EXPECT_FALSE(bundleMgrHelper->QueryEnabledExtensionAbilityInfo(want, DEFAULT_USERID, result));
}

/**
 * @tc.name: QueryEnabledExtensionAbilityInfo_004
 * @tc.desc: Test QueryEnabledExtensionAbilityInfo returns false with empty results
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryEnabledExtensionAbilityInfo_004, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "ServiceExtAbility");

    MockBundleMgrStub::mockExtensionInfos.clear();
    MockBundleMgrStub::mockQueryExtensionAbilityInfosV9Ret = ERR_OK;

    ExtensionAbilityInfo result;
    EXPECT_FALSE(bundleMgrHelper->QueryEnabledExtensionAbilityInfo(want, DEFAULT_USERID, result));
}

/**
 * @tc.name: QueryEnabledExtensionAbilityInfo_005
 * @tc.desc: Test QueryEnabledExtensionAbilityInfo returns false with multiple results
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryEnabledExtensionAbilityInfo_005, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "ServiceExtAbility");

    ExtensionAbilityInfo info1;
    info1.bundleName = "com.test.bundle";
    info1.applicationInfo.enabled = true;

    ExtensionAbilityInfo info2;
    info2.bundleName = "com.test.bundle";
    info2.applicationInfo.enabled = true;

    MockBundleMgrStub::mockExtensionInfos = {info1, info2};
    MockBundleMgrStub::mockQueryExtensionAbilityInfosV9Ret = ERR_OK;

    ExtensionAbilityInfo result;
    EXPECT_FALSE(bundleMgrHelper->QueryEnabledExtensionAbilityInfo(want, DEFAULT_USERID, result));
}

}  // namespace AppExecFwk
}  // namespace OHOS
