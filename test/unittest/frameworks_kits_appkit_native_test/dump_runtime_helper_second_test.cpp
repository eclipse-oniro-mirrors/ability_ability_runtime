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

#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <utility>

#define private public
#include "dump_runtime_helper.h"
#undef private

#include "app_loader.h"
#include "dfx_maps.h"
#include "dump_fd_guard.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "ohos_application.h"

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr const char *DEV_NULL_PATH = "/dev/null";
constexpr uint64_t UNOWNED_TAG = 0;
}

class DumpRuntimeHelperTestSecond : public testing::Test {
public:
    DumpRuntimeHelperTestSecond()
    {}
    ~DumpRuntimeHelperTestSecond()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DumpRuntimeHelperTestSecond::SetUpTestCase(void)
{}

void DumpRuntimeHelperTestSecond::TearDownTestCase(void)
{}

void DumpRuntimeHelperTestSecond::SetUp(void)
{}

void DumpRuntimeHelperTestSecond::TearDown(void)
{}

/**
 * @tc.number: SetAppFreezeFilterCallback_0100
 * @tc.name: SetAppFreezeFilterCallback
 * @tc.desc: Test whether SetAppFreezeFilterCallback and are called normally.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, SetAppFreezeFilterCallback_0100, TestSize.Level1)
{
    std::shared_ptr<OHOSApplication> application = nullptr;
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    helper->SetAppFreezeFilterCallback();
    EXPECT_EQ(application, nullptr);
}

/**
 * @tc.number: SetAppFreezeFilterCallback_0200
 * @tc.name: SetAppFreezeFilterCallback
 * @tc.desc: Test whether SetAppFreezeFilterCallback and are called normally.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, SetAppFreezeFilterCallback_0200, TestSize.Level1)
{
    std::shared_ptr<OHOSApplication> application = std::shared_ptr<OHOSApplication>(
        ApplicationLoader::GetInstance().GetApplicationByName());
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    helper->SetAppFreezeFilterCallback();
    EXPECT_NE(application, nullptr);

    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    application->SetRuntime(std::move(runtime));
    helper = std::make_shared<DumpRuntimeHelper>(application);
    helper->SetAppFreezeFilterCallback();
    EXPECT_NE(application, nullptr);
}

/**
 * @tc.number: DumpJsHeap_0300
 * @tc.name: DumpJsHeap
 * @tc.desc: Test whether DumpJsHeap and are called normally.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, DumpJsHeap_0300, TestSize.Level1)
{
    std::shared_ptr<OHOSApplication> application = nullptr;
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    OHOS::AppExecFwk::JsHeapDumpInfo info;
    info.tid = 1;
    helper->DumpJsHeap(info);
    EXPECT_EQ(application, nullptr);
}

/**
 * @tc.number: DumpJsHeap_0400
 * @tc.name: DumpJsHeap
 * @tc.desc: Test whether DumpJsHeap and are called normally.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, DumpJsHeap_0400, TestSize.Level1)
{
    std::shared_ptr<OHOSApplication> application = std::shared_ptr<OHOSApplication>(
        ApplicationLoader::GetInstance().GetApplicationByName());
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto helper = std::make_shared<DumpRuntimeHelper>(application, appInfo);
    OHOS::AppExecFwk::JsHeapDumpInfo info;
    info.tid = 1;
    helper->DumpJsHeap(info);
    EXPECT_NE(application, nullptr);

    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    application->SetRuntime(std::move(runtime));
    helper = std::make_shared<DumpRuntimeHelper>(application);
    helper->DumpJsHeap(info);
    EXPECT_NE(application, nullptr);

    info.needLeakobj = true;
    info.needSnapshot = true;
    helper->DumpJsHeap(info);
    EXPECT_EQ(info.needLeakobj, true);

    info.needSnapshot = false;
    info.needGc = true;
    helper->DumpJsHeap(info);
    EXPECT_EQ(info.needSnapshot, false);
}

/**
 * @tc.number: DumpJsHandleMap_0100
 * @tc.name: DumpJsHandleMap
 * @tc.desc: Test DumpJsHandleMap when application_ is nullptr (early return path).
 */
HWTEST_F(DumpRuntimeHelperTestSecond, DumpJsHandleMap_0100, TestSize.Level1)
{
    std::shared_ptr<OHOSApplication> application = nullptr;
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    EXPECT_EQ(helper->application_, nullptr);
    OHOS::AppExecFwk::JsHandleMapInfo info;
    info.tid = 1;
    helper->DumpJsHandleMap(info);
    EXPECT_EQ(helper->application_, nullptr);
}

/**
 * @tc.number: DumpJsHandleMap_0200
 * @tc.name: DumpJsHandleMap
 * @tc.desc: Test DumpJsHandleMap with null runtime then with JS runtime.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, DumpJsHandleMap_0200, TestSize.Level1)
{
    std::shared_ptr<OHOSApplication> application = std::shared_ptr<OHOSApplication>(
        ApplicationLoader::GetInstance().GetApplicationByName());
    ASSERT_NE(application, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto helper = std::make_shared<DumpRuntimeHelper>(application, appInfo);
    EXPECT_EQ(application->GetRuntime(), nullptr);
    OHOS::AppExecFwk::JsHandleMapInfo info;
    info.tid = 1;
    helper->DumpJsHandleMap(info);

    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    application->SetRuntime(std::move(runtime));
    ASSERT_NE(application->GetRuntime(), nullptr);
    EXPECT_EQ(application->GetRuntime()->GetLanguage(), AbilityRuntime::Runtime::Language::JS);
    helper = std::make_shared<DumpRuntimeHelper>(application);
    helper->DumpJsHandleMap(info);
}

/**
 * @tc.number: DumpJsHandleMap_0300
 * @tc.name: DumpJsHandleMap
 * @tc.desc: Test DumpJsHandleMap when Runtime::Create returns nullptr for UNKNOWN language.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, DumpJsHandleMap_0300, TestSize.Level1)
{
    std::shared_ptr<OHOSApplication> application = std::shared_ptr<OHOSApplication>(
        ApplicationLoader::GetInstance().GetApplicationByName());
    ASSERT_NE(application, nullptr);
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::UNKNOWN;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_EQ(runtime, nullptr);
    application->SetRuntime(std::move(runtime));
    EXPECT_EQ(application->GetRuntime(), nullptr);
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    OHOS::AppExecFwk::JsHandleMapInfo info;
    info.tid = 1;
    helper->DumpJsHandleMap(info);
}

/**
 * @tc.number: GetCheckList_0500
 * @tc.name: GetCheckList
 * @tc.desc: Test whether GetCheckList and are called normally.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, GetCheckList_0500, TestSize.Level1)
{
    std::string checkList = "";
    std::shared_ptr<OHOSApplication> application = std::shared_ptr<OHOSApplication>(
        ApplicationLoader::GetInstance().GetApplicationByName());
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    application->SetRuntime(std::move(runtime));
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    helper->GetCheckList(helper->application_->GetRuntime(), checkList);
    EXPECT_NE(checkList, "");
}

/**
 * @tc.number: GetJsLeakModule_0600
 * @tc.name: GetJsLeakModule
 * @tc.desc: Test whether GetJsLeakModule and are called normally.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, GetJsLeakModule_0600, TestSize.Level1)
{
    std::shared_ptr<OHOSApplication> application = std::shared_ptr<OHOSApplication>(
        ApplicationLoader::GetInstance().GetApplicationByName());
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    application->SetRuntime(std::move(runtime));
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    napi_env env = nullptr;
    napi_value global = nullptr;
    napi_get_global(env, &global);
    napi_value requireValue = helper->GetJsLeakModule(env, global);
    EXPECT_EQ(requireValue, nullptr);
}

/**
 * @tc.number: GetJsLeakModule_0700
 * @tc.name: GetJsLeakModule
 * @tc.desc: Test whether GetJsLeakModule and are called normally.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, GetJsLeakModule_0700, TestSize.Level1)
{
    std::shared_ptr<OHOSApplication> application = std::shared_ptr<OHOSApplication>(
        ApplicationLoader::GetInstance().GetApplicationByName());
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    application->SetRuntime(std::move(runtime));
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    AbilityRuntime::JsRuntime &jsruntime = static_cast<AbilityRuntime::JsRuntime&>(
        *helper->application_->GetRuntime());
    AbilityRuntime::HandleScope handleScope(jsruntime);
    auto env = jsruntime.GetNapiEnv();
    napi_value global = nullptr;
    napi_get_global(env, &global);
    napi_value requireValue = helper->GetJsLeakModule(env, global);
    EXPECT_NE(requireValue, nullptr);
}

/**
 * @tc.number: GetMethodCheck_0800
 * @tc.name: GetMethodCheck
 * @tc.desc: Test whether GetMethodCheck and are called normally.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, GetMethodCheck_0800, TestSize.Level1)
{
    std::shared_ptr<OHOSApplication> application = std::shared_ptr<OHOSApplication>(
        ApplicationLoader::GetInstance().GetApplicationByName());
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    application->SetRuntime(std::move(runtime));
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    napi_env env = nullptr;
    napi_value global = nullptr;
    napi_get_global(env, &global);
    napi_value requireValue = nullptr;
    napi_value result = helper->GetMethodCheck(env, requireValue, global);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.number: WriteCheckList_0900
 * @tc.name: WriteCheckList
 * @tc.desc: Test whether WriteCheckList and are called normally.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, WriteCheckList_0900, TestSize.Level1)
{
    std::shared_ptr<OHOSApplication> application = std::shared_ptr<OHOSApplication>(
        ApplicationLoader::GetInstance().GetApplicationByName());
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    application->SetRuntime(std::move(runtime));
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    std::string checkList = "test";
    helper->WriteCheckList(checkList);
    EXPECT_NE(checkList, "");
}

/**
 * @tc.number: DumpJsHeap_0500
 * @tc.name: DumpJsHeap with needProcDump
 * @tc.desc: Test DumpJsHeap when needProcDump is set to true.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, DumpJsHeap_0500, TestSize.Level1)
{
    std::shared_ptr<OHOSApplication> application = std::shared_ptr<OHOSApplication>(
        ApplicationLoader::GetInstance().GetApplicationByName());
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto helper = std::make_shared<DumpRuntimeHelper>(application, appInfo);
    OHOS::AppExecFwk::JsHeapDumpInfo info;
    info.tid = 1;
    info.needSnapshot = true;
    info.needProcDump = true;
    info.needBinary = true;
    helper->DumpJsHeap(info);
    EXPECT_NE(application, nullptr);

    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    application->SetRuntime(std::move(runtime));
    helper = std::make_shared<DumpRuntimeHelper>(application);
    helper->DumpJsHeap(info);
    EXPECT_NE(application, nullptr);
}

/**
 * @tc.number: DumpJsHeap_0600
 * @tc.name: DumpJsHeap with needProcDump false
 * @tc.desc: Test DumpJsHeap when needProcDump is set to false.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, DumpJsHeap_0600, TestSize.Level1)
{
    std::shared_ptr<OHOSApplication> application = std::shared_ptr<OHOSApplication>(
        ApplicationLoader::GetInstance().GetApplicationByName());
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto helper = std::make_shared<DumpRuntimeHelper>(application, appInfo);
    OHOS::AppExecFwk::JsHeapDumpInfo info;
    info.tid = 1;
    info.needSnapshot = true;
    info.needProcDump = false;
    info.needBinary = false;
    info.needGc = false;
    info.needClearNodeIdCache = false;
    helper->DumpJsHeap(info);
    EXPECT_NE(application, nullptr);

    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    application->SetRuntime(std::move(runtime));
    helper = std::make_shared<DumpRuntimeHelper>(application);
    helper->DumpJsHeap(info);
    EXPECT_NE(application, nullptr);
}

/**
 * @tc.number: DumpJsHeap_0700
 * @tc.name: DumpJsHeap with all params set
 * @tc.desc: Test DumpJsHeap when all JsHeapDumpInfo params are set.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, DumpJsHeap_0700, TestSize.Level1)
{
    std::shared_ptr<OHOSApplication> application = std::shared_ptr<OHOSApplication>(
        ApplicationLoader::GetInstance().GetApplicationByName());
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto helper = std::make_shared<DumpRuntimeHelper>(application, appInfo);
    OHOS::AppExecFwk::JsHeapDumpInfo info;
    info.tid = 1;
    info.pid = 1;
    info.needSnapshot = true;
    info.needProcDump = true;
    info.needBinary = true;
    info.needGc = true;
    info.needClearNodeIdCache = true;
    info.needLeakobj = false;
    helper->DumpJsHeap(info);
    EXPECT_NE(application, nullptr);

    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    application->SetRuntime(std::move(runtime));
    helper = std::make_shared<DumpRuntimeHelper>(application);
    helper->DumpJsHeap(info);
    EXPECT_NE(application, nullptr);
}

/**
 * @tc.number: DumpNativeHeap_1000
 * @tc.name: DumpNativeHeap
 * @tc.desc: Test DumpNativeHeap when application is nullptr.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, DumpNativeHeap_1000, TestSize.Level1)
{
    std::shared_ptr<OHOSApplication> application = nullptr;
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    OHOS::AppExecFwk::MemDumpInfo info;
    std::string dumpResult = "";
    helper->DumpNativeHeap(info, dumpResult);
    EXPECT_EQ(application, nullptr);
}

/**
 * @tc.number: DumpNativeHeap_1100
 * @tc.name: DumpNativeHeap
 * @tc.desc: Test DumpNativeHeap when info.needLeakobj is true.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, DumpNativeHeap_1100, TestSize.Level1)
{
    std::shared_ptr<OHOSApplication> application = std::shared_ptr<OHOSApplication>(
        ApplicationLoader::GetInstance().GetApplicationByName());
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    OHOS::AppExecFwk::MemDumpInfo info;
    info.needLeakobj = true;
    std::string dumpResult = "";

    helper->DumpNativeHeap(info, dumpResult);
    EXPECT_NE(application, nullptr);
}

/**
 * @tc.number: DumpNativeHeap_1200
 * @tc.name: DumpNativeHeap
 * @tc.desc: Test DumpNativeHeap when info.needLeakobj is false.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, DumpNativeHeap_1200, TestSize.Level1)
{
    std::shared_ptr<OHOSApplication> application = std::shared_ptr<OHOSApplication>(
        ApplicationLoader::GetInstance().GetApplicationByName());
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    OHOS::AppExecFwk::MemDumpInfo info;
    info.needLeakobj = false;
    std::string dumpResult = "";

    helper->DumpNativeHeap(info, dumpResult);
    EXPECT_NE(application, nullptr);
}

/**
 * @tc.number: DumpJsHeap_0800
 * @tc.name: DumpJsHeap with languageEnv dynamic
 * @tc.desc: Test DumpJsHeap when appInfo has dynamic arkTSMode.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, DumpJsHeap_0800, TestSize.Level1)
{
    std::shared_ptr<OHOSApplication> application = std::shared_ptr<OHOSApplication>(
        ApplicationLoader::GetInstance().GetApplicationByName());
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    appInfo->arkTSMode = OHOS::AbilityRuntime::CODE_LANGUAGE_ARKTS_1_0;
    auto helper = std::make_shared<DumpRuntimeHelper>(application, appInfo);
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    application->SetRuntime(std::move(runtime));
    helper = std::make_shared<DumpRuntimeHelper>(application, appInfo);
    OHOS::AppExecFwk::JsHeapDumpInfo info;
    info.tid = 1;
    info.needSnapshot = true;
    helper->DumpJsHeap(info);
    EXPECT_NE(application, nullptr);
}

/**
 * @tc.number: DumpJsHeap_0900
 * @tc.name: DumpJsHeap with languageEnv static
 * @tc.desc: Test DumpJsHeap when appInfo has static arkTSMode.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, DumpJsHeap_0900, TestSize.Level1)
{
    std::shared_ptr<OHOSApplication> application = std::shared_ptr<OHOSApplication>(
        ApplicationLoader::GetInstance().GetApplicationByName());
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    appInfo->arkTSMode = OHOS::AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2;
    auto helper = std::make_shared<DumpRuntimeHelper>(application, appInfo);
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::ETS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    application->SetRuntime(std::move(runtime));
    helper = std::make_shared<DumpRuntimeHelper>(application, appInfo);
    OHOS::AppExecFwk::JsHeapDumpInfo info;
    info.tid = 1;
    info.needSnapshot = true;
    helper->DumpJsHeap(info);
    EXPECT_NE(application, nullptr);
}

/**
 * @tc.number: DumpJsHeap_1000
 * @tc.name: DumpJsHeap with languageEnv hybrid
 * @tc.desc: Test DumpJsHeap when appInfo has hybrid arkTSMode.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, DumpJsHeap_1000, TestSize.Level1)
{
    std::shared_ptr<OHOSApplication> application = std::shared_ptr<OHOSApplication>(
        ApplicationLoader::GetInstance().GetApplicationByName());
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    appInfo->arkTSMode = OHOS::AbilityRuntime::CODE_LANGUAGE_ARKTS_HYBRID;
    auto helper = std::make_shared<DumpRuntimeHelper>(application, appInfo);
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::ETS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    application->SetRuntime(std::move(runtime));
    helper = std::make_shared<DumpRuntimeHelper>(application, appInfo);
    OHOS::AppExecFwk::JsHeapDumpInfo info;
    info.tid = 1;
    info.needSnapshot = true;
    helper->DumpJsHeap(info);
    EXPECT_NE(application, nullptr);
}

/**
 * @tc.number: DumpJsHeap_1100
 * @tc.name: DumpJsHeap with appInfo nullptr fallback to dynamic
 * @tc.desc: Test DumpJsHeap when appInfo is nullptr, languageEnv defaults to dynamic.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, DumpJsHeap_1100, TestSize.Level1)
{
    std::shared_ptr<OHOSApplication> application = std::shared_ptr<OHOSApplication>(
        ApplicationLoader::GetInstance().GetApplicationByName());
    auto helper = std::make_shared<DumpRuntimeHelper>(application);
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    application->SetRuntime(std::move(runtime));
    helper = std::make_shared<DumpRuntimeHelper>(application);
    OHOS::AppExecFwk::JsHeapDumpInfo info;
    info.tid = 1;
    info.needSnapshot = true;
    helper->DumpJsHeap(info);
    EXPECT_NE(application, nullptr);
}

/**
 * @tc.number: DumpFdGuard_0100
 * @tc.name: DumpFdGuard
 * @tc.desc: DumpFdGuard claims ownership on construction and closes the fd with the valid tag.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, DumpFdGuard_0100, TestSize.Level1)
{
    int32_t fd = open(DEV_NULL_PATH, O_RDWR | O_CLOEXEC);
    ASSERT_GE(fd, 0);
    ASSERT_EQ(fdsan_get_owner_tag(fd), UNOWNED_TAG);
    {
        DumpFdGuard fdGuard(fd);
        EXPECT_EQ(fdGuard.Get(), fd);
        EXPECT_EQ(fdsan_get_owner_tag(fd), static_cast<uint64_t>(DUMP_FD_SAN_TAG));
    }
    EXPECT_EQ(fdsan_get_owner_tag(fd), UNOWNED_TAG);
    int32_t ret = close(fd);
    int32_t savedErrno = errno;
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(savedErrno, EBADF);
}

/**
 * @tc.number: DumpFdGuard_0200
 * @tc.name: DumpFdGuard
 * @tc.desc: DumpFdGuard neither claims nor closes an invalid fd.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, DumpFdGuard_0200, TestSize.Level1)
{
    uint64_t stdinTag = fdsan_get_owner_tag(STDIN_FILENO);
    uint64_t stdoutTag = fdsan_get_owner_tag(STDOUT_FILENO);
    {
        DumpFdGuard fdGuard(-1);
        EXPECT_EQ(fdGuard.Get(), -1);
    }
    EXPECT_EQ(fdsan_get_owner_tag(STDIN_FILENO), stdinTag);
    EXPECT_EQ(fdsan_get_owner_tag(STDOUT_FILENO), stdoutTag);
    EXPECT_NE(fcntl(STDIN_FILENO, F_GETFD), -1);
}

/**
 * @tc.number: DumpFdGuard_0300
 * @tc.name: DumpFdGuard
 * @tc.desc: Move construction transfers ownership so the source guard never closes the fd.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, DumpFdGuard_0300, TestSize.Level1)
{
    int32_t fd = open(DEV_NULL_PATH, O_RDWR | O_CLOEXEC);
    ASSERT_GE(fd, 0);
    {
        DumpFdGuard srcGuard(fd);
        EXPECT_EQ(fdsan_get_owner_tag(fd), static_cast<uint64_t>(DUMP_FD_SAN_TAG));
        DumpFdGuard dstGuard(std::move(srcGuard));
        EXPECT_EQ(srcGuard.Get(), -1);
        EXPECT_EQ(dstGuard.Get(), fd);
        EXPECT_EQ(fdsan_get_owner_tag(fd), static_cast<uint64_t>(DUMP_FD_SAN_TAG));
    }
    EXPECT_EQ(fdsan_get_owner_tag(fd), UNOWNED_TAG);
    int32_t ret = close(fd);
    int32_t savedErrno = errno;
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(savedErrno, EBADF);
}

/**
 * @tc.number: DumpFdGuard_0400
 * @tc.name: DumpFdGuard
 * @tc.desc: Move assignment closes the previously held fd exactly once.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, DumpFdGuard_0400, TestSize.Level1)
{
    int32_t fd1 = open(DEV_NULL_PATH, O_RDWR | O_CLOEXEC);
    int32_t fd2 = open(DEV_NULL_PATH, O_RDWR | O_CLOEXEC);
    ASSERT_GE(fd1, 0);
    ASSERT_GE(fd2, 0);
    {
        DumpFdGuard guard1(fd1);
        DumpFdGuard guard2(fd2);
        guard1 = std::move(guard2);
        EXPECT_EQ(guard1.Get(), fd2);
        EXPECT_EQ(guard2.Get(), -1);
        EXPECT_EQ(fdsan_get_owner_tag(fd1), UNOWNED_TAG);
        EXPECT_EQ(fdsan_get_owner_tag(fd2), static_cast<uint64_t>(DUMP_FD_SAN_TAG));
    }
    EXPECT_EQ(fdsan_get_owner_tag(fd1), UNOWNED_TAG);
    EXPECT_EQ(fdsan_get_owner_tag(fd2), UNOWNED_TAG);
}

/**
 * @tc.number: DetectAndUnwindThreadStackLeak_0100
 * @tc.name: DetectAndUnwindThreadStackLeak
 * @tc.desc: DetectAndUnwindThreadStackLeak returns a string (possibly empty) without crashing on a normal process.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, DetectAndUnwindThreadStackLeak_0100, TestSize.Level1)
{
    std::string result = DumpRuntimeHelper::DetectAndUnwindThreadStackLeak();
    // On a normal test process there should be no leaked thread stack, so the
    // result should contain the header with "leak thread stack count: 0".
    GTEST_LOG_(INFO) << "DetectAndUnwindThreadStackLeak result:" << result;
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("total thread count:"), std::string::npos);
    EXPECT_NE(result.find("leak thread stack count: 0"), std::string::npos);
}

/**
 * @tc.number: ParseTidFromMapName_0100
 * @tc.name: ParseTidFromMapName
 * @tc.desc: Test ParseTidFromMapName extracts tid from valid map name.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, ParseTidFromMapName_0100, TestSize.Level1)
{
    int32_t tid1 = DumpRuntimeHelper::ParseTidFromMapName("[anon:stack:67890]");
    EXPECT_EQ(tid1, 67890);
    int32_t tid2 = DumpRuntimeHelper::ParseTidFromMapName("[anon:stack:1]");
    EXPECT_EQ(tid2, 1);
    int32_t tid3 = DumpRuntimeHelper::ParseTidFromMapName("[anon:stack:0]");
    EXPECT_EQ(tid3, 0);
    int32_t tid4 = DumpRuntimeHelper::ParseTidFromMapName("[anon:stack:99999]");
    EXPECT_EQ(tid4, 99999);
}

/**
 * @tc.number: ParseTidFromMapName_0200
 * @tc.name: ParseTidFromMapName
 * @tc.desc: Test ParseTidFromMapName returns 0 for invalid inputs.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, ParseTidFromMapName_0200, TestSize.Level1)
{
    EXPECT_EQ(DumpRuntimeHelper::ParseTidFromMapName(""), 0);
    EXPECT_EQ(DumpRuntimeHelper::ParseTidFromMapName("[anon:stack:]"), 0);
    EXPECT_EQ(DumpRuntimeHelper::ParseTidFromMapName("[anon:stack:xyz]"), 0);
    EXPECT_EQ(DumpRuntimeHelper::ParseTidFromMapName("invalid"), 0);
}

/**
 * @tc.number: IsTidAlive_0100
 * @tc.name: IsTidAlive
 * @tc.desc: Test IsTidAlive returns true for current thread and false for invalid tids.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, IsTidAlive_0100, TestSize.Level1)
{
    EXPECT_TRUE(DumpRuntimeHelper::IsTidAlive(gettid()));
    EXPECT_FALSE(DumpRuntimeHelper::IsTidAlive(0));
    EXPECT_FALSE(DumpRuntimeHelper::IsTidAlive(-1));
    EXPECT_FALSE(DumpRuntimeHelper::IsTidAlive(999999999));
}

/**
 * @tc.number: CountTotalThreads_0100
 * @tc.name: CountTotalThreads
 * @tc.desc: Test CountTotalThreads returns at least 1 for a running process.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, CountTotalThreads_0100, TestSize.Level1)
{
    size_t count = DumpRuntimeHelper::CountTotalThreads();
    EXPECT_GE(count, static_cast<size_t>(1));
    GTEST_LOG_(INFO) << "CountTotalThreads: " << count;
}

/**
 * @tc.number: ParseLeakedThreadStackRegions_0100
 * @tc.name: ParseLeakedThreadStackRegions
 * @tc.desc: Test ParseLeakedThreadStackRegions returns empty on normal process.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, ParseLeakedThreadStackRegions_0100, TestSize.Level1)
{
    auto regions = DumpRuntimeHelper::ParseLeakedThreadStackRegions();
    EXPECT_TRUE(regions.empty());
    GTEST_LOG_(INFO) << "ParseLeakedThreadStackRegions returned " << regions.size() << " regions";
    for (const auto &region : regions) {
        GTEST_LOG_(INFO) << "leaked region tid:" << region.tid
                         << " begin:" << region.begin << " end:" << region.end;
    }
}

/**
 * @tc.number: CollectPcsFromStack_0100
 * @tc.name: CollectPcsFromStack
 * @tc.desc: Test CollectPcsFromStack handles invalid ranges without crashing.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, CollectPcsFromStack_0100, TestSize.Level1)
{
    auto pcs = DumpRuntimeHelper::CollectPcsFromStack(0, 0, nullptr);
    EXPECT_TRUE(pcs.empty());
    GTEST_LOG_(INFO) << "CollectPcsFromStack with zero range returned " << pcs.size() << " pcs";
    pcs = DumpRuntimeHelper::CollectPcsFromStack(0x1000, 0x1000, nullptr);
    EXPECT_TRUE(pcs.empty());
    GTEST_LOG_(INFO) << "CollectPcsFromStack with equal begin/end returned " << pcs.size() << " pcs";
}

/**
 * @tc.number: FormatLeakedStack_0100
 * @tc.name: FormatLeakedStack
 * @tc.desc: Test FormatLeakedStack returns empty for empty and bogus PCs.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, FormatLeakedStack_0100, TestSize.Level1)
{
    std::vector<uintptr_t> emptyPcs;
    EXPECT_TRUE(DumpRuntimeHelper::FormatLeakedStack(12345, emptyPcs).empty());
    std::vector<uintptr_t> bogusPcs = {0xDEADBEEF};
    EXPECT_TRUE(DumpRuntimeHelper::FormatLeakedStack(99999, bogusPcs).empty());
}

/**
 * @tc.number: CollectPcsFromStack_0200
 * @tc.name: CollectPcsFromStack
 * @tc.desc: Test CollectPcsFromStack with non-zero range but nullptr maps returns empty.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, CollectPcsFromStack_0200, TestSize.Level1)
{
    int stackVar = 0;
    uint64_t begin = reinterpret_cast<uint64_t>(&stackVar);
    uint64_t end = begin + sizeof(uintptr_t);
    auto pcs = DumpRuntimeHelper::CollectPcsFromStack(begin, end, nullptr);
    EXPECT_TRUE(pcs.empty());
}

/**
 * @tc.number: FormatLeakedStack_0200
 * @tc.name: FormatLeakedStack
 * @tc.desc: Test FormatLeakedStack with multiple bogus PCs returns empty.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, FormatLeakedStack_0200, TestSize.Level1)
{
    std::vector<uintptr_t> bogusPcs = {0x1000, 0x2000, 0x3000};
    EXPECT_TRUE(DumpRuntimeHelper::FormatLeakedStack(99999, bogusPcs).empty());
    std::string result = DumpRuntimeHelper::FormatLeakedStack(99999, bogusPcs);
    EXPECT_TRUE(result.empty());
    GTEST_LOG_(INFO) << "FormatLeakedStack with bogus PCs result size:" << result.size();
    bogusPcs.push_back(0xDEADBEEF);
    bogusPcs.push_back(0xCAFEF00D);
    result = DumpRuntimeHelper::FormatLeakedStack(88888, bogusPcs);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.number: ParseTidFromMapName_0300
 * @tc.name: ParseTidFromMapName
 * @tc.desc: Test ParseTidFromMapName returns 0 for value exceeding INT32_MAX.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, ParseTidFromMapName_0300, TestSize.Level1)
{
    int32_t tid1 = DumpRuntimeHelper::ParseTidFromMapName("[anon:stack:2147483648]");
    EXPECT_EQ(tid1, 0);
    int32_t tid2 = DumpRuntimeHelper::ParseTidFromMapName("[anon:stack:9999999999]");
    EXPECT_EQ(tid2, 0);
    GTEST_LOG_(INFO) << "overflow tid1:" << tid1 << " tid2:" << tid2;
    int32_t tid3 = DumpRuntimeHelper::ParseTidFromMapName("[anon:stack:2147483647]");
    EXPECT_EQ(tid3, 2147483647);
}

/**
 * @tc.number: CollectPcsFromStack_0300
 * @tc.name: CollectPcsFromStack
 * @tc.desc: Test CollectPcsFromStack finds executable PCs from a real stack buffer.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, CollectPcsFromStack_0300, TestSize.Level1)
{
    auto maps = OHOS::HiviewDFX::DfxMaps::Create(0, true, false);
    ASSERT_NE(maps, nullptr);
    uintptr_t funcAddr = reinterpret_cast<uintptr_t>(&DumpRuntimeHelper::ParseTidFromMapName);
    uintptr_t buf[4] = {0, funcAddr, 0, 0};
    uint64_t begin = reinterpret_cast<uint64_t>(&buf[0]);
    uint64_t end = reinterpret_cast<uint64_t>(&buf[4]);
    auto pcs = DumpRuntimeHelper::CollectPcsFromStack(begin, end, maps);
    EXPECT_GE(pcs.size(), static_cast<size_t>(1));
    GTEST_LOG_(INFO) << "CollectPcsFromStack_0300 found " << pcs.size() << " PCs";
}

/**
 * @tc.number: CollectPcsFromStack_0400
 * @tc.name: CollectPcsFromStack
 * @tc.desc: Test CollectPcsFromStack with unmapped range triggers process_vm_readv failure.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, CollectPcsFromStack_0400, TestSize.Level1)
{
    auto maps = OHOS::HiviewDFX::DfxMaps::Create(0, true, false);
    ASSERT_NE(maps, nullptr);
    auto pcs = DumpRuntimeHelper::CollectPcsFromStack(0x10000000, 0x10001000, maps);
    EXPECT_TRUE(pcs.empty());
    GTEST_LOG_(INFO) << "CollectPcsFromStack_0400 returned " << pcs.size() << " PCs";
}

/**
 * @tc.number: FormatLeakedStack_0300
 * @tc.name: FormatLeakedStack
 * @tc.desc: Test FormatLeakedStack with a real function address does not crash.
 */
HWTEST_F(DumpRuntimeHelperTestSecond, FormatLeakedStack_0300, TestSize.Level1)
{
    uintptr_t pc = reinterpret_cast<uintptr_t>(&DumpRuntimeHelper::ParseTidFromMapName);
    std::vector<uintptr_t> pcs = {pc};
    std::string result = DumpRuntimeHelper::FormatLeakedStack(gettid(), pcs);
    GTEST_LOG_(INFO) << "FormatLeakedStack_0300 result size:" << result.size();
}

/**
 * @tc.number: ParseTidFromMapName_0400
 * @tc.name: ParseTidFromMapName
 * @tc.desc: Test ParseTidFromMapName returns 0 for negative sign (non-digit stops parsing).
 */
HWTEST_F(DumpRuntimeHelperTestSecond, ParseTidFromMapName_0400, TestSize.Level1)
{
    EXPECT_EQ(DumpRuntimeHelper::ParseTidFromMapName("[anon:stack:-2147483649]"), 0);
    EXPECT_EQ(DumpRuntimeHelper::ParseTidFromMapName("[anon:stack:-1]"), 0);
    GTEST_LOG_(INFO) << "negative sign correctly returns 0";
}
} // namespace AppExecFwk
} // namespace OHOS
