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

#include "accesstoken_kit.h"
#include "app_utils.h"
#include "array_wrapper.h"
#include "string_wrapper.h"

#include "uri_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
const int32_t BEYOND_MAX_URI_COUNT = 501;
const int32_t MAX_URI_COUNT = 500;
constexpr const char* ABILIY_PARAM_STREAM = "ability.params.stream";
}
using namespace Security::AccessToken;
class UriUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UriUtilsTest::SetUpTestCase() {}

void UriUtilsTest::TearDownTestCase() {}

void UriUtilsTest::SetUp()
{
    AccessTokenKit::InitMockResult();
}

void UriUtilsTest::TearDown() {}

/*
 * Feature: UriUtils
 * Function: GetUriListFromWantDms
 * SubFunction: NA
 * FunctionPoints: UriUtils GetUriListFromWantDms
 */
HWTEST_F(UriUtilsTest, GetUriListFromWantDms_001, TestSize.Level1)
{
    Want want;
    auto uriList = UriUtils::GetInstance().GetUriListFromWantDms(want);
    EXPECT_EQ(uriList.size(), 0);

    WantParams params;
    sptr<AAFwk::IArray> ao = new (std::nothrow) AAFwk::Array(BEYOND_MAX_URI_COUNT, AAFwk::g_IID_IString);
    if (ao != nullptr) {
        for (size_t i = 0; i < BEYOND_MAX_URI_COUNT; i++) {
            ao->Set(i, String::Box("file"));
        }
        params.SetParam("ability.verify.uri", ao);
    }
    want.SetParams(params);
    auto uriList2 = UriUtils::GetInstance().GetUriListFromWantDms(want);
    EXPECT_EQ(uriList2.size(), 0);

    sptr<AAFwk::IArray> ao2 = new (std::nothrow) AAFwk::Array(1, AAFwk::g_IID_IString);
    if (ao2 != nullptr) {
        ao2->Set(0, String::Box("file://data/storage/el2/distributedfiles/test.txt"));
        params.SetParam("ability.verify.uri", ao2);
    }
    want.SetParams(params);
    auto uriList3 = UriUtils::GetInstance().GetUriListFromWantDms(want);
    EXPECT_EQ(uriList3.size(), 0);
}

/*
 * Feature: UriUtils
 * Function: GetPermissionedUriList
 * SubFunction: NA
 * FunctionPoints: UriUtils GetPermissionedUriList
 */
HWTEST_F(UriUtilsTest, GetPermissionedUriList_001, TestSize.Level1)
{
    AccessTokenKit::hapInfo.apiVersion = 20;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::getHapTokenInfoRet = 0;
    std::vector<std::string> uriVec;
    std::vector<bool> checkResults = {true};
    Want want;
    std::vector<Uri> vec = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(vec.size(), 0);

    want.SetUri("ability.verify.uri");
    uriVec.push_back("file://data/storage/el2/distributedfiles/test.txt");
    std::vector<Uri> vec2 = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(vec2.size(), 1);

    checkResults[0] = false;
    std::vector<Uri> vec3 = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(vec3.size(), 0);

    checkResults[0] = true;
    uriVec.push_back("https//test.openharmony.com");
    checkResults.push_back(true);
    std::vector<Uri> vec4 = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(vec4.size(), 2);

    checkResults[1] = false;
    std::vector<Uri> vec5 = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(vec5.size(), 1);
}

/*
 * Feature: UriUtils
 * Function: GetPermissionedUriList
 * SubFunction: NA
 * FunctionPoints: UriUtils GetPermissionedUriList, want has permissioned file uri param
 */
HWTEST_F(UriUtilsTest, GetPermissionedUriList_002, TestSize.Level1)
{
    AccessTokenKit::hapInfo.apiVersion = 20;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::getHapTokenInfoRet = 0;
    std::string uri = "file://com.example.test/test.txt";
    Want want;
    want.SetUri(uri);
    std::vector<std::string> uriVec = { uri };
    std::vector<bool> checkResults = { true };
    std::vector<Uri> vec = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(want.GetUriString(), uri);
}

/*
 * Feature: UriUtils
 * Function: GetPermissionedUriList
 * SubFunction: NA
 * FunctionPoints: UriUtils GetPermissionedUriList, want has unPrivileged file uri param
 */
HWTEST_F(UriUtilsTest, GetPermissionedUriList_003, TestSize.Level1)
{
    AccessTokenKit::hapInfo.apiVersion = 20;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::getHapTokenInfoRet = 0;
    std::string uri = "file://docs/test.txt";
    Want want;
    want.SetUri(uri);
    std::vector<std::string> uriVec = { uri };
    std::vector<bool> checkResults = { false };
    std::vector<Uri> vec = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(want.GetUriString(), "");
}

/*
 * Feature: UriUtils
 * Function: GetPermissionedUriList
 * SubFunction: NA
 * FunctionPoints: UriUtils GetPermissionedUriList, want has other uri param (scheme is not empty)
 */
