/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "app_utils.h"
#include "child_process_manager.h"
#undef private
#include "child_process_manager_error_utils.h"
#include "js_runtime.h"
#include "mock_app_mgr_service.h"
#include "mock_bundle_manager.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"
#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class ChildProcessManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ChildProcessManagerTest::SetUpTestCase()
{
    AAFwk::AppUtils::GetInstance().isMultiProcessModel_.isLoaded = true;
    AAFwk::AppUtils::GetInstance().isMultiProcessModel_.value = true;
    AAFwk::AppUtils::GetInstance().isSupportNativeChildProcess_.isLoaded = true;
    AAFwk::AppUtils::GetInstance().isSupportNativeChildProcess_.value = true;

    sptr<IRemoteObject> bundleMgrService =  sptr<IRemoteObject>(new (std::nothrow) AppExecFwk::BundleMgrService());
    sptr<IRemoteObject> mockAppMgrService = sptr<IRemoteObject>(new (std::nothrow) AppExecFwk::MockAppMgrService());
    auto sysMgr = DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    if (sysMgr == nullptr) {
        GTEST_LOG_(ERROR) << "Failed to get ISystemAbilityManager.";
        return;
    }
    sysMgr->RegisterSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleMgrService);
    sysMgr->RegisterSystemAbility(APP_MGR_SERVICE_ID, mockAppMgrService);
}

void ChildProcessManagerTest::TearDownTestCase()
{
    AAFwk::AppUtils::GetInstance().isMultiProcessModel_.isLoaded = false;
    AAFwk::AppUtils::GetInstance().isMultiProcessModel_.value = false;
    AAFwk::AppUtils::GetInstance().isSupportNativeChildProcess_.isLoaded = false;
    AAFwk::AppUtils::GetInstance().isSupportNativeChildProcess_.value = false;
}

void ChildProcessManagerTest::SetUp()
{}

void ChildProcessManagerTest::TearDown()
{}

