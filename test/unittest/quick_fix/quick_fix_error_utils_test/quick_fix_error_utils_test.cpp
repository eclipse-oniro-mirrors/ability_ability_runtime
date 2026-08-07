/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "quick_fix_error_utils.h"
#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class QuickFixErrorUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void QuickFixErrorUtilsTest::SetUpTestCase(void)
{}

void QuickFixErrorUtilsTest::TearDownTestCase(void)
{}

void QuickFixErrorUtilsTest::SetUp()
{}

void QuickFixErrorUtilsTest::TearDown()
{}

/**
 * @tc.name: GetErrorCode_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorCode_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    int32_t errCode;

    // external error code
    errCode = QuickFixErrorUtil::GetErrorCode(ERR_QUICKFIX_PERMISSION_DENIED);
    EXPECT_EQ(errCode, ERR_QUICKFIX_PERMISSION_DENIED);

    // internal error code
    errCode = QuickFixErrorUtil::GetErrorCode(QUICK_FIX_COPY_FILES_FAILED);
    EXPECT_EQ(errCode, ERR_QUICKFIX_HQF_INVALID);

    // unknown error code
    errCode = QuickFixErrorUtil::GetErrorCode(1000); // 1000 is not a defined error code in quick fix
    EXPECT_EQ(errCode, ERR_QUICKFIX_INTERNAL_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorMessage_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorMessage_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::string errMsg;

    // external error code
    errMsg = QuickFixErrorUtil::GetErrorMessage(ERR_QUICKFIX_PARAM_INVALID);
    EXPECT_EQ(errMsg, "Invalid input parameter.");

    // internal error code
    errMsg = QuickFixErrorUtil::GetErrorMessage(QUICK_FIX_WRITE_PARCEL_FAILED);
    EXPECT_EQ(errMsg, "Internal error. Write parcel failed.");

    // unknown error code
    errMsg = QuickFixErrorUtil::GetErrorMessage(1000); // 1000 is not a defined error code in quick fix
    EXPECT_EQ(errMsg, "Internal error.");

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorCode_0200
 * @tc.desc: QUICK_FIX_COPY_FILES_FAILED should map to ERR_QUICKFIX_HQF_INVALID.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorCode_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_COPY_FILES_FAILED), ERR_QUICKFIX_HQF_INVALID);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorCode_0300
 * @tc.desc: QUICK_FIX_INVALID_PARAM should map to ERR_QUICKFIX_PARAM_INVALID.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorCode_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_INVALID_PARAM), ERR_QUICKFIX_PARAM_INVALID);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorCode_0400
 * @tc.desc: QUICK_FIX_VERIFY_PERMISSION_FAILED should map to ERR_QUICKFIX_PERMISSION_DENIED.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorCode_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_VERIFY_PERMISSION_FAILED), ERR_QUICKFIX_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorCode_0500
 * @tc.desc: QUICK_FIX_GET_BUNDLE_INFO_FAILED should map to ERR_QUICKFIX_BUNDLE_NAME_INVALID.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorCode_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_GET_BUNDLE_INFO_FAILED), ERR_QUICKFIX_BUNDLE_NAME_INVALID);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorCode_0600
 * @tc.desc: QUICK_FIX_DEPLOY_FAILED should map to ERR_QUICKFIX_HQF_DEPLOY_FAILED.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorCode_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_DEPLOY_FAILED), ERR_QUICKFIX_HQF_DEPLOY_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorCode_0700
 * @tc.desc: QUICK_FIX_SWICH_FAILED should map to ERR_QUICKFIX_HQF_SWITCH_FAILED.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorCode_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_SWICH_FAILED), ERR_QUICKFIX_HQF_SWITCH_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorCode_0800
 * @tc.desc: QUICK_FIX_DELETE_FAILED should map to ERR_QUICKFIX_HQF_DELETE_FAILED.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorCode_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_DELETE_FAILED), ERR_QUICKFIX_HQF_DELETE_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorCode_0900
 * @tc.desc: QUICK_FIX_NOTIFY_LOAD_PATCH_FAILED should map to ERR_QUICKFIX_LOAD_PATCH_FAILED.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorCode_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_NOTIFY_LOAD_PATCH_FAILED), ERR_QUICKFIX_LOAD_PATCH_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorCode_1000
 * @tc.desc: QUICK_FIX_NOTIFY_UNLOAD_PATCH_FAILED should map to ERR_QUICKFIX_UNLOAD_PATCH_FAILED.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorCode_1000, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_NOTIFY_UNLOAD_PATCH_FAILED), ERR_QUICKFIX_UNLOAD_PATCH_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorCode_1100
 * @tc.desc: QUICK_FIX_NOT_SYSTEM_APP should map to ERR_QUICKFIX_NOT_SYSTEM_APP.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorCode_1100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_NOT_SYSTEM_APP), ERR_QUICKFIX_NOT_SYSTEM_APP);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorCode_1200
 * @tc.desc: QUICK_FIX_DEPLOYING_TASK should map to ERR_QUICKFIX_DEPLOYING_TASK.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorCode_1200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_DEPLOYING_TASK), ERR_QUICKFIX_DEPLOYING_TASK);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorCode_1300
 * @tc.desc: internal codes mapped to INTERNAL_ERROR should all return ERR_QUICKFIX_INTERNAL_ERROR.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorCode_1300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_WRITE_PARCEL_FAILED), ERR_QUICKFIX_INTERNAL_ERROR);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_READ_PARCEL_FAILED), ERR_QUICKFIX_INTERNAL_ERROR);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_SEND_REQUEST_FAILED), ERR_QUICKFIX_INTERNAL_ERROR);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_CONNECT_FAILED), ERR_QUICKFIX_INTERNAL_ERROR);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_NOTIFY_RELOAD_PAGE_FAILED), ERR_QUICKFIX_INTERNAL_ERROR);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_REGISTER_OBSERVER_FAILED), ERR_QUICKFIX_INTERNAL_ERROR);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_APPMGR_INVALID), ERR_QUICKFIX_INTERNAL_ERROR);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_BUNDLEMGR_INVALID), ERR_QUICKFIX_INTERNAL_ERROR);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_SET_INFO_FAILED), ERR_QUICKFIX_INTERNAL_ERROR);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_PROCESS_TIMEOUT), ERR_QUICKFIX_INTERNAL_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorCode_1400
 * @tc.desc: all external error codes should pass through unchanged.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorCode_1400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(ERR_OK), ERR_OK);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(ERR_QUICKFIX_PERMISSION_DENIED), ERR_QUICKFIX_PERMISSION_DENIED);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(ERR_QUICKFIX_NOT_SYSTEM_APP), ERR_QUICKFIX_NOT_SYSTEM_APP);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(ERR_QUICKFIX_PARAM_INVALID), ERR_QUICKFIX_PARAM_INVALID);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(ERR_QUICKFIX_BUNDLE_NAME_INVALID), ERR_QUICKFIX_BUNDLE_NAME_INVALID);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(ERR_QUICKFIX_HQF_INVALID), ERR_QUICKFIX_HQF_INVALID);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorCode_1500
 * @tc.desc: remaining external error codes should pass through unchanged.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorCode_1500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(ERR_QUICKFIX_HQF_DEPLOY_FAILED), ERR_QUICKFIX_HQF_DEPLOY_FAILED);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(ERR_QUICKFIX_HQF_SWITCH_FAILED), ERR_QUICKFIX_HQF_SWITCH_FAILED);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(ERR_QUICKFIX_HQF_DELETE_FAILED), ERR_QUICKFIX_HQF_DELETE_FAILED);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(ERR_QUICKFIX_LOAD_PATCH_FAILED), ERR_QUICKFIX_LOAD_PATCH_FAILED);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(ERR_QUICKFIX_UNLOAD_PATCH_FAILED), ERR_QUICKFIX_UNLOAD_PATCH_FAILED);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(ERR_QUICKFIX_INTERNAL_ERROR), ERR_QUICKFIX_INTERNAL_ERROR);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(ERR_QUICKFIX_DEPLOYING_TASK), ERR_QUICKFIX_DEPLOYING_TASK);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorCode_1600
 * @tc.desc: QUICK_FIX_OK should map to ERR_OK.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorCode_1600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(QUICK_FIX_OK), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorCode_1700
 * @tc.desc: boundary values should fall back to ERR_QUICKFIX_INTERNAL_ERROR.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorCode_1700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(-1), ERR_QUICKFIX_INTERNAL_ERROR);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(100), ERR_QUICKFIX_INTERNAL_ERROR);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorCode(500), ERR_QUICKFIX_INTERNAL_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorMessage_0200
 * @tc.desc: ERR_QUICKFIX_PERMISSION_DENIED should map to its external message.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorMessage_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorMessage(ERR_QUICKFIX_PERMISSION_DENIED),
        "The application does not have permission to call the interface.");
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorMessage_0300
 * @tc.desc: ERR_QUICKFIX_NOT_SYSTEM_APP should map to its external message.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorMessage_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorMessage(ERR_QUICKFIX_NOT_SYSTEM_APP),
        "The application is not system-app, can not use system-api.");
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorMessage_0400
 * @tc.desc: ERR_QUICKFIX_BUNDLE_NAME_INVALID should map to its external message.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorMessage_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorMessage(ERR_QUICKFIX_BUNDLE_NAME_INVALID),
        "The bundle does not exist or no patch has been applied.");
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorMessage_0500
 * @tc.desc: ERR_QUICKFIX_HQF_INVALID should map to its external message.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorMessage_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorMessage(ERR_QUICKFIX_HQF_INVALID),
        "The specified hqf is invalid. Hqf may not exist or inaccessible.");
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorMessage_0600
 * @tc.desc: ERR_QUICKFIX_DEPLOYING_TASK should map to its external message.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorMessage_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorMessage(ERR_QUICKFIX_DEPLOYING_TASK),
        "The application has an ongoing quick fix task.");
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorMessage_0700
 * @tc.desc: internal code with internal-message entry should append the internal message.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorMessage_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorMessage(QUICK_FIX_CONNECT_FAILED), "Internal error. Connect failed.");
    EXPECT_EQ(QuickFixErrorUtil::GetErrorMessage(QUICK_FIX_PROCESS_TIMEOUT), "Internal error. Process timeout.");
    EXPECT_EQ(QuickFixErrorUtil::GetErrorMessage(QUICK_FIX_APPMGR_INVALID), "Internal error. AppMgr invalid.");
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorMessage_0800
 * @tc.desc: internal code mapped to a distinct external code should return only the external message.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorMessage_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorMessage(QUICK_FIX_DEPLOY_FAILED), "Deploy hqf failed.");
    EXPECT_EQ(QuickFixErrorUtil::GetErrorMessage(QUICK_FIX_SWICH_FAILED), "Switch hqf failed.");
    EXPECT_EQ(QuickFixErrorUtil::GetErrorMessage(QUICK_FIX_DELETE_FAILED), "Failed to remove the patch package.");
    EXPECT_EQ(QuickFixErrorUtil::GetErrorMessage(QUICK_FIX_NOTIFY_LOAD_PATCH_FAILED), "Load patch failed.");
    EXPECT_EQ(QuickFixErrorUtil::GetErrorMessage(QUICK_FIX_NOTIFY_UNLOAD_PATCH_FAILED), "Unload patch failed.");
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetErrorMessage_0900
 * @tc.desc: ERR_OK should map to the success message.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixErrorUtilsTest, GetErrorMessage_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_EQ(QuickFixErrorUtil::GetErrorMessage(ERR_OK), "Success.");
    EXPECT_EQ(QuickFixErrorUtil::GetErrorMessage(QUICK_FIX_OK), "Success.");
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}
} // namespace AAFwk
} // namespace OHOS