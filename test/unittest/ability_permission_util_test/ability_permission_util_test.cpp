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
#include <unistd.h>

#define private public
#include "utils/ability_permission_util.h"
#undef private
#include "ability_info.h"
#include "ability_manager_errors.h"
#include "ability_record.h"
#include "ability_util.h"
#include "accesstoken_kit.h"
#include "app_utils.h"
#include "app_mgr_util.h"
#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "mock_app_mgr_impl.h"
#include "mock_my_flag.h"
#include "multi_instance_utils.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "parameters.h"
#include "running_process_info.h"
#include "start_ability_utils.h"
#include "user_controller.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using AbilityRequest = OHOS::AAFwk::AbilityRequest;
using OHOS::AppExecFwk::AbilityType;
using OHOS::AAFwk::AbilityPermissionUtil;
using OHOS::AppExecFwk::MockAppMgrImpl;

namespace OHOS {
namespace AAFwk {
bool AppUtils::isAllowStartAbilityWithoutCallerToken = false;
bool AppUtils::isSupportMultiInstance = false;
bool AppUtils::isStartOptionsWithAnimation = false;
bool AppUtils::isPrepareTerminateEnabled = false;

std::string MultiInstanceUtils::retInstanceKey;
bool MultiInstanceUtils::isMultiInstanceApp = false;
bool MultiInstanceUtils::isDefaultInstanceKey = false;
bool MultiInstanceUtils::isSupportedExtensionType = false;
bool MultiInstanceUtils::isInstanceKeyExist = false;

bool StartAbilityUtils::retGetApplicationInfo = false;
AppExecFwk::ApplicationInfo StartAbilityUtils::retApplicationInfo;

bool AppMgrUtil::isNullAppMgr = false;
sptr<OHOS::AppExecFwk::IAppMgr> AppMgrUtil::mockAppMgr = sptr<MockAppMgrImpl>::MakeSptr();
} // namespace AAFwk

namespace AppExecFwk {
bool BundleMgrHelper::isNullBundleMgrInstance = false;
int32_t BundleMgrHelper::retGetNameForUid = 0;
BundleInfo BundleMgrHelper::retBundleInfo;
bool BundleMgrHelper::retGetBundleInfo = 0;
} // namespace AppExecFwk

namespace AbilityRuntime {
using namespace AAFwk;

namespace {
constexpr const char* IS_DELEGATOR_CALL = "isDelegatorCall";
constexpr const char* SETTINGS = "settings";
constexpr char INSIGHT_INTENT_EXECUTE_PARAM_NAME[] = "ohos.insightIntent.executeParam.name";
} // namespace

class AbilityPermissionUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityPermissionUtilTest::SetUpTestCase(void)
{
    MyFlag::flag_ = 0;
    AppUtils::isAllowStartAbilityWithoutCallerToken = false;
    BundleMgrHelper::isNullBundleMgrInstance = false;
    BundleMgrHelper::retGetNameForUid = 0;
    BundleMgrHelper::retGetBundleInfo = 0;
    AppUtils::isStartOptionsWithAnimation = false;
    MultiInstanceUtils::isMultiInstanceApp = false;
    MultiInstanceUtils::isDefaultInstanceKey = false;
    MultiInstanceUtils::isSupportedExtensionType = false;
    MultiInstanceUtils::isInstanceKeyExist = false;
    AppMgrUtil::isNullAppMgr = false;
}

void AbilityPermissionUtilTest::TearDownTestCase(void) {}
void AbilityPermissionUtilTest::SetUp()
{
    // Reset UserController mock status before each test
    AbilityRuntime::UserController::GetInstance().SetMockUserLockStatus(
        AbilityRuntime::UserController::UserLockStatus::USER_LOCK_STATUS_BUTT);
}
void AbilityPermissionUtilTest::TearDown() {}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0100
 * @tc.desc: CheckMultiInstanceAndAppClone
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0100 start");
    Want want;
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceAndAppClone(want, 100, 0, nullptr, false);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0100 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0200
 * @tc.desc: CheckMultiInstanceAndAppClone
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0200 start");
    AppUtils::isSupportMultiInstance = false;
    Want want;
    std::string instanceKey = "app_instance_0";
    want.SetParam(Want::APP_INSTANCE_KEY, instanceKey);
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceAndAppClone(want, 100, 0, nullptr, false);
    EXPECT_EQ(result, ERR_MULTI_INSTANCE_NOT_SUPPORTED);

    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0200 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0300
 * @tc.desc: CheckMultiInstanceAndAppClone
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0300 start");
    AppUtils::isSupportMultiInstance = false;
    Want want;
    want.SetParam(Want::CREATE_APP_INSTANCE_KEY, true);
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceAndAppClone(want, 100, 0, nullptr, false);
    EXPECT_EQ(result, ERR_MULTI_INSTANCE_NOT_SUPPORTED);

    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0300 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0400
 * @tc.desc: CheckMultiInstanceAndAppClone
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0400 start");
    AppUtils::isSupportMultiInstance = true;
    StartAbilityUtils::retGetApplicationInfo = false;
    Want want;
    want.SetParam(Want::CREATE_APP_INSTANCE_KEY, true);
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceAndAppClone(want, 100, 0, nullptr, false);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0400 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0500
 * @tc.desc: CheckMultiInstanceAndAppClone
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0500 start");
    AppUtils::isSupportMultiInstance = true;
    StartAbilityUtils::retGetApplicationInfo = true;
    StartAbilityUtils::retApplicationInfo.multiAppMode.multiAppModeType = AppExecFwk::MultiAppModeType::UNSPECIFIED;
    Want want;
    want.SetParam(Want::CREATE_APP_INSTANCE_KEY, true);
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceAndAppClone(want, 100, 0, nullptr, false);
    EXPECT_EQ(result, ERR_MULTI_APP_NOT_SUPPORTED);

    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0500 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0600
 * @tc.desc: CheckMultiInstanceAndAppClone
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0600 start");
    AppUtils::isSupportMultiInstance = true;
    StartAbilityUtils::retGetApplicationInfo = true;
    StartAbilityUtils::retApplicationInfo.multiAppMode.multiAppModeType = AppExecFwk::MultiAppModeType::UNSPECIFIED;
    Want want;
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceAndAppClone(want, 100, 1, nullptr, false);
    EXPECT_EQ(result, ERR_MULTI_APP_NOT_SUPPORTED);

    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0600 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0700
 * @tc.desc: CheckMultiInstanceAndAppClone
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0700 start");
    AppUtils::isSupportMultiInstance = true;
    StartAbilityUtils::retGetApplicationInfo = true;
    StartAbilityUtils::retApplicationInfo.multiAppMode.multiAppModeType = AppExecFwk::MultiAppModeType::MULTI_INSTANCE;
    Want want;
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceAndAppClone(want, 100, 1, nullptr, false);
    EXPECT_EQ(result, ERR_NOT_SUPPORT_APP_CLONE);

    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0700 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0800
 * @tc.desc: CheckMultiInstanceAndAppClone
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0800 start");
    AppUtils::isSupportMultiInstance = true;
    StartAbilityUtils::retGetApplicationInfo = true;
    int32_t maxCount = 5;
    StartAbilityUtils::retApplicationInfo.multiAppMode.multiAppModeType = AppExecFwk::MultiAppModeType::MULTI_INSTANCE;
    StartAbilityUtils::retApplicationInfo.multiAppMode.maxCount = maxCount;
    Want want;
    want.SetParam(Want::CREATE_APP_INSTANCE_KEY, true);
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceAndAppClone(want, 100, 0, nullptr, false);
    EXPECT_EQ(result, AbilityPermissionUtil::GetInstance().CheckMultiInstance(want, nullptr, maxCount, false));

    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0800 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstance_0100
 * @tc.desc: CheckMultiInstance
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstance_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0100 start");

