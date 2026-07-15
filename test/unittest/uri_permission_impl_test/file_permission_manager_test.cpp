/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "file_permission_manager.h"
#undef private
#include "mock_my_flag.h"
#include "mock_permission_verification.h"

using namespace testing::ext;
namespace OHOS {
namespace AAFwk {
class FilePermissionManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void FilePermissionManagerTest::SetUpTestCase()
{}

void FilePermissionManagerTest::TearDownTestCase()
{}

void FilePermissionManagerTest::SetUp()
{
    MyFlag::Init();
}

void FilePermissionManagerTest::TearDown()
{}
/*
 * Feature: CheckUriPersistentPermission
 * Function: CheckUriPersistentPermission
 * SubFunction: NA
 * FunctionPoints: CheckUriPersistentPermission
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_001, TestSize.Level1)
{
    std::vector<Uri> uriVec;
    uint32_t callerTokenId = 0;
    uint32_t flag = 0;
    std::vector<PolicyInfo> pathPolicies;
    auto ret = FilePermissionManager::CheckUriPersistentPermission(uriVec, callerTokenId, flag, "", pathPolicies);
    bool res = false;
    if (ret.empty()) {
        res = true;
    }
    EXPECT_EQ(res, true);
}

/*
 * Feature: CheckUriPersistentPermission
 * Function: CheckUriPersistentPermission
 * SubFunction: NA
 * FunctionPoints: UPMSUtils CheckUriPersistentPermission
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_002, TestSize.Level1)
{
    std::vector<Uri> uriVec;
    Uri uri = Uri("file://docs/storage/Users/currentUser/appdata/el2/base/demo");
    Uri uri1 = Uri("file://docs/storage/Users/currentUser/Download");
    Uri uri2 = Uri("file://docs/storage/Users/currentUser");
    Uri uri3 = Uri("file://docs/storage/Users");
    Uri uri4 = Uri("file://docs/storage");
    Uri uri5 = Uri("file://com.demo/data/storage/el2/base/haps/entry/files/test_A.txt");
    uriVec.emplace_back(uri);
    uriVec.emplace_back(uri1);
    uriVec.emplace_back(uri2);
    uriVec.emplace_back(uri3);
    uriVec.emplace_back(uri4);
    uriVec.emplace_back(uri5);
    uint32_t callerTokenId = 1002;
    uint32_t flag = 0;
    std::vector<PolicyInfo> pathPolicies;
    auto ret = FilePermissionManager::CheckUriPersistentPermission(uriVec, callerTokenId, flag, "", pathPolicies);
    EXPECT_FALSE(ret.empty());
}

/*
 * Feature: CheckUriPersistentPermission
 * Function: CheckUriPersistentPermission
 * SubFunction: NA
 * FunctionPoints: UPMSUtils CheckUriPersistentPermission
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_003, TestSize.Level1)
{
    std::vector<Uri> uriVec;
    Uri uri = Uri("file://docs/storage/Users/currentUser/appdata/el2/base/");
    Uri uri1 = Uri("file://docs/storage/Users/currentUser/appdata/el2/base/demo");
    Uri uri2 = Uri("file://docs/storage/Users/currentUser/Download");
    Uri uri3 = Uri("file://docs/storage/Users/currentUser");
    Uri uri4 = Uri("file://docs/storage/Users");
    Uri uri5 = Uri("file://docs/storage");
    uriVec.emplace_back(uri);
    uriVec.emplace_back(uri1);
    uriVec.emplace_back(uri2);
    uriVec.emplace_back(uri3);
    uriVec.emplace_back(uri4);
    uriVec.emplace_back(uri5);
    uint32_t callerTokenId = 1002;
    uint32_t flag = 0;
    MyFlag::permissionFileAccessManager_ = true;
    MyFlag::permissionSandboxAccessManager_ = true;
    std::vector<PolicyInfo> pathPolicies;
    auto ret = FilePermissionManager::CheckUriPersistentPermission(uriVec, callerTokenId, flag, "", pathPolicies);
    EXPECT_EQ(ret.size(), uriVec.size());
    bool res = true;
    for (size_t i = 0; i < ret.size(); i++) {
        if (!ret[i]) {
            res = ret[i];
        }
    }
    EXPECT_EQ(res, true);
}

/*
 * Feature: CheckUriPersistentPermission
 * Function: CheckUriPersistentPermission
 * SubFunction: NA
 * FunctionPoints: UPMSUtils CheckUriPersistentPermission
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_004, TestSize.Level1)
{
    std::vector<Uri> uriVec;
    Uri uri4 = Uri("file://docs/data/storage/el2/base/haps/entry/files/test_A.txt");
    uriVec.emplace_back(uri4);
    uint32_t callerTokenId = 1002;
    uint32_t flag = 0;
    MyFlag::permissionFileAccessManager_ = true;
    MyFlag::permissionSandboxAccessManager_ = true;
    std::vector<PolicyInfo> pathPolicies;
    auto ret = FilePermissionManager::CheckUriPersistentPermission(uriVec, callerTokenId, flag, "", pathPolicies);
    EXPECT_FALSE(ret[0]);
}

/*
 * Feature: GetPathPolicyInfoFromUri
 * Function: GetPathPolicyInfoFromUri
 * SubFunction: NA
 * FunctionPoints: GetPathPolicyInfoFromUri
 */
HWTEST_F(FilePermissionManagerTest, GetPathPolicyInfoFromUri_001, TestSize.Level1)
{
    Uri uri("file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt");
    uint32_t flag = 0;
    auto ret = FilePermissionManager::GetPathPolicyInfoFromUri(uri, flag);
    bool strRes = false;
    if (!ret.path.empty()) {
        strRes = true;
    }
    EXPECT_EQ(strRes, true);
}

/*
 * Feature: CheckDocsUriPermission
 * Function: CheckDocsUriPermission
 * SubFunction: NA
 * FunctionPoints: CheckDocsUriPermission
 */
HWTEST_F(FilePermissionManagerTest, CheckDocsUriPermission_001, TestSize.Level1)
{
    std::string appDataPath = "/storage/Users/currentUser/appdata/test_A.txt";
    TokenIdPermission caller(0);
    MyFlag::permissionFileAccessManager_ = false;
    MyFlag::permissionSandboxAccessManager_ = false;
    bool ret = FilePermissionManager::CheckDocsUriPermission(caller, appDataPath);
    ASSERT_FALSE(ret);
}

/*
 * Feature: CheckDocsUriPermission
 * Function: CheckDocsUriPermission
 * SubFunction: NA
 * FunctionPoints: CheckDocsUriPermission
 */
HWTEST_F(FilePermissionManagerTest, CheckDocsUriPermission_002, TestSize.Level1)
{
    TokenIdPermission caller(0);
    MyFlag::permissionFileAccessManager_ = false;
    MyFlag::permissionSandboxAccessManager_ = false;
    std::string appDataPath = "/storage/Users/currentUser/test/test_A.txt";
    bool ret = FilePermissionManager::CheckDocsUriPermission(caller, appDataPath);
    ASSERT_FALSE(ret);
}

/*
 * Feature: CheckDocsUriPermission
 * Function: CheckDocsUriPermission
 * SubFunction: NA
 * FunctionPoints: CheckDocsUriPermission
 */
HWTEST_F(FilePermissionManagerTest, CheckDocsUriPermission_003, TestSize.Level1)
{
    TokenIdPermission caller(0);
    MyFlag::permissionFileAccessManager_ = false;
    MyFlag::permissionSandboxAccessManager_ = false;
    std::string appDataPath = "/test/Users/currentUser/test/test_A.txt";
    bool ret = FilePermissionManager::CheckDocsUriPermission(caller, appDataPath);
    ASSERT_FALSE(ret);
}

/*
 * Feature: CheckDocsUriPermission
 * Function: CheckDocsUriPermission
 * SubFunction: NA
 * FunctionPoints: APPDATA_URI case-insensitive validation blocks FILE_ACCESS_MANAGER bypass
 */
HWTEST_F(FilePermissionManagerTest, CheckDocsUriPermission_004, TestSize.Level1)
{
    TokenIdPermission caller(0);
    MyFlag::permissionFileAccessManager_ = true;
    MyFlag::permissionSandboxAccessManager_ = false;
    std::string appDataPath = "/storage/Users/currentUser/Appdata/el2/base/";
    bool ret = FilePermissionManager::CheckDocsUriPermission(caller, appDataPath);
    ASSERT_FALSE(ret);
}

/*
 * Feature: CheckDocsUriPermission
 * Function: CheckDocsUriPermission
 * SubFunction: NA
 * FunctionPoints: APPDATA_URI case-insensitive validation blocks all-uppercase variant
 */
HWTEST_F(FilePermissionManagerTest, CheckDocsUriPermission_005, TestSize.Level1)
{
    TokenIdPermission caller(0);
    MyFlag::permissionFileAccessManager_ = true;
    MyFlag::permissionSandboxAccessManager_ = false;
    std::string appDataPath = "/storage/Users/currentUser/APPDATA/el2/base/";
    bool ret = FilePermissionManager::CheckDocsUriPermission(caller, appDataPath);
    ASSERT_FALSE(ret);
}

/*
 * Feature: CheckDocsUriPermission
 * Function: CheckDocsUriPermission
 * SubFunction: NA
 * FunctionPoints: APPDATA_URI case-variant path still grantable via SandboxAccess
 */
HWTEST_F(FilePermissionManagerTest, CheckDocsUriPermission_006, TestSize.Level1)
{
    TokenIdPermission caller(0);
    MyFlag::permissionFileAccessManager_ = true;
    MyFlag::permissionSandboxAccessManager_ = true;
    std::string appDataPath = "/storage/Users/currentUser/Appdata/el2/base/test.txt";
    bool ret = FilePermissionManager::CheckDocsUriPermission(caller, appDataPath);
    ASSERT_TRUE(ret);
}

/*
 * Feature: CheckDocsUriPermission
 * Function: CheckDocsUriPermission
 * SubFunction: NA
 * FunctionPoints: APPDATA_URI exact-case behavior unchanged when sandbox denied
 */
HWTEST_F(FilePermissionManagerTest, CheckDocsUriPermission_007, TestSize.Level1)
{
    TokenIdPermission caller(0);
    MyFlag::permissionFileAccessManager_ = true;
    MyFlag::permissionSandboxAccessManager_ = false;
    std::string appDataPath = "/storage/Users/currentUser/appdata/el2/base/";
    bool ret = FilePermissionManager::CheckDocsUriPermission(caller, appDataPath);
    ASSERT_FALSE(ret);
}

/*
 * Feature: CheckDocsUriPermission
 * Function: CheckDocsUriPermission
 * SubFunction: NA
 * FunctionPoints: AGENT_FILE_ACCESS(B) not declared, A denied -> denied (old A flow)
 */
HWTEST_F(FilePermissionManagerTest, CheckDocsUriPermission_AgentFileAccess_001, TestSize.Level1)
{
    TokenIdPermission caller(0);
    MyFlag::permissionFileAccessManager_ = false;
    MyFlag::permissionAgentFileAccess_ = false;
    MyFlag::permissionAgentFileAccessDeclared_ = false;
    std::string path = "/storage/Users/currentUser/files/test.txt";
    bool ret = FilePermissionManager::CheckDocsUriPermission(caller, path);
    ASSERT_FALSE(ret);
}

/*
 * Feature: CheckDocsUriPermission
 * Function: CheckDocsUriPermission
 * SubFunction: NA
 * FunctionPoints: AGENT_FILE_ACCESS(B) not declared, A granted -> granted (old A flow)
 */
HWTEST_F(FilePermissionManagerTest, CheckDocsUriPermission_AgentFileAccess_002, TestSize.Level1)
{
    TokenIdPermission caller(0);
    MyFlag::permissionFileAccessManager_ = true;
    MyFlag::permissionAgentFileAccess_ = false;
    MyFlag::permissionAgentFileAccessDeclared_ = false;
    std::string path = "/storage/Users/currentUser/files/test.txt";
    bool ret = FilePermissionManager::CheckDocsUriPermission(caller, path);
    ASSERT_TRUE(ret);
}

/*
 * Feature: CheckDocsUriPermission
 * Function: CheckDocsUriPermission
 * SubFunction: NA
 * FunctionPoints: AGENT_FILE_ACCESS(B) declared and granted -> granted (B governs, even if A denied)
 */
HWTEST_F(FilePermissionManagerTest, CheckDocsUriPermission_AgentFileAccess_003, TestSize.Level1)
{
    TokenIdPermission caller(0);
    MyFlag::permissionFileAccessManager_ = false;
    MyFlag::permissionAgentFileAccess_ = true;
    MyFlag::permissionAgentFileAccessDeclared_ = true;
    std::string path = "/storage/Users/currentUser/files/test.txt";
    bool ret = FilePermissionManager::CheckDocsUriPermission(caller, path);
    ASSERT_TRUE(ret);
}

/*
 * Feature: CheckDocsUriPermission
 * Function: CheckDocsUriPermission
 * SubFunction: NA
 * FunctionPoints: AGENT_FILE_ACCESS(B) declared but denied -> denied (B governs, even if A granted)
 */
HWTEST_F(FilePermissionManagerTest, CheckDocsUriPermission_AgentFileAccess_004, TestSize.Level1)
{
    TokenIdPermission caller(0);
    MyFlag::permissionFileAccessManager_ = true;
    MyFlag::permissionAgentFileAccess_ = false;
    MyFlag::permissionAgentFileAccessDeclared_ = true;
    std::string path = "/storage/Users/currentUser/files/test.txt";
    bool ret = FilePermissionManager::CheckDocsUriPermission(caller, path);
    ASSERT_FALSE(ret);
}

/*
 * Feature: InitDlSymbol
 * Function: InitDlSymbol
 * SubFunction: NA
 * FunctionPoints: InitDlSymbol
 */
HWTEST_F(FilePermissionManagerTest, InitDlSymbol_001, TestSize.Level1)
{
    DllWrapper wrapper;
    const char* soName = "test.z.so";
    const char* funcName = "testFunc";
    bool ret = wrapper.InitDlSymbol(soName, funcName);
    ASSERT_FALSE(ret);
}

/*
 * Feature: InitDlSymbol
 * Function: InitDlSymbol
 * SubFunction: NA
 * FunctionPoints: InitDlSymbol
 */
HWTEST_F(FilePermissionManagerTest, InitDlSymbol_002, TestSize.Level1)
{
    DllWrapper wrapper;
    const char* soName = "libmission_list.z.so";
    const char* funcName = "testFunc";
    bool ret = wrapper.InitDlSymbol(soName, funcName);
    ASSERT_FALSE(ret);
}

/*
 * Feature: InitDlSymbol
 * Function: InitDlSymbol
 * SubFunction: NA
 * FunctionPoints: InitDlSymbol
 */
HWTEST_F(FilePermissionManagerTest, InitDlSymbol_003, TestSize.Level1)
{
    DllWrapper wrapper;
    const char* soName = nullptr;
    const char* funcName = "testFunc";
    bool ret = wrapper.InitDlSymbol(soName, funcName);
    ASSERT_FALSE(ret);
}

/*
 * Feature: GetFunc
 * Function: GetFunc
 * SubFunction: NA
 * FunctionPoints: GetFunc
 */
HWTEST_F(FilePermissionManagerTest, GetFunc_001, TestSize.Level1)
{
    DllWrapper wrapper;
    auto ret = wrapper.GetFunc();
    EXPECT_EQ(ret, nullptr);
}

}
}