HWTEST_F(UriUtilsTest, GetPermissionedUriList_004, TestSize.Level1)
{
    AccessTokenKit::hapInfo.apiVersion = 20;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::getHapTokenInfoRet = 0;
    std::string uri = "http://com.example.test/test.txt";
    Want want;
    want.SetUri(uri);
    std::vector<std::string> uriVec = { uri };
    std::vector<bool> checkResults = { false };
    std::vector<Uri> vec = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(want.GetUriString(), uri);
}

/*
 * Feature: UriUtils
 * Function: GetPermissionedUriList
 * SubFunction: NA
 * FunctionPoints: UriUtils GetPermissionedUriList, want has other uri param (scheme is empty)
 */
HWTEST_F(UriUtilsTest, GetPermissionedUriList_005, TestSize.Level1)
{
    AccessTokenKit::hapInfo.apiVersion = 20;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::getHapTokenInfoRet = 0;
    std::string uri = "/data/storage/el2/temp.txt";
    Want want;
    want.SetUri(uri);
    std::vector<std::string> uriVec = { uri };
    std::vector<bool> checkResults = { false };
    std::vector<Uri> vec = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(want.GetUriString(), "");
}

/*
 * Feature: UriUtils
 * Function: GetPermissionedUriList
 * SubFunction: NA
 * FunctionPoints: UriUtils GetPermissionedUriList, want has other uri param (scheme is empty)
 */
HWTEST_F(UriUtilsTest, GetPermissionedUriList_006, TestSize.Level1)
{
    AccessTokenKit::hapInfo.apiVersion = 20;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::getHapTokenInfoRet = 0;
    std::string uri = ":temp.txt";
    Want want;
    want.SetUri(uri);
    std::vector<std::string> uriVec = { uri };
    std::vector<bool> checkResults = { false };
    std::vector<Uri> vec = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(want.GetUriString(), "");
}

/*
 * Feature: UriUtils
 * Function: GetPermissionedUriList
 * SubFunction: NA
 * FunctionPoints: UriUtils GetPermissionedUriList, param stream with permissioned file uri
 */
HWTEST_F(UriUtilsTest, GetPermissionedUriList_007, TestSize.Level1)
{
    AccessTokenKit::hapInfo.apiVersion = 20;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::getHapTokenInfoRet = 0;
    std::string uri = "file://com.example.test/test.txt";
    Want want;
    std::vector<std::string> paramStreamUris = { uri };
    want.SetParam("ability.params.stream", paramStreamUris);

    std::vector<std::string> uriVec = { uri };
    std::vector<bool> checkResults = { true };
    std::vector<Uri> vec = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(vec.size(), 1);
    
    std::vector<std::string> paramStreamUris2 = want.GetStringArrayParam("ability.params.stream");
    EXPECT_EQ(paramStreamUris2.size(), 1);
}

/*
 * Feature: UriUtils
 * Function: GetPermissionedUriList
 * SubFunction: NA
 * FunctionPoints: UriUtils GetPermissionedUriList, param stream with unprivileged file uri
 */
HWTEST_F(UriUtilsTest, GetPermissionedUriList_008, TestSize.Level1)
{
    AccessTokenKit::hapInfo.apiVersion = 20;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::getHapTokenInfoRet = 0;
    std::string uri = "file://docs/test.txt";
    Want want;
    std::vector<std::string> paramStreamUris = { uri };
    want.SetParam("ability.params.stream", paramStreamUris);

    std::vector<std::string> uriVec = { uri };
    std::vector<bool> checkResults = { false };
    std::vector<Uri> vec = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(vec.size(), 0);
    
    std::vector<std::string> paramStreamUris2 = want.GetStringArrayParam("ability.params.stream");
    EXPECT_EQ(paramStreamUris2.size(), 0);
}

/*
 * Feature: UriUtils
 * Function: GetPermissionedUriList
 * SubFunction: NA
 * FunctionPoints: UriUtils GetPermissionedUriList, param stream with other uri(scheme is not empty)
 */
HWTEST_F(UriUtilsTest, GetPermissionedUriList_009, TestSize.Level1)
{
    AccessTokenKit::hapInfo.apiVersion = 20;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::getHapTokenInfoRet = 0;
    std::string uri = "http://temp/test.txt";
    Want want;
    std::vector<std::string> paramStreamUris = { uri };
    want.SetParam("ability.params.stream", paramStreamUris);

    std::vector<std::string> uriVec = { uri };
    std::vector<bool> checkResults = { false };
    std::vector<Uri> vec = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(vec.size(), 0);
    
    std::vector<std::string> paramStreamUris2 = want.GetStringArrayParam("ability.params.stream");
    EXPECT_EQ(paramStreamUris2.size(), 0);
}

/*
 * Feature: UriUtils
 * Function: GetPermissionedUriList
 * SubFunction: NA
 * FunctionPoints: UriUtils GetPermissionedUriList, param stream with other uri(scheme is empty)
 */
