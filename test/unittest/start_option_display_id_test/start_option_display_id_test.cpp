/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "ability_manager_service.h"
#include "mission_list_manager.h"
#undef private
#undef protected

#include "app_process_data.h"
#include "system_ability_definition.h"
#include "ability_manager_errors.h"
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AAFwk {
namespace {
const int32_t USER_ID_U100 = 100;
constexpr int32_t DISPLAY_ID = 256;
}  // namespace

class StartOptionDisplayIdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void StartOptionDisplayIdTest::SetUpTestCase() { }

void StartOptionDisplayIdTest::TearDownTestCase() { }

void StartOptionDisplayIdTest::SetUp() { }

void StartOptionDisplayIdTest::TearDown() { }

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify that the diplayId is correctly parsed test.
 */
HWTEST_F(StartOptionDisplayIdTest, start_option_display_id_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ElementName element("device", "com.ix.hiMusic", "MusicAbility");
    want.SetElement(element);
    StartOptions option;
    option.SetDisplayID(DISPLAY_ID);
    auto result = abilityMs_->StartAbility(want, option, nullptr);
    if (result == OHOS::ERR_OK) {
        auto topAbility = reinterpret_cast<MissionListManager*>(abilityMs_->
            GetMissionListManagerByUserId(USER_ID_U100).get())->GetCurrentTopAbilityLocked();
        if (topAbility) {
            auto defualtDisplayId = 0;
            auto displayId = topAbility->GetWant().GetIntParam(Want::PARAM_RESV_DISPLAY_ID, defualtDisplayId);
            EXPECT_EQ(displayId, DISPLAY_ID);
        }
    }
}

/*
 * Feature: AbilityManagerService
 * Function: StartAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StartAbility
 * EnvConditions: NA
 * CaseDescription: Verify that displayId does not support service startup.
 */
HWTEST_F(StartOptionDisplayIdTest, start_option_display_id_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    Want want;
    ElementName element("device", "com.ix.musicService", "MusicService");
    want.SetElement(element);
    StartOptions option;
    option.SetDisplayID(DISPLAY_ID);
    auto result = abilityMs_->StartAbility(want, option, nullptr);
    EXPECT_EQ(RESOLVE_ABILITY_ERR, result);
}
}  // namespace AAFwk
}  // namespace OHOS
