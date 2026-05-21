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

#define private public
#define protected public
#include "app_preloader.h"
#undef protected
#undef private
#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppPreloaderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();

protected:
    std::shared_ptr<RemoteClientManager> remoteClientManager_ = nullptr;
};

void AppPreloaderTest::SetUp()
{
    remoteClientManager_ = std::make_shared<RemoteClientManager>();
    auto bundleMgrHelper = std::make_shared<BundleMgrHelper>();
    remoteClientManager_->SetBundleManagerHelper(bundleMgrHelper);
}

void AppPreloaderTest::TearDown()
{}

/**
 * @tc.number: AppPreloaderTest_GeneratePreloadRequest_0100
 * @tc.desc: Test GeneratePreloadRequest works
 * @tc.type: FUNC
 * @tc.Function: GeneratePreloadRequest
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GeneratePreloadRequest_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_GeneratePreloadRequest_0100 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = 100;
    int32_t appIndex = 0;
    PreloadRequest request;
    auto ret = manager->GeneratePreloadRequest(bundleName, userId, appIndex, request);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: AppPreloaderTest_GeneratePreloadRequest_0200
 * @tc.desc: Test GeneratePreloadRequest works
 * @tc.type: FUNC
 * @tc.Function: GeneratePreloadRequest
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GeneratePreloadRequest_0200, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_GeneratePreloadRequest_0200 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = 1;
    int32_t appIndex = 0;
    PreloadRequest request;
    auto ret = manager->GeneratePreloadRequest(bundleName, userId, appIndex, request);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: AppPreloaderTest_GeneratePreloadRequest_0300
 * @tc.desc: Test GeneratePreloadRequest works
 * @tc.type: FUNC
 * @tc.Function: GeneratePreloadRequest
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GeneratePreloadRequest_0300, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_GeneratePreloadRequest_0300 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = 0;
    int32_t appIndex = 0;
    PreloadRequest request;
    auto ret = manager->GeneratePreloadRequest(bundleName, userId, appIndex, request);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: AppPreloaderTest_GeneratePreloadRequest_0400
 * @tc.desc: Test GeneratePreloadRequest works
 * @tc.type: FUNC
 * @tc.Function: GeneratePreloadRequest
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GeneratePreloadRequest_0400, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_GeneratePreloadRequest_0400 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = -1;
    int32_t appIndex = 0;
    PreloadRequest request;
    auto ret = manager->GeneratePreloadRequest(bundleName, userId, appIndex, request);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: AppPreloaderTest_GeneratePreloadRequest_0500
 * @tc.desc: Test GeneratePreloadRequest works
 * @tc.type: FUNC
 * @tc.Function: GeneratePreloadRequest
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GeneratePreloadRequest_0500, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_GeneratePreloadRequest_0500 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = 100;
    int32_t appIndex = 1;
    PreloadRequest request;
    auto ret = manager->GeneratePreloadRequest(bundleName, userId, appIndex, request);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: AppPreloaderTest_GeneratePreloadRequest_0500
 * @tc.desc: Test GeneratePreloadRequest works
 * @tc.type: FUNC
 * @tc.Function: GeneratePreloadRequest
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GeneratePreloadRequest_0600, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_GeneratePreloadRequest_0600 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = 1;
    int32_t appIndex = 1;
    PreloadRequest request;
    auto ret = manager->GeneratePreloadRequest(bundleName, userId, appIndex, request);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: AppPreloaderTest_GeneratePreloadRequest_0700
 * @tc.desc: Test GeneratePreloadRequest works
 * @tc.type: FUNC
 * @tc.Function: GeneratePreloadRequest
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GeneratePreloadRequest_0700, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_GeneratePreloadRequest_0700 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = 0;
    int32_t appIndex = 1;
    PreloadRequest request;
    auto ret = manager->GeneratePreloadRequest(bundleName, userId, appIndex, request);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: AppPreloaderTest_GeneratePreloadRequest_0800
 * @tc.desc: Test GeneratePreloadRequest works
 * @tc.type: FUNC
 * @tc.Function: GeneratePreloadRequest
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GeneratePreloadRequest_0800, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_GeneratePreloadRequest_0800 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = 0;
    int32_t appIndex = -1;
    PreloadRequest request;
    auto ret = manager->GeneratePreloadRequest(bundleName, userId, appIndex, request);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: AppPreloaderTest_GeneratePreloadRequest_0900
 * @tc.desc: Test GeneratePreloadRequest works
 * @tc.type: FUNC
 * @tc.Function: GeneratePreloadRequest
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GeneratePreloadRequest_0900, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppPreloaderTest_GeneratePreloadRequest_0900 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = UidCheckCode::UID_CHECK_NOT_ENTRY_HAP_MODULE_INFO;
    int32_t appIndex = 0;
    PreloadRequest request;
    request.preloadMode = PreloadMode::PRELOAD_BY_PHASE;
    auto ret = manager->GeneratePreloadRequest(bundleName, userId, appIndex, request);
    EXPECT_EQ(ret, AAFwk::ERR_PRELOAD_NOT_ENTRY_MODULE);
    TAG_LOGI(AAFwkTag::TEST, "AppPreloaderTest_GeneratePreloadRequest_0900 ned.");
}

/**
 * @tc.number: AppPreloaderTest_GeneratePreloadRequest_1000
 * @tc.desc: Test GeneratePreloadRequest works
 * @tc.type: FUNC
 * @tc.Function: GeneratePreloadRequest
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GeneratePreloadRequest_1000, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppPreloaderTest_GeneratePreloadRequest_1000 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = 100;
    int32_t appIndex = 0;
    PreloadRequest request;
    request.preloadMode = PreloadMode::PRELOAD_BY_PHASE;
    auto ret = manager->GeneratePreloadRequest(bundleName, userId, appIndex, request);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppPreloaderTest_GeneratePreloadRequest_1000 ned.");
}

/**
 * @tc.number: AppPreloaderTest_PreCheck_0100
 * @tc.desc: Test PreCheck works
 * @tc.type: FUNC
 * @tc.Function: GeneratePreloadRequest
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_PreCheck_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_PreCheck_0100 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.acts.testxxxx";
    auto ret = manager->PreCheck(bundleName, PreloadMode::PRE_MAKE);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.number: AppPreloaderTest_PreCheck_0200
 * @tc.desc: Test PreCheck works
 * @tc.type: FUNC
 * @tc.Function: GeneratePreloadRequest
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_PreCheck_0200, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_PreCheck_0200 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "";
    auto ret = manager->PreCheck(bundleName, PreloadMode::PRE_MAKE);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.number: AppPreloaderTest_PreCheck_0300
 * @tc.desc: Test PreCheck works
 * @tc.type: FUNC
 * @tc.Function: GeneratePreloadRequest
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_PreCheck_0300, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_PreCheck_0300 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.acts.testxxxx";
    auto ret = manager->PreCheck(bundleName, PreloadMode::PRESS_DOWN);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.number: AppPreloaderTest_PreCheck_0400
 * @tc.desc: Test PreCheck works
 * @tc.type: FUNC
 * @tc.Function: GeneratePreloadRequest
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_PreCheck_0400, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_PreCheck_0400 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.acts.testxxxx";
    auto ret = manager->PreCheck(bundleName, PreloadMode::PRESS_DOWN);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.number: AppPreloaderTest_PreCheck_0500
 * @tc.desc: Test PreCheck works
 * @tc.type: FUNC
 * @tc.Function: GeneratePreloadRequest
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_PreCheck_0500, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppPreloaderTest_PreCheck_0500 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.acts.testxxxx";
    auto ret = manager->PreCheck(bundleName, PreloadMode::PRELOAD_BY_PHASE);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "AppPreloaderTest_PreCheck_0500 end.");
}

/**
 * @tc.number: AppPreloaderTest_GetLaunchWant_0100
 * @tc.desc: Test GetLaunchWant works
 * @tc.type: FUNC
 * @tc.Function: GetLaunchWant
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GetLaunchWant_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_GetLaunchWant_0100 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = -2;
    int32_t appIndex = 0;
    PreloadRequest request;
    auto ret = manager->GeneratePreloadRequest(bundleName, userId, appIndex, request);
    EXPECT_EQ(ret, AAFwk::ERR_TARGET_BUNDLE_NOT_EXIST);
}

/**
 * @tc.number: AppPreloaderTest_GetLaunchWant_0200
 * @tc.desc: Test GetLaunchWant works
 * @tc.type: FUNC
 * @tc.Function: GetLaunchWant
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GetLaunchWant_0200, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_GetLaunchWant_0200 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    manager->remoteClientManager_ = nullptr;
    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = -2;
    Want want;
    auto ret = manager->GetLaunchWant(bundleName, userId, want);
    manager->remoteClientManager_ = remoteClientManager_;
    EXPECT_FALSE(ret);
}

 /**
 * @tc.number: AppPreloaderTest_GetLaunchAbilityInfo_0100
 * @tc.desc: Test GetLaunchAbilityInfo works
 * @tc.type: FUNC
 * @tc.Function: GetLaunchAbilityInfo
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GetLaunchAbilityInfo_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_GetLaunchAbilityInfo_0100 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    manager->remoteClientManager_ = nullptr;
    Want want;
    int32_t userId = -2;
    AbilityInfo abilityInfo;
    auto ret = manager->GetLaunchAbilityInfo(want, userId, abilityInfo);
    manager->remoteClientManager_ = remoteClientManager_;
    EXPECT_FALSE(ret);
}

 /**
 * @tc.number: AppPreloaderTest_GetLaunchAbilityInfo_0200
 * @tc.desc: Test GetLaunchAbilityInfo works
 * @tc.type: FUNC
 * @tc.Function: GetLaunchAbilityInfo
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GetLaunchAbilityInfo_0200, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_GetLaunchAbilityInfo_0200 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = -3;
    int32_t appIndex = 0;
    PreloadRequest request;
    auto ret = manager->GeneratePreloadRequest(bundleName, userId, appIndex, request);
    EXPECT_EQ(ret, AAFwk::ERR_GET_LAUNCH_ABILITY_INFO_FAILED);
}

/**
 * @tc.number: AppPreloaderTest_CheckPreloadConditions_0100
 * @tc.desc: Test CheckPreloadConditions works
 * @tc.type: FUNC
 * @tc.Function: CheckPreloadConditions
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_CheckPreloadConditions_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_CheckPreloadConditions_0100 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = -5;
    int32_t appIndex = 0;
    PreloadRequest request;
    auto ret = manager->GeneratePreloadRequest(bundleName, userId, appIndex, request);
    EXPECT_EQ(ret, AAFwk::ERR_CHECK_PRELOAD_CONDITIONS_FAILED);
}

/**
 * @tc.number: AppPreloaderTest_CheckPreloadConditions_0200
 * @tc.desc: Test CheckPreloadConditions works
 * @tc.type: FUNC
 * @tc.Function: CheckPreloadConditions
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_CheckPreloadConditions_0200, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_CheckPreloadConditions_0200 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = -6;
    int32_t appIndex = 0;
    PreloadRequest request;
    auto ret = manager->GeneratePreloadRequest(bundleName, userId, appIndex, request);
    EXPECT_EQ(ret, AAFwk::ERR_CHECK_PRELOAD_CONDITIONS_FAILED);
}

/**
 * @tc.number: AppPreloaderTest_CheckPreloadConditions_0300
 * @tc.desc: Test CheckPreloadConditions works
 * @tc.type: FUNC
 * @tc.Function: CheckPreloadConditions
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_CheckPreloadConditions_0300, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_CheckPreloadConditions_0300 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = -7;
    int32_t appIndex = 0;
    PreloadRequest request;
    auto ret = manager->GeneratePreloadRequest(bundleName, userId, appIndex, request);
    EXPECT_EQ(ret, AAFwk::ERR_CHECK_PRELOAD_CONDITIONS_FAILED);
}
 
 /**
  * @tc.number: AppPreloaderTest_GetBundleAndHapInfo_0100
  * @tc.desc: Test GetBundleAndHapInfo works
  * @tc.type: FUNC
  * @tc.Function: GetBundleAndHapInfo
  * @tc.SubFunction: NA
  * @tc.EnvConditions: NA
  */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GetBundleAndHapInfo_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_GetBundleAndHapInfo_0100 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = -9;
    int32_t appIndex = 0;
    PreloadRequest request;
    auto ret = manager->GeneratePreloadRequest(bundleName, userId, appIndex, request);
    EXPECT_EQ(ret, AAFwk::GET_BUNDLE_INFO_FAILED);
}
 
 /**
  * @tc.number: AppPreloaderTest_GetBundleAndHapInfo_0200
  * @tc.desc: Test GetBundleAndHapInfo works
  * @tc.type: FUNC
  * @tc.Function: GetBundleAndHapInfo
  * @tc.SubFunction: NA
  * @tc.EnvConditions: NA
  */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GetBundleAndHapInfo_0200, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_GetBundleAndHapInfo_0200 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = -10;
    int32_t appIndex = 0;
    PreloadRequest request;
    auto ret = manager->GeneratePreloadRequest(bundleName, userId, appIndex, request);
    EXPECT_EQ(ret, AAFwk::GET_BUNDLE_INFO_FAILED);
}

 /**
 * @tc.number: AppPreloaderTest_GetBundleAndHapInfo_0300
 * @tc.desc: Test GetBundleAndHapInfo works
 * @tc.type: FUNC
 * @tc.Function: GetBundleAndHapInfo
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GetBundleAndHapInfo_0300, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_GetBundleAndHapInfo_0300 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    manager->remoteClientManager_ = nullptr;
    std::string bundleName = "com.acts.preloadtest";
    int32_t userId = -2;
    AbilityInfo abilityInfo;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto ret = manager->GetBundleAndHapInfo(bundleName, userId, abilityInfo, bundleInfo, hapModuleInfo);
    manager->remoteClientManager_ = remoteClientManager_;
    EXPECT_FALSE(ret);
}

 /**
  * @tc.number: AppPreloaderTest_GetBundleManagerHelper_0100
  * @tc.desc: Test GetBundleManagerHelper works
  * @tc.type: FUNC
  * @tc.Function: GetBundleManagerHelper
  * @tc.SubFunction: NA
  * @tc.EnvConditions: NA
  */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GetBundleManagerHelper_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_GetBundleManagerHelper_0100 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);
    manager->remoteClientManager_ = nullptr;
    auto ret = manager->GetBundleManagerHelper();
    manager->remoteClientManager_ = remoteClientManager_;
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: AppPreloaderTest_GeneratePreloadExtensionRequest_0100
 * @tc.desc: Test GeneratePreloadExtensionRequest works
 * @tc.type: FUNC
 * @tc.Function: GeneratePreloadExtensionRequest
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GeneratePreloadExtensionRequest_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "AppPreloaderTest_GeneratePreloadExtensionRequest_0100 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.example.hmos.inputmethod";
    std::string abilityName = "InputService";
    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);

    int32_t userId = -9; // failed GetBundleAndHapInfo
    int32_t appIndex = 0;
    PreloadRequest request;

    auto ret = manager->GeneratePreloadExtensionRequest(want, userId, appIndex, request);
    EXPECT_EQ(ret, AAFwk::GET_BUNDLE_INFO_FAILED);
}

