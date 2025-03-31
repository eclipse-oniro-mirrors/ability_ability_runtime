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

#include "app_preloader.h"
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
}  // namespace AppExecFwk
}  // namespace OHOS
