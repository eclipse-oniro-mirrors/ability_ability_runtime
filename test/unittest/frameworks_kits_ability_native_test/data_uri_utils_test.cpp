/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <vector>
#define private public
#define protected public
#include "data_uri_utils.h"
#include "uri.h"
#undef private
#undef protected
namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
namespace {
    const int32_t THOUSAND = 1000;
    const int32_t NEGATIVE = -1;
}
class DataUriUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::unique_ptr<DataUriUtils> data_uri_util_ = nullptr;
};

void DataUriUtilsTest::SetUpTestCase(void)
{}

void DataUriUtilsTest::TearDownTestCase(void)
{}

void DataUriUtilsTest::SetUp()
{
    data_uri_util_ = std::make_unique<DataUriUtils>();
}

void DataUriUtilsTest::TearDown()
{}

/**
 * @tc.number: AaFwk_DataUriUtils_AttachId_GetId_0100
 * @tc.name: AttachId/GetId
 * @tc.desc: Test if attachd and getid return values are correct.
 */
HWTEST_F(DataUriUtilsTest, AaFwk_DataUriUtils_AttachId_Get001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataUriUtils_001 start";

    Uri uri("scheme://authority/path1/path2/path3?id = 1&name = mingming&old#fragment");
    Uri uriRet1 = DataUriUtils::AttachId(uri, 1000);

    long long ret1 = DataUriUtils::GetId(uriRet1);
    EXPECT_EQ(ret1, 1000);

    GTEST_LOG_(INFO) << "AaFwk_DataUriUtils_001 end";
}

/**
 * @tc.number: AaFwk_DataUriUtils_AttachId_GetId_0100
 * @tc.name: AttachId/DeleteId/IsAttachedId
 * @tc.desc: Test whether the return values of attachid, deleteid and isattachedidare correct.
 */
HWTEST_F(DataUriUtilsTest, AaFwk_DataUriUtils_DeleteId_IsAttachedId001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataUriUtils_DeleteId_IsAttachedId001 start";

    Uri uri("scheme://authority/path1/path2/path3?id = 1&name = mingming&old#fragment");
    Uri uriRet1 = DataUriUtils::AttachId(uri, 1000);

    Uri uriRet2 = DataUriUtils::DeleteId(uriRet1);

    bool ret2 = DataUriUtils::IsAttachedId(uriRet2);
    EXPECT_EQ(ret2, false);

    GTEST_LOG_(INFO) << "AaFwk_DataUriUtils_DeleteId_IsAttachedId001 end";
}

/**
 * @tc.number: AaFwk_DataUriUtils_DeleteId_IsAttachedId002
 * @tc.name: AttachId/DeleteId/IsAttachedId
 * @tc.desc: Test whether the return values of attachid, deleteid and isattachedidare correct.
 */
HWTEST_F(DataUriUtilsTest, AaFwk_DataUriUtils_DeleteId_IsAttachedId002, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataUriUtils_DeleteId_IsAttachedId002 start";

    Uri uri("scheme://authority/path1/path2/path3?id = 1&name = mingming&old#fragment");
    Uri uriRet1 = DataUriUtils::AttachId(uri, -1000);
    Uri uriRet2 = DataUriUtils::DeleteId(uriRet1);
    bool ret2 = DataUriUtils::IsAttachedId(uriRet2);
    EXPECT_EQ(ret2, false);

    GTEST_LOG_(INFO) << "AaFwk_DataUriUtils_DeleteId_IsAttachedId002 end";
}
/**
 * @tc.number: AaFwk_DataUriUtils_DeleteId_IsAttachedId003
 * @tc.name: AttachId/DeleteId/IsAttachedId
 * @tc.desc: Test whether the return values of attachid, deleteid and isattachedidare correct.
 */
HWTEST_F(DataUriUtilsTest, AaFwk_DataUriUtils_DeleteId_IsAttachedId003, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataUriUtils_DeleteId_IsAttachedId003 start";

    Uri uri("scheme://authority/path1/path2/path3?id = 1&name = mingming&old#fragment");
    Uri uriRet1 = DataUriUtils::AttachId(uri, 123456789011);

    long long id = DataUriUtils::GetId(uriRet1);
    EXPECT_EQ(id, 123456789011);
    Uri uriRet2 = DataUriUtils::DeleteId(uriRet1);

    bool ret2 = DataUriUtils::IsAttachedId(uriRet2);
    EXPECT_EQ(ret2, false);

    GTEST_LOG_(INFO) << "AaFwk_DataUriUtils_DeleteId_IsAttachedId003 end";
}
/**
 * @tc.number: AaFwk_DataUriUtils_AttachIdUpdateId_0100
 * @tc.name: AttachId/UpdateId/GetId
 * @tc.desc: Test whether the return values of attachid, updateid and getid are correct.
 */
HWTEST_F(DataUriUtilsTest, AaFwk_DataUriUtils_AttachIdUpdateId001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataUriUtils_AttachIdUpdateId001 start";

    Uri uri("scheme://authority/path1/path2/path3?id = 1&name = mingming&old#fragment");
    // case 3
    Uri uriRet3 = DataUriUtils::AttachId(uri, 100);
    Uri uriRet4 = DataUriUtils::UpdateId(uriRet3, 800);
    long ret4Id = DataUriUtils::GetId(uriRet4);

    EXPECT_EQ(ret4Id, 800);

    GTEST_LOG_(INFO) << "AaFwk_DataUriUtils_AttachIdUpdateId001 end";
}

