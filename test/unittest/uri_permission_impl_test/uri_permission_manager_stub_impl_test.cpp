/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "ability_manager_client.h"
#include "mock_my_flag.h"
#include "batch_uri.h"

#define private public
#define protected public
#include "uri_permission_manager_client.h"
#include "uri_permission_manager_stub_impl.h"
#include "ability_manager_errors.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int OFFSET = 30;
const std::string POLICY_INFO_PATH = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt";
}
class UriPermissionManagerStubImplTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UriPermissionManagerStubImplTest::SetUpTestCase() {}

void UriPermissionManagerStubImplTest::TearDownTestCase() {}

void UriPermissionManagerStubImplTest::SetUp()
{
    MyFlag::Init();
    IAbilityManagerCollaborator::verifyResult = 0;
}

void UriPermissionManagerStubImplTest::TearDown() {}

/*
 * Feature: UriPermissionManagerService
 * Function: VerifyUriPermission
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService VerifyUriPermission
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_VerifyUriPermission_001, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    MyFlag::isDFSCallRet_ = false;
    std::string uri = "uri";
    uint32_t flag = 1;
    uint32_t tokenId = 1;
    bool funcResult = true;
    auto result = upmsi->VerifyUriPermission(Uri(uri), flag, tokenId, funcResult);
    EXPECT_FALSE(funcResult);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: UriPermissionManagerService
 * Function: VerifyUriPermission
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService VerifyUriPermission
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_VerifyUriPermission_002, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    MyFlag::isDFSCallRet_ = true;
    std::string uri = "uri";
    uint32_t flag = 0;
    uint32_t tokenId = 1;
    bool funcResult = true;
    auto result = upmsi->VerifyUriPermission(Uri(uri), flag, tokenId, funcResult);
    EXPECT_FALSE(funcResult);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: UriPermissionManagerService
 * Function: VerifyUriPermission
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService VerifyUriPermission
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_VerifyUriPermission_003, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    MyFlag::isDFSCallRet_ = true;
    MyFlag::isUriTypeValid_ = false;
    std::string uri = "uri";
    uint32_t flag = 1;
    uint32_t tokenId = 1;
    bool funcResult = true;
    auto result = upmsi->VerifyUriPermission(Uri(uri), flag, tokenId, funcResult);
    EXPECT_FALSE(funcResult);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: UriPermissionManagerService
 * Function: VerifySubDirUriPermission
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService VerifySubDirUriPermission
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_VerifySubDirUriPermission_001, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    upmsi->uriMap_.clear();
    std::string uriStr = "?networkid=";
    uint32_t newFlag = 1;
    uint32_t tokenId = 1;
    auto result = upmsi->VerifySubDirUriPermission(uriStr, newFlag, tokenId);
    EXPECT_FALSE(result);
}

/*
 * Feature: UriPermissionManagerService
 * Function: IsDistributedSubDirUri
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService IsDistributedSubDirUri
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_IsDistributedSubDirUri_001, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    std::string inputUri = "";
    std::string cachedUri = "";
    auto result = upmsi->IsDistributedSubDirUri(inputUri, cachedUri);
    EXPECT_FALSE(result);
}

/*
 * Feature: UriPermissionManagerService
 * Function: IsDistributedSubDirUri
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService IsDistributedSubDirUri
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_IsDistributedSubDirUri_002, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    std::string inputUri = "file://home/user/shared_folder?networkid=12345";
    std::string cachedUri = "file://home/user?networkid=12345";
    auto result = upmsi->IsDistributedSubDirUri(inputUri, cachedUri);
    EXPECT_TRUE(result);
}

/*
 * Feature: UriPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_GrantUriPermission_001, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    MyFlag::isSystemAppCall_ = true;
    Uri uri("uri");
    uint32_t flag = 1;
    std::string targetBundleName = "targetBundleName";
    int32_t appIndex = 1;
    uint32_t initiatorTokenId = 1;
    int32_t funcResult;
    auto result = upmsi->GrantUriPermission(uri, flag, targetBundleName, appIndex, initiatorTokenId, funcResult);
    EXPECT_EQ(funcResult, ERR_CODE_INVALID_URI_TYPE);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: UriPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_GrantUriPermission_002, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    MyFlag::isSystemAppCall_ = false;
    Uri uri("file");
    uint32_t flag = 1;
    std::string targetBundleName = "targetBundleName";
    int32_t appIndex = 1;
    uint32_t initiatorTokenId = 1;
    int32_t funcResult = 0;
    auto result = upmsi->GrantUriPermission(uri, flag, targetBundleName, appIndex, initiatorTokenId, funcResult);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: UriPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_GrantUriPermission_003, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    std::vector<std::string> uriVec;
    uint32_t flag = 1;
    std::string targetBundleName = "targetBundleName";
    int32_t appIndex = 1;
    uint32_t initiatorTokenId = 1;
    int32_t funcResult = 1;
    auto result = upmsi->GrantUriPermission(uriVec, flag, targetBundleName, appIndex, initiatorTokenId, funcResult);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: UriPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_GrantUriPermission_004, TestSize.Level1)
{
#define ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    std::vector<std::string> mediaUris;
    mediaUris.push_back("media://test");
    uint32_t flag = 1;
    uint32_t callerTokenId = 1;
    uint32_t targetTokenId = 1;
    int32_t hideSensitiveType = 1;
    auto result = upmsi->GrantBatchMediaUriPermissionImpl(
        mediaUris, flag, callerTokenId, targetTokenId, hideSensitiveType);
    EXPECT_EQ(result, -1);
#undef ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE
}

/*
 * Feature: UriPermissionManagerService
 * Function: GrantBatchUriPermissionImpl
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService GrantBatchUriPermissionImpl
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_GrantBatchUriPermissionImpl_001, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    std::vector<std::string> uriVec;
    uriVec.push_back("file://test");
    uint32_t flag = 1;
    TokenId callerTokenId = 1;
    TokenId targetTokenId = 1;
    auto result = upmsi->GrantBatchUriPermissionImpl(uriVec, flag, callerTokenId, targetTokenId);
    EXPECT_EQ(result, INNER_ERR);
}

/*
 * Feature: UriPermissionManagerService
 * Function: AddTempUriPermission
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService AddTempUriPermission
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_AddTempUriPermission_001, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    std::string uri = "file://test";
    uint32_t flag = 1;
    TokenId fromTokenId = 1;
    TokenId targetTokenId = 1;
    GrantInfo info = { flag, fromTokenId, targetTokenId };
    upmsi->uriMap_.insert({ uri, { info } });
    auto result = upmsi->AddTempUriPermission(uri, flag, fromTokenId, targetTokenId);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: UriPermissionManagerService
 * Function: AddTempUriPermission
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService AddTempUriPermission
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_AddTempUriPermission_002, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    std::string uri = "file://test";
    uint32_t flag = 1;
    TokenId fromTokenId = 1;
    TokenId targetTokenId = 1;
    GrantInfo info = { 0, 0, 0 };
    upmsi->uriMap_.insert({ uri, { info } });
    auto result = upmsi->AddTempUriPermission(uri, flag, fromTokenId, targetTokenId);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: UriPermissionManagerService
 * Function: GrantUriPermissionPrivileged
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService GrantUriPermissionPrivileged
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_GrantUriPermissionPrivileged_001, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    std::vector<std::string> uriVec;
    uint32_t flag = 1;
    const std::string targetBundleName = "targetBundleName";
    int32_t appIndex = 1;
    uint32_t initiatorTokenId = 1;
    int32_t hideSensitiveType = 1;
    int32_t funcResult = ERR_CODE_INVALID_URI_TYPE;
    auto result = upmsi->GrantUriPermissionPrivileged(uriVec, flag, targetBundleName, appIndex, initiatorTokenId,
        hideSensitiveType, funcResult);
    EXPECT_EQ(funcResult, ERR_URI_LIST_OUT_OF_RANGE);
    EXPECT_EQ(result, ERR_URI_LIST_OUT_OF_RANGE);
}

/*
 * Feature: UriPermissionManagerService
 * Function: GrantUriPermissionPrivileged
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService GrantUriPermissionPrivileged
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_GrantUriPermissionPrivileged_002, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    UriPermissionRawData rawData;
    rawData.data = nullptr;
    uint32_t flag = 1;
    const std::string targetBundleName = "";
    int32_t appIndex = 1;
    uint32_t initiatorTokenId = 1;
    int32_t hideSensitiveType = 1;
    int32_t funcResult = ERR_CODE_INVALID_URI_TYPE;
    auto result = upmsi->GrantUriPermissionPrivileged(rawData, flag, targetBundleName, appIndex, initiatorTokenId,
        hideSensitiveType, funcResult);
    EXPECT_EQ(funcResult, ERR_DEAD_OBJECT);
    EXPECT_EQ(result, ERR_DEAD_OBJECT);
}

/*
 * Feature: UriPermissionManagerService
 * Function: GrantUriPermissionPrivilegedInner
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService GrantUriPermissionPrivilegedInner
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_GrantUriPermissionPrivilegedInner_001, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    Uri uri("content");
    std::vector<Uri> uriVec;
    uriVec.push_back(uri);
    uint32_t flag = 1;
    uint32_t callerTokenId = 1;
    uint32_t targetTokenId = 1;
    std::string targetAlterBundleName = "targetAlterBundleName";
    int32_t hideSensitiveType = 1;
    MyFlag::isUriTypeValid_ = true;
    MyFlag::isDocsCloudUri_ = true;
    std::string bundleName = "bundleName";
    FUDAppInfo callerInfo = { callerTokenId, "caller", "callerAlterName" };
    FUDAppInfo targetAppInfo = { targetTokenId, bundleName, targetAlterBundleName };
    std::vector<int32_t> permissionTypes(uriVec.size(), 0);
    auto result = upmsi->GrantUriPermissionPrivilegedInner(uriVec, flag, callerInfo, targetAppInfo,
        hideSensitiveType, permissionTypes);
    EXPECT_EQ(result, INNER_ERR);
}

/*
 * Feature: UriPermissionManagerService
 * Function: GrantUriPermissionPrivilegedInner
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService GrantUriPermissionPrivilegedInner
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_GrantUriPermissionPrivilegedInner_002, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    Uri uri("uri://targetAlterBundleName");
    std::vector<Uri> uriVec;
    uriVec.push_back(uri);
    uint32_t flag = 1;
    uint32_t callerTokenId = 1;
    uint32_t targetTokenId = 1;
    std::string targetAlterBundleName = "targetAlterBundleName";
    int32_t hideSensitiveType = 1;
    MyFlag::isUriTypeValid_ = true;
    MyFlag::isDocsCloudUri_ = false;
    std::string bundleName = "bundleName";
    FUDAppInfo callerInfo = { callerTokenId, "caller", "callerAlterName" };
    FUDAppInfo targetAppInfo = { targetTokenId, bundleName, targetAlterBundleName };
    std::vector<int32_t> permissionTypes(uriVec.size(), 0);
    auto result = upmsi->GrantUriPermissionPrivilegedInner(uriVec, flag, callerInfo, targetAppInfo,
        hideSensitiveType, permissionTypes);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: UriPermissionManagerService
 * Function: GrantUriPermissionPrivilegedInner
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService GrantUriPermissionPrivilegedInner
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_GrantUriPermissionPrivilegedInner_003, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    Uri uri("uri://media");
    std::vector<Uri> uriVec;
    uriVec.push_back(uri);
    uint32_t flag = 1;
    uint32_t callerTokenId = 1;
    uint32_t targetTokenId = 1;
    std::string targetAlterBundleName = "targetAlterBundleName";
    int32_t hideSensitiveType = 1;
    MyFlag::isUriTypeValid_ = true;
    MyFlag::isDocsCloudUri_ = false;
    std::string bundleName = "bundleName";
    FUDAppInfo callerInfo = { callerTokenId, "caller", "callerAlterName" };
    FUDAppInfo targetAppInfo = { targetTokenId, bundleName, targetAlterBundleName };
    std::vector<int32_t> permissionTypes(uriVec.size(), 0);
    auto result = upmsi->GrantUriPermissionPrivilegedInner(uriVec, flag, callerInfo, targetAppInfo,
        hideSensitiveType, permissionTypes);
    EXPECT_EQ(result, INNER_ERR);
}

/*
 * Feature: UriPermissionManagerService
 * Function: CheckUriAuthorization
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService CheckUriAuthorization
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_CheckUriAuthorization_001, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    std::vector<std::string> uriStrVec;
    uint32_t flag = 1;
    uint32_t tokenId = 1;
    std::vector<bool> funcResult;
    auto result = upmsi->CheckUriAuthorization(uriStrVec, flag, tokenId, funcResult);
    EXPECT_EQ(result, ERR_URI_LIST_OUT_OF_RANGE);
}

/*
 * Feature: UriPermissionManagerService
 * Function: CheckUriAuthorization
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService CheckUriAuthorization
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_CheckUriAuthorization_002, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    UriPermissionRawData rawData;
    rawData.data = nullptr;
    uint32_t flag = 1;
    uint32_t tokenId = 1;
    UriPermissionRawData funcResult;
    auto result = upmsi->CheckUriAuthorization(rawData, flag, tokenId, funcResult);
    EXPECT_EQ(result, ERR_DEAD_OBJECT);
}

/*
 * Feature: UriPermissionManagerService
 * Function: RevokeAllMapUriPermissions
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService RevokeAllMapUriPermissions
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_RevokeAllMapUriPermissions_001, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    MyFlag::bundleName_ = "callerAuthority";
    GrantInfo info = { 1, 1, 1 };
    upmsi->uriMap_.insert({ "uri://callerAuthority", { info } });
    uint32_t tokenId = 1;
    auto result = upmsi->RevokeAllMapUriPermissions(tokenId);
    EXPECT_EQ(result, 0);
}

/*
 * Feature: UriPermissionManagerService
 * Function: RevokeAllMapUriPermissions
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService RevokeAllMapUriPermissions
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_RevokeAllMapUriPermissions_002, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    MyFlag::bundleName_ = "bundleName";
    GrantInfo info = { 1, 1, 1 };
    upmsi->uriMap_.insert({ "uri://callerAuthority", { info } });
    uint32_t tokenId = 1;
    auto result = upmsi->RevokeAllMapUriPermissions(tokenId);
    EXPECT_EQ(upmsi->uriMap_.size(), 0);
    EXPECT_EQ(result, 0);
}

/*
 * Feature: UriPermissionManagerService
 * Function: RevokeUriPermissionManually
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService RevokeUriPermissionManually
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_RevokeUriPermissionManually_001, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    MyFlag::isSystemAppCall_ = true;
    MyFlag::isUriTypeValid_ = true;
    MyFlag::getTokenIdByBundleNameStatus_ = -1;
    Uri uri("uri");
    std::string bundleName = "bundleName";
    int32_t appIndex = 1;
    int32_t funcResult = 1;
    auto result = upmsi->RevokeUriPermissionManually(uri, bundleName, appIndex, funcResult);
    EXPECT_EQ(funcResult, -1);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: UriPermissionManagerService
 * Function: RevokeUriPermissionManuallyInner
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService RevokeUriPermissionManuallyInner
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_RevokeUriPermissionManuallyInner_001, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    upmsi->uriMap_.clear();
    Uri uri("uri://content");
    uint32_t targetTokenId = 1;
    MyFlag::isDocsCloudUri_ = true;
    auto result = upmsi->RevokeUriPermissionManuallyInner(uri, targetTokenId);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: UriPermissionManagerService
 * Function: RevokeUriPermissionManuallyInner
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService RevokeUriPermissionManuallyInner
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_RevokeUriPermissionManuallyInner_002, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    upmsi->uriMap_.clear();
    Uri uri("uri://media");
    uint32_t targetTokenId = 1;
    MyFlag::isDocsCloudUri_ = false;
    auto result = upmsi->RevokeUriPermissionManuallyInner(uri, targetTokenId);
    EXPECT_EQ(result, -1);
}

/*
 * Feature: UriPermissionManagerService
 * Function: RevokeMapUriPermissionManually
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService RevokeMapUriPermissionManually
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_RevokeMapUriPermissionManually_001, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    uint32_t callerTokenId = 1;
    uint32_t targetTokenId = 1;
    Uri uri("uri");
    upmsi->uriMap_.clear();
    auto result = upmsi->RevokeMapUriPermissionManually(callerTokenId, targetTokenId, uri);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: UriPermissionManagerService
 * Function: RevokeMapUriPermissionManually
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService RevokeMapUriPermissionManually
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_RevokeMapUriPermissionManually_002, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    uint32_t callerTokenId = 1;
    uint32_t targetTokenId = 1;
    Uri uri("uri");
    GrantInfo info = { 1, 1, 1 };
    upmsi->uriMap_.clear();
    upmsi->uriMap_.insert({ uri.ToString(), { info } });
    auto result = upmsi->RevokeMapUriPermissionManually(callerTokenId, targetTokenId, uri);
    EXPECT_EQ(upmsi->uriMap_.size(), 0);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: UriPermissionManagerService
 * Function: ClearPermissionTokenByMap
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService ClearPermissionTokenByMap
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_ClearPermissionTokenByMap_001, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    uint32_t tokenId = 0;
    int32_t funcResult = 0;
    auto result = upmsi->ClearPermissionTokenByMap(tokenId, funcResult);
    EXPECT_EQ(funcResult, ERR_PERMISSION_DENIED);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: UriPermissionManagerService
 * Function: BoolVecToCharVec
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService BoolVecToCharVec
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_BoolVecToCharVec_001, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    std::vector<bool> boolVector;
    std::vector<char> charVector;
    upmsi->BoolVecToCharVec(boolVector, charVector);
    EXPECT_EQ(charVector.size(), 0);
}

/*
 * Feature: UriPermissionManagerService
 * Function: RawDataToStringVec
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService RawDataToStringVec
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_RawDataToStringVec_001, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    UriPermissionRawData rawData;
    rawData.data = nullptr;
    std::vector<std::string> stringVec;
    auto result = upmsi->RawDataToStringVec(rawData, stringVec);
    EXPECT_EQ(result, ERR_DEAD_OBJECT);
}

/*
 * Feature: UriPermissionManagerService
 * Function: RawDataToStringVec
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService RawDataToStringVec
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_RawDataToStringVec_002, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    UriPermissionRawData rawData;
    rawData.data = upmsi.get();
    rawData.size = 0;
    std::vector<std::string> stringVec;
    auto result = upmsi->RawDataToStringVec(rawData, stringVec);
    EXPECT_EQ(result, ERR_DEAD_OBJECT);
}

/*
 * Feature: UriPermissionManagerService
 * Function: RawDataToStringVec
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService RawDataToStringVec
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_RawDataToStringVec_003, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    auto &upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::vector<std::string> strArray;
    strArray.emplace_back(POLICY_INFO_PATH);
    UriPermissionRawData rawData;
    upmc.StringVecToRawData(strArray, rawData);
    rawData.size -= OFFSET;
    UriPermissionRawData stubRawData;
    stubRawData.size = rawData.size;
    EXPECT_EQ(stubRawData.RawDataCpy(rawData.data), ERR_NONE);
    std::vector<std::string> stringVec;
    auto result = upmsi->RawDataToStringVec(stubRawData, stringVec);
    EXPECT_EQ(result, ERR_DEAD_OBJECT);
}

/*
 * Feature: UriPermissionManagerService
 * Function: RawDataToPolicyInfo
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService RawDataToPolicyInfo
 */
