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
#include "ws_common.h"

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
    abilityRequest.abilityInfo.bundleName = "com.test.demo";
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
    auto result = KioskManager::GetInstance().EnterKioskMode(callerToken, static_cast<int32_t>(
        Rosen::KioskType::DEFAULT));
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
    auto result = KioskManager::GetInstance().EnterKioskMode(nullptr, static_cast<int32_t>(Rosen::KioskType::DEFAULT));
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
    EXPECT_EQ(ret, ERR_NOT_SYSTEM_APP);
}

/*
 * Feature: KioskManager
 * Function: UpdateKioskApplicationList
 * SubFunction: NA
 * FunctionPoints: KioskManager UpdateKioskApplicationList
 */
HWTEST_F(KioskManagerTest, UpdateKioskApplicationList_003, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::flag_ = true;
    MyFlag::permissionFlag_ = true;
    std::vector<std::string> appList;
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    KioskManager::GetInstance().NotifyKioskModeChanged(true, "", 0, static_cast<int32_t>(Rosen::KioskType::DEFAULT));
    std::string bundleName = "com.test.demo";
    appList.emplace_back(bundleName);
    KioskManager::GetInstance().kioskStatus_.kioskBundleName_ = bundleName;
    auto ret = KioskManager::GetInstance().UpdateKioskApplicationList(appList);
    EXPECT_EQ(ret, INNER_ERR);
}

/*
 * Feature: KioskManager
 * Function: ExitKioskMode
 * SubFunction: NA
 * FunctionPoints: KioskManager ExitKioskModeInner
 */
HWTEST_F(KioskManagerTest, ExitKioskModeInner_002, TestSize.Level1) {
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    auto callerToken = MockToken(AbilityType::PAGE);
    std::string bundleName = "com.test.demo";
    KioskManager::GetInstance().whitelist_.emplace(bundleName);
    KioskManager::GetInstance().NotifyKioskModeChanged(false, bundleName, 0, static_cast<int32_t>(
        Rosen::KioskType::DEFAULT));
    AppInfo appInfo;
    appInfo.bundleName = bundleName;
    appInfo.state = AppState::BEGIN;
    KioskManager::GetInstance().OnAppStop(appInfo);
    MyStatus::GetInstance().ipcGetCallingUid_ = KioskManager::GetInstance().kioskStatus_.kioskBundleUid_;
    auto result = KioskManager::GetInstance().ExitKioskModeInner(bundleName, callerToken, false);
    EXPECT_EQ(result, INNER_ERR);
}

/**
 * @tc.name: ExitKioskModeInner_003
 * @tc.desc: caller is not the entering SA (callingUid != kioskCallerUid_) -> CHECK_PERMISSION_FAILED
 */
HWTEST_F(KioskManagerTest, ExitKioskModeInner_003, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::saFlag_ = MyFlag::FLAG::IS_SA_CALL;
    MyFlag::permissionFlag_ = 1;
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    auto callerToken = MockToken(AbilityType::PAGE);
    std::string bundleName = "com.test.demo";
    KioskManager::GetInstance().whitelist_.emplace(bundleName);
    KioskManager::GetInstance().kioskStatus_.kioskBundleName_ = bundleName;
    MyStatus::GetInstance().ipcGetCallingUid_ = KioskManager::GetInstance().kioskStatus_.kioskBundleUid_ + 1;
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    auto result = KioskManager::GetInstance().ExitKioskModeInner(bundleName, callerToken, false);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
    KioskManager::GetInstance().kioskStatus_.Clear();
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
}

/**
 * @tc.name: ExitKioskModeInner_004
 * @tc.desc: self-exit (callingUid == kioskBundleUid) proceeds past UID check
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

/**
 * @tc.name: ExitKioskModeInner_007
 * @tc.desc: SA proxy exit, not SA call -> ERR_NOT_IN_KIOSK_MODE
 */