HWTEST_F(UriUtilsTest, GetPermissionedUriList_010, TestSize.Level1)
{
    AccessTokenKit::hapInfo.apiVersion = 20;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::getHapTokenInfoRet = 0;
    std::string uri = "data/temp/test.txt";
    Want want;
    std::vector<std::string> paramStreamUris = { uri };
    want.SetParam("ability.params.stream", paramStreamUris);

    std::vector<std::string> uriVec = { uri };
    std::vector<bool> checkResults = { false };
    std::vector<Uri> vec = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(vec.size(), 0);
    
    std::vector<std::string> paramStreamUris2 = want.GetStringArrayParam("ability.params.stream");
    EXPECT_EQ(paramStreamUris2.size(), 0);
}

/*
 * Feature: UriUtils
 * Function: GetPermissionedUriList
 * SubFunction: NA
 * FunctionPoints: UriUtils GetPermissionedUriList, param stream with other uri(scheme is empty)
 */
HWTEST_F(UriUtilsTest, GetPermissionedUriList_011, TestSize.Level1)
{
    AccessTokenKit::hapInfo.apiVersion = 20;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::getHapTokenInfoRet = 0;
    std::string uri = ":data/temp/test.txt";
    Want want;
    std::vector<std::string> paramStreamUris = { uri };
    want.SetParam("ability.params.stream", paramStreamUris);

    std::vector<std::string> uriVec = { uri };
    std::vector<bool> checkResults = { false };
    std::vector<Uri> vec = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(vec.size(), 0);
    
    std::vector<std::string> paramStreamUris2 = want.GetStringArrayParam("ability.params.stream");
    EXPECT_EQ(paramStreamUris2.size(), 0);
}

/*
 * Feature: UriUtils
 * Function: GetPermissionedUriList
 * SubFunction: NA
 * FunctionPoints: UriUtils GetPermissionedUriList, param stream with file uri and Uri is file uri
 */
HWTEST_F(UriUtilsTest, GetPermissionedUriList_012, TestSize.Level1)
{
    AccessTokenKit::hapInfo.apiVersion = 20;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::getHapTokenInfoRet = 0;
    std::string uri = "file://com.exmaple.test/data/temp/test.txt";
    Want want;
    want.SetUri(uri);
    std::vector<std::string> paramStreamUris = { uri };
    want.SetParam("ability.params.stream", paramStreamUris);

    std::vector<std::string> uriVec = { uri, uri };
    std::vector<bool> checkResults = { true, true };
    std::vector<Uri> vec = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(vec.size(), 2);
    // param stream
    std::vector<std::string> paramStreamUris2 = want.GetStringArrayParam("ability.params.stream");
    EXPECT_EQ(paramStreamUris2.size(), 1);
    // uri
    EXPECT_EQ(want.GetUriString(), uri);
}

/*
 * Feature: UriUtils
 * Function: GetPermissionedUriList
 * SubFunction: NA
 * FunctionPoints: UriUtils GetPermissionedUriList, param stream with other uri and Uri is file uri
 */
HWTEST_F(UriUtilsTest, GetPermissionedUriList_013, TestSize.Level1)
{
    AccessTokenKit::hapInfo.apiVersion = 20;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::getHapTokenInfoRet = 0;
    std::string uri1 = "file://com.exmaple.test/data/temp/test.txt";
    std::string uri2 = "http://com.exmaple.test/data/temp/test.txt";
    Want want;
    want.SetUri(uri1);
    std::vector<std::string> paramStreamUris = { uri2 };
    want.SetParam("ability.params.stream", paramStreamUris);

    std::vector<std::string> uriVec = { uri1, uri2 };
    std::vector<bool> checkResults = { true, false };
    std::vector<Uri> vec = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(vec.size(), 1);
    // param stream
    std::vector<std::string> paramStreamUris2 = want.GetStringArrayParam("ability.params.stream");
    EXPECT_EQ(paramStreamUris2.size(), 0);
    // uri
    EXPECT_EQ(want.GetUriString(), uri1);
}

/*
 * Feature: UriUtils
 * Function: GetPermissionedUriList
 * SubFunction: NA
 * FunctionPoints: UriUtils GetPermissionedUriList, param stream with file uri and Uri is other uri(schem is not empty)
 */
HWTEST_F(UriUtilsTest, GetPermissionedUriList_014, TestSize.Level1)
{
    AccessTokenKit::hapInfo.apiVersion = 20;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::getHapTokenInfoRet = 0;
    std::string uri1 = "file://com.exmaple.test/data/temp/test.txt";
    std::string uri2 = "http://com.exmaple.test/data/temp/test.txt";
    Want want;
    want.SetUri(uri2);
    std::vector<std::string> paramStreamUris = { uri1 };
    want.SetParam("ability.params.stream", paramStreamUris);

    std::vector<std::string> uriVec = { uri2, uri1 };
    std::vector<bool> checkResults = { false, true };
    std::vector<Uri> vec = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(vec.size(), 1);
    // param stream
    std::vector<std::string> paramStreamUris2 = want.GetStringArrayParam("ability.params.stream");
    EXPECT_EQ(paramStreamUris2.size(), 1);
    // uri
    EXPECT_EQ(want.GetUriString(), uri2);
}

