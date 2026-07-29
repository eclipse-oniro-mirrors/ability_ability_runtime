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
#include "ability.h"
#include "ability_local_record.h"
#include "ability_record_mgr.h"
#include "ability_stage.h"
#include "application_configuration_manager.h"
#include "application_context.h"
#include "application_impl.h"
#include "application_info.h"
#include "configuration.h"
#include "context_deal.h"
#include "context_impl.h"
#include "fa_ability_thread.h"
#include "hap_module_info.h"
#include "hilog_tag_wrapper.h"
#include "mock_i_remote_object.h"
#include "mock_runtime.h"
#include "ohos_application.h"
#include "pac_map.h"
#include "process_info.h"
#include "resource_manager.h"
#include "runtime.h"
#include "ui_ability.h"
#undef private
#include <unordered_map>

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AppExecFwk {
class OHOSApplicationFirstTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<OHOSApplication> ohosApplication_;
    std::unordered_map<std::string, std::shared_ptr<AbilityRuntime::AbilityStage>> abilityStages_;
};

void OHOSApplicationFirstTest::SetUpTestCase(void)
{}

void OHOSApplicationFirstTest::TearDownTestCase(void)
{}

void OHOSApplicationFirstTest::SetUp()
{
    ohosApplication_ = std::make_shared<OHOSApplication>();
}

void OHOSApplicationFirstTest::TearDown()
{
    abilityStages_.clear();
    ohosApplication_ = nullptr;
}

/*
* @tc.number: AppExecFwk_OHOSApplicationFirstTest_CleanEmptyAbilityStage_0100
* @tc.name: CleanEmptyAbilityStage
* @tc.desc: Test when all abilityStages are nullptr then the map should be empty.
*/
HWTEST_F(OHOSApplicationFirstTest, CleanEmptyAbilityStage_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "CleanEmptyAbilityStage_0100 start");
    ohosApplication_->abilityStages_ = abilityStages_;
    ohosApplication_->CleanEmptyAbilityStage();
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    TAG_LOGI(AAFwkTag::TEST, "CleanEmptyAbilityStage_0100 end");
}

/*
* @tc.number: AppExecFwk_OHOSApplicationFirstTest_CleanEmptyAbilityStage_0200
* @tc.name: CleanEmptyAbilityStage
* @tc.desc: Test when some abilityStages are not nullptr then the map should not be empty.
*/
HWTEST_F(OHOSApplicationFirstTest, CleanEmptyAbilityStage_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "CleanEmptyAbilityStage_0200 start");
    AppExecFwk::HapModuleInfo info;
    AbilityRuntime::Runtime::Options options;
    auto runTime = AbilityRuntime::Runtime::Create(options);
    abilityStages_.emplace("1", AbilityRuntime::AbilityStage::Create(runTime, info));
    abilityStages_.emplace("2", AbilityRuntime::AbilityStage::Create(runTime, info));
    abilityStages_.emplace("3", AbilityRuntime::AbilityStage::Create(runTime, info));
    ohosApplication_->abilityStages_ = abilityStages_;
    EXPECT_FALSE(ohosApplication_->abilityStages_.empty());
    ohosApplication_->CleanEmptyAbilityStage();
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    abilityStages_.clear();
    TAG_LOGI(AAFwkTag::TEST, "CleanEmptyAbilityStage_0200 end");
}

/*
* @tc.number: AppExecFwk_OHOSApplicationFirstTest_CleanEmptyAbilityStage_0300
* @tc.name: CleanEmptyAbilityStage
* @tc.desc: Test when all abilityStages are non-empty then the map should not be empty.
*/
HWTEST_F(OHOSApplicationFirstTest, CleanEmptyAbilityStage_0300, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "CleanEmptyAbilityStage_0300 start");
    AppExecFwk::HapModuleInfo info;
    AbilityRuntime::Runtime::Options options;
    auto runTime = AbilityRuntime::Runtime::Create(options);
    abilityStages_["1"] = nullptr;
    abilityStages_["2"] = nullptr;
    abilityStages_["3"] = nullptr;
    for (auto &it : abilityStages_) {
        it.second = AbilityRuntime::AbilityStage::Create(runTime, info);
    }
    ohosApplication_->abilityStages_ = abilityStages_;
    EXPECT_FALSE(ohosApplication_->abilityStages_.empty());
    ohosApplication_->CleanEmptyAbilityStage();
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    abilityStages_.clear();
    TAG_LOGI(AAFwkTag::TEST, "CleanEmptyAbilityStage_0300 end");
}