HWTEST_F(UriPermissionManagerStubImplTest, Upmsi_RawDataToPolicyInfo_001, TestSize.Level1)
{
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    auto &upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    PolicyInfo policyInfo;
    policyInfo.path = POLICY_INFO_PATH;
    policyInfo.mode = 1;
    std::vector<PolicyInfo> policyInfoArray;
    policyInfoArray.push_back(policyInfo);
    UriPermissionRawData policyRawData;
    upmc.PolicyInfoToRawData(policyInfoArray, policyRawData);
    policyRawData.size -= OFFSET;
    UriPermissionRawData stubPolicyRawData;
    stubPolicyRawData.size = policyRawData.size;
    EXPECT_EQ(stubPolicyRawData.RawDataCpy(policyRawData.data), ERR_NONE);
    std::vector<PolicyInfo> policy;
    auto result = upmsi->RawDataToPolicyInfo(stubPolicyRawData, policy);
    EXPECT_EQ(result, INVALID_PARAMETERS_ERR);
#endif
}

/*
 * Feature: UriPermissionManagerService
 * Function: VerifyContentUriPermission
 * SubFunction: NA
 * FunctionPoints: VerifyContentUriPermission with empty contentUris
 */
HWTEST_F(UriPermissionManagerStubImplTest, VerifyContentUriPermission_001, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    std::vector<std::string> contentUris; // Empty
    uint32_t flag = 1;
    uint32_t tokenId = 1;
    std::vector<bool> checkResults;
    auto result = upmsi->VerifyContentUriPermission(contentUris, flag, tokenId, checkResults);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: UriPermissionManagerService
 * Function: CheckProxyUriPermission
 * SubFunction: NA
 * FunctionPoints: CheckProxyUriPermission without proxy authorization permission
 */
HWTEST_F(UriPermissionManagerStubImplTest, CheckProxyUriPermission_001, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    MyFlag::permissionProxyAuthorization_ = false;
    BatchUri batchUri;
    batchUri.Init({"file://test/file.txt"});
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    uint32_t callerTokenId = 1;
    uint32_t flag = 1;
    auto result = upmsi->CheckProxyUriPermission(batchUri, callerTokenId, flag);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerService
 * Function: CheckProxyUriPermission
 * SubFunction: NA
 * FunctionPoints: CheckProxyUriPermission with empty proxyUrisByPolicy and empty contentUris
 */
HWTEST_F(UriPermissionManagerStubImplTest, CheckProxyUriPermission_002, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    MyFlag::permissionProxyAuthorization_ = true;
    BatchUri batchUri;
    batchUri.Init({"file://caller/file.txt"}, 0, "caller", "", false);
    uint32_t callerTokenId = 1;
    uint32_t flag = 1;
    auto result = upmsi->CheckProxyUriPermission(batchUri, callerTokenId, flag);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: UriPermissionManagerService
 * Function: CheckProxyUriPermission
 * SubFunction: NA
 * FunctionPoints: CheckProxyUriPermission with proxyUrisByPolicy
 */
HWTEST_F(UriPermissionManagerStubImplTest, CheckProxyUriPermission_003, TestSize.Level1)
{
    auto upmsi = std::make_shared<UriPermissionManagerStubImpl>();
    MyFlag::permissionProxyAuthorization_ = true;
    BatchUri batchUri;
    batchUri.Init({"file://docs/file.txt"}, 0, "", "target", false);
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    uint32_t callerTokenId = 1;
    uint32_t flag = 1;
    auto result = upmsi->CheckProxyUriPermission(batchUri, callerTokenId, flag);
    EXPECT_EQ(result, ERR_OK);
}

}  // namespace AAFwk
}  // namespace OHOS