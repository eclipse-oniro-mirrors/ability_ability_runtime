/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include <chrono>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thread>

#define private public
#include "uri_permission_manager_client.h"
#include "uri_permission_load_callback.h"
#include "ability_manager_errors.h"
#include "mock_sa_call.h"
#include "mock_system_ability_manager.h"
#include "iservice_registry.h"
#include "want.h"
using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace AAFwk {
namespace {
const int MAX_URI_COUNT = 200000;
}

class UriPermissionManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UriPermissionManagerTest::SetUpTestCase() {}

void UriPermissionManagerTest::TearDownTestCase() {}

void UriPermissionManagerTest::SetUp()
{
    AAFwk::UriPermissionManagerClient::GetInstance().isUriPermServiceStarted_.store(false);
}

void UriPermissionManagerTest::TearDown() {}

/*
 * Feature: UriPermissionManagerClient
 * Function: ConnectUriPermService
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerClient ConnectUriPermService
 */
HWTEST_F(UriPermissionManagerTest, ConnectUriPermService_001, TestSize.Level1)
{
    sptr<AppExecFwk::MockSystemAbilityManager> mockSystemAbilityManager =
        new (std::nothrow) AppExecFwk::MockSystemAbilityManager();
    ASSERT_NE(mockSystemAbilityManager, nullptr);
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = mockSystemAbilityManager;
    EXPECT_CALL(*mockSystemAbilityManager,
        LoadSystemAbility(::testing::An<int32_t>(), ::testing::An<const sptr<ISystemAbilityLoadCallback>&>()))
        .WillOnce(::testing::Return(1));
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    auto ret = upmc.ConnectUriPermService();
    EXPECT_EQ(ret, nullptr);
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = nullptr;
}

/*
 * Feature: UriPermissionManagerClient
 * Function: ConnectUriPermService
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerClient ConnectUriPermService
 */
HWTEST_F(UriPermissionManagerTest, ConnectUriPermService_002, TestSize.Level1)
{
    sptr<AppExecFwk::MockSystemAbilityManager> mockSystemAbilityManager =
        new (std::nothrow) AppExecFwk::MockSystemAbilityManager();
    ASSERT_NE(mockSystemAbilityManager, nullptr);
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = mockSystemAbilityManager;
    EXPECT_CALL(*mockSystemAbilityManager,
        LoadSystemAbility(::testing::An<int32_t>(), ::testing::An<const sptr<ISystemAbilityLoadCallback>&>()))
        .WillOnce(::testing::Return(0));
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    upmc.saLoadFinished_ = true;
    auto ret = upmc.ConnectUriPermService();
    EXPECT_EQ(ret, nullptr);
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = nullptr;
}

/*
 * Feature: UriPermissionManagerClient
 * Function: LoadUriPermService
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerClient LoadUriPermService
 */
HWTEST_F(UriPermissionManagerTest, LoadUriPermService_001, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    upmc.saLoadFinished_ = true;
    auto ret = upmc.LoadUriPermService();
    EXPECT_TRUE(ret);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: GrantUriPermission
 * SubFunction: SingleGrantUriPermission
 * FunctionPoints: NA.
 * CaseDescription: Verify UriPermissionManagerClient GrantUriPermission
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_GrantUriPermission_001, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    auto uri = Uri("file://com.example.test1001/data/storage/el2/base/haps/entry/files/test_A.txt");
    std::string bundleName = "com.example.test1001";
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    auto ret = upmc.GrantUriPermission(uri, flag, bundleName, 0, 0);
    EXPECT_NE(ret, ERR_OK);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: GrantUriPermission
 * SubFunction: SingleGrantUriPermission
 * FunctionPoints: NA.
 * CaseDescription: Verify UriPermissionManagerClient GrantUriPermission
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_GrantUriPermission_002, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    auto uri = Uri("invalidScheme://temp.txt");
    std::string bundleName = "com.example.test1001";
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    auto ret = upmc.GrantUriPermission(uri, flag, bundleName, 0, 0);
    EXPECT_NE(ret, ERR_OK);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: GrantUriPermission
 * SubFunction: BatchGrantUriPermission
 * FunctionPoints: Size of uris is 0
 * CaseDescription: Verify UriPermissionManagerClient GrantUriPermission
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_BatchGrantUriPermission_001, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::vector<Uri> uriVec;
    std::string bundleName = "com.example.test1001";
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    auto ret = upmc.GrantUriPermission(uriVec, flag, bundleName, 0, 0);
    EXPECT_EQ(ret, ERR_URI_LIST_OUT_OF_RANGE);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: GrantUriPermission
 * SubFunction: BatchGrantUriPermission
 * FunctionPoints: Size of uris is more than 500
 * CaseDescription: Verify UriPermissionManagerClient GrantUriPermission
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_BatchGrantUriPermission_002, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    Uri uri = Uri("file://com.example.test1001/data/storage/el2/base/haps/entry/files/test_A.txt");
    std::vector<Uri> uriVec(MAX_URI_COUNT + 1, uri);
    std::string bundleName = "com.example.test1001";
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    auto ret = upmc.GrantUriPermission(uriVec, flag, bundleName, 0, 0);
    EXPECT_EQ(ret, ERR_URI_LIST_OUT_OF_RANGE);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: GrantUriPermission
 * SubFunction: BatchGrantUriPermission
 * FunctionPoints: Size of uris is betweent 1 and 500
 * CaseDescription: Verify UriPermissionManagerClient GrantUriPermission
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_BatchGrantUriPermission_003, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    Uri uri = Uri("file://com.example.test1001/data/storage/el2/base/haps/entry/files/test_A.txt");
    std::vector<Uri> uriVec(500, uri);
    std::string bundleName = "com.example.test1001";
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    auto ret = upmc.GrantUriPermission(uriVec, flag, bundleName, 0, 0);
    EXPECT_NE(ret, ERR_OK);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: RevokeUriPermissionManually
 * SubFunction: RevokeUriPermissionManually
 * FunctionPoints: Uri is invalid.
 * CaseDescription: Verify UriPermissionManagerClient RevokeUriPermissionManually
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_RevokeUriPermissionManually_001, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    auto uri = Uri("http://com.example.test1001/data/storage/el2/base/haps/entry/files/test_A.txt");
    std::string bundleName = "com.example.test1001";
    auto ret = upmc.RevokeUriPermissionManually(uri, bundleName);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: RevokeUriPermissionManually
 * SubFunction: RevokeUriPermissionManually
 * FunctionPoints: Uri is valid.
 * CaseDescription: Verify UriPermissionManagerClient RevokeUriPermissionManually
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_RevokeUriPermissionManually_002, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    auto uri = Uri("file://com.example.test1001/data/storage/el2/base/haps/entry/files/test_A.txt");
    std::string bundleName = "com.example.test1001";
    auto ret = upmc.RevokeUriPermissionManually(uri, bundleName);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: RevokeUriPermissionManually
 * SubFunction: RevokeUriPermissionManually
 * FunctionPoints: Uri is valid.
 * CaseDescription: Verify UriPermissionManagerClient RevokeUriPermissionManually
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_RevokeUriPermissionManually_003, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    auto uri = Uri("file://com.example.test1001/data/storage/el2/base/haps/entry/files/test_A.txt");
    std::string bundleName = "com.example.test1001";
    auto ret = upmc.RevokeUriPermissionManually(uri, bundleName, 1001);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: RevokeAllUriPermissions
 * SubFunction: RevokeAllUriPermissions
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerClient RevokeAllUriPermissions
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_RevokeAllUriPermissions_001, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    uint32_t targetTokenId = 1001;
    auto ret = upmc.RevokeAllUriPermissions(targetTokenId);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: VerifyUriPermission
 * SubFunction: VerifyUriPermission
 * FunctionPoints: NA
 * CaseDescription: Verify UriPermissionManagerClient VerifyUriPermission
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_VerifyUriPermission_001, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    auto uriStr = "file://docs/storage/Users/currentUser/test.txt";
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string bundleName = "com.example.test";
    uint32_t targetTokenId = 100002;
    Uri uri(uriStr);
    bool res = upmc.VerifyUriPermission(uri, flag, targetTokenId);
    EXPECT_EQ(res, false);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: CheckUriAuthorization
 * SubFunction: CheckUriAuthorization
 * FunctionPoints: Size of uris is 0
 * CaseDescription: Verify UriPermissionManagerClient CheckUriAuthorization
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_CheckUriAuthorization_001, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::vector<std::string> uriVec;
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;
    auto res = upmc.CheckUriAuthorization(uriVec, flag, tokenId);
    std::vector<bool> expectRes;
    EXPECT_EQ(res, expectRes);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: CheckUriAuthorization
 * SubFunction: CheckUriAuthorization
 * FunctionPoints: Size of uris is between 1 and 500
 * CaseDescription: Verify UriPermissionManagerClient CheckUriAuthorization
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_CheckUriAuthorization_002, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::string uriStr = "file://docs/storage/Users/currentUser/test.txt";
    std::vector<std::string> uriVec(1, uriStr);
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;
    auto res = upmc.CheckUriAuthorization(uriVec, flag, tokenId);
    std::vector<bool> expectRes(1, false);
    EXPECT_EQ(res, expectRes);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: CheckUriAuthorization
 * SubFunction: CheckUriAuthorization
 * FunctionPoints: Size of uris is more than 500
 * CaseDescription: Verify UriPermissionManagerClient CheckUriAuthorization
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_CheckUriAuthorization_003, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::string uriStr = "file://docs/storage/Users/currentUser/test.txt";
    std::vector<std::string> uriVec(MAX_URI_COUNT + 1, uriStr);
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;
    auto res = upmc.CheckUriAuthorization(uriVec, flag, tokenId);
    std::vector<bool> expectRes(MAX_URI_COUNT + 1, false);
    EXPECT_EQ(res, expectRes);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: CheckUriAuthorization
 * SubFunction: CheckUriAuthorization
 * FunctionPoints: Write Uri and result by raw data
 * CaseDescription: Verify UriPermissionManagerClient CheckUriAuthorization
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_CheckUriAuthorization_004, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::string uriStr = "file://docs/storage/Users/currentUser/test.txt";
    std::vector<std::string> uriVec(50000, uriStr);
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;
    auto res = upmc.CheckUriAuthorization(uriVec, flag, tokenId);
    std::vector<bool> expectRes(50000, false);
    EXPECT_EQ(res, expectRes);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: CheckUriAuthorization
 * SubFunction: CheckUriAuthorization
 * FunctionPoints: Write Uri and result by raw data
 * CaseDescription: Verify UriPermissionManagerClient CheckUriAuthorization
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_CheckUriAuthorization_005, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::string uriStr = "file://docs/storage/Users/currentUser/test.txt";
    std::vector<std::string> uriVec(MAX_URI_COUNT, uriStr);
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;
    auto res = upmc.CheckUriAuthorization(uriVec, flag, tokenId);
    std::vector<bool> expectRes(MAX_URI_COUNT, false);
    EXPECT_EQ(res, expectRes);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: GrantUriPermissionPrivileged
 * SubFunction: GrantUriPermissionPrivileged
 * FunctionPoints: Size of uris is 0
 * CaseDescription: Verify UriPermissionManagerClient GrantUriPermissionPrivileged
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_GrantUriPermissionPrivileged_001, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::string uriStr = "file://docs/storage/Users/currentUser/test.txt";
    std::vector<Uri> uriVec;
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string bundleName = "com.example.test1001";
    int32_t appIndex = 0;
    auto res = upmc.GrantUriPermissionPrivileged(uriVec, flag, bundleName, appIndex, 0, 0);
    EXPECT_EQ(res, ERR_URI_LIST_OUT_OF_RANGE);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: GrantUriPermissionPrivileged
 * SubFunction: GrantUriPermissionPrivileged
 * FunctionPoints: Size of uris is more than 500
 * CaseDescription: Verify UriPermissionManagerClient GrantUriPermissionPrivileged
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_GrantUriPermissionPrivileged_002, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::string uriStr = "file://docs/storage/Users/currentUser/test.txt";
    std::vector<Uri> uriVec(MAX_URI_COUNT + 1, Uri(uriStr));
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string bundleName = "com.example.test1001";
    int32_t appIndex = 0;
    auto res = upmc.GrantUriPermissionPrivileged(uriVec, flag, bundleName, appIndex, 0, 0);
    EXPECT_EQ(res, ERR_URI_LIST_OUT_OF_RANGE);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: GrantUriPermissionPrivileged
 * SubFunction: GrantUriPermissionPrivileged
 * FunctionPoints: size of uri is between 1 and 500
 * CaseDescription: Verify UriPermissionManagerClient GrantUriPermissionPrivileged
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_GrantUriPermissionPrivileged_003, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::string uriStr = "file://docs/storage/Users/currentUser/test.txt";
    std::vector<Uri> uriVec(1, Uri(uriStr));
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string bundleName = "com.example.test1001";
    int32_t appIndex = 0;
    auto res = upmc.GrantUriPermissionPrivileged(uriVec, flag, bundleName, appIndex, 0, 0);
    EXPECT_EQ(res, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: GrantUriPermissionPrivileged
 * SubFunction: GrantUriPermissionPrivileged
 * FunctionPoints: Write Uri by raw data
 * CaseDescription: Verify UriPermissionManagerClient GrantUriPermissionPrivileged
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_GrantUriPermissionPrivileged_004, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::string uriStr = "file://docs/storage/Users/currentUser/test.txt";
    std::vector<Uri> uriVec(50000, Uri(uriStr));
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string bundleName = "com.example.test1001";
    int32_t appIndex = 0;
    auto res = upmc.GrantUriPermissionPrivileged(uriVec, flag, bundleName, appIndex, 0, 0);
    EXPECT_EQ(res, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: GrantUriPermissionPrivileged
 * SubFunction: GrantUriPermissionPrivileged
 * FunctionPoints: Write Uri by raw data
 * CaseDescription: Verify UriPermissionManagerClient GrantUriPermissionPrivileged
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_GrantUriPermissionPrivileged_005, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::string uriStr = "file://docs/storage/Users/currentUser/test.txt";
    std::vector<Uri> uriVec(MAX_URI_COUNT, Uri(uriStr));
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string bundleName = "com.example.test1001";
    int32_t appIndex = 0;
    auto res = upmc.GrantUriPermissionPrivileged(uriVec, flag, bundleName, appIndex, 0, 0);
    EXPECT_EQ(res, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: GrantUriPermissionPrivileged
 * SubFunction: GrantUriPermissionPrivileged
 * FunctionPoints: Write Uri by raw data, more than 128M
 * CaseDescription: Verify UriPermissionManagerClient GrantUriPermissionPrivileged
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_GrantUriPermissionPrivileged_006, TestSize.Level1)
{
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::string uriStr = "file://docs/storage/Users/currentUser/";
    for (int32_t i = 0; i < 100; i++) {
        uriStr += "aaaaaaaaaaaa/";
    }
    uriStr += "1.txt";
    std::vector<Uri> uriVec(MAX_URI_COUNT, Uri(uriStr));
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string bundleName = "com.example.test1001";
    int32_t appIndex = 0;
    auto res = upmc.GrantUriPermissionPrivileged(uriVec, flag, bundleName, appIndex, 0, 0);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: GrantUriPermissionByKey
 * SubFunction: NA
 * FunctionPoints: Grant permission failed
 * CaseDescription: Verify UriPermissionManagerClient GrantUriPermissionByKey
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_GrantUriPermissionByKey_001, TestSize.Level1)
{
    auto &upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::string key = "testKey";
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t targetTokenId = 100002;
    auto ret = upmc.GrantUriPermissionByKey(key, flag, targetTokenId);
    EXPECT_NE(ret, ERR_OK);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: GrantUriPermissionByKeyAsCaller
 * SubFunction: NA
 * FunctionPoints: Grant permission failed
 * CaseDescription: Verify UriPermissionManagerClient GrantUriPermissionByKeyAsCaller
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_GrantUriPermissionByKeyAsCaller_001, TestSize.Level1)
{
    auto &upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::string key = "testKey";
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t callerTokenId = 100001;
    uint32_t targetTokenId = 100002;
    auto ret = upmc.GrantUriPermissionByKeyAsCaller(key, flag, callerTokenId, targetTokenId);
    EXPECT_NE(ret, ERR_OK);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: ClearPermissionTokenByMap
 * SubFunction: ClearPermissionTokenByMap
 * FunctionPoints: Do clear uri permission, when upms is started and not started.
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_ClearPermissionTokenByMap_001, TestSize.Level1)
{
    auto &upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    uint32_t tokenId = 1001;
    auto res = upmc.ClearPermissionTokenByMap(tokenId);
    EXPECT_EQ(res, ERR_UPMS_SERVICE_NOT_START);

    upmc.isUriPermServiceStarted_.store(true);
    res = upmc.ClearPermissionTokenByMap(tokenId);
    EXPECT_NE(res, ERR_UPMS_SERVICE_NOT_START);
}

#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
/*
 * Feature: UriPermissionManagerClient
 * Function: Active
 * SubFunction: Active
 * FunctionPoints: Params is invalid.
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_Active_001, TestSize.Level1)
{
    auto &upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::vector<AccessControl::SandboxManager::PolicyInfo> policies;
    std::vector<uint32_t> result;
    auto res = upmc.Active(policies, result);
    EXPECT_EQ(res, ERR_URI_LIST_OUT_OF_RANGE);

    AccessControl::SandboxManager::PolicyInfo policy;
    policies = std::vector<AccessControl::SandboxManager::PolicyInfo>(MAX_URI_COUNT + 1, policy);
    res = upmc.Active(policies, result);
    EXPECT_EQ(res, ERR_URI_LIST_OUT_OF_RANGE);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: Active
 * SubFunction: Active
 * FunctionPoints: Data of policies is too large.
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_Active_002, TestSize.Level1)
{
    auto &upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::string path = std::string(1000, 'a');
    AccessControl::SandboxManager::PolicyInfo policy = { .path = path };
    std::vector<AccessControl::SandboxManager::PolicyInfo> policies(MAX_URI_COUNT, policy);
    std::vector<uint32_t> result;
    auto res = upmc.Active(policies, result);
    EXPECT_EQ(res, INNER_ERR);
}

/*
 * Feature: UriPermissionManagerClient
 * Function: Active
 * SubFunction: Active
 * FunctionPoints: Failed to call Active.
 */
HWTEST_F(UriPermissionManagerTest, UriPermissionManager_Active_003, TestSize.Level1)
{
    auto &upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    AccessControl::SandboxManager::PolicyInfo policy = { .path = "/data/path/test.txt" };
    std::vector<AccessControl::SandboxManager::PolicyInfo> policies(1, policy);
    std::vector<uint32_t> result;
    auto res = upmc.Active(policies, result);
    EXPECT_NE(res, ERR_OK);
}
#endif // ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
}  // namespace AAFwk
}  // namespace OHOS