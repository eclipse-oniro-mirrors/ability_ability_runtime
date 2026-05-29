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

#define private public
#include "bundle_mgr_helper.h"
#undef private

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

// ========== QueryEnabledAbilityInfo(with appIndex) tests ==========

/**
 * @tc.name: QueryEnabledAbilityInfo_001
 * @tc.desc: Test QueryEnabledAbilityInfo with exact appIndex match
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryEnabledAbilityInfo_001, TestSize.Level1)
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
    EXPECT_TRUE(bundleMgrHelper->QueryEnabledAbilityInfo(want, DEFAULT_USERID, 1, result));
    EXPECT_EQ(result.appIndex, 1);
}

/**
 * @tc.name: QueryEnabledAbilityInfo_002
 * @tc.desc: Test QueryEnabledAbilityInfo fallback to min valid appIndex when exact not found
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryEnabledAbilityInfo_002, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");

    AbilityInfo info1;
    info1.bundleName = "com.test.bundle";
    info1.appIndex = 0;
    info1.applicationInfo.enabled = true;

    AbilityInfo info2;
    info2.bundleName = "com.test.bundle";
    info2.appIndex = 2;
    info2.applicationInfo.enabled = true;

    MockBundleMgrStub::mockAbilityInfos = {info1, info2};
    MockBundleMgrStub::mockQueryAbilityInfosV9Ret = ERR_OK;

    AbilityInfo result;
    EXPECT_TRUE(bundleMgrHelper->QueryEnabledAbilityInfo(want, DEFAULT_USERID, 5, result));
    EXPECT_EQ(result.appIndex, 0);
}

/**
 * @tc.name: QueryEnabledAbilityInfo_003
 * @tc.desc: Test QueryEnabledAbilityInfo returns false when no enabled ability
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryEnabledAbilityInfo_003, TestSize.Level1)
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
    EXPECT_FALSE(bundleMgrHelper->QueryEnabledAbilityInfo(want, DEFAULT_USERID, 0, result));
}

/**
 * @tc.name: QueryEnabledAbilityInfo_004
 * @tc.desc: Test QueryEnabledAbilityInfo returns false when QueryAbilityInfos fails
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryEnabledAbilityInfo_004, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");

    MockBundleMgrStub::mockQueryAbilityInfosV9Ret = ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR;

    AbilityInfo result;
    EXPECT_FALSE(bundleMgrHelper->QueryEnabledAbilityInfo(want, DEFAULT_USERID, 0, result));
}

/**
 * @tc.name: QueryEnabledAbilityInfo_005
 * @tc.desc: Test QueryEnabledAbilityInfo with empty results
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryEnabledAbilityInfo_005, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");

    MockBundleMgrStub::mockAbilityInfos.clear();
    MockBundleMgrStub::mockQueryAbilityInfosV9Ret = ERR_OK;

    AbilityInfo result;
    EXPECT_FALSE(bundleMgrHelper->QueryEnabledAbilityInfo(want, DEFAULT_USERID, 0, result));
}

/**
 * @tc.name: QueryEnabledAbilityInfo_006
 * @tc.desc: Test QueryEnabledAbilityInfo skips disabled and returns min enabled appIndex
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryEnabledAbilityInfo_006, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");

    AbilityInfo disabledInfo;
    disabledInfo.bundleName = "com.test.bundle";
    disabledInfo.appIndex = 0;
    disabledInfo.applicationInfo.enabled = false;

    AbilityInfo enabledInfo1;
    enabledInfo1.bundleName = "com.test.bundle";
    enabledInfo1.appIndex = 1;
    enabledInfo1.applicationInfo.enabled = true;

    AbilityInfo enabledInfo2;
    enabledInfo2.bundleName = "com.test.bundle";
    enabledInfo2.appIndex = 2;
    enabledInfo2.applicationInfo.enabled = true;

    MockBundleMgrStub::mockAbilityInfos = {disabledInfo, enabledInfo1, enabledInfo2};
    MockBundleMgrStub::mockQueryAbilityInfosV9Ret = ERR_OK;

    AbilityInfo result;
    EXPECT_TRUE(bundleMgrHelper->QueryEnabledAbilityInfo(want, DEFAULT_USERID, 99, result));
    EXPECT_EQ(result.appIndex, 1);
}

/**
 * @tc.name: QueryEnabledAbilityInfo_007
 * @tc.desc: Test QueryEnabledAbilityInfo exact match takes priority over fallback
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperThirdTest, QueryEnabledAbilityInfo_007, TestSize.Level1)
{
    Want want;
    want.SetElementName("com.test.bundle", "MainAbility");

    AbilityInfo info1;
    info1.bundleName = "com.test.bundle";
    info1.appIndex = 0;
    info1.applicationInfo.enabled = true;

    AbilityInfo info2;
    info2.bundleName = "com.test.bundle";
    info2.appIndex = 3;
    info2.applicationInfo.enabled = true;

    MockBundleMgrStub::mockAbilityInfos = {info1, info2};
    MockBundleMgrStub::mockQueryAbilityInfosV9Ret = ERR_OK;

    AbilityInfo result;
    EXPECT_TRUE(bundleMgrHelper->QueryEnabledAbilityInfo(want, DEFAULT_USERID, 3, result));
    EXPECT_EQ(result.appIndex, 3);
}

}  // namespace AppExecFwk
}  // namespace OHOS
