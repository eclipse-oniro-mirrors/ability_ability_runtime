/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "implicit_start_processor.h"
#include "operation.h"
#undef private
#undef protected
#include "ability_ecological_rule_mgr_service_param.h"
#include "hilog_tag_wrapper.h"
#include "parameters.h"
#include "mock_parameters.h"
#include "mock_bundle_mgr_helper_status.h"
#include "mock_multi_app_utils_status.h"

using namespace OHOS;
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
namespace {
constexpr const char* SUPPORT_ACTION_START_SELECTOR = "persist.sys.ability.support.action_start_selector";
const std::string PARAM_ABILITY_APPINFOS = "ohos.ability.params.appInfos";
const std::string OPEN_LINK_APP_LINKING_ONLY = "appLinkingOnly";
const int NFC_CALLER_UID = 1027;
const std::string BUNDLE_NAME = "test_bundle";
const std::string NAME = "test_name";
const size_t TRUSTLIST_MAX_SIZE = 50;
const std::string APP_LAUNCH_TRUSTLIST = "ohos.params.appLaunchTrustList";
}
namespace OHOS {
namespace AAFwk {

const size_t IDENTITY_LIST_MAX_SIZE = 10;
class ImplicitStartProcessorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ImplicitStartProcessorTest::SetUpTestCase(void)
{}
void ImplicitStartProcessorTest::TearDownTestCase(void)
{}
void ImplicitStartProcessorTest::SetUp()
{
    MockBundleMgrHelperStatus::Reset();
    MockMultiAppUtilsStatus::Reset();
}
void ImplicitStartProcessorTest::TearDown()
{}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbility
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbility
 * EnvConditions: NA
 * CaseDescription: Verify ImplicitStartAbility; isAppCloneSelector = true
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ImplicitStartAbility_001 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    int32_t userId = 0;
    bool res = processor->ImplicitStartAbility(request, userId,
        AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED, "", true);
    EXPECT_TRUE(res);
    TAG_LOGI(AAFwkTag::TEST, "ImplicitStartAbility_001 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbility
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbility
 * EnvConditions: NA
 * CaseDescription: 无trustlist; isAppCloneSelector = false
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ImplicitStartAbility_002 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    int32_t userId = 0;
    std::string mockAction = "action.system.home";
    request.want.SetAction(mockAction);
    bool res = processor->ImplicitStartAbility(request, userId,
        AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED, "", false);
    EXPECT_TRUE(res);
    TAG_LOGI(AAFwkTag::TEST, "ImplicitStartAbility_002 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbility
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbility
 * EnvConditions: NA
 * CaseDescription: 有trustlist; isAppCloneSelector = true; 不进入交集处理
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbility_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ImplicitStartAbility_003 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    int32_t userId = 0;
    std::string mockAction = "action.system.home";
    std::vector<std::string> mockTrustlist = {
        "com.example.hmos.advisor",
        "com.example.hmos.calculator",
        "com.example.hmos.databackup",
    };
    request.want.SetAction(mockAction);
    request.want.SetParam(APP_LAUNCH_TRUSTLIST, mockTrustlist);
    bool res = processor->ImplicitStartAbility(request, userId,
        AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED, "", true);
    EXPECT_TRUE(res);
    TAG_LOGI(AAFwkTag::TEST, "ImplicitStartAbility_003 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbility
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbility
 * EnvConditions: NA
 * CaseDescription: 有trustlist; SIZE超过50; isAppCloneSelector = false;
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbility_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ImplicitStartAbility_004 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    int32_t userId = 0;
    std::string mockAction = "action.system.home";
    std::vector<std::string> mockTrustlist = {
        "aaa", "bbb", "ccc", "ddd", "eee",
        "anfa", "bfb", "cnc", "ddm", "mje",
        "asra", "btr", "chtc", "djy", "essse",
        "bfgb", "gbgbb", "nbbbh", "ttrtry", "eergrge",
        "aaqwea", "bqwebb", "ccretc", "ddfghd", "enghee",
        "aretgaa", "fgdbbb", "ccdfgc", "ddgegrd", "ebggggee",
        "allaa", "bllbb", "ccllc", "llddd", "ellee",
        "aapa", "bbpb", "cppcc", "ddpd", "epee",
        "aawsxca", "bbvvvb", "cbvcc", "dbvdd", "ebvee",
        "areaa", "bbreb", "crecc",
        "com.example.hmos.advisor",
        "com.example.hmos.calculator",
        "com.example.hmos.databackup",
    };
    request.want.SetAction(mockAction);
    request.want.SetParam(APP_LAUNCH_TRUSTLIST, mockTrustlist);
    bool res = processor->ImplicitStartAbility(request, userId,
        AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED, "", false);
    EXPECT_TRUE(res);
    TAG_LOGI(AAFwkTag::TEST, "ImplicitStartAbility_004 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbility
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbility
 * EnvConditions: NA
 * CaseDescription: 有trustlist; 交集为0; isAppCloneSelector = false;
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbility_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ImplicitStartAbility_005 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    int32_t userId = 0;
    std::string mockAction = "action.system.home";
    std::vector<std::string> mockTrustlist = {""};
    request.want.SetAction(mockAction);
    request.want.SetParam(APP_LAUNCH_TRUSTLIST, mockTrustlist);
    bool res = processor->ImplicitStartAbility(request, userId,
        AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED, "", false);
    EXPECT_TRUE(res);
    TAG_LOGI(AAFwkTag::TEST, "ImplicitStartAbility_005 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbility
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbility
 * EnvConditions: NA
 * CaseDescription: 有trustlist; 交集为1; scheme 为 file;
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbility_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ImplicitStartAbility_006 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    int32_t userId = 0;
    std::string mockUriString = "file://com.example.test/test.txt";
    std::string mockAction = "action.system.home";
    std::vector<std::string> mockTrustlist = {
        "com.example.hmos.advisor",
        "demo1",
    };
    request.want.SetUri(mockUriString);
    request.want.SetAction(mockAction);
    request.want.SetParam(APP_LAUNCH_TRUSTLIST, mockTrustlist);
    bool res = processor->ImplicitStartAbility(request, userId,
        AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED, "", false);
    EXPECT_TRUE(res);
    TAG_LOGI(AAFwkTag::TEST, "ImplicitStartAbility_006 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbility
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbility
 * EnvConditions: NA
 * CaseDescription: 有trustlist; 非法输入
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbility_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ImplicitStartAbility_007 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    int32_t userId = 0;
    std::string mockAction = "action.system.home";
    std::string mockTrustlist = "demo1";
    request.want.SetAction(mockAction);
    request.want.SetParam(APP_LAUNCH_TRUSTLIST, mockTrustlist);
    bool res = processor->ImplicitStartAbility(request, userId,
        AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED, "", false);
    EXPECT_TRUE(res);
    TAG_LOGI(AAFwkTag::TEST, "ImplicitStartAbility_007 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbility
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbility
 * EnvConditions: NA
 * CaseDescription: 测试dialogAppInfos.size() == 0且设置了FLAG_START_WITHOUT_TIPS的场景
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbility_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ImplicitStartAbility_008 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    int32_t userId = 0;
    std::string mockAction = "action.system.home";
    request.want.SetAction(mockAction);
    // Set FLAG_START_WITHOUT_TIPS flag to trigger the specific code path
    request.want.SetFlags(Want::FLAG_START_WITHOUT_TIPS);
    int32_t res = processor->ImplicitStartAbility(request, userId,
        AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED, "", false);
    EXPECT_EQ(res, ERR_IMPLICIT_START_ABILITY_FAIL);
    TAG_LOGI(AAFwkTag::TEST, "ImplicitStartAbility_008 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbility
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor ImplicitStartAbility
 * EnvConditions: NA
 * CaseDescription: dialogAppInfos.size() == 0 and !IsSelectorDialogDefaultPossion()
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbility_009, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ImplicitStartAbility_009 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    int32_t userId = 0;
    std::string mockAction = "action.system.home";
    request.want.SetAction(mockAction);
    request.want.SetParam("isCreateAppGallerySelector", true);
    request.hideFailureTipDialog = false;

    int32_t res = processor->ImplicitStartAbility(request, userId,
        AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED, "", false);
    EXPECT_EQ(res, ERR_IMPLICIT_START_ABILITY_FAIL);

    request.hideFailureTipDialog = true;
    res = processor->ImplicitStartAbility(request, userId,
        AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED, "", false);
    EXPECT_EQ(res, ERR_IMPLICIT_START_ABILITY_FAIL);
    TAG_LOGI(AAFwkTag::TEST, "ImplicitStartAbility_009 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: TrustlistIntersectionProcess
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor TrustlistIntersectionProcess
 * EnvConditions: NA
 * CaseDescription: scheme 为 file; early return; dialogAppInfos unchanged.
 */
HWTEST_F(ImplicitStartProcessorTest, TrustlistIntersectionProcess_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TrustlistIntersectionProcess_001 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();

    int32_t userId = 0;
    AbilityRequest request;
    std::string mockUriString = "file://com.example.test/test.txt";
    std::vector<std::string> mockTrustlist = {
        "com.example.hmos.advisor",
        "demo1",
    };
    request.want.SetUri(mockUriString);
    request.want.SetParam(APP_LAUNCH_TRUSTLIST, mockTrustlist);

    std::vector<DialogAppInfo> dialogAppInfos;
    DialogAppInfo dialogAppInfo1;
    dialogAppInfo1.bundleName = "demo1";
    dialogAppInfos.emplace_back(dialogAppInfo1);
    DialogAppInfo dialogAppInfo2;
    dialogAppInfo2.bundleName = "demo2";
    dialogAppInfos.emplace_back(dialogAppInfo2);

    int32_t infosOldSize = static_cast<int32_t>(dialogAppInfos.size());
    TAG_LOGI(AAFwkTag::TEST, "infosOldSize: %{public}d", infosOldSize);

    processor->TrustlistIntersectionProcess(request, dialogAppInfos, userId);

    EXPECT_EQ(dialogAppInfos.size(), static_cast<size_t>(infosOldSize));
    EXPECT_EQ(dialogAppInfos.front().bundleName, "demo1");
    EXPECT_EQ(dialogAppInfos.back().bundleName, "demo2");
    TAG_LOGI(AAFwkTag::TEST, "TrustlistIntersectionProcess_001 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: TrustlistIntersectionProcess
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor TrustlistIntersectionProcess
 * EnvConditions: NA
 * CaseDescription: trustlist 为非数组(string); GetStringArrayParam 为空; early return; unchanged.
 */
HWTEST_F(ImplicitStartProcessorTest, TrustlistIntersectionProcess_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TrustlistIntersectionProcess_002 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();

    int32_t userId = 0;
    AbilityRequest request;
    std::string mockTrustlist = "demo1";
    request.want.SetParam(APP_LAUNCH_TRUSTLIST, mockTrustlist);

    std::vector<DialogAppInfo> dialogAppInfos;
    DialogAppInfo dialogAppInfo1;
    dialogAppInfo1.bundleName = "demo1";
    dialogAppInfos.emplace_back(dialogAppInfo1);
    DialogAppInfo dialogAppInfo2;
    dialogAppInfo2.bundleName = "demo2";
    dialogAppInfos.emplace_back(dialogAppInfo2);

    int32_t infosOldSize = static_cast<int32_t>(dialogAppInfos.size());
    TAG_LOGI(AAFwkTag::TEST, "infosOldSize: %{public}d", infosOldSize);

    processor->TrustlistIntersectionProcess(request, dialogAppInfos, userId);

    EXPECT_EQ(dialogAppInfos.size(), static_cast<size_t>(infosOldSize));
    TAG_LOGI(AAFwkTag::TEST, "TrustlistIntersectionProcess_002 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: TrustlistIntersectionProcess
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor TrustlistIntersectionProcess
 * EnvConditions: NA
 * CaseDescription: trustlist size>50 (trimmed locally to 50); appIdentifier of demo1 matches the
 *                 first trustlist entry; intersection=1; dialogAppInfos filtered to [demo1].
 *                 The want param itself is NOT modified (resize is on a local copy).
 */
HWTEST_F(ImplicitStartProcessorTest, TrustlistIntersectionProcess_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TrustlistIntersectionProcess_003 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();

    int32_t userId = 0;
    AbilityRequest request;
    std::vector<std::string> mockTrustlist;
    mockTrustlist.emplace_back("id_demo1");
    for (int i = 0; i < TRUSTLIST_MAX_SIZE; ++i) {
        mockTrustlist.emplace_back("junk_" + std::to_string(i));
    }
    request.want.SetParam(APP_LAUNCH_TRUSTLIST, mockTrustlist);
    MockBundleMgrHelperStatus::bundleAppIdentifierMap_["demo1"] = "id_demo1";
    MockBundleMgrHelperStatus::bundleAppIdentifierMap_["demo2"] = "id_demo2";

    std::vector<DialogAppInfo> dialogAppInfos;
    DialogAppInfo dialogAppInfo1;
    dialogAppInfo1.bundleName = "demo1";
    dialogAppInfos.emplace_back(dialogAppInfo1);
    DialogAppInfo dialogAppInfo2;
    dialogAppInfo2.bundleName = "demo2";
    dialogAppInfos.emplace_back(dialogAppInfo2);

    int32_t trustlistOldSize = static_cast<int32_t>(request.want.GetStringArrayParam(APP_LAUNCH_TRUSTLIST).size());
    TAG_LOGI(AAFwkTag::TEST, "trustlistOldSize: %{public}d", trustlistOldSize);

    processor->TrustlistIntersectionProcess(request, dialogAppInfos, userId);

    int32_t trustlistNewSize = static_cast<int32_t>(request.want.GetStringArrayParam(APP_LAUNCH_TRUSTLIST).size());
    TAG_LOGI(AAFwkTag::TEST, "trustlistNewSize: %{public}d, infosNewSize: %{public}zu",
        trustlistNewSize, dialogAppInfos.size());

    EXPECT_EQ(trustlistNewSize, trustlistOldSize);
    ASSERT_EQ(dialogAppInfos.size(), 1);
    EXPECT_EQ(dialogAppInfos.front().bundleName, "demo1");
    TAG_LOGI(AAFwkTag::TEST, "TrustlistIntersectionProcess_003 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: TrustlistIntersectionProcess
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor TrustlistIntersectionProcess
 * EnvConditions: NA
 * CaseDescription: GetBundleInfo succeeds but no appIdentifier matches trustlist; intersection=0;
 *                 dialogAppInfos cleared.
 */
HWTEST_F(ImplicitStartProcessorTest, TrustlistIntersectionProcess_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TrustlistIntersectionProcess_004 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();

    int32_t userId = 0;
    AbilityRequest request;
    std::vector<std::string> mockTrustlist = {
        "nonexistent_id",
    };
    request.want.SetParam(APP_LAUNCH_TRUSTLIST, mockTrustlist);
    MockBundleMgrHelperStatus::bundleAppIdentifierMap_["demo1"] = "id_demo1";
    MockBundleMgrHelperStatus::bundleAppIdentifierMap_["demo2"] = "id_demo2";

    std::vector<DialogAppInfo> dialogAppInfos;
    DialogAppInfo dialogAppInfo1;
    dialogAppInfo1.bundleName = "demo1";
    dialogAppInfos.emplace_back(dialogAppInfo1);
    DialogAppInfo dialogAppInfo2;
    dialogAppInfo2.bundleName = "demo2";
    dialogAppInfos.emplace_back(dialogAppInfo2);

    processor->TrustlistIntersectionProcess(request, dialogAppInfos, userId);

    EXPECT_EQ(dialogAppInfos.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "TrustlistIntersectionProcess_004 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: TrustlistIntersectionProcess
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor TrustlistIntersectionProcess
 * EnvConditions: NA
 * CaseDescription: intersection=1 (only demo1's appIdentifier matches); dialogAppInfos=[demo1].
 */
HWTEST_F(ImplicitStartProcessorTest, TrustlistIntersectionProcess_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TrustlistIntersectionProcess_005 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();

    int32_t userId = 0;
    AbilityRequest request;
    std::vector<std::string> mockTrustlist = {
        "id_demo1",
    };
    request.want.SetParam(APP_LAUNCH_TRUSTLIST, mockTrustlist);
    MockBundleMgrHelperStatus::bundleAppIdentifierMap_["demo1"] = "id_demo1";
    MockBundleMgrHelperStatus::bundleAppIdentifierMap_["demo2"] = "id_demo2";

    std::vector<DialogAppInfo> dialogAppInfos;
    DialogAppInfo dialogAppInfo1;
    dialogAppInfo1.bundleName = "demo1";
    dialogAppInfos.emplace_back(dialogAppInfo1);
    DialogAppInfo dialogAppInfo2;
    dialogAppInfo2.bundleName = "demo2";
    dialogAppInfos.emplace_back(dialogAppInfo2);

    processor->TrustlistIntersectionProcess(request, dialogAppInfos, userId);

    ASSERT_EQ(dialogAppInfos.size(), 1);
    EXPECT_EQ(dialogAppInfos.front().bundleName, "demo1");
    TAG_LOGI(AAFwkTag::TEST, "TrustlistIntersectionProcess_005 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: TrustlistIntersectionProcess
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor TrustlistIntersectionProcess
 * EnvConditions: NA
 * CaseDescription: intersection=2 (>1); dialogAppInfos NOT modified (keeps full list).
 */
HWTEST_F(ImplicitStartProcessorTest, TrustlistIntersectionProcess_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TrustlistIntersectionProcess_006 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();

    int32_t userId = 0;
    AbilityRequest request;
    std::vector<std::string> mockTrustlist = {
        "id_demo1",
        "id_demo2",
    };
    request.want.SetParam(APP_LAUNCH_TRUSTLIST, mockTrustlist);
    MockBundleMgrHelperStatus::bundleAppIdentifierMap_["demo1"] = "id_demo1";
    MockBundleMgrHelperStatus::bundleAppIdentifierMap_["demo2"] = "id_demo2";

    std::vector<DialogAppInfo> dialogAppInfos;
    DialogAppInfo dialogAppInfo1;
    dialogAppInfo1.bundleName = "demo1";
    dialogAppInfos.emplace_back(dialogAppInfo1);
    DialogAppInfo dialogAppInfo2;
    dialogAppInfo2.bundleName = "demo2";
    dialogAppInfos.emplace_back(dialogAppInfo2);

    processor->TrustlistIntersectionProcess(request, dialogAppInfos, userId);

    EXPECT_EQ(dialogAppInfos.size(), 2);
    EXPECT_EQ(dialogAppInfos.front().bundleName, "demo1");
    EXPECT_EQ(dialogAppInfos.back().bundleName, "demo2");
    TAG_LOGI(AAFwkTag::TEST, "TrustlistIntersectionProcess_006 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: CheckImplicitStartExtensionIsValid
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor CheckImplicitStartExtensionIsValid
 * EnvConditions: NA
 * CaseDescription: Verify CheckImplicitStartExtensionIsValid
 */
HWTEST_F(ImplicitStartProcessorTest, CheckImplicitStartExtensionIsValid_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    ExtensionAbilityInfo extensionInfo;
    Want want;
    want.SetElementName("bundle", "");
    request.want = want;
    bool res = processor->CheckImplicitStartExtensionIsValid(request, extensionInfo);
    EXPECT_TRUE(res);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: CheckImplicitStartExtensionIsValid
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor CheckImplicitStartExtensionIsValid
 * EnvConditions: NA
 * CaseDescription: Verify CheckImplicitStartExtensionIsValid
 */
HWTEST_F(ImplicitStartProcessorTest, CheckImplicitStartExtensionIsValid_002, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    ExtensionAbilityInfo extensionInfo;
    Want want;
    want.SetElementName("", "");
    request.want = want;
    extensionInfo.type = ExtensionAbilityType::WORK_SCHEDULER;
    bool res = processor->CheckImplicitStartExtensionIsValid(request, extensionInfo);
    EXPECT_FALSE(res);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: CheckImplicitStartExtensionIsValid
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor CheckImplicitStartExtensionIsValid
 * EnvConditions: NA
 * CaseDescription: Verify CheckImplicitStartExtensionIsValid
 */
HWTEST_F(ImplicitStartProcessorTest, CheckImplicitStartExtensionIsValid_003, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    ExtensionAbilityInfo extensionInfo;
    Want want;
    want.SetElementName("", "");
    request.want = want;
    extensionInfo.type = ExtensionAbilityType::FORM;
    bool res = processor->CheckImplicitStartExtensionIsValid(request, extensionInfo);
    EXPECT_TRUE(res);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbilityInner
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify ImplicitStartAbilityInner
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbilityInner_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    Want want;
    AbilityRequest request;
    int32_t userId = 0;
    request.callType = AbilityCallType::START_OPTIONS_TYPE;
    bool res = processor->ImplicitStartAbilityInner(want, request, userId);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbilityInner
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify ImplicitStartAbilityInner
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbilityInner_002, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    Want want;
    AbilityRequest request;
    int32_t userId = 0;
    request.callType = AbilityCallType::START_SETTINGS_TYPE;
    bool res = processor->ImplicitStartAbilityInner(want, request, userId);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbilityInner
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify ImplicitStartAbilityInner
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbilityInner_003, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    Want want;
    AbilityRequest request;
    int32_t userId = 0;
    request.callType = AbilityCallType::START_EXTENSION_TYPE;
    bool res = processor->ImplicitStartAbilityInner(want, request, userId);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAbilityInner
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify ImplicitStartAbilityInner
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAbilityInner_004, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    Want want;
    AbilityRequest request;
    int32_t userId = 0;
    request.callType = AbilityCallType::CALL_REQUEST_TYPE;
    bool res = processor->ImplicitStartAbilityInner(want, request, userId);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: CallStartAbilityInner
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor CallStartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify CallStartAbilityInner
 */
HWTEST_F(ImplicitStartProcessorTest, CallStartAbilityInner_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    int32_t userId = 0;
    Want want;
    AbilityRequest request;
    AbilityCallType callType = AbilityCallType::INVALID_TYPE;
    bool res = processor->CallStartAbilityInner(userId, want, request, callType);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: CallStartAbilityInner
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor CallStartAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify CallStartAbilityInner
 */
HWTEST_F(ImplicitStartProcessorTest, CallStartAbilityInner_002, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    int32_t userId = 0;
    Want want;
    AbilityRequest request;
    AbilityCallType callType = AbilityCallType::CALL_REQUEST_TYPE;
    bool res = processor->CallStartAbilityInner(userId, want, request, callType);
    EXPECT_NE(res, ERR_OK);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: SetTargetLinkInfo
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor SetTargetLinkInfo
 * EnvConditions: NA
 * CaseDescription: Verify SetTargetLinkInfo
 */
HWTEST_F(ImplicitStartProcessorTest, SetTargetLinkInfo_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<AppExecFwk::SkillUriForAbilityAndExtension> skillUri;
    AppExecFwk::SkillUriForAbilityAndExtension uri;
    uri.isMatch = true;
    uri.scheme = "https";
    skillUri.emplace_back(uri);
    Want want;
    want.SetParam("appLinkingOnly", true);
    processor->SetTargetLinkInfo(skillUri, want);
    int32_t targetLinkType = want.GetIntParam("send_to_erms_targetLinkType", 0);
    EXPECT_EQ(targetLinkType, 1);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: SetTargetLinkInfo
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor SetTargetLinkInfo
 * EnvConditions: NA
 * CaseDescription: Verify SetTargetLinkInfo
 */
HWTEST_F(ImplicitStartProcessorTest, SetTargetLinkInfo_002, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<AppExecFwk::SkillUriForAbilityAndExtension> skillUri;
    AppExecFwk::SkillUriForAbilityAndExtension uri;
    uri.isMatch = true;
    uri.scheme = "https";
    skillUri.emplace_back(uri);
    Want want;
    want.SetAction("ohos.want.action.viewData");
    processor->SetTargetLinkInfo(skillUri, want);
    int32_t targetLinkType = want.GetIntParam("send_to_erms_targetLinkType", 0);
    EXPECT_EQ(targetLinkType, 3);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: SetTargetLinkInfo
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor SetTargetLinkInfo
 * EnvConditions: NA
 * CaseDescription: Verify SetTargetLinkInfo
 */
HWTEST_F(ImplicitStartProcessorTest, SetTargetLinkInfo_003, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<AppExecFwk::SkillUriForAbilityAndExtension> skillUri;
    AppExecFwk::SkillUriForAbilityAndExtension uri;
    uri.isMatch = true;
    uri.scheme = "https";
    skillUri.emplace_back(uri);
    Want want;
    processor->SetTargetLinkInfo(skillUri, want);
    int32_t targetLinkType = want.GetIntParam("send_to_erms_targetLinkType", 0);
    EXPECT_EQ(targetLinkType, 2);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: MatchTypeAndUri
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor MatchTypeAndUri
 * EnvConditions: NA
 * CaseDescription: Verify MatchTypeAndUri
 */
HWTEST_F(ImplicitStartProcessorTest, MatchTypeAndUri_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    Want want;
    processor->MatchTypeAndUri(want);
    want.SetType("haha");
    EXPECT_EQ("haha", processor->MatchTypeAndUri(want));
    want.SetType("");
    want.SetUri("http://wwwsocom");
    EXPECT_EQ("", processor->MatchTypeAndUri(want));
    want.SetUri("http://www.so.com");
    processor->MatchTypeAndUri(want);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: OnlyKeepReserveApp
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor OnlyKeepReserveApp
 * EnvConditions: NA
 * CaseDescription: uriReservedFlag=false (default); early return; abilityInfos and
 *                 extensionAbInfos are unchanged.
 */
HWTEST_F(ImplicitStartProcessorTest, OnlyKeepReserveApp_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionAbInfos;
    AppExecFwk::ExtensionAbilityInfo extensionAbInfo;
    extensionAbInfos.push_back(extensionAbInfo);
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = "haha";
    abilityInfos.push_back(abilityInfo);
    AbilityRequest abilityRequest;
    abilityRequest.uriReservedFlag = false;

    processor->OnlyKeepReserveApp(abilityInfos, extensionAbInfos, abilityRequest);

    EXPECT_EQ(abilityInfos.size(), 1);
    EXPECT_EQ(abilityInfos.front().bundleName, "haha");
    EXPECT_EQ(extensionAbInfos.size(), 1);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: OnlyKeepReserveApp
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor OnlyKeepReserveApp
 * EnvConditions: NA
 * CaseDescription: Verify OnlyKeepReserveApp when uriReservedFlag is false
 */
HWTEST_F(ImplicitStartProcessorTest, OnlyKeepReserveApp_002, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionAbInfos;
    AppExecFwk::ExtensionAbilityInfo extensionAbInfo;
    extensionAbInfos.push_back(extensionAbInfo);
    AppExecFwk::AbilityInfo abilityInfo1;
    abilityInfo1.bundleName = "test1";
    abilityInfos.push_back(abilityInfo1);
    AppExecFwk::AbilityInfo abilityInfo2;
    abilityInfo2.bundleName = "test2";
    abilityInfos.push_back(abilityInfo2);
    int32_t originalAbilitySize = abilityInfos.size();
    int32_t originalExtensionSize = extensionAbInfos.size();
    AbilityRequest abilityRequest;
    abilityRequest.uriReservedFlag = false;
    abilityRequest.reservedBundleName = "test1";
    processor->OnlyKeepReserveApp(abilityInfos, extensionAbInfos, abilityRequest);
    EXPECT_EQ(abilityInfos.size(), originalAbilitySize);
    EXPECT_EQ(extensionAbInfos.size(), originalExtensionSize);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: OnlyKeepReserveApp
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor OnlyKeepReserveApp
 * EnvConditions: NA
 * CaseDescription: Verify OnlyKeepReserveApp when uriReservedFlag is true and extensionInfos is not empty
 */
HWTEST_F(ImplicitStartProcessorTest, OnlyKeepReserveApp_003, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionAbInfos;
    AppExecFwk::ExtensionAbilityInfo extensionAbInfo;
    extensionAbInfos.push_back(extensionAbInfo);
    extensionAbInfos.push_back(extensionAbInfo);
    AppExecFwk::AbilityInfo abilityInfo1;
    abilityInfo1.bundleName = "test1";
    abilityInfos.push_back(abilityInfo1);
    AbilityRequest abilityRequest;
    abilityRequest.uriReservedFlag = true;
    abilityRequest.reservedBundleName = "test1";
    int32_t originalExtensionSize = extensionAbInfos.size();
    EXPECT_GT(originalExtensionSize, 0);
    processor->OnlyKeepReserveApp(abilityInfos, extensionAbInfos, abilityRequest);
    EXPECT_EQ(extensionAbInfos.size(), 0);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: OnlyKeepReserveApp
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor OnlyKeepReserveApp
 * EnvConditions: NA
 * CaseDescription: Verify OnlyKeepReserveApp when uriReservedFlag is true and filtering abilityInfos
 */
HWTEST_F(ImplicitStartProcessorTest, OnlyKeepReserveApp_004, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionAbInfos;
    AppExecFwk::AbilityInfo abilityInfo1;
    abilityInfo1.bundleName = "test1";
    abilityInfos.push_back(abilityInfo1);
    AppExecFwk::AbilityInfo abilityInfo2;
    abilityInfo2.bundleName = "test2";
    abilityInfos.push_back(abilityInfo2);
    AppExecFwk::AbilityInfo abilityInfo3;
    abilityInfo3.bundleName = "test1";
    abilityInfos.push_back(abilityInfo3);
    int32_t originalAbilitySize = abilityInfos.size();
    EXPECT_EQ(originalAbilitySize, 3);
    AbilityRequest abilityRequest;
    abilityRequest.uriReservedFlag = true;
    abilityRequest.reservedBundleName = "test1";
    processor->OnlyKeepReserveApp(abilityInfos, extensionAbInfos, abilityRequest);
    EXPECT_EQ(abilityInfos.size(), 2);
    for (const auto& ability : abilityInfos) {
        EXPECT_EQ(ability.bundleName, "test1");
    }
}

/*
 * Feature: ImplicitStartProcessor
 * Function: OnlyKeepReserveApp
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor OnlyKeepReserveApp
 * EnvConditions: NA
 * CaseDescription: Verify OnlyKeepReserveApp when uriReservedFlag is true and filtering abilityInfos
 */
HWTEST_F(ImplicitStartProcessorTest, OnlyKeepReserveApp_005, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionAbInfos;
    AppExecFwk::AbilityInfo abilityInfo1;
    abilityInfo1.bundleName = "test1";
    abilityInfos.push_back(abilityInfo1);
    AppExecFwk::AbilityInfo abilityInfo2;
    abilityInfo2.bundleName = "test1";
    abilityInfos.push_back(abilityInfo2);
    int32_t originalAbilitySize = abilityInfos.size();
    EXPECT_EQ(originalAbilitySize, 2);
    AbilityRequest abilityRequest;
    abilityRequest.uriReservedFlag = true;
    abilityRequest.reservedBundleName = "test1";
    processor->OnlyKeepReserveApp(abilityInfos, extensionAbInfos, abilityRequest);
    EXPECT_EQ(abilityInfos.size(), 2);
    for (const auto& ability : abilityInfos) {
        EXPECT_EQ(ability.bundleName, "test1");
    }
}

/*
 * Feature: ImplicitStartProcessor
 * Function: GetDefaultAppProxy
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor GetDefaultAppProxy
 * EnvConditions: NA
 * CaseDescription: Verify GetDefaultAppProxy  etc.
 */
HWTEST_F(ImplicitStartProcessorTest, GetDefaultAppProxy_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    auto result = processor->GetDefaultAppProxy();
    EXPECT_EQ(result, nullptr);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: FilterAbilityList
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor FilterAbilityList
 * EnvConditions: NA
 * CaseDescription: Verify FilterAbilityList  etc.
 */
HWTEST_F(ImplicitStartProcessorTest, FilterAbilityList_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    Want want;
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionAbInfos;
    int32_t  userId = 100;
    auto result = processor->FilterAbilityList(want, abilityInfos, extensionAbInfos, userId);
    EXPECT_EQ(result, true);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: AddIdentity
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor AddIdentity
 * EnvConditions: NA
 * CaseDescription: Verify AddIdentity  ResetCallingIdentityAsCaller AddAbilityInfoToDialogInfos etc.
 */
HWTEST_F(ImplicitStartProcessorTest, AddIdentity_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    int32_t  userId = 102;
    std::string  identity;
    processor->AddIdentity(userId, identity);
    processor->ResetCallingIdentityAsCaller(userId, true);
    AddInfoParam param;
    std::vector<DialogAppInfo> dialogAppInfos;
    param.isExtension = true;
    param.info.type = AbilityType::FORM;
    processor->AddAbilityInfoToDialogInfos(param, dialogAppInfos);
    param.isExtension = false;
    processor->AddAbilityInfoToDialogInfos(param, dialogAppInfos);
    EXPECT_TRUE(processor != nullptr);
}
/*
 * Feature: ImplicitStartProcessor
 * Function: AddIdentity
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor AddIdentity
 * EnvConditions: NA
 * CaseDescription: Verify AddIdentity when identityList_ is at max capacity.
 */
HWTEST_F(ImplicitStartProcessorTest, AddIdentity_002, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    for (size_t i = 0; i < IDENTITY_LIST_MAX_SIZE; i++) {
        int32_t tokenId = 100 + i;
        std::string identity = "identity_" + std::to_string(i);
        processor->AddIdentity(tokenId, identity);
    }
    EXPECT_EQ(processor->identityList_.front().tokenId, 100);
    int32_t newTokenId = 200;
    std::string newIdentity = "new_identity";
    processor->AddIdentity(newTokenId, newIdentity);
    EXPECT_EQ(processor->identityList_.size(), IDENTITY_LIST_MAX_SIZE);
    EXPECT_EQ(processor->identityList_.back().tokenId, newTokenId);
    EXPECT_NE(processor->identityList_.front().tokenId, 100);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: AddIdentity
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor AddIdentity
 * EnvConditions: NA
 * CaseDescription: Verify AddIdentity.
 */
HWTEST_F(ImplicitStartProcessorTest, AddIdentity_003, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    for (size_t i = 0; i < IDENTITY_LIST_MAX_SIZE; i++) {
        int32_t tokenId = 100 + i;
        std::string identity = "identity_" + std::to_string(i);
        processor->AddIdentity(tokenId, identity);
    }
    EXPECT_EQ(processor->identityList_.front().tokenId, 100);
    EXPECT_EQ(processor->identityList_.size(), IDENTITY_LIST_MAX_SIZE);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: IsExtensionInWhiteList
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor IsExtensionInWhiteList
 * EnvConditions: NA
 * CaseDescription: Verify IsExtensionInWhiteList.
 */
HWTEST_F(ImplicitStartProcessorTest, IsExtensionInWhiteList_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsExtensionInWhiteList_001 start");
    AppExecFwk::ExtensionAbilityType type = AppExecFwk::ExtensionAbilityType::FORM;
    auto processor = std::make_shared<ImplicitStartProcessor>();
    auto res = processor->IsExtensionInWhiteList(type);
    EXPECT_TRUE(res);
    type = AppExecFwk::ExtensionAbilityType::INPUTMETHOD;
    res = processor->IsExtensionInWhiteList(type);
    EXPECT_TRUE(res);
    type = AppExecFwk::ExtensionAbilityType::WALLPAPER;
    res = processor->IsExtensionInWhiteList(type);
    EXPECT_TRUE(res);
    type = AppExecFwk::ExtensionAbilityType::WINDOW;
    res = processor->IsExtensionInWhiteList(type);
    EXPECT_TRUE(res);
    type = AppExecFwk::ExtensionAbilityType::THUMBNAIL;
    res = processor->IsExtensionInWhiteList(type);
    EXPECT_TRUE(res);
    type = AppExecFwk::ExtensionAbilityType::PREVIEW;
    res = processor->IsExtensionInWhiteList(type);
    EXPECT_TRUE(res);
    type = AppExecFwk::ExtensionAbilityType::DRIVER;
    res = processor->IsExtensionInWhiteList(type);
    EXPECT_FALSE(res);
    TAG_LOGI(AAFwkTag::TEST, "IsExtensionInWhiteList_001 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: FindExtensionInfo
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor FindExtensionInfo
 * EnvConditions: NA
 * CaseDescription: Verify FindExtensionInfo.
 */
HWTEST_F(ImplicitStartProcessorTest, FindExtensionInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FindExtensionInfo_001 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    Want want;
    int32_t flags = 0;
    int32_t userId = 100;
    int32_t appIndex = 0;
    AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ElementName elementName("", "bundleName", "abilityName");
    want.SetElement(elementName);
    auto test = want.GetElement().GetBundleName();
    TAG_LOGI(AAFwkTag::TEST, "GetBundleName %{public}s", test.c_str());
    test = want.GetElement().GetAbilityName();
    TAG_LOGI(AAFwkTag::TEST, "GetAbilityName %{public}s", test.c_str());
    auto res = processor->FindExtensionInfo(elementName, flags, userId, appIndex, abilityInfo);
    EXPECT_EQ(res, RESOLVE_ABILITY_ERR);
    TAG_LOGI(AAFwkTag::TEST, "FindExtensionInfo_001 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: IsActionImplicitStart
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor IsActionImplicitStart
 * EnvConditions: NA
 * CaseDescription: Verify IsActionImplicitStart.
 */
HWTEST_F(ImplicitStartProcessorTest, IsActionImplicitStart_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsActionImplicitStart_001 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    Want want;
    system::SetParameter(SUPPORT_ACTION_START_SELECTOR, "false");
    bool findDeafultApp = true;
    auto res = processor->IsActionImplicitStart(want, findDeafultApp);
    EXPECT_FALSE(res);
    system::SetParameter(SUPPORT_ACTION_START_SELECTOR, "true");
    res = processor->IsActionImplicitStart(want, findDeafultApp);
    EXPECT_FALSE(res);
    findDeafultApp = false;
    res = processor->IsActionImplicitStart(want, findDeafultApp);
    EXPECT_FALSE(res);
    OHOS::AAFwk::Operation operation;
    operation.abilityName_ = "abilityName";
    operation.bundleName_ = "bundleName";
    operation.moduleName_ = "moduleName";
    std::string uriString = "action:com.ix.hi";
    OHOS::Uri uri(uriString);
    operation.SetUri(uri);
    want.operation_ = operation;
    res = processor->IsActionImplicitStart(want, findDeafultApp);
    EXPECT_FALSE(res);
    uri.uriString_ = "file";
    uri.scheme_ = "file";
    operation.SetUri(uri);
    want.operation_ = operation;
    res = processor->IsActionImplicitStart(want, findDeafultApp);
    EXPECT_FALSE(res);
    OHOS::AppExecFwk::ElementName elementName("deviceId", "", "abilityName");
    want.SetElement(elementName);
    res = processor->IsActionImplicitStart(want, findDeafultApp);
    EXPECT_TRUE(res);
    TAG_LOGI(AAFwkTag::TEST, "IsActionImplicitStart_001 end");
    system::SetParameter(SUPPORT_ACTION_START_SELECTOR, "false");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: FindAbilityAppClone
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor FindAbilityAppClone
 * EnvConditions: NA
 * CaseDescription: Verify FindAbilityAppClone.
 */
HWTEST_F(ImplicitStartProcessorTest, FindAbilityAppClone_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FindAbilityAppClone_001 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    bool res = processor->FindAbilityAppClone(abilityInfos);
    EXPECT_FALSE(res);
    AppExecFwk::AbilityInfo abilityInfo1;
    AppExecFwk::AbilityInfo abilityInfo2;
    abilityInfo1.bundleName = BUNDLE_NAME;
    abilityInfo1.name = NAME;
    abilityInfo2.bundleName = BUNDLE_NAME;
    abilityInfo2.name = NAME;
    abilityInfos.emplace_back(abilityInfo1);
    abilityInfos.emplace_back(abilityInfo2);
    res = processor->FindAbilityAppClone(abilityInfos);
    EXPECT_TRUE(res);
    TAG_LOGI(AAFwkTag::TEST, "FindAbilityAppClone_001 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: FindExtensionAppClone
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor FindExtensionAppClone
 * EnvConditions: NA
 * CaseDescription: Verify FindExtensionAppClone.
 */
HWTEST_F(ImplicitStartProcessorTest, FindExtensionAppClone_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FindExtensionAppClone start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionAbilityInfos;
    bool res = processor->FindExtensionAppClone(extensionAbilityInfos);
    EXPECT_FALSE(res);
    AppExecFwk::ExtensionAbilityInfo extensionAbilityInfo1;
    AppExecFwk::ExtensionAbilityInfo extensionAbilityInfo2;
    extensionAbilityInfo1.bundleName = BUNDLE_NAME;
    extensionAbilityInfo1.name = NAME;
    extensionAbilityInfo2.bundleName = BUNDLE_NAME;
    extensionAbilityInfo2.name = NAME;
    extensionAbilityInfos.emplace_back(extensionAbilityInfo1);
    extensionAbilityInfos.emplace_back(extensionAbilityInfo2);
    res = processor->FindExtensionAppClone(extensionAbilityInfos);
    EXPECT_TRUE(res);
    TAG_LOGI(AAFwkTag::TEST, "FindExtensionAppClone end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: FindAppClone
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor FindAppClone
 * EnvConditions: NA
 * CaseDescription: Verify FindAppClone.
 */
HWTEST_F(ImplicitStartProcessorTest, FindAppClone_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FindAppClone_001 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionAbilityInfos;
    AppExecFwk::AbilityInfo abilityInfo1;
    AppExecFwk::AbilityInfo abilityInfo2;
    AppExecFwk::ExtensionAbilityInfo extensionAbilityInfo1;
    AppExecFwk::ExtensionAbilityInfo extensionAbilityInfo2;
    extensionAbilityInfo1.name = NAME;
    extensionAbilityInfo1.bundleName = BUNDLE_NAME;
    extensionAbilityInfo2.name = NAME;
    extensionAbilityInfo2.bundleName = BUNDLE_NAME;
    abilityInfo1.bundleName = BUNDLE_NAME;
    abilityInfo1.name = NAME;
    abilityInfo2.bundleName = BUNDLE_NAME;
    abilityInfo2.name = NAME;
    bool isAppCloneSelector = false;
    abilityInfos.emplace_back(abilityInfo1);
    extensionAbilityInfos.emplace_back(extensionAbilityInfo1);
    auto res = processor->FindAppClone(abilityInfos, extensionAbilityInfos, isAppCloneSelector);
    EXPECT_EQ(res, ERR_OK);
    abilityInfos.clear();
    extensionAbilityInfos.emplace_back(extensionAbilityInfo2);
    res = processor->FindAppClone(abilityInfos, extensionAbilityInfos, isAppCloneSelector);
    EXPECT_EQ(res, ERR_OK);
    extensionAbilityInfos.clear();
    abilityInfos.emplace_back(abilityInfo1);
    abilityInfos.emplace_back(abilityInfo2);
    res = processor->FindAppClone(abilityInfos, extensionAbilityInfos, isAppCloneSelector);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "FindAppClone_001 end");
}
/*
 * Feature: ImplicitStartProcessor
 * Function: ImplicitStartAG
 * SubFunction: NA
 * FunctionPoints:ImplicitStartProcessor ImplicitStartAG
 * EnvConditions: NA
 * CaseDescription: Verify ImplicitStartAG.
 */
HWTEST_F(ImplicitStartProcessorTest, ImplicitStartAG_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ImplicitStartAG start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    int32_t userId = 100;
    AbilityRequest request;
    std::vector<DialogAppInfo> dialogAppInfos;
    GenerateRequestParam genReqParam;
    genReqParam.isMoreHapList = false;
    genReqParam.findDefaultApp = false;
    genReqParam.isAppCloneSelector = false;
    bool queryAGSuccess = true;
    auto ret = processor->ImplicitStartAG(userId, request, dialogAppInfos, genReqParam, queryAGSuccess);
    EXPECT_NE(ret, 0);
    TAG_LOGI(AAFwkTag::TEST, "ImplicitStartAG end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: NeedQueryFromAG
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor NeedQueryFromAG
 * EnvConditions: NA
 * CaseDescription: Verify NeedQueryFromAG when OPEN_LINK_APP_LINKING_ONLY is false
 */
HWTEST_F(ImplicitStartProcessorTest, NeedQueryFromAG_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    bool applinkExist = false;
    auto res = processor->NeedQueryFromAG(request, applinkExist);
    EXPECT_FALSE(res);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: NeedQueryFromAG
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor NeedQueryFromAG
 * EnvConditions: NA
 * CaseDescription: Verify NeedQueryFromAG when appLinkingOnly is true
 */
HWTEST_F(ImplicitStartProcessorTest, NeedQueryFromAG_002, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    bool applinkExist = false;
    request.want.SetParam(OPEN_LINK_APP_LINKING_ONLY, true);
    auto res = processor->NeedQueryFromAG(request, applinkExist);
    EXPECT_FALSE(res);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: NeedQueryFromAG
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor NeedQueryFromAG
 * EnvConditions: NA
 * CaseDescription: Verify NeedQueryFromAG when linkUriScheme is not http/https
 */
HWTEST_F(ImplicitStartProcessorTest, NeedQueryFromAG_003, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    bool applinkExist = false;
    request.want.SetParam(OPEN_LINK_APP_LINKING_ONLY, false);
    OHOS::Uri uri("file://example.com/test.txt");
    request.want.SetUri(uri);
    auto res = processor->NeedQueryFromAG(request, applinkExist);
    EXPECT_FALSE(res);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: NeedQueryFromAG
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor NeedQueryFromAG
 * EnvConditions: NA
 * CaseDescription: Verify NeedQueryFromAG when applinkExist is true
 */
HWTEST_F(ImplicitStartProcessorTest, NeedQueryFromAG_004, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    bool applinkExist = true;
    request.want.SetParam(OPEN_LINK_APP_LINKING_ONLY, false);
    OHOS::Uri uri("https://example.com/test.html");
    request.want.SetUri(uri);
    auto res = processor->NeedQueryFromAG(request, applinkExist);
    EXPECT_FALSE(res);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: NeedQueryFromAG
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor NeedQueryFromAG
 * EnvConditions: NA
 * CaseDescription: Verify NeedQueryFromAG returns true when all conditions are met
 */
HWTEST_F(ImplicitStartProcessorTest, NeedQueryFromAG_005, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    bool applinkExist = false;
    request.want.SetParam(OPEN_LINK_APP_LINKING_ONLY, false);
    OHOS::Uri uri("https://example.com/test.html");
    request.want.SetUri(uri);
    auto res = processor->NeedQueryFromAG(request, applinkExist);
    EXPECT_TRUE(res);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: NeedQueryFromAG
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor NeedQueryFromAG
 * EnvConditions: NA
 * CaseDescription: Verify NeedQueryFromAG with http scheme
 */
HWTEST_F(ImplicitStartProcessorTest, NeedQueryFromAG_006, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    AbilityRequest request;
    bool applinkExist = false;
    request.want.SetParam(OPEN_LINK_APP_LINKING_ONLY, false);
    OHOS::Uri uri("http://example.com/test.html");
    request.want.SetUri(uri);
    auto res = processor->NeedQueryFromAG(request, applinkExist);
    EXPECT_TRUE(res);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: ResetCallingIdentityAsCaller
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor ResetCallingIdentityAsCaller
 * EnvConditions: NA
 * CaseDescription: Verify ResetCallingIdentityAsCaller.
 */
HWTEST_F(ImplicitStartProcessorTest, ResetCallingIdentityAsCaller_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    for (size_t i = 0; i < IDENTITY_LIST_MAX_SIZE; i++) {
        int32_t tokenId = 100 + i;
        std::string identity = "identity_" + std::to_string(i);
        processor->AddIdentity(tokenId, identity);
    }
    EXPECT_EQ(processor->identityList_.front().tokenId, 100);
    int32_t newTokenId = 200;
    processor->ResetCallingIdentityAsCaller(newTokenId, false);
    EXPECT_EQ(processor->identityList_.size(), IDENTITY_LIST_MAX_SIZE);
    EXPECT_EQ(processor->identityList_.front().tokenId, 100);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: RemoveIdentity
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor RemoveIdentity
 * EnvConditions: NA
 * CaseDescription: Verify RemoveIdentity.
 */
HWTEST_F(ImplicitStartProcessorTest, RemoveIdentity_001, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    for (size_t i = 0; i < IDENTITY_LIST_MAX_SIZE; i++) {
        int32_t tokenId = 100 + i;
        std::string identity = "identity_" + std::to_string(i);
        processor->AddIdentity(tokenId, identity);
    }
    EXPECT_EQ(processor->identityList_.front().tokenId, 100);
    int32_t newTokenId = 100;
    processor->RemoveIdentity(newTokenId);
    EXPECT_NE(processor->identityList_.size(), IDENTITY_LIST_MAX_SIZE);
    EXPECT_NE(processor->identityList_.front().tokenId, 100);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: RemoveIdentity
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor RemoveIdentity
 * EnvConditions: NA
 * CaseDescription: Verify RemoveIdentity.
 */
HWTEST_F(ImplicitStartProcessorTest, RemoveIdentity_002, TestSize.Level1)
{
    auto processor = std::make_shared<ImplicitStartProcessor>();
    for (size_t i = 0; i < IDENTITY_LIST_MAX_SIZE; i++) {
        int32_t tokenId = 100 + i;
        std::string identity = "identity_" + std::to_string(i);
        processor->AddIdentity(tokenId, identity);
    }
    EXPECT_EQ(processor->identityList_.front().tokenId, 100);
    int32_t newTokenId = 200;
    processor->RemoveIdentity(newTokenId);
    EXPECT_EQ(processor->identityList_.size(), IDENTITY_LIST_MAX_SIZE);
    EXPECT_EQ(processor->identityList_.front().tokenId, 100);
}

/*
 * Feature: ImplicitStartProcessor
 * Function: FilterCloneByDefaultApp
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor FilterCloneByDefaultApp
 * EnvConditions: NA
 * CaseDescription: Verify FilterCloneByDefaultApp filters to the matching default clone.
 */
HWTEST_F(ImplicitStartProcessorTest, FilterCloneByDefaultApp_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FilterCloneByDefaultApp_001 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<DialogAppInfo> dialogAppInfos;
    DialogAppInfo info1;
    info1.bundleName = BUNDLE_NAME;
    info1.abilityName = NAME;
    info1.appIndex = 1;
    DialogAppInfo info2;
    info2.bundleName = BUNDLE_NAME;
    info2.abilityName = NAME;
    info2.appIndex = 2;
    DialogAppInfo info3;
    info3.bundleName = BUNDLE_NAME;
    info3.abilityName = NAME;
    info3.appIndex = 3;
    dialogAppInfos.emplace_back(info1);
    dialogAppInfos.emplace_back(info2);
    dialogAppInfos.emplace_back(info3);

    processor->FilterCloneByDefaultApp(dialogAppInfos, BUNDLE_NAME, 2);
    EXPECT_EQ(dialogAppInfos.size(), 1);
    EXPECT_EQ(dialogAppInfos.front().appIndex, 2);
    TAG_LOGI(AAFwkTag::TEST, "FilterCloneByDefaultApp_001 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: FilterCloneByDefaultApp
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor FilterCloneByDefaultApp
 * EnvConditions: NA
 * CaseDescription: Verify FilterCloneByDefaultApp does not filter when no appIndex match.
 */
HWTEST_F(ImplicitStartProcessorTest, FilterCloneByDefaultApp_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FilterCloneByDefaultApp_002 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<DialogAppInfo> dialogAppInfos;
    DialogAppInfo info1;
    info1.bundleName = BUNDLE_NAME;
    info1.abilityName = NAME;
    info1.appIndex = 1;
    DialogAppInfo info2;
    info2.bundleName = BUNDLE_NAME;
    info2.abilityName = NAME;
    info2.appIndex = 2;
    dialogAppInfos.emplace_back(info1);
    dialogAppInfos.emplace_back(info2);

    processor->FilterCloneByDefaultApp(dialogAppInfos, BUNDLE_NAME, 99);
    EXPECT_EQ(dialogAppInfos.size(), 2);
    TAG_LOGI(AAFwkTag::TEST, "FilterCloneByDefaultApp_002 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: FilterCloneByDefaultApp
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor FilterCloneByDefaultApp
 * EnvConditions: NA
 * CaseDescription: Verify FilterCloneByDefaultApp does not filter when bundleName mismatches.
 */
HWTEST_F(ImplicitStartProcessorTest, FilterCloneByDefaultApp_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FilterCloneByDefaultApp_003 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<DialogAppInfo> dialogAppInfos;
    DialogAppInfo info1;
    info1.bundleName = BUNDLE_NAME;
    info1.abilityName = NAME;
    info1.appIndex = 1;
    dialogAppInfos.emplace_back(info1);

    processor->FilterCloneByDefaultApp(dialogAppInfos, "other_bundle", 1);
    EXPECT_EQ(dialogAppInfos.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "FilterCloneByDefaultApp_003 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: FilterCloneByDefaultApp
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor FilterCloneByDefaultApp
 * EnvConditions: NA
 * CaseDescription: Verify FilterCloneByDefaultApp with empty dialogAppInfos.
 */
HWTEST_F(ImplicitStartProcessorTest, FilterCloneByDefaultApp_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FilterCloneByDefaultApp_004 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<DialogAppInfo> dialogAppInfos;

    processor->FilterCloneByDefaultApp(dialogAppInfos, BUNDLE_NAME, 1);
    EXPECT_EQ(dialogAppInfos.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "FilterCloneByDefaultApp_004 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: FilterCloneByDefaultApp
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor FilterCloneByDefaultApp
 * EnvConditions: NA
 * CaseDescription: default clone matches one bundle; entries of OTHER bundles are also removed,
 *                 only the matched default clone survives.
 */
HWTEST_F(ImplicitStartProcessorTest, FilterCloneByDefaultApp_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FilterCloneByDefaultApp_005 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<DialogAppInfo> dialogAppInfos;
    DialogAppInfo info1;
    info1.bundleName = BUNDLE_NAME;
    info1.abilityName = NAME;
    info1.appIndex = 1;
    DialogAppInfo info2;
    info2.bundleName = BUNDLE_NAME;
    info2.abilityName = NAME;
    info2.appIndex = 2;
    DialogAppInfo info3;
    info3.bundleName = "other_bundle";
    info3.abilityName = NAME;
    info3.appIndex = 1;
    dialogAppInfos.emplace_back(info1);
    dialogAppInfos.emplace_back(info2);
    dialogAppInfos.emplace_back(info3);

    processor->FilterCloneByDefaultApp(dialogAppInfos, BUNDLE_NAME, 2);
    ASSERT_EQ(dialogAppInfos.size(), 1);
    EXPECT_EQ(dialogAppInfos.front().bundleName, BUNDLE_NAME);
    EXPECT_EQ(dialogAppInfos.front().appIndex, 2);
    TAG_LOGI(AAFwkTag::TEST, "FilterCloneByDefaultApp_005 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: FilterCloneByDefaultApp
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor FilterCloneByDefaultApp
 * EnvConditions: NA
 * CaseDescription: default appIndex is 0 (main app); match against an entry whose appIndex is the
 *                 default-initialized value 0.
 */
HWTEST_F(ImplicitStartProcessorTest, FilterCloneByDefaultApp_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FilterCloneByDefaultApp_006 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<DialogAppInfo> dialogAppInfos;
    DialogAppInfo info1;
    info1.bundleName = BUNDLE_NAME;
    info1.abilityName = NAME;
    info1.appIndex = 0;
    DialogAppInfo info2;
    info2.bundleName = BUNDLE_NAME;
    info2.abilityName = NAME;
    info2.appIndex = 1;
    dialogAppInfos.emplace_back(info1);
    dialogAppInfos.emplace_back(info2);

    processor->FilterCloneByDefaultApp(dialogAppInfos, BUNDLE_NAME, 0);
    ASSERT_EQ(dialogAppInfos.size(), 1);
    EXPECT_EQ(dialogAppInfos.front().bundleName, BUNDLE_NAME);
    EXPECT_EQ(dialogAppInfos.front().appIndex, 0);
    TAG_LOGI(AAFwkTag::TEST, "FilterCloneByDefaultApp_006 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: FilterCloneByDefaultApp
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor FilterCloneByDefaultApp
 * EnvConditions: NA
 * CaseDescription: duplicate (bundleName, appIndex) entries exist; only the first match is kept.
 */
HWTEST_F(ImplicitStartProcessorTest, FilterCloneByDefaultApp_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FilterCloneByDefaultApp_007 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<DialogAppInfo> dialogAppInfos;
    DialogAppInfo info1;
    info1.bundleName = BUNDLE_NAME;
    info1.abilityName = NAME;
    info1.appIndex = 2;
    DialogAppInfo info2;
    info2.bundleName = BUNDLE_NAME;
    info2.abilityName = NAME;
    info2.appIndex = 2;
    DialogAppInfo info3;
    info3.bundleName = BUNDLE_NAME;
    info3.abilityName = NAME;
    info3.appIndex = 1;
    dialogAppInfos.emplace_back(info1);
    dialogAppInfos.emplace_back(info2);
    dialogAppInfos.emplace_back(info3);

    processor->FilterCloneByDefaultApp(dialogAppInfos, BUNDLE_NAME, 2);
    ASSERT_EQ(dialogAppInfos.size(), 1);
    EXPECT_EQ(dialogAppInfos.front().appIndex, 2);
    TAG_LOGI(AAFwkTag::TEST, "FilterCloneByDefaultApp_007 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: IsExistDefaultApp
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor IsExistDefaultApp
 * EnvConditions: NA
 * CaseDescription: Verify IsExistDefaultApp returns false when no default app proxy available.
 */
HWTEST_F(ImplicitStartProcessorTest, IsExistDefaultApp_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsExistDefaultApp_001 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::string defaultBundleName;
    int32_t defaultAppIndex = 0;
    bool result = processor->IsExistDefaultApp(0, ".pdf", defaultBundleName, defaultAppIndex);
    EXPECT_FALSE(result);
    EXPECT_TRUE(defaultBundleName.empty());
    EXPECT_EQ(defaultAppIndex, 0);
    TAG_LOGI(AAFwkTag::TEST, "IsExistDefaultApp_001 end");
}

HWTEST_F(ImplicitStartProcessorTest, FilterClonesByPreferredIndex_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FilterClonesByPreferredIndex_001 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<DialogAppInfo> dialogAppInfos;
    DialogAppInfo info1;
    info1.bundleName = "bundleA";
    info1.appIndex = 1;
    DialogAppInfo info2;
    info2.bundleName = "bundleA";
    info2.appIndex = 2;
    DialogAppInfo info3;
    info3.bundleName = "bundleB";
    info3.appIndex = 1;
    DialogAppInfo info4;
    info4.bundleName = "bundleB";
    info4.appIndex = 2;
    dialogAppInfos.emplace_back(info1);
    dialogAppInfos.emplace_back(info2);
    dialogAppInfos.emplace_back(info3);
    dialogAppInfos.emplace_back(info4);

    processor->FilterClonesByPreferredIndex(dialogAppInfos, 0);
    EXPECT_EQ(dialogAppInfos.size(), 4);
    TAG_LOGI(AAFwkTag::TEST, "FilterClonesByPreferredIndex_001 end");
}

HWTEST_F(ImplicitStartProcessorTest, FilterClonesByPreferredIndex_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FilterClonesByPreferredIndex_002 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<DialogAppInfo> dialogAppInfos;
    DialogAppInfo info1;
    info1.bundleName = BUNDLE_NAME;
    info1.appIndex = 1;
    dialogAppInfos.emplace_back(info1);

    processor->FilterClonesByPreferredIndex(dialogAppInfos, 0);
    EXPECT_EQ(dialogAppInfos.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "FilterClonesByPreferredIndex_002 end");
}

HWTEST_F(ImplicitStartProcessorTest, FilterClonesByPreferredIndex_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FilterClonesByPreferredIndex_003 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<DialogAppInfo> dialogAppInfos;

    processor->FilterClonesByPreferredIndex(dialogAppInfos, 0);
    EXPECT_EQ(dialogAppInfos.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "FilterClonesByPreferredIndex_003 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: FilterClonesByPreferredIndex
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor FilterClonesByPreferredIndex
 * EnvConditions: NA
 * CaseDescription: preferred index configured & present in candidates; non-preferred clones of
 *                 that bundle are removed; bundles without preferred config are kept intact.
 */
HWTEST_F(ImplicitStartProcessorTest, FilterClonesByPreferredIndex_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FilterClonesByPreferredIndex_004 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<DialogAppInfo> dialogAppInfos;
    DialogAppInfo info1;
    info1.bundleName = "bundleA";
    info1.appIndex = 1;
    DialogAppInfo info2;
    info2.bundleName = "bundleA";
    info2.appIndex = 2;
    DialogAppInfo info3;
    info3.bundleName = "bundleA";
    info3.appIndex = 3;
    DialogAppInfo info4;
    info4.bundleName = "bundleB";
    info4.appIndex = 1;
    DialogAppInfo info5;
    info5.bundleName = "bundleB";
    info5.appIndex = 2;
    dialogAppInfos.emplace_back(info1);
    dialogAppInfos.emplace_back(info2);
    dialogAppInfos.emplace_back(info3);
    dialogAppInfos.emplace_back(info4);
    dialogAppInfos.emplace_back(info5);

    MockMultiAppUtilsStatus::preferredIndexMap_["bundleA"] = 2;
    processor->FilterClonesByPreferredIndex(dialogAppInfos, 0);

    ASSERT_EQ(dialogAppInfos.size(), 3);
    int32_t bundleACount = 0;
    int32_t bundleBCount = 0;
    int32_t bundleAPreferredCount = 0;
    for (const auto &info : dialogAppInfos) {
        if (info.bundleName == "bundleA") {
            bundleACount++;
            if (info.appIndex == 2) {
                bundleAPreferredCount++;
            }
        } else if (info.bundleName == "bundleB") {
            bundleBCount++;
        }
    }
    EXPECT_EQ(bundleACount, 1);
    EXPECT_EQ(bundleAPreferredCount, 1);
    EXPECT_EQ(bundleBCount, 2);
    TAG_LOGI(AAFwkTag::TEST, "FilterClonesByPreferredIndex_004 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: FilterClonesByPreferredIndex
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor FilterClonesByPreferredIndex
 * EnvConditions: NA
 * CaseDescription: preferred index configured but NOT present in candidates (clone uninstalled);
 *                 the bundle is not filtered (found=false), dialogAppInfos unchanged.
 */
HWTEST_F(ImplicitStartProcessorTest, FilterClonesByPreferredIndex_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FilterClonesByPreferredIndex_005 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<DialogAppInfo> dialogAppInfos;
    DialogAppInfo info1;
    info1.bundleName = "bundleA";
    info1.appIndex = 1;
    DialogAppInfo info2;
    info2.bundleName = "bundleA";
    info2.appIndex = 2;
    dialogAppInfos.emplace_back(info1);
    dialogAppInfos.emplace_back(info2);

    MockMultiAppUtilsStatus::preferredIndexMap_["bundleA"] = 99;
    processor->FilterClonesByPreferredIndex(dialogAppInfos, 0);

    EXPECT_EQ(dialogAppInfos.size(), 2);
    TAG_LOGI(AAFwkTag::TEST, "FilterClonesByPreferredIndex_005 end");
}

/*
 * Feature: ImplicitStartProcessor
 * Function: FilterClonesByPreferredIndex
 * SubFunction: NA
 * FunctionPoints: ImplicitStartProcessor FilterClonesByPreferredIndex
 * EnvConditions: NA
 * CaseDescription: bundles with a single candidate (count<=1) are skipped even when a preferred
 *                 index is configured; multi-clone bundles with preferred index are filtered.
 */
HWTEST_F(ImplicitStartProcessorTest, FilterClonesByPreferredIndex_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FilterClonesByPreferredIndex_006 start");
    auto processor = std::make_shared<ImplicitStartProcessor>();
    std::vector<DialogAppInfo> dialogAppInfos;
    DialogAppInfo info1;
    info1.bundleName = "bundleA";
    info1.appIndex = 1;
    DialogAppInfo info2;
    info2.bundleName = "bundleA";
    info2.appIndex = 2;
    DialogAppInfo info3;
    info3.bundleName = "bundleB";
    info3.appIndex = 1;
    dialogAppInfos.emplace_back(info1);
    dialogAppInfos.emplace_back(info2);
    dialogAppInfos.emplace_back(info3);

    MockMultiAppUtilsStatus::preferredIndexMap_["bundleA"] = 2;
    MockMultiAppUtilsStatus::preferredIndexMap_["bundleB"] = 1;
    processor->FilterClonesByPreferredIndex(dialogAppInfos, 0);

    ASSERT_EQ(dialogAppInfos.size(), 2);
    bool preferredKept = false;
    bool singleBundleKept = false;
    for (const auto &info : dialogAppInfos) {
        if (info.bundleName == "bundleA" && info.appIndex == 2) {
            preferredKept = true;
        }
        if (info.bundleName == "bundleB" && info.appIndex == 1) {
            singleBundleKept = true;
        }
    }
    EXPECT_TRUE(preferredKept);
    EXPECT_TRUE(singleBundleKept);
    TAG_LOGI(AAFwkTag::TEST, "FilterClonesByPreferredIndex_006 end");
}
}  // namespace AAFwk
}  // namespace OHOS
