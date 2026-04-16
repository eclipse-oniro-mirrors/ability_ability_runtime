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

#define private public
#include "app_running_manager.h"
#include "app_running_record.h"
#include "ability_record.h"
#include "ability_running_record.h"
#include "module_running_record.h"
#undef private

#include "app_mgr_service_dump_error_code.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string BUNDLE_NAME = "testBundleName";
const std::string PROCESS_NAME = "testProcessName";
const std::string MODULE_NAME = "testModuleName";
const std::string APP_NAME = "appName";
const std::string APP_NAME_EMPTY = "appName";
constexpr int32_t TEST_BASE_USER_RANGE = 200000;
constexpr int32_t TEST_UID = 200000;
constexpr int32_t TEST_PID = 100;
constexpr int32_t ONE = 1;
constexpr int32_t TWO = 2;
constexpr int32_t THREE = 3;
constexpr int32_t RECORD_ID = 1;
}
class AppRunningManagerFourthTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static std::shared_ptr<ApplicationInfo> appInfo_;
    std::shared_ptr<AppRunningManager> appRunningManager_ = nullptr;
};

std::shared_ptr<ApplicationInfo> AppRunningManagerFourthTest::appInfo_ = nullptr;

void AppRunningManagerFourthTest::SetUpTestCase(void)
{
    appInfo_ = std::make_shared<ApplicationInfo>();
    appInfo_->bundleName = BUNDLE_NAME;
    appInfo_->uid = TEST_UID;
}

void AppRunningManagerFourthTest::TearDownTestCase(void)
{}

void AppRunningManagerFourthTest::SetUp()
{
    appRunningManager_ = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager_, nullptr);
}

void AppRunningManagerFourthTest::TearDown()
{
    appRunningManager_.reset();
}

sptr<Token> GetTestAbilityToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.utTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord =
        AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

std::shared_ptr<AbilityRunningRecord> GetTestAbilityRuningRecord()
{
    auto token = GetTestAbilityToken();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    auto abilityRunningRecord =
        std::make_shared<AbilityRunningRecord>(abilityInfo, token, ONE);
    return abilityRunningRecord;
}

