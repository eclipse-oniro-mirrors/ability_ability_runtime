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

#include <gtest/gtest.h>

#define private public
#include "js_test_runner.h"
#include "ability_delegator.h"
#include "ability_manager_client.h"
#include "ability_runtime/context/context_impl.h"
#undef private

#include "constants.h"
#include "ability_delegator_registry.h"
#include "app_loader.h"
#include "event_runner.h"
#include "hilog_tag_wrapper.h"
#include "napi/native_common.h"
#include "mock_ability_delegator_stub.h"
#include "mock_test_observer_stub.h"
#include "ohos_application.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;
using namespace OHOS::AbilityBase::Constants;
using namespace OHOS::RunnerRuntime;

namespace {
const std::string KEY_TEST_BUNDLE_NAME = "-p";
const std::string VALUE_TEST_BUNDLE_NAME = "com.example.myapplicationjs";
const std::string KEY_TEST_RUNNER_CLASS = "-s unittest";
const std::string VALUE_TEST_RUNNER_CLASS = "JSUserTestRunnerjs";
const std::string KEY_TEST_CASE = "-s class";
const std::string VALUE_TEST_CASE =
"ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010_js";
const std::string KEY_TEST_WAIT_TIMEOUT = "-w";
const std::string VALUE_TEST_WAIT_TIMEOUT = "35";
const std::string REPORT_FINISH_MSG = "report finish message";
}

class JsTestRunnerTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void MakeMockObjects() const;
};

void JsTestRunnerTest::SetUpTestCase()
{}

void JsTestRunnerTest::TearDownTestCase()
{}

void JsTestRunnerTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void JsTestRunnerTest::TearDown()
{}

void JsTestRunnerTest::MakeMockObjects() const
{
    // mock a stub
    auto managerStubPtr = sptr<OHOS::AAFwk::IAbilityManager>(new MockAbilityDelegatorStub);

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

/**
 * @tc.number: Js_Test_Runner_Test_0100
 * @tc.name: ReportFinished
 * @tc.desc: Verify the ReportFinished is invalid.
 */
HWTEST_F(JsTestRunnerTest, Js_Test_Runner_Test_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Js_Test_Runner_Test_0100 is called");

    MockAbilityDelegatorStub::finishFlag_ = false;
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<AbilityDelegatorArgs> abilityArgs = std::make_shared<AbilityDelegatorArgs>(want);
    AbilityDelegatorRegistry::RegisterInstance(nullptr, abilityArgs,
        OHOS::AbilityRuntime::Runtime::Language::JS);

    JsTestRunner* jsRunnerdrive = nullptr;
    jsRunnerdrive->ReportFinished(REPORT_FINISH_MSG);
    EXPECT_NE(abilityArgs, nullptr);
}

/**
 * @tc.number: Js_Test_Runner_Test_0200
 * @tc.name: ReportFinished
 * @tc.desc: Verify the ReportFinished is valid.
 */
HWTEST_F(JsTestRunnerTest, Js_Test_Runner_Test_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Js_Test_Runner_Test_0200 is called");

    MockAbilityDelegatorStub::finishFlag_ = false;
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<AbilityDelegatorArgs> abilityArgs = std::make_shared<AbilityDelegatorArgs>(want);

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        abilityArgs,
        true);
    EXPECT_TRUE(testRunner->Initialize());
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<AbilityDelegator> abilityDelegator =
        std::make_shared<AbilityDelegator>(context, std::move(testRunner), iRemoteObj);
    AbilityDelegatorRegistry::RegisterInstance(abilityDelegator, abilityArgs,
        OHOS::AbilityRuntime::Runtime::Language::JS);

    JsTestRunner* jsRunnerdrive = nullptr;
    jsRunnerdrive->ReportFinished(REPORT_FINISH_MSG);
    EXPECT_EQ(testRunner, nullptr);
}

/**
 * @tc.number: Js_Test_Runner_Test_0300
 * @tc.name: ReportStatus
 * @tc.desc: Verify the ReportStatus is invalid.
 */
