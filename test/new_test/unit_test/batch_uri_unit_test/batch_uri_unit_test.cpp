/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#define protected public
#include "batch_uri.h"
#include "fud_constants.h"
#include "check_result.h"
#undef private
#undef protected

using namespace testing::ext;
namespace OHOS {
namespace AAFwk {
class BatchUriTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.number: BatchUri_Branch_01
 * @tc.name: Init_Branches
 * @tc.desc: Coverage: 1. uriVec.empty() True; 2. index == 0 log; 3. Invalid scheme skip.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_01, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    EXPECT_EQ(batchUri.Init({}), 0);

    std::vector<std::string> uris = {"file://valid/1.txt", "ftp://invalid/2.txt"};
    // Original Init returns validUriCount (1)
    EXPECT_EQ(batchUri.Init(uris), 1);
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
}

/**
 * @tc.number: BatchUri_Branch_02
 * @tc.name: Init_SpecialSchemes
 * @tc.desc: Coverage: 1. ANCO_SCHEME; 2. MEDIA_AUTHORITY.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_02, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    std::vector<std::string> uris = {"content://test/1.txt", "file://media/2.txt"};
    EXPECT_EQ(batchUri.Init(uris), 2);
}

/**
 * @tc.number: BatchUri_Branch_03
 * @tc.name: Init_DocsAndTargetBundle
 * @tc.desc: Coverage: 1. DOCS_AUTHORITY; 2. isTargetBundleUri (True).
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_03, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    std::vector<std::string> uris = {"file://docs/1.txt", "file://target/2.txt"};
    EXPECT_EQ(batchUri.Init(uris, 0, "", "target", false), 2);
    // otherIndexes size is 2
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
}

/**
 * @tc.number: BatchUri_Branch_04
 * @tc.name: Init_SandboxAndCaller
 * @tc.desc: Coverage: 1. haveSandboxAccessPermission; 2. authority == caller.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_04, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    EXPECT_EQ(batchUri.Init({"file://caller/1.txt"}, 0, "caller", "", false), 1);
    
    BatchUri batchUri2;
    EXPECT_EQ(batchUri2.Init({"file://other/1.txt"}, 0, "", "", true), 1);
}

/**
 * @tc.number: BatchUri_Branch_05
 * @tc.name: Init_TargetWithPermission
 * @tc.desc: Coverage: 1. isTargetBundleUri && result=true; 2. mode > 0 selfBundlePolicy.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_05, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    // targetAlterBundleName and authority match, InitFileUriInfo returns early for target bundle
    EXPECT_EQ(batchUri.Init({"file://target/1.txt"}, 1, "", "target", true), 1);
    EXPECT_EQ(batchUri.selfBundlePolicyInfos.size(), 0);
}

/**
 * @tc.number: BatchUri_Branch_06
 * @tc.name: SetResults_Branches
 * @tc.desc: Coverage: 1. SetMediaUriCheckResult; 2. SetOtherUriCheckResult (target bundle && result=true).
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_06, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"file://media/1.txt", "file://target/2.txt"}, 0, "", "target", false);
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{}); // target bundle falls through to otherIndexes
    batchUri.SetMediaUriCheckResult({true});
    batchUri.SetOtherUriCheckResult({true});
    EXPECT_TRUE(batchUri.checkResult[1].result);
}

/**
 * @tc.number: BatchUri_Branch_07
 * @tc.name: GetMedia_And_Select
 * @tc.desc: Coverage: 1. GetMediaUriToGrant (result False path); 2. SelectPermissionedUri.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_07, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"file://media/1.txt"});
    batchUri.SetMediaUriCheckResult({false});
    std::vector<std::string> out;
    EXPECT_EQ(batchUri.GetMediaUriToGrant(out), 0);

    std::vector<Uri> uris = {Uri("file://1.txt")};
    std::vector<int32_t> ids = {0};
    batchUri.checkResult[0].result = true;
    batchUri.SelectPermissionedUri(uris, ids, out);
    EXPECT_EQ(out.size(), 1);
}

/**
 * @tc.number: BatchUri_Branch_08
 * @tc.name: Proxy_Mismatch_And_False
 * @tc.desc: Coverage: 1. GetNeedCheckProxyPermissionURI (result True -> skip); 2. SetCheckProxyByPolicyResult mismatch.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_08, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"file://target/1.txt"}, 0, "", "target", true);
    std::vector<PolicyInfo> proxyUris;
    batchUri.GetNeedCheckProxyPermissionURI(proxyUris);
    EXPECT_FALSE(batchUri.SetCheckProxyByPolicyResult({true}));
}

/**
 * @tc.number: BatchUri_Branch_09
 * @tc.name: Proxy_Success_Branches
 * @tc.desc: Coverage: 1. SetCheckProxyByPolicyResult (isTarget True); 2. SetCheckProxyByPolicyResult (isTarget False).
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_09, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"file://target/1.txt", "file://other/2.txt"}, 0, "", "target", false);
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});

    std::vector<PolicyInfo> proxyUris;
    batchUri.GetNeedCheckProxyPermissionURI(proxyUris);
    EXPECT_TRUE(batchUri.SetCheckProxyByPolicyResult({true, true}));
}

/**
 * @tc.number: BatchUri_Branch_10
 * @tc.name: GrantPolicy1_False_And_Target
 * @tc.desc: Coverage: 1. GetUriToGrantByPolicy1 (result=False -> skip); 2. isTarget -> skip.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_10, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"file://other/1.txt", "file://target/2.txt"}, 0, "", "target", false);
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});

    batchUri.checkResult[1].result = true;
    std::vector<PolicyInfo> d, b;
    EXPECT_EQ(batchUri.GetUriToGrantByPolicy(d, b), 0);
}

/**
 * @tc.number: BatchUri_Branch_11
 * @tc.name: GrantPolicy1_Collect_Branches
 * @tc.desc: Coverage: 1. GetUriToGrantByPolicy1 (Docs); 2. GetUriToGrantByPolicy1 (OtherBundle).
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_11, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"file://docs/1.txt", "file://other/2.txt"}, 0, "", "target", false);
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});

    batchUri.checkResult[0].result = true;
    batchUri.checkResult[1].result = true;
    std::vector<PolicyInfo> d, b;
    EXPECT_EQ(batchUri.GetUriToGrantByPolicy(d, b), 2);
}

/**
 * @tc.number: BatchUri_Branch_12
 * @tc.name: GrantPolicy2_Fail_And_Target
 * @tc.desc: Coverage: 1. GetUriToGrantByPolicy2 (result=False -> return false); 2. isTarget skip.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_12, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"file://other/1.txt"});
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    std::vector<PolicyInfo> p;
    EXPECT_FALSE(batchUri.GetUriToGrantByPolicy(p));

    BatchUri batchUri2;
    batchUri2.Init({"file://target/1.txt"}, 0, "", "target", false);
    batchUri2.otherPolicyInfos.emplace_back(PolicyInfo{});
    batchUri2.checkResult[0].result = true;
    EXPECT_TRUE(batchUri2.GetUriToGrantByPolicy(p));
}

/**
 * @tc.number: BatchUri_Branch_13
 * @tc.name: Misc_Branch
 * @tc.desc: Coverage: 1. SetCheckUriAuthorizationResult mismatch; 2. IsAllUriPermissioned False.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_13, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"file://1.txt"});
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    std::vector<bool> r;
    EXPECT_FALSE(batchUri.SetCheckUriAuthorizationResult(r));
    EXPECT_FALSE(batchUri.IsAllUriPermissioned());
}

/**
 * @tc.number: BatchUri_Branch_14
 * @tc.name: Misc_True_Branches
 * @tc.desc: Coverage: 1. SetCheckUriAuthorizationResult Success; 2. IsAllUriPermissioned True.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_14, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"file://1.txt"});
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    batchUri.checkResult[0].result = true;
    std::vector<bool> r = {false};
    EXPECT_TRUE(batchUri.SetCheckUriAuthorizationResult(r));
    EXPECT_TRUE(batchUri.IsAllUriPermissioned());
}

/**
 * @tc.number: BatchUri_Branch_15
 * @tc.name: Init_EmptyAuthority
 * @tc.desc: Coverage: 1. authority.empty() path.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_15, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    EXPECT_EQ(batchUri.Init({"file:///path"}), 1);
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
}

/**
 * @tc.number: BatchUri_Branch_16
 * @tc.name: Init_TargetLogic_Branches
 * @tc.desc: Coverage: 1. targetAlterBundleName empty; 2. authority != target.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_16, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    EXPECT_EQ(batchUri.Init({"file://some/1.txt"}, 0, "", "", false), 1);
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});

    BatchUri batchUri2;
    EXPECT_EQ(batchUri2.Init({"file://other/1.txt"}, 0, "", "target", false), 1);
    batchUri2.otherPolicyInfos.emplace_back(PolicyInfo{});
}

/**
 * @tc.number: BatchUri_Branch_17
 * @tc.name: SetProxyResult_FalsePath
 * @tc.desc: Coverage: 1. SetCheckProxyByPolicyResult loop result=False.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_17, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"file://other/1.txt"});
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    std::vector<PolicyInfo> p;
    batchUri.GetNeedCheckProxyPermissionURI(p);
    EXPECT_TRUE(batchUri.SetCheckProxyByPolicyResult({false}));
}

/**
 * @tc.number: BatchUri_Branch_18
 * @tc.name: GetPermissionedCount_Branch
 * @tc.desc: Coverage: 1. GetPermissionedUriCount loop; 2. GetUriToGrantByMap baseline.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_18, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"file://1.txt", "file://2.txt"});
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    batchUri.checkResult[0].result = true;
    EXPECT_EQ(batchUri.GetPermissionedUriCount(), 1);
}

/**
 * @tc.number: BatchUri_Branch_19
 * @tc.name: Init_Multi_IndexLog
 * @tc.desc: Coverage: 1. index > 0 path; 2. mode == 0 path.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_19, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    EXPECT_EQ(batchUri.Init({"file://1/1.txt", "file://2/2.txt"}, 0, "1", "", false), 2);
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
}

/**
 * @tc.number: BatchUri_Branch_20
 * @tc.name: Proxy_Success_Clear
 * @tc.desc: Coverage: 1. proxyIndexesByPolicy.clear() path.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_20, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"file://other/1.txt"});
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    std::vector<PolicyInfo> p;
    batchUri.GetNeedCheckProxyPermissionURI(p);
    batchUri.SetCheckProxyByPolicyResult({true});
    EXPECT_EQ(batchUri.proxyIndexesByPolicy.size(), 0);
}

/**
 * @tc.number: BatchUri_Branch_21
 * @tc.name: SetOtherResult_Multi
 * @tc.desc: Coverage: 1. SetOtherUriCheckResult loop with mix.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_21, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"file://other1/1.txt", "file://other2/2.txt"});
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    batchUri.SetOtherUriCheckResult({true, false});
    EXPECT_TRUE(batchUri.checkResult[0].result);
}

/**
 * @tc.number: BatchUri_Branch_22
 * @tc.name: Final_Branches
 * @tc.desc: Coverage: 1. GetUriToGrantByPolicy1 selfBundle loop; 2. Mode > 0 other branches.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_22, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    // selfBundlePolicyInfos has 1, otherIndexes has 1
    batchUri.Init({"file://caller/1.txt", "file://docs/2.txt"}, 1, "caller", "", false);
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{}); // exact 1 for docs URI

    batchUri.checkResult[1].result = true;
    std::vector<PolicyInfo> d, b;
    EXPECT_EQ(batchUri.GetUriToGrantByPolicy(d, b), 2);
}

/**
 * @tc.number: BatchUri_Branch_23
 * @tc.name: SetCheckProxyByContentUriResult_Mismatch
 * @tc.desc: Coverage: SetCheckProxyByContentUriResult size mismatch.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_23, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"content://test/1.txt"});
    std::vector<bool> contentUriResult = {true, false}; // Size mismatch
    EXPECT_FALSE(batchUri.SetCheckProxyByContentUriResult(contentUriResult));
}

/**
 * @tc.number: BatchUri_Branch_24
 * @tc.name: SetCheckProxyByContentUriResult_Success
 * @tc.desc: Coverage: SetCheckProxyByContentUriResult success path.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_24, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"content://test/1.txt", "content://test/2.txt"});
    std::vector<bool> contentUriResult = {true, false};
    EXPECT_TRUE(batchUri.SetCheckProxyByContentUriResult(contentUriResult));
    EXPECT_TRUE(batchUri.checkResult[0].result);
    EXPECT_FALSE(batchUri.checkResult[1].result);
    EXPECT_EQ(batchUri.checkResult[0].permissionType, static_cast<int32_t>(PolicyType::AUTHORIZATION_PATH));
}

/**
 * @tc.number: BatchUri_Branch_25
 * @tc.name: SetCheckProxyByContentUriResult_AllTrue
 * @tc.desc: Coverage: SetCheckProxyByContentUriResult all true.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_25, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"content://test/1.txt", "content://test/2.txt"});
    std::vector<bool> contentUriResult = {true, true};
    EXPECT_TRUE(batchUri.SetCheckProxyByContentUriResult(contentUriResult));
    EXPECT_TRUE(batchUri.IsAllUriPermissioned());
}

/**
 * @tc.number: BatchUri_Branch_26
 * @tc.name: IsAllUriValid_Partial
 * @tc.desc: Coverage: IsAllUriValid with some invalid URIs.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_26, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"file://valid/1.txt", "ftp://invalid/2.txt", "file://valid/3.txt"});
    EXPECT_FALSE(batchUri.IsAllUriValid());
    EXPECT_EQ(batchUri.validUriCount, 2);
    EXPECT_EQ(batchUri.totalUriCount, 3);
}

/**
 * @tc.number: BatchUri_Branch_27
 * @tc.name: IsAllUriValid_AllValid
 * @tc.desc: Coverage: IsAllUriValid all URIs valid.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_27, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"file://valid/1.txt", "file://valid/2.txt"});
    EXPECT_TRUE(batchUri.IsAllUriValid());
}

/**
 * @tc.number: BatchUri_Branch_28
 * @tc.name: GetPermissionedUriCount_All
 * @tc.desc: Coverage: GetPermissionedUriCount all permissioned.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_28, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"file://1.txt", "file://2.txt", "file://3.txt"});
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    batchUri.checkResult[0].result = true;
    batchUri.checkResult[1].result = true;
    batchUri.checkResult[2].result = true;
    EXPECT_EQ(batchUri.GetPermissionedUriCount(), 3);
}

/**
 * @tc.number: BatchUri_Branch_29
 * @tc.name: GetPermissionedUriCount_None
 * @tc.desc: Coverage: GetPermissionedUriCount none permissioned.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_29, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"file://1.txt", "file://2.txt"});
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    EXPECT_EQ(batchUri.GetPermissionedUriCount(), 0);
}

/**
 * @tc.number: BatchUri_Branch_30
 * @tc.name: Init_ContentUri_Mixed
 * @tc.desc: Coverage: Init with content URIs mixed with file URIs.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_30, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    std::vector<std::string> uris = {
        "content://test/1.txt",
        "file://media/2.txt",
        "content://test/3.txt",
        "file://docs/4.txt"
    };
    EXPECT_EQ(batchUri.Init(uris), 4);
    EXPECT_EQ(batchUri.contentUris.size(), 2);
    EXPECT_EQ(batchUri.mediaUris.size(), 1);
    EXPECT_EQ(batchUri.otherUris.size(), 1);
}

/**
 * @tc.number: BatchUri_Branch_31
 * @tc.name: SetCheckProxyByPolicyResult_TargetBundleTrue
 * @tc.desc: Coverage: SetCheckProxyByPolicyResult with target bundle true.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_31, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"file://target/1.txt"}, 0, "", "target", false);
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    std::vector<PolicyInfo> proxyUris;
    batchUri.GetNeedCheckProxyPermissionURI(proxyUris);
    EXPECT_TRUE(batchUri.SetCheckProxyByPolicyResult({true}));
    EXPECT_EQ(batchUri.targetBundleUriCount, 1);
}

/**
 * @tc.number: BatchUri_Branch_32
 * @tc.name: GetUriToGrantByPolicy2_EmptySelfBundle
 * @tc.desc: Coverage: GetUriToGrantByPolicy2 with empty selfBundlePolicyInfos.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_32, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"file://docs/1.txt"});
    batchUri.otherPolicyInfos.emplace_back(PolicyInfo{});
    batchUri.checkResult[0].result = true;
    std::vector<PolicyInfo> policyVec;
    EXPECT_TRUE(batchUri.GetUriToGrantByPolicy(policyVec));
    EXPECT_EQ(policyVec.size(), 1);
}

/**
 * @tc.number: BatchUri_Branch_33
 * @tc.name: GetUriToGrantByPolicy2_OnlySelfBundle
 * @tc.desc: Coverage: GetUriToGrantByPolicy2 with only self bundle URIs.
 */
HWTEST_F(BatchUriTest, BatchUri_Branch_33, Function | MediumTest | Level1)
{
    BatchUri batchUri;
    batchUri.Init({"file://caller/1.txt"}, 1, "caller", "", false);
    batchUri.checkResult[0].result = true;
    std::vector<PolicyInfo> policyVec;
    EXPECT_TRUE(batchUri.GetUriToGrantByPolicy(policyVec));
    EXPECT_EQ(policyVec.size(), 1);
}

} // AAFwk
} // OHOS
