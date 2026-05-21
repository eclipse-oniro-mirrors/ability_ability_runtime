/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#include <gtest/hwext/gtest-multithread.h>
#include <cstdlib>
#include <fstream>

#define private public
#define protected public
#include "ets_environment.h"
#include "ets_runtime.h"
#undef private
#undef protected
#include "bundle_constants.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "runtime.h"
#include "ets_environment.h"
#include "hap_module_info.h"

using namespace testing;
using namespace testing::ext;
using namespace testing::mt;

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string TEST_BUNDLE_NAME = "com.ohos.contactsdataability";
const std::string TEST_MODULE_NAME = ".ContactsDataAbility";
const std::string TEST_ABILITY_NAME = "ContactsDataAbility";
const std::string TEST_CODE_PATH = "/data/storage/el1/bundle";
const std::string TEST_HAP_PATH = "/system/app/com.ohos.contactsdataabilityContacts_DataAbility.hap";
const std::string TEST_LIB_PATH = "/data/storage/el1/bundle/lib/";
const std::string TEST_MODULE_PATH = "/data/storage/el1/bundle/curJsModulePath";
const std::string TEST_HOST_AOT_ROOT = "/system/app/ark_cache/";
const std::string TEST_HOST_AOT_BUNDLE_NAME = "com.example.hostaot";

std::string GetHostAotAnPath(const std::string &bundleName, const std::string &moduleName)
{
    return TEST_HOST_AOT_ROOT + bundleName + "/" + moduleName + ".an";
}

void CreateHostAotAnFile(const std::string &bundleName, const std::string &moduleName)
{
    std::string anPath = GetHostAotAnPath(bundleName, moduleName);
    std::string mkdirCmd = "mkdir -p " + TEST_HOST_AOT_ROOT + bundleName;
    ASSERT_EQ(std::system(mkdirCmd.c_str()), 0);

    std::ofstream anFile(anPath);
    ASSERT_TRUE(anFile.is_open());
    anFile << "test";
    anFile.close();
}
} // namespace

class EtsRuntimeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    Runtime::Options options_;
};

void EtsRuntimeTest::SetUpTestCase() {}

void EtsRuntimeTest::TearDownTestCase() {}

void EtsRuntimeTest::SetUp()
{
    Runtime::Options newOptions;
    options_ = newOptions;
    options_.bundleName = TEST_BUNDLE_NAME;
    options_.codePath = TEST_CODE_PATH;
    options_.loadAce = false;
    options_.isBundle = true;
    options_.preload = false;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::Create(TEST_ABILITY_NAME);
    options_.eventRunner = eventRunner;
}

void EtsRuntimeTest::TearDown()
{
    std::string rmCmd = "rm -rf " + TEST_HOST_AOT_ROOT + TEST_HOST_AOT_BUNDLE_NAME;
    std::system(rmCmd.c_str());
}