HWTEST_F(JsTestRunnerTest, Js_Test_Runner_Test_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Js_Test_Runner_Test_0300 is called");

    MockAbilityDelegatorStub::finishFlag_ = false;
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<AbilityDelegatorArgs> abilityArgs = std::make_shared<AbilityDelegatorArgs>(want);
    AbilityDelegatorRegistry::RegisterInstance(nullptr, abilityArgs,
        OHOS::AbilityRuntime::Runtime::Language::JS);

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        std::make_shared<AbilityDelegatorArgs>(want),
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub());
    AbilityDelegator abilityDelegator(context, std::move(testRunner), iRemoteObj);
    sptr<IRemoteObject> shobserver = sptr<IRemoteObject>(new MockTestObserverStub);
    abilityDelegator.observer_ = shobserver;

    JsTestRunner* jsRunnerdrive = nullptr;
    jsRunnerdrive->ReportStatus(REPORT_FINISH_MSG);

    EXPECT_FALSE(iface_cast<MockTestObserverStub>(shobserver)->testStatusFlag);
}

/**
 * @tc.number: Js_Test_Runner_Test_0400
 * @tc.name: ReportStatus
 * @tc.desc: Verify the ReportStatus is valid.
 */
HWTEST_F(JsTestRunnerTest, Js_Test_Runner_Test_0400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Js_Test_Runner_Test_0400 is called");

    MockAbilityDelegatorStub::finishFlag_ = false;
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<AbilityDelegatorArgs> abilityArgs = std::make_shared<AbilityDelegatorArgs>(want);

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        abilityArgs,
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<AbilityDelegator> abilityDelegator =
        std::make_shared<AbilityDelegator>(context, std::move(testRunner), iRemoteObj);
    AbilityDelegatorRegistry::RegisterInstance(abilityDelegator, abilityArgs,
        OHOS::AbilityRuntime::Runtime::Language::JS);

    sptr<IRemoteObject> shobserver = sptr<IRemoteObject>(new MockTestObserverStub);
    abilityDelegator->observer_ = shobserver;

    JsTestRunner* jsRunnerdrive = nullptr;
    jsRunnerdrive->ReportStatus(REPORT_FINISH_MSG);

    EXPECT_TRUE(iface_cast<MockTestObserverStub>(shobserver)->testStatusFlag);
}

/**
 * @tc.number: Js_Test_Runner_Test_0500
 * @tc.name: ReportFinished
 * @tc.desc: Verify the ReportFinished is valid.
 */
HWTEST_F(JsTestRunnerTest, Js_Test_Runner_Test_0500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Js_Test_Runner_Test_0500 is called");

    MockAbilityDelegatorStub::finishFlag_ = false;
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<AbilityDelegatorArgs> abilityArgs = std::make_shared<AbilityDelegatorArgs>(want);

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        abilityArgs,
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<AbilityDelegator> abilityDelegator =
        std::make_shared<AbilityDelegator>(context, std::move(testRunner), iRemoteObj);
    AbilityDelegatorRegistry::RegisterInstance(abilityDelegator, abilityArgs,
        OHOS::AbilityRuntime::Runtime::Language::JS);

    JsTestRunner* pTestRunner = static_cast<JsTestRunner*>(static_cast<void*>((testRunner.get())));
    pTestRunner->ReportFinished(REPORT_FINISH_MSG);
    EXPECT_EQ(testRunner, nullptr);
}

/**
 * @tc.number: Js_Test_Runner_Test_0600
 * @tc.name: ReportStatus
 * @tc.desc: Verify the ReportStatus is valid.
 */
HWTEST_F(JsTestRunnerTest, Js_Test_Runner_Test_0600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Js_Test_Runner_Test_0600 is called");

    MockAbilityDelegatorStub::finishFlag_ = false;
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<AbilityDelegatorArgs> abilityArgs = std::make_shared<AbilityDelegatorArgs>(want);

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        abilityArgs,
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<AbilityDelegator> abilityDelegator =
        std::make_shared<AbilityDelegator>(context, std::move(testRunner), iRemoteObj);
    AbilityDelegatorRegistry::RegisterInstance(abilityDelegator, abilityArgs,
        OHOS::AbilityRuntime::Runtime::Language::JS);

    sptr<IRemoteObject> shobserver = sptr<IRemoteObject>(new MockTestObserverStub);
    abilityDelegator->observer_ = shobserver;

    JsTestRunner* pTestRunner = static_cast<JsTestRunner*>(static_cast<void*>((testRunner.get())));
    pTestRunner->ReportStatus(REPORT_FINISH_MSG);

    EXPECT_TRUE(iface_cast<MockTestObserverStub>(shobserver)->testStatusFlag);
}

