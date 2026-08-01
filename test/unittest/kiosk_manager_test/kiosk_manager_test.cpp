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

#include "ability_bundle_event_callback.h"
#include "ability_connect_manager.h"
#include "ability_manager_service.h"
#include "ability_util.h"
#include "connection_data.h"
#include "interceptor/kiosk_interceptor.h"
#include "mock_ability_connect_callback.h"
#include "mock_ipc_skeleton.h"
#include "mock_my_flag.h"
#include "mock_my_status.h"
#include "mock_parameters.h"
#include "mock_permission_verification.h"
#include "mock_test_object.h"
#include "mock_scene_board_judgement.h"
#include "remote_on_listener_stub_mock.h"
#include "session/host/include/session.h"
#include "system_ability_definition.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::AbilityType;
constexpr char KIOSK_WHITE_LIST[] = "KioskWhitelist";

namespace OHOS {
namespace AAFwk {
class KioskManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityRecord> MockAbilityRecord(AbilityType);
    sptr<Token> MockToken(AbilityType);
    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);

    std::shared_ptr<AbilityRecord> abilityRecord;
    sptr<SessionInfo> MockSessionInfo(int32_t persistentId);
};

void KioskManagerTest::SetUpTestCase() {}

void KioskManagerTest::TearDownTestCase() {}

void KioskManagerTest::SetUp() {}

void KioskManagerTest::TearDown() {}

std::shared_ptr<AbilityRecord> KioskManagerTest::MockAbilityRecord(AbilityType abilityType)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    return AbilityRecord::CreateAbilityRecord(abilityRequest);
}

