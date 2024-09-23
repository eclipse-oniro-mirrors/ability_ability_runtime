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

#define private public
#include "ability_record.h"
#include "utils/ability_permission_util.h"
#include "app_utils.h"
#undef private
#include "hilog_tag_wrapper.h"
#include "parameters.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class AbilityPermissionUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityPermissionUtilTest::SetUpTestCase(void) {}
void AbilityPermissionUtilTest::TearDownTestCase(void) {}
void AbilityPermissionUtilTest::SetUp() {}
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
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceAndAppClone(want, 100, 0, nullptr);

    bool isSupportMultiInstance = AppUtils::GetInstance().IsSupportMultiInstance();
    std::string deviceType = OHOS::system::GetDeviceType();
    TAG_LOGI(AAFwkTag::TEST, "current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "2in1") {
        EXPECT_EQ(result, RESOLVE_APP_ERR);
    } else {
        EXPECT_EQ(result, ERR_OK);
    }

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
    Want want;
    std::string instanceKey = "app_instance_0";
    want.SetParam(Want::APP_INSTANCE_KEY, instanceKey);
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceAndAppClone(want, 100, 0, nullptr);

    bool isSupportMultiInstance = AppUtils::GetInstance().IsSupportMultiInstance();
    std::string deviceType = OHOS::system::GetDeviceType();
    TAG_LOGI(AAFwkTag::TEST, "current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "2in1") {
        EXPECT_EQ(result, RESOLVE_APP_ERR);
    } else {
        EXPECT_EQ(result, ERR_MULTI_INSTANCE_NOT_SUPPORTED);
    }

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
    Want want;
    want.SetParam(Want::CREATE_APP_INSTANCE_KEY, true);
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceAndAppClone(want, 100, 0, nullptr);

    bool isSupportMultiInstance = AppUtils::GetInstance().IsSupportMultiInstance();
    std::string deviceType = OHOS::system::GetDeviceType();
    TAG_LOGI(AAFwkTag::TEST, "current deviceType is %{public}s", deviceType.c_str());
    if (deviceType == "2in1") {
        EXPECT_EQ(result, RESOLVE_APP_ERR);
    } else {
        EXPECT_EQ(result, ERR_MULTI_INSTANCE_NOT_SUPPORTED);
    }

    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceAndAppClone_0300 end");
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

    Want want;
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstance(want, nullptr, true, "", 0);
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

    std::string bundleName = "com.ohos.test";
    Want want;
    want.SetBundle(bundleName);
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = bundleName;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto isInitial = abilityRecord->Init();
    EXPECT_TRUE(isInitial);
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstance(want, abilityRecord->GetToken(), true, "", 1);
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

    std::string bundleName = "com.ohos.test";
    Want want;
    want.SetBundle(bundleName);
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = bundleName;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto isInitial = abilityRecord->Init();
    EXPECT_TRUE(isInitial);
    std::string instanceKey = "app_instance_0";
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstance(want, abilityRecord->GetToken(), true,
        instanceKey, 1);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
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

    std::string bundleName = "com.ohos.test";
    Want want;
    want.SetBundle(bundleName);
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = bundleName;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    auto isInitial = abilityRecord->Init();
    EXPECT_TRUE(isInitial);
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstance(want, abilityRecord->GetToken(), true, "", 0);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstance_0400 end");
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

    AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(Want::APP_INSTANCE_KEY, std::string("app_instance_0"));
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

    AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(Want::APP_INSTANCE_KEY, std::string("app_instance_0"));
    (void)AppUtils::GetInstance().IsSupportMultiInstance();
    AppUtils::GetInstance().isSupportMultiInstance_.value = true;
    auto result = AbilityPermissionUtil::GetInstance().CheckMultiInstanceKeyForExtension(abilityRequest);
    EXPECT_EQ(result, ERR_MULTI_INSTANCE_NOT_SUPPORTED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtil_CheckMultiInstanceKeyForExtension_0300 end");
}
}  // namespace AAFwk
}  // namespace OHOS