/**
 * @tc.name: AppRunningManager_CheckAppRunningRecordIsExist_0100
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_CheckAppRunningRecordIsExist_0100, TestSize.Level1)
{
    int uid = 0;
    BundleInfo bundleInfo;
    bool *isProCache = nullptr;
    std::string instanceKey = "instanceKey";
    std::string processName;
    std::string customProcessFlag;
    std::string specifiedProcessFlag = "specifiedProcessFlag";

    bundleInfo.jointUserId = "joint456";
    auto ret = appRunningManager_->CheckAppRunningRecordIsExist(APP_NAME_EMPTY, processName,
        uid, bundleInfo, specifiedProcessFlag, isProCache, instanceKey, customProcessFlag);
    EXPECT_EQ(ret, nullptr);

    bundleInfo.jointUserId = "";
    ret = appRunningManager_->CheckAppRunningRecordIsExist(APP_NAME_EMPTY, processName,
        uid, bundleInfo, specifiedProcessFlag, isProCache, instanceKey, customProcessFlag);

    std::shared_ptr<AppRunningRecord> record =
        appRunningManager_->CreateAppRunningRecord(appInfo_, processName, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->CheckAppRunningRecordIsExist(APP_NAME_EMPTY, PROCESS_NAME,
        uid, bundleInfo, specifiedProcessFlag, isProCache, instanceKey, customProcessFlag);
    EXPECT_EQ(ret, nullptr);

    record = appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->CheckAppRunningRecordIsExist(APP_NAME_EMPTY, processName,
        uid, bundleInfo, specifiedProcessFlag, isProCache, instanceKey, customProcessFlag);
    EXPECT_EQ(ret, nullptr);

    record = appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->CheckAppRunningRecordIsExist(APP_NAME_EMPTY, PROCESS_NAME,
        uid, bundleInfo, specifiedProcessFlag, isProCache, instanceKey, customProcessFlag);
    EXPECT_EQ(ret, nullptr);

    record = appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    record->SetInstanceKey(instanceKey);
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->CheckAppRunningRecordIsExist(APP_NAME_EMPTY, PROCESS_NAME,
        uid, bundleInfo, specifiedProcessFlag, isProCache, instanceKey, customProcessFlag);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: AppRunningManager_CheckAppRunningRecordIsExist_0200
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_CheckAppRunningRecordIsExist_0200, TestSize.Level1)
{
    int uid = 0;
    BundleInfo bundleInfo;
    bool *isProCache = nullptr;
    std::string instanceKey;
    std::string customProcessFlag;
    std::string specifiedProcessFlag = "specifiedProcessFlag";

    std::shared_ptr<AppRunningRecord> record =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    record->SetSpecifiedProcessFlag(specifiedProcessFlag);
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    auto ret = appRunningManager_->CheckAppRunningRecordIsExist(APP_NAME_EMPTY, PROCESS_NAME,
        uid, bundleInfo, specifiedProcessFlag, isProCache, instanceKey, customProcessFlag);
    EXPECT_EQ(ret, nullptr);

    record = appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    record->SetSpecifiedProcessFlag(specifiedProcessFlag);
    record->SetCustomProcessFlag("customProcessFlag");
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    std::string specifiedProcessFlag1 = "";
    ret = appRunningManager_->CheckAppRunningRecordIsExist(APP_NAME_EMPTY, PROCESS_NAME,
        uid, bundleInfo, specifiedProcessFlag1, isProCache, instanceKey, customProcessFlag);
    EXPECT_EQ(ret, nullptr);

    record = appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    record->SetTerminating();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->CheckAppRunningRecordIsExist(APP_NAME_EMPTY, PROCESS_NAME,
        uid, bundleInfo, specifiedProcessFlag1, isProCache, instanceKey, customProcessFlag);
    EXPECT_EQ(ret, nullptr);

    record = appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    record->SetKilling();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->CheckAppRunningRecordIsExist(APP_NAME_EMPTY, PROCESS_NAME,
        uid, bundleInfo, specifiedProcessFlag1, isProCache, instanceKey, customProcessFlag);
    EXPECT_EQ(ret, nullptr);

    record = appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    record->SetRestartAppFlag(true);
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->CheckAppRunningRecordIsExist(APP_NAME_EMPTY, PROCESS_NAME,
        uid, bundleInfo, specifiedProcessFlag1, isProCache, instanceKey, customProcessFlag);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: AppRunningManager_CheckAppRunningRecordIsExist_0300
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_CheckAppRunningRecordIsExist_0300, TestSize.Level1)
{
    int uid = 0;
    BundleInfo bundleInfo;
    bool *isProCache = nullptr;
    std::string instanceKey;
    std::string customProcessFlag;
    std::string specifiedProcessFlag;

    std::shared_ptr<AppRunningRecord> record =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    record->SetUserRequestCleaning();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    auto ret = appRunningManager_->CheckAppRunningRecordIsExist(APP_NAME_EMPTY, PROCESS_NAME,
        uid, bundleInfo, specifiedProcessFlag, isProCache, instanceKey, customProcessFlag);
    EXPECT_EQ(ret, nullptr);

    record = appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    record->SetProcessCaching(true);
    record->SetProcessCacheBlocked(true);
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->CheckAppRunningRecordIsExist(APP_NAME_EMPTY, PROCESS_NAME,
        uid, bundleInfo, specifiedProcessFlag, isProCache, instanceKey, customProcessFlag);
    EXPECT_EQ(ret, nullptr);

    record = appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    record->SetProcessCaching(true);
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->CheckAppRunningRecordIsExist(APP_NAME_EMPTY, PROCESS_NAME,
        uid, bundleInfo, specifiedProcessFlag, isProCache, instanceKey, customProcessFlag);
    EXPECT_EQ(ret, nullptr);

    record = appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    record->SetProcessCacheBlocked(true);
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->CheckAppRunningRecordIsExist(APP_NAME_EMPTY, PROCESS_NAME,
        uid, bundleInfo, specifiedProcessFlag, isProCache, instanceKey, customProcessFlag);
    EXPECT_EQ(ret, nullptr);

    record = appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->CheckAppRunningRecordIsExist(APP_NAME_EMPTY, PROCESS_NAME,
        uid, bundleInfo, specifiedProcessFlag, isProCache, instanceKey, customProcessFlag);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: AppRunningManager_CheckAppRunningRecordIsExist_0400
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_CheckAppRunningRecordIsExist_0400, TestSize.Level1)
{
    int uid = TEST_UID;
    BundleInfo bundleInfo;
    std::string specifiedProcessFlag;
    bool *isProCache = nullptr;
    std::string instanceKey;
    std::string customProcessFlag;

    appInfo_->name = APP_NAME;
    std::shared_ptr<AppRunningRecord> record =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    record->appInfos_.insert(std::make_pair("test", appInfo_));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    auto ret = appRunningManager_->CheckAppRunningRecordIsExist(APP_NAME, PROCESS_NAME,
        uid, bundleInfo, specifiedProcessFlag, isProCache, instanceKey, customProcessFlag);
    EXPECT_NE(ret, nullptr);

    bool value = false;
    isProCache = &value;
    record = appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    record->appInfos_.insert(std::make_pair("test", appInfo_));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->CheckAppRunningRecordIsExist(APP_NAME, PROCESS_NAME,
        uid, bundleInfo, specifiedProcessFlag, isProCache, instanceKey, customProcessFlag);
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name: AppRunningManager_CheckAppRunningRecordIsExist_0401
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_CheckAppRunningRecordIsExist_0401, TestSize.Level1)
{
    int uid = TEST_UID;
    BundleInfo bundleInfo;
    std::string specifiedProcessFlag;
    bool *isProCache = nullptr;
    std::string instanceKey;
    std::string customProcessFlag;

    appInfo_->name = APP_NAME;
    std::shared_ptr<AppRunningRecord> record =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    record->appInfos_.insert(std::make_pair("test", appInfo_));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    auto ret = appRunningManager_->CheckAppRunningRecordIsExist(APP_NAME, PROCESS_NAME,
        uid, bundleInfo, specifiedProcessFlag, isProCache, instanceKey, customProcessFlag);
    EXPECT_NE(ret, nullptr);

    bool value = false;
    isProCache = &value;
    record = appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    record->appInfos_.insert(std::make_pair("test", appInfo_));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));

    bool notReuseCachedPorcess = true;
    ret = appRunningManager_->CheckAppRunningRecordIsExist(APP_NAME, PROCESS_NAME,
        uid, bundleInfo, specifiedProcessFlag, isProCache, instanceKey, customProcessFlag, notReuseCachedPorcess);
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name: AppRunningManager_GetProcessInfosByUserId_0100
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_GetProcessInfosByUserId_0100, TestSize.Level1)
{
    int32_t userId = ONE;
    std::list<SimpleProcessInfo> processInfos;
    ApplicationInfo appInfo;
    BundleInfo bundleInfo;
    std::shared_ptr<AppRunningRecord> record =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, nullptr));
    auto ret = appRunningManager_->GetProcessInfosByUserId(userId, processInfos);
    EXPECT_FALSE(ret);

    appRunningManager_->appRunningRecordMap_.clear();
    record->SetUid(TEST_BASE_USER_RANGE);
    record->appInfos_.insert(std::make_pair("test", appInfo_));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->GetProcessInfosByUserId(userId, processInfos);
    EXPECT_FALSE(ret);

    appRunningManager_->appRunningRecordMap_.clear();
    record->SetUid(TEST_BASE_USER_RANGE);
    record->priorityObject_->SetPid(TEST_PID);
    record->appInfos_.insert(std::make_pair("test", appInfo_));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->GetProcessInfosByUserId(userId, processInfos);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: AppRunningManager_IsAppRunningByBundleNameAndUserId_0100
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_IsAppRunningByBundleNameAndUserId_0100, TestSize.Level1)
{
    std::string bundleName;
    int32_t userId = ONE;
    bool isRunning = false;
    BundleInfo bundleInfo;
    std::shared_ptr<AppRunningRecord> record =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, nullptr));
    auto ret = appRunningManager_->IsAppRunningByBundleNameAndUserId(bundleName, userId, isRunning);
    EXPECT_FALSE(isRunning);

    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->IsAppRunningByBundleNameAndUserId(BUNDLE_NAME, userId, isRunning);
    EXPECT_FALSE(isRunning);

    appRunningManager_->appRunningRecordMap_.clear();
    record->SetRestartAppFlag(true);
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->IsAppRunningByBundleNameAndUserId(BUNDLE_NAME, userId, isRunning);
    EXPECT_FALSE(isRunning);

    appRunningManager_->appRunningRecordMap_.clear();
    record->SetRestartAppFlag(false);
    record->SetUid(0);
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->IsAppRunningByBundleNameAndUserId(BUNDLE_NAME, userId, isRunning);
    EXPECT_FALSE(isRunning);

    appRunningManager_->appRunningRecordMap_.clear();
    record->SetUid(TEST_BASE_USER_RANGE);
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->IsAppRunningByBundleNameAndUserId(BUNDLE_NAME, userId, isRunning);
    EXPECT_TRUE(isRunning);
}

/**
 * @tc.name: AppRunningManager_IsAppRunningByBundleName_0100
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_IsAppRunningByBundleName_0100, TestSize.Level1)
{
    std::string bundleName;
    int32_t userId = ONE;
    bool isRunning = false;
    BundleInfo bundleInfo;
    std::shared_ptr<AppRunningRecord> record =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, nullptr));
    auto ret = appRunningManager_->IsAppRunningByBundleName(bundleName, 0, userId, isRunning);
    EXPECT_FALSE(isRunning);

    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->IsAppRunningByBundleName(BUNDLE_NAME, 0, userId, isRunning);
    EXPECT_FALSE(isRunning);

    appRunningManager_->appRunningRecordMap_.clear();
    record->SetRestartAppFlag(true);
    record->SetUid(TEST_BASE_USER_RANGE);
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->IsAppRunningByBundleName(BUNDLE_NAME, 0, userId, isRunning);
    EXPECT_FALSE(isRunning);

    appRunningManager_->appRunningRecordMap_.clear();
    record->SetRestartAppFlag(false);
    record->SetUid(0);
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->IsAppRunningByBundleName(BUNDLE_NAME, 0, userId, isRunning);
    EXPECT_FALSE(isRunning);

    appRunningManager_->appRunningRecordMap_.clear();
    record->SetRestartAppFlag(false);
    record->SetUid(TEST_BASE_USER_RANGE);
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->IsAppRunningByBundleName(BUNDLE_NAME, 1, userId, isRunning);
    EXPECT_FALSE(isRunning);

    appRunningManager_->appRunningRecordMap_.clear();
    record->SetRestartAppFlag(false);
    record->SetUid(TEST_BASE_USER_RANGE);
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->IsAppRunningByBundleName(BUNDLE_NAME, 0, userId, isRunning);
    EXPECT_TRUE(isRunning);
}

/**
 * @tc.name: AppRunningManager_ProcessUpdateApplicationInfoInstalled_0100
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_ProcessUpdateApplicationInfoInstalled_0100, TestSize.Level1)
{
    int32_t userId = ONE;
    ApplicationInfo appInfo;
    BundleInfo bundleInfo;
    std::shared_ptr<AppRunningRecord> record =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager_->appRunningRecordMap_.clear();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, nullptr));
    auto ret = appRunningManager_->ProcessUpdateApplicationInfoInstalled(appInfo, MODULE_NAME);
    EXPECT_EQ(ret, ERR_OK);

    appRunningManager_->appRunningRecordMap_.clear();
    record->appInfos_.insert(std::make_pair("test", appInfo_));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->ProcessUpdateApplicationInfoInstalled(appInfo, MODULE_NAME);
    EXPECT_EQ(ret, ERR_OK);

    appRunningManager_->appRunningRecordMap_.clear();
    record->appInfos_.insert(std::make_pair("test", appInfo_));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    appInfo.bundleName = BUNDLE_NAME;
    ret = appRunningManager_->ProcessUpdateApplicationInfoInstalled(appInfo, MODULE_NAME);
    EXPECT_EQ(ret, ERR_OK);

    appRunningManager_->appRunningRecordMap_.clear();
    record->appInfos_.insert(std::make_pair("test", appInfo_));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    appInfo.bundleName = "";
    appInfo.uid = TEST_UID;
    ret = appRunningManager_->ProcessUpdateApplicationInfoInstalled(appInfo, MODULE_NAME);
    EXPECT_EQ(ret, ERR_OK);

    appRunningManager_->appRunningRecordMap_.clear();
    record->appInfos_.insert(std::make_pair("test", appInfo_));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    appInfo.bundleName = BUNDLE_NAME;
    appInfo.uid = TEST_UID;
    ret = appRunningManager_->ProcessUpdateApplicationInfoInstalled(appInfo, MODULE_NAME);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: AppRunningManager_HandleUserRequestClean_0100
 * @tc.desc: Test HandleUserRequestClean
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_HandleUserRequestClean_0100, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize AppRunningManager instance
     * @tc.expected: expect step1 succeed
     */
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_HandleUserRequestClean_0100 start");
    BundleInfo bundleInfo;

    /**
     * @tc.steps: step2. Initialize AppRunningManager instance
     * @tc.expected: expect step2 focused false
     */
    sptr<Token> token = nullptr;
    pid_t targetPid = 0;
    int32_t targetUid = 0;
    int32_t recordIdRet = -1;
    auto ret = appRunningManager_->HandleUserRequestClean(token, targetPid, targetUid, recordIdRet);
    EXPECT_EQ(ret, false);

    token = GetTestAbilityToken();
    ret = appRunningManager_->HandleUserRequestClean(token, targetPid, targetUid, recordIdRet);
    EXPECT_EQ(ret, false);

    std::shared_ptr<AppRunningRecord> appRunningRecord =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRunningRecords;
    auto moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo_, nullptr);
    auto abilityRuningRecord = GetTestAbilityRuningRecord();
    moduleRecord->abilities_.emplace(token, abilityRuningRecord);
    moduleRunningRecords.push_back(moduleRecord);
    appRunningRecord->hapModules_.emplace(BUNDLE_NAME, moduleRunningRecords);
    auto recordId = AppRecordId::Create();
    appRunningManager_->appRunningRecordMap_.emplace(recordId, appRunningRecord);
    ret = appRunningManager_->HandleUserRequestClean(token, targetPid, targetUid, recordId);
    EXPECT_EQ(ret, true);

    appRunningRecord->procCacheSupportState_ = SupportProcessCacheState::SUPPORT;
    ret = appRunningManager_->HandleUserRequestClean(token, targetPid, targetUid, recordId);
    EXPECT_EQ(ret, false);

    appRunningRecord->procCacheSupportState_ = SupportProcessCacheState::UNSPECIFIED;

    appRunningRecord->isMainProcess_= true;
    appRunningRecord->isKeepAliveBundle_= true;
    appRunningRecord->isKeepAliveRdb_= true;
    appRunningRecord->mainUid_ = TEST_BASE_USER_RANGE;
    ret = appRunningManager_->HandleUserRequestClean(token, targetPid, targetUid, recordId);
    EXPECT_EQ(ret, false);

    auto priorityObject = std::make_shared<PriorityObject>();
    appRunningRecord->isUserRequestCleaning_= true;
    appRunningRecord->isMainProcess_= false;
    appRunningRecord->priorityObject_ = nullptr;
    ret = appRunningManager_->HandleUserRequestClean(token, targetPid, targetUid, recordId);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_HandleUserRequestClean_0100 end");
}

