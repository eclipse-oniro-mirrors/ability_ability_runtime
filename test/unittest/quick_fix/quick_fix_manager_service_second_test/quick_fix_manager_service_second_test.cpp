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

#include "bundle_mgr_interface.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "mock_bundle_installer_service.h"
#include "mock_bundle_manager.h"
#include "mock_bundle_manager_service.h"
#include "mock_quick_fix_util.h"
#include "mock_system_ability_manager.h"
#include "quick_fix_error_utils.h"
#define private public
#include "iservice_registry.h"
#include "quick_fix_manager_service.h"
#undef private
#include "system_ability_definition.h"
#include "mock_my_flag.h"
#include "bundle_mgr_helper.h"
#include "mock_permission_verification.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class QuickFixManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<QuickFixManagerService> quickFixMs_ = nullptr;
};

void QuickFixManagerServiceTest::SetUpTestCase(void)
{}

void QuickFixManagerServiceTest::TearDownTestCase(void)
{}

void QuickFixManagerServiceTest::SetUp()
{
    quickFixMs_ = QuickFixManagerService::GetInstance();
    ASSERT_NE(quickFixMs_, nullptr);

    auto ret = quickFixMs_->Init();
    ASSERT_TRUE(ret);
    ASSERT_NE(quickFixMs_->eventRunner_, nullptr);
    ASSERT_NE(quickFixMs_->eventHandler_, nullptr);
}

void QuickFixManagerServiceTest::TearDown()
{
}

/**
 * @tc.name: ApplyQuickFix_0100
 * @tc.desc: ApplyQuickFix
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerServiceTest, ApplyQuickFix_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    MyFlag::isAllowedToUseSystemAPIFlag_ = false;
    std::vector<std::string> quickFixFiles;
    int32_t ret = quickFixMs_->ApplyQuickFix(quickFixFiles);
    EXPECT_EQ(ret, QUICK_FIX_NOT_SYSTEM_APP);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ApplyQuickFix_0200
 * @tc.desc: ApplyQuickFix
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerServiceTest, ApplyQuickFix_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::isVerifyInstallBundlePermission_= false;
    std::vector<std::string> quickFixFiles;
    int32_t ret = quickFixMs_->ApplyQuickFix(quickFixFiles);
    EXPECT_EQ(ret, QUICK_FIX_VERIFY_PERMISSION_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ApplyQuickFix_0300
 * @tc.desc: ApplyQuickFix
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerServiceTest, ApplyQuickFix_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::isVerifyInstallBundlePermission_= true;
    MyFlag::isVerifyPrivilegedPermission_= false;
    std::vector<std::string> quickFixFiles;
    int32_t ret = quickFixMs_->ApplyQuickFix(quickFixFiles);
    EXPECT_EQ(ret, QUICK_FIX_CONNECT_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ApplyQuickFix_0400
 * @tc.desc: ApplyQuickFix
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerServiceTest, ApplyQuickFix_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::isVerifyInstallBundlePermission_= true;
    MyFlag::isVerifyPrivilegedPermission_= true;
    QuickFixUtil::setBundleMgrProxyNull_ = true;
    std::vector<std::string> quickFixFiles;
    int32_t ret = quickFixMs_->ApplyQuickFix(quickFixFiles);
    QuickFixUtil::setBundleMgrProxyNull_ = false;
    EXPECT_EQ(ret, QUICK_FIX_CONNECT_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ApplyQuickFix_0500
 * @tc.desc: ApplyQuickFix
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerServiceTest, ApplyQuickFix_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::isVerifyInstallBundlePermission_= true;
    MyFlag::isVerifyPrivilegedPermission_= true;
    QuickFixUtil::setAppManagerProxyNull_ = true;
    std::vector<std::string> quickFixFiles;
    int32_t ret = quickFixMs_->ApplyQuickFix(quickFixFiles);
    QuickFixUtil::setAppManagerProxyNull_ = false;
    EXPECT_EQ(ret, QUICK_FIX_CONNECT_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetApplyedQuickFixInfo_0100
 * @tc.desc: GetApplyedQuickFixInfo
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerServiceTest, GetApplyedQuickFixInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    MyFlag::isAllowedToUseSystemAPIFlag_ = false;
    std::string bundleName = "test bundleName";
    ApplicationQuickFixInfo quickFixFileInfo;
    int32_t ret = quickFixMs_->GetApplyedQuickFixInfo(bundleName, quickFixFileInfo);
    EXPECT_EQ(ret, QUICK_FIX_NOT_SYSTEM_APP);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetApplyedQuickFixInfo_0200
 * @tc.desc: GetApplyedQuickFixInfo
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerServiceTest, GetApplyedQuickFixInfo_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::isVerifyPrivilegedPermission_ = false;
    std::string bundleName = "test bundleName";
    ApplicationQuickFixInfo quickFixFileInfo;
    int32_t ret = quickFixMs_->GetApplyedQuickFixInfo(bundleName, quickFixFileInfo);
    EXPECT_EQ(ret, QUICK_FIX_VERIFY_PERMISSION_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetApplyedQuickFixInfo_0300
 * @tc.desc: GetApplyedQuickFixInfo
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerServiceTest, GetApplyedQuickFixInfo_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::isVerifyPrivilegedPermission_= true;
    QuickFixUtil::setBundleMgrProxyNull_ = true;
    std::string bundleName = "test bundleName";
    ApplicationQuickFixInfo quickFixFileInfo;
    int32_t ret = quickFixMs_->GetApplyedQuickFixInfo(bundleName, quickFixFileInfo);
    QuickFixUtil::setBundleMgrProxyNull_ = false;
    EXPECT_EQ(ret, QUICK_FIX_GET_BUNDLE_INFO_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: RevokeQuickFix_0100
 * @tc.desc: RevokeQuickFix
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerServiceTest, RevokeQuickFix_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    MyFlag::isAllowedToUseSystemAPIFlag_ = false;
    std::string bundleName = "test bundleName";
    int32_t ret = quickFixMs_->RevokeQuickFix(bundleName);
    EXPECT_EQ(ret, QUICK_FIX_NOT_SYSTEM_APP);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: RevokeQuickFix_0200
 * @tc.desc: RevokeQuickFix
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerServiceTest, RevokeQuickFix_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::isVerifyInstallBundlePermission_= false;
    std::string bundleName = "test bundleName";
    int32_t ret = quickFixMs_->RevokeQuickFix(bundleName);
    EXPECT_EQ(ret, QUICK_FIX_VERIFY_PERMISSION_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: RevokeQuickFix_0300
 * @tc.desc: RevokeQuickFix
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerServiceTest, RevokeQuickFix_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::isVerifyInstallBundlePermission_= true;
    MyFlag::isVerifyPrivilegedPermission_= false;
    std::string bundleName = "test bundleName";
    int32_t ret = quickFixMs_->RevokeQuickFix(bundleName);
    EXPECT_EQ(ret, QUICK_FIX_VERIFY_PERMISSION_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ApplyQuickFix_0600
 * @tc.desc: test ApplyQuickFix
 * @tc.type: FUNC
 */