/*
* @tc.number: AppExecFwk_OHOSApplicationFirstTest_IsUpdateColorNeeded_0100
* @tc.name: IsUpdateColorNeeded
* @tc.desc: Test when all conditions are met, IsUpdateColorNeeded should return false.
*/
HWTEST_F(OHOSApplicationFirstTest, IsUpdateColorNeeded_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUpdateColorNeeded_0100 start");
    Configuration config;
    std::string colorMode = "light";
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, colorMode);
    auto colorMode1 = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    AbilityRuntime::SetLevel level = AbilityRuntime::SetLevel::Application;
    AbilityRuntime::ApplicationConfigurationManager::GetInstance().SetColorModeSetLevel(level, colorMode1);
    AbilityRuntime::SetLevel level1 = AbilityRuntime::SetLevel::System;
    std::string colorModeIsSetBySa =
        config.GetItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_SA);
    bool result = ohosApplication_->IsUpdateColorNeeded(config, level1);
    if (level1 < AbilityRuntime::SetLevel::SA && !colorModeIsSetBySa.empty()) {
        level1 = AbilityRuntime::SetLevel::SA;
    }
    auto colorLevel =
        static_cast<uint8_t>(AbilityRuntime::ApplicationConfigurationManager::GetInstance().GetColorModeSetLevel());
    TAG_LOGI(AAFwkTag::TEST, "colorLevel = %{public}d", colorLevel);
    EXPECT_TRUE(level1 < AbilityRuntime::ApplicationConfigurationManager::GetInstance().GetColorModeSetLevel());
    EXPECT_FALSE(colorMode1.empty());
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsUpdateColorNeeded_0100 end");
}

/*
* @tc.number: AppExecFwk_OHOSApplicationFirstTest_IsUpdateColorNeeded_0200
* @tc.name: IsUpdateColorNeeded
* @tc.desc: Test when all conditions are met, IsUpdateColorNeeded should return true.
*/
HWTEST_F(OHOSApplicationFirstTest, IsUpdateColorNeeded_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUpdateColorNeeded_0200 start");
    Configuration config;
    std::string colorMode = "auto";
    ohosApplication_->configuration_ = std::make_shared<Configuration>();
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, colorMode);
    std::string colorMode1 = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    EXPECT_FALSE(colorMode1.empty());
    AbilityRuntime::SetLevel level = AbilityRuntime::SetLevel::Application;
    bool result = ohosApplication_->IsUpdateColorNeeded(config, level);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsUpdateColorNeeded_0200 end");
}

/*
* @tc.number: AppExecFwk_OHOSApplicationFirstTest_IsUpdateLanguageNeeded_0100
* @tc.name: IsUpdateLanguageNeeded
* @tc.desc: Test IsUpdateLanguageNeeded should return true.
*/
HWTEST_F(OHOSApplicationFirstTest, IsUpdateLanguageNeeded_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUpdateLanguageNeeded_0100 start");
    Configuration config;
    AbilityRuntime::SetLevel level = AbilityRuntime::SetLevel::Application;
    AbilityRuntime::ApplicationConfigurationManager::GetInstance().SetLanguageSetLevel(level);
    std::string testLanguge = "ch-zh";
    ohosApplication_->configuration_ = std::make_shared<Configuration>();
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, testLanguge);
    std::string language = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_FALSE(language.empty());
    bool result = ohosApplication_->IsUpdateLanguageNeeded(config, level);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsUpdateLanguageNeeded_0100 end");
}

/*
* @tc.number: AppExecFwk_OHOSApplicationFirstTest_IsUpdateLanguageNeeded_0200
* @tc.name: IsUpdateLanguageNeeded
* @tc.desc: Test IsUpdateLanguageNeeded should return false.
*/
HWTEST_F(OHOSApplicationFirstTest, IsUpdateLanguageNeeded_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUpdateLanguageNeeded_0200 start");
    Configuration config;
    AbilityRuntime::SetLevel level = AbilityRuntime::SetLevel::Application;
    AbilityRuntime::ApplicationConfigurationManager::GetInstance().SetLanguageSetLevel(level);
    ohosApplication_->configuration_ = std::make_shared<Configuration>();
    std::string language = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    bool result = ohosApplication_->IsUpdateLanguageNeeded(config, level);
    EXPECT_TRUE(language.empty());
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsUpdateLanguageNeeded_0200 end");
}

