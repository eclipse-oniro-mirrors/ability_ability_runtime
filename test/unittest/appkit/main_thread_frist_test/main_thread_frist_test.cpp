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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#define private public
#include "main_thread.h"
#undef private
#include "hilog_tag_wrapper.h"
#include "ability_local_record.h"
#include "app_launch_data.h"
#include "application_info.h"
#include "mock_ability_token.h"
#include "overlay_module_info.h"
#include "plugin/plugin_bundle_info.h"
#include "process_info.h"
#include "profile.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class MainThreadFristTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    // Use raw pointer to avoid automatic sptr destruction which would trigger SIGILL
    static MainThread* sharedThread_;
};

MainThread* MainThreadFristTest::sharedThread_ = nullptr;

void MainThreadFristTest::SetUpTestCase()
{
    sharedThread_ = new (std::nothrow) MainThread();
    ASSERT_NE(sharedThread_, nullptr);
}

void MainThreadFristTest::TearDownTestCase()
{
    // Do NOT delete sharedThread_ - heap corruption occurs during MainThread destruction
}

void MainThreadFristTest::SetUp()
{}

void MainThreadFristTest::TearDown()
{}

/**
 * @tc.name: SmokeTest_0100
 * @tc.desc: Verify test framework works.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, SmokeTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_TRUE(true);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetMainThreadState_0100
 * @tc.desc: Test GetMainThreadState returns INIT after construction.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, GetMainThreadState_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    EXPECT_EQ(sharedThread_->GetMainThreadState(), MainThreadState::INIT);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: SetRunnerStarted_0100
 * @tc.desc: Test SetRunnerStarted works.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, SetRunnerStarted_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    sharedThread_->SetRunnerStarted(true);
    EXPECT_TRUE(sharedThread_->isRunnerStarted_);
    sharedThread_->SetRunnerStarted(false);
    EXPECT_FALSE(sharedThread_->isRunnerStarted_);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetRunnerStarted_0100
 * @tc.desc: Test GetRunnerStarted returns correct value.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, GetRunnerStarted_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    EXPECT_FALSE(sharedThread_->GetRunnerStarted());
    sharedThread_->SetRunnerStarted(true);
    EXPECT_TRUE(sharedThread_->GetRunnerStarted());
    sharedThread_->SetRunnerStarted(false);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetNewThreadId_0100
 * @tc.desc: Test GetNewThreadId increments from initial value -1.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, GetNewThreadId_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    // newThreadId_ starts at -1, GetNewThreadId returns post-increment
    int firstId = sharedThread_->GetNewThreadId();
    EXPECT_EQ(firstId, -1);
    int secondId = sharedThread_->GetNewThreadId();
    EXPECT_EQ(secondId, 0);
    int thirdId = sharedThread_->GetNewThreadId();
    EXPECT_EQ(thirdId, 1);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetApplication_0100
 * @tc.desc: Test GetApplication returns nullptr before launch.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, GetApplication_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    EXPECT_EQ(sharedThread_->GetApplication(), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetApplicationInfo_0100
 * @tc.desc: Test GetApplicationInfo returns nullptr before launch.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, GetApplicationInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    EXPECT_EQ(sharedThread_->GetApplicationInfo(), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetApplicationImpl_0100
 * @tc.desc: Test GetApplicationImpl returns nullptr before launch.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, GetApplicationImpl_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    EXPECT_EQ(sharedThread_->GetApplicationImpl(), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetMainHandler_0100
 * @tc.desc: Test GetMainHandler returns nullptr before Init.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, GetMainHandler_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    EXPECT_EQ(sharedThread_->GetMainHandler(), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: RemoveAppMgrDeathRecipient_0100
 * @tc.desc: Test RemoveAppMgrDeathRecipient with null appMgr_.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, RemoveAppMgrDeathRecipient_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    sharedThread_->RemoveAppMgrDeathRecipient();
    EXPECT_EQ(sharedThread_->appMgr_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ScheduleClearPageStack_0100
 * @tc.desc: Test ScheduleClearPageStack with null applicationInfo_.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, ScheduleClearPageStack_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    sharedThread_->ScheduleClearPageStack();
    EXPECT_EQ(sharedThread_->applicationInfo_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ScheduleLowMemory_0100
 * @tc.desc: Test ScheduleLowMemory does nothing.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, ScheduleLowMemory_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    sharedThread_->ScheduleLowMemory();
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: IsBgWorkingThread_0100
 * @tc.desc: Test IsBgWorkingThread returns true for BACKUP type.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, IsBgWorkingThread_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    AbilityInfo backupInfo;
    backupInfo.extensionAbilityType = ExtensionAbilityType::BACKUP;
    EXPECT_TRUE(sharedThread_->IsBgWorkingThread(backupInfo));
    AbilityInfo normalInfo;
    normalInfo.extensionAbilityType = ExtensionAbilityType::UNSPECIFIED;
    EXPECT_FALSE(sharedThread_->IsBgWorkingThread(normalInfo));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ScheduleProfileChanged_0100
 * @tc.desc: Test ScheduleProfileChanged logs profile name.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, ScheduleProfileChanged_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    Profile profile("test_profile");
    sharedThread_->ScheduleProfileChanged(profile);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckLaunchApplicationParam_0100
 * @tc.desc: Test CheckLaunchApplicationParam returns false when name empty.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, CheckLaunchApplicationParam_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    AppLaunchData data;
    EXPECT_FALSE(sharedThread_->CheckLaunchApplicationParam(data));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckLaunchApplicationParam_0200
 * @tc.desc: Test CheckLaunchApplicationParam returns false when processName empty.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, CheckLaunchApplicationParam_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    AppLaunchData data;
    ApplicationInfo appInfo;
    appInfo.name = "com.test.app";
    data.SetApplicationInfo(appInfo);
    EXPECT_FALSE(sharedThread_->CheckLaunchApplicationParam(data));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckLaunchApplicationParam_0300
 * @tc.desc: Test CheckLaunchApplicationParam returns true when both name and processName set.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, CheckLaunchApplicationParam_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    AppLaunchData data;
    ApplicationInfo appInfo;
    appInfo.name = "com.test.app";
    data.SetApplicationInfo(appInfo);
    ProcessInfo processInfo("com.test.process", 0);
    data.SetProcessInfo(processInfo);
    EXPECT_TRUE(sharedThread_->CheckLaunchApplicationParam(data));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckAbilityItem_0100
 * @tc.desc: Test CheckAbilityItem returns false when record is null.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, CheckAbilityItem_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    EXPECT_FALSE(sharedThread_->CheckAbilityItem(nullptr));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckAbilityItem_0200
 * @tc.desc: Test CheckAbilityItem returns false when abilityInfo is null.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, CheckAbilityItem_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr;
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, want, 0);
    EXPECT_FALSE(sharedThread_->CheckAbilityItem(record));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckAbilityItem_0300
 * @tc.desc: Test CheckAbilityItem returns false when token is null.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, CheckAbilityItem_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = nullptr;
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, want, 0);
    EXPECT_FALSE(sharedThread_->CheckAbilityItem(record));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckAbilityItem_0400
 * @tc.desc: Test CheckAbilityItem returns true when record is legal.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, CheckAbilityItem_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, want, 0);
    EXPECT_TRUE(sharedThread_->CheckAbilityItem(record));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: IsApplicationReady_0100
 * @tc.desc: Test IsApplicationReady returns false before launch.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, IsApplicationReady_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    EXPECT_FALSE(sharedThread_->IsApplicationReady());
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ScanDir_0100
 * @tc.desc: Test ScanDir returns false when dir does not exist.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, ScanDir_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    std::vector<std::string> files;
    EXPECT_FALSE(sharedThread_->ScanDir("/nonexistent_dir_path_for_test", files));
    EXPECT_TRUE(files.empty());
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ScanDir_0200
 * @tc.desc: Test ScanDir returns true for existing dir.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, ScanDir_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    std::vector<std::string> files;
    EXPECT_TRUE(sharedThread_->ScanDir("/tmp", files));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckFileType_0100
 * @tc.desc: Test CheckFileType returns false for empty filename.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, CheckFileType_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    EXPECT_FALSE(sharedThread_->CheckFileType("", ".so"));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckFileType_0200
 * @tc.desc: Test CheckFileType returns false when no extension.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, CheckFileType_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    EXPECT_FALSE(sharedThread_->CheckFileType("filename_no_ext", ".so"));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckFileType_0300
 * @tc.desc: Test CheckFileType returns true when extension matches.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, CheckFileType_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    EXPECT_TRUE(sharedThread_->CheckFileType("libtest.so", ".so"));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckFileType_0400
 * @tc.desc: Test CheckFileType is case-insensitive.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, CheckFileType_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    EXPECT_TRUE(sharedThread_->CheckFileType("libtest.SO", ".so"));
    EXPECT_FALSE(sharedThread_->CheckFileType("libtest.txt", ".so"));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: CheckMainThreadIsAlive_0100
 * @tc.desc: Test CheckMainThreadIsAlive with null watchdog.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, CheckMainThreadIsAlive_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    sharedThread_->CheckMainThreadIsAlive();
    EXPECT_EQ(sharedThread_->watchdog_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: NotifyDeviceDisConnect_0100
 * @tc.desc: Test NotifyDeviceDisConnect returns false with null appMgr_.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, NotifyDeviceDisConnect_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    EXPECT_FALSE(sharedThread_->NotifyDeviceDisConnect());
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: SetWatchdogBackgroundStatus_0100
 * @tc.desc: Test SetWatchdogBackgroundStatus with null watchdog.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, SetWatchdogBackgroundStatus_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    sharedThread_->SetWatchdogBackgroundStatus(true);
    sharedThread_->SetWatchdogBackgroundStatus(false);
    EXPECT_EQ(sharedThread_->watchdog_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ScheduleChangeAppGcState_0100
 * @tc.desc: Test ScheduleChangeAppGcState returns error when mainHandler null.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, ScheduleChangeAppGcState_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    EXPECT_EQ(sharedThread_->ScheduleChangeAppGcState(0), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: ChangeAppGcState_0100
 * @tc.desc: Test ChangeAppGcState returns error when application null.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, ChangeAppGcState_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    EXPECT_EQ(sharedThread_->ChangeAppGcState(0), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: OnAttachLocalDebug_0100
 * @tc.desc: Test OnAttachLocalDebug returns error when application null.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, OnAttachLocalDebug_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    ASSERT_NE(sharedThread_, nullptr);
    EXPECT_EQ(sharedThread_->OnAttachLocalDebug(true), ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

// Forward declarations of free functions defined in main_thread.cpp (external linkage,
// compiled into appkit_native). Used to unit-test the native-library-load decision logic
// introduced by the "native application 整改" commit without constructing a full AppLaunchData.
bool IsNeedLoadLibrary(const std::vector<Metadata> &metaData, const bool isSystemApp);
bool IsFormRenderService(const std::string &bundleName);

/**
 * @tc.name: IsNeedLoadLibrary_EmptyMetadata_ReturnsFalse_0100
 * @tc.desc: Empty metadata vector never matches -> false regardless of isSystemApp.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, IsNeedLoadLibrary_EmptyMetadata_ReturnsFalse_0100, TestSize.Level1)
{
    std::vector<Metadata> metaData;
    EXPECT_FALSE(IsNeedLoadLibrary(metaData, true));
    EXPECT_FALSE(IsNeedLoadLibrary(metaData, false));
}

/**
 * @tc.name: IsNeedLoadLibrary_HitMetadataAndSystemApp_ReturnsTrue_0100
 * @tc.desc: metadata declares loadNativeLibraryForApplication=true && isSystemApp -> true.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, IsNeedLoadLibrary_HitMetadataAndSystemApp_ReturnsTrue_0100, TestSize.Level1)
{
    std::vector<Metadata> metaData = {
        Metadata("ohos.ability.loadNativeLibraryForApplication", "true", ""),
    };
    EXPECT_TRUE(IsNeedLoadLibrary(metaData, true));
}

/**
 * @tc.name: IsNeedLoadLibrary_HitMetadataButNotSystemApp_ReturnsFalse_0100
 * @tc.desc: metadata hits but isSystemApp=false -> false (security gate holds).
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, IsNeedLoadLibrary_HitMetadataButNotSystemApp_ReturnsFalse_0100, TestSize.Level1)
{
    std::vector<Metadata> metaData = {
        Metadata("ohos.ability.loadNativeLibraryForApplication", "true", ""),
    };
    EXPECT_FALSE(IsNeedLoadLibrary(metaData, false));
}

/**
 * @tc.name: IsNeedLoadLibrary_NoHitMetadataAndSystemApp_ReturnsFalse_0100
 * @tc.desc: metadata name mismatch -> false even for system app.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, IsNeedLoadLibrary_NoHitMetadataAndSystemApp_ReturnsFalse_0100, TestSize.Level1)
{
    std::vector<Metadata> metaData = {
        Metadata("ohos.other.metadata", "true", ""),
    };
    EXPECT_FALSE(IsNeedLoadLibrary(metaData, true));
}

/**
 * @tc.name: IsNeedLoadLibrary_MetadataValueNotTrue_ReturnsFalse_0100
 * @tc.desc: metadata name hits but value != "true" -> false.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, IsNeedLoadLibrary_MetadataValueNotTrue_ReturnsFalse_0100, TestSize.Level1)
{
    std::vector<Metadata> metaDataFalse = {
        Metadata("ohos.ability.loadNativeLibraryForApplication", "false", ""),
    };
    EXPECT_FALSE(IsNeedLoadLibrary(metaDataFalse, true));
    std::vector<Metadata> metaDataOne = {
        Metadata("ohos.ability.loadNativeLibraryForApplication", "1", ""),
    };
    EXPECT_FALSE(IsNeedLoadLibrary(metaDataOne, true));
    std::vector<Metadata> metaDataEmpty = {
        Metadata("ohos.ability.loadNativeLibraryForApplication", "", ""),
    };
    EXPECT_FALSE(IsNeedLoadLibrary(metaDataEmpty, true));
}

/**
 * @tc.name: IsNeedLoadLibrary_MultipleMetadataOneHit_ReturnsTrue_0100
 * @tc.desc: any_of semantics: one matching item among many -> true.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, IsNeedLoadLibrary_MultipleMetadataOneHit_ReturnsTrue_0100, TestSize.Level1)
{
    std::vector<Metadata> metaData = {
        Metadata("ohos.other.metadata", "value", ""),
        Metadata("ohos.ability.loadNativeLibraryForApplication", "true", ""),
        Metadata("ohos.third.metadata", "false", ""),
    };
    EXPECT_TRUE(IsNeedLoadLibrary(metaData, true));
}

/**
 * @tc.name: IsNeedLoadLibrary_MultipleMetadataNoneHit_ReturnsFalse_0100
 * @tc.desc: any_of semantics: no matching item among many -> false.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, IsNeedLoadLibrary_MultipleMetadataNoneHit_ReturnsFalse_0100, TestSize.Level1)
{
    std::vector<Metadata> metaData = {
        Metadata("ohos.first.metadata", "true", ""),
        Metadata("ohos.second.metadata", "true", ""),
    };
    EXPECT_FALSE(IsNeedLoadLibrary(metaData, true));
}

/**
 * @tc.name: IsFormRenderService_Match_ReturnsTrue_0100
 * @tc.desc: bundleName equals com.ohos.formrenderservice -> true.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, IsFormRenderService_Match_ReturnsTrue_0100, TestSize.Level1)
{
    EXPECT_TRUE(IsFormRenderService("com.ohos.formrenderservice"));
}

/**
 * @tc.name: IsFormRenderService_NoMatch_ReturnsFalse_0100
 * @tc.desc: non-matching bundleName -> false.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, IsFormRenderService_NoMatch_ReturnsFalse_0100, TestSize.Level1)
{
    EXPECT_FALSE(IsFormRenderService("com.example.otherbundle"));
    EXPECT_FALSE(IsFormRenderService("com.ohos.formrenderserviceX"));
    EXPECT_FALSE(IsFormRenderService("xcom.ohos.formrenderservice"));
}

/**
 * @tc.name: IsFormRenderService_EmptyName_ReturnsFalse_0100
 * @tc.desc: empty bundleName -> false.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadFristTest, IsFormRenderService_EmptyName_ReturnsFalse_0100, TestSize.Level1)
{
    EXPECT_FALSE(IsFormRenderService(""));
}
} // namespace AppExecFwk
} // namespace OHOS