sptr<Token> KioskManagerTest::MockToken(AbilityType abilityType)
{
    abilityRecord = MockAbilityRecord(abilityType);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

sptr<SessionInfo> KioskManagerTest::MockSessionInfo(int32_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        TAG_LOGE(AAFwkTag::TEST, "sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    return sessionInfo;
}

AbilityRequest KioskManagerTest::GenerateAbilityRequest(const std::string& deviceName,
    const std::string& abilityName, const std::string& appName, const std::string& bundleName,
    const std::string& moduleName)
{
    ElementName element(deviceName, bundleName, abilityName, moduleName);
    Want want;
    want.SetElement(element);

    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = appName;
    abilityInfo.type = AbilityType::SERVICE;
    abilityInfo.name = abilityName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.moduleName = moduleName;
    abilityInfo.deviceId = deviceName;
    ApplicationInfo appinfo;
    appinfo.name = appName;
    abilityInfo.applicationInfo = appinfo;
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;
    abilityInfo.process = bundleName;

    return abilityRequest;
}

/*
 * Feature: KioskManager
 * Function: IsInKioskMode
 * SubFunction: NA
 * FunctionPoints: KioskManager IsInKioskMode
 */
HWTEST_F(KioskManagerTest, IsInKioskMode_001, TestSize.Level1) {
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = false;
    bool result = KioskManager::GetInstance().IsInKioskModeInner();
    EXPECT_EQ(result, false);

    result = KioskManager::GetInstance().IsInKioskMode();
    EXPECT_EQ(result, false);
}

/*
 * Feature: KioskManager
 * Function: IsInKioskMode
 * SubFunction: NA
 * FunctionPoints: KioskManager IsInKioskMode
 */
HWTEST_F(KioskManagerTest, IsInKioskMode_002, TestSize.Level1) {
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    bool result = KioskManager::GetInstance().IsInKioskModeInner();
    EXPECT_EQ(result, true);

    result = KioskManager::GetInstance().IsInKioskMode();
    EXPECT_EQ(result, true);
}

/*
 * Feature: KioskManager
 * Function: IsInWhiteList
 * SubFunction: NA
 * FunctionPoints: KioskManager IsInWhiteList
 */
HWTEST_F(KioskManagerTest, IsInWhiteList_001, TestSize.Level1) {
    std::string bundleName = "com.test.example";
    KioskManager::GetInstance().whitelist_.clear();
    bool result = KioskManager::GetInstance().IsInWhiteListInner(bundleName);
    EXPECT_EQ(result, false);

    result = KioskManager::GetInstance().IsInWhiteList(bundleName);
    EXPECT_EQ(result, false);
}

/*
 * Feature: KioskManager
 * Function: IsInWhiteList
 * SubFunction: NA
 * FunctionPoints: KioskManager IsInWhiteList
 */
HWTEST_F(KioskManagerTest, IsInWhiteList_002, TestSize.Level1) {
    std::string bundleName = "com.test.example";
    KioskManager::GetInstance().whitelist_.emplace(bundleName);
    bool result = KioskManager::GetInstance().IsInWhiteListInner(bundleName);
    EXPECT_EQ(result, true);

    result = KioskManager::GetInstance().IsInWhiteList(bundleName);
    EXPECT_EQ(result, true);
}

/*
 * Feature: KioskManager
 * Function: ExitKioskMode
 * SubFunction: NA
 * FunctionPoints: KioskManager ExitKioskMode
 */
HWTEST_F(KioskManagerTest, ExitKioskMode_001, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = false;
    auto callerToken = MockToken(AbilityType::PAGE);
    auto result = KioskManager::GetInstance().ExitKioskMode(callerToken, false);
    EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
}

/*
 * Feature: KioskManager
 * Function: ExitKioskMode
 * SubFunction: NA
 * FunctionPoints: KioskManager ExitKioskMode
 */
HWTEST_F(KioskManagerTest, ExitKioskMode_002, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    auto result = KioskManager::GetInstance().ExitKioskMode(nullptr, false);
    EXPECT_EQ(result, INVALID_PARAMETERS_ERR);
}

/*
 * Feature: KioskManager
 * Function: ExitKioskMode
 * SubFunction: NA
 * FunctionPoints: KioskManager ExitKioskMode
 */
HWTEST_F(KioskManagerTest, ExitKioskMode_003, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    auto callerToken = MockToken(AbilityType::PAGE);
    MyStatus::GetInstance().ipcGetCallingUid_ = KioskManager::GetInstance().kioskStatus_.kioskBundleUid_;
    auto result = KioskManager::GetInstance().ExitKioskMode(callerToken, false);
    EXPECT_EQ(result, ERR_KIOSK_MODE_NOT_IN_WHITELIST);
}

/*
 * Feature: KioskManager
 * Function: ExitKioskMode
 * SubFunction: NA
 * FunctionPoints: KioskManager ExitKioskMode
 */
HWTEST_F(KioskManagerTest, ExitKioskMode_004, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    auto callerToken = MockToken(AbilityType::PAGE);
    MyStatus::GetInstance().ipcGetCallingUid_ = KioskManager::GetInstance().kioskStatus_.kioskBundleUid_;
    auto result = KioskManager::GetInstance().ExitKioskMode(callerToken, true);
    EXPECT_EQ(result, ERR_KIOSK_MODE_NOT_IN_WHITELIST);
}

/*
 * Feature: KioskManager
 * Function: FilterDialogAppInfos
 * SubFunction: NA
 * FunctionPoints: KioskManager FilterDialogAppInfos
 */
HWTEST_F(KioskManagerTest, FilterDialogAppInfos_001, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = false;
    std::string bundleName = "com.test.demo";
    KioskManager::GetInstance().whitelist_.emplace(bundleName);
    std::vector<DialogAppInfo> dialogAppInfos;
    DialogAppInfo dialogAppInfo;
    dialogAppInfo.bundleName = bundleName;
    dialogAppInfos.emplace_back(dialogAppInfo);
    uint32_t size = dialogAppInfos.size();
    KioskManager::GetInstance().FilterDialogAppInfos(dialogAppInfos);
    EXPECT_EQ(dialogAppInfos.size(), size);
}

/*
 * Feature: KioskManager
 * Function: FilterDialogAppInfos
 * SubFunction: NA
 * FunctionPoints: KioskManager FilterDialogAppInfos
 */
HWTEST_F(KioskManagerTest, FilterDialogAppInfos_002, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    std::string bundleName = "com.test.demo";
    KioskManager::GetInstance().whitelist_.clear();
    std::vector<DialogAppInfo> dialogAppInfos;
    DialogAppInfo dialogAppInfo;
    dialogAppInfo.bundleName = bundleName;
    dialogAppInfos.emplace_back(dialogAppInfo);
    KioskManager::GetInstance().FilterDialogAppInfos(dialogAppInfos);
    EXPECT_TRUE(dialogAppInfos.empty());
}

/*
 * Feature: KioskManager
 * Function: FilterAbilityInfos
 * SubFunction: NA
 * FunctionPoints: KioskManager FilterAbilityInfos
 */
HWTEST_F(KioskManagerTest, FilterAbilityInfos_001, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = false;
    std::string bundleName = "com.test.demo";
    KioskManager::GetInstance().whitelist_.emplace(bundleName);
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = bundleName;
    abilityInfos.emplace_back(abilityInfo);
    uint32_t size = abilityInfos.size();
    KioskManager::GetInstance().FilterAbilityInfos(abilityInfos);
    EXPECT_EQ(abilityInfos.size(), size);
}

/*
 * Feature: KioskManager
 * Function: FilterAbilityInfos
 * SubFunction: NA
 * FunctionPoints: KioskManager FilterAbilityInfos
 */
HWTEST_F(KioskManagerTest, FilterAbilityInfos_002, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    std::string bundleName = "com.test.demo";
    KioskManager::GetInstance().whitelist_.clear();
    std::vector<AppExecFwk::AbilityInfo> abilityInfos;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = bundleName;
    abilityInfos.emplace_back(abilityInfo);
    KioskManager::GetInstance().FilterAbilityInfos(abilityInfos);
    EXPECT_TRUE(abilityInfos.empty());
}

/*
 * Feature: KioskManager
 * Function: ExitKioskMode
 * SubFunction: NA
 * FunctionPoints: KioskManager ExitKioskModeInner
 */
HWTEST_F(KioskManagerTest, ExitKioskModeInner_001, TestSize.Level1) {
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = false;
    auto callerToken = MockToken(AbilityType::PAGE);
    std::string bundleName = "com.test.example";
    KioskManager::GetInstance().whitelist_.emplace(bundleName);
    auto result = KioskManager::GetInstance().ExitKioskModeInner(bundleName, callerToken, false);
    EXPECT_EQ(result, ERR_NOT_IN_KIOSK_MODE);
}

/*
 * Feature: KioskManager
 * Function: GetKioskStatus
 * SubFunction: NA
 * FunctionPoints: KioskManager GetKioskStatus
 */
HWTEST_F(KioskManagerTest, GetKioskStatus_001, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = false;
    KioskStatus kioskStatus;
    auto result = KioskManager::GetInstance().GetKioskStatus(kioskStatus);
    EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
}

/*
 * Feature: KioskManager
 * Function: GetKioskStatus
 * SubFunction: NA
 * FunctionPoints: KioskManager GetKioskStatus
 */
HWTEST_F(KioskManagerTest, GetKioskStatus_002, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::flag_ = false;
    MyFlag::saFlag_ = false;
    MyFlag::permissionFlag_ = false;
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = false;
    KioskStatus kioskStatus;
    auto result = KioskManager::GetInstance().GetKioskStatus(kioskStatus);
    EXPECT_EQ(result, ERR_NOT_SYSTEM_APP);
}

/*
 * Feature: KioskManager
 * Function: GetKioskStatus
 * SubFunction: NA
 * FunctionPoints: KioskManager GetKioskStatus
 */
HWTEST_F(KioskManagerTest, GetKioskStatus_003, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::flag_ = false;
    MyFlag::saFlag_ = true;
    MyFlag::permissionFlag_ = false;
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = false;
    KioskStatus kioskStatus;
    auto result = KioskManager::GetInstance().GetKioskStatus(kioskStatus);
    EXPECT_EQ(result, ERR_NOT_SYSTEM_APP);
}

/*
 * Feature: KioskManager
 * Function: GetKioskStatus
 * SubFunction: NA
 * FunctionPoints: KioskManager GetKioskStatus
 */
HWTEST_F(KioskManagerTest, GetKioskStatus_004, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::flag_ = false;
    MyFlag::saFlag_ = true;
    MyFlag::permissionFlag_ = true;
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = false;
    KioskStatus kioskStatus;
    auto result = KioskManager::GetInstance().GetKioskStatus(kioskStatus);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: KioskManager
 * Function: GetKioskStatus
 * SubFunction: NA
 * FunctionPoints: KioskManager GetKioskStatus
 */
HWTEST_F(KioskManagerTest, GetKioskStatus_005, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::flag_ = true;
    MyFlag::saFlag_ = true;
    MyFlag::permissionFlag_ = true;
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = false;
    KioskStatus kioskStatus;
    auto result = KioskManager::GetInstance().GetKioskStatus(kioskStatus);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: KioskManager
 * Function: GetKioskStatus
 * SubFunction: NA
 * FunctionPoints: KioskManager GetKioskStatus
 */
HWTEST_F(KioskManagerTest, GetKioskStatus_006, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::flag_ = true;
    MyFlag::saFlag_ = false;
    MyFlag::permissionFlag_ = true;
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = false;
    KioskStatus kioskStatus;
    auto result = KioskManager::GetInstance().GetKioskStatus(kioskStatus);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: KioskManager
 * Function: EnterKioskMode
 * SubFunction: NA
 * FunctionPoints: KioskManager EnterKioskMode
 */
HWTEST_F(KioskManagerTest, EnterKioskMode_001, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = false;
    auto callerToken = MockToken(AbilityType::PAGE);
    auto result = KioskManager::GetInstance().EnterKioskMode(callerToken);
    EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
}

/*
 * Feature: KioskManager
 * Function: EnterKioskMode
 * SubFunction: NA
 * FunctionPoints: KioskManager EnterKioskMode
 */
HWTEST_F(KioskManagerTest, EnterKioskMode_002, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    auto result = KioskManager::GetInstance().EnterKioskMode(nullptr);
    EXPECT_EQ(result, INVALID_PARAMETERS_ERR);
}

/*
 * Feature: KioskManager
 * Function: UpdateKioskApplicationList
 * SubFunction: NA
 * FunctionPoints: KioskManager UpdateKioskApplicationList
 */
HWTEST_F(KioskManagerTest, UpdateKioskApplicationList_001, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = false;
    std::vector<std::string> appList;
    auto result = KioskManager::GetInstance().UpdateKioskApplicationList(appList);
    EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
}

/*
 * Feature: KioskManager
 * Function: KioskInterceptor
 * SubFunction: NA
 * FunctionPoints: KioskInterceptor DoProcess
 */
HWTEST_F(KioskManagerTest, KioskInterceptor_001, TestSize.Level1)
{
    auto kioskInterceptor = std::make_shared<KioskInterceptor>();
    Want want;
    want.SetElementName("com.example.test", "MainAbility");
    want.SetParam(AAFwk::SCREEN_MODE_KEY, ScreenMode::IDLE_SCREEN_MODE);
    want.SetAction("com.example.myapplication");
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, 0, 0, false, nullptr,
        shouldBlockFunc);
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = false;
    int32_t result = kioskInterceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: KioskManager
 * Function: KioskInterceptor
 * SubFunction: NA
 * FunctionPoints: KioskInterceptor DoProcess
 */
HWTEST_F(KioskManagerTest, KioskInterceptor_002, TestSize.Level1)
{
    auto kioskInterceptor = std::make_shared<KioskInterceptor>();
    Want want;
    want.SetElementName("com.example.test", "MainAbility");
    want.SetParam(AAFwk::SCREEN_MODE_KEY, ScreenMode::IDLE_SCREEN_MODE);
    want.SetAction("com.example.myapplication");
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, 0, 0, false, nullptr,
        shouldBlockFunc);
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    KioskManager::GetInstance().whitelist_.clear();
    int32_t result = kioskInterceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_KIOSK_MODE_NOT_IN_WHITELIST);
}

/*
 * Feature: KioskManager
 * Function: KioskInterceptor
 * SubFunction: NA
 * FunctionPoints: KioskInterceptor DoProcess
 */
HWTEST_F(KioskManagerTest, KioskInterceptor_003, TestSize.Level1)
{
    auto kioskInterceptor = std::make_shared<KioskInterceptor>();
    Want want;
    std::string bundleName = "com.test.example";
    want.SetElementName(bundleName, "MainAbility");
    want.SetParam(AAFwk::SCREEN_MODE_KEY, ScreenMode::IDLE_SCREEN_MODE);
    want.SetAction("com.example.myapplication");
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, 0, 0, false, nullptr,
        shouldBlockFunc);
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    KioskManager::GetInstance().whitelist_.emplace(bundleName);
    int32_t result = kioskInterceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: KioskManager
 * Function: KioskInterceptor
 * SubFunction: NA
 * FunctionPoints: KioskInterceptor DoProcess
 */
HWTEST_F(KioskManagerTest, KioskInterceptor_004, TestSize.Level1)
{
    auto kioskInterceptor = std::make_shared<KioskInterceptor>();
    Want want;
    std::string bundleName = "";
    want.SetElementName(bundleName, "MainAbility");
    want.SetParam(AAFwk::SCREEN_MODE_KEY, ScreenMode::IDLE_SCREEN_MODE);
    want.SetAction("com.example.myapplication");
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, 0, 0, false, nullptr,
        shouldBlockFunc);
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    int32_t result = kioskInterceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_KIOSK_MODE_NOT_IN_WHITELIST);
}

/*
 * Feature: KioskManager
 * Function: KioskInterceptor
 * SubFunction: NA
 * FunctionPoints: KioskInterceptor DoProcess
 */
HWTEST_F(KioskManagerTest, KioskInterceptor_005, TestSize.Level1)
{
    auto kioskInterceptor = std::make_shared<KioskInterceptor>();
    Want want;
    std::string bundleName = "com.test.example";
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, 0, 0, false, nullptr,
        shouldBlockFunc);
    int32_t result = kioskInterceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: KioskManager
 * Function: AddKioskInterceptor
 * SubFunction: NA
 * FunctionPoints: KioskManager AddKioskInterceptor
 */
HWTEST_F(KioskManagerTest, AddKioskInterceptor_001, TestSize.Level1) {
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    ASSERT_NE(abilityMgr, nullptr);
    abilityMgr->InitInterceptor();
    KioskManager::GetInstance().AddKioskInterceptor();
    KioskManager::GetInstance().GetEnterKioskModeCallback()();
    auto interceptorExecuter = abilityMgr->GetAbilityInterceptorExecuter();
    ASSERT_NE(interceptorExecuter, nullptr);
    EXPECT_NE(interceptorExecuter->interceptorMap_.count(KIOSK_WHITE_LIST), 0);
}

/*
 * Feature: KioskManager
 * Function: AddKioskInterceptor
 * SubFunction: NA
 * FunctionPoints: KioskManager AddKioskInterceptor
 */
HWTEST_F(KioskManagerTest, AddKioskInterceptor_002, TestSize.Level1) {
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    ASSERT_NE(abilityMgr, nullptr);
    abilityMgr->interceptorExecuter_ = nullptr;
    KioskManager::GetInstance().AddKioskInterceptor();
    auto interceptorExecuter = abilityMgr->GetAbilityInterceptorExecuter();
    ASSERT_EQ(interceptorExecuter, nullptr);
}

/*
 * Feature: KioskManager
 * Function: RemoveKioskInterceptor
 * SubFunction: NA
 * FunctionPoints: KioskManager RemoveKioskInterceptor
 */
HWTEST_F(KioskManagerTest, RemoveKioskInterceptor_001, TestSize.Level1) {
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    ASSERT_NE(abilityMgr, nullptr);
    abilityMgr->InitInterceptor();
    KioskManager::GetInstance().AddKioskInterceptor();
    KioskManager::GetInstance().GetExitKioskModeCallback()();
    KioskManager::GetInstance().RemoveKioskInterceptor();
    auto interceptorExecuter = abilityMgr->GetAbilityInterceptorExecuter();
    ASSERT_NE(interceptorExecuter, nullptr);
    EXPECT_EQ(interceptorExecuter->interceptorMap_.count(KIOSK_WHITE_LIST), 0);
}

/*
 * Feature: KioskManager
 * Function: RemoveKioskInterceptor
 * SubFunction: NA
 * FunctionPoints: KioskManager RemoveKioskInterceptor
 */
HWTEST_F(KioskManagerTest, RemoveKioskInterceptor_002, TestSize.Level1) {
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    ASSERT_NE(abilityMgr, nullptr);
    abilityMgr->interceptorExecuter_ = nullptr;
    KioskManager::GetInstance().RemoveKioskInterceptor();
    auto interceptorExecuter = abilityMgr->GetAbilityInterceptorExecuter();
    ASSERT_EQ(interceptorExecuter, nullptr);
}

/*
 * Feature: KioskManager
 * Function: UpdateKioskApplicationList
 * SubFunction: NA
 * FunctionPoints: KioskManager UpdateKioskApplicationList
 */
HWTEST_F(KioskManagerTest, UpdateKioskApplicationList_002, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::flag_ = true;
    MyFlag::permissionFlag_ = true;
    std::vector<std::string> appList;
    KioskManager::GetInstance().whitelist_.clear();
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    bool result = KioskManager::GetInstance().IsInKioskModeInner();
    EXPECT_EQ(result, true);
    auto ret = KioskManager::GetInstance().UpdateKioskApplicationList(appList);
    EXPECT_EQ(ret, ERR_KIOSK_MODE_NOT_IN_WHITELIST);
}

/*
 * Feature: KioskManager
 * Function: ExitKioskMode
 * SubFunction: NA
 * FunctionPoints: KioskManager ExitKioskModeInner
 */
HWTEST_F(KioskManagerTest, ExitKioskModeInner_003, TestSize.Level1) {
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    auto callerToken = MockToken(AbilityType::PAGE);
    std::string bundleName = "com.test.demo";
    KioskManager::GetInstance().whitelist_.emplace(bundleName);
    KioskManager::GetInstance().kioskStatus_.kioskBundleName_ = bundleName;
    MyStatus::GetInstance().ipcGetCallingUid_ = KioskManager::GetInstance().kioskStatus_.kioskBundleUid_ + 1;
    auto result = KioskManager::GetInstance().ExitKioskModeInner(bundleName, callerToken, false);
    EXPECT_EQ(result, ERR_NOT_IN_KIOSK_MODE);
}

/*
 * Feature: KioskManager
 * Function: ExitKioskMode
 * SubFunction: NA
 * FunctionPoints: KioskManager ExitKioskModeInner
 */
HWTEST_F(KioskManagerTest, ExitKioskModeInner_004, TestSize.Level1) {
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    auto callerToken = MockToken(AbilityType::PAGE);
    std::string bundleName = "com.test.demo";
    KioskManager::GetInstance().whitelist_.emplace(bundleName);
    KioskManager::GetInstance().kioskStatus_.kioskBundleName_ = bundleName;
    MyStatus::GetInstance().ipcGetCallingUid_ = KioskManager::GetInstance().kioskStatus_.kioskBundleUid_;
    auto result = KioskManager::GetInstance().ExitKioskModeInner(bundleName, callerToken, false);
    EXPECT_NE(result, ERR_NOT_IN_KIOSK_MODE);
}

/*
 * Feature: KioskManager
 * Function: ExitKioskMode
 * SubFunction: NA
 * FunctionPoints: KioskManager ExitKioskModeInner
 */
HWTEST_F(KioskManagerTest, ExitKioskModeInner_005, TestSize.Level1) {
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    auto callerToken = MockToken(AbilityType::PAGE);
    std::string bundleName = "com.test.demo";
    KioskManager::GetInstance().whitelist_.emplace(bundleName);
    KioskManager::GetInstance().kioskStatus_.kioskBundleName_ = bundleName;
    MyStatus::GetInstance().ipcGetCallingUid_ = KioskManager::GetInstance().kioskStatus_.kioskBundleUid_ + 1;
    auto result = KioskManager::GetInstance().ExitKioskModeInner(bundleName, callerToken, true);
    EXPECT_NE(result, ERR_NOT_IN_KIOSK_MODE);
}

/*
 * Feature: KioskManager
 * Function: ExitKioskMode
 * SubFunction: NA
 * FunctionPoints: KioskManager ExitKioskModeInner
 */
HWTEST_F(KioskManagerTest, ExitKioskModeInner_006, TestSize.Level1) {
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    auto callerToken = MockToken(AbilityType::PAGE);
    std::string bundleName = "com.test.demo";
    KioskManager::GetInstance().whitelist_.emplace(bundleName);
    KioskManager::GetInstance().kioskStatus_.kioskBundleName_ = bundleName;
    MyStatus::GetInstance().ipcGetCallingUid_ = KioskManager::GetInstance().kioskStatus_.kioskBundleUid_;
    auto result = KioskManager::GetInstance().ExitKioskModeInner(bundleName, callerToken, true);
    EXPECT_NE(result, ERR_NOT_IN_KIOSK_MODE);
}

/*
 * Feature: KioskManager
 * Function: IsKioskBundleUid
 * FunctionPoints: KioskManager IsKioskBundleUid
 */
HWTEST_F(KioskManagerTest, IsKioskBundleUid_001, TestSize.Level1) {
    int32_t uid = 20010080;
    KioskManager::GetInstance().kioskStatus_.kioskBundleUid_ = uid;
    bool result = KioskManager::GetInstance().IsKioskBundleUid(uid);
    EXPECT_TRUE(result);

    uid = uid + 1;
    result = KioskManager::GetInstance().IsKioskBundleUid(uid);
    EXPECT_FALSE(result);
}
} // namespace AAFwk
} // namespace OHOS