/*
* @tc.number: AppExecFwk_OHOSApplicationFirstTest_IsUpdateLanguageNeeded_0300
* @tc.name: IsUpdateLanguageNeeded
* @tc.desc: Test IsUpdateLanguageNeeded should return false.
*/
HWTEST_F(OHOSApplicationFirstTest, IsUpdateLanguageNeeded_0300, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUpdateLanguageNeeded_0300 start");
    Configuration config;
    AbilityRuntime::SetLevel level = AbilityRuntime::SetLevel::Application;
    AbilityRuntime::SetLevel testLevel = AbilityRuntime::SetLevel::System;
    AbilityRuntime::ApplicationConfigurationManager::GetInstance().SetLanguageSetLevel(level);
    std::string testLanguge = "ch-zh";
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, testLanguge);
    ohosApplication_->configuration_ = std::make_shared<Configuration>();
    std::string language = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    bool result = ohosApplication_->IsUpdateLanguageNeeded(config, testLevel);
    EXPECT_FALSE(language.empty());
    EXPECT_FALSE(level < AbilityRuntime::ApplicationConfigurationManager::GetInstance().GetLanguageSetLevel());
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsUpdateLanguageNeeded_0300 end");
}

/*
* @tc.number: AppExecFwk_OHOSApplicationFirstTest_IsMainProcess_0100
* @tc.name: IsMainProcess
* @tc.desc: Test when processType is not NORMAL then IsMainProcess returns false.
*/
HWTEST_F(OHOSApplicationFirstTest, IsMainProcess_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsMainProcess_0100 start");
    auto info = std::make_shared<ProcessInfo>("testProcess", 10);
    info->SetProcessType(ProcessType::EXTENSION);
    ohosApplication_->SetProcessInfo(info);
    auto res = ohosApplication_->IsMainProcess("testBundleName", "testProcess");
    auto processInfo = ohosApplication_->GetProcessInfo();
    std::string processName = processInfo->GetProcessName();
    EXPECT_EQ(processName, "testProcess");
    EXPECT_TRUE(res);
    TAG_LOGI(AAFwkTag::TEST, "IsMainProcess_0100 end");
}

/*
* @tc.number: AppExecFwk_OHOSApplicationFirstTest_IsMainProcess_0200
* @tc.name: IsMainProcess
* @tc.desc: Test when processType is not NORMAL then IsMainProcess returns false.
*/
HWTEST_F(OHOSApplicationFirstTest, IsMainProcess_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsMainProcess_0200 start");
    auto info = std::make_shared<ProcessInfo>("testProcess", 10);
    info->SetProcessType(ProcessType::EXTENSION);
    ohosApplication_->SetProcessInfo(info);
    auto res = ohosApplication_->IsMainProcess("testBundleName", "testProcess");
    auto processInfo = ohosApplication_->GetProcessInfo();
    ProcessType processType = processInfo->GetProcessType();
    EXPECT_NE(processType, ProcessType::NORMAL);
    EXPECT_TRUE(res);
    TAG_LOGI(AAFwkTag::TEST, "IsMainProcess_0200 end");
}

/*
* @tc.number: AppExecFwk_OHOSApplicationFirstTest_IsMainProcess_0300
* @tc.name: IsMainProcess
* @tc.desc: Test when processType is NORMAL then IsMainProcess returns true.
*/
HWTEST_F(OHOSApplicationFirstTest, IsMainProcess_0300, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsMainProcess_0300 start");
    auto info = std::make_shared<ProcessInfo>("testProcess", 10);
    ohosApplication_->SetProcessInfo(info);
    auto res = ohosApplication_->IsMainProcess("testBundleName", "testProcess");
    auto processInfo = ohosApplication_->GetProcessInfo();
    ProcessType processType = processInfo->GetProcessType();
    EXPECT_EQ(processType, ProcessType::NORMAL);
    EXPECT_TRUE(res);
    TAG_LOGI(AAFwkTag::TEST, "IsMainProcess_0300 end");
}

