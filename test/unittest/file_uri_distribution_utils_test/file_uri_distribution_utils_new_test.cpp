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
#include "system_ability_definition.h"
#include "tokenid_kit.h"
#define private public
#include "uri_permission_utils.h"
#undef private
#include "mock_my_flag.h"
#include "accesstoken_kit.h"
#include "uri.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class FileUriDistributionUtilsNewTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void FileUriDistributionUtilsNewTest::SetUpTestCase() {}

void FileUriDistributionUtilsNewTest::TearDownTestCase() {}

void FileUriDistributionUtilsNewTest::SetUp() {}

void FileUriDistributionUtilsNewTest::TearDown() {}

/*
 * Feature: UPMSUtils
 * Function: ConnectManager
 * SubFunction: NA
 * FunctionPoints: UPMSUtils SendShareUnPrivilegeUriEvent
 */
HWTEST_F(FileUriDistributionUtilsNewTest, SendShareUnPrivilegeUriEvent_001, TestSize.Level1)
{
    uint32_t callerTokenId = 1;
    uint32_t targetTokenId = 2;
    MyFlag::tokenInfos_.clear();
    MyFlag::tokenInfos_[callerTokenId] = TokenInfo(callerTokenId, Security::AccessToken::ATokenTypeEnum::TOKEN_HAP,
        "com.test.test", "com.test.test");
    MyFlag::tokenInfos_[targetTokenId] = TokenInfo(targetTokenId, Security::AccessToken::ATokenTypeEnum::TOKEN_HAP,
        "com.test.test", "com.test.test");
    auto result = UPMSUtils::SendShareUnPrivilegeUriEvent(callerTokenId, targetTokenId);
    EXPECT_TRUE(result);
}

/*
 * Feature: UPMSUtils
 * Function: ConnectManager
 * SubFunction: NA
 * FunctionPoints: UPMSUtils CheckAndCreateEventInfo
 */
HWTEST_F(FileUriDistributionUtilsNewTest, CheckAndCreateEventInfo_001, TestSize.Level1)
{
    uint32_t callerTokenId = 1;
    uint32_t targetTokenId = 2;
    EventInfo eventInfo;
    MyFlag::tokenInfos_.clear();
    MyFlag::tokenInfos_[callerTokenId] = TokenInfo(callerTokenId, Security::AccessToken::ATokenTypeEnum::TOKEN_HAP,
        "com.test.ohos", "com.test.ohos");
    MyFlag::tokenInfos_[targetTokenId] = TokenInfo(targetTokenId, Security::AccessToken::ATokenTypeEnum::TOKEN_HAP,
        "com.test.ohos", "com.test.ohos");
    auto result = UPMSUtils::CheckAndCreateEventInfo(callerTokenId, targetTokenId, eventInfo);
    EXPECT_FALSE(result);
}

/*
 * Feature: UPMSUtils
 * Function: GenerateFUDAppInfo
 * SubFunction: NA
 * FunctionPoints: UPMSUtils GenerateFUDAppInfo
 */
HWTEST_F(FileUriDistributionUtilsNewTest, GenerateFUDAppInfo_001, TestSize.Level1)
{
    uint32_t tokenId = 1001;
    FUDAppInfo info = { .tokenId = tokenId };
    MyFlag::tokenInfos_.clear();
    MyFlag::tokenInfos_[tokenId] = TokenInfo(tokenId, Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE);
    auto result = UPMSUtils::GenerateFUDAppInfo(info);
    EXPECT_FALSE(result);
}

/*
 * Feature: UPMSUtils
 * Function: GenerateFUDAppInfo
 * SubFunction: NA
 * FunctionPoints: UPMSUtils GenerateFUDAppInfo
 */
HWTEST_F(FileUriDistributionUtilsNewTest, GenerateFUDAppInfo_002, TestSize.Level1)
{
    uint32_t tokenId = 1001;
    FUDAppInfo info = { .tokenId = tokenId };
    MyFlag::tokenInfos_.clear();
    MyFlag::tokenInfos_[tokenId] = TokenInfo(tokenId, Security::AccessToken::ATokenTypeEnum::TOKEN_HAP);
    MyFlag::retHapSuccValue_ = Security::AccessToken::AccessTokenKitRet::RET_FAILED;
    auto result = UPMSUtils::GenerateFUDAppInfo(info);
    EXPECT_FALSE(result);
}

/*
 * Feature: UPMSUtils
 * Function: GenerateFUDAppInfo
 * SubFunction: NA
 * FunctionPoints: UPMSUtils GenerateFUDAppInfo
 */
HWTEST_F(FileUriDistributionUtilsNewTest, GenerateFUDAppInfo_003, TestSize.Level1)
{
    uint32_t tokenId = 1001;
    FUDAppInfo info = { .tokenId = tokenId };
    std::string bundleName = "testApp";
    std::string processName = "testApp";
    MyFlag::tokenInfos_.clear();
    MyFlag::tokenInfos_[tokenId] = TokenInfo(tokenId, Security::AccessToken::ATokenTypeEnum::TOKEN_HAP,
        processName, bundleName);
    MyFlag::retHapSuccValue_ = Security::AccessToken::AccessTokenKitRet::RET_SUCCESS;
    auto result = UPMSUtils::GenerateFUDAppInfo(info);
    EXPECT_TRUE(result);
    EXPECT_EQ(info.bundleName, bundleName);
}

