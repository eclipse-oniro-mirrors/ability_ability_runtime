/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "ability_process.h"
#undef private
#include "mock_new_ability.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class AbilityProcessTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static std::shared_ptr<AbilityProcess> process_;
};
std::shared_ptr<AbilityProcess> AbilityProcessTest::process_ = nullptr;

void AbilityProcessTest::SetUpTestCase(void)
{
    process_ = AbilityProcess::GetInstance();
}

void AbilityProcessTest::TearDownTestCase(void)
{
}

void AbilityProcessTest::SetUp(void)
{
}

void AbilityProcessTest::TearDown(void)
{
}

/**
 * @tc.number: AaFwk_AbilityProcess_0100
 * @tc.name: GetInstance
 * @tc.desc: Successfully verified GetInstance.
 */
HWTEST_F(AbilityProcessTest, AaFwk_AbilityProcess_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_0100 start";
    auto process = AbilityProcess::GetInstance();
    EXPECT_TRUE(process != nullptr);
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityProcess_0200
 * @tc.name: StartAbility
 * @tc.desc: StartAbility fails when the ability is nullptr.
 */
HWTEST_F(AbilityProcessTest, AaFwk_AbilityProcess_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_0200 start";
    Ability *ability = nullptr;
    CallAbilityParam param;
    CallbackInfo callback;
    auto result = process_->StartAbility(ability, param, callback);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityProcess_0300
 * @tc.name: StartAbility
 * @tc.desc: Failed to verify StartAbility when forResultOption is true.
 */
HWTEST_F(AbilityProcessTest, AaFwk_AbilityProcess_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_0300 start";
    Ability *ability = new (std::nothrow) MockNewAbility();
    CallAbilityParam param;
    param.forResultOption = true;
    CallbackInfo callback;
    auto result = process_->StartAbility(ability, param, callback);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    delete ability;
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_0300 end";
}

/**
 * @tc.number: AaFwk_AbilityProcess_0400
 * @tc.name: StartAbility
 * @tc.desc: Failed to verify StartAbility when forResultOption is true.
 */
HWTEST_F(AbilityProcessTest, AaFwk_AbilityProcess_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_0400 start";
    Ability *ability = new (std::nothrow) MockNewAbility();
    CallAbilityParam param;
    param.forResultOption = true;
    param.setting = AbilityStartSetting::GetEmptySetting();
    CallbackInfo callback;
    auto result = process_->StartAbility(ability, param, callback);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    delete ability;
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_0400 end";
}

/**
 * @tc.number: AaFwk_AbilityProcess_0500
 * @tc.name: StartAbility
 * @tc.desc: Failed to verify StartAbility when forResultOption is false.
 */
HWTEST_F(AbilityProcessTest, AaFwk_AbilityProcess_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_0500 start";
    Ability *ability = new (std::nothrow) MockNewAbility();
    CallAbilityParam param;
    param.forResultOption = false;
    param.setting = AbilityStartSetting::GetEmptySetting();
    CallbackInfo callback;
    auto result = process_->StartAbility(ability, param, callback);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    delete ability;
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_0500 end";
}

/**
 * @tc.number: AaFwk_AbilityProcess_0600
 * @tc.name: StartAbility
 * @tc.desc: Failed to verify StartAbility when forResultOption is false.
 */
HWTEST_F(AbilityProcessTest, AaFwk_AbilityProcess_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_0600 start";
    Ability *ability = new (std::nothrow) MockNewAbility();
    CallAbilityParam param;
    param.forResultOption = false;
    CallbackInfo callback;
    auto result = process_->StartAbility(ability, param, callback);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    delete ability;
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_0600 end";
}

/**
 * @tc.number: AaFwk_AbilityProcess_0700
 * @tc.name: StartAbility
 * @tc.desc: Successful case of verifying StartAbility.
 */
HWTEST_F(AbilityProcessTest, AaFwk_AbilityProcess_0700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_0700 start";
    process_->abilityResultMap_.clear();
    Ability *ability = new (std::nothrow) MockNewAbility();
    CallAbilityParam param;
    param.forResultOption = true;
    param.requestCode = 1;
    CallbackInfo callback;
    std::map<int, CallbackInfo> callbackMap;
    callbackMap.emplace(param.requestCode, callback);
    process_->abilityResultMap_.emplace(ability, callbackMap);
    process_->StartAbility(ability, param, callback);
    EXPECT_EQ(static_cast<int32_t>(process_->abilityResultMap_.size()), 1);
    delete ability;
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_0700 end";
}

/**
 * @tc.number: AaFwk_AbilityProcess_0800
 * @tc.name: AddAbilityResultCallback
 * @tc.desc: Verify that AddAbilityResultCallback is successful_ Successfully added data.
 */
HWTEST_F(AbilityProcessTest, AaFwk_AbilityProcess_0800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_0800 start";
    process_->abilityResultMap_.clear();
    Ability *ability = new (std::nothrow) MockNewAbility();
    CallAbilityParam param;
    param.forResultOption = true;
    param.requestCode = 1;
    CallbackInfo callback;
    std::map<int, CallbackInfo> callbackMap;
    callbackMap.emplace(param.requestCode, callback);
    process_->abilityResultMap_.emplace(ability, callbackMap);
    process_->AddAbilityResultCallback(ability, param, 1, callback);
    EXPECT_EQ(static_cast<int32_t>(process_->abilityResultMap_.size()), 1);
    delete ability;
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_0800 end";
}

/**
 * @tc.number: AaFwk_AbilityProcess_0900
 * @tc.name: AddAbilityResultCallback
 * @tc.desc: Verify that AddAbilityResultCallback is successful_ Successfully added data.
 */
HWTEST_F(AbilityProcessTest, AaFwk_AbilityProcess_0900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_0900 start";
    process_->abilityResultMap_.clear();
    Ability *ability = new (std::nothrow) MockNewAbility();
    CallAbilityParam param;
    param.forResultOption = true;
    param.requestCode = 1;
    CallbackInfo callback;
    process_->AddAbilityResultCallback(ability, param, 1, callback);
    EXPECT_EQ(static_cast<int32_t>(process_->abilityResultMap_.size()), 1);

    Ability *abilityOne = new (std::nothrow) MockNewAbility();
    CallAbilityParam paramOne;
    paramOne.forResultOption = true;
    paramOne.requestCode = 2;
    CallbackInfo callbackOne;
    process_->AddAbilityResultCallback(abilityOne, paramOne, 2, callbackOne);
    EXPECT_EQ(static_cast<int32_t>(process_->abilityResultMap_.size()), 2);
    delete ability;
    delete abilityOne;
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_0900 end";
}

/**
 * @tc.number: AaFwk_AbilityProcess_1000
 * @tc.name: OnAbilityResult
 * @tc.desc: Failed to verify OnAbilityResult.
 */
HWTEST_F(AbilityProcessTest, AaFwk_AbilityProcess_1000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_1000 start";
    process_->abilityResultMap_.clear();
    Ability *ability = new (std::nothrow) MockNewAbility();
    CallAbilityParam param;
    param.forResultOption = true;
    param.requestCode = 1;
    CallbackInfo callback;
    std::map<int, CallbackInfo> callbackMap;
    callbackMap.emplace(param.requestCode, callback);
    Want resultData;
    process_->OnAbilityResult(ability, param.requestCode, 1, resultData);
    EXPECT_EQ(static_cast<int32_t>(process_->abilityResultMap_[0].size()), 0);
    delete ability;
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_1000 end";
}

/**
 * @tc.number: AaFwk_AbilityProcess_1100
 * @tc.name: OnAbilityResult
 * @tc.desc: Failed to verify OnAbilityResult.
 */
HWTEST_F(AbilityProcessTest, AaFwk_AbilityProcess_1100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_1100 start";
    process_->abilityResultMap_.clear();
    Ability *ability = new (std::nothrow) MockNewAbility();
    CallAbilityParam param;
    param.forResultOption = true;
    param.requestCode = 1;
    CallbackInfo callback;
    std::map<int, CallbackInfo> callbackMap;
    process_->abilityResultMap_.emplace(ability, callbackMap);
    Want resultData;
    process_->OnAbilityResult(ability, param.requestCode, 1, resultData);
    EXPECT_EQ(static_cast<int32_t>(process_->abilityResultMap_[0].size()), 0);
    delete ability;
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_1100 end";
}

/**
 * @tc.number: AaFwk_AbilityProcess_1300
 * @tc.name: RequestPermissionsFromUser
 * @tc.desc: Validate RequestPermissionsFromUser, unable to validate through assertion.
 */
HWTEST_F(AbilityProcessTest, AaFwk_AbilityProcess_1300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_1300 start";
    Ability *ability = nullptr;
    CallAbilityPermissionParam param;
    param.permission_list.emplace_back("permission");
    CallbackInfo callback;
    process_->RequestPermissionsFromUser(ability, param, callback);
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_1300 end";
}

/**
 * @tc.number: AaFwk_AbilityProcess_1400
 * @tc.name: RequestPermissionsFromUser
 * @tc.desc: Validate RequestPermissionsFromUser, unable to validate through assertion.
 */
HWTEST_F(AbilityProcessTest, AaFwk_AbilityProcess_1400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_1400 start";
    Ability *ability = new (std::nothrow) MockNewAbility();
    CallAbilityPermissionParam param;
    param.permission_list.emplace_back("permission");
    CallbackInfo callback;
    process_->RequestPermissionsFromUser(ability, param, callback);
    delete ability;
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_1400 end";
}

/**
 * @tc.number: AaFwk_AbilityProcess_1500
 * @tc.name: RequestPermissionsFromUser
 * @tc.desc: Validate RequestPermissionsFromUser, unable to validate through assertion.
 */
HWTEST_F(AbilityProcessTest, AaFwk_AbilityProcess_1500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_1500 start";
    Ability *ability = new (std::nothrow) MockNewAbility();
    CallAbilityPermissionParam param;
    CallbackInfo callback;
    process_->RequestPermissionsFromUser(ability, param, callback);
    delete ability;
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_1500 end";
}

/**
 * @tc.number: AaFwk_AbilityProcess_1600
 * @tc.name: CallFunc
 * @tc.desc: Successful case of verifying CallFunc.
 */
HWTEST_F(AbilityProcessTest, AaFwk_AbilityProcess_1600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_1600 start";
    int32_t requestCode = 1;
    std::vector<std::string> permissions;
    std::vector<int> permissionsState;
    CallbackInfo callback;
    auto result = process_->CallFunc(requestCode, permissions, permissionsState, callback);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_1600 end";
}

/**
 * @tc.number: AaFwk_AbilityProcess_StartAbility_ForResultCbReg_0100
 * @tc.name: StartAbility with forResultOption registers callback in abilityResultMap_
 * @tc.desc: Test that StartAbility registers callback in abilityResultMap_ after StartAbilityForResult
 *           call returns. 调用者 feature_ability.cpp:312 已通过 AddAbilityResultCallback
 *           先行注册，此测试验证 StartAbility 自身的注册逻辑。
 */
HWTEST_F(AbilityProcessTest, AaFwk_AbilityProcess_StartAbility_ForResultCbReg_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_StartAbility_ForResultCbReg_0100 start";
    process_->abilityResultMap_.clear();
    Ability *ability = new (std::nothrow) MockNewAbility();
    ASSERT_NE(ability, nullptr);

    CallAbilityParam param;
    param.forResultOption = true;
    param.requestCode = 100;
    param.setting = nullptr;
    CallbackInfo callback;
    callback.errCode = 0;

    auto result = process_->StartAbility(ability, param, callback);
    (void)result;

    EXPECT_EQ(static_cast<int32_t>(process_->abilityResultMap_.size()), 1);
    auto it = process_->abilityResultMap_.find(ability);
    ASSERT_NE(it, process_->abilityResultMap_.end());
    auto cbIt = it->second.find(param.requestCode);
    EXPECT_NE(cbIt, it->second.end());

    delete ability;
    GTEST_LOG_(INFO) << "AaFwk_AbilityProcess_StartAbility_ForResultCbReg_0100 end";
}
} // namespace AppExecFwk
} // namespace OHOS