HWTEST_F(KioskManagerTest, ExitKioskModeInner_007, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::saFlag_ = 0;
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    auto callerToken = MockToken(AbilityType::PAGE);
    std::string bundleName = "com.test.demo";
    KioskManager::GetInstance().whitelist_.emplace(bundleName);
    KioskManager::GetInstance().kioskStatus_.kioskBundleName_ = bundleName;
    MyStatus::GetInstance().ipcGetCallingUid_ = KioskManager::GetInstance().kioskStatus_.kioskBundleUid_ + 1;
    auto result = KioskManager::GetInstance().ExitKioskModeInner(bundleName, callerToken, false);
    EXPECT_EQ(result, ERR_NOT_IN_KIOSK_MODE);
    MyFlag::saFlag_ = MyFlag::FLAG::IS_SA_CALL;
    KioskManager::GetInstance().kioskStatus_.Clear();
}

/**
 * @tc.name: ExitKioskModeInner_008
 * @tc.desc: caller is the entering SA (callingUid == kioskCallerUid_) -> proceeds
 */
HWTEST_F(KioskManagerTest, ExitKioskModeInner_008, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::saFlag_ = MyFlag::FLAG::IS_SA_CALL;
    MyFlag::permissionFlag_ = 1;
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    auto callerToken = MockToken(AbilityType::PAGE);
    std::string bundleName = "com.test.demo";
    KioskManager::GetInstance().whitelist_.emplace(bundleName);
    KioskManager::GetInstance().kioskStatus_.kioskBundleName_ = bundleName;
    KioskManager::GetInstance().kioskStatus_.kioskBundleUid_ = 0;
    KioskManager::GetInstance().kioskStatus_.kioskCallerUid_ = 1; // entering SA uid == callingUid
    MyStatus::GetInstance().ipcGetCallingUid_ = 1;
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    KioskManager::GetInstance().kioskWhitelistMap_[1] = {"com.test.demo"};
    auto result = KioskManager::GetInstance().ExitKioskModeInner(bundleName, callerToken, false);
    EXPECT_TRUE(result == INNER_ERR || result == ERR_OK);
    KioskManager::GetInstance().kioskStatus_.Clear();
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
}

/**
 * @tc.name: ExitKioskModeInner_009
 * @tc.desc: proxy-entered kiosk, app self-exit (callingUid==kioskBundleUid_) -> blocked by isProxyEnter_ guard
 */
HWTEST_F(KioskManagerTest, ExitKioskModeInner_009, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::saFlag_ = 0; // app is not SA
    KioskManager::GetInstance().kioskStatus_.Clear();
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    KioskManager::GetInstance().kioskStatus_.isProxyEnter_ = true;
    KioskManager::GetInstance().kioskStatus_.kioskBundleUid_ = 0;
    KioskManager::GetInstance().kioskStatus_.kioskCallerUid_ = 1;
    KioskManager::GetInstance().kioskStatus_.kioskBundleName_ = "com.test.demo";
    auto callerToken = MockToken(AbilityType::PAGE);
    std::string bundleName = "com.test.demo";
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    KioskManager::GetInstance().kioskWhitelistMap_[1] = {"com.test.demo"};
    MyStatus::GetInstance().ipcGetCallingUid_ = 0; // == kioskBundleUid_ => app self-exit
    auto result = KioskManager::GetInstance().ExitKioskModeInner(bundleName, callerToken, false);
    EXPECT_EQ(result, ERR_NOT_IN_KIOSK_MODE); // blocked: app can't self-exit proxy-entered kiosk
    EXPECT_TRUE(KioskManager::GetInstance().kioskStatus_.isKioskMode_); // still in kiosk
    MyFlag::saFlag_ = MyFlag::FLAG::IS_SA_CALL;
    KioskManager::GetInstance().kioskStatus_.Clear();
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
}

/**
 * @tc.name: ExitKioskModeInner_010
 * @tc.desc: proxy-entered kiosk, SA exit (callingUid!=kioskBundleUid_, SA+perm+whitelist) -> proceeds
 */