/**
 * @tc.name: AppRunningManager_CheckIsKiaProcess_0100
 * @tc.desc: Test CheckIsKiaProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_CheckIsKiaProcess_0100, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize AppRunningManager instance
     * @tc.expected: expect step1 succeed
     */
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_CheckIsKiaProcess_0100 start");
    BundleInfo bundleInfo;

    /**
     * @tc.steps: step2. Initialize AppRunningManager instance
     * @tc.expected: expect step2 focused false
     */
    pid_t pid = 0;
    bool isKia = false;
    auto ret = appRunningManager_->CheckIsKiaProcess(pid, isKia);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    pid = ONE;
    std::shared_ptr<AppRunningRecord> appRunningRecord =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningRecord->priorityObject_ = std::make_shared<PriorityObject>();
    appRunningRecord->priorityObject_->SetPid(pid);
    auto recordId = AppRecordId::Create();
    appRunningManager_->appRunningRecordMap_.emplace(recordId, appRunningRecord);
    ret = appRunningManager_->CheckIsKiaProcess(pid, isKia);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_CheckIsKiaProcess_0100 end");
}

/**
 * @tc.name: AppRunningManager_CheckAppRunningRecordIsLast_0100
 * @tc.desc: Test CheckAppRunningRecordIsLast
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_CheckAppRunningRecordIsLast_0100, TestSize.Level1)
{
    /**
     * @tc.steps: step1. Initialize AppRunningManager instance
     * @tc.expected: expect step1 succeed
     */
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_CheckAppRunningRecordIsLast_0100 start");

    /**
     * @tc.steps: step2. Initialize AppRunningManager instance
     * @tc.expected: expect step2 focused false
     */
    std::shared_ptr<AppRunningRecord> appRunningRecord = nullptr;
    auto ret = appRunningManager_->CheckAppRunningRecordIsLast(appRunningRecord);
    EXPECT_EQ(ret, false);

    appRunningRecord = std::make_shared<AppRunningRecord>(appInfo_, ONE, PROCESS_NAME);
    ret = appRunningManager_->CheckAppRunningRecordIsLast(appRunningRecord);
    EXPECT_EQ(ret, true);

    appRunningManager_->appRunningRecordMap_.emplace(TWO, nullptr);
    ret = appRunningManager_->CheckAppRunningRecordIsLast(appRunningRecord);
    EXPECT_EQ(ret, true);

    appRunningRecord->SetUid(TWO);
    appRunningManager_->appRunningRecordMap_.emplace(ONE, appRunningRecord);
    ret = appRunningManager_->CheckAppRunningRecordIsLast(appRunningRecord);
    EXPECT_EQ(ret, true);

    auto appRunningRecord2 = std::make_shared<AppRunningRecord>(appInfo_, TWO, PROCESS_NAME);
    appRunningRecord2->SetUid(ONE);
    ret = appRunningManager_->CheckAppRunningRecordIsLast(appRunningRecord2);
    EXPECT_EQ(ret, true);

    appRunningRecord2->SetUid(TWO);
    ret = appRunningManager_->CheckAppRunningRecordIsLast(appRunningRecord2);
    EXPECT_EQ(ret, false);

    appRunningRecord2->SetRestartAppFlag(!appRunningRecord2->GetRestartAppFlag());
    ret = appRunningManager_->CheckAppRunningRecordIsLast(appRunningRecord2);
    EXPECT_EQ(ret, true);

    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_CheckAppRunningRecordIsLast_0100 end");
}

