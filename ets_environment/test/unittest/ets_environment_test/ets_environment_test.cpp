/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <cstdarg>
#include <gtest/gtest.h>
#include <dlfcn.h>
#include <gtest/hwext/gtest-multithread.h>
#include <string>

#include "runtime.h"
#define private public
#include "ets_environment.h"
#undef private

#include "mock_ani_env.h"

using namespace testing;
using namespace testing::ext;
using namespace testing::mt;

namespace {
bool g_callbackModuleFlag;
}

namespace OHOS {
namespace EtsEnv {
const std::string TEST_ABILITY_NAME = "ContactsDataAbility";

class EtsEnvironmentTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void EtsEnvironmentTest::SetUpTestCase() {}

void EtsEnvironmentTest::TearDownTestCase() {}

void EtsEnvironmentTest::SetUp() {}

void EtsEnvironmentTest::TearDown() {}

namespace {
void CallBackModuleFunc()
{
    g_callbackModuleFlag = true;
}
} // namespace

/**
 * @tc.name: LoadBootPathFile_0100
 * @tc.desc: LoadBootPathFile.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, LoadBootPathFile_0100, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    std::string str = "LoadBootPathFile";
    bool bVal = etsEnv->LoadBootPathFile(str);
    EXPECT_EQ(bVal, true);
}
/**
 * @tc.name: LoadRuntimeApis_0100
 * @tc.desc: LoadRuntimeApis.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, LoadRuntimeApis_0100, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    bool bVal = etsEnv->LoadRuntimeApis();
    EXPECT_EQ(bVal, true);
}

/**
 * @tc.name: GetBuildId_0100
 * @tc.desc: Test GetBuildId with stack containing non-parseable lines.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, GetBuildId_0100, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    std::string stack = "NonParseableLineWithoutSpace\n"
                        "#00 pc 000000000001a0b8 /system/lib64/libutils.so\n"
                        "AnotherLineWithoutSpace\n";
    std::string result = etsEnv->GetBuildId(stack);
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("/system/lib64/libutils.so"), std::string::npos);
    EXPECT_EQ(result.find("NonParseableLineWithoutSpace"), std::string::npos);
    EXPECT_EQ(result.find("AnotherLineWithoutSpace"), std::string::npos);
    size_t expectedLines = 1;
    size_t resultLines = std::count(result.begin(), result.end(), '\n');
    EXPECT_EQ(resultLines, expectedLines);
}

/**
 * @tc.name: GetBuildId_0200
 * @tc.desc: Test GetBuildId with empty input.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, GetBuildId_0200, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    std::string stack = "";
    std::string result = etsEnv->GetBuildId(stack);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetBuildId_0300
 * @tc.desc: Test GetBuildId with stack containing only newlines.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, GetBuildId_0300, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    std::string stack = "\n\n\n";
    std::string result = etsEnv->GetBuildId(stack);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: RegisterUncaughtExceptionHandler_0100
 * @tc.desc: Test basic registration and triggering of uncaught exception handler.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, RegisterUncaughtExceptionHandler_0100, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    g_callbackModuleFlag = false;
    ETSUncaughtExceptionInfo handlerInfo;
    handlerInfo.uncaughtTask = [](const std::string& summary, const ETSErrorObject& errorObj) {
        g_callbackModuleFlag = true;
    };
    etsEnv->RegisterUncaughtExceptionHandler(handlerInfo);
    ETSErrorObject errorObj;
    errorObj.name = "TestError";
    errorObj.message = "Test error message";
    errorObj.stack = "Test stack trace";
    etsEnv->uncaughtExceptionInfo_.uncaughtTask("Test summary", errorObj);
    EXPECT_TRUE(g_callbackModuleFlag);
}

/**
 * @tc.name: GetAniEnv_0100
 * @tc.desc: GetAniEnv.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, GetAniEnv_0100, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    etsEnv->vmEntry_.aniVm_ = nullptr;
    etsEnv->vmEntry_.aniEnv_ = nullptr;
    auto result = etsEnv->GetAniEnv();
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: LoadSymbolCreateVM_0100
 * @tc.desc: Test LoadSymbolCreateVM when dlsym returns nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, LoadSymbolCreateVM_0100, TestSize.Level1)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    void* handle = dlopen(nullptr, RTLD_LAZY);
    ASSERT_NE(handle, nullptr);
    ETSRuntimeAPI apis = {};
    bool result = etsEnv->LoadSymbolCreateVM(handle, apis);
    dlclose(handle);
    EXPECT_TRUE(result);
    EXPECT_NE(apis.ANI_CreateVM, nullptr);
}

/**
 * @tc.name: LoadSymbolCreateVM_0200
 * @tc.desc: Test LoadSymbolCreateVM returns false when symbol is not found.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, LoadSymbolCreateVM_0200, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    void* invalidHandle = reinterpret_cast<void*>(0x1);
    ETSRuntimeAPI apis = {};
    bool result = etsEnv->LoadSymbolCreateVM(invalidHandle, apis);
    EXPECT_FALSE(result);
    EXPECT_EQ(apis.ANI_CreateVM, nullptr);
}

/**
 * @tc.name: LoadSymbolANIGetCreatedVMs_0100
 * @tc.desc: Test LoadSymbolANIGetCreatedVMs when symbol is not found.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, LoadSymbolANIGetCreatedVMs_0100, TestSize.Level1)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    void* handle = dlopen(nullptr, RTLD_LAZY);
    ASSERT_NE(handle, nullptr);
    ETSRuntimeAPI apis = {};
    bool result = etsEnv->LoadSymbolANIGetCreatedVMs(handle, apis);
    dlclose(handle);
    EXPECT_TRUE(result);
    EXPECT_NE(apis.ANI_GetCreatedVMs, nullptr);
}

/**
 * @tc.name: LoadSymbolANIGetCreatedVMs_0200
 * @tc.desc: Test LoadSymbolANIGetCreatedVMs returns false when symbol is not found.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, LoadSymbolANIGetCreatedVMs_0200, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    void* invalidHandle = reinterpret_cast<void*>(0x1);
    ETSRuntimeAPI apis = {};
    bool result = etsEnv->LoadSymbolANIGetCreatedVMs(invalidHandle, apis);
    EXPECT_FALSE(result);
    EXPECT_EQ(apis.ANI_GetCreatedVMs, nullptr);
}

/**
 * @tc.name: Initialize_0100
 * @tc.desc: Test Initialize can be called without crash.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, Initialize_0100, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    bool result = etsEnv->Initialize(nullptr, false);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: HandleUncaughtError_0100
 * @tc.desc: Test HandleUncaughtError can be called without crash.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, HandleUncaughtError_0100, TestSize.Level1)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    bool result = etsEnv->HandleUncaughtError();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: LoadAbcLinker_0100
 * @tc.desc: LoadAbcLinker returns false when env is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, LoadAbcLinker_0100, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    ani_class cls = nullptr;
    ani_object obj = nullptr;
    bool result = etsEnv->LoadAbcLinker(nullptr, "testModule", cls, obj);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: LoadAbcLinker_0200
 * @tc.desc: Test LoadAbcLinker.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, LoadAbcLinker_0200, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    etsEnv->vmEntry_.abcLinkerClass_ = reinterpret_cast<ani_class>(0x123);
    etsEnv->vmEntry_.abcLinkerRef_ = reinterpret_cast<ani_ref>(0x123);

    OHOS::AbilityRuntime::CommonHspBundleInfo info;
    info.versionCode = 0;
    info.bundleName = "bundle";
    info.moduleName = "module";
    info.hapPath = "/data/app/el1/bundle/public/hsp/test.hsp";
    info.moduleArkTSMode = "";
    etsEnv->commonHspBundleInfos_.clear();
    etsEnv->commonHspBundleInfos_.push_back(info);

    MockAniEnv mockEnv;
    mockEnv.GetState().callMethodStatus = ANI_ERROR;

    ani_class cls = nullptr;
    ani_object obj = nullptr;
    bool result = etsEnv->LoadAbcLinker(mockEnv.GetEnv(), "testModule", cls, obj);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: LoadAbcLinker_0300
 * @tc.desc: Test LoadAbcLinker.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, LoadAbcLinker_0300, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    etsEnv->vmEntry_.abcLinkerClass_ = reinterpret_cast<ani_class>(0x123);
    etsEnv->vmEntry_.abcLinkerRef_ = reinterpret_cast<ani_ref>(0x123);

    MockAniEnv mockEnv;
    mockEnv.GetState().stringNewStatus = ANI_ERROR;

    ani_class cls = nullptr;
    ani_object obj = nullptr;
    bool result = etsEnv->LoadAbcLinker(mockEnv.GetEnv(), "testModule", cls, obj);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: LoadAbcLinker_0400
 * @tc.desc: Test LoadAbcLinker.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, LoadAbcLinker_0400, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    etsEnv->vmEntry_.abcLinkerClass_ = reinterpret_cast<ani_class>(0x123);
    etsEnv->vmEntry_.abcLinkerRef_ = reinterpret_cast<ani_ref>(0x123);
    etsEnv->vmEntry_.abcCacheMap_.emplace("testModule", true);
    etsEnv->vmEntry_.isSetDefaultInteropLinker_ = false;

    MockAniEnv mockEnv;
    mockEnv.GetState().findClassStatus = ANI_ERROR;

    ani_class cls = nullptr;
    ani_object obj = nullptr;
    bool result = etsEnv->LoadAbcLinker(mockEnv.GetEnv(), "testModule", cls, obj);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: LoadAbcLinker_0500
 * @tc.desc: Test LoadAbcLinker.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, LoadAbcLinker_0500, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    etsEnv->vmEntry_.abcLinkerClass_ = reinterpret_cast<ani_class>(0x123);
    etsEnv->vmEntry_.abcLinkerRef_ = reinterpret_cast<ani_ref>(0x123);
    etsEnv->vmEntry_.abcCacheMap_.emplace("testModule", true);
    etsEnv->vmEntry_.isSetDefaultInteropLinker_ = false;

    MockAniEnv mockEnv;
    mockEnv.GetState().classCallStaticMethodStatus = ANI_ERROR;

    ani_class cls = nullptr;
    ani_object obj = nullptr;
    bool result = etsEnv->LoadAbcLinker(mockEnv.GetEnv(), "testModule", cls, obj);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: LoadAbcLinker_0600
 * @tc.desc: Test LoadAbcLinker.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, LoadAbcLinker_0600, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    etsEnv->vmEntry_.abcLinkerClass_ = reinterpret_cast<ani_class>(0x123);
    etsEnv->vmEntry_.abcLinkerRef_ = reinterpret_cast<ani_ref>(0x123);
    etsEnv->vmEntry_.abcCacheMap_.emplace("testModule", true);
    etsEnv->vmEntry_.isSetDefaultInteropLinker_ = true;

    MockAniEnv mockEnv;

    ani_class cls = nullptr;
    ani_object obj = nullptr;
    bool result = etsEnv->LoadAbcLinker(mockEnv.GetEnv(), "testModule", cls, obj);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: LoadAbcLinker_0700
 * @tc.desc: Test LoadAbcLinker.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, LoadAbcLinker_0700, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    etsEnv->vmEntry_.abcLinkerClass_ = nullptr;
    etsEnv->vmEntry_.abcLinkerRef_ = reinterpret_cast<ani_ref>(0x123);

    MockAniEnv mockEnv;

    ani_class cls = nullptr;
    ani_object obj = nullptr;
    bool result = etsEnv->LoadAbcLinker(mockEnv.GetEnv(), "testModule", cls, obj);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: LoadAbcLinker_0800
 * @tc.desc: Test LoadAbcLinker.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, LoadAbcLinker_0800, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    etsEnv->vmEntry_.abcLinkerClass_ = reinterpret_cast<ani_class>(0x123);
    etsEnv->vmEntry_.abcLinkerRef_ = nullptr;

    MockAniEnv mockEnv;

    ani_class cls = nullptr;
    ani_object obj = nullptr;
    bool result = etsEnv->LoadAbcLinker(mockEnv.GetEnv(), "testModule", cls, obj);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: PreloadModule_0100
 * @tc.desc: PreloadModule returns false when env is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, PreloadModule_0100, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    bool result = etsEnv->PreloadModule("testModule");
    EXPECT_FALSE(result);
}

/**
 * @tc.name: LoadModule_0100
 * @tc.desc: LoadModule returns false when env is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, LoadModule_0100, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    void *cls = nullptr;
    void *obj = nullptr;
    void *ref = nullptr;
    bool result = etsEnv->LoadModule("testModule", "testModule", cls, obj, ref);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: FinishPreload_0100
 * @tc.desc: Sts environment FinishPreload.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, FinishPreload_0100, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    auto result = etsEnv->FinishPreload(nullptr);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: PostFork_0100
 * @tc.desc: Sts environment PostFork.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, PostFork_0100, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    void *napiEnv = nullptr;
    std::string aotPath = "aotPath";
    std::vector<std::string> appInnerHspPathList;
    std::vector<OHOS::AbilityRuntime::CommonHspBundleInfo> commonHspBundleInfos;
    std::shared_ptr<OHOS::AppExecFwk::EventRunner> eventRunner;
    auto result = etsEnv->PostFork(napiEnv, aotPath, appInnerHspPathList, commonHspBundleInfos, eventRunner);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: PreloadSystemClass_0100
 * @tc.desc: Sts environment PreloadSystemClass.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, PreloadSystemClass_0100, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    std::string className = "className";
    auto result = etsEnv->PreloadSystemClass(className.c_str());
    EXPECT_FALSE(result);
}

/**
 * @tc.name: GetDebuggerPostTask_0100
 * @tc.desc: Sts environment GetDebuggerPostTask.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, GetDebuggerPostTask_0100, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    auto task = etsEnv->GetDebuggerPostTask();
    ASSERT_NE(task, nullptr);
}

/**
 * @tc.name: ParseHdcRegisterOption_0100
 * @tc.desc: Js environment ParseHdcRegisterOption.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, ParseHdcRegisterOption_0100, TestSize.Level2)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    std::string option1 = "";
    int result1 = etsEnv->ParseHdcRegisterOption(option1);
    ASSERT_EQ(result1, -1);
    std::string option2 = "@";
    int result2 = etsEnv->ParseHdcRegisterOption(option2);
    ASSERT_EQ(result2, -1);
    std::string option3 = ":";
    int result3 = etsEnv->ParseHdcRegisterOption(option3);
    ASSERT_EQ(result3, -1);
    std::string option4 = "ark:123@Debugger";
    int result4 = etsEnv->ParseHdcRegisterOption(option4);
    ASSERT_EQ(result4, 123);
    std::string option5 = "ark:123@456@Debugger";
    int result5 = etsEnv->ParseHdcRegisterOption(option5);
    ASSERT_EQ(result5, 456);
}

/**
 * @tc.name: SetHspAbcFiles_0100
 * @tc.desc: Test SetHspAbcFiles.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, SetHspAbcFiles_0100, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    ani_env *env = nullptr;
    ani_object obj = nullptr;
    auto result = etsEnv->SetHspAbcFiles(env, obj);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: SetHspAbcFiles_0200
 * @tc.desc: Test SetHspAbcFiles.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, SetHspAbcFiles_0200, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    etsEnv->commonHspBundleInfos_.clear();
    etsEnv->appInnerHspPathList_.clear();

    MockAniEnv mockEnv;
    ani_object mockObj = reinterpret_cast<ani_object>(0x1);
    auto result = etsEnv->SetHspAbcFiles(mockEnv.GetEnv(), mockObj);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: SetHspAbcFiles_0300
 * @tc.desc: Test SetHspAbcFiles.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, SetHspAbcFiles_0300, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    OHOS::AbilityRuntime::CommonHspBundleInfo info;
    info.versionCode = 0;
    info.bundleName = "bundle";
    info.moduleName = "module";
    info.hapPath = "/data/app/el1/bundle/public/hsp/test.hsp";
    info.moduleArkTSMode = "";
    etsEnv->commonHspBundleInfos_.clear();
    etsEnv->commonHspBundleInfos_.push_back(info);
    etsEnv->appInnerHspPathList_.clear();

    const auto paths = etsEnv->GetHspPathList();
    etsEnv->vmEntry_.abcCacheMap_.clear();
    for (const auto &path : paths) {
        etsEnv->vmEntry_.abcCacheMap_.emplace(path, true);
    }

    MockAniEnv mockEnv;
    ani_object mockObj = reinterpret_cast<ani_object>(0x1);
    auto result = etsEnv->SetHspAbcFiles(mockEnv.GetEnv(), mockObj);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: SetHspAbcFiles_0400
 * @tc.desc: Test SetHspAbcFiles.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, SetHspAbcFiles_0400, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    OHOS::AbilityRuntime::CommonHspBundleInfo info;
    info.versionCode = 0;
    info.bundleName = "bundle";
    info.moduleName = "module";
    info.hapPath = "/data/app/el1/bundle/public/hsp/test.hsp";
    info.moduleArkTSMode = "";
    etsEnv->commonHspBundleInfos_.clear();
    etsEnv->commonHspBundleInfos_.push_back(info);

    MockAniEnv mockEnv;
    mockEnv.GetState().getUndefinedStatus = ANI_ERROR;

    ani_object mockObj = reinterpret_cast<ani_object>(0x1);
    auto result = etsEnv->SetHspAbcFiles(mockEnv.GetEnv(), mockObj);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: SetHspAbcFiles_0500
 * @tc.desc: Test SetHspAbcFiles.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, SetHspAbcFiles_0500, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    OHOS::AbilityRuntime::CommonHspBundleInfo info;
    info.versionCode = 0;
    info.bundleName = "bundle";
    info.moduleName = "module";
    info.hapPath = "/data/app/el1/bundle/public/hsp/test.hsp";
    info.moduleArkTSMode = "";
    etsEnv->commonHspBundleInfos_.clear();
    etsEnv->commonHspBundleInfos_.push_back(info);

    MockAniEnv mockEnv;
    mockEnv.GetState().callMethodStatus = ANI_ERROR;

    ani_object mockObj = reinterpret_cast<ani_object>(0x1);
    auto result = etsEnv->SetHspAbcFiles(mockEnv.GetEnv(), mockObj);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: SetHspAbcFiles_0600
 * @tc.desc: Test SetHspAbcFiles.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, SetHspAbcFiles_0600, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    OHOS::AbilityRuntime::CommonHspBundleInfo info;
    info.versionCode = 0;
    info.bundleName = "bundle";
    info.moduleName = "module";
    info.hapPath = "/data/app/el1/bundle/public/hsp/test.hsp";
    info.moduleArkTSMode = "";
    etsEnv->commonHspBundleInfos_.clear();
    etsEnv->commonHspBundleInfos_.push_back(info);

    MockAniEnv mockEnv;

    ani_object mockObj = reinterpret_cast<ani_object>(0x1);
    auto result = etsEnv->SetHspAbcFiles(mockEnv.GetEnv(), mockObj);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: InitAbcLinker_0100
 * @tc.desc: Test InitAbcLinker.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, InitAbcLinker_0100, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    ani_env *env = nullptr;
    auto result = etsEnv->InitAbcLinker(env);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: InitAbcLinker_0200
 * @tc.desc: Test InitAbcLinker.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, InitAbcLinker_0200, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    MockAniEnv mockEnv;
    mockEnv.GetState().findClassStatus = ANI_ERROR;

    auto result = etsEnv->InitAbcLinker(mockEnv.GetEnv());
    EXPECT_FALSE(result);
}

/**
 * @tc.name: InitAbcLinker_0300
 * @tc.desc: Test InitAbcLinker.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, InitAbcLinker_0300, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    MockAniEnv mockEnv;
    mockEnv.GetState().getUndefinedStatus = ANI_ERROR;

    auto result = etsEnv->InitAbcLinker(mockEnv.GetEnv());
    EXPECT_FALSE(result);
}

/**
 * @tc.name: InitAbcLinker_0400
 * @tc.desc: Test InitAbcLinker.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, InitAbcLinker_0400, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    MockAniEnv mockEnv;
    mockEnv.GetState().arrayNewStatus = ANI_ERROR;

    auto result = etsEnv->InitAbcLinker(mockEnv.GetEnv());
    EXPECT_FALSE(result);
}

/**
 * @tc.name: InitAbcLinker_0500
 * @tc.desc: Test InitAbcLinker.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, InitAbcLinker_0500, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    MockAniEnv mockEnv;
    mockEnv.GetState().classFindMethodStatus = ANI_ERROR;

    auto result = etsEnv->InitAbcLinker(mockEnv.GetEnv());
    EXPECT_FALSE(result);
}

/**
 * @tc.name: InitAbcLinker_0600
 * @tc.desc: Test InitAbcLinker.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, InitAbcLinker_0600, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    MockAniEnv mockEnv;
    mockEnv.GetState().objectNewStatus = ANI_ERROR;

    auto result = etsEnv->InitAbcLinker(mockEnv.GetEnv());
    EXPECT_FALSE(result);
}

/**
 * @tc.name: InitAbcLinker_0700
 * @tc.desc: Test InitAbcLinker.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, InitAbcLinker_0700, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    MockAniEnv mockEnv;
    mockEnv.GetState().globalRefCreateStatus = ANI_ERROR;

    auto result = etsEnv->InitAbcLinker(mockEnv.GetEnv());
    EXPECT_FALSE(result);
}

/**
 * @tc.name: InitAbcLinker_0800
 * @tc.desc: Test InitAbcLinker.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, InitAbcLinker_0800, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    MockAniEnv mockEnv;
    auto result = etsEnv->InitAbcLinker(mockEnv.GetEnv());
    EXPECT_TRUE(result);
}

/**
 * @tc.name: AddAbcFiles_0100
 * @tc.desc: Test AddAbcFiles.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, AddAbcFiles_0100, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);
    ani_env *env = nullptr;
    std::string obj = "";
    auto result = etsEnv->AddAbcFiles(env, obj);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AddAbcFiles_0300
 * @tc.desc: Test AddAbcFiles.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, AddAbcFiles_0300, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    MockAniEnv mockEnv;
    mockEnv.GetState().stringNewStatus = ANI_ERROR;

    std::string obj;
    auto result = etsEnv->AddAbcFiles(mockEnv.GetEnv(), obj);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AddAbcFiles_0400
 * @tc.desc: Test AddAbcFiles.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, AddAbcFiles_0400, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    MockAniEnv mockEnv;
    mockEnv.GetState().getUndefinedStatus = ANI_ERROR;

    std::string obj;
    auto result = etsEnv->AddAbcFiles(mockEnv.GetEnv(), obj);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AddAbcFiles_0500
 * @tc.desc: Test AddAbcFiles.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, AddAbcFiles_0500, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    MockAniEnv mockEnv;
    mockEnv.GetState().arrayNewStatus = ANI_ERROR;

    std::string obj;
    auto result = etsEnv->AddAbcFiles(mockEnv.GetEnv(), obj);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AddAbcFiles_0600
 * @tc.desc: Test AddAbcFiles.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, AddAbcFiles_0600, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    MockAniEnv mockEnv;
    mockEnv.GetState().arraySetStatus = ANI_ERROR;

    std::string obj;
    auto result = etsEnv->AddAbcFiles(mockEnv.GetEnv(), obj);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AddAbcFiles_0700
 * @tc.desc: Test AddAbcFiles.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, AddAbcFiles_0700, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    MockAniEnv mockEnv;
    mockEnv.GetState().callMethodStatus = ANI_ERROR;

    std::string obj;
    auto result = etsEnv->AddAbcFiles(mockEnv.GetEnv(), obj);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AddAbcFiles_0800
 * @tc.desc: Test AddAbcFiles.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, AddAbcFiles_0800, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    MockAniEnv mockEnv;
    std::string obj;
    auto result = etsEnv->AddAbcFiles(mockEnv.GetEnv(), obj);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: Destructor_0100
 * @tc.desc: Test Destructor.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, Destructor_0100, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    etsEnv->vmEntry_.aniVm_ = nullptr;
    etsEnv->vmEntry_.aniEnv_ = nullptr;
    etsEnv->vmEntry_.abcLinkerRef_ = reinterpret_cast<ani_ref>(0x123);

    auto before = etsEnv->vmEntry_.abcLinkerRef_;
    etsEnv.reset();

    EXPECT_NE(before, nullptr);
}

/**
 * @tc.name: CreateRuntimeLinker_0100
 * @tc.desc: Test CreateRuntimeLinker.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, CreateRuntimeLinker_0100, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    MockAniEnv mockEnv;
    mockEnv.GetState().classFindMethodStatus = ANI_ERROR;

    ani_class cls = reinterpret_cast<ani_class>(0x1);
    ani_ref undefinedRef = reinterpret_cast<ani_ref>(0x2);
    ani_array refArray = reinterpret_cast<ani_array>(0x3);

    auto obj = etsEnv->CreateRuntimeLinker(mockEnv.GetEnv(), cls, undefinedRef, refArray);
    EXPECT_EQ(obj, nullptr);
}

/**
 * @tc.name: CreateRuntimeLinker_0200
 * @tc.desc: Test CreateRuntimeLinker.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, CreateRuntimeLinker_0200, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    MockAniEnv mockEnv;
    mockEnv.GetState().objectNewStatus = ANI_ERROR;

    ani_class cls = reinterpret_cast<ani_class>(0x1);
    ani_ref undefinedRef = reinterpret_cast<ani_ref>(0x2);
    ani_array refArray = reinterpret_cast<ani_array>(0x3);

    auto obj = etsEnv->CreateRuntimeLinker(mockEnv.GetEnv(), cls, undefinedRef, refArray);
    EXPECT_EQ(obj, nullptr);
}

/**
 * @tc.name: CreateRuntimeLinker_0300
 * @tc.desc: Test CreateRuntimeLinker.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, CreateRuntimeLinker_0300, TestSize.Level0)
{
    auto etsEnv = std::make_shared<ETSEnvironment>();
    ASSERT_NE(etsEnv, nullptr);

    MockAniEnv mockEnv;

    ani_class cls = reinterpret_cast<ani_class>(0x1);
    ani_ref undefinedRef = reinterpret_cast<ani_ref>(0x2);
    ani_array refArray = reinterpret_cast<ani_array>(0x3);

    auto obj = etsEnv->CreateRuntimeLinker(mockEnv.GetEnv(), cls, undefinedRef, refArray);
    EXPECT_NE(obj, nullptr);
}
} // namespace EtsEnv
} // namespace OHOS