/*
* @tc.number: AppExecFwk_OHOSApplicationFirstTest_IsMainProcess_0400
* @tc.name: IsMainProcess
* @tc.desc: Test when none of the processInfo are met then IsMainProcess returns false.
*/
HWTEST_F(OHOSApplicationFirstTest, IsMainProcess_0400, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsMainProcess_0400 start");
    auto info = std::make_shared<ProcessInfo>("testProcess", 10);
    ohosApplication_->SetProcessInfo(info);
    auto res = ohosApplication_->IsMainProcess("testBundleName", "testProcess");
    auto processInfo = ohosApplication_->GetProcessInfo();
    EXPECT_TRUE(res);
    EXPECT_NE(processInfo, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "IsMainProcess_0400 end");
}

/*
* @tc.number: AppExecFwk_OHOSApplicationFirstTest_IsMainProcess_0500
* @tc.name: IsMainProcess
* @tc.desc: Test when processInfo is not nullptr then IsMainProcess returns.
*/
HWTEST_F(OHOSApplicationFirstTest, IsMainProcess_0500, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "IsMainProcess_0500 start");
    auto res = ohosApplication_->IsMainProcess("testBundleName", "testProcess");
    auto processInfo = ohosApplication_->GetProcessInfo();
    EXPECT_FALSE(res);
    EXPECT_EQ(processInfo, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "IsMainProcess_0500 end");
}

/*
* @tc.number: AppExecFwk_OHOSApplicationFirstTest_UpdateAppContextResMgr_0100
* @tc.name: UpdateAppContextResMgr
* @tc.desc: Test when context is nullptr then UpdateAppContextResMgr returns.
*/
HWTEST_F(OHOSApplicationFirstTest, UpdateAppContextResMgr_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateAppContextResMgr_0100 start");
    Configuration config;
    auto context = ohosApplication_->GetAppContext();
    ohosApplication_->UpdateAppContextResMgr(config);
    EXPECT_EQ(context, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "UpdateAppContextResMgr_0100 end");
}

/*
 * @tc.number: AppExecFwk_OHOSApplicationFirstTest_UpdateAppContextResMgr_0200
 * @tc.name: UpdateAppContextResMgr
 * @tc.desc: Test when context is not nullptr then UpdateAppContextResMgr updates the global config.
 */
HWTEST_F(OHOSApplicationFirstTest, UpdateAppContextResMgr_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateAppContextResMgr_0200 start");
    Configuration config;
    auto instance = AbilityRuntime::ApplicationContext::GetInstance();
    ohosApplication_->abilityRuntimeContext_ = instance;
    auto context = ohosApplication_->GetAppContext();
    ohosApplication_->UpdateAppContextResMgr(config);
    EXPECT_NE(context, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "UpdateAppContextResMgr_0200 end");
}

