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

#include <cstring>
#include <gtest/gtest.h>

#define private public
#define protected public
#include "application_env_impl.h"
#include "application_context.h"
#undef private
#undef protected
#include "application_env.h"
#include "context_impl.h"
#include "mock_application.h"
#include "app_mgr_constants.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class ApplicationEnvImplTest : public testing::Test {
public:
    ApplicationEnvImplTest()
    {}
    ~ApplicationEnvImplTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ApplicationEnvImplTest::SetUpTestCase(void)
{}

void ApplicationEnvImplTest::TearDownTestCase(void)
{}

void ApplicationEnvImplTest::SetUp(void)
{}

void ApplicationEnvImplTest::TearDown(void)
{}

/**
 * @tc.number: AppExecFwk_ApplicationEnvImplTest_SetAppInfo_0100
 * @tc.name: ApplicationEnvImpl SetAppInfo
 * @tc.desc: Test ApplicationEnvImpl set and get applicationInfo.
 */
HWTEST_F(ApplicationEnvImplTest, AppExecFwk_ApplicationEnvImplTest_SetAppInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvImplTest_SetAppInfo_0100 start";
    ApplicationInfo applicationInfo;
    applicationInfo.bundleName = "bundleName";
    applicationInfo.dataDir = "/dataDir";
    applicationInfo.codePath = "/codePath";
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    ApplicationEnvImpl::GetInstance()->SetAppInfo(applicationInfo, preloadMode);
    EXPECT_EQ(ApplicationEnvImpl::GetInstance()->GetBundleName(), "bundleName");
    EXPECT_EQ(ApplicationEnvImpl::GetInstance()->GetSrcPath(), "/codePath");
    EXPECT_EQ(ApplicationEnvImpl::GetInstance()->GetDataPath(), "/dataDir");
    EXPECT_EQ(ApplicationEnvImpl::GetInstance()->GetAppPreloadType(), 1);
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvImplTest_SetAppInfo_0100 end";
}

/**
 * @tc.number: AppExecFwk_ApplicationEnvImplTest_SetAppInfo_0200
 * @tc.name: ApplicationEnvImpl SetAppInfo
 * @tc.desc: Test ApplicationEnvImpl set and get appInfo.
 */
HWTEST_F(ApplicationEnvImplTest, AppExecFwk_ApplicationEnvImplTest_SetAppInfo_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvImplTest_SetAppInfo_0200 start";
    AppInfo appInfo;
    appInfo.bundleName = "bundleName";
    appInfo.dataPath = "/dataDir";
    appInfo.srcPath = "/codePath";
    ApplicationEnvImpl::GetInstance()->SetAppInfo(appInfo);
    EXPECT_EQ(ApplicationEnvImpl::GetInstance()->GetBundleName(), "bundleName");
    EXPECT_EQ(ApplicationEnvImpl::GetInstance()->GetSrcPath(), "/codePath");
    EXPECT_EQ(ApplicationEnvImpl::GetInstance()->GetDataPath(), "/dataDir");
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvImplTest_SetAppInfo_0200 end";
}

/**
 * @tc.number: AppExecFwk_ApplicationEnvTest_SetAppInfo_0100
 * @tc.name: ApplicationEnv SetAppInfo
 * @tc.desc: Test ApplicationEnv get appInfo.
 */
HWTEST_F(ApplicationEnvImplTest, AppExecFwk_ApplicationEnvTest_SetAppInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_SetAppInfo_0100 start";
    AppInfo appInfo;
    appInfo.bundleName = "bundleName";
    appInfo.dataPath = "/dataDir";
    appInfo.srcPath = "/codePath";
    ApplicationEnvImpl::GetInstance()->SetAppInfo(appInfo);
    std::string bundleName(GetBundleName());
    std::string codePath(GetSrcPath());
    std::string dataDir(GetDataPath());
    EXPECT_EQ(bundleName, "bundleName");
    EXPECT_EQ(codePath, "/codePath");
    EXPECT_EQ(dataDir, "/dataDir");
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_SetAppInfo_0100 end";
}

/**
 * @tc.number: AppExecFwk_ApplicationEnvTest_SetAppInfo_0200
 * @tc.name: ApplicationEnv SetAppInfo
 * @tc.desc: Test ApplicationEnv get applicationInfo.
 */
HWTEST_F(ApplicationEnvImplTest, AppExecFwk_ApplicationEnvTest_SetAppInfo_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_SetAppInfo_0200 start";
    ApplicationInfo applicationInfo;
    applicationInfo.bundleName = "bundleName";
    applicationInfo.dataDir = "/dataDir";
    applicationInfo.codePath = "/codePath";
    PreloadMode preloadMode = PreloadMode::PRE_MAKE;
    ApplicationEnvImpl::GetInstance()->SetAppInfo(applicationInfo, preloadMode);
    std::string bundleName(GetBundleName());
    std::string codePath(GetSrcPath());
    std::string dataDir(GetDataPath());
    EXPECT_EQ(bundleName, "bundleName");
    EXPECT_EQ(codePath, "/codePath");
    EXPECT_EQ(dataDir, "/dataDir");
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_SetAppInfo_0200 end";
}

