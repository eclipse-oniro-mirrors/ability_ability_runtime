/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "ability_info.h"
#include "hilog_tag_wrapper.h"
#define private public
#include "res_sched_util.h"
#undef private
#include "parameters.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
using AbilityInfo = AppExecFwk::AbilityInfo;
class ResSchedUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ResSchedUtilTest::SetUpTestCase(void)
{}

void ResSchedUtilTest::TearDownTestCase(void)
{}

void ResSchedUtilTest::SetUp()
{}

void ResSchedUtilTest::TearDown()
{}

/**
 * @tc.number: ResSchedUtilTest_0100
 * @tc.desc: Test ReportAbilityStartInfoToRSS works
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedUtilTest, ResSchedUtilTest_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ResSchedUtilTest_0100 called.");
    AbilityInfo abilityInfo;
    int64_t resSchedType = -1;
    AAFwk::ResSchedUtil::GetInstance().ReportAbilityStartInfoToRSS(abilityInfo, -1, false, false, -1);
    AAFwk::ResSchedUtil::GetInstance().ReportAbilityAssociatedStartInfoToRSS(abilityInfo, resSchedType, 0, 0);
    int64_t ret = AAFwk::ResSchedUtil::GetInstance().convertType(resSchedType);
    EXPECT_EQ(resSchedType, ret);
}

/**
 * @tc.number: ResSchedUtilTest_0200
 * @tc.desc: Test ReportAbilityStartInfoToRSS works
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedUtilTest, ResSchedUtilTest_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ResSchedUtilTest_0200 called.");
    std::string testName = "ResSchedUtilTest";
    AAFwk::ResSchedUtil::GetInstance().ReportEventToRSS(0, testName, testName);
    std::unordered_set<int32_t> frozenPids;
    AAFwk::ResSchedUtil::GetInstance().GetAllFrozenPidsFromRSS(frozenPids);
    int64_t resSchedType = AAFwk::RES_TYPE_SCB_START_ABILITY;
    int64_t ret = AAFwk::ResSchedUtil::GetInstance().convertType(resSchedType);
    EXPECT_EQ(resSchedType, ret);
}

/**
 * @tc.number: ResSchedUtilTest_0300
 * @tc.desc: Test ReportLoadingEventToRss works
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedUtilTest, ResSchedUtilTest_0300, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ResSchedUtilTest_0300 called.");
    AAFwk::ResSchedUtil::GetInstance().ReportLoadingEventToRss(AAFwk::LoadingStage::LOAD_BEGIN,
        0, 0, 0);
    int64_t resSchedType = AAFwk::RES_TYPE_SCB_START_ABILITY;
    int64_t ret = AAFwk::ResSchedUtil::GetInstance().convertType(resSchedType);
    EXPECT_EQ(resSchedType, ret);
}

/**
 * @tc.number: ResSchedUtilTest_0400
 * @tc.desc: Test ReportUIExtensionProcColdStartToRss works
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedUtilTest, ResSchedUtilTest_0400, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ResSchedUtilTest_0400 called.");
    int32_t pickerType = static_cast<int32_t>(AppExecFwk::ExtensionAbilityType::SYSPICKER_PHOTOPICKER);
    int32_t hostPid = 102;
    std::string hostBundleName = "hostBundleName";
    std::string bundleName = "bundleName";
    std::string abilityName = "abilityName";
    std::string moduleName = "moduleName";
    bool isPreloadUIExtension = false;
    AAFwk::ResSchedUtil::GetInstance().ReportUIExtensionProcColdStartToRss(pickerType,
        hostPid, hostBundleName, bundleName, abilityName, moduleName, isPreloadUIExtension);
    SUCCEED();
    int64_t resSchedType = AAFwk::RES_TYPE_SCB_START_ABILITY;
    int64_t ret = AAFwk::ResSchedUtil::GetInstance().convertType(resSchedType);
    EXPECT_EQ(resSchedType, ret);
}

/**
 * @tc.number: GetThawReasonByAbilityType
 * @tc.desc: Test GetThawReasonByAbilityType
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedUtilTest, GetThawReasonByAbilityType, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetThawReasonByAbilityType called.");
    AbilityInfo abilityInfo;
    std::string reason = AAFwk::ResSchedUtil::GetInstance().GetThawReasonByAbilityType(abilityInfo);
    EXPECT_EQ(reason, "THAW_BY_START_NOT_PAGE_ABILITY");

    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    reason = AAFwk::ResSchedUtil::GetInstance().GetThawReasonByAbilityType(abilityInfo);
    EXPECT_EQ(reason, "THAW_BY_START_PAGE_ABILITY");

    abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    reason = AAFwk::ResSchedUtil::GetInstance().GetThawReasonByAbilityType(abilityInfo);
    EXPECT_EQ(reason, "THAW_BY_START_SERVICE_EXTENSION");

    abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI;
    reason = AAFwk::ResSchedUtil::GetInstance().GetThawReasonByAbilityType(abilityInfo);
    EXPECT_EQ(reason, "THAW_BY_START_UI_EXTENSION");
}

/**
 * @tc.number: PromotePriorityToRSS_0100
 * @tc.desc: Test PromotePriorityToRSS works
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedUtilTest, PromotePriorityToRSS_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "PromotePriorityToRSS_0100 start");
    std::string testName = "PromotePriorityToRSS";
    int32_t callerUid = 1000;
    int32_t callerPid = 10001;
    std::string targetBundleName = "com.example.target";
    int32_t targetUid = 2000;
    int32_t targetPid = 20001;
    AAFwk::ResSchedUtil::GetInstance().PromotePriorityToRSS(
        callerUid, callerPid, targetBundleName, targetUid, targetPid);
    EXPECT_EQ(callerUid, 1000);
    EXPECT_EQ(callerPid, 10001);
    EXPECT_EQ(targetBundleName, "com.example.target");
    EXPECT_EQ(targetUid, 2000);
    EXPECT_EQ(targetPid, 20001);
    TAG_LOGI(AAFwkTag::TEST, "PromotePriorityToRSS_0100 end");
}

/**
 * @tc.number: ReportForkAllEventToRSS_0100
 * @tc.desc: Test ReportForkAllEventToRSS works
 * @tc.type: FUNC
 */
HWTEST_F(ResSchedUtilTest, ReportForkAllEventToRSS_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "ReportForkAllEventToRSS_0100 start");
    std::string testName = "ReportForkAllEventToRSS";
    int32_t imagePid = 100;
    int32_t originPid = 200;
    int32_t forkAllState = 0;
    AAFwk::ResSchedUtil::GetInstance().ReportForkAllEventToRSS(imagePid, originPid, nullptr, forkAllState);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AAFwk::ResSchedUtil::GetInstance().ReportForkAllEventToRSS(imagePid, originPid, abilityInfo, forkAllState);
    EXPECT_EQ(imagePid, 100);
    EXPECT_EQ(originPid, 200);
    EXPECT_EQ(forkAllState, 0);
    TAG_LOGI(AAFwkTag::TEST, "ReportForkAllEventToRSS_0100 end");
}
}  // namespace AbilityRuntime
}  // namespace OHOS
