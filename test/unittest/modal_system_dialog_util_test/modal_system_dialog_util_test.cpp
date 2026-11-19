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

#define private public
#include "ability_record.h"
#include "app_utils.h"
#include "modal_system_dialog_util.h"
#undef private
#include "hilog_tag_wrapper.h"
#include "mock_parameters.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class ModalSystemDialogUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ModalSystemDialogUtilTest::SetUpTestCase(void) {}
void ModalSystemDialogUtilTest::TearDownTestCase(void) {}
void ModalSystemDialogUtilTest::SetUp() {}
void ModalSystemDialogUtilTest::TearDown() {}

/**
 * @tc.name: CheckDebugAppNotInDeveloperMode_0100
 * @tc.desc: GetInstanceKey
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(ModalSystemDialogUtilTest, CheckDebugAppNotInDeveloperMode_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckDebugAppNotInDeveloperMode_0100 start");
    AppExecFwk::ApplicationInfo info;
    info.appProvisionType = "debug";
    bool ret = AbilityRuntime::ModalSystemDialogUtil::CheckDebugAppNotInDeveloperMode(info);
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "CheckDebugAppNotInDeveloperMode_0100 end");
}

/**
 * @tc.name: CheckDebugAppNotInDeveloperMode_0200
 * @tc.desc: GetInstanceKey
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(ModalSystemDialogUtilTest, CheckDebugAppNotInDeveloperMode_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckDebugAppNotInDeveloperMode_0200 start");
    AppExecFwk::ApplicationInfo info;
    info.appProvisionType = "debugggg";
    bool ret = AbilityRuntime::ModalSystemDialogUtil::CheckDebugAppNotInDeveloperMode(info);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "CheckDebugAppNotInDeveloperMode_0200 end");
}

/**
 * @tc.name: CheckDebugAppNotInDeveloperMode_0300
 * @tc.desc: GetInstanceKey
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(ModalSystemDialogUtilTest, CheckDebugAppNotInDeveloperMode_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckDebugAppNotInDeveloperMode_0300 start");
    AppExecFwk::ApplicationInfo info;
    info.appProvisionType = "debug";
    std::string key = "";
    system::SetBoolParameter(key, true);
    bool ret = AbilityRuntime::ModalSystemDialogUtil::CheckDebugAppNotInDeveloperMode(info);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "CheckDebugAppNotInDeveloperMode_0300 end");
}
}  // namespace AAFwk
}  // namespace OHOS