    AppMgrUtil::isNullAppMgr = true;
    Want want;
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstance(want, nullptr, 0, false);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0100 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstance_0200
 * @tc.desc: CheckMultiInstance
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstance_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0200 start");

    MockAppMgrImpl::retGetAllRunningInstanceKeysByBundleName = -1;
    Want want;
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstance(want, nullptr, 0, false);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0200 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstance_0300
 * @tc.desc: CheckMultiInstance
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstance_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0300 start");

    AppMgrUtil::isNullAppMgr = false;
    MockAppMgrImpl::retGetAllRunningInstanceKeysByBundleName = 0;
    sptr<IRemoteObject> callerToken = sptr<Token>::MakeSptr();
    std::string bundleName = "com.ohos.test";
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>();
    abilityRecord->abilityInfo.bundleName = bundleName;
    Token::abilityRecord = abilityRecord;

    Want want;
    want.SetBundle(bundleName);
    std::string instanceKey = "app_instance_0";
    want.SetParam(Want::APP_INSTANCE_KEY, instanceKey);
    want.SetParam(Want::CREATE_APP_INSTANCE_KEY, true);
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstance(want, callerToken, 1, false);
    EXPECT_EQ(result, ERR_APP_INSTANCE_KEY_NOT_SUPPORT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0300 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstance_0400
 * @tc.desc: CheckMultiInstance
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstance_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0400 start");

    AppMgrUtil::isNullAppMgr = false;
    MockAppMgrImpl::retGetAllRunningInstanceKeysByBundleName = 0;
    sptr<IRemoteObject> callerToken = sptr<Token>::MakeSptr();
    std::string bundleName = "com.ohos.test";
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>();
    abilityRecord->abilityInfo.bundleName = bundleName;
    Token::abilityRecord = abilityRecord;
    MockAppMgrImpl::retInstanceKeys = { "app_instance_0" };

    Want want;
    want.SetBundle(bundleName);
    want.SetParam(Want::CREATE_APP_INSTANCE_KEY, true);
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstance(want, callerToken, 1, false);
    EXPECT_EQ(result, ERR_UPPER_LIMIT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0400 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstance_0500
 * @tc.desc: CheckMultiInstance
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstance_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0500 start");

    AppMgrUtil::isNullAppMgr = false;
    MockAppMgrImpl::retGetAllRunningInstanceKeysByBundleName = 0;
    sptr<IRemoteObject> callerToken = sptr<Token>::MakeSptr();
    std::string bundleName = "com.ohos.test";
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>();
    abilityRecord->abilityInfo.bundleName = bundleName;
    Token::abilityRecord = abilityRecord;
    MockAppMgrImpl::retInstanceKeys = { "app_instance_0" };

    Want want;
    want.SetBundle(bundleName);
    want.SetParam(Want::CREATE_APP_INSTANCE_KEY, true);
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstance(want, callerToken, 2, false);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0500 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstance_0600
 * @tc.desc: CheckMultiInstance
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstance_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0600 start");

    AppMgrUtil::isNullAppMgr = false;
    MockAppMgrImpl::retGetAllRunningInstanceKeysByBundleName = 0;
    sptr<IRemoteObject> callerToken = sptr<Token>::MakeSptr();
    std::string bundleName = "com.ohos.test";
    std::string instanceKey = "app_instance_0";
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>();
    abilityRecord->abilityInfo.bundleName = bundleName;
    abilityRecord->instanceKey = instanceKey;
    Token::abilityRecord = abilityRecord;
    std::vector<std::string> instanceKeys = { instanceKey };
    MockAppMgrImpl::retInstanceKeys = instanceKeys;

    Want want;
    want.SetBundle("com.ohos.diff");
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstance(want, callerToken, 1, false);
    EXPECT_EQ(result, AbilityPermissionUtil::GetInstance().UpdateInstanceKey(
        want, instanceKey, instanceKeys, instanceKey));
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0600 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstance_0700
 * @tc.desc: CheckMultiInstance
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstance_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0700 start");

    AppMgrUtil::isNullAppMgr = false;
    MockAppMgrImpl::retGetAllRunningInstanceKeysByBundleName = 0;
    sptr<IRemoteObject> callerToken = nullptr;
    std::string bundleName = "com.ohos.test";
    std::string instanceKey = "app_instance_0";
    Token::abilityRecord = nullptr;
    std::vector<std::string> instanceKeys = { instanceKey };
    MockAppMgrImpl::retInstanceKeys = instanceKeys;
    AppUtils::isStartOptionsWithAnimation = true;
    AbilityPermissionUtil instance;
    instance.startSelfUIAbilityRecords_.push_back({getprocpid(), 1000, 1});
    PermissionVerification::retVerifyStartSelfUIAbility = true;

    Want want;
    want.SetBundle(bundleName);
    auto result = instance.CheckMultiInstance(want, callerToken, 1, false);
    EXPECT_EQ(result, instance.UpdateInstanceKey(
        want, instanceKey, instanceKeys, instanceKey));
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0700 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstance_0800
 * @tc.desc: CheckMultiInstance
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstance_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0800 start");

    AppMgrUtil::isNullAppMgr = false;
    MockAppMgrImpl::retGetAllRunningInstanceKeysByBundleName = 0;
    sptr<IRemoteObject> callerToken = nullptr;
    std::string bundleName = "com.ohos.test";
    std::string instanceKey = "app_instance_0";
    Token::abilityRecord = nullptr;
    std::vector<std::string> instanceKeys = { instanceKey };
    MockAppMgrImpl::retInstanceKeys = instanceKeys;
    AppUtils::isStartOptionsWithAnimation = false;

    Want want;
    want.SetBundle(bundleName);
    want.SetParam(Want::CREATE_APP_INSTANCE_KEY, true);
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstance(want, callerToken, 1, false);
    EXPECT_EQ(result, ERR_CREATE_NEW_INSTANCE_NOT_SUPPORT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0800 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstance_0900
 * @tc.desc: CheckMultiInstance
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstance_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0900 start");

    AppMgrUtil::isNullAppMgr = false;
    MockAppMgrImpl::retGetAllRunningInstanceKeysByBundleName = 0;
    sptr<IRemoteObject> callerToken = nullptr;
    std::string bundleName = "com.ohos.test";
    std::string instanceKey = "app_instance_0";
    Token::abilityRecord = nullptr;
    std::vector<std::string> instanceKeys = { instanceKey };
    MockAppMgrImpl::retInstanceKeys = instanceKeys;
    AppUtils::isStartOptionsWithAnimation = false;

    Want want;
    want.SetBundle(bundleName);
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstance(want, callerToken, 1, false);
    EXPECT_EQ(result, AbilityPermissionUtil::GetInstance().UpdateInstanceKey(
        want, instanceKey, instanceKeys, instanceKey));
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0900 end");
}

/**
 * @tc.name: AbilityPermissionUtil_UpdateInstanceKey_0100
 * @tc.desc: UpdateInstanceKey
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_UpdateInstanceKey_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_UpdateInstanceKey_0100 start");

    Want want;
    std::vector<std::string> instanceKeyArray;
    auto result = AbilityPermissionUtil::GetInstance().UpdateInstanceKey(want, "", instanceKeyArray, "");
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_UpdateInstanceKey_0100 end");
}

/**
 * @tc.name: AbilityPermissionUtil_UpdateInstanceKey_0200
 * @tc.desc: UpdateInstanceKey
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_UpdateInstanceKey_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_UpdateInstanceKey_0200 start");

    Want want;
    std::string originInstanceKey = "app_instance_0";
    std::vector<std::string> instanceKeyArray;
    auto result = AbilityPermissionUtil::GetInstance().UpdateInstanceKey(want, originInstanceKey, instanceKeyArray, "");
    EXPECT_EQ(result, ERR_INVALID_APP_INSTANCE_KEY);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_UpdateInstanceKey_0200 end");
}

/**
 * @tc.name: AbilityPermissionUtil_UpdateInstanceKey_0300
 * @tc.desc: UpdateInstanceKey
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_UpdateInstanceKey_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_UpdateInstanceKey_0300 start");

    Want want;
    std::string originInstanceKey = "app_instance_0";
    std::vector<std::string> instanceKeyArray;
    instanceKeyArray.push_back(originInstanceKey);
    auto result = AbilityPermissionUtil::GetInstance().UpdateInstanceKey(want, originInstanceKey, instanceKeyArray, "");
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_UpdateInstanceKey_0300 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0100
 * @tc.desc: CheckMultiInstanceKeyForExtension
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0100 start");

    AbilityRequest abilityRequest;
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceKeyForExtension(abilityRequest);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0100 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0200
 * @tc.desc: CheckMultiInstanceKeyForExtension
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0200 start");

    AppUtils::isSupportMultiInstance = false;
    MultiInstanceUtils::retInstanceKey = "app_instance_0";
    AbilityRequest abilityRequest;
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceKeyForExtension(abilityRequest);
    EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0200 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0300
 * @tc.desc: CheckMultiInstanceKeyForExtension
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0300 start");

    AppUtils::isSupportMultiInstance = true;
    MultiInstanceUtils::isMultiInstanceApp = false;
    MultiInstanceUtils::retInstanceKey = "app_instance_0";
    AbilityRequest abilityRequest;
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceKeyForExtension(abilityRequest);
    EXPECT_EQ(result, ERR_MULTI_INSTANCE_NOT_SUPPORTED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0300 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0400
 * @tc.desc: CheckMultiInstanceKeyForExtension
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0400 start");

    MultiInstanceUtils::retInstanceKey = "app_instance_0";
    AppUtils::isSupportMultiInstance = true;
    MultiInstanceUtils::isMultiInstanceApp = true;
    MultiInstanceUtils::isDefaultInstanceKey = true;
    AbilityRequest abilityRequest;
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceKeyForExtension(abilityRequest);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0400 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0500
 * @tc.desc: CheckMultiInstanceKeyForExtension
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0500 start");

    MultiInstanceUtils::retInstanceKey = "app_instance_0";
    AppUtils::isSupportMultiInstance = true;
    MultiInstanceUtils::isMultiInstanceApp = true;
    MultiInstanceUtils::isDefaultInstanceKey = false;
    MultiInstanceUtils::isSupportedExtensionType = false;
    AbilityRequest abilityRequest;
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceKeyForExtension(abilityRequest);
    EXPECT_EQ(result, ERR_INVALID_EXTENSION_TYPE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0500 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0600
 * @tc.desc: CheckMultiInstanceKeyForExtension
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0600 start");

    MultiInstanceUtils::retInstanceKey = "app_instance_0";
    AppUtils::isSupportMultiInstance = true;
    MultiInstanceUtils::isMultiInstanceApp = true;
    MultiInstanceUtils::isDefaultInstanceKey = false;
    MultiInstanceUtils::isSupportedExtensionType = true;
    MultiInstanceUtils::isInstanceKeyExist = false;
    AbilityRequest abilityRequest;
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceKeyForExtension(abilityRequest);
    EXPECT_EQ(result, ERR_INVALID_APP_INSTANCE_KEY);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0600 end");
}

/**
 * @tc.name: AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0700
 * @tc.desc: CheckMultiInstanceKeyForExtension
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0700 start");

    MultiInstanceUtils::retInstanceKey = "app_instance_0";
    AppUtils::isSupportMultiInstance = true;
    MultiInstanceUtils::isMultiInstanceApp = true;
    MultiInstanceUtils::isDefaultInstanceKey = false;
    MultiInstanceUtils::isSupportedExtensionType = true;
    MultiInstanceUtils::isInstanceKeyExist = true;
    AbilityRequest abilityRequest;
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceKeyForExtension(abilityRequest);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0700 end");
}

/**
 * @tc.name: IsDominateScreen_0100
 * @tc.desc: IsDominateScreen_0100 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, IsDominateScreen_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_0100 start");

    Want want;
    bool isPendingWantCaller = true;
    bool ret = AbilityPermissionUtil::GetInstance().IsDominateScreen(want, isPendingWantCaller);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_0100 end");
}

/**
 * @tc.name: IsDominateScreen_0200
 * @tc.desc: IsDominateScreen_0200 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, IsDominateScreen_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_0200 start");

    MyFlag::flag_ = 1;
    Want want;
    bool isPendingWantCaller = false;
    bool ret = AbilityPermissionUtil::GetInstance().IsDominateScreen(want, isPendingWantCaller);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_0200 end");
}

/**
 * @tc.name: IsDominateScreen_0300
 * @tc.desc: IsDominateScreen_0300 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, IsDominateScreen_0300, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_0300 start");

    MyFlag::flag_ = 2;
    Want want;
    bool isPendingWantCaller = false;
    bool ret = AbilityPermissionUtil::GetInstance().IsDominateScreen(want, isPendingWantCaller);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_0300 end");
}

/**
 * @tc.name: IsDominateScreen_0400
 * @tc.desc: IsDominateScreen_0400 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, IsDominateScreen_0400, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_0400 start");

    MyFlag::flag_ = 3;
    Want want;
    bool isPendingWantCaller = false;
    bool ret = AbilityPermissionUtil::GetInstance().IsDominateScreen(want, isPendingWantCaller);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_0400 end");
}

/**
 * @tc.name: IsDominateScreen_0500
 * @tc.desc: IsDominateScreen_0500 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, IsDominateScreen_0500, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_0500 start");

    MyFlag::flag_ = 3;
    Want want;
    want.SetParam("ohos.insightIntent.executeParam.name", true);
    bool isPendingWantCaller = false;
    bool ret = AbilityPermissionUtil::GetInstance().IsDominateScreen(want, isPendingWantCaller);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_0500 end");
}

/**
 * @tc.name: IsDominateScreen_0600
 * @tc.desc: IsDominateScreen_0600 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, IsDominateScreen_0600, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_0600 start");

    AppUtils::isAllowStartAbilityWithoutCallerToken = true;
    Want want;
    bool isPendingWantCaller = false;
    bool ret = AbilityPermissionUtil::GetInstance().IsDominateScreen(want, isPendingWantCaller);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_0600 end");
}

/**
 * @tc.name: IsDominateScreen_0700
 * @tc.desc: IsDominateScreen_0700 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, IsDominateScreen_0700, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_0700 start");

    Want want;
    want.SetParam(IS_DELEGATOR_CALL, true);
    bool isPendingWantCaller = false;
    bool ret = AbilityPermissionUtil::GetInstance().IsDominateScreen(want, isPendingWantCaller);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_0700 end");
}

/**
 * @tc.name: IsDominateScreen_0800
 * @tc.desc: IsDominateScreen_0800 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, IsDominateScreen_0800, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_0800 start");

    Want want;
    want.SetParam(INSIGHT_INTENT_EXECUTE_PARAM_NAME, true);
    bool isPendingWantCaller = false;
    bool ret = AbilityPermissionUtil::GetInstance().IsDominateScreen(want, isPendingWantCaller);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_0800 end");
}

/**
 * @tc.name: IsDominateScreen_0900
 * @tc.desc: IsDominateScreen_0900 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, IsDominateScreen_0900, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_0900 start");

    AppUtils::isAllowStartAbilityWithoutCallerToken = true;
    BundleMgrHelper::isNullBundleMgrInstance = true;
    Want want;
    want.SetElementName(SETTINGS, SETTINGS);
    bool isPendingWantCaller = false;
    bool ret = AbilityPermissionUtil::GetInstance().IsDominateScreen(want, isPendingWantCaller);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_0900 end");
}

/**
 * @tc.name: IsDominateScreen_1000
 * @tc.desc: IsDominateScreen_1000 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, IsDominateScreen_1000, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_1000 start");

    AppUtils::isAllowStartAbilityWithoutCallerToken = true;
    BundleMgrHelper::retGetNameForUid = -1;
    Want want;
    want.SetElementName(SETTINGS, SETTINGS);
    bool isPendingWantCaller = false;
    bool ret = AbilityPermissionUtil::GetInstance().IsDominateScreen(want, isPendingWantCaller);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_1000 end");
}

/**
 * @tc.name: IsDominateScreen_1100
 * @tc.desc: IsDominateScreen_1100 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, IsDominateScreen_1100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_1100 start");

    AppUtils::isAllowStartAbilityWithoutCallerToken = true;
    BundleMgrHelper::retBundleInfo.applicationInfo.needAppDetail = true;
    Want want;
    want.SetElementName(SETTINGS, SETTINGS);
    bool isPendingWantCaller = false;
    bool ret = AbilityPermissionUtil::GetInstance().IsDominateScreen(want, isPendingWantCaller);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_1100 end");
}

/**
 * @tc.name: IsDominateScreen_1200
 * @tc.desc: IsDominateScreen_1200 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, IsDominateScreen_1200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_1200 start");

    AppUtils::isStartOptionsWithAnimation = true;
    PermissionVerification::retVerifyStartSelfUIAbility = true;
    Want want;
    bool isPendingWantCaller = false;
    bool ret = AbilityPermissionUtil::GetInstance().IsDominateScreen(want, isPendingWantCaller);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_1200 end");
}

/**
 * @tc.name: IsDominateScreen_1300
 * @tc.desc: IsDominateScreen_1300 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, IsDominateScreen_1300, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_1300 start");

    AppUtils::isStartOptionsWithAnimation = false;
    Want want;
    bool isPendingWantCaller = false;
    bool ret = AbilityPermissionUtil::GetInstance().IsDominateScreen(want, isPendingWantCaller);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_1300 end");
}

/**
 * @tc.name: IsDominateScreen_1400
 * @tc.desc: IsDominateScreen_1400 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, IsDominateScreen_1400, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_1400 start");

    AppUtils::isAllowStartAbilityWithoutCallerToken = true;
    BundleMgrHelper::retGetBundleInfo = -1;
    Want want;
    want.SetElementName(SETTINGS, SETTINGS);
    bool isPendingWantCaller = false;
    bool ret = AbilityPermissionUtil::GetInstance().IsDominateScreen(want, isPendingWantCaller);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil IsDominateScreen_1400 end");
}

/**
 * @tc.name: GetClosestHapTokenId_0100
 * @tc.desc: a hap token id is unresolved without the ATM service -> ERR_INVALID_VALUE, out-param 0
 */
HWTEST_F(AbilityPermissionUtilTest, GetClosestHapTokenId_0100, TestSize.Level2)
{
    uint32_t tokenId = 0x12345; // a hap token id
    uint32_t hapTokenId = 0;
    // Resolution now goes through the access_token kit (IPC); with no ATM service in the
    // unit-test env it fail-closes to ERR_INVALID_VALUE and zeroes the out-param.
    EXPECT_EQ(AbilityPermissionUtil::GetInstance().GetClosestHapTokenId(tokenId, hapTokenId), ERR_INVALID_VALUE);
    EXPECT_EQ(hapTokenId, 0);
}

/**
 * @tc.name: GetClosestHapTokenId_0200
 * @tc.desc: tokenId 0 (invalid) -> ERR_INVALID_VALUE, out-param zeroed
 */
HWTEST_F(AbilityPermissionUtilTest, GetClosestHapTokenId_0200, TestSize.Level2)
{
    uint32_t tokenId = 0;
    uint32_t hapTokenId = 1; // ensure it gets overwritten
    EXPECT_EQ(AbilityPermissionUtil::GetInstance().GetClosestHapTokenId(tokenId, hapTokenId), ERR_INVALID_VALUE);
    EXPECT_EQ(hapTokenId, 0);
}

/**
 * @tc.name: GetClosestHapTokenId_0300
 * @tc.desc: a non-hap token (0x800000 bit) takes the access_token kit path; with no ATM service
 *          (or an unresolvable token) it fails
 */
HWTEST_F(AbilityPermissionUtilTest, GetClosestHapTokenId_0300, TestSize.Level2)
{
    uint32_t tokenId = 0x800123; // 0x800000 bit set -> non-hap
    uint32_t hapTokenId = 0;
    EXPECT_EQ(AbilityPermissionUtil::GetInstance().GetClosestHapTokenId(tokenId, hapTokenId), ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetClosestHapTokenId_0400
 * @tc.desc: the actual calling token is unresolvable without the ATM service -> ERR_INVALID_VALUE,
 *           out-param zeroed (env-independent: any calling token fail-closes without the service)
 */
HWTEST_F(AbilityPermissionUtilTest, GetClosestHapTokenId_0400, TestSize.Level2)
{
    MyFlag::flag_ = 0;
    AppUtils::isAllowStartAbilityWithoutCallerToken = false;
    PermissionVerification::retVerifyStartSelfUIAbility = false;
    uint32_t callingTokenId = IPCSkeleton::GetCallingTokenID();
    uint32_t hapTokenId = 1;
    int32_t hapRet = AbilityPermissionUtil::GetInstance().GetClosestHapTokenId(callingTokenId, hapTokenId);
    EXPECT_EQ(hapRet, ERR_INVALID_VALUE);
    EXPECT_EQ(hapTokenId, 0);
}

/**
 * @tc.name: StartSelfUIAbilityRecordGuard_0100
 * @tc.desc: StartSelfUIAbilityRecordGuard_0100 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, StartSelfUIAbilityRecordGuard_0100, TestSize.Level2)
{
    pid_t pid = 8888;
    int32_t tokenId = 1;

    AbilityPermissionUtil::GetInstance().startSelfUIAbilityRecords_.clear();
    StartSelfUIAbilityRecordGuard startSelfUIAbilityRecordGuard(pid, tokenId);
    EXPECT_FALSE(AbilityPermissionUtil::GetInstance().startSelfUIAbilityRecords_.empty());

    AbilityPermissionUtil::GetInstance().AddStartSelfUIAbilityRecord(pid, tokenId);
    for (const auto &item : AbilityPermissionUtil::GetInstance().startSelfUIAbilityRecords_) {
        EXPECT_EQ(item[2], 2);
    }
}

/**
 * @tc.name: AddStartSelfUIAbilityRecord_0100
 * @tc.desc: AddStartSelfUIAbilityRecord_0100 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, AddStartSelfUIAbilityRecord_0100, TestSize.Level2)
{
    pid_t pid = 8888;
    int32_t tokenId = 1000;

    AbilityPermissionUtil instance;
    EXPECT_TRUE(instance.startSelfUIAbilityRecords_.empty());
    instance.startSelfUIAbilityRecords_.push_back({pid, tokenId, 1});
    EXPECT_FALSE(instance.startSelfUIAbilityRecords_.empty());

    instance.AddStartSelfUIAbilityRecord(pid, tokenId);
    EXPECT_EQ(instance.startSelfUIAbilityRecords_[0][2], 2);
}

/**
 * @tc.name: RemoveStartSelfUIAbilityRecord_0100
 * @tc.desc: RemoveStartSelfUIAbilityRecord_0100 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, RemoveStartSelfUIAbilityRecord_0100, TestSize.Level2)
{
    pid_t pid = 8888;
    int32_t tokenId = 1000;

    AbilityPermissionUtil instance;
    EXPECT_TRUE(instance.startSelfUIAbilityRecords_.empty());
    instance.startSelfUIAbilityRecords_.push_back({pid, tokenId, 1});
    EXPECT_FALSE(instance.startSelfUIAbilityRecords_.empty());

    instance.RemoveStartSelfUIAbilityRecord(pid);
    EXPECT_TRUE(instance.startSelfUIAbilityRecords_.empty());
}

/**
 * @tc.name: RemoveStartSelfUIAbilityRecord_0200
 * @tc.desc: RemoveStartSelfUIAbilityRecord_0200 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, RemoveStartSelfUIAbilityRecord_0200, TestSize.Level2)
{
    pid_t pid = 8888;
    AbilityPermissionUtil::GetInstance().startSelfUIAbilityRecords_.clear();
    AbilityPermissionUtil::GetInstance().RemoveStartSelfUIAbilityRecord(pid);
    EXPECT_EQ(AbilityPermissionUtil::GetInstance().startSelfUIAbilityRecords_.size(), 0);

    AbilityPermissionUtil::GetInstance().startSelfUIAbilityRecords_.push_back({pid, 1, 1});
    EXPECT_EQ(AbilityPermissionUtil::GetInstance().startSelfUIAbilityRecords_.size(), 1);
    AbilityPermissionUtil::GetInstance().RemoveStartSelfUIAbilityRecord(pid);
    EXPECT_EQ(AbilityPermissionUtil::GetInstance().startSelfUIAbilityRecords_.size(), 0);

    AbilityPermissionUtil::GetInstance().startSelfUIAbilityRecords_.push_back({pid, 1, 2});
    AbilityPermissionUtil::GetInstance().RemoveStartSelfUIAbilityRecord(pid);
    for (const auto &item : AbilityPermissionUtil::GetInstance().startSelfUIAbilityRecords_) {
        EXPECT_EQ(item[2], 1);
    }
}

/**
 * @tc.name: GetTokenIdByPid_0100
 * @tc.desc: GetTokenIdByPid_0100 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, GetTokenIdByPid_0100, TestSize.Level2)
{
    pid_t pid = 8888;
    AbilityPermissionUtil::GetInstance().startSelfUIAbilityRecords_.clear();
    auto ret = AbilityPermissionUtil::GetInstance().GetTokenIdByPid(pid);
    EXPECT_EQ(ret, -1);

    AbilityPermissionUtil::GetInstance().startSelfUIAbilityRecords_.push_back({pid, 1, 1});
    ret = AbilityPermissionUtil::GetInstance().GetTokenIdByPid(pid);
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name: IsStartSelfUIAbility_0100
 * @tc.desc: IsStartSelfUIAbility_0100 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, IsStartSelfUIAbility_0100, TestSize.Level2)
{
    AppUtils::isStartOptionsWithAnimation = false;
    auto ret = AbilityPermissionUtil::GetInstance().IsStartSelfUIAbility();
    EXPECT_EQ(ret, false);

    AppUtils::isStartOptionsWithAnimation = true;
    ret = AbilityPermissionUtil::GetInstance().IsStartSelfUIAbility();
    EXPECT_EQ(ret, false);

    AbilityPermissionUtil instance;
    instance.startSelfUIAbilityRecords_.push_back({getprocpid(), 1000, 1});
    PermissionVerification::retVerifyStartSelfUIAbility = false;
    ret = instance.IsStartSelfUIAbility();
    EXPECT_EQ(ret, false);

    PermissionVerification::retVerifyStartSelfUIAbility = true;
    ret = instance.IsStartSelfUIAbility();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: CheckStartRecentAbility_0100
 * @tc.desc: CheckStartRecentAbility_0100 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, CheckStartRecentAbility_0100, TestSize.Level1)
{
    Want want;
    AbilityRequest request;
    want.SetParam(Want::PARAM_RESV_START_RECENT, false);
    auto ret = AbilityPermissionUtil::GetInstance().CheckStartRecentAbility(want, request);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: SetStartAbilityInfo_0200
 * @tc.desc: CheckStartRecentAbility_0200 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, CheckStartRecentAbility_0200, TestSize.Level1)
{
    Want want;
    AbilityRequest request;
    want.SetParam(Want::PARAM_RESV_START_RECENT, false);
    auto ret = AbilityPermissionUtil::GetInstance().CheckStartRecentAbility(want, request);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: CheckPrepareTerminateEnable_0100
 * @tc.desc: CheckPrepareTerminateEnable_0100 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, CheckPrepareTerminateEnable_0100, TestSize.Level1)
{
    AppUtils::isPrepareTerminateEnabled = false;
    auto ret = AbilityPermissionUtil::GetInstance().CheckPrepareTerminateEnable(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: CheckPrepareTerminateEnable_0200
 * @tc.desc: CheckPrepareTerminateEnable_0200 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, CheckPrepareTerminateEnable_0200, TestSize.Level1)
{
    AppUtils::isPrepareTerminateEnabled = true;
    auto ret = AbilityPermissionUtil::GetInstance().CheckPrepareTerminateEnable(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>();
    abilityRecord->isTerminating = true;
    ret = AbilityPermissionUtil::GetInstance().CheckPrepareTerminateEnable(abilityRecord);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: CheckPrepareTerminateEnable_0300
 * @tc.desc: CheckPrepareTerminateEnable_0300 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, CheckPrepareTerminateEnable_0300, TestSize.Level1)
{
    AppUtils::isPrepareTerminateEnabled = true;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>();
    abilityRecord->isTerminating = false;
    abilityRecord->abilityInfo.isStageBasedModel = false;
    auto ret = AbilityPermissionUtil::GetInstance().CheckPrepareTerminateEnable(abilityRecord);
    EXPECT_EQ(ret, RESOLVE_CALL_ABILITY_TYPE_ERR);

    abilityRecord->abilityInfo.isStageBasedModel = true;
    abilityRecord->abilityInfo.type = AppExecFwk::AbilityType::UNKNOWN;
    ret = AbilityPermissionUtil::GetInstance().CheckPrepareTerminateEnable(abilityRecord);
    EXPECT_EQ(ret, RESOLVE_CALL_ABILITY_TYPE_ERR);
}

/**
 * @tc.name: CheckPrepareTerminateEnable_0400
 * @tc.desc: CheckPrepareTerminateEnable_0400 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, CheckPrepareTerminateEnable_0400, TestSize.Level1)
{
    AppUtils::isPrepareTerminateEnabled = true;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>();
    abilityRecord->isTerminating = false;
    abilityRecord->abilityInfo.isStageBasedModel = true;
    abilityRecord->abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    PermissionVerification::retVerifyPrepareTerminatePermission = false;
    auto ret = AbilityPermissionUtil::GetInstance().CheckPrepareTerminateEnable(abilityRecord);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: CheckPrepareTerminateEnable_0500
 * @tc.desc: CheckPrepareTerminateEnable_0500 Test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, CheckPrepareTerminateEnable_0500, TestSize.Level1)
{
    AppUtils::isPrepareTerminateEnabled = true;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>();
    abilityRecord->isTerminating = false;
    abilityRecord->abilityInfo.isStageBasedModel = true;
    abilityRecord->abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    PermissionVerification::retVerifyPrepareTerminatePermission = true;
    auto ret = AbilityPermissionUtil::GetInstance().CheckPrepareTerminateEnable(abilityRecord);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: NeedCheckStatusBar_0100
 * @tc.desc: UIAbility true; uid true;
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, NeedCheckStatusBar_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil NeedCheckStatusBar_0100 start");

    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>();
    abilityRecord->uid_ = 1000;
    abilityRecord->instanceKey = "instanceKey";

    AbilityRequest abilityRequest;
    std::string instanceKey = "instanceKey";
    abilityRequest.want.SetParam(Want::APP_INSTANCE_KEY, instanceKey);
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.uid = 1000;
    abilityRequest.appInfo.multiAppMode.multiAppModeType = AppExecFwk::MultiAppModeType::MULTI_INSTANCE;

    auto ret = AbilityPermissionUtil::GetInstance().NeedCheckStatusBar(abilityRecord, abilityRequest);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil NeedCheckStatusBar_0100 end");
}

/**
 * @tc.name: NeedCheckStatusBar_0200
 * @tc.desc: UIAbility false; uid true;
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, NeedCheckStatusBar_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil NeedCheckStatusBar_0200 start");

    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>();
    abilityRecord->uid_ = 1000;
    abilityRecord->instanceKey = "instanceKey";

    AbilityRequest abilityRequest;
    std::string instanceKey = "instanceKey";
    abilityRequest.want.SetParam(Want::APP_INSTANCE_KEY, instanceKey);
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    abilityRequest.abilityInfo.uid = 1000;
    abilityRequest.appInfo.multiAppMode.multiAppModeType = AppExecFwk::MultiAppModeType::MULTI_INSTANCE;

    auto ret = AbilityPermissionUtil::GetInstance().NeedCheckStatusBar(abilityRecord, abilityRequest);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil NeedCheckStatusBar_0200 end");
}

/**
 * @tc.name: NeedCheckStatusBar_0300
 * @tc.desc: UIAbility true; uid false;
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, NeedCheckStatusBar_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil NeedCheckStatusBar_0300 start");

    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>();
    abilityRecord->uid_ = 1000;
    abilityRecord->instanceKey = "instanceKey";

    AbilityRequest abilityRequest;
    std::string instanceKey = "instanceKey";
    abilityRequest.want.SetParam(Want::APP_INSTANCE_KEY, instanceKey);
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.uid = 1100;
    abilityRequest.appInfo.multiAppMode.multiAppModeType = AppExecFwk::MultiAppModeType::MULTI_INSTANCE;

    auto ret = AbilityPermissionUtil::GetInstance().NeedCheckStatusBar(abilityRecord, abilityRequest);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil NeedCheckStatusBar_0300 end");
}

/**
 * @tc.name: NeedCheckStatusBar_0400
 * @tc.desc: UIAbility false; uid false;
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, NeedCheckStatusBar_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil NeedCheckStatusBar_0400 start");

    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>();
    abilityRecord->uid_ = 1000;
    abilityRecord->instanceKey = "instanceKey";

    AbilityRequest abilityRequest;
    std::string instanceKey = "instanceKey";
    abilityRequest.want.SetParam(Want::APP_INSTANCE_KEY, instanceKey);
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    abilityRequest.abilityInfo.uid = 1100;
    abilityRequest.appInfo.multiAppMode.multiAppModeType = AppExecFwk::MultiAppModeType::MULTI_INSTANCE;

    auto ret = AbilityPermissionUtil::GetInstance().NeedCheckStatusBar(abilityRecord, abilityRequest);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil NeedCheckStatusBar_0400 end");
}

/**
 * @tc.name: NeedCheckStatusBar_0500
 * @tc.desc: UIAbility true; uid true; isMultiInstance true; InstanceKey true;
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, NeedCheckStatusBar_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil NeedCheckStatusBar_0500 start");

    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>();
    abilityRecord->uid_ = 1000;
    abilityRecord->instanceKey = "instanceKey";

    AbilityRequest abilityRequest;
    std::string instanceKey = "instanceKey";
    abilityRequest.want.SetParam(Want::APP_INSTANCE_KEY, instanceKey);
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.uid = 1000;
    abilityRequest.appInfo.multiAppMode.multiAppModeType = AppExecFwk::MultiAppModeType::MULTI_INSTANCE;

    auto ret = AbilityPermissionUtil::GetInstance().NeedCheckStatusBar(abilityRecord, abilityRequest);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil NeedCheckStatusBar_0500 end");
}

/**
 * @tc.name: NeedCheckStatusBar_0600
 * @tc.desc: UIAbility true; uid true; isMultiInstance false; InstanceKey true;
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, NeedCheckStatusBar_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil NeedCheckStatusBar_0600 start");

    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>();
    abilityRecord->uid_ = 1000;
    abilityRecord->instanceKey = "instanceKey";

    AbilityRequest abilityRequest;
    std::string instanceKey = "instanceKey";
    abilityRequest.want.SetParam(Want::APP_INSTANCE_KEY, instanceKey);
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.uid = 1000;
    abilityRequest.appInfo.multiAppMode.multiAppModeType = AppExecFwk::MultiAppModeType::APP_CLONE;

    auto ret = AbilityPermissionUtil::GetInstance().NeedCheckStatusBar(abilityRecord, abilityRequest);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil NeedCheckStatusBar_0600 end");
}

/**
 * @tc.name: NeedCheckStatusBar_0700
 * @tc.desc: UIAbility true; uid true; isMultiInstance true; InstanceKey false;
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, NeedCheckStatusBar_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil NeedCheckStatusBar_0700 start");

    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>();
    abilityRecord->uid_ = 1000;
    abilityRecord->instanceKey = "instanceKey";

    AbilityRequest abilityRequest;
    std::string instanceKey = "instanceKey123";
    abilityRequest.want.SetParam(Want::APP_INSTANCE_KEY, instanceKey);
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.uid = 1000;
    abilityRequest.appInfo.multiAppMode.multiAppModeType = AppExecFwk::MultiAppModeType::MULTI_INSTANCE;

    auto ret = AbilityPermissionUtil::GetInstance().NeedCheckStatusBar(abilityRecord, abilityRequest);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil NeedCheckStatusBar_0700 end");
}

/**
 * @tc.name: NeedCheckStatusBar_0800
 * @tc.desc: UIAbility true; uid true; isMultiInstance false; InstanceKey false;
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, NeedCheckStatusBar_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil NeedCheckStatusBar_0800 start");

    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>();
    abilityRecord->uid_ = 1000;
    abilityRecord->instanceKey = "instanceKey";

    AbilityRequest abilityRequest;
    std::string instanceKey = "instanceKey123";
    abilityRequest.want.SetParam(Want::APP_INSTANCE_KEY, instanceKey);
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.uid = 1000;
    abilityRequest.appInfo.multiAppMode.multiAppModeType = AppExecFwk::MultiAppModeType::APP_CLONE;

    auto ret = AbilityPermissionUtil::GetInstance().NeedCheckStatusBar(abilityRecord, abilityRequest);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil NeedCheckStatusBar_0800 end");
}

/**
 * @tc.name: NeedCheckStatusBar_0900
 * @tc.desc: abilityRecord nullptr
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, NeedCheckStatusBar_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil NeedCheckStatusBar_0900 start");

    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;

    AbilityRequest abilityRequest;
    std::string instanceKey = "instanceKey123";
    abilityRequest.want.SetParam(Want::APP_INSTANCE_KEY, instanceKey);
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.uid = 1000;
    abilityRequest.appInfo.multiAppMode.multiAppModeType = AppExecFwk::MultiAppModeType::APP_CLONE;

    auto ret = AbilityPermissionUtil::GetInstance().NeedCheckStatusBar(abilityRecord, abilityRequest);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil NeedCheckStatusBar_0900 end");
}

/**
 * @tc.name: CheckStartUIAbilityByUserLockStatus_0100
 * @tc.desc: bundleName is empty, should return true
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, CheckStartUIAbilityByUserLockStatus_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckStartUIAbilityByUserLockStatus_0100 start");
    std::string emptyBundleName = "";
    auto ret = AbilityPermissionUtil::GetInstance().CheckStartUIAbilityByUserLockStatus(emptyBundleName);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "CheckStartUIAbilityByUserLockStatus_0100 end");
}

/**
 * @tc.name: CheckStartUIAbilityByUserLockStatus_0200
 * @tc.desc: user unlocked, should return true for all apps
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, CheckStartUIAbilityByUserLockStatus_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckStartUIAbilityByUserLockStatus_0200 start");

    AbilityRuntime::UserController::GetInstance().SetMockUserLockStatus(
        AbilityRuntime::UserController::UserLockStatus::USER_UNLOCKED);
    std::string bundleName = "com.example.test";
    auto ret = AbilityPermissionUtil::GetInstance().CheckStartUIAbilityByUserLockStatus(bundleName);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "CheckStartUIAbilityByUserLockStatus_0200 end");
}

/**
 * @tc.name: CheckStartUIAbilityByUserLockStatus_0300
 * @tc.desc: user locked + system app, should return true
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, CheckStartUIAbilityByUserLockStatus_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckStartUIAbilityByUserLockStatus_0300 start");
    AbilityRuntime::UserController::GetInstance().SetMockUserLockStatus(
        AbilityRuntime::UserController::UserLockStatus::USER_LOCKED);
    BundleMgrHelper::retGetBundleInfo = true;
    BundleMgrHelper::retBundleInfo.applicationInfo.isSystemApp = true;
    BundleMgrHelper::retBundleInfo.applicationInfo.name = "systemApp";
    BundleMgrHelper::retBundleInfo.applicationInfo.bundleName = "com.example.system";
    std::string bundleName = "com.example.system";
    auto ret = AbilityPermissionUtil::GetInstance().CheckStartUIAbilityByUserLockStatus(bundleName);
    EXPECT_EQ(ret, true);
    BundleMgrHelper::retBundleInfo.applicationInfo.isSystemApp = false;
    TAG_LOGI(AAFwkTag::TEST, "CheckStartUIAbilityByUserLockStatus_0300 end");
}

/**
 * @tc.name: CheckStartUIAbilityByUserLockStatus_0400
 * @tc.desc: user locked + bms is null, should return true (fail open)
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(AbilityPermissionUtilTest, CheckStartUIAbilityByUserLockStatus_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckStartUIAbilityByUserLockStatus_0400 start");
    AbilityRuntime::UserController::GetInstance().SetMockUserLockStatus(
        AbilityRuntime::UserController::UserLockStatus::USER_LOCKED);
    BundleMgrHelper::isNullBundleMgrInstance = true;
    std::string bundleName = "com.example.test";
    auto ret = AbilityPermissionUtil::GetInstance().CheckStartUIAbilityByUserLockStatus(bundleName);
    EXPECT_EQ(ret, true);
    BundleMgrHelper::isNullBundleMgrInstance = false;
    TAG_LOGI(AAFwkTag::TEST, "CheckStartUIAbilityByUserLockStatus_0400 end");
}
}  // namespace AAFwk
}  // namespace OHOS