/*
* @tc.number: AppExecFwk_OHOSApplicationFirstTest_OnConfigurationUpdated_0100
* @tc.name: OnConfigurationUpdated
* @tc.desc: Verify function OnConfigurationUpdated pointer abilityRecordMgr_ empty
*/
HWTEST_F(OHOSApplicationFirstTest, OnConfigurationUpdated_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnConfigurationUpdated_0100 start");
    Configuration config;
    AbilityRuntime::SetLevel level = AbilityRuntime::SetLevel::System;
    ohosApplication_->OnConfigurationUpdated(config, level);
    EXPECT_TRUE(ohosApplication_->abilityRecordMgr_ == nullptr);
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    ohosApplication_->OnConfigurationUpdated(config, level);
    EXPECT_TRUE(ohosApplication_->configuration_ == nullptr);
    EXPECT_TRUE(ohosApplication_->abilityRuntimeContext_ == nullptr);
    TAG_LOGI(AAFwkTag::TEST, "OnConfigurationUpdated_0100 end");
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0200
* @tc.name: OnConfigurationUpdated
* @tc.desc: Verify function OnConfigurationUpdated pointer abilityRecord not empty
*/
HWTEST_F(OHOSApplicationFirstTest, AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0200 start.";
    Configuration config;
    AbilityRuntime::SetLevel level = AbilityRuntime::SetLevel::System;
    ohosApplication_->abilityRuntimeContext_ = AbilityRuntime::ApplicationContext::GetInstance();
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "zh_CN");
    config.AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP, "value");
    config.AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_SA, "value");
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    ohosApplication_->configuration_ = std::make_shared<Configuration>();
    ohosApplication_->configuration_->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "value");
    ohosApplication_->configuration_->AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP, "value");
    ohosApplication_->configuration_->AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_SA, "value");
    sptr<Notification::MockIRemoteObject> token = new (std::nothrow) Notification::MockIRemoteObject();
    std::shared_ptr<AbilityInfo> info =  nullptr;
    auto want = std::make_shared<Want>();
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(info, token, want, 0);
    ohosApplication_->abilityRecordMgr_->abilityRecords_.emplace(token, abilityRecord);
    sptr<AbilityThread> abilityThread = new (std::nothrow) AbilityRuntime::FAAbilityThread();
    abilityRecord->SetAbilityThread(abilityThread);
    ohosApplication_->OnConfigurationUpdated(config, level);
    EXPECT_TRUE(!ohosApplication_->abilityRecordMgr_->abilityRecords_.empty());
    EXPECT_TRUE(abilityRecord != nullptr);
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0200 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0300
* @tc.name: OnConfigurationUpdated
* @tc.desc: Verify function OnConfigurationUpdated map abilityStages_ not empty
*/
HWTEST_F(OHOSApplicationFirstTest, AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0300 start.";
    Configuration config;
    ohosApplication_->abilityRuntimeContext_ = AbilityRuntime::ApplicationContext::GetInstance();
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "zh_CN");
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "value");
    config.AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP, "value");
    config.AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_SA, "value");
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    ohosApplication_->configuration_ = std::make_shared<Configuration>();
    ohosApplication_->configuration_->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "value");
    ohosApplication_->configuration_->AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP, "value");
    ohosApplication_->configuration_->AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_SA, "value");
    EXPECT_TRUE(ohosApplication_->abilityStages_.empty());
    std::string moduleName = "entry";
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStages = std::make_shared<AbilityRuntime::AbilityStage>();
    ohosApplication_->abilityStages_.emplace(moduleName, abilityStages);
    ohosApplication_->OnConfigurationUpdated(config);
    EXPECT_TRUE(!ohosApplication_->abilityStages_.empty());
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0300 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0400
* @tc.name: OnConfigurationUpdated
* @tc.desc: Verify function OnConfigurationUpdated IsUpdateColorNeeded return true
*/
HWTEST_F(OHOSApplicationFirstTest, AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0400 start.";
    Configuration config;
    AbilityRuntime::SetLevel level = AbilityRuntime::SetLevel::Application;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE,
                   ConfigurationInner::SYSTEM_DEFAULT_FONTSIZE_SCALE);
    ohosApplication_->abilityRuntimeContext_ = AbilityRuntime::ApplicationContext::GetInstance();
    ohosApplication_->configuration_ = std::make_shared<Configuration>();
    ohosApplication_->configuration_->AddItem(AAFwk::GlobalConfigurationKey::APP_FONT_SIZE_SCALE,
                            ConfigurationInner::IS_APP_FONT_FOLLOW_SYSTEM);
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    std::string colorMode = "auto";
    ohosApplication_->configuration_->GetItem(AAFwk::GlobalConfigurationKey::APP_FONT_SIZE_SCALE);
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, colorMode);
    std::string colorMode1 = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    EXPECT_FALSE(colorMode1.empty());
    AbilityRuntime::ApplicationConfigurationManager::GetInstance().SetLanguageSetLevel(level);
    std::string testLanguge = "ch-zh";
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, testLanguge);
    std::string language = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_FALSE(language.empty());
    EXPECT_TRUE(ohosApplication_->IsUpdateColorNeeded(config, level));
    EXPECT_TRUE(ohosApplication_->isUpdateFontSize(config, level));
    EXPECT_TRUE(ohosApplication_->IsUpdateLanguageNeeded(config, level));
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0400 end.";
}