/**
 * @tc.number: Js_GetTestRunnerPath_0100
 * @tc.name: ReportStatus
 * @tc.desc: gettestrunner path
 */
HWTEST_F(JsTestRunnerTest, Js_GetTestRunnerPath_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Js_GetTestRunnerPath_0100 is called");

    MockAbilityDelegatorStub::finishFlag_ = false;
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<AbilityDelegatorArgs> abilityArgs = std::make_shared<AbilityDelegatorArgs>(want);

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        abilityArgs,
        true);
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<AbilityDelegator> abilityDelegator =
        std::make_shared<AbilityDelegator>(context, std::move(testRunner), iRemoteObj);
    AbilityDelegatorRegistry::RegisterInstance(abilityDelegator, abilityArgs,
        OHOS::AbilityRuntime::Runtime::Language::JS);

    sptr<IRemoteObject> shobserver = sptr<IRemoteObject>(new MockTestObserverStub);
    abilityDelegator->observer_ = shobserver;

    JsTestRunner* pTestRunner = static_cast<JsTestRunner*>(static_cast<void*>((testRunner.get())));
    std::string path = pTestRunner->GetTestRunnerPath(abilityArgs);
    TAG_LOGI(AAFwkTag::TEST, "Js_GetTestRunnerPath_0100 GetTestRunnerPath is called: %{public}s ", path.c_str());
    EXPECT_EQ(path, "/ets/testrunner/JSUserTestRunnerjs");
}

/**
 * @tc.number: Js_GetTestRunnerPath_0200
 * @tc.name: ReportStatus
 * @tc.desc: gettestrunner path
 */
HWTEST_F(JsTestRunnerTest, Js_GetTestRunnerPath_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Js_GetTestRunnerPath_0200 is called");

    MockAbilityDelegatorStub::finishFlag_ = false;
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<AbilityDelegatorArgs> abilityArgs = std::make_shared<AbilityDelegatorArgs>(want);
    abilityArgs->SetTestRunnerModeAndPath("static", "mypatch.ets");

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        abilityArgs,
        true);
    JsTestRunner* pTestRunner = static_cast<JsTestRunner*>(static_cast<void*>((testRunner.get())));
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<AbilityDelegator> abilityDelegator =
        std::make_shared<AbilityDelegator>(context, std::move(testRunner), iRemoteObj);
    AbilityDelegatorRegistry::RegisterInstance(abilityDelegator, abilityArgs,
        OHOS::AbilityRuntime::Runtime::Language::JS);

    sptr<IRemoteObject> shobserver = sptr<IRemoteObject>(new MockTestObserverStub);
    abilityDelegator->observer_ = shobserver;

    std::string path = pTestRunner->GetTestRunnerPath(abilityArgs);
    TAG_LOGI(AAFwkTag::TEST, "Js_GetTestRunnerPath_0200 GetTestRunnerPath is called: %{public}s ", path.c_str());
    EXPECT_EQ(path, "/mypatch");
}

/**
 * @tc.number: Js_GetTestRunnerPath_0300
 * @tc.name: GetTestRunnerPath
 * @tc.desc: Verify GetTestRunnerPath when path has no dot, should not crash.
 */
HWTEST_F(JsTestRunnerTest, Js_GetTestRunnerPath_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Js_GetTestRunnerPath_0300 is called");

    MockAbilityDelegatorStub::finishFlag_ = false;
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<AbilityDelegatorArgs> abilityArgs = std::make_shared<AbilityDelegatorArgs>(want);
    abilityArgs->SetTestRunnerModeAndPath("static", "mypath");

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        abilityArgs,
        true);
    JsTestRunner* pTestRunner = static_cast<JsTestRunner*>(static_cast<void*>((testRunner.get())));
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<AbilityDelegator> abilityDelegator =
        std::make_shared<AbilityDelegator>(context, std::move(testRunner), iRemoteObj);
    AbilityDelegatorRegistry::RegisterInstance(abilityDelegator, abilityArgs,
        OHOS::AbilityRuntime::Runtime::Language::JS);

    sptr<IRemoteObject> shobserver = sptr<IRemoteObject>(new MockTestObserverStub);
    abilityDelegator->observer_ = shobserver;

    std::string path = pTestRunner->GetTestRunnerPath(abilityArgs);
    TAG_LOGI(AAFwkTag::TEST, "Js_GetTestRunnerPath_0300 GetTestRunnerPath is called: %{public}s ", path.c_str());
    EXPECT_EQ(path, "/mypath");
}