/*
 * Feature: UriUtils
 * Function: GetPermissionedUriList
 * SubFunction: NA
 * FunctionPoints: UriUtils GetPermissionedUriList, param stream with file uri and Uri is other uri(schem is empty)
 */
HWTEST_F(UriUtilsTest, GetPermissionedUriList_015, TestSize.Level1)
{
    AccessTokenKit::hapInfo.apiVersion = 20;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::getHapTokenInfoRet = 0;
    std::string uri1 = "file://com.exmaple.test/data/temp/test.txt";
    std::string uri2 = "/data/temp/test.txt";
    Want want;
    want.SetUri(uri2);
    std::vector<std::string> paramStreamUris = { uri1 };
    want.SetParam("ability.params.stream", paramStreamUris);

    std::vector<std::string> uriVec = { uri2, uri1 };
    std::vector<bool> checkResults = { false, true };
    std::vector<Uri> vec = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(vec.size(), 1);
    // param stream
    std::vector<std::string> paramStreamUris2 = want.GetStringArrayParam("ability.params.stream");
    EXPECT_EQ(paramStreamUris2.size(), 1);
    // uri
    EXPECT_EQ(want.GetUriString(), "");
}

/*
 * Feature: UriUtils
 * Function: GetPermissionedUriList
 * SubFunction: NA
 * FunctionPoints: UriUtils GetPermissionedUriList, param stream with other uri and Uri is other uri(schem is empty)
 */
HWTEST_F(UriUtilsTest, GetPermissionedUriList_016, TestSize.Level1)
{
    AccessTokenKit::hapInfo.apiVersion = 20;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::getHapTokenInfoRet = 0;
    std::string uri1 = "http://com.exmaple.test/data/temp/test.txt";
    std::string uri2 = "/data/temp/test.txt";
    Want want;
    want.SetUri(uri2);
    std::vector<std::string> paramStreamUris = { uri1 };
    want.SetParam("ability.params.stream", paramStreamUris);

    std::vector<std::string> uriVec = { uri2, uri1 };
    std::vector<bool> checkResults = { false, false };
    std::vector<Uri> vec = UriUtils::GetInstance().GetPermissionedUriList(uriVec, checkResults, 0, "target", want);
    EXPECT_EQ(vec.size(), 0);
    // param stream
    std::vector<std::string> paramStreamUris2 = want.GetStringArrayParam("ability.params.stream");
    EXPECT_EQ(paramStreamUris2.size(), 0);
    // uri
    EXPECT_EQ(want.GetUriString(), "");
}

/*
 * Feature: UriUtils
 * Function: GetPermissionedUriList
 * SubFunction: NA
 * FunctionPoints: UriUtils GetPermissionedUriList, param stream with other uri and Uri is other uri(schem is empty)
 */
HWTEST_F(UriUtilsTest, GetPermissionedUriList_017, TestSize.Level1)
{
    AccessTokenKit::hapInfo.apiVersion = 19;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::getHapTokenInfoRet = 0;
    std::string uri1 = "http://com.exmaple.test/data/temp/test.txt";
    std::string uri2 = "file://com.example.test/data/temp/test.txt";
    std::vector<std::string> paramStreamUris = { uri1, uri2 };
    Want want;
    want.SetParam("ability.params.stream", paramStreamUris);
    std::vector<bool> checkResults = { false, false };
    auto vec = UriUtils::GetInstance().GetPermissionedUriList(paramStreamUris, checkResults, 0, "target", want);
    EXPECT_EQ(vec.size(), 0);
    // param stream
    std::vector<std::string> paramStreamUris1 = want.GetStringArrayParam("ability.params.stream");
    EXPECT_EQ(paramStreamUris1.size(), 1);
}

/*
 * Feature: UriUtils
 * Function: GetUriListFromWant
 * SubFunction: NA
 * FunctionPoints: UriUtils GetUriListFromWant
 */