/**
 * @tc.number: AppPreloaderTest_GeneratePreloadExtensionRequest_0200
 * @tc.desc: Test GeneratePreloadExtensionRequest with appIndex != 0
 * @tc.type: FUNC
 * @tc.Function: GeneratePreloadExtensionRequest
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GeneratePreloadExtensionRequest_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppPreloaderTest_GeneratePreloadExtensionRequest_0200 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.example.hmos.inputmethod";
    std::string abilityName = "InputService";
    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);

    int32_t userId = 1;
    int32_t appIndex = 1;
    PreloadRequest request;

    auto ret = manager->GeneratePreloadExtensionRequest(want, userId, appIndex, request);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppPreloaderTest_GeneratePreloadExtensionRequest_0200 end.");
}

/**
 * @tc.number: AppPreloaderTest_GeneratePreloadExtensionRequest_0300
 * @tc.desc: Test GeneratePreloadExtensionRequest works
 * @tc.type: FUNC
 * @tc.Function: GeneratePreloadExtensionRequest
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GeneratePreloadExtensionRequest_0300, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppPreloaderTest_GeneratePreloadExtensionRequest_0300 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.example.hmos.inputmethod";
    std::string abilityName = "InputService";
    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);

    int32_t userId = 1;
    int32_t appIndex = 0;
    PreloadRequest request;

    auto ret = manager->GeneratePreloadExtensionRequest(want, userId, appIndex, request);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppPreloaderTest_GeneratePreloadExtensionRequest_0300 end.");
}


/**
 * @tc.number: AppPreloaderTest_GeneratePreloadExtensionRequest_0400
 * @tc.desc: Test GeneratePreloadExtensionRequest with null remoteClientManager
 * @tc.type: FUNC
 * @tc.Function: GeneratePreloadExtensionRequest
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GeneratePreloadExtensionRequest_0400, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppPreloaderTest_GeneratePreloadExtensionRequest_0400 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.example.hmos.inputmethod";
    std::string abilityName = "InputService";
    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);

    manager->remoteClientManager_ = nullptr;
    int32_t userId = 1;
    int32_t appIndex = 0;
    PreloadRequest request;

    auto ret = manager->GeneratePreloadExtensionRequest(want, userId, appIndex, request);
    manager->remoteClientManager_ = remoteClientManager_;
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AppPreloaderTest_GeneratePreloadExtensionRequest_0400 end.");
}

/**
 * @tc.number: AppPreloaderTest_GetAbilityInfo_0100
 * @tc.desc: Test GetAbilityInfo works correctly
 * @tc.type: FUNC
 * @tc.Function: GetAbilityInfo
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppPreloaderTest, AppPreloaderTest_GetAbilityInfo_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "AppPreloaderTest_GetAbilityInfo_0100 start.");
    auto manager = std::make_shared<AppPreloader>(remoteClientManager_);
    EXPECT_NE(manager, nullptr);

    std::string bundleName = "com.example.hmos.inputmethod";
    std::string abilityName = "InputService";
    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);

    int32_t userId = 1;
    int32_t appIndex = 0;
    PreloadRequest request;
    AbilityInfo abilityInfo;

    auto ret = manager->GetAbilityInfo(want, userId, appIndex, request, abilityInfo);
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppPreloaderTest_GetAbilityInfo_0100 end.");
}
}  // namespace AppExecFwk
}  // namespace OHOS