HWTEST_F(KioskManagerTest, ExitKioskModeInner_010, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::saFlag_ = MyFlag::FLAG::IS_SA_CALL;
    MyFlag::permissionFlag_ = 1;
    KioskManager::GetInstance().kioskStatus_.Clear();
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    KioskManager::GetInstance().kioskStatus_.isProxyEnter_ = true;
    KioskManager::GetInstance().kioskStatus_.kioskBundleUid_ = 0;
    KioskManager::GetInstance().kioskStatus_.kioskCallerUid_ = 1;
    KioskManager::GetInstance().kioskStatus_.kioskBundleName_ = "com.test.demo";
    auto callerToken = MockToken(AbilityType::PAGE);
    std::string bundleName = "com.test.demo";
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    KioskManager::GetInstance().kioskWhitelistMap_[1] = {"com.test.demo"};
    MyStatus::GetInstance().ipcGetCallingUid_ = 1; // SA uid != kioskBundleUid_(0)
    auto result = KioskManager::GetInstance().ExitKioskModeInner(bundleName, callerToken, false);
    EXPECT_TRUE(result == INNER_ERR || result == ERR_OK); // proceeds (WMS null in test)
    EXPECT_FALSE(KioskManager::GetInstance().kioskStatus_.isKioskMode_); // kiosk exited
    KioskManager::GetInstance().kioskStatus_.Clear();
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
}

/**
 * @tc.name: ExitKioskModeInner_011
 * @tc.desc: proxy-entered by SA-A, SA-B (different uid, has perm + bundle in own whitelist) tries to exit
 *           -> blocked by callingUid != kioskCallerUid_ (only entering SA can exit)
 */
HWTEST_F(KioskManagerTest, ExitKioskModeInner_011, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::saFlag_ = MyFlag::FLAG::IS_SA_CALL;
    MyFlag::permissionFlag_ = 1;
    KioskManager::GetInstance().kioskStatus_.Clear();
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    KioskManager::GetInstance().kioskStatus_.isProxyEnter_ = true;
    KioskManager::GetInstance().kioskStatus_.kioskBundleUid_ = 0;
    KioskManager::GetInstance().kioskStatus_.kioskCallerUid_ = 1; // entered by SA-A (uid=1)
    KioskManager::GetInstance().kioskStatus_.kioskBundleName_ = "com.test.demo";
    auto callerToken = MockToken(AbilityType::PAGE);
    std::string bundleName = "com.test.demo";
    // active whitelist (SA-A's slot) has the bundle for IsInWhiteListInner
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    KioskManager::GetInstance().kioskWhitelistMap_[1] = {"com.test.demo"};
    // SA-B also has the bundle in its own slot (data isolation) — should NOT grant exit
    KioskManager::GetInstance().kioskWhitelistMap_[2] = {"com.test.demo"};
    MyStatus::GetInstance().ipcGetCallingUid_ = 2; // SA-B calling, != entering SA (uid=1)
    auto result = KioskManager::GetInstance().ExitKioskModeInner(bundleName, callerToken, false);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED); // blocked: not the entering SA
    EXPECT_TRUE(KioskManager::GetInstance().kioskStatus_.isKioskMode_); // still in kiosk
    KioskManager::GetInstance().kioskStatus_.Clear();
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
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

/**
 * @tc.name: AddKioskApplicationList_001
 * @tc.desc: null applist -> ERR_OK
 */