HWTEST_F(UriUtilsTest, GetUriListFromWant_001, TestSize.Level1)
{
    Want want;
    WantParams params;
    sptr<AAFwk::IArray> ao = new (std::nothrow) AAFwk::Array(1, AAFwk::g_IID_IString);
    if (ao != nullptr) {
        ao->Set(0, String::Box("file"));
        params.SetParam("ability.params.stream", ao);
    }
    std::vector<std::string> uriVec;
    bool res0 = UriUtils::GetInstance().GetUriListFromWant(want, uriVec);
    EXPECT_EQ(res0, false);

    for (size_t i = 0; i < BEYOND_MAX_URI_COUNT; i++) {
        uriVec.push_back("https//test.openharmony.com");
    }
    want.SetUri("file://data/storage/el2/distributedfiles/test.txt");
    bool res1 = UriUtils::GetInstance().GetUriListFromWant(want, uriVec);
    EXPECT_EQ(res1, true);

    bool res2 = UriUtils::GetInstance().GetUriListFromWant(want, uriVec);
    EXPECT_EQ(res2, true);

    uriVec.clear();
    uriVec.push_back("https//test.openharmony.com");
    bool res3 = UriUtils::GetInstance().GetUriListFromWant(want, uriVec);
    EXPECT_EQ(res3, true);
}

/*
 * Feature: UriUtils
 * Function: IsDmsCall
 * SubFunction: NA
 * FunctionPoints: UriUtils IsDmsCall
 */
HWTEST_F(UriUtilsTest, IsDmsCall_001, TestSize.Level1)
{
    uint32_t fromTokenId = 1001;
    bool res1 = UriUtils::GetInstance().IsDmsCall(fromTokenId);
    EXPECT_EQ(res1, false);
}

#ifdef SUPPORT_UPMS
/*
 * Feature: UriUtils
 * Function: GrantDmsUriPermission
 * SubFunction: NA
 * FunctionPoints: UriUtils GrantDmsUriPermission
 */
HWTEST_F(UriUtilsTest, GrantDmsUriPermission_001, TestSize.Level1)
{
    Want want;
    uint32_t callerTokenId = 1;
    std::string targetBundleName = "com.example.tsapplication";
    int32_t appIndex = 101;
    WantParams params;
    sptr<AAFwk::IArray> ao2 = new (std::nothrow) AAFwk::Array(1, AAFwk::g_IID_IString);
    if (ao2 != nullptr) {
        ao2->Set(0, String::Box("file://data/storage/el2/distributedfiles/test.txt"));
        params.SetParam("ability.verify.uri", ao2);
    }
    want.SetParams(params);
    UriUtils::GetInstance().GrantDmsUriPermission(want, callerTokenId, targetBundleName, appIndex);
    bool res = want.GetParams().HasParam("ability.verify.uri");
    EXPECT_EQ(res, true);
}

/*
 * Feature: UriUtils
 * Function: GrantShellUriPermission
 * SubFunction: NA
 * FunctionPoints: UriUtils GrantShellUriPermission
 */
HWTEST_F(UriUtilsTest, GrantShellUriPermission_001, TestSize.Level1)
{
    std::vector<std::string> strUriVec = {"file://data/storage/el2/distributedfiles/test.txt"};
    uint32_t flag = 0;
    std::string targetPkg;
    int32_t appIndex = 101;
    bool res0 = UriUtils::GetInstance().GrantShellUriPermission(strUriVec, flag, targetPkg, appIndex);
    EXPECT_EQ(res0, false);

    strUriVec[0] = "content://data/storage/el2/distributedfiles/test.txt";
    bool res1 = UriUtils::GetInstance().GrantShellUriPermission(strUriVec, flag, targetPkg, appIndex);
    EXPECT_EQ(res1, true);
}

/*
 * Feature: UriUtils
 * Function: CheckUriPermission
 * SubFunction: NA
 * FunctionPoints: UriUtils CheckUriPermission
 */
HWTEST_F(UriUtilsTest, CheckUriPermission_001, TestSize.Level1)
{
    AccessTokenKit::hapInfo.apiVersion = 20;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::getHapTokenInfoRet = 0;
    uint32_t callerTokenId = 1;
    Want want;
    want.SetFlags(0x00000003);

    WantParams params;
    sptr<AAFwk::IArray> ao = new (std::nothrow) AAFwk::Array(BEYOND_MAX_URI_COUNT, AAFwk::g_IID_IString);
    if (ao != nullptr) {
        for (size_t i = 0; i < BEYOND_MAX_URI_COUNT; i++) {
            ao->Set(i, String::Box("file"));
        }
        params.SetParam("ability.params.stream", ao);
    }
    want.SetParams(params);
    UriUtils::GetInstance().CheckUriPermission(callerTokenId, want);
    sptr<IInterface> value = want.GetParams().GetParam("ability.params.stream");
    IArray *arr = IArray::Query(value);
    long arrSize = 0;
    if (arr != nullptr && Array::IsStringArray(arr)) {
        arr->GetLength(arrSize);
    }
    EXPECT_EQ(arrSize, 0);
}

/*
 * Feature: UriUtils
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: UriUtils GrantUriPermission
 */
