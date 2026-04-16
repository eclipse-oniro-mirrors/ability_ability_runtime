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

#define private public
#include "dump_runtime_helper.h"
#undef private

#include "app_loader.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "ohos_application.h"

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
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
}
}