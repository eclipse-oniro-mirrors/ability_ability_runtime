/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#define private public
#include "app_mgr_service_inner.h"
#undef private

#include <unistd.h>
#include <gtest/gtest.h>

#include "refbase.h"
#include "hilog_tag_wrapper.h"
#include "iremote_object.h"
#include "mock_bundle_manager.h"
#include "mock_ability_token.h"
#include "mock_app_scheduler.h"
#include "mock_app_spawn_client.h"

using namespace testing::ext;
using testing::_;
using testing::Return;
using testing::SetArgReferee;
using ::testing::DoAll;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t INDEX_NUM_1 = 1;
const int32_t INDEX_NUM_2 = 2;
const int32_t INDEX_NUM_3 = 3;
const int32_t INDEX_NUM_10 = 10;
const std::string TEST_APP_NAME = "com.ohos.test.helloworld";
const std::string TEST_ABILITY_NAME = "test_ability_";
}  // namespace

class AmsAppRecentListModuleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

protected:
    const std::shared_ptr<ApplicationInfo> GetApplicationByIndex(const int32_t index) const;
    const std::shared_ptr<AppRunningRecord> CreateAppRunningRecordByIndex(const int32_t index) const;

    std::shared_ptr<AppMgrServiceInner> serviceInner_{ nullptr };
    sptr<MockAbilityToken> mockToken_{ nullptr };
    std::shared_ptr<BundleMgrHelper> mockBundleMgr{ nullptr };
};

void AmsAppRecentListModuleTest::SetUpTestCase()
{}

void AmsAppRecentListModuleTest::TearDownTestCase()
{}

void AmsAppRecentListModuleTest::SetUp()
{
    serviceInner_.reset(new (std::nothrow) AppMgrServiceInner());
    serviceInner_->Init();
    mockBundleMgr = DelayedSingleton<BundleMgrHelper>::GetInstance();
    serviceInner_->SetBundleManagerHelper(mockBundleMgr);
}

void AmsAppRecentListModuleTest::TearDown()
{}

const std::shared_ptr<ApplicationInfo> AmsAppRecentListModuleTest::GetApplicationByIndex(const int32_t index) const
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = TEST_APP_NAME + std::to_string(index);
    return appInfo;
}

const std::shared_ptr<AppRunningRecord> AmsAppRecentListModuleTest::CreateAppRunningRecordByIndex(
    const int32_t index) const
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    appInfo->name = TEST_APP_NAME + std::to_string(index);
    abilityInfo->name = TEST_ABILITY_NAME + std::to_string(index);
    abilityInfo->applicationName = appInfo->name;
    abilityInfo->applicationInfo.bundleName = appInfo->name;
    abilityInfo->process = appInfo->name;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    EXPECT_TRUE(serviceInner_->GetBundleAndHapInfo(*abilityInfo, appInfo, bundleInfo, hapModuleInfo, 0));
    auto appRunningRecord = serviceInner_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, abilityInfo->process, abilityInfo->applicationInfo.uid, bundleInfo);
    EXPECT_NE(nullptr, appRunningRecord);
    return appRunningRecord;
}
}  // namespace AppExecFwk
}  // namespace OHOS