HWTEST_F(KioskManagerTest, AddKioskApplicationList_001, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = false;
    std::vector<std::string> appList;
    auto result = KioskManager::GetInstance().AddKioskApplicationList(appList);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AddKioskApplicationList_002
 * @tc.desc: not SA call -> ERR_NOT_SYSTEM_APP
 */
HWTEST_F(KioskManagerTest, AddKioskApplicationList_002, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::saFlag_ = 0;
    std::vector<std::string> appList;
    auto result = KioskManager::GetInstance().AddKioskApplicationList(appList);
    EXPECT_EQ(result, ERR_NOT_SYSTEM_APP);
    MyFlag::saFlag_ = MyFlag::FLAG::IS_SA_CALL;
}

/**
 * @tc.name: AddKioskApplicationList_003
 * @tc.desc: perm ok, appends before WMS notify; WMS null -> NE(ERR_OK) but apps ARE stored
 */
HWTEST_F(KioskManagerTest, AddKioskApplicationList_003, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::saFlag_ = MyFlag::FLAG::IS_SA_CALL;
    MyFlag::permissionFlag_ = 1;
    MyStatus::GetInstance().ipcGetCallingUid_ = 1;
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    std::vector<std::string> appList = {"com.test.demo", "com.test.exam"};
    auto result = KioskManager::GetInstance().AddKioskApplicationList(appList);
    EXPECT_NE(result, ERR_OK); // WMS null -> INNER_ERR
    // append happens before WMS notify => apps are stored
    EXPECT_EQ(KioskManager::GetInstance().kioskWhitelistMap_[1].count("com.test.demo"), 1UL);
    EXPECT_EQ(KioskManager::GetInstance().kioskWhitelistMap_[1].count("com.test.exam"), 1UL);
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
}

/**
 * @tc.name: AddKioskApplicationList_004
 * @tc.desc: two Add calls accumulate (append, not replace); existing entries preserved
 */
HWTEST_F(KioskManagerTest, AddKioskApplicationList_004, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::saFlag_ = MyFlag::FLAG::IS_SA_CALL;
    MyFlag::permissionFlag_ = 1;
    MyStatus::GetInstance().ipcGetCallingUid_ = 1;
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    std::vector<std::string> firstBatch = {"com.test.demo"};
    KioskManager::GetInstance().AddKioskApplicationList(firstBatch);
    std::vector<std::string> secondBatch = {"com.test.exam"};
    auto result = KioskManager::GetInstance().AddKioskApplicationList(secondBatch);
    EXPECT_NE(result, ERR_OK); // WMS null
    // both batches present (append, not replace)
    EXPECT_EQ(KioskManager::GetInstance().kioskWhitelistMap_[1].count("com.test.demo"), 1UL);
    EXPECT_EQ(KioskManager::GetInstance().kioskWhitelistMap_[1].count("com.test.exam"), 1UL);
    EXPECT_EQ(KioskManager::GetInstance().kioskWhitelistMap_[1].size(), 2UL);
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
}

/**
 * @tc.name: AddKioskApplicationList_005
 * @tc.desc: empty appList => no-op (ERR_OK), existing entries untouched, no WMS call
 */
HWTEST_F(KioskManagerTest, AddKioskApplicationList_005, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::saFlag_ = MyFlag::FLAG::IS_SA_CALL;
    MyFlag::permissionFlag_ = 1;
    MyStatus::GetInstance().ipcGetCallingUid_ = 1;
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    KioskManager::GetInstance().kioskWhitelistMap_[1] = {"com.existing"};
    std::vector<std::string> emptyList;
    auto result = KioskManager::GetInstance().AddKioskApplicationList(emptyList);
    EXPECT_EQ(result, ERR_OK); // no-op
    // existing entries untouched
    EXPECT_EQ(KioskManager::GetInstance().kioskWhitelistMap_[1].count("com.existing"), 1UL);
    EXPECT_EQ(KioskManager::GetInstance().kioskWhitelistMap_[1].size(), 1UL);
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
}

/**
 * @tc.name: DeleteKioskApplicationList_001
 * @tc.desc: null appList -> ERR_OK
 */
HWTEST_F(KioskManagerTest, DeleteKioskApplicationList_001, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = false;
    std::vector<std::string> appList;
    auto result = KioskManager::GetInstance().DeleteKioskApplicationList(appList);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: DeleteKioskApplicationList_002
 * @tc.desc: not SA call -> ERR_NOT_SYSTEM_APP
 */
HWTEST_F(KioskManagerTest, DeleteKioskApplicationList_002, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::saFlag_ = 0;
    std::vector<std::string> appList;
    auto result = KioskManager::GetInstance().DeleteKioskApplicationList(appList);
    EXPECT_EQ(result, ERR_NOT_SYSTEM_APP);
    MyFlag::saFlag_ = MyFlag::FLAG::IS_SA_CALL;
}

/**
 * @tc.name: DeleteKioskApplicationList_003
 * @tc.desc: empty appList => no-op (slot not cleared), returns ERR_OK
 */
HWTEST_F(KioskManagerTest, DeleteKioskApplicationList_003, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::saFlag_ = MyFlag::FLAG::IS_SA_CALL;
    MyFlag::permissionFlag_ = 1;
    MyStatus::GetInstance().ipcGetCallingUid_ = 1;
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    KioskManager::GetInstance().kioskWhitelistMap_[1] = {"com.test.demo"};
    std::vector<std::string> emptyList;
    auto result = KioskManager::GetInstance().DeleteKioskApplicationList(emptyList);
    EXPECT_EQ(result, ERR_OK);
    // slot NOT cleared — entry still present
    EXPECT_EQ(KioskManager::GetInstance().kioskWhitelistMap_[1].count("com.test.demo"), 1UL);
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
}

/**
 * @tc.name: DeleteKioskApplicationList_004
 * @tc.desc: non-empty, not in kiosk; WMS null in test env -> NE(ERR_OK); whitelist unchanged (mutate after notify)
 */
HWTEST_F(KioskManagerTest, DeleteKioskApplicationList_004, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::saFlag_ = MyFlag::FLAG::IS_SA_CALL;
    MyFlag::permissionFlag_ = 1;
    MyStatus::GetInstance().ipcGetCallingUid_ = 1;
    KioskManager::GetInstance().kioskStatus_.Clear();
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    KioskManager::GetInstance().kioskWhitelistMap_[1] = {"com.test.demo", "com.test.exam"};
    KioskManager::GetInstance().kioskWhitelistMap_[2] = {"com.other.sa"};
    std::vector<std::string> toDelete = {"com.test.demo"};
    auto result = KioskManager::GetInstance().DeleteKioskApplicationList(toDelete);
    EXPECT_NE(result, ERR_OK); // WMS null -> INNER_ERR before mutate
    // not in kiosk => no exit; WMS fails before mutate => whitelist unchanged
    EXPECT_EQ(KioskManager::GetInstance().kioskWhitelistMap_[1].count("com.test.demo"), 1UL);
    EXPECT_EQ(KioskManager::GetInstance().kioskWhitelistMap_[2].count("com.other.sa"), 1UL);
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    KioskManager::GetInstance().kioskStatus_.Clear();
}

/**
 * @tc.name: DeleteKioskApplicationList_005
 * @tc.desc: in kiosk (proxy-entered by this SA), kiosk app in delete list -> exit triggered before mutate
 */
HWTEST_F(KioskManagerTest, DeleteKioskApplicationList_005, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::saFlag_ = MyFlag::FLAG::IS_SA_CALL;
    MyFlag::permissionFlag_ = 1;
    MyStatus::GetInstance().ipcGetCallingUid_ = 1;
    KioskManager::GetInstance().kioskStatus_.Clear();
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    auto token = MockToken(AbilityType::PAGE);
    ASSERT_NE(token, nullptr);
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    KioskManager::GetInstance().kioskStatus_.isProxyEnter_ = true;
    KioskManager::GetInstance().kioskStatus_.kioskCallerUid_ = 1;
    KioskManager::GetInstance().kioskStatus_.kioskBundleName_ = "com.test.demo";
    KioskManager::GetInstance().kioskStatus_.kioskToken_ = token;
    KioskManager::GetInstance().kioskWhitelistMap_[1] = {"com.test.demo", "com.test.exam"};
    std::vector<std::string> toDelete = {"com.test.demo"};
    auto result = KioskManager::GetInstance().DeleteKioskApplicationList(toDelete);
    EXPECT_NE(result, ERR_OK); // ExitKioskModeInner WMS null -> INNER_ERR
    // kiosk exited (Clear happened before WMS call) even though WMS failed
    EXPECT_FALSE(KioskManager::GetInstance().kioskStatus_.isKioskMode_);
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    KioskManager::GetInstance().kioskStatus_.Clear();
}

/**
 * @tc.name: DeleteKioskApplicationList_006
 * @tc.desc: in kiosk (proxy-entered by DIFFERENT SA) -> scoped, no false exit; kiosk stays active
 */
HWTEST_F(KioskManagerTest, DeleteKioskApplicationList_006, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::saFlag_ = MyFlag::FLAG::IS_SA_CALL;
    MyFlag::permissionFlag_ = 1;
    MyStatus::GetInstance().ipcGetCallingUid_ = 1; // SA-1 calling
    KioskManager::GetInstance().kioskStatus_.Clear();
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    KioskManager::GetInstance().kioskStatus_.isProxyEnter_ = true;
    KioskManager::GetInstance().kioskStatus_.kioskCallerUid_ = 2; // entered by SA-2
    KioskManager::GetInstance().kioskStatus_.kioskBundleName_ = "com.other.app";
    KioskManager::GetInstance().kioskWhitelistMap_[1] = {"com.test.demo"};
    std::vector<std::string> toDelete = {"com.test.demo"};
    auto result = KioskManager::GetInstance().DeleteKioskApplicationList(toDelete);
    EXPECT_NE(result, ERR_OK); // WMS null in NotifyWms
    // scoped: callerUid(1) != kioskCallerUid_(2) => no false exit
    EXPECT_TRUE(KioskManager::GetInstance().kioskStatus_.isKioskMode_);
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    KioskManager::GetInstance().kioskStatus_.Clear();
}

/**
 * @tc.name: CheckAndExitIfOutOfActiveList_001
 * @tc.desc: not in kiosk -> ERR_OK (no exit)
 */
HWTEST_F(KioskManagerTest, CheckAndExitIfOutOfActiveList_001, TestSize.Level1) {
    KioskManager::GetInstance().kioskStatus_.Clear();
    std::vector<std::string> appList = {"com.test.demo"};
    auto result = KioskManager::GetInstance().CheckAndExitIfOutOfActiveList(1, appList);
    EXPECT_EQ(result, ERR_OK);
    KioskManager::GetInstance().kioskStatus_.Clear();
}

/**
 * @tc.name: CheckAndExitIfOutOfActiveList_002
 * @tc.desc: self-entered (isProxyEnter_=false) -> not affected by per-uid ops -> ERR_OK, no exit
 */
HWTEST_F(KioskManagerTest, CheckAndExitIfOutOfActiveList_002, TestSize.Level1) {
    KioskManager::GetInstance().kioskStatus_.Clear();
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    KioskManager::GetInstance().kioskStatus_.isProxyEnter_ = false;
    KioskManager::GetInstance().kioskStatus_.kioskBundleName_ = "com.test.demo";
    std::vector<std::string> appList = {"com.other"};
    auto result = KioskManager::GetInstance().CheckAndExitIfOutOfActiveList(1, appList);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(KioskManager::GetInstance().kioskStatus_.isKioskMode_);
    KioskManager::GetInstance().kioskStatus_.Clear();
}

/**
 * @tc.name: CheckAndExitIfOutOfActiveList_003
 * @tc.desc: proxy-entered by different SA (callerUid != kioskCallerUid_) -> scoped, no exit
 */
HWTEST_F(KioskManagerTest, CheckAndExitIfOutOfActiveList_003, TestSize.Level1) {
    KioskManager::GetInstance().kioskStatus_.Clear();
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    KioskManager::GetInstance().kioskStatus_.isProxyEnter_ = true;
    KioskManager::GetInstance().kioskStatus_.kioskCallerUid_ = 2; // entered by SA-2
    KioskManager::GetInstance().kioskStatus_.kioskBundleName_ = "com.test.demo";
    std::vector<std::string> appList = {"com.other"};
    auto result = KioskManager::GetInstance().CheckAndExitIfOutOfActiveList(1, appList); // SA-1 calling
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(KioskManager::GetInstance().kioskStatus_.isKioskMode_);
    KioskManager::GetInstance().kioskStatus_.Clear();
}

/**
 * @tc.name: CheckAndExitIfOutOfActiveList_004
 * @tc.desc: active SA, kiosk app NOT in appList -> exit triggered; kioskStatus cleared before WMS
 */
HWTEST_F(KioskManagerTest, CheckAndExitIfOutOfActiveList_004, TestSize.Level1) {
    KioskManager::GetInstance().kioskStatus_.Clear();
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    auto token = MockToken(AbilityType::PAGE);
    ASSERT_NE(token, nullptr);
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    KioskManager::GetInstance().kioskStatus_.isProxyEnter_ = true;
    KioskManager::GetInstance().kioskStatus_.kioskCallerUid_ = 1;
    KioskManager::GetInstance().kioskStatus_.kioskBundleName_ = "com.test.demo";
    KioskManager::GetInstance().kioskStatus_.kioskToken_ = token;
    KioskManager::GetInstance().kioskWhitelistMap_[1] = {"com.test.demo"}; // IsInWhiteListInner passes
    std::vector<std::string> appList = {"com.other"}; // kiosk app NOT in list
    auto result = KioskManager::GetInstance().CheckAndExitIfOutOfActiveList(1, appList);
    EXPECT_EQ(result, ERR_OK); // ExitKioskModeInner WMS null -> INNER_ERR
    EXPECT_FALSE(KioskManager::GetInstance().kioskStatus_.isKioskMode_); // cleared before WMS
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    KioskManager::GetInstance().kioskStatus_.Clear();
}

/**
 * @tc.name: CheckAndExitIfOutOfActiveList_005
 * @tc.desc: active SA, kiosk app IN appList -> no exit; kiosk stays active
 */
HWTEST_F(KioskManagerTest, CheckAndExitIfOutOfActiveList_005, TestSize.Level1) {
    KioskManager::GetInstance().kioskStatus_.Clear();
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    auto token = MockToken(AbilityType::PAGE);
    ASSERT_NE(token, nullptr);
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    KioskManager::GetInstance().kioskStatus_.isProxyEnter_ = true;
    KioskManager::GetInstance().kioskStatus_.kioskCallerUid_ = 1;
    KioskManager::GetInstance().kioskStatus_.kioskBundleName_ = "com.test.demo";
    KioskManager::GetInstance().kioskStatus_.kioskToken_ = token;
    KioskManager::GetInstance().kioskWhitelistMap_[1] = {"com.test.demo"};
    std::vector<std::string> appList = {"com.test.demo", "com.other"}; // kiosk app IN list
    auto result = KioskManager::GetInstance().CheckAndExitIfOutOfActiveList(1, appList);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(KioskManager::GetInstance().kioskStatus_.isKioskMode_);
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    KioskManager::GetInstance().kioskStatus_.Clear();
}

/**
 * @tc.name: EnterKioskMode_SAProxy_001
 * @tc.desc: SA proxy enter, target not in caller whitelist -> ERR_KIOSK_MODE_NOT_IN_WHITELIST
 */
HWTEST_F(KioskManagerTest, EnterKioskMode_SAProxy_001, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::saFlag_ = MyFlag::FLAG::IS_SA_CALL;
    MyFlag::permissionFlag_ = 1;
    MyStatus::GetInstance().ipcGetCallingUid_ = 1; // != record.uid(0) => proxy path
    KioskManager::GetInstance().kioskStatus_.Clear();
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    auto callerToken = MockToken(AbilityType::PAGE);
    ASSERT_NE(callerToken, nullptr);
    // caller(1) whitelist empty => target not whitelisted
    auto result = KioskManager::GetInstance().EnterKioskMode(callerToken, static_cast<int32_t>(
        Rosen::KioskType::DEFAULT));
    EXPECT_EQ(result, ERR_KIOSK_MODE_NOT_IN_WHITELIST);
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    KioskManager::GetInstance().kioskStatus_.Clear();
}

/**
 * @tc.name: EnterKioskMode_SAProxy_002
 * @tc.desc: SA proxy enter with target whitelisted; kioskType stored & isProxyEnter set
 */
HWTEST_F(KioskManagerTest, EnterKioskMode_SAProxy_002, TestSize.Level1) {
    MyStatus::GetInstance().paramGetBoolParameter_ = true;
    MyFlag::saFlag_ = MyFlag::FLAG::IS_SA_CALL;
    MyFlag::permissionFlag_ = 1;
    MyStatus::GetInstance().ipcGetCallingUid_ = 1; // != record.uid(0) => proxy path
    KioskManager::GetInstance().kioskStatus_.Clear();
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    auto callerToken = MockToken(AbilityType::PAGE);
    ASSERT_NE(callerToken, nullptr);
    KioskManager::GetInstance().kioskWhitelistMap_[1] = {"com.test.demo"};
    auto result = KioskManager::GetInstance().EnterKioskMode(callerToken, static_cast<int32_t>(
        Rosen::KioskType::EXAM_MODE));
    EXPECT_TRUE(result == INNER_ERR || result == ERR_OK);
    // kioskType stored from SA input (EXAM_MODE), proxy enter recorded
    EXPECT_EQ(KioskManager::GetInstance().kioskStatus_.kioskType_, static_cast<int32_t>(Rosen::KioskType::EXAM_MODE));
    EXPECT_EQ(KioskManager::GetInstance().kioskStatus_.isProxyEnter_, true);
    EXPECT_EQ(KioskManager::GetInstance().kioskStatus_.kioskCallerUid_, 1);
    KioskManager::GetInstance().kioskWhitelistMap_.clear();
    KioskManager::GetInstance().kioskStatus_.Clear();
}

} // namespace AAFwk
} // namespace OHOS