/**
 * @tc.name: ProcessExitByBundleNameAndAppIndex_0100
 * @tc.desc: test ProcessExitByBundleNameAndAppIndex
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, ProcessExitByBundleNameAndAppIndex_0100, TestSize.Level1)
{
    static std::shared_ptr<AppRunningManager> appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);
    std::shared_ptr<AppRunningRecord> appRunningRecord = nullptr;
    int32_t num = 100;
    appRunningManager->appRunningRecordMap_.insert(std::make_pair(num, appRunningRecord));

    std::string bundleName = "bundleName";
    int32_t appIndex = 8;
    std::list<pid_t> pids;
    bool clearPageStack = true;
    auto ret = appRunningManager->ProcessExitByBundleNameAndAppIndex(bundleName, appIndex, pids, clearPageStack);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: ProcessExitByBundleNameAndAppIndex_0200
 * @tc.desc: test ProcessExitByBundleNameAndAppIndex
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, ProcessExitByBundleNameAndAppIndex_0200, TestSize.Level1)
{
    static std::shared_ptr<AppRunningManager> appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    std::shared_ptr<ApplicationInfo> info = nullptr;
    int32_t recordId = 100;
    std::string processName = "processName";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(info, recordId, processName);
    ASSERT_NE(appRunningRecord, nullptr);

    int32_t priorityUid = 2000000;
    appRunningRecord->SetUid(priorityUid);
    appRunningRecord->isKeepAliveBundle_ = true;
    appRunningRecord->isKeepAliveRdb_ = true;
    int32_t num = 100;
    appRunningManager->appRunningRecordMap_.insert(std::make_pair(num, appRunningRecord));
    std::string bundleName = "bundleName";
    int32_t appIndex = 8;
    std::list<pid_t> pids;
    bool clearPageStack = true;
    auto ret = appRunningManager->ProcessExitByBundleNameAndAppIndex(bundleName, appIndex, pids, clearPageStack);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: ProcessExitByBundleNameAndAppIndex_0300
 * @tc.desc: test ProcessExitByBundleNameAndAppIndex
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, ProcessExitByBundleNameAndAppIndex_0300, TestSize.Level1)
{
    static std::shared_ptr<AppRunningManager> appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    std::shared_ptr<ApplicationInfo> info = nullptr;
    int32_t recordId = 100;
    std::string processName = "processName";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(info, recordId, processName);
    ASSERT_NE(appRunningRecord, nullptr);

    int32_t priorityUid = 2000000;
    appRunningRecord->SetUid(priorityUid);
    int32_t num = 100;
    appRunningManager->appRunningRecordMap_.insert(std::make_pair(num, appRunningRecord));
    std::string bundleName = "bundleName";
    int32_t appIndex = 8;
    std::list<pid_t> pids;
    bool clearPageStack = true;
    auto ret = appRunningManager->ProcessExitByBundleNameAndAppIndex(bundleName, appIndex, pids, clearPageStack);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: ProcessExitByBundleNameAndAppIndex_0400
 * @tc.desc: test ProcessExitByBundleNameAndAppIndex
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, ProcessExitByBundleNameAndAppIndex_0400, TestSize.Level1)
{
    static std::shared_ptr<AppRunningManager> appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    info->bundleName = "bundleName";
    int32_t recordId = 100;
    std::string processName = "processName";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(info, recordId, processName);
    ASSERT_NE(appRunningRecord, nullptr);

    int32_t priorityUid = 2000000;
    appRunningRecord->SetUid(priorityUid);
    appRunningRecord->priorityObject_ = nullptr;
    int32_t num = 100;
    appRunningManager->appRunningRecordMap_.insert(std::make_pair(num, appRunningRecord));
    std::string bundleName = "bundleName";
    int32_t appIndex = 8;
    std::list<pid_t> pids;
    bool clearPageStack = true;
    auto ret = appRunningManager->ProcessExitByBundleNameAndAppIndex(bundleName, appIndex, pids, clearPageStack);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: ProcessExitByBundleNameAndAppIndex_0500
 * @tc.desc: test ProcessExitByBundleNameAndAppIndex
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, ProcessExitByBundleNameAndAppIndex_0500, TestSize.Level1)
{
    static std::shared_ptr<AppRunningManager> appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    info->bundleName = "bundleName";
    int32_t recordId = 100;
    std::string processName = "processName";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(info, recordId, processName);
    ASSERT_NE(appRunningRecord, nullptr);

    appRunningRecord->priorityObject_ = std::make_shared<PriorityObject>();
    int32_t priorityPid = -1;
    appRunningRecord->priorityObject_->SetPid(priorityPid);
    int32_t priorityUid = 2000000;
    appRunningRecord->SetUid(priorityUid);
    int32_t num = 100;
    appRunningManager->appRunningRecordMap_.insert(std::make_pair(num, appRunningRecord));
    int32_t appIndex = 8;
    appRunningRecord->SetAppIndex(appIndex);
    std::string bundleName = "bundleName";
    std::list<pid_t> pids;
    bool clearPageStack = true;
    auto ret = appRunningManager->ProcessExitByBundleNameAndAppIndex(bundleName, appIndex, pids, clearPageStack);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: ProcessExitByBundleNameAndAppIndex_0600
 * @tc.desc: test ProcessExitByBundleNameAndAppIndex
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, ProcessExitByBundleNameAndAppIndex_0600, TestSize.Level1)
{
    static std::shared_ptr<AppRunningManager> appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    info->bundleName = "bundleName";
    int32_t recordId = 100;
    std::string processName = "processName";
    auto appRunningRecord = std::make_shared<AppRunningRecord>(info, recordId, processName);
    ASSERT_NE(appRunningRecord, nullptr);

    appRunningRecord->priorityObject_ = std::make_shared<PriorityObject>();
    int32_t priorityPid = 66;
    appRunningRecord->priorityObject_->SetPid(priorityPid);
    int32_t priorityUid = 2000000;
    appRunningRecord->SetUid(priorityUid);
    int32_t num = 100;
    appRunningManager->appRunningRecordMap_.insert(std::make_pair(num, appRunningRecord));
    int32_t appIndex = 8;
    appRunningRecord->SetAppIndex(appIndex);
    std::string bundleName = "bundleName";
    std::list<pid_t> pids;
    bool clearPageStack = true;
    auto ret = appRunningManager->ProcessExitByBundleNameAndAppIndex(bundleName, appIndex, pids, clearPageStack);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: AppRunningManager_DumpIpcStop_0100
 * @tc.desc: Test the state of DumpIpcStop
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_DumpIpcStop_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string result = "";
    int32_t recordId = 0;
    auto ret = appRunningManager->DumpIpcStop(recordId, result);
    EXPECT_EQ(ret, DumpErrorCode::ERR_INVALID_PID_ERROR);

    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, PROCESS_NAME);
    appRunningManager->appRunningRecordMap_.insert(make_pair(recordId, appRunningRecord));
    ret = appRunningManager->DumpIpcStop(recordId, result);
    EXPECT_NE(ret, DumpErrorCode::ERR_INVALID_PID_ERROR);
}

/**
 * @tc.name: AppRunningManager_DumpIpcStat_0100
 * @tc.desc: Test the state of DumpIpcStat
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_DumpIpcStat_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string result = "";
    int32_t recordId = 0;
    auto ret = appRunningManager->DumpIpcStat(recordId, result);
    EXPECT_EQ(ret, DumpErrorCode::ERR_INVALID_PID_ERROR);

    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, PROCESS_NAME);
    appRunningManager->appRunningRecordMap_.insert(make_pair(recordId, appRunningRecord));
    ret = appRunningManager->DumpIpcStat(recordId, result);
    EXPECT_NE(ret, DumpErrorCode::ERR_INVALID_PID_ERROR);
}

/**
 * @tc.name: AppRunningManager_DumpFfrt_0100
 * @tc.desc: Test the state of DumpFfrt
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_DumpFfrt_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string result = "";
    std::vector<int32_t> pids = {1, 5, 4, 6};
    int32_t recordId = 0;
    auto ret = appRunningManager->DumpFfrt(pids, result);
    EXPECT_EQ(ret, DumpErrorCode::ERR_INVALID_PID_ERROR);

    pids = {1, 0, 4, 6};
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, PROCESS_NAME);
    appRunningManager->appRunningRecordMap_.clear();
    appRunningRecord->appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    appRunningManager->appRunningRecordMap_.insert(make_pair(recordId, appRunningRecord));
    ret = appRunningManager->DumpFfrt(pids, result);
    EXPECT_EQ(ret, DumpErrorCode::ERR_INTERNAL_ERROR);

    result = "test";
    ret = appRunningManager->DumpFfrt(pids, result);
    EXPECT_EQ(ret, DumpErrorCode::ERR_OK);
}

/**
 * @tc.name: AppRunningManager_DumpArkWeb_0100
 * @tc.desc: Test the state of DumpArkWeb
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_DumpArkWeb_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpArkWeb_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    std::string customArgs;
    std::string result;
    std::vector<int32_t> pids = {1, 2, 3};
    auto ret = appRunningManager->DumpArkWeb(pids, customArgs, result);
    EXPECT_EQ(ret, DumpErrorCode::ERR_INVALID_PID_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpArkWeb_0100 end");
}

/**
 * @tc.name: AppRunningManager_DumpArkWeb_0200
 * @tc.desc: Test the state of DumpArkWeb
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_DumpArkWeb_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpArkWeb_0200 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, PROCESS_NAME);
    auto priorityObject = appRecord->GetPriorityObject();
    if (priorityObject) {
        priorityObject->SetPid(1);
    }
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRecord));

    std::string customArgs;
    std::string result;
    std::vector<int32_t> pids = {1, 2, 3};
    auto ret = appRunningManager->DumpArkWeb(pids, customArgs, result);
    EXPECT_EQ(ret, DumpErrorCode::ERR_INTERNAL_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpArkWeb_0200 end");
}

/**
 * @tc.name: AppRunningManager_DumpArkWeb_0300
 * @tc.desc: Test the state of DumpArkWeb
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_DumpArkWeb_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpArkWeb_0300 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, PROCESS_NAME);
    auto priorityObject = appRecord->GetPriorityObject();
    if (priorityObject) {
        priorityObject->SetPid(1);
    }
    appRecord->appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRecord));

    std::string customArgs;
    std::string result = "Result:";
    std::vector<int32_t> pids = {1, 2, 3};
    auto ret = appRunningManager->DumpArkWeb(pids, customArgs, result);
    EXPECT_EQ(ret, DumpErrorCode::ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpArkWeb_0300 end");
}

/**
 * @tc.name: AppRunningManager_OnRemoteRenderDied_0100
 * @tc.desc: Test the state of OnRemoteRenderDied
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_OnRemoteRenderDied_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    auto ret = appRunningManager->OnRemoteRenderDied(nullptr);
    EXPECT_EQ(ret, nullptr);

    OHOS::sptr<IRemoteObject> remote = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    appRunningManager->appRunningRecordMap_.clear();
    int32_t recordId = 0;
    ret  = appRunningManager->OnRemoteRenderDied(remote);
    EXPECT_EQ(ret, nullptr);

    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, PROCESS_NAME);
    appRunningRecord->appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    appRunningManager->appRunningRecordMap_.insert(make_pair(recordId, appRunningRecord));
    ret = appRunningManager->OnRemoteRenderDied(remote);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: UpdateConfigurationForBackgroundApp_0100
 * @tc.desc: UpdateConfigurationForBackgroundApp.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, UpdateConfigurationForBackgroundApp_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    std::vector<BackgroundAppInfo> appInfos;
    AppExecFwk::ConfigurationPolicy policy;
    int32_t userId = -1;
    policy.maxCountPerBatch  = -1;
    auto ret = appRunningManager->UpdateConfigurationForBackgroundApp(appInfos, policy, userId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    
    policy.maxCountPerBatch  = 1;
    policy.intervalTime = -1;
    ret = appRunningManager->UpdateConfigurationForBackgroundApp(appInfos, policy, userId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    policy.maxCountPerBatch  = 1;
    policy.intervalTime = 1;
    BackgroundAppInfo info;
    info.bandleName = "com.example.mytest";
    info.appIndex = 0;
    appInfos.push_back(info);
    ret = appRunningManager->UpdateConfigurationForBackgroundApp(appInfos, policy, userId);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: UpdateConfiguration_0100
 * @tc.desc: UpdateConfiguration.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, UpdateConfiguration_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    ApplicationInfo appInfo;
    appInfo.name = "KeepAliveApp";
    appInfo.bundleName = "KeepAliveApplication";
    appInfo.uid = 2100;
    auto app = std::make_shared<ApplicationInfo>(appInfo);

    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(app, 111, "KeepAliveApplication");
    auto ret = appRunningManager->UpdateConfiguration(appRecord, Rosen::ConfigMode::FONT_SCALE);
    EXPECT_FALSE(ret);

    appRecord->delayConfiguration_ = nullptr;
    ret = appRunningManager->UpdateConfiguration(appRecord, Rosen::ConfigMode::COLOR_MODE);
    EXPECT_FALSE(ret);

    appRecord->delayConfiguration_ = std::make_shared<Configuration>();
    appRecord->delayConfiguration_->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE,
        ConfigurationInner::EMPTY_STRING);
    ret = appRunningManager->UpdateConfiguration(appRecord, Rosen::ConfigMode::COLOR_MODE);
    EXPECT_FALSE(ret);

    appRecord->delayConfiguration_->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE,
        ConfigurationInner::COLOR_MODE_DARK);
    ret = appRunningManager->UpdateConfiguration(appRecord, Rosen::ConfigMode::COLOR_MODE);
    EXPECT_TRUE(ret);

    appRecord = nullptr;
    ret = appRunningManager->UpdateConfiguration(appRecord, Rosen::ConfigMode::COLOR_MODE);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: ExecuteConfigurationTask_0100
 * @tc.desc: ExecuteConfigurationTask.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, ExecuteConfigurationTask_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    BackgroundAppInfo info;
    int32_t userId = 0;
    ApplicationInfo appInfo;
    appInfo.name = "KeepAliveApp";
    appInfo.bundleName = "KeepAliveApplication";
    appInfo.uid = 2100;
    auto app = std::make_shared<ApplicationInfo>(appInfo);
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(app, 111, "KeepAliveApplication");

    appRunningManager->updateConfigurationDelayedMap_.emplace(0, true);
    appRunningManager->updateConfigurationDelayedMap_.emplace(1, true);
    appRecord->appRecordId_ = 0;
    appRecord->appIndex_ = 0;
    appRecord->curState_ = ApplicationState::APP_STATE_BACKGROUND;
    appRunningManager->appRunningRecordMap_.emplace(0, appRecord);
    appRunningManager->appRunningRecordMap_.emplace(1, nullptr);
    info.bandleName = "KeepAliveApplication";
    info.appIndex = 0;
    userId = 1;
    appRunningManager->ExecuteConfigurationTask(info, userId);

    userId = -1;
    appRecord->delayConfiguration_ = nullptr;
    appRunningManager->ExecuteConfigurationTask(info, userId);

    appRecord->delayConfiguration_ = std::make_shared<Configuration>();
    appRecord->delayConfiguration_->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE,
        ConfigurationInner::COLOR_MODE_DARK);
    appRunningManager->ExecuteConfigurationTask(info, userId);
    std::string value = appRecord->delayConfiguration_->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    EXPECT_TRUE(value == ConfigurationInner::EMPTY_STRING);
}

/**
 * @tc.name: AppRunningManager_IsSameAbilityType_0100
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_IsSameAbilityType_0100, TestSize.Level1)
{
    int uid = 0;
    BundleInfo bundleInfo;

    std::shared_ptr<AppRunningRecord> record =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(record, nullptr);
    AppExecFwk::AbilityInfo abilityInfo;

    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    auto ret = appRunningManager_->IsSameAbilityType(nullptr, abilityInfo);
    EXPECT_EQ(ret, false);

    record->processType_ = ProcessType::NORMAL;
    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    ret = appRunningManager_->IsSameAbilityType(record, abilityInfo);
    EXPECT_EQ(ret, true);

    record->processType_ = ProcessType::NORMAL;
    abilityInfo.type = AppExecFwk::AbilityType::FORM;
    ret = appRunningManager_->IsSameAbilityType(record, abilityInfo);
    EXPECT_EQ(ret, false);

    record->extensionType_ = AppExecFwk::ExtensionAbilityType::SERVICE;
    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    ret = appRunningManager_->IsSameAbilityType(record, abilityInfo);
    EXPECT_EQ(ret, true);

    record->extensionType_ = AppExecFwk::ExtensionAbilityType::DATASHARE;
    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    ret = appRunningManager_->IsSameAbilityType(record, abilityInfo);
    EXPECT_EQ(ret, true);

    record->extensionType_ = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    ret = appRunningManager_->IsSameAbilityType(record, abilityInfo);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: AppRunningManager_CheckMasterProcessAppRunningRecordIsExist_0100
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_CheckMasterProcessAppRunningRecordIsExist_0100, TestSize.Level1)
{
    int uid = 0;
    BundleInfo bundleInfo;
    std::string processName;
    std::shared_ptr<AppRunningRecord> record =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(record, nullptr);
    AppExecFwk::AbilityInfo abilityInfo;

    record->processType_ = ProcessType::NORMAL;
    record->SetMasterProcess(true);
    record->SetUid(uid);
    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    appRunningManager_->appRunningRecordMap_.clear();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    auto ret = appRunningManager_->CheckMasterProcessAppRunningRecordIsExist(APP_NAME_EMPTY, abilityInfo, uid);
    EXPECT_EQ(ret, true);

    record->processType_ = ProcessType::EXTENSION;
    record->SetMasterProcess(true);
    record->SetUid(uid);
    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    appRunningManager_->appRunningRecordMap_.clear();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->CheckMasterProcessAppRunningRecordIsExist(APP_NAME_EMPTY, abilityInfo, uid);
    EXPECT_EQ(ret, false);

    record->processType_ = ProcessType::NORMAL;
    record->SetMasterProcess(false);
    record->SetUid(uid);
    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    appRunningManager_->appRunningRecordMap_.clear();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, record));
    ret = appRunningManager_->CheckMasterProcessAppRunningRecordIsExist(APP_NAME_EMPTY, abilityInfo, uid);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: AppRunningManager_FindMasterProcessAppRunningRecord_0100
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_FindMasterProcessAppRunningRecord_0100, TestSize.Level1)
{
    int uid = 0;
    BundleInfo bundleInfo;
    std::shared_ptr<AppRunningRecord> recordOne =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordOne, nullptr);
    std::shared_ptr<AppRunningRecord> recordTwo =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordTwo, nullptr);
    std::shared_ptr<AppRunningRecord> recordThree =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordThree, nullptr);
    AppExecFwk::AbilityInfo abilityInfo;

    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    recordOne->processType_ = ProcessType::NORMAL;
    recordOne->SetMasterProcess(true);
    recordOne->SetUid(uid);
    recordTwo->processType_ = ProcessType::NORMAL;
    recordTwo->SetMasterProcess(false);
    recordTwo->SetUid(uid);
    recordThree->processType_ = ProcessType::EXTENSION;
    recordThree->SetMasterProcess(true);
    recordThree->SetUid(uid);
    appRunningManager_->appRunningRecordMap_.clear();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, recordOne));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(TWO, recordTwo));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(THREE, recordThree));
    auto ret = appRunningManager_->FindMasterProcessAppRunningRecord(APP_NAME_EMPTY, abilityInfo, uid);
    EXPECT_EQ(ret, recordOne);
}

/**
 * @tc.name: AppRunningManager_FindMasterProcessAppRunningRecord_0200
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_FindMasterProcessAppRunningRecord_0200, TestSize.Level1)
{
    int uid = 0;
    BundleInfo bundleInfo;
    std::shared_ptr<AppRunningRecord> recordOne =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordOne, nullptr);
    std::shared_ptr<AppRunningRecord> recordTwo =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordTwo, nullptr);
    std::shared_ptr<AppRunningRecord> recordThree =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordThree, nullptr);
    AppExecFwk::AbilityInfo abilityInfo;

    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    recordOne->processType_ = ProcessType::NORMAL;
    recordOne->SetMasterProcess(false);
    recordOne->SetUid(uid);
    recordTwo->processType_ = ProcessType::NORMAL;
    recordTwo->SetMasterProcess(false);
    recordTwo->SetTimeStamp(1);
    recordTwo->SetUid(uid);
    recordThree->processType_ = ProcessType::NORMAL;
    recordThree->SetTimeStamp(-1);
    recordThree->SetMasterProcess(false);
    recordThree->SetUid(uid);
    appRunningManager_->appRunningRecordMap_.clear();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, recordOne));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(TWO, recordTwo));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(THREE, recordThree));
    auto ret = appRunningManager_->FindMasterProcessAppRunningRecord(APP_NAME_EMPTY, abilityInfo, uid);
    EXPECT_EQ(ret, recordTwo);
}

/**
 * @tc.name: AppRunningManager_FindMasterProcessAppRunningRecord_0300
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_FindMasterProcessAppRunningRecord_0300, TestSize.Level1)
{
    int uid = 0;
    BundleInfo bundleInfo;
    std::shared_ptr<AppRunningRecord> recordOne =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordOne, nullptr);
    std::shared_ptr<AppRunningRecord> recordTwo =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordTwo, nullptr);
    std::shared_ptr<AppRunningRecord> recordThree =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordThree, nullptr);
    AppExecFwk::AbilityInfo abilityInfo;

    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    recordOne->processType_ = ProcessType::NORMAL;
    recordOne->SetMasterProcess(true);
    recordOne->SetUid(uid);
    recordTwo->processType_ = ProcessType::NORMAL;
    recordTwo->SetMasterProcess(false);
    recordTwo->SetTimeStamp(1);
    recordTwo->SetUid(uid);
    recordThree->processType_ = ProcessType::NORMAL;
    recordThree->SetTimeStamp(-1);
    recordThree->SetMasterProcess(false);
    recordThree->SetUid(uid);
    appRunningManager_->appRunningRecordMap_.clear();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, recordOne));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(TWO, recordTwo));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(THREE, recordThree));
    auto ret = appRunningManager_->FindMasterProcessAppRunningRecord(APP_NAME_EMPTY, abilityInfo, uid);
    EXPECT_EQ(ret, recordOne);
}

/**
 * @tc.name: AppRunningManager_FindMasterProcessAppRunningRecord_0400
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_FindMasterProcessAppRunningRecord_0400, TestSize.Level1)
{
    int uid = 0;
    BundleInfo bundleInfo;
    std::shared_ptr<AppRunningRecord> recordOne =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordOne, nullptr);
    std::shared_ptr<AppRunningRecord> recordTwo =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordTwo, nullptr);
    std::shared_ptr<AppRunningRecord> recordThree =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordThree, nullptr);
    AppExecFwk::AbilityInfo abilityInfo;

    abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    recordOne->processType_ = ProcessType::NORMAL;
    recordOne->SetMasterProcess(true);
    recordOne->SetUid(uid);
    recordTwo->extensionType_ = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    recordTwo->SetMasterProcess(true);
    recordTwo->SetTimeStamp(0);
    recordTwo->SetUid(uid);
    recordThree->extensionType_ = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    recordThree->SetTimeStamp(-1);
    recordThree->SetMasterProcess(false);
    recordThree->SetUid(uid);
    appRunningManager_->appRunningRecordMap_.clear();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, recordOne));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(TWO, recordTwo));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(THREE, recordThree));
    auto ret = appRunningManager_->FindMasterProcessAppRunningRecord(APP_NAME_EMPTY, abilityInfo, uid);
    EXPECT_EQ(ret, recordTwo);
}

/**
 * @tc.name: AppRunningManager_FindMasterProcessAppRunningRecord_0500
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_FindMasterProcessAppRunningRecord_0500, TestSize.Level1)
{
    int uid = 0;
    BundleInfo bundleInfo;
    std::shared_ptr<AppRunningRecord> recordOne =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordOne, nullptr);
    std::shared_ptr<AppRunningRecord> recordTwo =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordTwo, nullptr);
    std::shared_ptr<AppRunningRecord> recordThree =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordThree, nullptr);
    AppExecFwk::AbilityInfo abilityInfo;

    abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    recordOne->processType_ = ProcessType::NORMAL;
    recordOne->SetMasterProcess(true);
    recordOne->SetUid(uid);
    recordTwo->extensionType_ = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    recordTwo->SetMasterProcess(false);
    recordTwo->SetTimeStamp(-2);
    recordTwo->SetUid(uid);
    recordThree->extensionType_ = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    recordThree->SetTimeStamp(-1);
    recordThree->SetMasterProcess(false);
    recordThree->SetUid(uid);
    appRunningManager_->appRunningRecordMap_.clear();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, recordOne));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(TWO, recordTwo));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(THREE, recordThree));
    auto ret = appRunningManager_->FindMasterProcessAppRunningRecord(APP_NAME_EMPTY, abilityInfo, uid);
    EXPECT_EQ(ret, recordThree);
}

/**
 * @tc.name: AppRunningManager_FindMasterProcessAppRunningRecord_0600
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_FindMasterProcessAppRunningRecord_0600, TestSize.Level1)
{
    int uid = 0;
    BundleInfo bundleInfo;
    std::shared_ptr<AppRunningRecord> recordOne =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordOne, nullptr);
    std::shared_ptr<AppRunningRecord> recordTwo =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordTwo, nullptr);
    std::shared_ptr<AppRunningRecord> recordThree =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordThree, nullptr);
    AppExecFwk::AbilityInfo abilityInfo;

    abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    recordOne->extensionType_ = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    recordOne->SetMasterProcess(true);
    recordOne->SetUid(uid);
    recordTwo->extensionType_ = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    recordTwo->SetMasterProcess(false);
    recordTwo->SetTimeStamp(-2);
    recordTwo->SetUid(uid);
    recordThree->extensionType_ = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    recordThree->SetTimeStamp(-1);
    recordThree->SetMasterProcess(false);
    recordThree->SetUid(uid);
    appRunningManager_->appRunningRecordMap_.clear();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, recordOne));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(TWO, recordTwo));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(THREE, recordThree));
    auto ret = appRunningManager_->FindMasterProcessAppRunningRecord(APP_NAME_EMPTY, abilityInfo, uid);
    EXPECT_EQ(ret, recordOne);
}

/**
 * @tc.name: AppRunningManager_FindMasterProcessAppRunningRecord_0700
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_FindMasterProcessAppRunningRecord_0700, TestSize.Level1)
{
    int uid = 0;
    BundleInfo bundleInfo;
    std::shared_ptr<AppRunningRecord> recordOne =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordOne, nullptr);
    std::shared_ptr<AppRunningRecord> recordTwo =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordTwo, nullptr);
    std::shared_ptr<AppRunningRecord> recordThree =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordThree, nullptr);
    AppExecFwk::AbilityInfo abilityInfo;

    abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    recordOne->extensionType_ = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    recordOne->SetMasterProcess(false);
    recordOne->SetUid(uid);
    recordOne->appRecordId_ = 0;
    recordTwo->extensionType_ = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    recordTwo->SetMasterProcess(false);
    recordTwo->SetTimeStamp(0);
    recordTwo->SetUid(uid);
    recordTwo->appRecordId_ = 1;
    recordThree->extensionType_ = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    recordThree->SetTimeStamp(0);
    recordThree->SetMasterProcess(false);
    recordThree->SetUid(uid);
    recordThree->appRecordId_ = 2;
    appRunningManager_->appRunningRecordMap_.clear();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, recordOne));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(TWO, recordTwo));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(THREE, recordThree));
    auto ret = appRunningManager_->FindMasterProcessAppRunningRecord(APP_NAME_EMPTY, abilityInfo, uid);
    EXPECT_EQ(ret, recordOne);
}

/**
 * @tc.name: AppRunningManager_FindMasterProcessAppRunningRecord_0800
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_FindMasterProcessAppRunningRecord_0800, TestSize.Level1)
{
    int uid = 0;
    BundleInfo bundleInfo;
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SYS_COMMON_UI;
    auto ret = appRunningManager_->FindMasterProcessAppRunningRecord(APP_NAME_EMPTY, abilityInfo, uid);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: AppRunningManager_FindMasterProcessAppRunningRecord_0900
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_FindMasterProcessAppRunningRecord_0900, TestSize.Level1)
{
    int uid = 0;
    BundleInfo bundleInfo;
    std::shared_ptr<AppRunningRecord> recordOne =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordOne, nullptr);
    std::shared_ptr<AppRunningRecord> recordTwo =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordTwo, nullptr);
    std::shared_ptr<AppRunningRecord> recordThree =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordThree, nullptr);
    AppExecFwk::AbilityInfo abilityInfo;

    abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    recordOne->processType_ = ProcessType::NORMAL;
    recordOne->SetMasterProcess(false);
    recordOne->SetUid(uid);
    recordTwo->processType_ = ProcessType::NORMAL;
    recordTwo->SetMasterProcess(false);
    recordTwo->SetUid(uid);
    recordThree->processType_ = ProcessType::EXTENSION;
    recordThree->SetMasterProcess(false);
    recordThree->SetUid(uid);
    appRunningManager_->appRunningRecordMap_.clear();
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(ONE, recordOne));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(TWO, recordTwo));
    appRunningManager_->appRunningRecordMap_.insert(std::make_pair(THREE, recordThree));
    auto ret = appRunningManager_->FindMasterProcessAppRunningRecord(APP_NAME_EMPTY, abilityInfo, uid);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: AppRunningManager_IsAppRunningRecordValid_0100
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_IsAppRunningRecordValid_0100, TestSize.Level1)
{
    BundleInfo bundleInfo;
    std::shared_ptr<AppRunningRecord> recordOne =
        appRunningManager_->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    ASSERT_NE(recordOne, nullptr);
    auto ret = appRunningManager_->IsAppRunningRecordValid(recordOne);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: AppRunningManager_IsAppRunningRecordValid_0200
 * @tc.desc: NA
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_IsAppRunningRecordValid_0200, TestSize.Level1)
{
    auto ret = appRunningManager_->IsAppRunningRecordValid(nullptr);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: CheckAppRunningRecordForSpecifiedProcess_0100
 * @tc.desc: CheckAppRunningRecordForSpecifiedProcess.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, CheckAppRunningRecordForSpecifiedProcess_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    int32_t uid = 12345;
    std::string instanceKey = "instanceKey";
    std::string customProcessFlag = "customProcessFlag";
    ApplicationInfo appInfo;
    appInfo.name = "KeepAliveApp";
    appInfo.bundleName = "KeepAliveApplication";
    appInfo.uid = uid;
    auto app = std::make_shared<ApplicationInfo>(appInfo);
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(app, 111, "KeepAliveApplication");
    appRecord->SetInstanceKey(instanceKey);
    appRecord->SetUid(uid);
    appRecord->SetCustomProcessFlag(customProcessFlag);

    appRunningManager->appRunningRecordMap_.emplace(111, appRecord);
    auto result = appRunningManager->CheckAppRunningRecordForSpecifiedProcess(uid, instanceKey, customProcessFlag);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name: CheckAppRunningRecordForSpecifiedProcess_0200
 * @tc.desc: CheckAppRunningRecordForSpecifiedProcess.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, CheckAppRunningRecordForSpecifiedProcess_0200, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    int32_t uid = 12345;
    std::string instanceKey = "instanceKey";
    std::string customProcessFlag = "customProcessFlag";
    auto result = appRunningManager->CheckAppRunningRecordForSpecifiedProcess(uid, instanceKey, customProcessFlag);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: AppRunningManager_GetRunningProcessInfoByToken_0100
 * @tc.desc: Test GetRunningProcessInfoByToken with null token
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_GetRunningProcessInfoByToken_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    sptr<IRemoteObject> token = nullptr;
    AppExecFwk::RunningProcessInfo info;
    appRunningManager->GetRunningProcessInfoByToken(token, info);
    EXPECT_EQ(info.pid_, 0);
}

/**
 * @tc.name: AppRunningManager_GetRunningProcessInfoByToken_0200
 * @tc.desc: Test GetRunningProcessInfoByToken with valid token
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_GetRunningProcessInfoByToken_0200, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    auto token = GetTestAbilityToken();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = BUNDLE_NAME;
    appInfo->uid = TEST_UID;
    appInfo->accessTokenId = 1000;
    auto recordId = AppRecordId::Create();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, PROCESS_NAME);
    appRecord->curState_ = ApplicationState::APP_STATE_FOREGROUND;
    appRunningManager->appRunningRecordMap_.emplace(recordId, appRecord);

    AppExecFwk::RunningProcessInfo info;
    appRunningManager->GetRunningProcessInfoByToken(token, info);
    EXPECT_EQ(info.pid_, appRecord->GetPid());
}

/**
 * @tc.name: AppRunningManager_AssignRunningProcessInfoByAppRecord_0100
 * @tc.desc: Test AssignRunningProcessInfoByAppRecord with null record
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_AssignRunningProcessInfoByAppRecord_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    AppExecFwk::RunningProcessInfo info;
    int32_t result = appRunningManager->AssignRunningProcessInfoByAppRecord(appRecord, info);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: AppRunningManager_AssignRunningProcessInfoByAppRecord_0200
 * @tc.desc: Test AssignRunningProcessInfoByAppRecord with valid record
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_AssignRunningProcessInfoByAppRecord_0200, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = BUNDLE_NAME;
    appInfo->uid = TEST_UID;
    appInfo->accessTokenId = 1000;
    auto recordId = AppRecordId::Create();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, PROCESS_NAME);
    appRecord->curState_ = ApplicationState::APP_STATE_FOREGROUND;
    auto priorityObj = appRecord->GetPriorityObject();
    ASSERT_NE(priorityObj, nullptr);
    appRecord->GetPriorityObject()->SetPid(TEST_PID);

    AppExecFwk::RunningProcessInfo info;
    int32_t result = appRunningManager->AssignRunningProcessInfoByAppRecord(appRecord, info);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(info.pid_, TEST_PID);
    EXPECT_EQ(info.processName_, PROCESS_NAME);
}

/**
 * @tc.name: AppRunningManager_HandleTerminateTimeOut_0100
 * @tc.desc: Test HandleTerminateTimeOut
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_HandleTerminateTimeOut_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    int64_t eventId = 1000;
    appRunningManager->HandleTerminateTimeOut(eventId);
}

/**
 * @tc.name: AppRunningManager_GetAllAppRunningRecordCountByBundleName_0100
 * @tc.desc: Test GetAllAppRunningRecordCountByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_GetAllAppRunningRecordCountByBundleName_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    int32_t count = appRunningManager->GetAllAppRunningRecordCountByBundleName(BUNDLE_NAME);
    EXPECT_EQ(count, 0);

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = BUNDLE_NAME;
    auto recordId1 = AppRecordId::Create();
    auto appRecord1 = std::make_shared<AppRunningRecord>(appInfo, recordId1, PROCESS_NAME);
    auto recordId2 = AppRecordId::Create();
    auto appRecord2 = std::make_shared<AppRunningRecord>(appInfo, recordId2, PROCESS_NAME);
    appRunningManager->appRunningRecordMap_.emplace(recordId1, appRecord1);
    appRunningManager->appRunningRecordMap_.emplace(recordId2, appRecord2);

    count = appRunningManager->GetAllAppRunningRecordCountByBundleName(BUNDLE_NAME);
    EXPECT_EQ(count, 2);
}

/**
 * @tc.name: AppRunningManager_GetAppRunningRecordByChildRecordPid_0100
 * @tc.desc: Test GetAppRunningRecordByChildRecordPid
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_GetAppRunningRecordByChildRecordPid_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    pid_t childPid = 500;
    auto result = appRunningManager->GetAppRunningRecordByChildRecordPid(childPid);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: AppRunningManager_RemoveUIExtensionLauncherItem_0100
 * @tc.desc: Test RemoveUIExtensionLauncherItem
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_RemoveUIExtensionLauncherItem_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    pid_t pid = 100;
    int32_t result = appRunningManager->RemoveUIExtensionLauncherItem(pid);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppRunningManager_RemoveUIExtensionLauncherItemById_0100
 * @tc.desc: Test RemoveUIExtensionLauncherItemById
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_RemoveUIExtensionLauncherItemById_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    int32_t uiExtensionAbilityId = 1;
    int32_t result = appRunningManager->RemoveUIExtensionLauncherItemById(uiExtensionAbilityId);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppRunningManager_GetAllUIExtensionRootHostPid_0100
 * @tc.desc: Test GetAllUIExtensionRootHostPid
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_GetAllUIExtensionRootHostPid_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    pid_t pid = 100;
    std::vector<pid_t> hostPids;
    int32_t result = appRunningManager->GetAllUIExtensionRootHostPid(pid, hostPids);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppRunningManager_GetAllUIExtensionProviderPid_0100
 * @tc.desc: Test GetAllUIExtensionProviderPid
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerFourthTest, AppRunningManager_GetAllUIExtensionProviderPid_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    pid_t hostPid = 100;
    std::vector<pid_t> providerPids;
    int32_t result = appRunningManager->GetAllUIExtensionProviderPid(hostPid, providerPids);
    EXPECT_EQ(result, ERR_OK);
}
} // namespace AppExecFwk
} // namespace OHOS