HWTEST_F(UriUtilsTest, GrantUriPermission_001, TestSize.Level1)
{
    Want want;
    std::string targetBundleName = "";
    int32_t appIndex = 101;
    bool isSandboxApp = false;
    int32_t callerTokenId = 0;
    int32_t collaboratorType = 2;
    want.SetFlags(0x00000003);
    UriUtils::GetInstance().GrantUriPermission(want, targetBundleName, appIndex, isSandboxApp, callerTokenId,
        collaboratorType);

    want.SetFlags(0x00000001);
    UriUtils::GetInstance().GrantUriPermission(want, targetBundleName, appIndex, isSandboxApp, callerTokenId,
        collaboratorType);

    targetBundleName = "com.example.tsapplication";
    UriUtils::GetInstance().GrantUriPermission(want, targetBundleName, appIndex, isSandboxApp, callerTokenId,
        collaboratorType);

    callerTokenId = 1001;
    UriUtils::GetInstance().GrantUriPermission(want, targetBundleName, appIndex, isSandboxApp, callerTokenId,
        collaboratorType);

    WantParams params;
    sptr<AAFwk::IArray> ao = new (std::nothrow) AAFwk::Array(1, AAFwk::g_IID_IString);
    if (ao != nullptr) {
        ao->Set(0, String::Box("file"));
        params.SetParam("ability.params.stream", ao);
    }
    want.SetParams(params);
    UriUtils::GetInstance().GrantUriPermission(want, targetBundleName, appIndex, isSandboxApp, callerTokenId,
        collaboratorType);

    want.SetUri("file://data/storage/el2/distributedfiles/test.txt");
    UriUtils::GetInstance().GrantUriPermission(want, targetBundleName, appIndex, isSandboxApp, callerTokenId,
        collaboratorType);

    std::string bundleName = AppUtils::GetInstance().GetBrokerDelegateBundleName();
    EXPECT_EQ(bundleName.empty(), true);
}

/*
 * Feature: UriUtils
 * Function: GrantUriPermissionInner
 * SubFunction: NA
 * FunctionPoints: UriUtils GrantUriPermissionInner
 */
HWTEST_F(UriUtilsTest, GrantUriPermissionInner_001, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://data/storage/el2/distributedfiles/test.txt"};
    uint32_t callerTokenId = 0;
    std::string targetBundleName = "com.example.tsapplication";
    int32_t appIndex = 0;
    Want want;
    bool res = UriUtils::GetInstance().GrantUriPermissionInner(uriVec, callerTokenId, targetBundleName, appIndex, want);
    EXPECT_EQ(res, false);
}
#endif // SUPPORT_UPMS

/*
 * Feature: UriUtils
 * Function: IsSandboxApp
 * SubFunction: NA
 * FunctionPoints: UriUtils IsSandboxApp
 */
HWTEST_F(UriUtilsTest, IsSandboxApp_001, TestSize.Level1)
{
    uint32_t tokenId = 0;
    bool res = UriUtils::GetInstance().IsSandboxApp(tokenId);
    EXPECT_EQ(res, false);

    tokenId = 1001;
    res = UriUtils::GetInstance().IsSandboxApp(tokenId);
    EXPECT_EQ(res, false);
}

/*
 * Feature: UriUtils
 * Function: GrantUriPermissionForServiceExtension
 * SubFunction: NA
 * FunctionPoints: UriUtils GrantUriPermissionForServiceExtension
 */
HWTEST_F(UriUtilsTest, GrantUriPermissionForServiceExtension_001, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::FORM;
    UriUtils::GetInstance().GrantUriPermissionForServiceExtension(abilityRequest);

    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    UriUtils::GetInstance().GrantUriPermissionForServiceExtension(abilityRequest);
    EXPECT_EQ(abilityRequest.abilityInfo.extensionAbilityType, AppExecFwk::ExtensionAbilityType::SERVICE);
}

/*
 * Feature: UriUtils
 * Function: GrantUriPermissionForUIOrServiceExtension
 * SubFunction: NA
 * FunctionPoints: UriUtils GrantUriPermissionForUIOrServiceExtension
 */
HWTEST_F(UriUtilsTest, GrantUriPermissionForUIOrServiceExtension_001, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::FORM;
    UriUtils::GetInstance().GrantUriPermissionForUIOrServiceExtension(abilityRequest);

    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    UriUtils::GetInstance().GrantUriPermissionForUIOrServiceExtension(abilityRequest);
    EXPECT_EQ(abilityRequest.abilityInfo.extensionAbilityType, AppExecFwk::ExtensionAbilityType::SERVICE);
}

/*
 * Feature: UriUtils
 * Function: CheckIsInAncoAppIdentifier
 * SubFunction: NA
 * FunctionPoints: UriUtils CheckIsInAncoAppIdentifier
 */