/*
 * Feature: UPMSUtils
 * Function: ConnectManager
 * SubFunction: NA
 * FunctionPoints: UPMSUtils GetAlterableBundleNameByTokenId
 */
HWTEST_F(FileUriDistributionUtilsNewTest, GetAlterableBundleNameByTokenId_001, TestSize.Level1)
{
    uint32_t tokenId = 1;
    std::string bundleName = "";
    MyFlag::tokenInfos_.clear();
    MyFlag::tokenInfos_[tokenId] = TokenInfo(tokenId, Security::AccessToken::ATokenTypeEnum::TOKEN_HAP);
    MyFlag::retHapSuccValue_ = Security::AccessToken::AccessTokenKitRet::RET_FAILED;
    auto result = UPMSUtils::GetAlterableBundleNameByTokenId(tokenId, bundleName);
    EXPECT_FALSE(result);
}

/*
 * Feature: UPMSUtils
 * Function: ConnectManager
 * SubFunction: NA
 * FunctionPoints: UPMSUtils GetAppIdByBundleName
 */
HWTEST_F(FileUriDistributionUtilsNewTest, GetAppIdByBundleName_001, TestSize.Level1)
{
    std::string bundleName = "";
    std::string appId = "";
    auto result = UPMSUtils::GetAppIdByBundleName(bundleName, appId);
    EXPECT_EQ(result, INNER_ERR);
}

/*
 * Feature: UPMSUtils
 * Function: ConnectManager
 * SubFunction: NA
 * FunctionPoints: UPMSUtils GetTokenIdByBundleName
 */
HWTEST_F(FileUriDistributionUtilsNewTest, GetTokenIdByBundleName_001, TestSize.Level1)
{
    std::string bundleName = "";
    int32_t appIndex = 1001;
    uint32_t tokenId = 1;
    auto result = UPMSUtils::GetTokenIdByBundleName(bundleName, appIndex, tokenId);
    EXPECT_EQ(result, ERR_GET_TARGET_BUNDLE_INFO_FAILED);
}

/*
 * Feature: UPMSUtils
 * Function: IsDocsCloudUri
 * SubFunction: NA
 * FunctionPoints: UPMSUtils IsDocsCloudUri
 */
HWTEST_F(FileUriDistributionUtilsNewTest, IsDocsCloudUri_001, TestSize.Level1)
{
    Uri uri("ccc");
    EXPECT_FALSE(UPMSUtils::IsDocsCloudUri(uri));
}

/*
 * Feature: UPMSUtils
 * Function: CheckUriTypeIsValid
 * SubFunction: NA
 * FunctionPoints: UPMSUtils CheckUriTypeIsValid
 */
HWTEST_F(FileUriDistributionUtilsNewTest, CheckUriTypeIsValid_001, TestSize.Level1)
{
    Uri uri("ccc");
    EXPECT_FALSE(UPMSUtils::CheckUriTypeIsValid(uri));
}

/*
 * Feature: UPMSUtils
 * Function: CheckUriTypeIsValid
 * SubFunction: NA
 * FunctionPoints: UPMSUtils CheckUriTypeIsValid
 */
HWTEST_F(FileUriDistributionUtilsNewTest, CheckUriTypeIsValid_002, TestSize.Level1)
{
    Uri uri("file:///path/to/file");
    EXPECT_TRUE(UPMSUtils::CheckUriTypeIsValid(uri));
}

/*
 * Feature: UPMSUtils
 * Function: GetBundleApiTargetVersion
 * SubFunction: NA
 * FunctionPoints: UPMSUtils GetBundleApiTargetVersion
 */
HWTEST_F(FileUriDistributionUtilsNewTest, GetBundleApiTargetVersion_001, TestSize.Level1)
{
    std::string bundleName = "ohos.bundle.test";
    int32_t targetApiVersion = 0;
    EXPECT_FALSE(UPMSUtils::GetBundleApiTargetVersion(bundleName, targetApiVersion));
}

/*
 * Feature: UPMSUtils
 * Function: GetDirByBundleNameAndAppIndex
 * SubFunction: NA
 * FunctionPoints: UPMSUtils GetDirByBundleNameAndAppIndex
 */
HWTEST_F(FileUriDistributionUtilsNewTest, GetDirByBundleNameAndAppIndex_001, TestSize.Level1)
{
    std::string bundleName = "ohos.bundle.test";
    std::string dirName = "/Data";
    EXPECT_TRUE(UPMSUtils::GetDirByBundleNameAndAppIndex(bundleName, 0, dirName));
}
}  // namespace AAFwk
}  // namespace OHOS