/**
 * @tc.number: DataUriUtilsTest_0100
 * @tc.name: DataUriUtilsTest
 * @tc.desc: Test Function DataUriUtils::AttachId
 */
HWTEST_F(DataUriUtilsTest, DataUriUtilsTest_0100, Level1)
{
    GTEST_LOG_(INFO) << "DataUriUtilsTest_0100 start";
    DataUriUtils dataUriUtils;
    Uri uri1("");
    string path = const_cast<Uri &>(uri1).GetPath();
    EXPECT_TRUE(path.empty());
    dataUriUtils.AttachId(uri1, THOUSAND);
    Uri uri2("scheme://authority/");
    std::vector<string> pathVector;
    const_cast<Uri &>(uri2).GetPathSegments(pathVector);
    EXPECT_TRUE(pathVector.empty());
    dataUriUtils.AttachId(uri2, THOUSAND);
    GTEST_LOG_(INFO) << "DataUriUtilsTest_0100 end";
}

/**
 * @tc.number: DataUriUtilsTest_0200
 * @tc.name: DataUriUtilsTest
 * @tc.desc: Test Function DataUriUtils::GetId
 */
HWTEST_F(DataUriUtilsTest, DataUriUtilsTest_0200, Level1)
{
    GTEST_LOG_(INFO) << "DataUriUtilsTest_0200 start";
    DataUriUtils dataUriUtils;
    Uri uri1("");
    EXPECT_EQ(dataUriUtils.GetId(uri1), NEGATIVE);
    Uri uri2("scheme://authority/");
    EXPECT_EQ(dataUriUtils.GetId(uri2), NEGATIVE);
    Uri uri3("scheme://authority/path1/path2/");
    EXPECT_EQ(dataUriUtils.GetId(uri3), NEGATIVE);
    GTEST_LOG_(INFO) << "DataUriUtilsTest_0200 end";
}

/**
 * @tc.number: DataUriUtilsTest_0300
 * @tc.name: DataUriUtilsTest
 * @tc.desc: Test Function DataUriUtils::IsAttachedId
 */
HWTEST_F(DataUriUtilsTest, DataUriUtilsTest_0300, Level1)
{
    GTEST_LOG_(INFO) << "DataUriUtilsTest_0300 start";
    DataUriUtils dataUriUtils;
    Uri uri1("");
    EXPECT_FALSE(dataUriUtils.IsAttachedId(uri1));
    Uri uri2("scheme://authority/");
    EXPECT_FALSE(dataUriUtils.IsAttachedId(uri2));
    GTEST_LOG_(INFO) << "DataUriUtilsTest_0300 end";
}

/**
 * @tc.number: DataUriUtilsTest_0400
 * @tc.name: DataUriUtilsTest
 * @tc.desc: Test Function DataUriUtils::UriUpateLastPath
 */
HWTEST_F(DataUriUtilsTest, DataUriUtilsTest_0400, Level1)
{
    GTEST_LOG_(INFO) << "DataUriUtilsTest_0400 start";
    DataUriUtils dataUriUtils;
    const std::string empty = "";
    Uri uri1("");
    string path = const_cast<Uri &>(uri1).GetPath();
    EXPECT_TRUE(path.empty());
    dataUriUtils.UriUpateLastPath(uri1, empty);
    Uri uri2("scheme://authority/");
    std::vector<string> pathVector;
    const_cast<Uri &>(uri2).GetPathSegments(pathVector);
    EXPECT_TRUE(pathVector.empty());
    dataUriUtils.UriUpateLastPath(uri2, empty);
    Uri uri3("scheme://authority/path1/path2/");
    const_cast<Uri &>(uri3).GetPathSegments(pathVector);
    string lastPath = pathVector[pathVector.size() - 1];
    EXPECT_TRUE(!(dataUriUtils.IsNumber(lastPath)));
    dataUriUtils.UriUpateLastPath(uri3, empty);
    GTEST_LOG_(INFO) << "DataUriUtilsTest_0400 end";
}

/**
 * @tc.number: DataUriUtilsTest_0500
 * @tc.name: DataUriUtilsTest
 * @tc.desc: Test floating-point path segments are not treated as integer ids.
 */
HWTEST_F(DataUriUtilsTest, DataUriUtilsTest_0500, Level1)
{
    GTEST_LOG_(INFO) << "DataUriUtilsTest_0500 start";
    DataUriUtils dataUriUtils;
    Uri uri("scheme://authority/path1/path2/12.5?id = 1&name = mingming&old#fragment");

    EXPECT_FALSE(dataUriUtils.IsNumber("12.5"));
    EXPECT_FALSE(dataUriUtils.IsNumber("+0.0"));
    EXPECT_FALSE(dataUriUtils.IsNumber("-1.0"));
    EXPECT_FALSE(dataUriUtils.IsAttachedId(uri));
    EXPECT_EQ(dataUriUtils.GetId(uri), NEGATIVE);
    EXPECT_EQ(dataUriUtils.DeleteId(uri).ToString(), uri.ToString());
    EXPECT_EQ(dataUriUtils.UpdateId(uri, THOUSAND).ToString(), uri.ToString());

    GTEST_LOG_(INFO) << "DataUriUtilsTest_0500 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