HWTEST_F(QuickFixManagerServiceTest, ApplyQuickFix_0600, TestSize.Level1)
{
    MyFlag::isAllowedToUseSystemAPIFlag_ = false;
    MyFlag::verifyCallingPermissionFlag_ = false;
    std::vector<std::string> quickFixFiles;
    auto ret = quickFixMs_->ApplyQuickFix(quickFixFiles);
    EXPECT_EQ(ret, QUICK_FIX_NOT_SYSTEM_APP);

    MyFlag::verifyCallingPermissionFlag_ = true;
    ret = quickFixMs_->ApplyQuickFix(quickFixFiles);
    EXPECT_EQ(ret, QUICK_FIX_CONNECT_FAILED);
}

/**
 * @tc.name: ApplyQuickFix_0700
 * @tc.desc: test ApplyQuickFix
 * @tc.type: FUNC
 */
HWTEST_F(QuickFixManagerServiceTest, ApplyQuickFix_0700, TestSize.Level1)
{
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::isVerifyInstallBundlePermission_ = true;
    std::vector<std::string> quickFixFiles;
    auto ret = quickFixMs_->ApplyQuickFix(quickFixFiles);
    EXPECT_EQ(ret, QUICK_FIX_CONNECT_FAILED);

    MyFlag::isVerifyInstallBundlePermission_ = false;
    MyFlag::verifyCallingPermissionFlag_ = false;
    ret = quickFixMs_->ApplyQuickFix(quickFixFiles);
    EXPECT_EQ(ret, QUICK_FIX_VERIFY_PERMISSION_FAILED);

    MyFlag::verifyCallingPermissionFlag_ = true;
    ret = quickFixMs_->ApplyQuickFix(quickFixFiles);
    EXPECT_EQ(ret, QUICK_FIX_CONNECT_FAILED);
}

/**
 * @tc.name: GetApplyedQuickFixInfo_0400
 * @tc.desc: test GetApplyedQuickFixInfo
 * @tc.type: FUNC
 */
HWTEST_F(QuickFixManagerServiceTest, GetApplyedQuickFixInfo_0400, TestSize.Level1)
{
    MyFlag::isAllowedToUseSystemAPIFlag_ = false;
    MyFlag::verifyCallingPermissionFlag_ = false;
    std::string bundleName = "test bundleName";
    ApplicationQuickFixInfo quickFixFileInfo;
    auto ret = quickFixMs_->GetApplyedQuickFixInfo(bundleName, quickFixFileInfo);
    EXPECT_EQ(ret, QUICK_FIX_NOT_SYSTEM_APP);

    MyFlag::verifyCallingPermissionFlag_ = true;
    ret = quickFixMs_->GetApplyedQuickFixInfo(bundleName, quickFixFileInfo);
    EXPECT_EQ(ret, QUICK_FIX_GET_BUNDLE_INFO_FAILED);
}

/**
 * @tc.name: GetApplyedQuickFixInfo_0500
 * @tc.desc: test GetApplyedQuickFixInfo
 * @tc.type: FUNC
 */
HWTEST_F(QuickFixManagerServiceTest, GetApplyedQuickFixInfo_0500, TestSize.Level1)
{
    MyFlag::isAllowedToUseSystemAPIFlag_ = true;
    MyFlag::isVerifyPrivilegedPermission_ = true;
    std::string bundleName = "test bundleName";
    ApplicationQuickFixInfo quickFixFileInfo;
    auto ret = quickFixMs_->GetApplyedQuickFixInfo(bundleName, quickFixFileInfo);
    EXPECT_EQ(ret, QUICK_FIX_GET_BUNDLE_INFO_FAILED);

    MyFlag::isVerifyPrivilegedPermission_ = false;
    MyFlag::verifyCallingPermissionFlag_ = false;
    ret = quickFixMs_->GetApplyedQuickFixInfo(bundleName, quickFixFileInfo);
    EXPECT_EQ(ret, QUICK_FIX_VERIFY_PERMISSION_FAILED);

    MyFlag::verifyCallingPermissionFlag_ = true;
    ret = quickFixMs_->GetApplyedQuickFixInfo(bundleName, quickFixFileInfo);
    EXPECT_EQ(ret, QUICK_FIX_GET_BUNDLE_INFO_FAILED);
}
} // namespace AppExecFwk
} // namespace OHOS