/**
 * @tc.number: StartChildProcessBySelfFork_0100
 * @tc.desc: Test StartChildProcessBySelfFork works
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, StartChildProcessBySelfFork_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "StartChildProcessBySelfFork_0100 called.");
    pid_t pid;
    auto ret = ChildProcessManager::GetInstance().StartChildProcessBySelfFork("./ets/process/DemoProcess.ts", pid);
    EXPECT_NE(ret, ChildProcessManagerErrorCode::ERR_FORK_FAILED);
}

/**
 * @tc.number: StartChildProcessBySelfFork_0200
 * @tc.desc: Test StartChildProcessBySelfFork works
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, StartChildProcessBySelfFork_0200, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "StartChildProcessBySelfFork_0200 called.");
    AAFwk::AppUtils::GetInstance().isMultiProcessModel_.isLoaded = true;
    AAFwk::AppUtils::GetInstance().isMultiProcessModel_.value = true;
    pid_t pid;
    auto ret = ChildProcessManager::GetInstance().StartChildProcessBySelfFork("./ets/process/DemoProcess.ts", pid);
    EXPECT_NE(ret, ChildProcessManagerErrorCode::ERR_FORK_FAILED);
}

/**
 * @tc.number: StartChildProcessByAppSpawnFork_0100
 * @tc.desc: Test StartChildProcessByAppSpawnFork works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, StartChildProcessByAppSpawnFork_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "StartChildProcessByAppSpawnFork_0100 called.");
    pid_t pid;
    auto ret = ChildProcessManager::GetInstance().StartChildProcessByAppSpawnFork("./ets/process/DemoProcess.ts", pid);
    EXPECT_NE(ret, ChildProcessManagerErrorCode::ERR_FORK_FAILED);
}

/**
 * @tc.number: IsChildProcess_0100
 * @tc.desc: Test IsChildProcess works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, IsChildProcess_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "IsChildProcess_0100 called.");
    auto manager = std::make_shared<ChildProcessManager>();
    ChildProcessManager::GetInstance().IsChildProcess();
    EXPECT_TRUE(manager != nullptr);
}

/**
 * @tc.number: GetBundleInfo_0100
 * @tc.desc: Test GetBundleInfo works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, GetBundleInfo_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "GetBundleInfo_0100 called.");
    AppExecFwk::BundleInfo bundleInfo;
    auto ret = ChildProcessManager::GetInstance().GetBundleInfo(bundleInfo);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: GetHapModuleInfo_0100
 * @tc.desc: Test GetHapModuleInfo works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, GetHapModuleInfo_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "GetHapModuleInfo_0100 called.");
    AppExecFwk::BundleInfo bundleInfo;
    auto ret = ChildProcessManager::GetInstance().GetBundleInfo(bundleInfo);
    EXPECT_TRUE(ret);

    AppExecFwk::HapModuleInfo hapModuleInfo;
    ret = ChildProcessManager::GetInstance().GetHapModuleInfo(bundleInfo, "entry", hapModuleInfo);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: GetEntryHapModuleInfo_0100
 * @tc.desc: Test GetEntryHapModuleInfo works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, GetEntryHapModuleInfo_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "GetEntryHapModuleInfo_0100 called.");
    AppExecFwk::BundleInfo bundleInfo;
    auto ret = ChildProcessManager::GetInstance().GetBundleInfo(bundleInfo);
    EXPECT_TRUE(ret);

    AppExecFwk::HapModuleInfo hapModuleInfo;
    ret = ChildProcessManager::GetInstance().GetEntryHapModuleInfo(bundleInfo, hapModuleInfo);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: CreateRuntime_0100
 * @tc.desc: Test CreateRuntime works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, CreateRuntime_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "CreateRuntime_0100 called.");
    std::unique_ptr<AbilityRuntime::Runtime> CreateRuntime(const AppExecFwk::BundleInfo &bundleInfo,
        const AppExecFwk::HapModuleInfo &hapModuleInfo, const bool fromAppSpawn);
    AppExecFwk::BundleInfo bundleInfo;
    auto ret = ChildProcessManager::GetInstance().GetBundleInfo(bundleInfo);
    EXPECT_TRUE(ret);

    AppExecFwk::HapModuleInfo hapModuleInfo;
    ret = ChildProcessManager::GetInstance().GetEntryHapModuleInfo(bundleInfo, hapModuleInfo);
    EXPECT_TRUE(ret);

    auto runtime = ChildProcessManager::GetInstance().CreateRuntime(bundleInfo, hapModuleInfo, false, false);
    EXPECT_TRUE(runtime != nullptr);
}

/**
 * @tc.number: ChildProcessErrorUtils_0100
 * @tc.desc: Test ChildProcessErrorUtils.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, ChildProcessErrorUtils_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "ChildProcessErrorUtils_0100 called.");
    auto err = ChildProcessManagerErrorUtil::GetAbilityErrorCode(ChildProcessManagerErrorCode::ERR_OK);
    EXPECT_EQ(err, AbilityErrorCode::ERROR_OK);
}

/**
 * @tc.number: HandleChildProcessBySelfFork_0100
 * @tc.desc: Test HandleChildProcessBySelfFork works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, HandleChildProcessBySelfFork, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "HandleChildProcessBySelfFork_0100 called.");
    AppExecFwk::BundleInfo bundleInfo;
    ChildProcessManager::GetInstance().HandleChildProcessBySelfFork("./ets/process/DemoProcess.ts", bundleInfo);
    EXPECT_EQ(ChildProcessManager::GetInstance().isChildProcessBySelfFork_, true);
}

/**
 * @tc.number: LoadJsFile_0100
 * @tc.desc: Test LoadJsFile works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, LoadJsFile_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "LoadJsFile_0100 called.");
    std::unique_ptr<Runtime> runtime;
    AppExecFwk::HapModuleInfo hapModuleInfo;
    auto ret = ChildProcessManager::GetInstance().LoadJsFile("./ets/process/AProcess.ts", hapModuleInfo, runtime);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: LoadJsFile_0200
 * @tc.desc: Test LoadJsFile works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, LoadJsFile_0200, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "LoadJsFile_0200 called.");
    std::unique_ptr<Runtime> runtime;
    AppExecFwk::HapModuleInfo hapModuleInfo;
    auto args = std::make_shared<AppExecFwk::ChildProcessArgs>();
    auto ret = ChildProcessManager::GetInstance().LoadJsFile("./ets/process/AProcess.ts", hapModuleInfo, runtime, args);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: SetForkProcessDebugOption_0100
 * @tc.desc: Test SetForkProcessDebugOption.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, SetForkProcessDebugOption_0100, TestSize.Level0)
{
TAG_LOGD(AAFwkTag::TEST, "SetForkProcessDebugOption called.");
AbilityRuntime::Runtime::DebugOption debugOption;
ChildProcessManager::GetInstance().SetForkProcessDebugOption("test", false, false, false);
EXPECT_TRUE(true);
}

/**
 * @tc.number: StartNativeChildProcessByAppSpawnFork_0100
 * @tc.desc: Test StartNativeChildProcessByAppSpawnFork works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, StartNativeChildProcessByAppSpawnFork_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "StartNativeChildProcessByAppSpawnFork_0100 called.");
    sptr<IRemoteObject> callback;
    auto ret = ChildProcessManager::GetInstance().StartNativeChildProcessByAppSpawnFork("test.so", callback);
    EXPECT_NE(ret, ChildProcessManagerErrorCode::ERR_FORK_FAILED);
}

/**
 * @tc.number: GetModuleNameFromSrcEntry_0100
 * @tc.desc: Test GetModuleNameFromSrcEntry works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, GetModuleNameFromSrcEntry_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "GetModuleNameFromSrcEntry_0100 called.");
    std::string srcEntry = "entry/./ets/process/AProcess.ts";
    auto moduleName = ChildProcessManager::GetInstance().GetModuleNameFromSrcEntry(srcEntry);
    EXPECT_EQ(moduleName, "entry");
}

/**
 * @tc.number: GetModuleNameFromSrcEntry_0200
 * @tc.desc: Test GetModuleNameFromSrcEntry works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, GetModuleNameFromSrcEntry_0200, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "GetModuleNameFromSrcEntry_0200 called.");
    std::string srcEntry = "AProcess.ts";
    auto moduleName = ChildProcessManager::GetInstance().GetModuleNameFromSrcEntry(srcEntry);
    EXPECT_EQ(moduleName, "");
}

/**
 * @tc.number: GetModuleNameFromSrcEntry_0300
 * @tc.desc: Test GetModuleNameFromSrcEntry works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, GetModuleNameFromSrcEntry_0300, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "GetModuleNameFromSrcEntry_0300 called.");
    std::string srcEntry = "./ets/process/AProcess.ts";
    auto moduleName = ChildProcessManager::GetInstance().GetModuleNameFromSrcEntry(srcEntry);
    EXPECT_EQ(moduleName, "");
}

/**
 * @tc.number: SetAppSpawnForkDebugOption_0100
 * @tc.desc: Test SetAppSpawnForkDebugOption works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, SetAppSpawnForkDebugOption_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "SetAppSpawnForkDebugOption_0100 called.");
    Runtime::DebugOption debugOption;
    ChildProcessManager::GetInstance().SetAppSpawnForkDebugOption(debugOption, nullptr);
    EXPECT_EQ(debugOption.processName, "");
}

/**
 * @tc.number: SetAppSpawnForkDebugOption_0200
 * @tc.desc: Test SetAppSpawnForkDebugOption works.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, SetAppSpawnForkDebugOption_0200, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "SetAppSpawnForkDebugOption_0200 called.");
    Runtime::DebugOption debugOption;
    auto processInfo = std::make_shared<AppExecFwk::ChildProcessInfo>();
    auto processName = "com.test.abc";
    processInfo->processName = processName;
    ChildProcessManager::GetInstance().SetAppSpawnForkDebugOption(debugOption, processInfo);
    EXPECT_EQ(debugOption.processName, processName);
}
}  // namespace AbilityRuntime
}  // namespace OHOS