/**
 * @tc.number: AppExecFwk_ApplicationEnvTest_SetAppInfo_0300
 * @tc.name: ApplicationEnv SetAppInfo
 * @tc.desc: Test ApplicationEnv get applicationInfo.
 */
HWTEST_F(ApplicationEnvImplTest, AppExecFwk_ApplicationEnvTest_SetAppInfo_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_SetAppInfo_0300 start";
    ApplicationInfo applicationInfo;
    applicationInfo.bundleName = "bundleName";
    applicationInfo.dataDir = "/dataDir";
    applicationInfo.codePath = "/codePath";
    PreloadMode preloadMode = PreloadMode::PRESS_DOWN;
    ApplicationEnvImpl::GetInstance()->SetAppInfo(applicationInfo, preloadMode);
    EXPECT_EQ(ApplicationEnvImpl::GetInstance()->GetAppPreloadType(), 1);
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_SetAppInfo_0300 end";
}

/**
 * @tc.number: AppExecFwk_ApplicationEnvTest_SetAppInfo_0400
 * @tc.name: ApplicationEnv SetAppInfo
 * @tc.desc: Test ApplicationEnv get applicationInfo.
 */
HWTEST_F(ApplicationEnvImplTest, AppExecFwk_ApplicationEnvTest_SetAppInfo_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_SetAppInfo_0400 start";
    ApplicationInfo applicationInfo;
    applicationInfo.bundleName = "bundleName";
    applicationInfo.dataDir = "/dataDir";
    applicationInfo.codePath = "/codePath";
    PreloadMode preloadMode = PreloadMode::PRELOAD_MODULE;
    ApplicationEnvImpl::GetInstance()->SetAppInfo(applicationInfo, preloadMode);
    EXPECT_EQ(ApplicationEnvImpl::GetInstance()->GetAppPreloadType(), 2);
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_SetAppInfo_0400 end";
}

/**
 * @tc.number: AppExecFwk_ApplicationEnvTest_SetAppInfo_0500
 * @tc.name: ApplicationEnv SetAppInfo
 * @tc.desc: Test ApplicationEnv get applicationInfo.
 */
HWTEST_F(ApplicationEnvImplTest, AppExecFwk_ApplicationEnvTest_SetAppInfo_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_SetAppInfo_0500 start";
    ApplicationInfo applicationInfo;
    applicationInfo.bundleName = "bundleName";
    applicationInfo.dataDir = "/dataDir";
    applicationInfo.codePath = "/codePath";
    PreloadMode preloadMode = PreloadMode::PRELOAD_NONE;
    ApplicationEnvImpl::GetInstance()->SetAppInfo(applicationInfo, preloadMode);
    EXPECT_EQ(ApplicationEnvImpl::GetInstance()->GetAppPreloadType(), 0);
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_SetAppInfo_0500 end";
}

/**
 * @tc.number: AppExecFwk_ApplicationEnvTest_SetAppInfo_0600
 * @tc.name: ApplicationEnv SetAppInfo
 * @tc.desc: Test ApplicationEnv get applicationInfo.
 */
HWTEST_F(ApplicationEnvImplTest, AppExecFwk_ApplicationEnvTest_SetAppInfo_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_SetAppInfo_0500 start";
    ApplicationInfo applicationInfo;
    applicationInfo.bundleName = "bundleName";
    applicationInfo.dataDir = "/dataDir";
    applicationInfo.codePath = "/codePath";
    PreloadMode preloadMode = PreloadMode::PRELOAD_BY_PHASE;
    applicationInfo.appPreloadPhase = AppExecFwk::AppPreloadPhase::PROCESS_CREATED;
    ApplicationEnvImpl::GetInstance()->SetAppInfo(applicationInfo, preloadMode);
    EXPECT_EQ(ApplicationEnvImpl::GetInstance()->GetAppPreloadType(), 1);
    applicationInfo.appPreloadPhase = AppExecFwk::AppPreloadPhase::ABILITY_STAGE_CREATED;
    ApplicationEnvImpl::GetInstance()->SetAppInfo(applicationInfo, preloadMode);
    EXPECT_EQ(ApplicationEnvImpl::GetInstance()->GetAppPreloadType(), 2);
    applicationInfo.appPreloadPhase = AppExecFwk::AppPreloadPhase::WINDOW_STAGE_CREATED;
    ApplicationEnvImpl::GetInstance()->SetAppInfo(applicationInfo, preloadMode);
    EXPECT_EQ(ApplicationEnvImpl::GetInstance()->GetAppPreloadType(), 3);
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_SetAppInfo_0600 end";
}

