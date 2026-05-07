/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
#include "ability.h"
#include "ability_local_record.h"
#include "ability_record_mgr.h"
#include "application_context.h"
#include "application_impl.h"
#include "application_info.h"
#include "context_deal.h"
#include "context_impl.h"
#include "fa_ability_thread.h"
#include "hilog_tag_wrapper.h"
#include "mock_i_remote_object.h"
#include "mock_runtime.h"
#include "ohos_application.h"
#include "pac_map.h"
#include "resource_manager.h"
#include "runtime.h"
#include "ui_ability.h"
#ifdef SUPPORT_SCREEN
#include "ui_ability_thread.h"
#endif
#undef private
#include "mock_ability_stage.h"
#include "mock_js_runtime.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
class PrepareTerminateCountStage : public AbilityRuntime::AbilityStage {
public:
    explicit PrepareTerminateCountStage(bool returnValue) : returnValue_(returnValue) {}

    bool OnPrepareTerminate(
        AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult> *,
        bool &isAsync) const override
    {
        callCount_++;
        isAsync = false;
        return returnValue_;
    }

    mutable int32_t callCount_ = 0;
    bool returnValue_ = false;
};

class ConfigUpdateCountStage : public AbilityRuntime::AbilityStage {
public:
    void OnConfigurationUpdated(const AppExecFwk::Configuration &) override
    {
        ++updateCount_;
    }

    int32_t updateCount_ = 0;
};
}

class OHOSApplicationTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<OHOSApplication> ohosApplication_;
};

void OHOSApplicationTest::SetUpTestCase()
{}

void OHOSApplicationTest::TearDownTestCase()
{}

void OHOSApplicationTest::SetUp()
{
    ohosApplication_ = std::make_shared<OHOSApplication>();
}

