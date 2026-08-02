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
#include "bundle_mgr_helper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class BundleMgrHelperSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static std::shared_ptr<BundleMgrHelper> bundleMgrHelper;
};

std::shared_ptr<BundleMgrHelper> BundleMgrHelperSecondTest::bundleMgrHelper =
    DelayedSingleton<BundleMgrHelper>::GetInstance();

void BundleMgrHelperSecondTest::SetUpTestCase(void)
{}

void BundleMgrHelperSecondTest::TearDownTestCase(void)
{}

void BundleMgrHelperSecondTest::SetUp()
{}

void BundleMgrHelperSecondTest::TearDown()
{}

/**
 * @tc.name: BundleMgrHelperSecondTest_GetNameAndIndexForUid_001
 * @tc.desc: GetNameAndIndexForUid
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperSecondTest, BundleMgrHelperSecondTest_GetNameAndIndexForUid_001, TestSize.Level1)
{
    std::string bundleName = "bundleName";
    int32_t uid = 1;
    int32_t appIndex = 1;
    auto ret = bundleMgrHelper->GetNameAndIndexForUid(uid, bundleName, appIndex);
    EXPECT_NE(ret, ERR_APPEXECFWK_SERVICE_INTERNAL_ERROR);
}

/**
 * @tc.name: BundleMgrHelperSecondTest_GetAppIdByBundleName_001
 * @tc.desc: GetAppIdByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperSecondTest, BundleMgrHelperSecondTest_GetAppIdByBundleName_001, TestSize.Level1)
{
    std::string bundleName = "";
    int32_t userId = 1;
    auto ret = bundleMgrHelper->GetAppIdByBundleName(bundleName, userId);
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name: BundleMgrHelperSecondTest_GetStringById_001
 * @tc.desc: GetStringById
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperSecondTest, BundleMgrHelperSecondTest_GetAbilityLabel_001, TestSize.Level1)
{
    std::string bundleName = "bundleName";
    std::string moduleName = "moduleName";
    uint32_t resId= 3;
    int32_t userId = 1;
    auto ret = bundleMgrHelper->GetStringById(bundleName, moduleName, resId, userId);
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name: BundleMgrHelperSecondTest_GetDataDir_001
 * @tc.desc: GetDataDir
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperSecondTest, BundleMgrHelperSecondTest_GetDataDir_001, TestSize.Level1)
{
    std::string bundleName = "bundleName";
    int32_t appIndex = 1;
    auto ret = bundleMgrHelper->GetDataDir(bundleName, appIndex);
    EXPECT_NE(ret, "");
}

/**
 * @tc.name: BundleMgrHelperSecondTest_GetPluginInfoForTarget_001
 * @tc.desc: GetDataDir
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperSecondTest, BundleMgrHelperSecondTest_GetPluginInfoForTarget_001, TestSize.Level1)
{
    std::string hostBundleName = "hostBundleName";
    std::string pluginBundleName = "pluginBundleName";
    AppExecFwk::PluginBundleInfo pluginBundleInfo;
    auto ret = bundleMgrHelper->GetPluginInfoForTarget(hostBundleName, pluginBundleName, 0, pluginBundleInfo);
    EXPECT_NE(ret, ERR_OK);
}
}  // namespace AppExecFwk
}  // namespace OHOS