/**
 * @tc.number: AppExecFwk_ApplicationEnvTest_SetAppInfo_0700
 * @tc.name: ApplicationEnv SetAppInfo
 * @tc.desc: Test ApplicationEnv get applicationInfo.
 */
HWTEST_F(ApplicationEnvImplTest, AppExecFwk_ApplicationEnvTest_SetAppInfo_0700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_SetAppInfo_0700 start";
    ApplicationInfo applicationInfo;
    applicationInfo.bundleName = "bundleName";
    applicationInfo.dataDir = "/dataDir";
    applicationInfo.codePath = "/codePath";
    PreloadMode preloadMode = PreloadMode::PRE_LAUNCH;
    ApplicationEnvImpl::GetInstance()->SetAppInfo(applicationInfo, preloadMode);
    EXPECT_EQ(ApplicationEnvImpl::GetInstance()->GetAppPreloadType(), 4);
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_SetAppInfo_0300 end";
}

/**
 * @tc.number: AppExecFwk_ApplicationEnvTest_InvokeAppFreezeFilterCallback_0100
 * @tc.name: InvokeAppFreezeFilterCallback null buffer
 * @tc.desc: InvokeAppFreezeFilterCallback returns false when eventConfigKey is nullptr.
 */
HWTEST_F(ApplicationEnvImplTest, AppExecFwk_ApplicationEnvTest_InvokeAppFreezeFilterCallback_0100,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_InvokeAppFreezeFilterCallback_0100 start";
    EXPECT_FALSE(InvokeAppFreezeFilterCallback(1, 1, nullptr, 64));
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_InvokeAppFreezeFilterCallback_0100 end";
}

/**
 * @tc.number: AppExecFwk_ApplicationEnvTest_InvokeAppFreezeFilterCallback_0200
 * @tc.name: InvokeAppFreezeFilterCallback invalid buffer size
 * @tc.desc: InvokeAppFreezeFilterCallback returns false when keyBufSize <= 0.
 */
HWTEST_F(ApplicationEnvImplTest, AppExecFwk_ApplicationEnvTest_InvokeAppFreezeFilterCallback_0200,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_InvokeAppFreezeFilterCallback_0200 start";
    char buf[64] = "user.event_config.jsvm_heap_log_enable";
    EXPECT_FALSE(InvokeAppFreezeFilterCallback(1, 1, buf, 0));
    EXPECT_FALSE(InvokeAppFreezeFilterCallback(1, 1, buf, -1));
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_InvokeAppFreezeFilterCallback_0200 end";
}

/**
 * @tc.number: AppExecFwk_ApplicationEnvTest_InvokeAppFreezeFilterCallback_0300
 * @tc.name: InvokeAppFreezeFilterCallback no callback registered
 * @tc.desc: InvokeAppFreezeFilterCallback returns false when no callback is registered.
 */
HWTEST_F(ApplicationEnvImplTest, AppExecFwk_ApplicationEnvTest_InvokeAppFreezeFilterCallback_0300,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_InvokeAppFreezeFilterCallback_0300 start";
    auto applicationContext = OHOS::AbilityRuntime::ApplicationContext::GetInstance();
    ASSERT_NE(applicationContext, nullptr);
    applicationContext->appFreezeFilterCallback_ = nullptr;
    char buf[64] = "user.event_config.jsvm_heap_log_enable";
    EXPECT_FALSE(InvokeAppFreezeFilterCallback(1, 1, buf, 64));
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_InvokeAppFreezeFilterCallback_0300 end";
}

/**
 * @tc.number: AppExecFwk_ApplicationEnvTest_InvokeAppFreezeFilterCallback_0400
 * @tc.name: InvokeAppFreezeFilterCallback callback returns false
 * @tc.desc: InvokeAppFreezeFilterCallback returns false when callback returns false.
 */
HWTEST_F(ApplicationEnvImplTest, AppExecFwk_ApplicationEnvTest_InvokeAppFreezeFilterCallback_0400,
         Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_InvokeAppFreezeFilterCallback_0400 start";
    auto applicationContext = OHOS::AbilityRuntime::ApplicationContext::GetInstance();
    ASSERT_NE(applicationContext, nullptr);
    auto cb = [](int32_t, bool, std::string &) -> bool { return false; };
    applicationContext->appFreezeFilterCallback_ = cb;
    char buf[64] = "user.event_config.jsvm_heap_log_enable";
    EXPECT_FALSE(InvokeAppFreezeFilterCallback(1, 1, buf, 64));
    GTEST_LOG_(INFO) << "AppExecFwk_ApplicationEnvTest_InvokeAppFreezeFilterCallback_0400 end";
}

}  // namespace AppExecFwk
}  // namespace OHOS
