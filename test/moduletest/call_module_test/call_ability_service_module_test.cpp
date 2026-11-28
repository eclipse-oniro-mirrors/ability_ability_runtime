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
#define protected public
#include "ability_manager_service.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class AbilityManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
public:
    AbilityRequest abilityRequest_;
    std::shared_ptr<AbilityRecord> abilityRecord_{ nullptr };
};

void AbilityManagerServiceTest::SetUpTestCase() {}

void AbilityManagerServiceTest::TearDownTestCase() {}

void AbilityManagerServiceTest::SetUp() {}

void AbilityManagerServiceTest::TearDown() {}

/**
 * @tc.name: CheckStartByCallPermission_001
 * @tc.desc: Verify function CheckStartByCallPermission return RESOLVE_CALL_ABILITY_TYPE_ERR
 *           when ability type is not PAGE
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityManagerServiceTest, CheckStartByCallPermission_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    if (abilityRecord_ == nullptr) {
        abilityRequest_.appInfo.bundleName = "data.client.bundle";
        abilityRequest_.abilityInfo.name = "ClientAbility";
        abilityRequest_.abilityInfo.type = AbilityType::DATA;
        abilityRecord_ = AbilityRecord::CreateAbilityRecord(abilityRequest_);
    }
    abilityRequest_.callerUid = 0;
    EXPECT_EQ(RESOLVE_CALL_ABILITY_TYPE_ERR, abilityMs_->CheckStartByCallPermission(abilityRequest_));
}

/**
 * @tc.name: CheckStartByCallPermission_002
 * @tc.desc: Verify function CheckStartByCallPermission return RESOLVE_CALL_NO_PERMISSIONS
 *           when caller has no start-by-call permission
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityManagerServiceTest, CheckStartByCallPermission_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    if (abilityRecord_ == nullptr) {
        abilityRequest_.appInfo.bundleName = "data.client.bundle";
        abilityRequest_.abilityInfo.name = "ClientAbility";
        abilityRequest_.abilityInfo.type = AbilityType::DATA;
        abilityRecord_ = AbilityRecord::CreateAbilityRecord(abilityRequest_);
    }
    abilityRequest_.callerUid = 1000;
    abilityRequest_.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest_.abilityInfo.launchMode = AppExecFwk::LaunchMode::SINGLETON;
    EXPECT_EQ(RESOLVE_CALL_NO_PERMISSIONS, abilityMs_->CheckStartByCallPermission(abilityRequest_));
}
}
}