HWTEST_F(UriUtilsTest, CheckIsInAncoAppIdentifier_001, TestSize.Level1)
{
    std::string identifier = "";
    std::string bundleName = "";
    auto ret = UriUtils::GetInstance().CheckIsInAncoAppIdentifier(identifier, bundleName);
    EXPECT_EQ(ret, false);

    identifier = "com.example.test";
    bundleName = "";
    ret = UriUtils::GetInstance().CheckIsInAncoAppIdentifier(identifier, bundleName);
    EXPECT_EQ(ret, false);

    identifier = "com.example.test1|com.example.test2";
    bundleName = "com.example.test";
    ret = UriUtils::GetInstance().CheckIsInAncoAppIdentifier(identifier, bundleName);
    EXPECT_EQ(ret, false);

    identifier = "com.example.test";
    bundleName = "com.example.test";
    ret = UriUtils::GetInstance().CheckIsInAncoAppIdentifier(identifier, bundleName);
    EXPECT_EQ(ret, true);

    identifier = "com.example.test|com.example.test1";
    bundleName = "com.example.test";
    ret = UriUtils::GetInstance().CheckIsInAncoAppIdentifier(identifier, bundleName);
    EXPECT_EQ(ret, true);
}

/*
 * Feature: UriUtils
 * Function: ProcessUDMFKey
 * SubFunction: NA
 * FunctionPoints: UriUtils ProcessUDMFKey
 */
HWTEST_F(UriUtilsTest, ProcessUDMFKey_001, TestSize.Level1)
{
    auto &uriUtils = UriUtils::GetInstance();
    Want want;
    std::string udKey = "udmf:tempKey";
    want.SetParam(Want::PARAM_ABILITY_UNIFIED_DATA_KEY, udKey);
    uriUtils.ProcessUDMFKey(want);
    EXPECT_EQ(want.GetStringParam(Want::PARAM_ABILITY_UNIFIED_DATA_KEY).empty(), false);

    std::vector<std::string> uris = { "file://com.example.test/aaa.txt" };
    want.SetParam(ABILIY_PARAM_STREAM, uris);
    uriUtils.ProcessUDMFKey(want);
    EXPECT_EQ(want.GetStringParam(Want::PARAM_ABILITY_UNIFIED_DATA_KEY).empty(), true);
}

/*
 * Feature: UriUtils
 * Function: ProcessUDMFKey
 * SubFunction: NA
 * FunctionPoints: UriUtils ProcessUDMFKey
 */
HWTEST_F(UriUtilsTest, ProcessUDMFKey_002, TestSize.Level1)
{
    auto &uriUtils = UriUtils::GetInstance();
    Want want;
    std::string udKey;
    want.SetParam(Want::PARAM_ABILITY_UNIFIED_DATA_KEY, udKey);
    uriUtils.ProcessUDMFKey(want);
    EXPECT_EQ(want.GetStringParam(Want::PARAM_ABILITY_UNIFIED_DATA_KEY).empty(), true);

    std::vector<std::string> uris = { "file://com.example.test/aaa.txt" };
    want.SetParam(ABILIY_PARAM_STREAM, uris);
    uriUtils.ProcessUDMFKey(want);
    EXPECT_EQ(want.GetStringParam(Want::PARAM_ABILITY_UNIFIED_DATA_KEY).empty(), true);
}

/*
 * Feature: UriUtils
 * Function: IsInAncoAppIdentifier
 * SubFunction: NA
 * FunctionPoints: UriUtils IsInAncoAppIdentifier
 */
HWTEST_F(UriUtilsTest, IsInAncoAppIdentifier_001, TestSize.Level1)
{
    auto &uriUtils = UriUtils::GetInstance();
    std::string uri = "";
    Want want;
    want.SetUri(uri);
    uriUtils.PublishFileOpenEvent(want);
    auto result = uriUtils.IsInAncoAppIdentifier("com.example.test");
    EXPECT_FALSE(result);
}

/*
 * Feature: UriUtils
 * Function: IsInAncoAppIdentifier
 * SubFunction: NA
 * FunctionPoints: UriUtils IsInAncoAppIdentifier
 */
HWTEST_F(UriUtilsTest, IsInAncoAppIdentifier_002, TestSize.Level1)
{
    auto &uriUtils = UriUtils::GetInstance();
    std::string uri = "file://com.example.test/test.txt";
    Want want;
    want.SetUri(uri);
    uriUtils.PublishFileOpenEvent(want);
    auto result = uriUtils.IsInAncoAppIdentifier("com.example.test");
    EXPECT_FALSE(result);
}

/*
 * Feature: UriUtils
 * Function: GetCallerNameAndApiVersion
 * SubFunction: NA
 * FunctionPoints: token native type, get native info failed.
 */
HWTEST_F(UriUtilsTest, GetCallerNameAndApiVersion_001, TestSize.Level1)
{
    auto &uriUtils = UriUtils::GetInstance();
    uint32_t tokenId = 0;
    std::string callerName;
    int32_t apiVersion = 0;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_NATIVE;
    AccessTokenKit::getNativeTokenInfoRet = -1;
    auto ret = uriUtils.GetCallerNameAndApiVersion(tokenId, callerName, apiVersion);
    EXPECT_EQ(ret, false);
}

/*
 * Feature: UriUtils
 * Function: GetCallerNameAndApiVersion
 * SubFunction: NA
 * FunctionPoints: token native type, get native info success.
 */