/*
* @tc.number: AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0500
* @tc.name: OnConfigurationUpdated
* @tc.desc: Verify function OnConfigurationUpdated IsUpdateColorNeeded return false
*/
HWTEST_F(OHOSApplicationFirstTest, AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0500 start.";
    Configuration config;
    AbilityRuntime::SetLevel level = AbilityRuntime::SetLevel::Application;
    ohosApplication_->abilityRuntimeContext_ = AbilityRuntime::ApplicationContext::GetInstance();
    ohosApplication_->configuration_ = std::make_shared<Configuration>();
    ohosApplication_->configuration_->AddItem(AAFwk::GlobalConfigurationKey::APP_FONT_SIZE_SCALE,
                            ConfigurationInner::IS_APP_FONT_FOLLOW_SYSTEM);
    ohosApplication_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    std::string colorMode = "light";
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, colorMode);
    auto colorMode1 = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    AbilityRuntime::ApplicationConfigurationManager::GetInstance().SetColorModeSetLevel(level, colorMode1);
    AbilityRuntime::SetLevel level1 = AbilityRuntime::SetLevel::System;
    std::string colorModeIsSetBySa =
        config.GetItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_SA);
    AbilityRuntime::ApplicationConfigurationManager::GetInstance().SetLanguageSetLevel(level);
    ohosApplication_->configuration_ = std::make_shared<Configuration>();
    std::string language = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    EXPECT_TRUE(language.empty());
    EXPECT_FALSE(ohosApplication_->IsUpdateColorNeeded(config, level1));
    EXPECT_FALSE(ohosApplication_->isUpdateFontSize(config, level1));
    EXPECT_FALSE(ohosApplication_->IsUpdateLanguageNeeded(config, level1));
    GTEST_LOG_(INFO) << "AppExecFwk_OHOSApplicationTest_OnConfigurationUpdated_0500 end.";
}

/*
* @tc.number: OnConfigurationUpdated_0600
* @tc.name: OnConfigurationUpdated
* @tc.desc: Function test abilityRuntimeContext_ not empty
*/
HWTEST_F(OHOSApplicationFirstTest, OnConfigurationUpdated_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnConfigurationUpdated_0600 start");
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
    ohosApplication_->configuration_->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "value");
    ohosApplication_->configuration_->AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP, "value");
    ohosApplication_->configuration_->AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_SA, "value");
    Configuration config;
    ohosApplication_->abilityRuntimeContext_ = AbilityRuntime::ApplicationContext::GetInstance();
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "zh_CN");
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "value");
    config.AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP, "value");
    config.AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP, "value");
    config.AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_SA, "value");
    config.AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_SA, "value");
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "zh");
    ohosApplication_->OnConfigurationUpdated(config);
    EXPECT_TRUE(config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE) == "zh");
    TAG_LOGI(AAFwkTag::TEST, "OnConfigurationUpdated_0600 end");
}

/**
 * @tc.number: ApplicationConfigurationManager_0100
 * @tc.name: SetColorModeSetLevel
 * @tc.desc: Verify auto color mode does not override and application flag is set.
 */
HWTEST_F(OHOSApplicationFirstTest, ApplicationConfigurationManager_0100, Function | MediumTest | Level1)
{
    auto &mgr = AbilityRuntime::ApplicationConfigurationManager::GetInstance();
    mgr.SetColorModeSetLevel(AbilityRuntime::SetLevel::System,
        AppExecFwk::ConfigurationInner::COLOR_MODE_LIGHT);
    mgr.SetColorModeSetLevel(AbilityRuntime::SetLevel::SA,
        AppExecFwk::ConfigurationInner::COLOR_MODE_AUTO);
    mgr.SetColorModeSetLevel(AbilityRuntime::SetLevel::Application,
        AppExecFwk::ConfigurationInner::COLOR_MODE_AUTO);
    auto mode = mgr.GetColorMode();
    EXPECT_EQ(mode, AppExecFwk::ConfigurationInner::COLOR_MODE_LIGHT);
    EXPECT_EQ(mgr.GetColorModeSetLevel(), AbilityRuntime::SetLevel::System);
    EXPECT_TRUE(mgr.ColorModeHasSetByApplication());
}

/**
 * @tc.number: ApplicationConfigurationManager_0200
 * @tc.name: SetColorModeSetLevel
 * @tc.desc: Verify higher non-auto level is selected.
 */