/**
 * @tc.name: Create_100
 * @tc.desc: EtsRuntime test for Create Initialize failed.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, Create_100, TestSize.Level1)
{
    options_.lang = Runtime::Language::JS;
    options_.preload = true;
    options_.isStageModel = false;
    std::unique_ptr<Runtime> jsRuntime = JsRuntime::Create(options_);
    auto etsRuntime = ETSRuntime::Create(options_, jsRuntime);
    EXPECT_EQ(etsRuntime, nullptr);
    options_.lang = Runtime::Language::ETS;
    options_.preload = false;
    options_.isStageModel = true;
}

/**
 * @tc.name: SetAppLibPath_100
 * @tc.desc: EtsRuntime test for SetAppLibPath.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, SetAppLibPath_100, TestSize.Level1)
{
    std::map<std::string, std::vector<std::string>> testPathMap;
    testPathMap["com.example.app"] = { "/data/abc", "/data/def" };
    testPathMap["com.example.demo"] = { "/data/demo/es", "/data/demo/ts" };
    std::map<std::string, std::string> abcPathsToBundleModuleNameMap;
    std::unique_ptr<ETSRuntime> etsRuntime = std::make_unique<ETSRuntime>();
    etsRuntime->SetAppLibPath(testPathMap, abcPathsToBundleModuleNameMap, false);
    EXPECT_NE(testPathMap.size(), 0);
}

/**
 * @tc.name: Initialize_100
 * @tc.desc: EtsRuntime test for Initialize lang is not ETS.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, Initialize_100, TestSize.Level1)
{
    options_.lang = Runtime::Language::JS;
    std::unique_ptr<Runtime> jsRuntime = nullptr;
    std::unique_ptr<ETSRuntime> etsRuntime = std::make_unique<ETSRuntime>();
    bool result = etsRuntime->Initialize(options_, jsRuntime);
    EXPECT_EQ(result, false);
    options_.lang = Runtime::Language::ETS;
}

/**
 * @tc.name: Initialize_200
 * @tc.desc: EtsRuntime test for Initialize lang is not ETS.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, Initialize_200, TestSize.Level1)
{
    Runtime::Options options;
    options.lang = Runtime::Language::ETS;
    options.arkNativeFilePath = "test_app/";
    options.moduleName = "TestModule";
    std::unique_ptr<Runtime> jsRuntime = AbilityRuntime::JsRuntime::Create(options);
    ASSERT_NE(jsRuntime, nullptr);
    std::unique_ptr<ETSRuntime> etsRuntime = std::make_unique<ETSRuntime>();
    ASSERT_NE(etsRuntime, nullptr);
    bool result = etsRuntime->Initialize(options, jsRuntime);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: LoadModule_0100
 * @tc.desc: LoadModule with non-empty hapPath should construct file path directly.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, LoadModule_0100, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    etsRuntime->codePath_ = "/test/code/path";
    std::string moduleName = "abc";
    std::string modulePath = "dir.test.module";
    std::string hapPath = "/some/hap";
    std::string srcEntrance = "main.ets";
    etsRuntime->PreloadModule(moduleName, hapPath, false, false);
    auto result = etsRuntime->LoadModule(moduleName, modulePath, hapPath, false, false, srcEntrance);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: LoadModule_0200
 * @tc.desc: LoadModule trims moduleName containing "::" correctly.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, LoadModule_0200, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    etsRuntime->codePath_ = "/code";
    std::string moduleName = "lib::submod";
    std::string modulePath = "m.js";
    std::string hapPath = "/hap";
    std::string srcEntrance = "main";
    etsRuntime->LoadModule(moduleName, modulePath, hapPath, false, false, srcEntrance);
    EXPECT_EQ(etsRuntime->moduleName_, "lib");
}

/**
 * @tc.name: Deinitialize_100
 * @tc.desc: EtsRuntime test for Deinitialize.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, Deinitialize_100, TestSize.Level1)
{
    std::unique_ptr<ETSRuntime> etsRuntime = std::make_unique<ETSRuntime>();
    etsRuntime->Deinitialize();
    EXPECT_EQ(etsRuntime->jsRuntime_, nullptr);
}

/**
 * @tc.name: GetAniEnv_100
 * @tc.desc: EtsRuntime test for GetAniEnv.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, GetAniEnv_100, TestSize.Level1)
{
    std::unique_ptr<ETSRuntime> etsRuntime = std::make_unique<ETSRuntime>();
    auto env = etsRuntime->GetAniEnv();
    EXPECT_EQ(env, nullptr);
}

/**
 * @tc.name: LoadModule_100
 * @tc.desc: EtsRuntime test for LoadModule.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, LoadModule_100, TestSize.Level1)
{
    std::unique_ptr<ETSRuntime> etsRuntime = std::make_unique<ETSRuntime>();

    std::string moduleName = TEST_MODULE_NAME;
    moduleName += "::";
    std::string modulePath = TEST_MODULE_PATH;
    std::string hapPath = "";
    bool esmodule = true;
    bool useCommonChunk = false;
    std::string srcEntrance = "";

    auto env = etsRuntime->LoadModule(moduleName, modulePath, hapPath, esmodule, useCommonChunk, srcEntrance);
    EXPECT_EQ(env, nullptr);

    env = nullptr;
    hapPath = TEST_HAP_PATH;
    env = etsRuntime->LoadModule(moduleName, modulePath, hapPath, esmodule, useCommonChunk, srcEntrance);
    EXPECT_EQ(env, nullptr);
}

/**
 * @tc.name: LoadEtsModule_100
 * @tc.desc: EtsRuntime test for LoadEtsModule.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, LoadEtsModule_100, TestSize.Level1)
{
    std::unique_ptr<ETSRuntime> etsRuntime = std::make_unique<ETSRuntime>();

    std::string moduleName = TEST_MODULE_NAME;
    moduleName += "::";
    std::string modulePath = TEST_MODULE_PATH;
    std::string hapPath = "";
    std::string srcEntrance = "";
    auto env = etsRuntime->LoadEtsModule(moduleName, modulePath, hapPath, srcEntrance);
    EXPECT_EQ(env, nullptr);
}

/**
 * @tc.name: GetLanguage_100
 * @tc.desc: EtsRuntime test for GetLanguage.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, GetLanguage_100, TestSize.Level1)
{
    std::unique_ptr<ETSRuntime> etsRuntime = std::make_unique<ETSRuntime>();
    auto language = etsRuntime->GetLanguage();
    EXPECT_EQ(language, Runtime::Language::ETS);
}

/**
 * @tc.name: PreFork_100
 * @tc.desc: EtsRuntime test for PreFork.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, PreFork_100, TestSize.Level1)
{
    Runtime::Options options;
    options.lang = Runtime::Language::JS;
    options.preload = true;
    options.isStageModel = false;
    std::unique_ptr<Runtime> jsRuntime = nullptr;
    std::unique_ptr<ETSRuntime> etsRuntime = std::make_unique<ETSRuntime>();
    auto instance = etsRuntime->PreFork(options, jsRuntime);
    EXPECT_EQ(instance, nullptr);
}

/**
 * @tc.name: PostFork_100
 * @tc.desc: EtsRuntime test for PostFork.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, PostFork_100, TestSize.Level1)
{
    Runtime::Options jsOptions;
    jsOptions.lang = Runtime::Language::JS;
    jsOptions.preload = true;
    jsOptions.isStageModel = false;
    std::unique_ptr<Runtime> jsRuntime = JsRuntime::Create(jsOptions);;
    ASSERT_NE(jsRuntime, nullptr);

    Runtime::Options etsOptions;
    etsOptions.lang = Runtime::Language::ETS;
    etsOptions.preload = false;
    etsOptions.isStageModel = true;
    std::unique_ptr<ETSRuntime> etsRuntime = std::make_unique<ETSRuntime>();
    auto result = etsRuntime->PostFork(etsOptions, jsRuntime);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: PreloadSystemClass_100
 * @tc.desc: EtsRuntime test for PreloadSystemClass.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, PreloadSystemClass_100, TestSize.Level1)
{
    std::unique_ptr<ETSRuntime> etsRuntime = std::make_unique<ETSRuntime>();
    std::string className = "className";
    auto result = etsRuntime->PreloadSystemClass(className.c_str());
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: SetModuleLoadChecker_100
 * @tc.desc: EtsRuntime test for SetModuleLoadChecker.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, SetModuleLoadChecker_100, TestSize.Level1)
{
    std::unique_ptr<ETSRuntime> etsRuntime = std::make_unique<ETSRuntime>();
    etsRuntime->SetModuleLoadChecker(nullptr);
    EXPECT_EQ(etsRuntime->GetJsRuntime(), nullptr);
    etsRuntime->jsRuntime_ = std::make_unique<JsRuntime>();
    etsRuntime->SetModuleLoadChecker(nullptr);
    EXPECT_NE(etsRuntime->GetJsRuntime(), nullptr);
}

/**
 * @tc.name: SetExtensionApiCheckCallback_100
 * @tc.desc: EtsRuntime test for SetExtensionApiCheckCallback.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, SetExtensionApiCheckCallback_100, TestSize.Level1)
{
    std::unique_ptr<ETSRuntime> etsRuntime = std::make_unique<ETSRuntime>();
    std::function<bool(const std::string &clsName, const std::string &fName)> callback =
        [](const std::string &className, const std::string &fileName) -> bool {
        return false;
    };
    etsRuntime->SetExtensionApiCheckCallback(callback);
    EXPECT_EQ(etsRuntime->GetJsRuntime(), nullptr);
}

/**
 * @tc.name: GetHspPathList_100
 * @tc.desc: EtsRuntime test for GetHspPathList.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, GetHspPathList_100, TestSize.Level1)
{
    std::unique_ptr<EtsEnv::ETSEnvironment> etsEnvironment = std::make_unique<EtsEnv::ETSEnvironment>();
    etsEnvironment->commonHspBundleInfos_ = {};
    auto hspPathList = etsEnvironment->GetHspPathList();
    EXPECT_EQ(hspPathList.size(), 0);
}

/**
 * @tc.name: GetHspPathList_200
 * @tc.desc: EtsRuntime test for GetHspPathList.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, GetHspPathList_200, TestSize.Level1)
{
    std::unique_ptr<EtsEnv::ETSEnvironment> etsEnvironment = std::make_unique<EtsEnv::ETSEnvironment>();
    etsEnvironment->commonHspBundleInfos_ = {{0, 0, "", "", "/system/app/path1", "static"}};
    auto hspPathList = etsEnvironment->GetHspPathList();
    EXPECT_EQ(hspPathList[0], "/system/app/path1");
}

/**
 * @tc.name: GetAotPath_001
 * @tc.desc: EtsRuntime test for GetAotPath with empty arkNativeFilePath.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, GetAotPath_001, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    Runtime::Options options;
    options.arkNativeFilePath = "";
    auto result = etsRuntime->GetAotPath(options);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: GetAotPath_002
 * @tc.desc: EtsRuntime test for GetAotPath with arkNativeFilePath but empty maps.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, GetAotPath_002, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    Runtime::Options options;
    options.arkNativeFilePath = "arm64/";
    options.moduleName = "testModule";
    options.aotCompileStatusMap = {};
    options.commonHspBundleInfos = {};
    auto result = etsRuntime->GetAotPath(options);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: GetAotPath_003
 * @tc.desc: EtsRuntime test for GetAotPath with aotCompileStatusMap having true values.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, GetAotPath_003, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    Runtime::Options options;
    options.arkNativeFilePath = "arm64/";
    options.moduleName = "entry";
    options.aotCompileStatusMap = {
        {"entry", static_cast<int32_t>(AppExecFwk::AOTCompileStatus::IDLE_COMPILE_SUCCESS)},
        {"module1", static_cast<int32_t>(AppExecFwk::AOTCompileStatus::IDLE_COMPILE_SUCCESS)},
        {"module2", static_cast<int32_t>(AppExecFwk::AOTCompileStatus::NOT_COMPILED)}
    };
    options.commonHspBundleInfos = {};
    auto result = etsRuntime->GetAotPath(options);
    // Should contain entry.an and module1.an (true values only)
    EXPECT_NE(result.find("entry.an"), std::string::npos);
    EXPECT_NE(result.find("module1.an"), std::string::npos);
    EXPECT_EQ(result.find("module2.an"), std::string::npos);
}

/**
 * @tc.name: GetAotPath_004
 * @tc.desc: EtsRuntime test for GetAotPath with aotCompileStatusMap all false.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, GetAotPath_004, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    Runtime::Options options;
    options.arkNativeFilePath = "arm64/";
    options.moduleName = "test";
    options.aotCompileStatusMap = {
        {"module1", static_cast<int32_t>(AppExecFwk::AOTCompileStatus::NOT_COMPILED)},
        {"module2", static_cast<int32_t>(AppExecFwk::AOTCompileStatus::NOT_COMPILED)}
    };
    options.commonHspBundleInfos = {};
    auto result = etsRuntime->GetAotPath(options);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: GetAotPath_005
 * @tc.desc: EtsRuntime test for GetAotPath with commonHspBundleInfos ARKTS_MODE_DYNAMIC.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, GetAotPath_005, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    Runtime::Options options;
    options.arkNativeFilePath = "arm64/";
    options.moduleName = "entry";
    options.aotCompileStatusMap = {};
    // ARKTS_MODE_DYNAMIC should be skipped
    std::vector<OHOS::AbilityRuntime::CommonHspBundleInfo> hspInfos = {
        {1001, static_cast<int32_t>(AppExecFwk::AOTCompileStatus::NOT_COMPILED), "com.example.hsp1", "hspModule1",
        "/system/app/hsp1.hap", AppExecFwk::Constants::ARKTS_MODE_DYNAMIC}
    };
    options.commonHspBundleInfos = hspInfos;
    auto result = etsRuntime->GetAotPath(options);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: GetAotPath_006
 * @tc.desc: EtsRuntime test for GetAotPath with commonHspBundleInfos static mode, system path.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, GetAotPath_006, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    Runtime::Options options;
    options.arkNativeFilePath = "arm64/";
    options.moduleName = "entry";
    options.aotCompileStatusMap = {};
    // Static mode with system app path - should be included
    std::vector<OHOS::AbilityRuntime::CommonHspBundleInfo> hspInfos = {
        {1001, static_cast<int32_t>(AppExecFwk::AOTCompileStatus::NOT_COMPILED), "com.example.hsp1", "hspModule1",
        "/system/app/hsp1.hap", AppExecFwk::Constants::ARKTS_MODE_STATIC}
    };
    options.commonHspBundleInfos = hspInfos;
    auto result = etsRuntime->GetAotPath(options);
    // Should not contain the outer HSP path
    EXPECT_EQ(result.find("/data/service/el1/public/for-all-app/shared_bundles_ark_cache/"), std::string::npos);
    EXPECT_EQ(result.find("com.example.hsp1"), std::string::npos);
    EXPECT_EQ(result.find("hspModule1.an"), std::string::npos);
}

/**
 * @tc.name: GetAotPath_007
 * @tc.desc: EtsRuntime test for GetAotPath with commonHspBundleInfos non-system path without slash.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, GetAotPath_007, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    Runtime::Options options;
    options.arkNativeFilePath = "arm64/";
    options.moduleName = "entry";
    options.aotCompileStatusMap = {};
    // Non-system path (doesn't start with /system/app/) and has no '/' - should be skipped
    std::vector<OHOS::AbilityRuntime::CommonHspBundleInfo> hspInfos = {
        {1001, static_cast<int32_t>(AppExecFwk::AOTCompileStatus::NOT_COMPILED), "com.example.hsp1", "hspModule1",
        "relativePathWithoutSlash", AppExecFwk::Constants::ARKTS_MODE_STATIC}
    };
    options.commonHspBundleInfos = hspInfos;
    auto result = etsRuntime->GetAotPath(options);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: GetAotPath_008
 * @tc.desc: EtsRuntime test for GetAotPath with commonHspBundleInfos non-system path with slash.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, GetAotPath_008, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    Runtime::Options options;
    options.arkNativeFilePath = "arm64/";
    options.moduleName = "entry";
    options.aotCompileStatusMap = {};
    // Non-system path but has '/' - should be included
    std::vector<OHOS::AbilityRuntime::CommonHspBundleInfo> hspInfos = {
        {1001, static_cast<int32_t>(AppExecFwk::AOTCompileStatus::NOT_COMPILED), "com.example.hsp1", "hspModule1",
        "/data/app/hsp1.hap", AppExecFwk::Constants::ARKTS_MODE_STATIC}
    };
    options.commonHspBundleInfos = hspInfos;
    auto result = etsRuntime->GetAotPath(options);
    // Should not contain the outer HSP path
    EXPECT_EQ(result.find("com.example.hsp1"), std::string::npos);
    EXPECT_EQ(result.find("hspModule1.an"), std::string::npos);
}

/**
 * @tc.name: GetAotPath_009
 * @tc.desc: EtsRuntime test for GetAotPath with multiple commonHspBundleInfos entries.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, GetAotPath_009, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    Runtime::Options options;
    options.arkNativeFilePath = "arm64/";
    options.moduleName = "entry";
    options.aotCompileStatusMap = {{"entry", true}};
    // Mix of different HSP types
    std::vector<OHOS::AbilityRuntime::CommonHspBundleInfo> hspInfos = {
        // Dynamic mode - should be skipped
        {1001, static_cast<int32_t>(AppExecFwk::AOTCompileStatus::NOT_COMPILED), "com.example.hsp1", "hspModule1",
            "/system/app/hsp1.hap", AppExecFwk::Constants::ARKTS_MODE_DYNAMIC},
        // Static mode, system path - should be included
        {1002, static_cast<int32_t>(AppExecFwk::AOTCompileStatus::NOT_COMPILED), "com.example.hsp2", "hspModule2",
            "/system/app/hsp2.hap", AppExecFwk::Constants::ARKTS_MODE_STATIC},
        // Static mode, non-system path with slash - should be included
        {1003, static_cast<int32_t>(AppExecFwk::AOTCompileStatus::INSTALL_COMPILE_SUCCESS), "com.example.hsp3",
            "hspModule3", "/data/app/hsp3.hap", AppExecFwk::Constants::ARKTS_MODE_STATIC},
        // Non-system path without slash - should be skipped
        {1004, static_cast<int32_t>(AppExecFwk::AOTCompileStatus::NOT_COMPILED), "com.example.hsp4", "hspModule4",
            "relativePath", AppExecFwk::Constants::ARKTS_MODE_STATIC}
    };
    options.commonHspBundleInfos = hspInfos;
    auto result = etsRuntime->GetAotPath(options);
    // Should contain entry.an (from aotCompileStatusMap)
    EXPECT_NE(result.find("entry.an"), std::string::npos);
    // Should not contain hsp2 (static mode, system path) since compile status is not compiled
    EXPECT_EQ(result.find("hspModule2.an"), std::string::npos);
    // Should contain hsp3 (non-system path with slash)
    EXPECT_NE(result.find("hspModule3.an"), std::string::npos);
    // Should NOT contain hsp1 (dynamic mode)
    EXPECT_EQ(result.find("hspModule1.an"), std::string::npos);
    // Should NOT contain hsp4 (relative path without slash)
    EXPECT_EQ(result.find("hspModule4.an"), std::string::npos);
    // Check that paths are separated by colons
    EXPECT_NE(result.find(":"), std::string::npos);
}

/**
 * @tc.name: GetAotPath_010
 * @tc.desc: EtsRuntime test for GetAotPath with complete options including versionCode.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, GetAotPath_010, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    Runtime::Options options;
    options.arkNativeFilePath = "arm64/";
    options.moduleName = "entry";
    options.aotCompileStatusMap = {
        {"entry", static_cast<int32_t>(AppExecFwk::AOTCompileStatus::IDLE_COMPILE_SUCCESS)}
    };
    std::vector<OHOS::AbilityRuntime::CommonHspBundleInfo> hspInfos = {
        {1002003, static_cast<int32_t>(AppExecFwk::AOTCompileStatus::IDLE_COMPILE_SUCCESS), "com.example.hsp",
            "hspModule", "/system/app/hsp.hap", AppExecFwk::Constants::ARKTS_MODE_STATIC}
    };
    options.commonHspBundleInfos = hspInfos;
    auto result = etsRuntime->GetAotPath(options);
    // Should include version in the path
    EXPECT_NE(result.find("1002003"), std::string::npos);
    // Should have proper path structure
    EXPECT_NE(result.find("/data/service/el1/public/for-all-app/shared_bundles_ark_cache/"), std::string::npos);
}

/**
 * @tc.name: IsAotCompiledSuccess_001
 * @tc.desc: Test IsAotCompiledSuccess with IDLE_COMPILE_SUCCESS status.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, IsAotCompiledSuccess_001, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    bool result = etsRuntime->IsAotCompiledSuccess(
        static_cast<int32_t>(AppExecFwk::AOTCompileStatus::IDLE_COMPILE_SUCCESS));
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsAotCompiledSuccess_002
 * @tc.desc: Test IsAotCompiledSuccess with INSTALL_COMPILE_SUCCESS status.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, IsAotCompiledSuccess_002, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    bool result = etsRuntime->IsAotCompiledSuccess(
        static_cast<int32_t>(AppExecFwk::AOTCompileStatus::INSTALL_COMPILE_SUCCESS));
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsAotCompiledSuccess_003
 * @tc.desc: Test IsAotCompiledSuccess with NOT_COMPILED status.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, IsAotCompiledSuccess_003, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    bool result = etsRuntime->IsAotCompiledSuccess(
        static_cast<int32_t>(AppExecFwk::AOTCompileStatus::NOT_COMPILED));
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsAotCompiledSuccess_004
 * @tc.desc: Test IsAotCompiledSuccess with COMPILE_FAILED status.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, IsAotCompiledSuccess_004, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    bool result = etsRuntime->IsAotCompiledSuccess(
        static_cast<int32_t>(AppExecFwk::AOTCompileStatus::COMPILE_FAILED));
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsAotCompiledSuccess_005
 * @tc.desc: Test IsAotCompiledSuccess with COMPILE_CRASH status.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, IsAotCompiledSuccess_005, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    bool result = etsRuntime->IsAotCompiledSuccess(
        static_cast<int32_t>(AppExecFwk::AOTCompileStatus::COMPILE_CRASH));
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsAotCompiledSuccess_006
 * @tc.desc: Test IsAotCompiledSuccess with COMPILE_CANCELLED status.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, IsAotCompiledSuccess_006, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    bool result = etsRuntime->IsAotCompiledSuccess(
        static_cast<int32_t>(AppExecFwk::AOTCompileStatus::COMPILE_CANCELLED));
    EXPECT_FALSE(result);
}

/**
 * @tc.name: GetAotPath_011
 * @tc.desc: Test GetAotPath with IDLE_COMPILE_SUCCESS status in aotCompileStatusMap.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, GetAotPath_011, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    Runtime::Options options;
    options.arkNativeFilePath = "arm64/";
    options.moduleName = "entry";
    options.aotCompileStatusMap = {
        {"entry", static_cast<int32_t>(AppExecFwk::AOTCompileStatus::IDLE_COMPILE_SUCCESS)},
        {"module1", static_cast<int32_t>(AppExecFwk::AOTCompileStatus::COMPILE_FAILED)}
    };
    options.commonHspBundleInfos = {};
    auto result = etsRuntime->GetAotPath(options);
    EXPECT_NE(result.find("entry.an"), std::string::npos);
    EXPECT_EQ(result.find("module1.an"), std::string::npos);
}

/**
 * @tc.name: GetAotPath_012
 * @tc.desc: Test GetAotPath with INSTALL_COMPILE_SUCCESS status in commonHspBundleInfos.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, GetAotPath_012, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    Runtime::Options options;
    options.arkNativeFilePath = "arm64/";
    options.moduleName = "entry";
    options.aotCompileStatusMap = {};
    std::vector<OHOS::AbilityRuntime::CommonHspBundleInfo> hspInfos = {
        {1001, static_cast<int32_t>(AppExecFwk::AOTCompileStatus::INSTALL_COMPILE_SUCCESS),
         "com.example.hsp1", "hspModule1", "/system/app/hsp1.hap",
         AppExecFwk::Constants::ARKTS_MODE_STATIC}
    };
    options.commonHspBundleInfos = hspInfos;
    auto result = etsRuntime->GetAotPath(options);
    EXPECT_NE(result.find("hspModule1.an"), std::string::npos);
}

/**
 * @tc.name: GetAotPath_013
 * @tc.desc: Test GetAotPath loads preinstall host AOT an file when aot status is failed.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, GetAotPath_013, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    Runtime::Options options;
    options.bundleName = TEST_HOST_AOT_BUNDLE_NAME;
    options.aotCompileStatusMap = {
        {"entry", static_cast<int32_t>(AppExecFwk::AOTCompileStatus::NOT_COMPILED)}
    };
    options.commonHspBundleInfos = {};
    CreateHostAotAnFile(TEST_HOST_AOT_BUNDLE_NAME, "entry");

    auto result = etsRuntime->GetAotPath(options);
    EXPECT_EQ(result, GetHostAotAnPath(TEST_HOST_AOT_BUNDLE_NAME, "entry"));
}

/**
 * @tc.name: GetAotPath_014
 * @tc.desc: Test GetAotPath skips missing preinstall host AOT an file when aot status is failed.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, GetAotPath_014, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    Runtime::Options options;
    options.bundleName = TEST_HOST_AOT_BUNDLE_NAME;
    options.aotCompileStatusMap = {
        {"entry", static_cast<int32_t>(AppExecFwk::AOTCompileStatus::COMPILE_FAILED)}
    };
    options.commonHspBundleInfos = {};

    auto result = etsRuntime->GetAotPath(options);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: GetAotPath_015
 * @tc.desc: Test GetAotPath only loads existing preinstall host AOT an files for failed modules.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, GetAotPath_015, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    Runtime::Options options;
    options.bundleName = TEST_HOST_AOT_BUNDLE_NAME;
    options.aotCompileStatusMap = {
        {"entry", static_cast<int32_t>(AppExecFwk::AOTCompileStatus::COMPILE_FAILED)},
        {"feature", static_cast<int32_t>(AppExecFwk::AOTCompileStatus::COMPILE_FAILED)}
    };
    options.commonHspBundleInfos = {};
    CreateHostAotAnFile(TEST_HOST_AOT_BUNDLE_NAME, "entry");

    auto result = etsRuntime->GetAotPath(options);
    EXPECT_NE(result.find(GetHostAotAnPath(TEST_HOST_AOT_BUNDLE_NAME, "entry")), std::string::npos);
    EXPECT_EQ(result.find(GetHostAotAnPath(TEST_HOST_AOT_BUNDLE_NAME, "feature")), std::string::npos);
}

/**
 * @tc.name: GetAotPath_016
 * @tc.desc: Test successful AOT status still loads original sandbox an path.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, GetAotPath_016, TestSize.Level1)
{
    auto etsRuntime = std::make_unique<ETSRuntime>();
    Runtime::Options options;
    options.bundleName = TEST_HOST_AOT_BUNDLE_NAME;
    options.aotCompileStatusMap = {
        {"entry", static_cast<int32_t>(AppExecFwk::AOTCompileStatus::IDLE_COMPILE_SUCCESS)}
    };
    options.commonHspBundleInfos = {};
    CreateHostAotAnFile(TEST_HOST_AOT_BUNDLE_NAME, "entry");

    auto result = etsRuntime->GetAotPath(options);
    EXPECT_EQ(result, "/data/storage/ark-cache/arm64/entry.an");
    EXPECT_EQ(result.find(GetHostAotAnPath(TEST_HOST_AOT_BUNDLE_NAME, "entry")), std::string::npos);
}

/**
 * @tc.name: ForceFullGC_0100
 * @tc.desc: EtsRuntimeTest test for ForceFullGC.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, ForceFullGC_0100, TestSize.Level2)
{
    std::unique_ptr<ETSRuntime> etsRuntime = std::make_unique<ETSRuntime>();
    etsRuntime->ForceFullGC(0);
    ASSERT_NE(etsRuntime, nullptr);
}

/**
 * @tc.name: XGC_0100
 * @tc.desc: EtsRuntimeTest test for XGC.
 * @tc.type: FUNC
 */
HWTEST_F(EtsRuntimeTest, XGC_0100, TestSize.Level2)
{
    std::unique_ptr<ETSRuntime> etsRuntime = std::make_unique<ETSRuntime>();
    etsRuntime->XGC();
    ASSERT_NE(etsRuntime, nullptr);
}
} // namespace AbilityRuntime
} // namespace OHOS
