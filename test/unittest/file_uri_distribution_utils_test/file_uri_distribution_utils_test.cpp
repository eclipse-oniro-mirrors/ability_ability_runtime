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

#include "ability_manager_errors.h"
#include "event_report.h"
#define private public
#include "file_uri_distribution_utils.h"
#undef private
#include "system_ability_definition.h"
#include "tokenid_kit.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class FileUriDistributionUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void FileUriDistributionUtilsTest::SetUpTestCase() {}

void FileUriDistributionUtilsTest::TearDownTestCase() {}

void FileUriDistributionUtilsTest::SetUp() {}

void FileUriDistributionUtilsTest::TearDown() {}

/*
 * Feature: FUDUtils
 * Function: ConnectManager
 * SubFunction: NA
 * FunctionPoints: FUDUtils SendSystemAppGrantUriPermissionEvent
 */
HWTEST_F(FileUriDistributionUtilsTest, SendSystemAppGrantUriPermissionEvent_001, TestSize.Level1)
{
    std::vector<std::string> uriVec = { "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt" };
    const std::vector<CheckResult> resVec = { CheckResult(true, 0) };
    auto ret = FUDUtils::SendSystemAppGrantUriPermissionEvent(1001, 1002, uriVec, resVec);
    ASSERT_EQ(ret, false);
}

/*
 * Feature: FUDUtils
 * Function: SendShareUnPrivilegeUriEvent
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService SendShareUnPrivilegeUriEvent
 */
HWTEST_F(FileUriDistributionUtilsTest, SendShareUnPrivilegeUriEvent_001, TestSize.Level1)
{
    auto ret = FUDUtils::SendShareUnPrivilegeUriEvent(1001, 1002);
    EXPECT_EQ(ret, false);
}

/*
 * Feature: FUDUtils
 * Function: GetCurrentAccountId
 * SubFunction: NA
 * FunctionPoints: FUDUtils GetCurrentAccountId
 */
HWTEST_F(FileUriDistributionUtilsTest, GetCurrentAccountId_001, TestSize.Level1)
{
    auto testCurrentAccountId = FUDUtils::GetCurrentAccountId();
    EXPECT_EQ(testCurrentAccountId, 100);
}

/*
 * Feature: FUDUtils
 * Function: IsFoundationCall
 * SubFunction: NA
 * FunctionPoints: FUDUtils IsFoundationCall
 */
HWTEST_F(FileUriDistributionUtilsTest, IsFoundationCall_001, TestSize.Level1)
{
    auto testIsFoundationCall = FUDUtils::IsFoundationCall();
    EXPECT_EQ(testIsFoundationCall, false);
}

/*
 * Feature: FUDUtils
 * Function: IsSAOrSystemAppCall
 * SubFunction: NA
 * FunctionPoints: FUDUtils IsSAOrSystemAppCall
 */
HWTEST_F(FileUriDistributionUtilsTest, IsSAOrSystemAppCall_001, TestSize.Level1)
{
    auto testIsSAOrSystemAppCall = FUDUtils::IsSAOrSystemAppCall();
    EXPECT_EQ(testIsSAOrSystemAppCall, false);
}

/*
 * Feature: FUDUtils
 * Function: IsSystemAppCall
 * SubFunction: NA
 * FunctionPoints: FUDUtils IsSystemAppCall
 */
HWTEST_F(FileUriDistributionUtilsTest, IsSystemAppCall_001, TestSize.Level1)
{
    auto testIsSystemAppCall = FUDUtils::IsSystemAppCall();
    EXPECT_EQ(testIsSystemAppCall, false);
}

/*
 * Feature: FUDUtils
 * Function: CheckIsSystemAppByBundleName
 * SubFunction: NA
 * FunctionPoints: FUDUtils CheckIsSystemAppByBundleName
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckIsSystemAppByBundleName_001, TestSize.Level1)
{
    std::string bundleName = "";
    auto testIsSystemApp = FUDUtils::CheckIsSystemAppByBundleName(bundleName);
    EXPECT_EQ(testIsSystemApp, false);
}

/*
 * Feature: FUDUtils
 * Function: CheckIsSystemAppByTokenId
 * SubFunction: NA
 * FunctionPoints: FUDUtils CheckIsSystemAppByTokenId
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckIsSystemAppByTokenId_001, TestSize.Level1)
{
    uint32_t tokenId = 1001;
    auto testIsSystemApp = FUDUtils::CheckIsSystemAppByTokenId(tokenId);
    EXPECT_EQ(testIsSystemApp, false);
}

/*
 * Feature: FUDUtils
 * Function: GetBundleNameByTokenId
 * SubFunction: NA
 * FunctionPoints: FUDUtils GetBundleNameByTokenId
 */
HWTEST_F(FileUriDistributionUtilsTest, GetBundleNameByTokenId_001, TestSize.Level1)
{
    uint32_t tokenId = 1001;
    std::string bundleName = "";
    auto testGetBundleNameByTokenIdFlag = FUDUtils::GetBundleNameByTokenId(tokenId, bundleName);
    EXPECT_EQ (testGetBundleNameByTokenIdFlag, false);
}

/*
 * Feature: FUDUtils
 * Function: GetTokenIdByBundleName
 * SubFunction: NA
 * FunctionPoints: FUDUtils GetTokenIdByBundleName
 */
HWTEST_F(FileUriDistributionUtilsTest, GetTokenIdByBundleName_001, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    int32_t appIndex = 10;
    int32_t userId = 100;
    uint32_t tokenId;
    auto testTokenId = FUDUtils::GetTokenIdByBundleName(bundleName, appIndex, userId, tokenId);
    EXPECT_EQ(testTokenId, ERR_GET_TARGET_BUNDLE_INFO_FAILED);
}

/*
 * Feature: FUDUtils
 * Function: IsPrivilegedSACall
 * SubFunction: NA
 * FunctionPoints: FUDUtils IsPrivilegedSACall
 */
HWTEST_F(FileUriDistributionUtilsTest, IsPrivilegedSACall_001, TestSize.Level1)
{
    bool ret = FUDUtils::IsPrivilegedSACall();
    EXPECT_EQ(ret, false);
}

/*
 * Feature: FUDUtils
 * Function: IsUdmfOrPasteboardCall
 * SubFunction: NA
 * FunctionPoints: FUDUtils IsUdmfOrPasteboardCall
 */
HWTEST_F(FileUriDistributionUtilsTest, IsUdmfOrPasteboardCall_001, TestSize.Level1)
{
    bool ret = FUDUtils::IsUdmfOrPasteboardCall();
    EXPECT_EQ(ret, false);
}

/*
 * Feature: FUDUtils
 * Function: IsDFSCall
 * SubFunction: NA
 * FunctionPoints: FUDUtils IsDFSCall
 */
HWTEST_F(FileUriDistributionUtilsTest, IsDFSCall_001, TestSize.Level1)
{
    bool ret = FUDUtils::IsDFSCall();
    EXPECT_EQ(ret, false);
}
}  // namespace AAFwk
}  // namespace OHOS