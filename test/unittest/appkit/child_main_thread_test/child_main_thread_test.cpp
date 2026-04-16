/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "child_main_thread.h"
#undef private
#include "child_process_info.h"
#include "event_handler.h"
#include "hilog_tag_wrapper.h"
#include "mock_app_mgr_service.h"
#include "mock_bundle_manager.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"
#include "parameters.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class ChildMainThreadTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ChildMainThreadTest::SetUpTestCase()
{
    sptr<IRemoteObject> bundleMgrService = sptr<IRemoteObject>(new (std::nothrow) BundleMgrService());
    sptr<IRemoteObject> mockAppMgrService = sptr<IRemoteObject>(new (std::nothrow) MockAppMgrService());
    auto sysMgr = DelayedSingleton<SysMrgClient>::GetInstance();
    if (sysMgr == nullptr) {
        GTEST_LOG_(ERROR) << "Failed to get ISystemAbilityManager.";
        return;
    }

    sysMgr->RegisterSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleMgrService);
    sysMgr->RegisterSystemAbility(APP_MGR_SERVICE_ID, mockAppMgrService);
}

void ChildMainThreadTest::TearDownTestCase()
{}

void ChildMainThreadTest::SetUp()
{}

void ChildMainThreadTest::TearDown()
{}

/**
 * @tc.number: Init_0100
 * @tc.desc: Test Init works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, Init_0100, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "Init_0100 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    ChildProcessInfo info;
    info.bundleInfo = std::make_shared<BundleInfo>();
    auto ret = thread->Init(runner, info);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: Init_0200
 * @tc.desc: Test Init works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, Init_0200, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "Init_0200 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    ChildProcessInfo info;
    info.bundleInfo = std::make_shared<BundleInfo>();
    info.bundleInfo->applicationInfo.arkTSMode = AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2;
    auto ret = thread->Init(runner, info);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: Init_0300
 * @tc.desc: Test Init works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, Init_0300, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "Init_0300 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    ChildProcessInfo info;
    info.bundleInfo = std::make_shared<BundleInfo>();
    info.bundleInfo->applicationInfo.arkTSMode = AbilityRuntime::CODE_LANGUAGE_ARKTS_HYBRID;
    auto ret = thread->Init(runner, info);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: Init_0400
 * @tc.desc: Test Init works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, Init_0400, TestSize.Level0)
{
    TAG_LOGD(AAFwkTag::TEST, "Init_0400 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    ChildProcessInfo info;
    info.bundleInfo = std::make_shared<BundleInfo>();
    info.bundleInfo->applicationInfo.arkTSMode = AbilityRuntime::CODE_LANGUAGE_ARKTS_1_0;
    auto oldParam = system::GetParameter("persist.sys.abilityms.plugin_default_namespace_lddictionary", "");
    system::SetParameter("persist.sys.abilityms.plugin_default_namespace_lddictionary", "/test/lib");
    auto ret = thread->Init(runner, info);
    system::SetParameter("persist.sys.abilityms.plugin_default_namespace_lddictionary", oldParam);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: IsEtsAPP_0100
 * @tc.desc: Test IsEtsAPP works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, IsEtsAPP_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "IsEtsAPP_0100 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    ChildProcessInfo processInfo;
    auto ret = thread->IsEtsAPP(processInfo);
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: Attach_0100
 * @tc.desc: Test Attach works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, Attach_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "Attach_0100 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    thread->mainHandler_ = std::make_shared<EventHandler>(runner);

    auto ret = thread->Attach();
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: ScheduleLoadChild_0100
 * @tc.desc: Test ScheduleLoadChild_0100 works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, ScheduleLoadChild_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "ScheduleLoadChild_0100 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    std::shared_ptr<EventHandler> handler = std::make_shared<EventHandler>(runner);
    thread->mainHandler_ = handler;
    thread->processInfo_ = std::make_shared<ChildProcessInfo>();

    auto ret = thread->ScheduleLoadChild();
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: HandleLoadJs_0100
 * @tc.desc: Test HandleLoadJs works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, HandleLoadJs_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "HandleLoadJs_0100 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    BundleInfo bundleInfo;
    std::vector<HapModuleInfo> hapModuleInfos;
    HapModuleInfo moduleInfo;
    moduleInfo.name = "entry";
    moduleInfo.moduleName = "entry";
    moduleInfo.moduleType = ModuleType::ENTRY;
    moduleInfo.hapPath = "/data/app/el1/bundle/public/com.ohos.demoprocess/entry";
    moduleInfo.compileMode = CompileMode::ES_MODULE;
    moduleInfo.isStageBasedModel = true;
    hapModuleInfos.push_back(moduleInfo);
    bundleInfo.hapModuleInfos = hapModuleInfos;

    ApplicationInfo applicationInfo;
    applicationInfo.uid = 2001;
    bundleInfo.applicationInfo = applicationInfo;
    
    thread->bundleInfo_ = std::make_shared<BundleInfo>(bundleInfo);
    thread->processInfo_ = std::make_shared<ChildProcessInfo>();
    thread->appMgr_ = sptr<MockAppMgrService>(new (std::nothrow) MockAppMgrService());
    thread->HandleLoadJs();
    ASSERT_NE(thread->runtime_, nullptr);
}

/**
 * @tc.number: HandleLoadArkTs_0100
 * @tc.desc: Test HandleLoadArkTs works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, HandleLoadArkTs_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "HandleLoadArkTs_0100 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    BundleInfo bundleInfo;
    std::vector<HapModuleInfo> hapModuleInfos;
    HapModuleInfo moduleInfo;
    moduleInfo.name = "entry";
    moduleInfo.moduleName = "entry";
    moduleInfo.moduleType = ModuleType::ENTRY;
    moduleInfo.hapPath = "/data/app/el1/bundle/public/com.ohos.demoprocess/entry";
    moduleInfo.compileMode = CompileMode::ES_MODULE;
    moduleInfo.isStageBasedModel = true;
    hapModuleInfos.push_back(moduleInfo);
    bundleInfo.hapModuleInfos = hapModuleInfos;

    ApplicationInfo applicationInfo;
    applicationInfo.uid = 2001;
    bundleInfo.applicationInfo = applicationInfo;
    
    thread->bundleInfo_ = std::make_shared<BundleInfo>(bundleInfo);
    thread->processInfo_ = std::make_shared<ChildProcessInfo>();
    thread->processInfo_->srcEntry = "entry/./ets/process/AProcess.ets";
    thread->appMgr_ = sptr<MockAppMgrService>(new (std::nothrow) MockAppMgrService());
    thread->HandleLoadArkTs();
    ASSERT_NE(thread->runtime_, nullptr);
}

/**
 * @tc.number: ScheduleExitProcessSafely_0100
 * @tc.desc: Test ScheduleExitProcessSafely works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, ScheduleExitProcessSafely_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "ScheduleExitProcessSafely_0100 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    std::shared_ptr<EventHandler> handler = std::make_shared<EventHandler>(runner);
    thread->mainHandler_ = handler;

    auto ret = thread->ScheduleExitProcessSafely();
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: ScheduleRunNativeProc_0100
 * @tc.desc: Test ScheduleRunNativeProc works
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, ScheduleRunNativeProc_0100, TestSize.Level2)
{
    TAG_LOGD(AAFwkTag::TEST, "ScheduleRunNativeProc_0100 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    std::shared_ptr<EventHandler> handler = std::make_shared<EventHandler>(runner);
    thread->mainHandler_ = handler;

    sptr<IRemoteObject> mainPorcessCb = nullptr;
    auto ret = thread->ScheduleRunNativeProc(mainPorcessCb);
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: SetFds_0100
 * @tc.desc: Test SetFds with valid fds map
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, SetFds_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SetFds_0100 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    std::map<std::string, int32_t> fds = {
        {"fd1", 10},
        {"fd2", 20},
        {"fd3", 30}
    };

    thread->SetFds(fds);

    ASSERT_NE(thread->processArgs_, nullptr);

    EXPECT_EQ(thread->processArgs_->fds.size(), 3);
    EXPECT_EQ(thread->processArgs_->fds["fd1"], 10);
    EXPECT_EQ(thread->processArgs_->fds["fd2"], 20);
    EXPECT_EQ(thread->processArgs_->fds["fd3"], 30);
}

/**
 * @tc.number: SetFds_0200
 * @tc.desc: Test SetFds when processArgs_ is null
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, SetFds_0200, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "SetFds_0200 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    thread->processArgs_ = nullptr;

    std::map<std::string, int32_t> fds = {{"test", 123}};

    thread->SetFds(fds);

    EXPECT_EQ(thread->processArgs_, nullptr);
}

/**
 * @tc.number: InitNativeLib_0100
 * @tc.desc: Test InitNativeLib for JS child process
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, InitNativeLib_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "InitNativeLib_0100 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    thread->processInfo_ = std::make_shared<ChildProcessInfo>();
    thread->processInfo_->childProcessType = CHILD_PROCESS_TYPE_JS;

    BundleInfo bundleInfo;
    ApplicationInfo appInfo;
    appInfo.isSystemApp = false;
    bundleInfo.applicationInfo = appInfo;

    HspList hspList;

    thread->InitNativeLib(bundleInfo, hspList);

    EXPECT_TRUE(thread->nativeLibModuleName_.empty());
}

/**
 * @tc.number: InitNativeLib_0200
 * @tc.desc: Test InitNativeLib for Native child process
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, InitNativeLib_0200, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "InitNativeLib_0200 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    thread->processInfo_ = std::make_shared<ChildProcessInfo>();
    thread->processInfo_->childProcessType = CHILD_PROCESS_TYPE_NATIVE;
    thread->processInfo_->srcEntry = "test_lib.so";

    BundleInfo bundleInfo;
    ApplicationInfo appInfo;
    appInfo.isSystemApp = true;
    appInfo.nativeLibraryPath = "libs/arm64";
    bundleInfo.applicationInfo = appInfo;

    HapModuleInfo hapInfo;
    hapInfo.moduleName = "entry";
    hapInfo.hapPath = "/data/app/el1/bundle/public/test.hap";
    bundleInfo.hapModuleInfos.push_back(hapInfo);

    HspList hspList;

    thread->InitNativeLib(bundleInfo, hspList);

    SUCCEED();
}

/**
 * @tc.number: HandleExitProcessSafely_0100
 * @tc.desc: Test HandleExitProcessSafely with valid runner
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, HandleExitProcessSafely_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "HandleExitProcessSafely_0100 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    std::shared_ptr<EventRunner> runner = EventRunner::Create(true);
    std::shared_ptr<EventHandler> handler = std::make_shared<EventHandler>(runner);
    thread->mainHandler_ = handler;

    runner->Run();

    thread->HandleExitProcessSafely();

    SUCCEED();
}

/**
 * @tc.number: HandleExitProcessSafely_0200
 * @tc.desc: Test HandleExitProcessSafely when runner is null
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, HandleExitProcessSafely_0200, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "HandleExitProcessSafely_0200 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    std::shared_ptr<EventRunner> runner = nullptr;
    std::shared_ptr<EventHandler> handler = std::make_shared<EventHandler>(runner);
    thread->mainHandler_ = handler;

    thread->HandleExitProcessSafely();

    SUCCEED();
}

/**
 * @tc.number: GetNativeLibPath_0100
 * @tc.desc: Test GetNativeLibPath with nativeLibraryPath in application
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, GetNativeLibPath_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetNativeLibPath_0100 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    BundleInfo bundleInfo;
    ApplicationInfo appInfo;
    appInfo.nativeLibraryPath = "libs/arm64";
    bundleInfo.applicationInfo = appInfo;

    HspList hspList;
    AppLibPathMap appLibPaths;

    thread->GetNativeLibPath(bundleInfo, hspList, appLibPaths);

    if (!appInfo.nativeLibraryPath.empty()) {
        EXPECT_TRUE(appLibPaths.find("default") != appLibPaths.end());
        EXPECT_FALSE(appLibPaths["default"].empty());

        for (const auto& path : appLibPaths["default"]) {
            EXPECT_TRUE(path.find("/data/storage/el1/bundle") != std::string::npos);
            EXPECT_TRUE(path.find("libs/arm64") != std::string::npos);
        }
    }
}

/**
 * @tc.number: GetNativeLibPath_0200
 * @tc.desc: Test GetNativeLibPath with hap module info
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, GetNativeLibPath_0200, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetNativeLibPath_0200 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    BundleInfo bundleInfo;
    ApplicationInfo appInfo;
    appInfo.nativeLibraryPath = "";
    bundleInfo.applicationInfo = appInfo;

    HapModuleInfo hapInfo;
    hapInfo.moduleName = "entry";
    hapInfo.isLibIsolated = false;
    hapInfo.compressNativeLibs = false;
    hapInfo.hapPath = "/data/app/el1/bundle/public/test.hap";
    bundleInfo.hapModuleInfos.push_back(hapInfo);

    HspList hspList;
    AppLibPathMap appLibPaths;

    thread->GetNativeLibPath(bundleInfo, hspList, appLibPaths);

    if (hapInfo.hapPath.find("/data/app/el1/bundle/public") == 0) {
        SUCCEED();
    }
}

/**
 * @tc.number: GetNativeLibPath_0300
 * @tc.desc: Test GetNativeLibPath with hsp list
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, GetNativeLibPath_0300, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetNativeLibPath_0300 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    BundleInfo bundleInfo;
    ApplicationInfo appInfo;
    appInfo.nativeLibraryPath = "";
    bundleInfo.applicationInfo = appInfo;

    HspList hspList;

    BaseSharedBundleInfo hspInfo;
    hspInfo.bundleName = "com.test.hsp";
    hspInfo.moduleName = "shared";
    hspInfo.nativeLibraryPath = "libs/arm64";
    hspInfo.hapPath = "/data/app/el1/bundle/public/shared.hsp";
    hspList.push_back(hspInfo);

    AppLibPathMap appLibPaths;

    thread->GetNativeLibPath(bundleInfo, hspList, appLibPaths);

    if (hspInfo.hapPath.find("/data/app/el1/bundle/public") == 0) {
        SUCCEED();
    }
}

/**
 * @tc.number: GetNativeLibPath_0400
 * @tc.desc: Test GetNativeLibPath with relative hap path
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, GetNativeLibPath_0400, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetNativeLibPath_0400 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    BundleInfo bundleInfo;
    ApplicationInfo appInfo;
    appInfo.nativeLibraryPath = "";
    bundleInfo.applicationInfo = appInfo;

    HapModuleInfo hapInfo;
    hapInfo.moduleName = "entry";
    hapInfo.isLibIsolated = true;
    hapInfo.compressNativeLibs = true;
    hapInfo.hapPath = "relative/path/test.hap";
    bundleInfo.hapModuleInfos.push_back(hapInfo);

    HspList hspList;
    AppLibPathMap appLibPaths;

    thread->GetNativeLibPath(bundleInfo, hspList, appLibPaths);

    SUCCEED();
}

/**
 * @tc.number: GetNativeLibPath_0500
 * @tc.desc: Test GetNativeLibPath with empty nativeLibraryPath
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, GetNativeLibPath_0500, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "GetNativeLibPath_0500 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    BundleInfo bundleInfo;
    ApplicationInfo appInfo;
    appInfo.nativeLibraryPath = "";
    bundleInfo.applicationInfo = appInfo;

    HspList hspList;
    AppLibPathMap appLibPaths;

    thread->GetNativeLibPath(bundleInfo, hspList, appLibPaths);

    SUCCEED();
}

/**
 * @tc.number: UpdateNativeChildLibModuleName_0100
 * @tc.desc: Test UpdateNativeChildLibModuleName when native module exists
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, UpdateNativeChildLibModuleName_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "UpdateNativeChildLibModuleName_0100 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    thread->processInfo_ = std::make_shared<ChildProcessInfo>();
    thread->processInfo_->srcEntry = "test.so";

    AppLibPathMap appLibPaths;

    std::vector<std::string> libPaths = {"/data/app/libs/arm64/"};
    appLibPaths["test_module"] = libPaths;

    bool isSystemApp = false;

    thread->UpdateNativeChildLibModuleName(appLibPaths, isSystemApp);

    EXPECT_TRUE(thread->nativeLibModuleName_.empty());
}

/**
 * @tc.number: UpdateNativeChildLibModuleName_0200
 * @tc.desc: Test UpdateNativeChildLibModuleName with multiple module paths
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, UpdateNativeChildLibModuleName_0200, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "UpdateNativeChildLibModuleName_0200 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    thread->processInfo_ = std::make_shared<ChildProcessInfo>();
    thread->processInfo_->srcEntry = "libtest.so";

    AppLibPathMap appLibPaths;

    appLibPaths["module1"] = {"/path1/libs/", "/path1/lib64/"};
    appLibPaths["module2"] = {"/path2/libs/", "/path2/lib64/"};
    appLibPaths["module3"] = {"/path3/libs/"};

    bool isSystemApp = true;

    thread->UpdateNativeChildLibModuleName(appLibPaths, isSystemApp);

    EXPECT_TRUE(thread->nativeLibModuleName_.empty());
}

/**
 * @tc.number: UpdateNativeChildLibModuleName_0300
 * @tc.desc: Test UpdateNativeChildLibModuleName when nativeModuleMgr is null
 * @tc.type: FUNC
 */
HWTEST_F(ChildMainThreadTest, UpdateNativeChildLibModuleName_0300, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "UpdateNativeChildLibModuleName_0300 called.");
    sptr<ChildMainThread> thread = sptr<ChildMainThread>(new (std::nothrow) ChildMainThread());
    ASSERT_NE(thread, nullptr);

    thread->processInfo_ = std::make_shared<ChildProcessInfo>();
    thread->processInfo_->srcEntry = "test.so";

    AppLibPathMap appLibPaths;
    appLibPaths["test_module"] = {"/test/path/"};

    bool isSystemApp = false;

    thread->UpdateNativeChildLibModuleName(appLibPaths, isSystemApp);

    SUCCEED();
}

} // namespace AppExecFwk
} // namespace OHOS