HWTEST_F(OHOSApplicationFirstTest, ApplicationConfigurationManager_0200, Function | MediumTest | Level1)
{
    auto &mgr = AbilityRuntime::ApplicationConfigurationManager::GetInstance();
    mgr.SetColorModeSetLevel(AbilityRuntime::SetLevel::Application,
        AppExecFwk::ConfigurationInner::COLOR_MODE_AUTO);
    auto mode = mgr.SetColorModeSetLevel(AbilityRuntime::SetLevel::SA,
        AppExecFwk::ConfigurationInner::COLOR_MODE_DARK);
    EXPECT_EQ(mode, AppExecFwk::ConfigurationInner::COLOR_MODE_DARK);
    EXPECT_EQ(mgr.GetColorModeSetLevel(), AbilityRuntime::SetLevel::SA);
    EXPECT_EQ(mgr.GetColorMode(), AppExecFwk::ConfigurationInner::COLOR_MODE_DARK);
}

/**
 * @tc.number: ApplicationConfigurationManager_0300
 * @tc.name: SetLanguageSetLevel
 * @tc.desc: Verify language and font levels can be set and retrieved.
 */
HWTEST_F(OHOSApplicationFirstTest, ApplicationConfigurationManager_0300, Function | MediumTest | Level1)
{
    auto &mgr = AbilityRuntime::ApplicationConfigurationManager::GetInstance();
    mgr.SetLanguageSetLevel(AbilityRuntime::SetLevel::Application);
    mgr.SetfontSetLevel(AbilityRuntime::SetLevel::SA);
    EXPECT_EQ(mgr.GetLanguageSetLevel(), AbilityRuntime::SetLevel::Application);
    EXPECT_EQ(mgr.GetFontSetLevel(), AbilityRuntime::SetLevel::SA);
}

/**
 * @tc.number: ApplicationConfigurationManager_0400
 * @tc.name: IgnoreContext
 * @tc.desc: Verify ignore context add and delete behavior.
 */
HWTEST_F(OHOSApplicationFirstTest, ApplicationConfigurationManager_0400, Function | MediumTest | Level1)
{
    auto &mgr = AbilityRuntime::ApplicationConfigurationManager::GetInstance();
    std::shared_ptr<AbilityRuntime::Context> ctx;
    std::shared_ptr<Global::Resource::ResourceManager> rm;
    mgr.AddIgnoreContext(ctx, rm);
    auto contexts = mgr.GetIgnoreContext();
    auto resources = mgr.GetIgnoreResource();
    ASSERT_EQ(contexts.size(), 1u);
    ASSERT_EQ(resources.size(), 1u);
    EXPECT_EQ(contexts[0], ctx);
    EXPECT_EQ(resources[0], rm);
    mgr.DeleteIgnoreContext(ctx);
    EXPECT_TRUE(mgr.GetIgnoreContext().empty());
}

/**
 * @tc.number: ApplicationConfigurationManager_0500
 * @tc.name: GetUpdatedLocale
 * @tc.desc: Verify empty or invalid inputs return empty.
 */
HWTEST_F(OHOSApplicationFirstTest, ApplicationConfigurationManager_0500, Function | MediumTest | Level1)
{
    EXPECT_TRUE(AbilityRuntime::ApplicationConfigurationManager::GetUpdatedLocale("", "en").empty());
    EXPECT_TRUE(AbilityRuntime::ApplicationConfigurationManager::GetUpdatedLocale("en-US", "").empty());
    EXPECT_TRUE(AbilityRuntime::ApplicationConfigurationManager::GetUpdatedLocale("invalid_tag", "en").empty());
}

/**
 * @tc.number: ApplicationConfigurationManager_0600
 * @tc.name: GetUpdatedLocale
 * @tc.desc: Verify locale composition with script, region and extension.
 */
HWTEST_F(OHOSApplicationFirstTest, ApplicationConfigurationManager_0600, Function | MediumTest | Level1)
{
    std::string locale = "en-US-u-ca-buddhist";
    std::string language = "zh-Hans";
    auto updated = AbilityRuntime::ApplicationConfigurationManager::GetUpdatedLocale(locale, language);
    EXPECT_EQ(updated, "zh-Hans-US-u-ca-buddhist");
}
}  // namespace AppExecFwk
}  // namespace OHOS
