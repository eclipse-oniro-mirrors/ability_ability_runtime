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
#include "bundlemgr/mock_bundle_manager.h"
#include "mock_ability_connect_callback.h"
#include "ability_manager_errors.h"
#include "ability_scheduler.h"
#define private public
#define protected public
#include "ability_event_handler.h"
#include "ability_manager_service.h"
#undef private
#undef protected
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#define private public
#define protected public
#include "prepare_terminate_callback_proxy.h"
#include "pending_want_manager.h"
#undef private
#undef protected
#include "sa_mgr_client.h"
#include "sender_info.h"
#include "system_ability_definition.h"
#include "wants_info.h"
#include "want_receiver_stub.h"
#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::ElementName;

namespace OHOS {
namespace AAFwk {
class PrepareTerminateCallbackProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    sptr<Token> MockToken();
};

void PrepareTerminateCallbackProxyTest::SetUpTestCase(void)
{}

void PrepareTerminateCallbackProxyTest::TearDownTestCase(void)
{}

void PrepareTerminateCallbackProxyTest::SetUp()
{}

void PrepareTerminateCallbackProxyTest::TearDown()
{}

sptr<Token> PrepareTerminateCallbackProxyTest::MockToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (!abilityRecord) {
        return nullptr;
    }
 
    return abilityRecord->GetToken();
}

/**
 * @tc.name: DoPrepareTerminate_0100
 * @tc.desc: Test the state of DoPrepareTerminate
 * @tc.type: FUNC
 */
HWTEST_F(PrepareTerminateCallbackProxyTest, DoPrepareTerminate_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoPrepareTerminate_0100 called start.");
    sptr<IRemoteObject> impl;
    auto Info = std::make_shared<PrepareTerminateCallbackProxy>(impl);
    Info->DoPrepareTerminate();
    EXPECT_EQ(impl, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "DoPrepareTerminate_0100 called end.");
}

/**
 * @tc.name: DoPrepareTerminate_0200
 * @tc.desc: Test the state of DoPrepareTerminate
 * @tc.type: FUNC
 */
HWTEST_F(PrepareTerminateCallbackProxyTest, DoPrepareTerminate_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DoPrepareTerminate_0200 called start.");
    sptr<Token> remoteObject = MockToken();
    sptr<IRemoteObject> impl(remoteObject);
    auto Info = std::make_shared<PrepareTerminateCallbackProxy>(impl);
    Info->DoPrepareTerminate();
    EXPECT_NE(impl, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "DoPrepareTerminate_0200 called end.");
}
}  // namespace AAFwk
}  // namespace OHOS