HWTEST_F(UriUtilsTest, GetCallerNameAndApiVersion_002, TestSize.Level1)
{
    auto &uriUtils = UriUtils::GetInstance();
    uint32_t tokenId = 0;
    std::string callerName;
    int32_t apiVersion = 0;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_NATIVE;
    AccessTokenKit::getNativeTokenInfoRet = 0;
    AccessTokenKit::nativeTokenInfo.processName = "caller";
    auto ret = uriUtils.GetCallerNameAndApiVersion(tokenId, callerName, apiVersion);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(callerName, "caller");
}

/*
 * Feature: UriUtils
 * Function: GetCallerNameAndApiVersion
 * SubFunction: NA
 * FunctionPoints: token hap type, get hap info failed.
 */
HWTEST_F(UriUtilsTest, GetCallerNameAndApiVersion_003, TestSize.Level1)
{
    auto &uriUtils = UriUtils::GetInstance();
    uint32_t tokenId = 0;
    std::string callerName;
    int32_t apiVersion = 0;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::getHapTokenInfoRet = -1;
    auto ret = uriUtils.GetCallerNameAndApiVersion(tokenId, callerName, apiVersion);
    EXPECT_EQ(ret, false);
}

/*
 * Feature: UriUtils
 * Function: GetCallerNameAndApiVersion
 * SubFunction: NA
 * FunctionPoints: token hap type, get hap info success.
 */
HWTEST_F(UriUtilsTest, GetCallerNameAndApiVersion_004, TestSize.Level1)
{
    auto &uriUtils = UriUtils::GetInstance();
    uint32_t tokenId = 0;
    std::string callerName;
    int32_t apiVersion = 0;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_HAP;
    AccessTokenKit::getHapTokenInfoRet = 0;
    AccessTokenKit::hapInfo.bundleName = "caller";
    AccessTokenKit::hapInfo.apiVersion = 20;
    auto ret = uriUtils.GetCallerNameAndApiVersion(tokenId, callerName, apiVersion);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(callerName, "caller");
    EXPECT_EQ(apiVersion, 20);
}

/*
 * Feature: UriUtils
 * Function: GetCallerNameAndApiVersion
 * SubFunction: NA
 * FunctionPoints: invalid token type.
 */
HWTEST_F(UriUtilsTest, GetCallerNameAndApiVersion_005, TestSize.Level1)
{
    auto &uriUtils = UriUtils::GetInstance();
    uint32_t tokenId = 0;
    std::string callerName;
    int32_t apiVersion = 0;
    AccessTokenKit::getTokenTypeFlagRet = ATokenTypeEnum::TOKEN_INVALID;
    auto ret = uriUtils.GetCallerNameAndApiVersion(tokenId, callerName, apiVersion);
    EXPECT_EQ(ret, false);
}

/*
 * Feature: UriUtils
 * Function: SendGrantUriPermissionEvent
 * SubFunction: NA
 * FunctionPoints: UriUtils SendGrantUriPermissionEvent
 */
HWTEST_F(UriUtilsTest, SendGrantUriPermissionEvent_001, TestSize.Level1)
{
    auto &uriUtils = UriUtils::GetInstance();
    std::string callerBundleName = "caller";
    std::string targetBundleName = "target";
    std::string oriUri = "file://caller/1.txt";
    int32_t apiVersion = 20;
    std::string eventType = "eraseUri";
    auto ret = uriUtils.SendGrantUriPermissionEvent(callerBundleName, targetBundleName, oriUri, apiVersion, eventType);
    EXPECT_EQ(ret, true);
}

/*
 * Feature: UriUtils
 * Function: ProcessWantUri
 * SubFunction: NA
 * FunctionPoints: ProcessWantUri test.
 */
HWTEST_F(UriUtilsTest, ProcessWantUri_001, TestSize.Level1)
{
    auto &uriUtils = UriUtils::GetInstance();
    Want want;
    bool checkResult = true;
    int32_t apiVersion = 20;
    std::vector<Uri> permissionedUri;
    auto ret = uriUtils.ProcessWantUri(checkResult, apiVersion, want, permissionedUri);
    EXPECT_EQ(ret, true);

    want.SetUri("file://caller/1.txt");
    ret = uriUtils.ProcessWantUri(checkResult, apiVersion, want, permissionedUri);
    EXPECT_EQ(ret, true);

    checkResult = false;
    // scheme is file
    ret = uriUtils.ProcessWantUri(checkResult, apiVersion, want, permissionedUri);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(want.GetUriString().empty(), true);

    want.SetUri("invalidUri");
    ret = uriUtils.ProcessWantUri(checkResult, apiVersion, want, permissionedUri);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(want.GetUriString().empty(), true);

    want.SetUri("invalidUri");
    apiVersion = 1;
    ret = uriUtils.ProcessWantUri(checkResult, apiVersion, want, permissionedUri);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(want.GetUriString().empty(), false);
}
}
}