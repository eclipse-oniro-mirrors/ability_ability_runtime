/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "app_lifecycle_deal.h"
#include "mock_app_scheduler.h"
#undef private

#include "application_info.h"
#include "app_mgr_service_dump_error_code.h"
#include "mock_ability_token.h"
#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppLifecycleDealTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppLifecycleDealTest::SetUpTestCase(void)
{}

void AppLifecycleDealTest::TearDownTestCase(void)
{}

void AppLifecycleDealTest::SetUp()
{}

void AppLifecycleDealTest::TearDown()
{}

/**
 * @tc.name: NotifyAppFault_001
 * @tc.desc: Verify that the NotifyAppFault interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, NotifyAppFault_001, TestSize.Level1)
{
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    FaultData faultData;
    int32_t result = appLifeCycle->NotifyAppFault(faultData);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: NotifyAppFault_002
 * @tc.desc: Verify that the NotifyAppFault interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, NotifyAppFault_002, TestSize.Level1)
{
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);
    FaultData faultData;
    auto retsult = appLifeCycle->NotifyAppFault(faultData);
    EXPECT_EQ(ERR_OK, retsult);
}

/**
 * @tc.name: AttachAppDebug_001
 * @tc.desc: Test the normal state of AttachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, AttachAppDebug_001, TestSize.Level1)
{
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    EXPECT_NE(appLifeCycle, nullptr);
    auto result = appLifeCycle->AttachAppDebug(false);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AttachAppDebug_002
 * @tc.desc: Test the abnormal state of AttachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, AttachAppDebug_002, TestSize.Level1)
{
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    EXPECT_NE(appLifeCycle, nullptr);
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);
    auto result = appLifeCycle->AttachAppDebug(false);
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: DetachAppDebug_001
 * @tc.desc: Test the abnormal state of DetachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, DetachAppDebug_001, TestSize.Level1)
{
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    EXPECT_NE(appLifeCycle, nullptr);
    auto result = appLifeCycle->DetachAppDebug();
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: DetachAppDebug_002
 * @tc.desc: Test the normal state of DetachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, DetachAppDebug_002, TestSize.Level1)
{
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    EXPECT_NE(appLifeCycle, nullptr);
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);
    auto result = appLifeCycle->DetachAppDebug();
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: ChangeAppGcState_001
 * @tc.desc: Verify that the ChangeAppGcState interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, ChangeAppGcState_001, TestSize.Level1)
{
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    int32_t result = appLifeCycle->ChangeAppGcState(0);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);
    int32_t result1 = appLifeCycle->ChangeAppGcState(0);
    EXPECT_EQ(ERR_OK, result1);
}

/**
 * @tc.name: ChangeAppGcState_002
 * @tc.desc: Verify that the ChangeAppGcState interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, ChangeAppGcState_002, TestSize.Level1)
{
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    int32_t result = appLifeCycle->ChangeAppGcState(0, 1);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);
    int32_t result1 = appLifeCycle->ChangeAppGcState(0, 1);
    EXPECT_EQ(ERR_OK, result1);
}

/**
 * @tc.name: UpdateApplicationInfoInstalled_001
 * @tc.desc: Test the normal state of UpdateApplicationInfoInstalled
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, UpdateApplicationInfoInstalled_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "UpdateApplicationInfoInstalled_001 start.");
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    std::shared_ptr<ApplicationInfo> appInfo;
    std::string moduleName;
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    EXPECT_CALL(*mockAppScheduler, ScheduleUpdateApplicationInfoInstalled(_, _)).Times(0);
    appLifeCycle->UpdateApplicationInfoInstalled(*appInfo, moduleName);
}

/**
 * @tc.name: AddAbilityStage_001
 * @tc.desc: Test the normal state of AddAbilityStage
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, AddAbilityStage_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AddAbilityStage_001 start.");
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    HapModuleInfo abilityStage;
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    EXPECT_CALL(*mockAppScheduler, ScheduleAbilityStage(_)).Times(0);
    appLifeCycle->AddAbilityStage(abilityStage);
}

/**
 * @tc.name: AddAbilityStage_002
 * @tc.desc: Test the normal state of AddAbilityStage
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, AddAbilityStage_002, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AddAbilityStage_002 start.");
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    HapModuleInfo abilityStage;
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);
    EXPECT_CALL(*mockAppScheduler, ScheduleAbilityStage(_)).Times(1);
    appLifeCycle->AddAbilityStage(abilityStage);
}

/**
 * @tc.name: UpdateApplicationInfoInstalled_002
 * @tc.desc: Test the normal state of UpdateApplicationInfoInstalled
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, UpdateApplicationInfoInstalled_002, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "UpdateApplicationInfoInstalled_002 start.");
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    ApplicationInfo appInfo;
    std::string moduleName;
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);
    EXPECT_CALL(*mockAppScheduler, ScheduleUpdateApplicationInfoInstalled(_, _)).Times(1);
    appLifeCycle->UpdateApplicationInfoInstalled(appInfo, moduleName);
}

/**
 * @tc.name: LaunchAbility_001
 * @tc.desc: Test the normal state of LaunchAbility
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, LaunchAbility_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "LaunchAbility_001 start.");
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto ability = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    EXPECT_CALL(*mockAppScheduler, ScheduleLaunchAbility(_, _, _, _, _)).Times(1);
    appLifeCycle->LaunchAbility(ability);
}

/**
 * @tc.name: LaunchAbility_002
 * @tc.desc: Test the normal state of LaunchAbility
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, LaunchAbility_002, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "LaunchAbility_002 start.");
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto ability = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    const std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    ability->SetWant(want);
    EXPECT_CALL(*mockAppScheduler, ScheduleLaunchAbility(_, _, _, _, _)).Times(1);
    appLifeCycle->LaunchAbility(ability);
}

/**
 * @tc.name: LaunchAbility_003
 * @tc.desc: Test the normal state of LaunchAbility
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, LaunchAbility_003, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "LaunchAbility_003 start.");
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::DATA;
    sptr<IRemoteObject> token = new MockAbilityToken();
    int32_t abilityRecordId = 1;
    auto ability = std::make_shared<AbilityRunningRecord>(abilityInfo, token, abilityRecordId);
    const std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    ability->SetWant(want);
    EXPECT_CALL(*mockAppScheduler, ScheduleLaunchAbility(_, _, _, _, _)).Times(1);
    appLifeCycle->LaunchAbility(ability);
}

/**
 * @tc.name: ScheduleForegroundRunning_001
 * @tc.desc: Test the normal state of ScheduleForegroundRunning
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, ScheduleForegroundRunning_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ScheduleForegroundRunning_001 start.");
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    auto result = appLifeCycle->ScheduleForegroundRunning();
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: ScheduleHeapMemory_001
 * @tc.desc: Test the normal state of ScheduleHeapMemory
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, ScheduleHeapMemory_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ScheduleHeapMemory_001 start.");
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);
    int32_t pid = 1001;
    OHOS::AppExecFwk::MallocInfo mallocInfo;
    EXPECT_CALL(*mockAppScheduler, ScheduleHeapMemory(_, _)).Times(1);
    appLifeCycle->ScheduleHeapMemory(pid, mallocInfo);
}

/**
 * @tc.name: ScheduleJsHeapMemory_001
 * @tc.desc: Test the normal state of ScheduleJsHeapMemory
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, ScheduleJsHeapMemory_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ScheduleJsHeapMemory_001 start.");
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);
    OHOS::AppExecFwk::JsHeapDumpInfo info;
    EXPECT_CALL(*mockAppScheduler, ScheduleJsHeapMemory(_)).Times(1);
    appLifeCycle->ScheduleJsHeapMemory(info);
}

/**
 * @tc.name: ScheduleClearPageStack_001
 * @tc.desc: Test the normal state of ScheduleClearPageStack
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, ScheduleClearPageStack_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ScheduleClearPageStack_001 start.");
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);
    EXPECT_CALL(*mockAppScheduler, ScheduleClearPageStack()).Times(1);
    appLifeCycle->ScheduleClearPageStack();
}

/**
 * @tc.name: ScheduleAcceptWant_001
 * @tc.desc: Test the normal state of ScheduleAcceptWant
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, ScheduleAcceptWant_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ScheduleAcceptWant_001 start.");
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);
    AAFwk::Want* want = new AAFwk::Want();
    std::string moduleName;
    EXPECT_CALL(*mockAppScheduler, ScheduleAcceptWant(_, _)).Times(1);
    appLifeCycle->ScheduleAcceptWant(*want, moduleName);
}

/**
 * @tc.name: SchedulePrepareTerminate_001
 * @tc.desc: Test the normal state of SchedulePrepareTerminate
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, SchedulePrepareTerminate_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SchedulePrepareTerminate_001 start.");
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);
    std::string moduleName;
    EXPECT_CALL(*mockAppScheduler, SchedulePrepareTerminate(_)).Times(1);
    appLifeCycle->SchedulePrepareTerminate(moduleName);
}

/**
 * @tc.name: ScheduleNewProcessRequest_001
 * @tc.desc: Test the normal state of ScheduleNewProcessRequest
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, ScheduleNewProcessRequest_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ScheduleNewProcessRequest_001 start.");
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);
    AAFwk::Want* want = new AAFwk::Want();
    std::string moduleName;
    EXPECT_CALL(*mockAppScheduler, ScheduleNewProcessRequest(_, _)).Times(1);
    appLifeCycle->ScheduleNewProcessRequest(*want, moduleName);
}

/**
 * @tc.name: ScheduleCacheProcess_001
 * @tc.desc: Test the normal state of ScheduleCacheProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, ScheduleCacheProcess_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ScheduleCacheProcess_001 start.");
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);
    EXPECT_CALL(*mockAppScheduler, ScheduleCacheProcess()).Times(1);
    appLifeCycle->ScheduleCacheProcess();
}

/**
 * @tc.name: DumpFfrt_001
 * @tc.desc: Test the normal state of DumpFfrt
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, DumpFfrt_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "DumpFfrt_001 start.");
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);
    std::string result;
    EXPECT_CALL(*mockAppScheduler, ScheduleDumpFfrt(_)).Times(1);
    appLifeCycle->DumpFfrt(result);
}

/**
 * @tc.name: ScheduleDumpArkWeb_001
 * @tc.desc: Test ScheduleDumpArkWeb func
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, ScheduleDumpArkWeb_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ScheduleDumpArkWeb_001 start.");
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    appLifeCycle->SetApplicationClient(nullptr);
    std::string result;
    std::string customArgs = "gpu";
    auto ret = appLifeCycle->DumpArkWeb(customArgs, result);
    EXPECT_EQ(ret, DumpErrorCode::ERR_INTERNAL_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "ScheduleDumpArkWeb_001 end.");
}

/**
 * @tc.name: ScheduleDumpArkWeb_002
 * @tc.desc: Test ScheduleDumpArkWeb func
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, ScheduleDumpArkWeb_002, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ScheduleDumpArkWeb_002 start.");
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);
    std::string result;
    std::string customArgs = "gpu";
    EXPECT_CALL(*mockAppScheduler, ScheduleDumpArkWeb(_, _)).Times(1);
    appLifeCycle->DumpArkWeb(customArgs, result);
    TAG_LOGD(AAFwkTag::TEST, "ScheduleDumpArkWeb_002 end.");
}

/**
 * @tc.name: SetWatchdogBackgroundStatusRunning_001
 * @tc.desc: Test the normal state of SetWatchdogBackgroundStatusRunning
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, SetWatchdogBackgroundStatusRunning_001, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SetWatchdogBackgroundStatusRunning_001 start.");
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);
    bool status = true;
    EXPECT_CALL(*mockAppScheduler, SetWatchdogBackgroundStatus(_)).Times(1);
    appLifeCycle->SetWatchdogBackgroundStatusRunning(status);
}
}  // namespace AppExecFwk
}  // namespace OHOS
