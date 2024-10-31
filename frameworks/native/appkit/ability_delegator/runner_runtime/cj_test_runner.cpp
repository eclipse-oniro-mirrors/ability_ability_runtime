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

#include "runner_runtime/cj_test_runner.h"

#include <regex>

#include "ability_delegator_registry.h"
#include "hilog_tag_wrapper.h"
#include "runner_runtime/cj_test_runner_object.h"

namespace OHOS {
namespace RunnerRuntime {

std::unique_ptr<TestRunner> CJTestRunner::Create(const std::unique_ptr<Runtime> &runtime,
    const std::shared_ptr<AbilityDelegatorArgs> &args, const AppExecFwk::BundleInfo &bundleInfo)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");
    if (!runtime) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid runtime");
        return nullptr;
    }

    auto cjRuntime = static_cast<CJRuntime*>(runtime.get());
    if (!cjRuntime->IsAppLibLoaded()) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "appLib not loaded");
        return nullptr;
    }

    if (!args) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid args");
        return nullptr;
    }

    auto pTestRunner = new (std::nothrow) CJTestRunner(*cjRuntime, args, bundleInfo);
    if (!pTestRunner) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "create testrunner failed");
        return nullptr;
    }

    return std::unique_ptr<CJTestRunner>(pTestRunner);
}

CJTestRunner::CJTestRunner(CJRuntime &cjRuntime, const std::shared_ptr<AbilityDelegatorArgs> &args,
    const AppExecFwk::BundleInfo &bundleInfo) : cjRuntime_(cjRuntime)
{
    std::string moduleName = args->GetTestRunnerClassName();
    cjTestRunnerObj_ = CJTestRunnerObject::LoadModule(moduleName);
}

CJTestRunner::~CJTestRunner() = default;

bool CJTestRunner::Initialize()
{
    if (!cjRuntime_.IsAppLibLoaded()) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "appLib not loaded");
        return false;
    }
    if (!cjTestRunnerObj_) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cjTestRunnerObj_");
        return false;
    }
    return true;
}

void CJTestRunner::Prepare()
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");
    TestRunner::Prepare();
    if (!cjTestRunnerObj_) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cjTestRunnerObj_");
        return;
    }
    cjTestRunnerObj_->OnPrepare();
}

void CJTestRunner::Run()
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");
    TestRunner::Run();
    if (!cjTestRunnerObj_) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null cjTestRunnerObj_");
        return;
    }
    cjTestRunnerObj_->OnRun();
}

void CJTestRunner::ReportFinished(const std::string &msg)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        return;
    }

    delegator->FinishUserTest(msg, -1);
}

void CJTestRunner::ReportStatus(const std::string &msg)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "Enter");
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        return;
    }

    delegator->Print(msg);
}
}  // namespace RunnerRuntime
}  // namespace OHOS
