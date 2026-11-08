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

#include "app_mgr_client.h"
#include "ability_manager_errors.h"
#include "configuration.h"
#include "hilog_tag_wrapper.h"
#include "mock_configuration_observer.h"
#include "mock_native_token.h"
#include "mock_sa_call.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int USER_ID = 100;
}  // namespace

class AppMgrClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppMgrClientTest::SetUpTestCase(void)
{
    MockNativeToken::SetNativeToken();
}

void AppMgrClientTest::TearDownTestCase(void)
{}

void AppMgrClientTest::SetUp()
{}

void AppMgrClientTest::TearDown()
{}

/**
 * @tc.name: AppMgrClient_GetProcessRunningInfosByUserId_0100
 * @tc.desc: GetProcessRunningInfosByUserId
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetProcessRunningInfosByUserId_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrClient_GetProcessRunningInfosByUserId_0100 start");

    AAFwk::IsMockSaCall::IsMockSpecificSystemAbilityAccessPermission();
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    std::vector<RunningProcessInfo> info;
    result = appMgrClient->GetProcessRunningInfosByUserId(info, USER_ID);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    TAG_LOGI(AAFwkTag::TEST, "info.size() = %{public}zu", info.size());

    TAG_LOGI(AAFwkTag::TEST, "AppMgrClient_GetProcessRunningInfosByUserId_0100 end");
}

/**
 * @tc.name: AppMgrClient_GetConfiguration_0100
 * @tc.desc: GetConfiguration
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetConfiguration_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrClient_GetConfiguration_0100 start");

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    Configuration config;
    result = appMgrClient->GetConfiguration(config);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrClient_GetConfiguration_0100 end");
}

/**
 * @tc.name: AppMgrClient_RegisterConfigurationObserver_0100
 * @tc.desc: RegisterConfigurationObserver
 * @tc.type: FUNC
 * @tc.require: SR000GVI12
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_RegisterConfigurationObserver_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrClient_RegisterConfigurationObserver_0100 start");

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    sptr<MockConfigurationObserver> observer(new (std::nothrow) MockConfigurationObserver());
    result = appMgrClient->RegisterConfigurationObserver(observer, -1);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    sleep(1);
    Configuration configuration;
    configuration.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "ZH-HANS");
    appMgrClient->UpdateConfiguration(configuration);

    sleep(1);
    result = appMgrClient->UnregisterConfigurationObserver(observer);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrClient_RegisterConfigurationObserver_0100 end");
}

/**
 * @tc.name: AppMgrClient_RegisterConfigurationObserver_0200
 * @tc.desc: RegisterConfigurationObserver
 * @tc.type: FUNC
 * @tc.require: SR000GVI12
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_RegisterConfigurationObserver_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrClient_RegisterConfigurationObserver_0200 start");

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    sptr<MockConfigurationObserver> observer(new (std::nothrow) MockConfigurationObserver());
    result = appMgrClient->RegisterConfigurationObserver(observer, -1);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    sptr<MockConfigurationObserver> observerOther(new (std::nothrow) MockConfigurationObserver());
    result = appMgrClient->RegisterConfigurationObserver(observerOther, -1);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    sleep(1);
    Configuration configuration;
    configuration.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "en_US");
    appMgrClient->UpdateConfiguration(configuration);

    sleep(1);
    result = appMgrClient->UnregisterConfigurationObserver(observer);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
    result = appMgrClient->UnregisterConfigurationObserver(observerOther);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrClient_RegisterConfigurationObserver_0200 end");
}

/**
 * @tc.name: AppMgrClient_UnregisterConfigurationObserver_0100
 * @tc.desc: UnregisterConfigurationObserver
 * @tc.type: FUNC
 * @tc.require: SR000GVI12
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_UnregisterConfigurationObserver_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrClient_UnregisterConfigurationObserver_0100 start");

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    sptr<MockConfigurationObserver> observer(new (std::nothrow) MockConfigurationObserver());
    result = appMgrClient->RegisterConfigurationObserver(observer);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    sleep(1);
    Configuration configuration;
    configuration.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "ZH-HANS");
    appMgrClient->UpdateConfiguration(configuration);

    sleep(1);
    result = appMgrClient->UnregisterConfigurationObserver(observer);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    sleep(1);
    Configuration configurationOther;
    configurationOther.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "en_US");
    appMgrClient->UpdateConfiguration(configurationOther);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrClient_UnregisterConfigurationObserver_0100 end");
}

/**
 * @tc.name: AppMgrClient_UnregisterConfigurationObserver_0200
 * @tc.desc: UnregisterConfigurationObserver
 * @tc.type: FUNC
 * @tc.require: SR000GVI12
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_UnregisterConfigurationObserver_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrClient_UnregisterConfigurationObserver_0200 start");

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    sptr<MockConfigurationObserver> observer(new (std::nothrow) MockConfigurationObserver());
    result = appMgrClient->RegisterConfigurationObserver(observer);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    sptr<MockConfigurationObserver> observerOther(new (std::nothrow) MockConfigurationObserver());
    result = appMgrClient->RegisterConfigurationObserver(observerOther);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    sleep(1);
    Configuration configuration;
    configuration.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "ZH-HANS");
    appMgrClient->UpdateConfiguration(configuration);

    sleep(1);
    result = appMgrClient->UnregisterConfigurationObserver(observer);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    sleep(1);
    Configuration configurationOther;
    configurationOther.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "en_US");
    appMgrClient->UpdateConfiguration(configurationOther);

    sleep(1);
    result = appMgrClient->UnregisterConfigurationObserver(observerOther);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrClient_UnregisterConfigurationObserver_0200 end");
}

/**
 * @tc.name: AppMgrClient_RegisterManagedProcess_0100
 * @tc.desc: The client forwards RegisterManagedProcess over the real service chain; the
 *           wrapper only admits the climgr process (aimgr uid), so any other system
 *           caller is rejected at the uid gate.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_RegisterManagedProcess_0100, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    auto appMgrClient = std::make_unique<AppMgrClient>();
    ASSERT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int64_t attributionId = 1;
    int32_t ret = appMgrClient->RegisterManagedProcess("client_register_managed_0100", 4321, 2, 3099,
        attributionId);
    // This process is not climgr (uid 3092): the wrapper gate rejects the full chain.
    EXPECT_EQ(ret, AAFwk::ERR_NOT_SYSTEM_APP);
    EXPECT_EQ(attributionId, 0);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrClient_RegisterManagedProcess_0100 end");
}

/**
 * @tc.name: AppMgrClient_UnregisterManagedProcess_0100
 * @tc.desc: The client forwards UnregisterManagedProcess over the real service chain;
 *           non-climgr callers are rejected at the uid gate.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_UnregisterManagedProcess_0100, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    auto appMgrClient = std::make_unique<AppMgrClient>();
    ASSERT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int32_t ret = appMgrClient->UnregisterManagedProcess("client_unregister_managed_0100", 77, 3099);
    EXPECT_EQ(ret, AAFwk::ERR_NOT_SYSTEM_APP);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrClient_UnregisterManagedProcess_0100 end");
}

/**
 * @tc.name: AppMgrClient_RegisterInAppSkillProcess_0100
 * @tc.desc: The client forwards RegisterInAppSkillProcess over the real service chain;
 *           non-climgr callers are rejected at the uid gate and resolvedPid stays
 *           invalid.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_RegisterInAppSkillProcess_0100, TestSize.Level1)
{
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    auto appMgrClient = std::make_unique<AppMgrClient>();
    ASSERT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int32_t resolvedPid = 0;
    int64_t attributionId = 1;
    int32_t ret = appMgrClient->RegisterInAppSkillProcess("client_in_app_skill_0100",
        "com.example.skill.host", 3099, resolvedPid, attributionId);
    EXPECT_EQ(ret, AAFwk::ERR_NOT_SYSTEM_APP);
    EXPECT_EQ(resolvedPid, -1);
    EXPECT_EQ(attributionId, 0);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrClient_RegisterInAppSkillProcess_0100 end");
}
}  // namespace AppExecFwk
}  // namespace OHOS