void OHOSApplicationTest::TearDown()
{
    ohosApplication_ = nullptr;
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnForeground_0100
* @tc.name: OnForeground
* @tc.desc: Verify function OnForeground pointer runtime_  empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnForeground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnForeground_0100 start.";
    ohosApplication_->OnForeground();
    EXPECT_TRUE(ohosApplication_->runtime_ == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnForeground_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnForeground_0200
* @tc.name: OnForeground
* @tc.desc: Verify function OnForeground pointer runtime_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnForeground_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnForeground_0200 start.";
    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockRuntime>();
    ohosApplication_->OnForeground();
    EXPECT_TRUE(ohosApplication_->runtime_ != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnForeground_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnBackground_0100
* @tc.name: OnBackground
* @tc.desc: Verify function OnBackground pointer runtime_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnBackground_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnBackground_0100 start.";
    ohosApplication_->OnBackground();
    EXPECT_TRUE(ohosApplication_->runtime_ == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnBackground_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnBackground_0200
* @tc.name: OnBackground
* @tc.desc: Verify function OnBackground pointer runtime_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnBackground_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnBackground_0200 start.";
    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockRuntime>();
    ohosApplication_->OnBackground();
    EXPECT_TRUE(ohosApplication_->runtime_ != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnBackground_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_DumpApplication_0100
* @tc.name: DumpApplication
* @tc.desc: Verify function DumpApplication pointer record not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_DumpApplication_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_DumpApplication_0100 start.";
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = nullptr;
    auto record = std::make_shared<AbilityLocalRecord>(info, token, nullptr, 0);
    ohosApplication_->abilityRecordMgr_->abilityRecords_.emplace(token, record);
    ohosApplication_->DumpApplication();
    EXPECT_TRUE(record != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_DumpApplication_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_DumpApplication_0200
* @tc.name: DumpApplication
* @tc.desc: Verify function DumpApplication pointer abilityInfo not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_DumpApplication_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_DumpApplication_0200 start.";
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    auto record =  std::make_shared<AbilityLocalRecord>(info, token, nullptr, 0);
    info->permissions.push_back(std::string("abc"));
    ohosApplication_->abilityRecordMgr_->abilityRecords_.emplace(token, record);
    ohosApplication_->DumpApplication();
    EXPECT_TRUE(record != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_DumpApplication_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_DumpApplication_0300
* @tc.name: DumpApplication
* @tc.desc: Verify function DumpApplication pointer applicationInfoPtr not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_DumpApplication_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_DumpApplication_0300 start.";
    ohosApplication_->DumpApplication();
    auto contextDeal = std::make_shared<ContextDeal>();
    auto appInfo = std::make_shared<ApplicationInfo>();
    contextDeal->SetApplicationInfo(appInfo);
    ohosApplication_->AttachBaseContext(contextDeal);
    ohosApplication_->DumpApplication();
    EXPECT_TRUE(ohosApplication_->GetApplicationInfo() != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_DumpApplication_0300 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_SetRuntime_0100
* @tc.name: SetRuntime
* @tc.desc: Verify function SetRuntime pointer runtime empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SetRuntime_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetRuntime_0100 start.";
    std::unique_ptr<AbilityRuntime::Runtime> runtime = nullptr;
    ohosApplication_->SetRuntime(std::move(runtime));
    EXPECT_TRUE(runtime == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetRuntime_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_SetRuntime_0200
* @tc.name: SetRuntime
* @tc.desc: Verify function SetRuntime pointer runtime_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SetRuntime_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetRuntime_0200 start.";
    std::unique_ptr<AbilityRuntime::Runtime> runtime = std::make_unique<AbilityRuntime::MockRuntime>();
    EXPECT_TRUE(runtime != nullptr);
    ohosApplication_->SetRuntime(std::move(runtime));
    EXPECT_TRUE(ohosApplication_->runtime_ != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetRuntime_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_GetSpecifiedRuntime_0100
* @tc.name: GetSpecifiedRuntime
* @tc.desc: Verify function GetSpecifiedRuntime pointer runtime empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_GetSpecifiedRuntime_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetSpecifiedRuntime_0100 start.";
    std::unique_ptr<AbilityRuntime::Runtime> runtime = nullptr;
    ohosApplication_->SetRuntime(std::move(runtime));
    auto &runtimeSpec = ohosApplication_->GetSpecifiedRuntime(OHOS::AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2);
    EXPECT_TRUE(runtimeSpec == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetSpecifiedRuntime_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_GetSpecifiedRuntime_0200
* @tc.name: GetSpecifiedRuntime
* @tc.desc: Verify function GetSpecifiedRuntime pointer runtime not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_GetSpecifiedRuntime_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetSpecifiedRuntime_0200 start.";
    std::unique_ptr<AbilityRuntime::Runtime> runtime = std::make_unique<AbilityRuntime::MockRuntime>();
    EXPECT_TRUE(runtime != nullptr);
    ohosApplication_->SetRuntime(std::move(runtime));
    auto &runtimeSpec = ohosApplication_->GetSpecifiedRuntime(OHOS::AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2);
    EXPECT_TRUE(runtimeSpec != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetSpecifiedRuntime_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_SetApplicationContext_0100
* @tc.name: SetApplicationContext
* @tc.desc: Verify function SetApplicationContext pointer abilityRuntimeContext_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SetApplicationContext_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetApplicationContext_0100 start.";
    std::shared_ptr<AbilityRuntime::ApplicationContext> abilityRuntimeContext = nullptr;
    ohosApplication_->SetApplicationContext(abilityRuntimeContext);
    EXPECT_TRUE(ohosApplication_->abilityRuntimeContext_ == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetApplicationContext_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_SetApplicationContext_0200
* @tc.name: SetApplicationContext
* @tc.desc: Verify function SetApplicationContext pointer abilityRuntimeContext_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SetApplicationContext_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetApplicationContext_0200 start.";
    std::shared_ptr<AbilityRuntime::ApplicationContext> abilityRuntimeContext =
        std::make_shared<AbilityRuntime::ApplicationContext>();
    EXPECT_TRUE(abilityRuntimeContext != nullptr);
    ohosApplication_->SetApplicationContext(abilityRuntimeContext);
    EXPECT_TRUE(ohosApplication_->abilityRuntimeContext_ != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetApplicationContext_0200 end.";
}

/*
 * @tc.number: AppExecFwk_OHOSApplicationTest_SetApplicationContext_0300
 * @tc.name: SetApplicationContext
 * @tc.desc: Verify function SetApplicationContext RegisterAppGetSpecifiedRuntime
 */
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SetApplicationContext_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetApplicationContext_0300 start.";
    std::shared_ptr<AbilityRuntime::ApplicationContext> abilityRuntimeContext =
        std::make_shared<AbilityRuntime::ApplicationContext>();
    EXPECT_TRUE(abilityRuntimeContext != nullptr);
    ohosApplication_->SetApplicationContext(abilityRuntimeContext);
    EXPECT_TRUE(ohosApplication_->abilityRuntimeContext_ != nullptr);
    ASSERT_NE(abilityRuntimeContext->appGetSpecifiedRuntimeCallback_, nullptr);

    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockRuntime>();
    const auto &runtime = abilityRuntimeContext->appGetSpecifiedRuntimeCallback_("");
    EXPECT_EQ(ohosApplication_->runtime_.get(), runtime.get());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetApplicationContext_0300 end.";
}

/*
 * @tc.number: AppExecFwk_OHOSApplicationTest_SetApplicationContext_0400
 * @tc.name: SetApplicationContext
 * @tc.desc: Verify function SetApplicationContext RegisterAppGetSpecifiedRuntime null applicationSptr
 */
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SetApplicationContext_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetApplicationContext_0400 start.";
    std::shared_ptr<AbilityRuntime::ApplicationContext> abilityRuntimeContext =
        std::make_shared<AbilityRuntime::ApplicationContext>();
    EXPECT_TRUE(abilityRuntimeContext != nullptr);
    ohosApplication_->SetApplicationContext(abilityRuntimeContext);
    EXPECT_TRUE(ohosApplication_->abilityRuntimeContext_ != nullptr);
    ASSERT_NE(abilityRuntimeContext->appGetSpecifiedRuntimeCallback_, nullptr);

    ohosApplication_ = nullptr;
    const auto &runtime = abilityRuntimeContext->appGetSpecifiedRuntimeCallback_("");
    EXPECT_EQ(runtime, nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetApplicationContext_0400 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_SetAbilityRecordMgr_0100
* @tc.name: SetAbilityRecordMgr
* @tc.desc: Verify function SetAbilityRecordMgr pointer abilityRecordMgr_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SetAbilityRecordMgr_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetAbilityRecordMgr_0100 start.";
    std::shared_ptr<AbilityRecordMgr> abilityRecordMgr = nullptr;
    ohosApplication_->SetAbilityRecordMgr(abilityRecordMgr);
    EXPECT_TRUE(ohosApplication_->abilityRecordMgr_ == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetAbilityRecordMgr_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_SetAbilityRecordMgr_0200
* @tc.name: SetAbilityRecordMgr
* @tc.desc: Verify function SetAbilityRecordMgr pointer abilityRecordMgr_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SetAbilityRecordMgr_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetAbilityRecordMgr_0200 start.";
    std::shared_ptr<AbilityRecordMgr> abilityRecordMgr = std::make_shared<AbilityRecordMgr>();
    EXPECT_TRUE(abilityRecordMgr != nullptr);
    ohosApplication_->SetAbilityRecordMgr(abilityRecordMgr);
    EXPECT_TRUE(ohosApplication_->abilityRecordMgr_ != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetAbilityRecordMgr_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0100
* @tc.name: OnConfigurationUpdated
* @tc.desc: Verify function OnConfigurationUpdated pointer abilityRecordMgr_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0100 start.";
    Configuration config;
    ohosApplication_->OnConfigurationUpdated(config);
    EXPECT_TRUE(ohosApplication_->abilityRecordMgr_ == nullptr);
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    ohosApplication_->OnConfigurationUpdated(config);
    EXPECT_TRUE(ohosApplication_->configuration_ == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0200
* @tc.name: OnConfigurationUpdated
* @tc.desc: Verify function OnConfigurationUpdated pointer abilityRecord not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0200 start.";
    Configuration config;
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    ohosApplication_->configuration_ = std::make_shared<Configuration>();
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info =  nullptr;
    auto abilityRecord =  std::make_shared<AbilityLocalRecord>(info, token, nullptr, 0);
    ohosApplication_->abilityRecordMgr_->abilityRecords_.emplace(token, abilityRecord);
    sptr<AbilityThread> abilityThread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    abilityRecord->SetAbilityThread(abilityThread);
    ohosApplication_->OnConfigurationUpdated(config);
    EXPECT_TRUE(!ohosApplication_->abilityRecordMgr_->abilityRecords_.empty());
    EXPECT_TRUE(abilityRecord != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0300
* @tc.name: OnConfigurationUpdated
* @tc.desc: Verify function OnConfigurationUpdated map abilityStages_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0300 start.";
    Configuration config;
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    ohosApplication_->configuration_ = std::make_shared<Configuration>();
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    std::string moduleName = "entry";
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStages = std::make_shared<AbilityRuntime::AbilityStage>();
    ohosApplication_->abilityStages_.emplace(moduleName, abilityStages);
    ohosApplication_->OnConfigurationUpdated(config);
    EXPECT_TRUE(!ohosApplication_->abilityStages_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0300 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0310
* @tc.name: OnConfigurationUpdated
* @tc.desc: Verify calls abilityStage OnConfigurationUpdated when skipAbilityStageLifecycle is false
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0310, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0310 start.";
    std::string bundleName = "test.bundleName";
    std::string moduleName = "test.moduleName";
    std::string hapPath = "/data/app/testHap";
    std::vector<std::string> overlayPaths;
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    ASSERT_NE(resConfig, nullptr);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager(
        bundleName, moduleName, hapPath, overlayPaths, *resConfig));
    ASSERT_NE(resourceManager, nullptr);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    contextImpl->SetResourceManager(resourceManager);
    auto appContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    appContext->AttachContextImpl(contextImpl);
    ohosApplication_->SetApplicationContext(appContext);

    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    ohosApplication_->configuration_ = std::make_shared<Configuration>();
    auto stage = std::make_shared<ConfigUpdateCountStage>();
    stage->SetSkipAbilityStageLifecycle(false);
    ohosApplication_->abilityStages_.emplace("entry_config_false", stage);

    Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "zh");
    ohosApplication_->OnConfigurationUpdated(config);
    EXPECT_EQ(stage->updateCount_, 1);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0310 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0320
* @tc.name: OnConfigurationUpdated
* @tc.desc: Verify does not call abilityStage OnConfigurationUpdated when skipAbilityStageLifecycle is true
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0320, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0320 start.";
    std::string bundleName = "test.bundleName";
    std::string moduleName = "test.moduleName";
    std::string hapPath = "/data/app/testHap";
    std::vector<std::string> overlayPaths;
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    ASSERT_NE(resConfig, nullptr);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager(
        bundleName, moduleName, hapPath, overlayPaths, *resConfig));
    ASSERT_NE(resourceManager, nullptr);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    contextImpl->SetResourceManager(resourceManager);
    auto appContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    appContext->AttachContextImpl(contextImpl);
    ohosApplication_->SetApplicationContext(appContext);

    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    ohosApplication_->configuration_ = std::make_shared<Configuration>();
    auto stage = std::make_shared<ConfigUpdateCountStage>();
    stage->SetSkipAbilityStageLifecycle(true);
    ohosApplication_->abilityStages_.emplace("entry_config_true", stage);

    Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "zh");
    ohosApplication_->OnConfigurationUpdated(config);
    EXPECT_EQ(stage->updateCount_, 0);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0320 end.";
}

/*
* @tc.number: OnConfigurationUpdated_0600
* @tc.name: OnConfigurationUpdated
* @tc.desc: Function test abilityRuntimeContext_ not empty
*/
HWTEST_F(OHOSApplicationTest, OnConfigurationUpdated_0600, TestSize.Level1)
{
    std::string bundleName = "test.bundleName";
    std::string moduleName = "test.moduleName";
    std::string hapPath = "/data/app/testHap";
    std::vector<std::string> overlayPaths;
    std::unique_ptr<Global::Resource::ResConfig> resConfigBefore(Global::Resource::CreateResConfig());
    ASSERT_NE(resConfigBefore, nullptr);
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager(
        bundleName, moduleName, hapPath, overlayPaths, *resConfigBefore));
    ASSERT_NE(resourceManager, nullptr);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    contextImpl->SetResourceManager(resourceManager);

    auto appContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    appContext->AttachContextImpl(contextImpl);
    ohosApplication_->SetApplicationContext(appContext);
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    ohosApplication_->configuration_ = std::make_shared<Configuration>();

    Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "zh");
    ohosApplication_->OnConfigurationUpdated(config);
    std::unique_ptr<Global::Resource::ResConfig> resConfigAfter(Global::Resource::CreateResConfig());
    ASSERT_NE(resConfigAfter, nullptr);
    resourceManager->GetResConfig(*resConfigAfter);
    const icu::Locale *localeInfo = resConfigAfter->GetLocaleInfo();
    ASSERT_NE(localeInfo, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "Update config language %{public}s succeed.", localeInfo->getLanguage());
    EXPECT_EQ(strcmp(localeInfo->getLanguage(), "zh"), 0);
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0100
* @tc.name: OnMemoryLevel
* @tc.desc: Verify function OnMemoryLevel pointer abilityRecord not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0100 start.";
    constexpr int32_t level = 1;
    ohosApplication_->OnMemoryLevel(level);
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    std::shared_ptr<AbilityInfo> info = nullptr;
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(info, token, nullptr, 0);
    EXPECT_TRUE(abilityRecord != nullptr);
    ohosApplication_->abilityRecordMgr_->abilityRecords_.emplace(token, abilityRecord);
    sptr<AbilityThread> abilityThread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    abilityRecord->SetAbilityThread(abilityThread);
    ohosApplication_->OnMemoryLevel(level);
    EXPECT_FALSE(ohosApplication_->abilityRecordMgr_->abilityRecords_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0200
* @tc.name: OnMemoryLevel
* @tc.desc: Verify function OnMemoryLevel map abilityStages_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0200 start.";
    constexpr int32_t level = 1;
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    std::string moduleName1 = "entry1";
    std::string moduleName2 = "entry2";
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStages1 = std::make_shared<AbilityRuntime::AbilityStage>();
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStages2 = nullptr;
    ohosApplication_->abilityStages_.emplace(moduleName1, abilityStages1);
    ohosApplication_->abilityStages_.emplace(moduleName2, abilityStages2);
    ohosApplication_->OnMemoryLevel(level);
    EXPECT_TRUE(!ohosApplication_->abilityStages_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0210
* @tc.name: OnMemoryLevel
* @tc.desc: Verify calls abilityStage OnMemoryLevel when skipAbilityStageLifecycle is false
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0210, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0210 start.";
    constexpr int32_t level = 1;
    ohosApplication_->abilityRuntimeContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    auto mockAbilityStage = std::make_shared<AbilityRuntime::MockAbilityStage>();
    mockAbilityStage->SetSkipAbilityStageLifecycle(false);
    ohosApplication_->abilityStages_.emplace("entry_memory_false", mockAbilityStage);

    EXPECT_CALL(*mockAbilityStage, OnMemoryLevel(level)).Times(1);
    ohosApplication_->OnMemoryLevel(level);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0210 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0220
* @tc.name: OnMemoryLevel
* @tc.desc: Verify does not call abilityStage OnMemoryLevel when skipAbilityStageLifecycle is true
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0220, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0220 start.";
    constexpr int32_t level = 1;
    ohosApplication_->abilityRuntimeContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    auto mockAbilityStage = std::make_shared<AbilityRuntime::MockAbilityStage>();
    mockAbilityStage->SetSkipAbilityStageLifecycle(true);
    ohosApplication_->abilityStages_.emplace("entry_memory_true", mockAbilityStage);

    EXPECT_CALL(*mockAbilityStage, OnMemoryLevel(level)).Times(0);
    ohosApplication_->OnMemoryLevel(level);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0220 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_0100
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage pointer abilityRecord empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0100 start.";
    ohosApplication_->OnStart();
    std::shared_ptr<AbilityLocalRecord> abilityRecord = nullptr;
    auto callback = [](const std::shared_ptr<AbilityRuntime::Context> &) {};
    bool isAsyncCallback = false;
    ohosApplication_->AddAbilityStage(abilityRecord, callback, isAsyncCallback);
    EXPECT_TRUE(abilityRecord == nullptr);
    ohosApplication_->OnTerminate();
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_0200
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage pointer abilityInfo empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0200 start.";
    std::shared_ptr<AbilityLocalRecord> abilityRecord = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr;
    auto callback = [](const std::shared_ptr<AbilityRuntime::Context> &) {};
    bool isAsyncCallback = false;
    ohosApplication_->AddAbilityStage(abilityRecord, callback, isAsyncCallback);
    EXPECT_TRUE(abilityInfo == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_0300
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage pointer hapModuleInfo empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0300 start.";
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    auto want = std::make_shared<Want>();
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(info, token, want, 0);
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    auto callback = [](const std::shared_ptr<AbilityRuntime::Context> &) {};
    bool isAsyncCallback = false;
    ohosApplication_->AddAbilityStage(abilityRecord, callback, isAsyncCallback);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0300 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_0400
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage abilityRecord->GetWant() not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0400 start.";
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    info->applicationInfo.multiProjects = true;
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(info, token, nullptr, 0);
    auto callback = [](const std::shared_ptr<AbilityRuntime::Context> &) {};
    bool isAsyncCallback = false;
    ohosApplication_->AddAbilityStage(abilityRecord, callback, isAsyncCallback);
    EXPECT_FALSE(ohosApplication_->abilityStages_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0400 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_0500
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage pointer abilityStages not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0500 start.";
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = nullptr;
    std::string moduleName = "entry";
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(info, token, nullptr, 0);
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStages = std::make_shared<AbilityRuntime::AbilityStage>();
    ohosApplication_->abilityStages_.emplace(moduleName, abilityStages);
    auto callback = [](const std::shared_ptr<AbilityRuntime::Context> &) {};
    bool isAsyncCallback = false;
    ohosApplication_->AddAbilityStage(abilityRecord, callback, isAsyncCallback);
    EXPECT_FALSE(ohosApplication_->abilityStages_.empty());
    EXPECT_TRUE(abilityStages != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0500 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_0600
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage pointer token empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0600 start.";
    sptr<Notification::MockIRemoteObject> token;
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    info->moduleName = "entry";
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(info, token, nullptr, 0);
    auto callback = [](const std::shared_ptr<AbilityRuntime::Context> &) {};
    bool isAsyncCallback = false;
    ohosApplication_->AddAbilityStage(abilityRecord, callback, isAsyncCallback);
    ohosApplication_->AddAbilityStage(abilityRecord, callback, isAsyncCallback);
    EXPECT_TRUE(token == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0600 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_0700
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage pointer token not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0700 start.";
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(info, token, nullptr, 0);
    abilityRecord->token_ = new (std::nothrow) Notification::MockIRemoteObject();
    auto callback = [](const std::shared_ptr<AbilityRuntime::Context> &) {};
    bool isAsyncCallback = false;
    ohosApplication_->AddAbilityStage(abilityRecord, callback, isAsyncCallback);
    EXPECT_TRUE(token != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0700 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_0710
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage skipAbilityStageLifecycle false branch
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_0710, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0710 start.";
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    info->moduleName = "entry_skip_false";
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(info, token, nullptr, 0);
    abilityRecord->SetSkipAbilityStageLifecycle(false);

    auto callback = [](const std::shared_ptr<AbilityRuntime::Context> &) {};
    bool isAsyncCallback = false;
    auto context = ohosApplication_->AddAbilityStage(abilityRecord, callback, isAsyncCallback);

    EXPECT_NE(context, nullptr);
    auto it = ohosApplication_->abilityStages_.find(info->moduleName);
    ASSERT_NE(it, ohosApplication_->abilityStages_.end());
    ASSERT_NE(it->second, nullptr);
    EXPECT_FALSE(it->second->IsSkipAbilityStageLifecycle());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0710 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_0720
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage skipAbilityStageLifecycle true branch
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_0720, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0720 start.";
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    info->moduleName = "entry_skip_true";
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(info, token, nullptr, 0);
    abilityRecord->SetSkipAbilityStageLifecycle(true);

    auto callback = [](const std::shared_ptr<AbilityRuntime::Context> &) {};
    bool isAsyncCallback = false;
    auto context = ohosApplication_->AddAbilityStage(abilityRecord, callback, isAsyncCallback);

    EXPECT_NE(context, nullptr);
    auto it = ohosApplication_->abilityStages_.find(info->moduleName);
    ASSERT_NE(it, ohosApplication_->abilityStages_.end());
    ASSERT_NE(it->second, nullptr);
    EXPECT_TRUE(it->second->IsSkipAbilityStageLifecycle());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0720 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_0800
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage pointer abilityRuntimeContext_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0800 start.";
    HapModuleInfo hapModuleInfo;
    auto callback = []() {};
    bool isAsyncCallback = false;
    ohosApplication_->AddAbilityStage(hapModuleInfo, callback, isAsyncCallback);
    EXPECT_TRUE(ohosApplication_->abilityRuntimeContext_ == nullptr);
    EXPECT_FALSE(ohosApplication_->AddAbilityStage(hapModuleInfo, callback, isAsyncCallback));
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0800 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_0900
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage pointer runtime_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0900 start.";
    HapModuleInfo hapModuleInfo;
    auto callback = []() {};
    bool isAsyncCallback = false;
    ohosApplication_->abilityRuntimeContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    ohosApplication_->AddAbilityStage(hapModuleInfo, callback, isAsyncCallback);
    EXPECT_TRUE(ohosApplication_->runtime_ == nullptr);
    EXPECT_FALSE(ohosApplication_->AddAbilityStage(hapModuleInfo, callback, isAsyncCallback));
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_0900 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_01000
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage map abilityStages_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_01000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_01000 start.";
    HapModuleInfo hapModuleInfo;
    auto callback = []() {};
    bool isAsyncCallback = false;
    std::string moduleName = "entry";
    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockRuntime>();
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStages = std::make_shared<AbilityRuntime::AbilityStage>();
    ohosApplication_->abilityStages_.emplace(moduleName, abilityStages);
    ohosApplication_->abilityRuntimeContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    ohosApplication_->AddAbilityStage(hapModuleInfo, callback, isAsyncCallback);
    EXPECT_FALSE(ohosApplication_->abilityStages_.empty());
    EXPECT_FALSE(ohosApplication_->AddAbilityStage(hapModuleInfo, callback, isAsyncCallback));
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_01000 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_01100
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage variable moduleInfo empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_01100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_01100 start.";
    HapModuleInfo hapModuleInfo;
    auto callback = []() {};
    bool isAsyncCallback = false;
    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockRuntime>();
    ohosApplication_->abilityRuntimeContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    ohosApplication_->abilityRuntimeContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    ohosApplication_->AddAbilityStage(hapModuleInfo, callback, isAsyncCallback);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->multiProjects = true;
    contextImpl->SetApplicationInfo(appInfo);
    ohosApplication_->abilityRuntimeContext_->AttachContextImpl(contextImpl);
    ohosApplication_->AddAbilityStage(hapModuleInfo, callback, isAsyncCallback);
    EXPECT_FALSE(ohosApplication_->abilityStages_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_01100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_01200
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage abilityRuntimeContext_->GetApplicationInfo() true
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_01200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_01200 start.";
    HapModuleInfo hapModuleInfo;
    auto callback = []() {};
    bool isAsyncCallback = false;
    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockRuntime>();
    ohosApplication_->abilityRuntimeContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    ohosApplication_->abilityRuntimeContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    ohosApplication_->AddAbilityStage(hapModuleInfo, callback, isAsyncCallback);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->multiProjects = false;
    contextImpl->SetApplicationInfo(appInfo);
    ohosApplication_->abilityRuntimeContext_->AttachContextImpl(contextImpl);
    ohosApplication_->AddAbilityStage(hapModuleInfo, callback, isAsyncCallback);
    EXPECT_FALSE(ohosApplication_->abilityStages_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_01200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_AddAbilityStage_01300
* @tc.name: AddAbilityStage
* @tc.desc: Verify function AddAbilityStage sets process name correctly in AbilityStageContext
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_AddAbilityStage_01300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_01300 start.";
    HapModuleInfo hapModuleInfo;
    auto callback = []() {};
    bool isAsyncCallback = false;
    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockRuntime>();
    ohosApplication_->abilityRuntimeContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    std::string testProcessName = "com.example.test_process";
    auto processInfo = std::make_shared<ProcessInfo>(testProcessName, 1234);
    ohosApplication_->SetProcessInfo(processInfo);
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    ohosApplication_->abilityRuntimeContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    ohosApplication_->AddAbilityStage(hapModuleInfo, callback, isAsyncCallback);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->multiProjects = false;
    contextImpl->SetApplicationInfo(appInfo);
    ohosApplication_->abilityRuntimeContext_->AttachContextImpl(contextImpl);
    ohosApplication_->AddAbilityStage(hapModuleInfo, callback, isAsyncCallback);
    EXPECT_FALSE(ohosApplication_->abilityStages_.empty());
    EXPECT_EQ(ohosApplication_->GetProcessName(), testProcessName);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_AddAbilityStage_01300 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0100
* @tc.name: CleanAbilityStage
* @tc.desc: Verify function CleanAbilityStage pointer abilityInfo empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0100 start.";
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr;
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    ohosApplication_->CleanAbilityStage(token, abilityInfo, false);
    EXPECT_TRUE(abilityInfo == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0200
* @tc.name: CleanAbilityStage
* @tc.desc: Verify function CleanAbilityStage pointer token empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0200 start.";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<Notification::MockIRemoteObject> token = nullptr;
    ohosApplication_->CleanAbilityStage(token, abilityInfo, false);
    EXPECT_TRUE(token == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0300
* @tc.name: CleanAbilityStage
* @tc.desc: Verify function CleanAbilityStage map abilityRecords_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0300 start.";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    abilityInfo->moduleName = "entry";
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage = std::make_shared<AbilityRuntime::AbilityStage>();
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    ohosApplication_->abilityStages_.emplace(abilityInfo->moduleName, abilityStage);
    EXPECT_FALSE(ohosApplication_->abilityStages_.empty());
    ohosApplication_->CleanAbilityStage(token, abilityInfo, false);
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0300 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0400
* @tc.name: CleanAbilityStage
* @tc.desc: Verify function CleanAbilityStage calls OnDestroy when skipAbilityStageLifecycle is false
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0400 start.";
    std::string moduleName = "entry_clean_destroy_false";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->moduleName = moduleName;
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);

    auto mockAbilityStage = std::make_shared<AbilityRuntime::MockAbilityStage>();
    mockAbilityStage->SetSkipAbilityStageLifecycle(false);
    mockAbilityStage->AddAbility(token, abilityRecord);
    ohosApplication_->abilityStages_.emplace(moduleName, mockAbilityStage);

    EXPECT_CALL(*mockAbilityStage, OnDestroy()).Times(1);
    ohosApplication_->CleanAbilityStage(token, abilityInfo, false);
    EXPECT_EQ(ohosApplication_->abilityStages_.find(moduleName), ohosApplication_->abilityStages_.end());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0400 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0500
* @tc.name: CleanAbilityStage
* @tc.desc: Verify function CleanAbilityStage does not call OnDestroy when skipAbilityStageLifecycle is true
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0500 start.";
    std::string moduleName = "entry_clean_destroy_true";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->moduleName = moduleName;
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);

    auto mockAbilityStage = std::make_shared<AbilityRuntime::MockAbilityStage>();
    mockAbilityStage->SetSkipAbilityStageLifecycle(true);
    mockAbilityStage->AddAbility(token, abilityRecord);
    ohosApplication_->abilityStages_.emplace(moduleName, mockAbilityStage);

    EXPECT_CALL(*mockAbilityStage, OnDestroy()).Times(0);
    ohosApplication_->CleanAbilityStage(token, abilityInfo, false);
    EXPECT_EQ(ohosApplication_->abilityStages_.find(moduleName), ohosApplication_->abilityStages_.end());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_CleanAbilityStage_0500 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_GetAppContext_0100
* @tc.name: GetAppContext
* @tc.desc: Verify function GetAppContext pointer token empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_GetAppContext_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetAppContext_0100 start.";
    auto context = ohosApplication_->GetAppContext();
    EXPECT_TRUE(context == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetAppContext_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_GetRuntime_0100
* @tc.name: GetRuntime
* @tc.desc: Verify function GetRuntime pointer token empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_GetRuntime_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetRuntime_0100 start.";
    auto &runtime = ohosApplication_->GetRuntime();
    EXPECT_TRUE(runtime == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetRuntime_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_SetConfiguration_0100
* @tc.name: SetConfiguration
* @tc.desc: Verify function SetConfiguration pointer token empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SetConfiguration_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetConfiguration_0100 start.";
    Configuration config;
    ohosApplication_->configuration_ = nullptr;
    ohosApplication_->SetConfiguration(config);
    ohosApplication_->SetConfiguration(config);
    EXPECT_TRUE(ohosApplication_->configuration_ != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetConfiguration_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_ScheduleAcceptWant_0100
* @tc.name: ScheduleAcceptWant
* @tc.desc: Verify function ScheduleAcceptWant pointer abilityStage not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_ScheduleAcceptWant_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_ScheduleAcceptWant_0100 start.";
    Want want;
    bool isAsync = false;
    std::string moduleName = "entry";
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage = std::make_shared<AbilityRuntime::AbilityStage>();
    ohosApplication_->abilityStages_.emplace(moduleName, abilityStage);
    EXPECT_FALSE(ohosApplication_->abilityStages_.empty());
    auto callback = [](std::string) {};
    ohosApplication_->ScheduleAcceptWant(want, moduleName, callback, isAsync);
    EXPECT_TRUE(abilityStage != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_ScheduleAcceptWant_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_SchedulePrepareTerminate_0100
* @tc.name: SchedulePrepareTerminate
* @tc.desc: Verify does not call OnPrepareTerminate when skipAbilityStageLifecycle is true
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SchedulePrepareTerminate_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SchedulePrepareTerminate_0100 start.";
    bool isAsync = true;
    bool callbackCalled = false;
    auto callback = [&callbackCalled](AppExecFwk::OnPrepareTerminationResult) {
        callbackCalled = true;
    };

    std::string moduleName = "entry_prepare_skip_true";
    auto abilityStage = std::make_shared<PrepareTerminateCountStage>(true);
    abilityStage->SetSkipAbilityStageLifecycle(true);
    ohosApplication_->abilityStages_.emplace(moduleName, abilityStage);

    ohosApplication_->SchedulePrepareTerminate(moduleName, callback, isAsync);
    EXPECT_EQ(abilityStage->callCount_, 0);
    EXPECT_TRUE(callbackCalled);
    EXPECT_FALSE(isAsync);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SchedulePrepareTerminate_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_SchedulePrepareTerminate_0200
* @tc.name: SchedulePrepareTerminate
* @tc.desc: Verify calls fallback callback when OnPrepareTerminate returns false
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SchedulePrepareTerminate_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SchedulePrepareTerminate_0200 start.";
    bool isAsync = false;
    bool callbackCalled = false;
    auto callback = [&callbackCalled](AppExecFwk::OnPrepareTerminationResult) {
        callbackCalled = true;
    };

    std::string moduleName = "entry_prepare_return_false";
    auto abilityStage = std::make_shared<PrepareTerminateCountStage>(false);
    abilityStage->SetSkipAbilityStageLifecycle(false);
    ohosApplication_->abilityStages_.emplace(moduleName, abilityStage);

    ohosApplication_->SchedulePrepareTerminate(moduleName, callback, isAsync);
    EXPECT_EQ(abilityStage->callCount_, 1);
    EXPECT_TRUE(callbackCalled);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SchedulePrepareTerminate_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_SchedulePrepareTerminate_0300
* @tc.name: SchedulePrepareTerminate
* @tc.desc: Verify does not call fallback callback when OnPrepareTerminate returns true
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SchedulePrepareTerminate_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SchedulePrepareTerminate_0300 start.";
    bool isAsync = false;
    bool callbackCalled = false;
    auto callback = [&callbackCalled](AppExecFwk::OnPrepareTerminationResult) {
        callbackCalled = true;
    };

    std::string moduleName = "entry_prepare_return_true";
    auto abilityStage = std::make_shared<PrepareTerminateCountStage>(true);
    abilityStage->SetSkipAbilityStageLifecycle(false);
    ohosApplication_->abilityStages_.emplace(moduleName, abilityStage);

    ohosApplication_->SchedulePrepareTerminate(moduleName, callback, isAsync);
    EXPECT_EQ(abilityStage->callCount_, 1);
    EXPECT_FALSE(callbackCalled);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SchedulePrepareTerminate_0300 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_GetConfiguration_0100
* @tc.name: GetConfiguration
* @tc.desc: Verify function GetConfiguration pointer configuration_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_GetConfiguration_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetConfiguration_0100 start.";
    Configuration config;
    ohosApplication_->configuration_ = nullptr;
    ohosApplication_->GetConfiguration();
    EXPECT_TRUE(ohosApplication_->configuration_ == nullptr);
    ohosApplication_->SetConfiguration(config);
    ohosApplication_->GetConfiguration();
    EXPECT_TRUE(ohosApplication_->configuration_ != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetConfiguration_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_SetExtensionTypeMap_0100
* @tc.name: SetExtensionTypeMap
* @tc.desc: Verify function SetExtensionTypeMap map extensionTypeMap_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SetExtensionTypeMap_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetExtensionTypeMap_0100 start.";
    const std::string name = "entry";
    constexpr int32_t id = 1;
    std::map<int32_t, std::string> map;
    map.emplace(id, name);
    ohosApplication_->SetExtensionTypeMap(map);
    EXPECT_FALSE(ohosApplication_->extensionTypeMap_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetExtensionTypeMap_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_NotifyLoadRepairPatch_0100
* @tc.name: NotifyLoadRepairPatch
* @tc.desc: Verify function NotifyLoadRepairPatch pointer runtime_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_NotifyLoadRepairPatch_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyLoadRepairPatch_0100 start.";
    const std::string hqfFile = "hqfFile";
    const std::string hapPat = "hapPat";
    EXPECT_TRUE(ohosApplication_->NotifyLoadRepairPatch(hqfFile, hapPat));
    EXPECT_TRUE(ohosApplication_->runtime_ == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyLoadRepairPatch_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_NotifyLoadRepairPatch_0200
* @tc.name: NotifyLoadRepairPatch
* @tc.desc: Verify function NotifyLoadRepairPatch function LoadRepairPatch called
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_NotifyLoadRepairPatch_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyLoadRepairPatch_0200 start.";
    const std::string hqfFile = "hqfFile";
    const std::string hapPath = "hapPath";
    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockRuntime>();
    ohosApplication_->NotifyLoadRepairPatch(hqfFile, hapPath);
    EXPECT_TRUE(ohosApplication_->runtime_ != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyLoadRepairPatch_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_NotifyHotReloadPage_0100
* @tc.name: NotifyHotReloadPage
* @tc.desc: Verify function NotifyHotReloadPage pointer runtime_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_NotifyHotReloadPage_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyHotReloadPage_0100 start.";
    ohosApplication_->NotifyHotReloadPage();
    EXPECT_TRUE(ohosApplication_->runtime_ == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyHotReloadPage_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_NotifyHotReloadPage_0200
* @tc.name: NotifyHotReloadPage
* @tc.desc: Verify function NotifyHotReloadPage pointer runtime_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_NotifyHotReloadPage_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyHotReloadPage_0200 start.";
    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockRuntime>();
    ohosApplication_->NotifyHotReloadPage();
    EXPECT_TRUE(ohosApplication_->runtime_ != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyHotReloadPage_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_NotifyUnLoadRepairPatch_0100
* @tc.name: NotifyUnLoadRepairPatch
* @tc.desc: Verify function NotifyHotReloadPage pointer runtime_ empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_NotifyUnLoadRepairPatch_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyUnLoadRepairPatch_0100 start.";
    std::string hqfFile = "hqfFile";
    ohosApplication_->NotifyUnLoadRepairPatch(hqfFile);
    EXPECT_TRUE(ohosApplication_->runtime_ == nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyUnLoadRepairPatch_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_NotifyUnLoadRepairPatch_0200
* @tc.name: NotifyUnLoadRepairPatch
* @tc.desc: Verify function NotifyHotReloadPage pointer runtime_ not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_NotifyUnLoadRepairPatch_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyUnLoadRepairPatch_0200 start.";
    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockRuntime>();
    std::string hqfFile = "entry";
    ohosApplication_->NotifyUnLoadRepairPatch(hqfFile);
    EXPECT_TRUE(ohosApplication_->runtime_ != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_NotifyUnLoadRepairPatch_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_SetAppEnv_0100
* @tc.name: SetAppEnv
* @tc.desc: Verify SetAppEnv function
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SetAppEnv_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetAppEnv_0100 start.";
    AppEnvironment appEnvironment;
    appEnvironment.name = "env_key_demo";
    appEnvironment.value = "env_value_demo";
    std::vector<AppEnvironment> appEnvironments = {appEnvironment};
    ohosApplication_->SetAppEnv(appEnvironments);
    std::string appEnvVal = getenv(appEnvironment.name.c_str());
    EXPECT_EQ(appEnvVal, appEnvironment.value);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetAppEnv_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_ScheduleNewProcessRequest_0100
* @tc.name: ScheduleNewProcessRequest
* @tc.desc: Verify function ScheduleNewProcessRequest pointer abilityStage not empty
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_ScheduleNewProcessRequest_0100, TestSize.Level1)
{
    Want want;
    bool isAsync = false;
    std::string moduleName = "entry";
    ohosApplication_->ScheduleNewProcessRequest(want, moduleName, nullptr, isAsync);
    EXPECT_FALSE(isAsync);
    auto callback = [](std::string) {};
    ohosApplication_->ScheduleNewProcessRequest(want, moduleName, callback, isAsync);
    EXPECT_FALSE(isAsync);
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage = std::make_shared<AbilityRuntime::AbilityStage>();
    ohosApplication_->abilityStages_.emplace(moduleName, abilityStage);
    std::string testName = "testName";
    ohosApplication_->ScheduleNewProcessRequest(want, testName, callback, isAsync);
    EXPECT_FALSE(isAsync);
    ohosApplication_->ScheduleNewProcessRequest(want, moduleName, callback, isAsync);
    EXPECT_FALSE(isAsync);
}

/*
* @tc.number: CreateFirstStartupCallbackForRecord_0100
* @tc.name: CreateFirstStartupCallbackForRecord
* @tc.desc: Verify function CreateFirstStartupCallbackForRecord
*/
HWTEST_F(OHOSApplicationTest, CreateFirstStartupCallbackForRecord_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForRecord_0100 start.";
    ASSERT_NE(ohosApplication_, nullptr);
    HapModuleInfo hapModuleInfo;
    auto firstCallback = ohosApplication_->CreateFirstStartupCallbackForRecord(nullptr, nullptr, hapModuleInfo,
        nullptr);
    EXPECT_EQ(firstCallback, nullptr);
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForRecord_0100 end.";
}

/*
* @tc.number: CreateFirstStartupCallbackForRecord_0200
* @tc.name: CreateFirstStartupCallbackForRecord
* @tc.desc: Verify function CreateFirstStartupCallbackForRecord
*/
HWTEST_F(OHOSApplicationTest, CreateFirstStartupCallbackForRecord_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForRecord_0200 start.";
    ASSERT_NE(ohosApplication_, nullptr);
    auto abilityStage = std::make_shared<AbilityRuntime::MockAbilityStage>();
    HapModuleInfo hapModuleInfo;
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = nullptr;
    auto want = std::make_shared<Want>();
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(info, token, want, 0);

    auto firstCallback = ohosApplication_->CreateFirstStartupCallbackForRecord(abilityStage, abilityRecord,
        hapModuleInfo, nullptr);
    EXPECT_EQ(firstCallback, nullptr);
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForRecord_0200 end.";
}

/*
* @tc.number: CreateFirstStartupCallbackForRecord_0300
* @tc.name: CreateFirstStartupCallbackForRecord
* @tc.desc: Verify function CreateFirstStartupCallbackForRecord
*/
HWTEST_F(OHOSApplicationTest, CreateFirstStartupCallbackForRecord_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForRecord_0300 start.";
    ASSERT_NE(ohosApplication_, nullptr);
    auto abilityStage = std::make_shared<AbilityRuntime::MockAbilityStage>();
    HapModuleInfo hapModuleInfo;
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    auto want = std::make_shared<Want>();
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(info, token, want, 0);

    auto processInfo = std::make_shared<ProcessInfo>();
    processInfo->SetProcessType(ProcessType::NORMAL);
    ohosApplication_->SetProcessInfo(processInfo);
    
    auto firstCallback = ohosApplication_->CreateFirstStartupCallbackForRecord(abilityStage, abilityRecord,
        hapModuleInfo, nullptr);
    EXPECT_NE(firstCallback, nullptr);

    ohosApplication_ = nullptr;
    EXPECT_CALL(*abilityStage, RunAutoStartupTask(_, _, _, _, _))
        .Times(0);
    firstCallback();
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForRecord_0300 end.";
}

/*
* @tc.number: CreateFirstStartupCallbackForRecord_0400
* @tc.name: CreateFirstStartupCallbackForRecord
* @tc.desc: Verify function CreateFirstStartupCallbackForRecord
*/
HWTEST_F(OHOSApplicationTest, CreateFirstStartupCallbackForRecord_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForRecord_0400 start.";
    ASSERT_NE(ohosApplication_, nullptr);
    auto abilityStage = std::make_shared<AbilityRuntime::MockAbilityStage>();
    HapModuleInfo hapModuleInfo;
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    auto want = std::make_shared<Want>();
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(info, token, want, 0);

    auto processInfo = std::make_shared<ProcessInfo>();
    processInfo->SetProcessType(ProcessType::NORMAL);
    ohosApplication_->SetProcessInfo(processInfo);
    
    auto firstCallback = ohosApplication_->CreateFirstStartupCallbackForRecord(nullptr, abilityRecord,
        hapModuleInfo, nullptr);
    EXPECT_NE(firstCallback, nullptr);

    EXPECT_CALL(*abilityStage, RunAutoStartupTask(_, _, _, _, _))
        .Times(0);
    firstCallback();
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForRecord_0400 end.";
}

/*
* @tc.number: CreateFirstStartupCallbackForRecord_0500
* @tc.name: CreateFirstStartupCallbackForRecord
* @tc.desc: Verify function CreateFirstStartupCallbackForRecord
*/
HWTEST_F(OHOSApplicationTest, CreateFirstStartupCallbackForRecord_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForRecord_0500 start.";
    ASSERT_NE(ohosApplication_, nullptr);
    auto abilityStage = std::make_shared<AbilityRuntime::MockAbilityStage>();
    HapModuleInfo hapModuleInfo;
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    auto want = std::make_shared<Want>();
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(info, token, want, 0);

    auto processInfo = std::make_shared<ProcessInfo>();
    processInfo->SetProcessType(ProcessType::NORMAL);
    ohosApplication_->SetProcessInfo(processInfo);
    
    auto firstCallback = ohosApplication_->CreateFirstStartupCallbackForRecord(abilityStage, abilityRecord,
        hapModuleInfo, nullptr);
    ASSERT_NE(firstCallback, nullptr);

    bool asyncCallback = true;
    auto mockHandler = [&asyncCallback](const std::function<void()> &callback, std::shared_ptr<AAFwk::Want> want,
        bool &isAsyncCallback, const std::shared_ptr<AbilityRuntime::Context> &stageContext, bool preAbilityStageLoad) {
        isAsyncCallback = false;
        asyncCallback = isAsyncCallback;
        return ERR_OK;
    };
    EXPECT_CALL(*abilityStage, RunAutoStartupTask(_, _, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockHandler));
    firstCallback();
    EXPECT_EQ(asyncCallback, false);
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForRecord_0500 end.";
}

/*
* @tc.number: CreateFirstStartupCallbackForRecord_0600
* @tc.name: CreateFirstStartupCallbackForRecord
* @tc.desc: Verify function CreateFirstStartupCallbackForRecord
*/
HWTEST_F(OHOSApplicationTest, CreateFirstStartupCallbackForRecord_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForRecord_0600 start.";
    ASSERT_NE(ohosApplication_, nullptr);
    auto abilityStage = std::make_shared<AbilityRuntime::MockAbilityStage>();
    HapModuleInfo hapModuleInfo;
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    auto want = std::make_shared<Want>();
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(info, token, want, 0);

    auto processInfo = std::make_shared<ProcessInfo>();
    processInfo->SetProcessType(ProcessType::NORMAL);
    ohosApplication_->SetProcessInfo(processInfo);
    
    auto firstCallback = ohosApplication_->CreateFirstStartupCallbackForRecord(abilityStage, abilityRecord,
        hapModuleInfo, nullptr);
    ASSERT_NE(firstCallback, nullptr);

    bool asyncCallback = false;
    auto mockHandler = [&asyncCallback](const std::function<void()> &callback, std::shared_ptr<AAFwk::Want> want,
        bool &isAsyncCallback, const std::shared_ptr<AbilityRuntime::Context> &stageContext, bool preAbilityStageLoad) {
        isAsyncCallback = true;
        asyncCallback = isAsyncCallback;
        return ERR_OK;
    };
    EXPECT_CALL(*abilityStage, RunAutoStartupTask(_, _, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockHandler));
    firstCallback();
    EXPECT_EQ(asyncCallback, true);
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForRecord_0600 end.";
}

/*
* @tc.number: CreateFirstStartupCallbackForHap_0100
* @tc.name: CreateFirstStartupCallbackForHap
* @tc.desc: Verify function CreateFirstStartupCallbackForRecord
*/
HWTEST_F(OHOSApplicationTest, CreateFirstStartupCallbackForHap_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForHap_0100 start.";
    ASSERT_NE(ohosApplication_, nullptr);
    ohosApplication_->SetApplicationContext(nullptr);
    HapModuleInfo hapModuleInfo;
    auto firstCallback = ohosApplication_->CreateFirstStartupCallbackForHap(nullptr, hapModuleInfo, nullptr);
    EXPECT_EQ(firstCallback, nullptr);
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForHap_0100 end.";
}

/*
* @tc.number: CreateFirstStartupCallbackForHap_0200
* @tc.name: CreateFirstStartupCallbackForHap
* @tc.desc: Verify function CreateFirstStartupCallbackForRecord
*/
HWTEST_F(OHOSApplicationTest, CreateFirstStartupCallbackForHap_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForHap_0200 start.";
    ASSERT_NE(ohosApplication_, nullptr);
    auto applicationContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    applicationContext->SetApplicationInfo(nullptr);
    ohosApplication_->SetApplicationContext(applicationContext);

    auto processInfo = std::make_shared<ProcessInfo>();
    processInfo->SetProcessType(ProcessType::NORMAL);
    ohosApplication_->SetProcessInfo(processInfo);

    auto abilityStage = std::make_shared<AbilityRuntime::MockAbilityStage>();
    HapModuleInfo hapModuleInfo;
    auto firstCallback = ohosApplication_->CreateFirstStartupCallbackForHap(abilityStage, hapModuleInfo, nullptr);
    EXPECT_EQ(firstCallback, nullptr);
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForHap_0200 end.";
}

/*
* @tc.number: CreateFirstStartupCallbackForHap_0300
* @tc.name: CreateFirstStartupCallbackForHap
* @tc.desc: Verify function CreateFirstStartupCallbackForRecord
*/
HWTEST_F(OHOSApplicationTest, CreateFirstStartupCallbackForHap_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForHap_0300 start.";
    ASSERT_NE(ohosApplication_, nullptr);
    auto applicationContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    applicationContext->AttachContextImpl(contextImpl);
    applicationContext->SetApplicationInfo(applicationInfo);
    ohosApplication_->SetApplicationContext(applicationContext);

    auto processInfo = std::make_shared<ProcessInfo>();
    processInfo->SetProcessType(ProcessType::NORMAL);
    ohosApplication_->SetProcessInfo(processInfo);
    
    auto abilityStage = std::make_shared<AbilityRuntime::MockAbilityStage>();
    HapModuleInfo hapModuleInfo;
    auto firstCallback = ohosApplication_->CreateFirstStartupCallbackForHap(abilityStage, hapModuleInfo, nullptr);
    EXPECT_NE(firstCallback, nullptr);

    ohosApplication_ = nullptr;
    EXPECT_CALL(*abilityStage, RunAutoStartupTask(_, _, _, _, _))
        .Times(0);
    firstCallback();
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForHap_0300 end.";
}

/*
* @tc.number: CreateFirstStartupCallbackForHap_0400
* @tc.name: CreateFirstStartupCallbackForHap
* @tc.desc: Verify function CreateFirstStartupCallbackForRecord
*/
HWTEST_F(OHOSApplicationTest, CreateFirstStartupCallbackForHap_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForHap_0400 start.";
    ASSERT_NE(ohosApplication_, nullptr);
    auto applicationContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    applicationContext->AttachContextImpl(contextImpl);
    applicationContext->SetApplicationInfo(applicationInfo);
    ohosApplication_->SetApplicationContext(applicationContext);

    auto processInfo = std::make_shared<ProcessInfo>();
    processInfo->SetProcessType(ProcessType::NORMAL);
    ohosApplication_->SetProcessInfo(processInfo);
    
    auto abilityStage = std::make_shared<AbilityRuntime::MockAbilityStage>();
    HapModuleInfo hapModuleInfo;
    auto firstCallback = ohosApplication_->CreateFirstStartupCallbackForHap(nullptr, hapModuleInfo, nullptr);
    EXPECT_NE(firstCallback, nullptr);

    EXPECT_CALL(*abilityStage, RunAutoStartupTask(_, _, _, _, _))
        .Times(0);
    firstCallback();
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForHap_0400 end.";
}

/*
* @tc.number: CreateFirstStartupCallbackForHap_0500
* @tc.name: CreateFirstStartupCallbackForHap
* @tc.desc: Verify function CreateFirstStartupCallbackForRecord
*/
HWTEST_F(OHOSApplicationTest, CreateFirstStartupCallbackForHap_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForHap_0500 start.";
    ASSERT_NE(ohosApplication_, nullptr);
    auto applicationContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    applicationContext->AttachContextImpl(contextImpl);
    applicationContext->SetApplicationInfo(applicationInfo);
    ohosApplication_->SetApplicationContext(applicationContext);

    auto processInfo = std::make_shared<ProcessInfo>();
    processInfo->SetProcessType(ProcessType::NORMAL);
    ohosApplication_->SetProcessInfo(processInfo);
    
    auto abilityStage = std::make_shared<AbilityRuntime::MockAbilityStage>();
    HapModuleInfo hapModuleInfo;
    auto firstCallback = ohosApplication_->CreateFirstStartupCallbackForHap(abilityStage, hapModuleInfo, nullptr);
    ASSERT_NE(firstCallback, nullptr);

    bool asyncCallback = false;
    auto mockHandler = [&asyncCallback](const std::function<void()> &callback, std::shared_ptr<AAFwk::Want> want,
        bool &isAsyncCallback, const std::shared_ptr<AbilityRuntime::Context> &stageContext, bool preAbilityStageLoad) {
        isAsyncCallback = true;
        asyncCallback = isAsyncCallback;
        return ERR_OK;
    };
    EXPECT_CALL(*abilityStage, RunAutoStartupTask(_, _, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockHandler));
    firstCallback();
    EXPECT_EQ(asyncCallback, true);
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForHap_0500 end.";
}

/*
* @tc.number: CreateFirstStartupCallbackForHap_0600
* @tc.name: CreateFirstStartupCallbackForHap
* @tc.desc: Verify function CreateFirstStartupCallbackForRecord
*/
HWTEST_F(OHOSApplicationTest, CreateFirstStartupCallbackForHap_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForHap_0600 start.";
    ASSERT_NE(ohosApplication_, nullptr);
    auto applicationContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    applicationContext->AttachContextImpl(contextImpl);
    applicationContext->SetApplicationInfo(applicationInfo);
    ohosApplication_->SetApplicationContext(applicationContext);

    auto processInfo = std::make_shared<ProcessInfo>();
    processInfo->SetProcessType(ProcessType::NORMAL);
    ohosApplication_->SetProcessInfo(processInfo);
    
    auto abilityStage = std::make_shared<AbilityRuntime::MockAbilityStage>();
    HapModuleInfo hapModuleInfo;
    auto firstCallback = ohosApplication_->CreateFirstStartupCallbackForHap(abilityStage, hapModuleInfo, nullptr);
    ASSERT_NE(firstCallback, nullptr);

    bool asyncCallback = true;
    auto mockHandler = [&asyncCallback](const std::function<void()> &callback, std::shared_ptr<AAFwk::Want> want,
        bool &isAsyncCallback, const std::shared_ptr<AbilityRuntime::Context> &stageContext, bool preAbilityStageLoad) {
        isAsyncCallback = false;
        asyncCallback = isAsyncCallback;
        return ERR_OK;
    };
    EXPECT_CALL(*abilityStage, RunAutoStartupTask(_, _, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockHandler));
    firstCallback();
    EXPECT_EQ(asyncCallback, false);
    GTEST_LOG_(INFO) << "CreateFirstStartupCallbackForHap_0600 end.";
}

/*
* @tc.number: UpdateETSRuntime_0100
* @tc.name: UpdateETSRuntime
* @tc.desc: Verify function UpdateETSRuntime
*/
HWTEST_F(OHOSApplicationTest, UpdateETSRuntime_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateETSRuntime_0100 start.";
    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockRuntime>();
    AbilityRuntime::Runtime::Options option;
    option.lang = AbilityRuntime::Runtime::Language::ETS;
    EXPECT_FALSE(ohosApplication_->UpdateETSRuntime(option));
    GTEST_LOG_(INFO) << "UpdateETSRuntime_0100 end.";
}

/*
* @tc.number: UpdateETSRuntime_0200
* @tc.name: UpdateETSRuntime
* @tc.desc: Verify runtime is null
*/
HWTEST_F(OHOSApplicationTest, UpdateETSRuntime_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateETSRuntime_0200 start.";
    AbilityRuntime::Runtime::Options option;
    option.lang = AbilityRuntime::Runtime::Language::ETS;
    ohosApplication_->runtime_ = nullptr;
    EXPECT_FALSE(ohosApplication_->UpdateETSRuntime(option));
    GTEST_LOG_(INFO) << "UpdateETSRuntime_0200 end.";
}

#ifdef SUPPORT_SCREEN
/*
 * @tc.number: AppExecFwk_OHOSApplicationTest_SetApplicationContext_0500
 * @tc.name: SetApplicationContext
 * @tc.desc: Verify function SetApplicationContext RegisterGetAllUIAbilitiesCallback
 */
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SetApplicationContext_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetApplicationContext_0500 start.";
    std::shared_ptr<AbilityRuntime::ApplicationContext> abilityRuntimeContext =
        std::make_shared<AbilityRuntime::ApplicationContext>();
    ASSERT_NE(abilityRuntimeContext, nullptr);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    abilityRuntimeContext->AttachContextImpl(contextImpl);
    ohosApplication_->SetApplicationContext(abilityRuntimeContext);
    ASSERT_NE(ohosApplication_->abilityRuntimeContext_, nullptr);
    ASSERT_NE(contextImpl->getAllUIAbilitiesCallback_, nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetApplicationContext_0500 end.";
}

/*
 * @tc.number: AppExecFwk_OHOSApplicationTest_SetApplicationContext_0600
 * @tc.name: SetApplicationContext
 * @tc.desc: Verify function SetApplicationContext RegisterGetAllUIAbilitiesCallback null applicationSptr
 */
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_SetApplicationContext_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetApplicationContext_0600 start.";
    std::shared_ptr<AbilityRuntime::ApplicationContext> abilityRuntimeContext =
        std::make_shared<AbilityRuntime::ApplicationContext>();
    ASSERT_NE(abilityRuntimeContext, nullptr);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    abilityRuntimeContext->AttachContextImpl(contextImpl);
    ohosApplication_->SetApplicationContext(abilityRuntimeContext);
    ASSERT_NE(ohosApplication_->abilityRuntimeContext_, nullptr);
    ASSERT_NE(contextImpl->getAllUIAbilitiesCallback_, nullptr);
    ohosApplication_ = nullptr;
    std::vector<std::shared_ptr<AbilityRuntime::UIAbility>> uIAbilities;
    contextImpl->GetAllUIAbilities(uIAbilities);
    EXPECT_EQ(uIAbilities.size(), 0);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_SetApplicationContext_0600 end.";
}

/*
 * @tc.number: AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0100
 * @tc.name: GetAllUIAbilities
 * @tc.desc: Verify function GetAllUIAbilities
 */
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0100 start.";
    std::vector<std::shared_ptr<AbilityRuntime::UIAbility>> uIAbilities;
    ohosApplication_->GetAllUIAbilities(uIAbilities);
    EXPECT_EQ(uIAbilities.size(), 0);

    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    ASSERT_NE(ohosApplication_->abilityRecordMgr_, nullptr);
    ohosApplication_->GetAllUIAbilities(uIAbilities);
    EXPECT_EQ(uIAbilities.size(), 0);

    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = nullptr;
    auto record = std::make_shared<AbilityLocalRecord>(info, token, nullptr, 0);
    ohosApplication_->abilityRecordMgr_->abilityRecords_.emplace(token, record);
    ohosApplication_->GetAllUIAbilities(uIAbilities);
    EXPECT_EQ(uIAbilities.size(), 0);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0100 end.";
}

/*
 * @tc.number: AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0200
 * @tc.name: GetAllUIAbilities
 * @tc.desc: Verify function GetAllUIAbilities
 */
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0200 start.";
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    ASSERT_NE(ohosApplication_->abilityRecordMgr_, nullptr);
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    ASSERT_NE(info, nullptr);
    auto record = std::make_shared<AbilityLocalRecord>(info, token, nullptr, 0);
    ohosApplication_->abilityRecordMgr_->abilityRecords_.emplace(token, record);
    std::vector<std::shared_ptr<AbilityRuntime::UIAbility>> uIAbilities;
    ohosApplication_->GetAllUIAbilities(uIAbilities);
    EXPECT_EQ(uIAbilities.size(), 0);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0200 end.";
}

/*
 * @tc.number: AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0300
 * @tc.name: GetAllUIAbilities
 * @tc.desc: Verify function GetAllUIAbilities
 */
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0300 start.";
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    ASSERT_NE(ohosApplication_->abilityRecordMgr_, nullptr);
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    ASSERT_NE(info, nullptr);
    info->type = AbilityType::PAGE;
    auto record = std::make_shared<AbilityLocalRecord>(info, token, nullptr, 0);
    auto abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    abilitythread->abilityImpl_ = std::make_shared<AbilityRuntime::UIAbilityImpl>();
    sptr<AbilityThread> abilityThread = abilitythread;
    ASSERT_NE(abilityThread, nullptr);
    record->SetAbilityThread(abilityThread);
    ohosApplication_->abilityRecordMgr_->abilityRecords_.emplace(token, record);
    std::vector<std::shared_ptr<AbilityRuntime::UIAbility>> uIAbilities;
    ohosApplication_->GetAllUIAbilities(uIAbilities);
    EXPECT_EQ(uIAbilities.size(), 0);

    ohosApplication_->GetAllUIAbilities(uIAbilities);
    EXPECT_EQ(uIAbilities.size(), 0);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0300 end.";
}

/*
 * @tc.number: AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0400
 * @tc.name: GetAllUIAbilities
 * @tc.desc: Verify function GetAllUIAbilities
 */
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0400 start.";
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    ASSERT_NE(ohosApplication_->abilityRecordMgr_, nullptr);
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    ohosApplication_->abilityRecordMgr_->abilityRecords_.emplace(token, nullptr);
    std::vector<std::shared_ptr<AbilityRuntime::UIAbility>> uiAbility;
    ohosApplication_->GetAllUIAbilities(uiAbility);
    EXPECT_EQ(uiAbility.size(), 0);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0400 end.";
}

/*
 * @tc.number: AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0500
 * @tc.name: GetAllUIAbilities
 * @tc.desc: Verify function GetAllUIAbilities
 */
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0500 start.";
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    ASSERT_NE(ohosApplication_->abilityRecordMgr_, nullptr);
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    ASSERT_NE(info, nullptr);
    info->type = AbilityType::PAGE;
    auto record = std::make_shared<AbilityLocalRecord>(info, token, nullptr, 0);
    ohosApplication_->abilityRecordMgr_->abilityRecords_.emplace(token, record);
    std::vector<std::shared_ptr<AbilityRuntime::UIAbility>> uiAbility;
    ohosApplication_->GetAllUIAbilities(uiAbility);
    EXPECT_EQ(uiAbility.size(), 0);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0500 end.";
}

/*
 * @tc.number: AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0600
 * @tc.name: GetAllUIAbilities
 * @tc.desc: Verify function GetAllUIAbilities
 */
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0600 start.";
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    ASSERT_NE(ohosApplication_->abilityRecordMgr_, nullptr);
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    ASSERT_NE(info, nullptr);
    info->type = AbilityType::PAGE;
    auto record = std::make_shared<AbilityLocalRecord>(info, token, nullptr, 0);
    auto abilitythread = new (std::nothrow) AbilityRuntime::UIAbilityThread();
    sptr<AbilityThread> abilityThread = abilitythread;
    ASSERT_NE(abilityThread, nullptr);
    record->SetAbilityThread(abilityThread);
    ohosApplication_->abilityRecordMgr_->abilityRecords_.emplace(token, record);
    std::vector<std::shared_ptr<AbilityRuntime::UIAbility>> uiAbility;
    ohosApplication_->GetAllUIAbilities(uiAbility);
    EXPECT_EQ(uiAbility.size(), 0);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_GetAllUIAbilities_0600 end.";
}

/*
 * @tc.number: AppExecFwk_OHOSApplicationTest_RegisterGetAllUIAbilitiesCallback_0100
 * @tc.name: RegisterGetAllUIAbilitiesCallback
 * @tc.desc: Verify function RegisterGetAllUIAbilitiesCallback
 */
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_RegisterGetAllUIAbilitiesCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_RegisterGetAllUIAbilitiesCallback_0100 start.";
    std::shared_ptr<AbilityRuntime::ApplicationContext> abilityRuntimeContext =
        std::make_shared<AbilityRuntime::ApplicationContext>();
    ASSERT_NE(abilityRuntimeContext, nullptr);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(contextImpl, nullptr);
    abilityRuntimeContext->AttachContextImpl(contextImpl);
    std::weak_ptr<OHOSApplication> emptyWeakPtr;
    ohosApplication_->RegisterGetAllUIAbilitiesCallback(abilityRuntimeContext, emptyWeakPtr);
    ASSERT_NE(contextImpl->getAllUIAbilitiesCallback_, nullptr);
    std::vector<std::shared_ptr<AbilityRuntime::UIAbility>> uiAbility;
    contextImpl->getAllUIAbilitiesCallback_(uiAbility);
    EXPECT_EQ(uiAbility.size(), 0);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_RegisterGetAllUIAbilitiesCallback_0100 end.";
}
#endif

/*
 * @tc.number: OnUpdate_0100
 * @tc.name: OnUpdate
 * @tc.desc: Verify function OnUpdate
 */
HWTEST_F(OHOSApplicationTest, OnUpdate_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnUpdate_0100 start.";
    auto ohosApplication = std::make_shared<OHOSApplication>();
    ASSERT_NE(ohosApplication, nullptr);
    ohosApplication->OnHyperSnapUpdate();
    std::shared_ptr<AbilityRuntime::ApplicationContext> abilityRuntimeContext =
        std::make_shared<AbilityRuntime::ApplicationContext>();
    ASSERT_NE(abilityRuntimeContext, nullptr);
    ohosApplication->SetApplicationContext(abilityRuntimeContext);
    auto abilityStage = std::make_shared<AbilityRuntime::MockAbilityStage>();
    ASSERT_NE(abilityStage, nullptr);
    EXPECT_CALL(*abilityStage, OnLaunchFromHyperSnap()).Times(1);
    ohosApplication->abilityStages_["test"] = abilityStage;
    ohosApplication->OnHyperSnapUpdate();
    GTEST_LOG_(INFO) << "OnUpdate_0100 end.";
}

/*
 * @tc.number: OnUpdate_0200
 * @tc.name: OnUpdate
 * @tc.desc: Verify skipAbilityStageLifecycle=true does not call OnLaunchFromHyperSnap
 */
HWTEST_F(OHOSApplicationTest, OnUpdate_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnUpdate_0200 start.";
    auto ohosApplication = std::make_shared<OHOSApplication>();
    ASSERT_NE(ohosApplication, nullptr);
    auto abilityStage = std::make_shared<AbilityRuntime::MockAbilityStage>();
    ASSERT_NE(abilityStage, nullptr);
    abilityStage->SetSkipAbilityStageLifecycle(true);
    ohosApplication->abilityStages_["test_skip_true"] = abilityStage;

    EXPECT_CALL(*abilityStage, OnLaunchFromHyperSnap()).Times(0);
    ohosApplication->OnHyperSnapUpdate();
    GTEST_LOG_(INFO) << "OnUpdate_0200 end.";
}

/*
 * @tc.number: AddAbility_ShouldDonothingWhenInputIsInvalid
 * @tc.name: AddAbility
 * @tc.desc: Verify function AddAbility_ShouldDonothingWhenInputIsInvalid
 */
HWTEST_F(OHOSApplicationTest, AddAbility_ShouldDonothingWhenInputIsInvalid, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddAbility_ShouldDonothingWhenInputIsInvalid start.";
    auto ohosApplication = std::make_shared<OHOSApplication>();
    ASSERT_NE(ohosApplication, nullptr);
    auto abilityStage = std::make_shared<AbilityRuntime::AbilityStage>();
    ASSERT_NE(abilityStage, nullptr);
    ohosApplication->AddAbility(nullptr, nullptr, nullptr);
    ohosApplication->AddAbility(abilityStage, nullptr, nullptr);
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    ohosApplication->AddAbility(abilityStage, token, nullptr);
    EXPECT_FALSE(abilityStage->ContainsAbility());
    GTEST_LOG_(INFO) << "AddAbility_ShouldDonothingWhenInputIsInvalid end.";
}

/*
 * @tc.number: AddAbility_ShouldAddAbilityToAbilityStageWhenInputIsValid
 * @tc.name: AddAbility
 * @tc.desc: Verify function AddAbility
 */
HWTEST_F(OHOSApplicationTest, AddAbility_ShouldAddAbilityToAbilityStageWhenInputIsValid, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddAbility_ShouldAddAbilityToAbilityStageWhenInputIsValid start.";
    auto ohosApplication = std::make_shared<OHOSApplication>();
    ASSERT_NE(ohosApplication, nullptr);
    auto abilityStage = std::make_shared<AbilityRuntime::AbilityStage>();
    ASSERT_NE(abilityStage, nullptr);
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(nullptr, token, nullptr, 0);
    ohosApplication->AddAbility(abilityStage, token, abilityRecord);
    EXPECT_TRUE(abilityStage->ContainsAbility());

    abilityStage->RemoveAbility(token);
    EXPECT_FALSE(abilityStage->ContainsAbility());
    std::shared_ptr<AbilityRuntime::ApplicationContext> abilityRuntimeContext =
        std::make_shared<AbilityRuntime::ApplicationContext>();
    ASSERT_NE(abilityRuntimeContext, nullptr);
    ohosApplication->SetApplicationContext(abilityRuntimeContext);
    ohosApplication->AddAbility(abilityStage, token, abilityRecord);
    EXPECT_TRUE(abilityStage->ContainsAbility());

    abilityStage->RemoveAbility(token);
    EXPECT_FALSE(abilityStage->ContainsAbility());
    ohosApplication->AddAbility(abilityStage, token, abilityRecord);
    EXPECT_TRUE(abilityStage->ContainsAbility());
    GTEST_LOG_(INFO) << "AddAbility_ShouldAddAbilityToAbilityStageWhenInputIsValid end.";
}

/*
 * @tc.number: AddAbility_0100
 * @tc.name: AddAbility
 * @tc.desc: Verify OnAboutToCreateAbility is not called when skipAbilityStageLifecycle is true
 */
HWTEST_F(OHOSApplicationTest, AddAbility_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddAbility_0200 start.";
    auto ohosApplication = std::make_shared<OHOSApplication>();
    ASSERT_NE(ohosApplication, nullptr);
    auto abilityStage = std::make_shared<AbilityRuntime::MockAbilityStage>();
    ASSERT_NE(abilityStage, nullptr);
    abilityStage->SetSkipAbilityStageLifecycle(true);

    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    ASSERT_NE(token, nullptr);
    auto abilityRecord = std::make_shared<AbilityLocalRecord>(nullptr, token, nullptr, 0);
    ASSERT_NE(abilityRecord, nullptr);

    EXPECT_CALL(*abilityStage, OnAboutToCreateAbility()).Times(0);
    ohosApplication->AddAbility(abilityStage, token, abilityRecord);
    EXPECT_TRUE(abilityStage->ContainsAbility());
    GTEST_LOG_(INFO) << "AddAbility_0200 end.";
}

/**
 * @tc.number: AppExecFwk_OHOSApplicationTest_LaunchElement_0100
 * @tc.name: AddAbilityStage sets launchElement with valid Want
 * @tc.desc: Test that AddAbilityStage correctly sets launchElement when Want contains valid ElementName.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_LaunchElement_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_LaunchElement_0100 start.";

    // Create mock objects using the correct types
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    auto want = std::make_shared<Want>();

    // Set ElementName in Want
    AppExecFwk::ElementName elementName;
    elementName.SetBundleName("com.example.launchtest");
    elementName.SetAbilityName("LaunchTestAbility");
    elementName.SetModuleName("entry");
    want->SetElement(elementName);

    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(info, token, want, 0);
    ASSERT_NE(abilityRecord, nullptr);

    auto callback = [](const std::shared_ptr<AbilityRuntime::Context> &) {};
    bool isAsyncCallback = false;

    // Call AddAbilityStage
    ohosApplication_->AddAbilityStage(abilityRecord, callback, isAsyncCallback);

    // Verify abilityStages_ is not empty
    EXPECT_FALSE(ohosApplication_->abilityStages_.empty());

    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_LaunchElement_0100 end.";
}

/**
 * @tc.number: AppExecFwk_OHOSApplicationTest_LaunchElement_0200
 * @tc.name: AddAbilityStage handles null Want gracefully
 * @tc.desc: Test that AddAbilityStage handles null Want without crashing.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_LaunchElement_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_LaunchElement_0200 start.";

    // Create mock objects with null Want
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();

    // Pass nullptr as Want
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(info, token, nullptr, 0);
    ASSERT_NE(abilityRecord, nullptr);

    auto callback = [](const std::shared_ptr<AbilityRuntime::Context> &) {};
    bool isAsyncCallback = false;

    // Call AddAbilityStage - should not crash even with null Want
    ohosApplication_->AddAbilityStage(abilityRecord, callback, isAsyncCallback);

    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_LaunchElement_0200 end.";
}

/**
 * @tc.number: AppExecFwk_OHOSApplicationTest_LaunchElement_0300
 * @tc.name: AddAbilityStage sets launchElement before onCreate
 * @tc.desc: Test that AddAbilityStage sets launchElement before calling abilityStage->OnCreate().
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_LaunchElement_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_LaunchElement_0300 start.";

    // Create mock objects
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    auto want = std::make_shared<Want>();

    // Set ElementName in Want
    AppExecFwk::ElementName elementName;
    elementName.SetBundleName("com.example.timingtest");
    elementName.SetAbilityName("TimingTestAbility");
    want->SetElement(elementName);

    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(info, token, want, 0);
    ASSERT_NE(abilityRecord, nullptr);

    auto callback = [](const std::shared_ptr<AbilityRuntime::Context> &) {};
    bool isAsyncCallback = false;

    // Call AddAbilityStage
    ohosApplication_->AddAbilityStage(abilityRecord, callback, isAsyncCallback);

    // Verify that abilityStages_ is not empty (AbilityStage was created)
    EXPECT_FALSE(ohosApplication_->abilityStages_.empty());

    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_LaunchElement_0300 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_InitJSLeakWatcher_0100
* @tc.name: InitJSLeakWatcher
* @tc.desc: Verify function InitJSLeakWatcher when runtime_ is nullptr
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_InitJSLeakWatcher_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_InitJSLeakWatcher_0100 start.";

    // Ensure runtime_ is nullptr
    ohosApplication_->runtime_ = nullptr;

    // Call InitJSLeakWatcher - should handle gracefully without crashing
    std::string bundleName = "com.example.test";
    ohosApplication_->InitJSLeakWatcher(bundleName);

    // Verify runtime_ is still nullptr (no crash occurred)
    EXPECT_TRUE(ohosApplication_->runtime_ == nullptr);

    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_InitJSLeakWatcher_0100 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_InitJSLeakWatcher_0200
* @tc.name: InitJSLeakWatcher
* @tc.desc: Verify function InitJSLeakWatcher with valid runtime_
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_InitJSLeakWatcher_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_InitJSLeakWatcher_0200 start.";

    // Set runtime_ to MockJsRuntime
    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockJsRuntime>();
    ASSERT_NE(ohosApplication_->runtime_, nullptr);

    // Call InitJSLeakWatcher with bundle name
    std::string bundleName = "com.example.test";
    ohosApplication_->InitJSLeakWatcher(bundleName);

    // Verify runtime_ is still valid after the call
    EXPECT_TRUE(ohosApplication_->runtime_ != nullptr);

    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_InitJSLeakWatcher_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_InitJSLeakWatcher_0300
* @tc.name: InitJSLeakWatcher
* @tc.desc: Verify function InitJSLeakWatcher with empty bundle name
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_InitJSLeakWatcher_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_InitJSLeakWatcher_0300 start.";

    // Set runtime_ to MockJsRuntime
    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockJsRuntime>();
    ASSERT_NE(ohosApplication_->runtime_, nullptr);

    // Call InitJSLeakWatcher with empty bundle name
    std::string bundleName = "";
    ohosApplication_->InitJSLeakWatcher(bundleName);

    // Verify runtime_ is still valid after the call
    EXPECT_TRUE(ohosApplication_->runtime_ != nullptr);

    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_InitJSLeakWatcher_0300 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_InitJSLeakWatcher_0400
* @tc.name: InitJSLeakWatcher
* @tc.desc: Verify function InitJSLeakWatcher with long bundle name
*/
HWTEST_F(OHOSApplicationTest, AppExecFwk_OHOSApplicationTest_InitJSLeakWatcher_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_InitJSLeakWatcher_0400 start.";

    // Set runtime_ to MockJsRuntime
    ohosApplication_->runtime_ = std::make_unique<AbilityRuntime::MockJsRuntime>();
    ASSERT_NE(ohosApplication_->runtime_, nullptr);

    // Call InitJSLeakWatcher with long bundle name
    std::string bundleName = "com.very.long.package.name.for.testing.purposes.bundle";
    ohosApplication_->InitJSLeakWatcher(bundleName);

    // Verify runtime_ is still valid after the call
    EXPECT_TRUE(ohosApplication_->runtime_ != nullptr);

    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_InitJSLeakWatcher_0400 end.";
}
}  // namespace AppExecFwk
}  // namespace OHOS