/**
 * @tc.number: Js_GetTestRunnerPath_0400
 * @tc.name: GetTestRunnerPath
 * @tc.desc: Verify GetTestRunnerPath when path has multiple dots, erase from last dot.
 */
HWTEST_F(JsTestRunnerTest, Js_GetTestRunnerPath_0400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Js_GetTestRunnerPath_0400 is called");

    MockAbilityDelegatorStub::finishFlag_ = false;
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<AbilityDelegatorArgs> abilityArgs = std::make_shared<AbilityDelegatorArgs>(want);
    abilityArgs->SetTestRunnerModeAndPath("static", "my.patch.ets");

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        abilityArgs,
        true);
    JsTestRunner* pTestRunner = static_cast<JsTestRunner*>(static_cast<void*>((testRunner.get())));
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<AbilityDelegator> abilityDelegator =
        std::make_shared<AbilityDelegator>(context, std::move(testRunner), iRemoteObj);
    AbilityDelegatorRegistry::RegisterInstance(abilityDelegator, abilityArgs,
        OHOS::AbilityRuntime::Runtime::Language::JS);

    sptr<IRemoteObject> shobserver = sptr<IRemoteObject>(new MockTestObserverStub);
    abilityDelegator->observer_ = shobserver;

    std::string path = pTestRunner->GetTestRunnerPath(abilityArgs);
    TAG_LOGI(AAFwkTag::TEST, "Js_GetTestRunnerPath_0400 GetTestRunnerPath is called: %{public}s ", path.c_str());
    EXPECT_EQ(path, "/my.patch");
}

/**
 * @tc.number: Js_GetTestRunnerPath_0500
 * @tc.name: GetTestRunnerPath
 * @tc.desc: Verify GetTestRunnerPath when path is empty, should not crash.
 */
HWTEST_F(JsTestRunnerTest, Js_GetTestRunnerPath_0500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Js_GetTestRunnerPath_0500 is called");

    MockAbilityDelegatorStub::finishFlag_ = false;
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::shared_ptr<AbilityDelegatorArgs> abilityArgs = std::make_shared<AbilityDelegatorArgs>(want);
    abilityArgs->SetTestRunnerModeAndPath("static", "");

    std::shared_ptr<OHOS::AbilityRuntime::Context> context = std::make_shared<OHOS::AbilityRuntime::ContextImpl>();
    std::unique_ptr<TestRunner> testRunner = TestRunner::Create(
        std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName())->GetRuntime(),
        abilityArgs,
        true);
    JsTestRunner* pTestRunner = static_cast<JsTestRunner*>(static_cast<void*>((testRunner.get())));
    sptr<IRemoteObject> iRemoteObj = sptr<IRemoteObject>(new MockAbilityDelegatorStub);
    std::shared_ptr<AbilityDelegator> abilityDelegator =
        std::make_shared<AbilityDelegator>(context, std::move(testRunner), iRemoteObj);
    AbilityDelegatorRegistry::RegisterInstance(abilityDelegator, abilityArgs,
        OHOS::AbilityRuntime::Runtime::Language::JS);

    sptr<IRemoteObject> shobserver = sptr<IRemoteObject>(new MockTestObserverStub);
    abilityDelegator->observer_ = shobserver;

    std::string path = pTestRunner->GetTestRunnerPath(abilityArgs);
    TAG_LOGI(AAFwkTag::TEST, "Js_GetTestRunnerPath_0500 GetTestRunnerPath is called: %{public}s ", path.c_str());
    EXPECT_EQ(path, "/");
}