/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include <regex>

#include "ability_delegator_registry.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime_utils.h"
#include "runner_runtime/js_test_runner.h"

namespace OHOS {
namespace RunnerRuntime {

std::unique_ptr<TestRunner> JsTestRunner::Create(const std::unique_ptr<Runtime> &runtime,
    const std::shared_ptr<AbilityDelegatorArgs> &args, const AppExecFwk::BundleInfo &bundleInfo, bool isFaJsModel)
{
    if (!runtime) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid runtime");
        return nullptr;
    }

    if (!args) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid args");
        return nullptr;
    }

    auto pTestRunner = new (std::nothrow) JsTestRunner(static_cast<JsRuntime &>(*runtime), args, bundleInfo,
        isFaJsModel);
    if (!pTestRunner) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null testRunner");
        return nullptr;
    }

    return std::unique_ptr<JsTestRunner>(pTestRunner);
}

JsTestRunner::JsTestRunner(
    JsRuntime &jsRuntime, const std::shared_ptr<AbilityDelegatorArgs> &args, const AppExecFwk::BundleInfo &bundleInfo,
    bool isFaJsModel)
    : jsRuntime_(jsRuntime), isFaJsModel_(isFaJsModel)
{
    std::string moduleName;
    if (args) {
        std::string srcPath;
        if (bundleInfo.hapModuleInfos.back().isModuleJson) {
            srcPath.append(GetTestRunnerPath(args));
            moduleName = args->GetTestModuleName();
        } else {
            srcPath.append(args->GetTestPackageName());
            srcPath.append("/assets/js/TestRunner/");
            moduleName = args->GetTestPackageName();
            srcPath.append(args->GetTestRunnerClassName());
        }
        srcPath.append(".abc");
        srcPath_ = srcPath;
    }
    TAG_LOGI(AAFwkTag::DELEGATOR, "srcPath: %{public}s", srcPath_.c_str());

    if (!moduleName.empty()) {
        for (auto hapModuleInfo : bundleInfo.hapModuleInfos) {
            if ((hapModuleInfo.isModuleJson && hapModuleInfo.name == moduleName) ||
                hapModuleInfo.package == moduleName) {
                hapPath_ = hapModuleInfo.hapPath;
                break;
            }
        }
    } else {
        hapPath_ = bundleInfo.hapModuleInfos.back().hapPath;
    }
    TAG_LOGD(AAFwkTag::DELEGATOR, "hapPath: %{public}s", hapPath_.c_str());

    if (isFaJsModel) {
        return;
    }

    moduleName.append("::").append("TestRunner");
    jsTestRunnerObj_ = jsRuntime_.LoadModule(moduleName, srcPath_, hapPath_,
        bundleInfo.hapModuleInfos.back().compileMode == AppExecFwk::CompileMode::ES_MODULE);
    if (!jsTestRunnerObj_ &&
        srcPath_.find(AbilityRuntime::LOWERCASETESTRUNNER) != std::string::npos &&
        !args->GetTestRunnerClassName().empty()) {
        TAG_LOGI(AAFwkTag::DELEGATOR, "not found %{public}s , retry load capital address", srcPath_.c_str());
        std::regex src_pattern(AbilityRuntime::LOWERCASETESTRUNNER);
        srcPath_ = std::regex_replace(srcPath_, src_pattern, AbilityRuntime::CAPITALTESTRUNNER);
        TAG_LOGD(AAFwkTag::DELEGATOR, "capital address is %{public}s", srcPath_.c_str());
        jsTestRunnerObj_ = jsRuntime_.LoadModule(moduleName, srcPath_, hapPath_,
            bundleInfo.hapModuleInfos.back().compileMode == AppExecFwk::CompileMode::ES_MODULE);
    }
}

JsTestRunner::~JsTestRunner() = default;

bool JsTestRunner::Initialize()
{
    HandleScope handleScope(jsRuntime_);
    if (isFaJsModel_) {
        if (!jsRuntime_.RunScript("/system/etc/strip.native.min.abc", "")) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "runScript err");
            return false;
        }

        if (!jsRuntime_.RunScript("/system/etc/abc/ability/delegator_mgmt.abc", "")) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "run delegator failed");
            return false;
        }

        if (!jsRuntime_.RunSandboxScript(srcPath_, hapPath_)) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "runScript srcPath_ err");
            return false;
        }

        napi_env env = jsRuntime_.GetNapiEnv();
        napi_value object = nullptr;
        napi_get_global(env, &object);
        if (object == nullptr) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "null object");
            return false;
        }
        napi_value mainEntryFunc = nullptr;
        napi_get_named_property(env, object, "___mainEntry___", &mainEntryFunc);
        if (mainEntryFunc == nullptr) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "null mainEntryFunc");
            return false;
        }
        napi_value value = nullptr;
        napi_get_global(env, &value);
        if (value == nullptr) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "null value");
            return false;
        }
        napi_call_function(env, value, mainEntryFunc, 1, &value, nullptr);
    }
    return true;
}

void JsTestRunner::Prepare()
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");
    TestRunner::Prepare();
    CallObjectMethod("onPrepare");
}

void JsTestRunner::Run()
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");
    std::unique_lock<std::mutex> lck(mutexHasStop_);
    hasStop_ = false;
    TestRunner::Run();
    CallObjectMethod("onRun");
}

void JsTestRunner::Stop()
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");
    std::unique_lock<std::mutex> lck(mutexHasStop_);
    if (!hasStop_) {
        hasStop_ = true;
        TestRunner::Stop();
        CallObjectMethod("onStop");
    }
}

void JsTestRunner::CallObjectMethod(const char *name, napi_value const *argv, size_t argc)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "callJsMethod(%{public}s)", name);
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (isFaJsModel_) {
        napi_value global = nullptr;
        napi_get_global(env, &global);
        napi_value exportObject = nullptr;
        napi_get_named_property(env, global, "exports", &exportObject);
        if (!CheckTypeForNapiValue(env, exportObject, napi_object)) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "get exportObject failed");
            return;
        }

        napi_value defaultObject = nullptr;
        napi_get_named_property(env, exportObject, "default", &defaultObject);
        if (!CheckTypeForNapiValue(env, defaultObject, napi_object)) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "get defaultObject failed");
            return;
        }

        napi_value func = nullptr;
        napi_get_named_property(env, defaultObject, name, &func);
        if (!CheckTypeForNapiValue(env, func, napi_function)) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "callRequest func:%{public}s", func == nullptr ? "nullptr" : "not func");
            return;
        }
        napi_call_function(env, CreateJsUndefined(env), func, argc, argv, nullptr);
        return;
    }

    if (!jsTestRunnerObj_) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "not found %{public}s", srcPath_.c_str());
        ReportFinished("Not found " + srcPath_);
        return;
    }

    napi_value obj = jsTestRunnerObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "get TestRunner object failed");
        ReportFinished("Failed to get Test Runner object");
        return;
    }

    napi_value methodOnCreate = nullptr;
    napi_get_named_property(env, obj, name, &methodOnCreate);
    if (methodOnCreate == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "get '%{public}s' from TestRunner object failed", name);
        ReportStatus("Failed to get " + std::string(name) + " from Test Runner object");
        return;
    }
    napi_call_function(env, obj, methodOnCreate, argc, argv, nullptr);
}

void JsTestRunner::ReportFinished(const std::string &msg)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "enter");
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        return;
    }

    delegator->FinishUserTest(msg, -1);
}

void JsTestRunner::ReportStatus(const std::string &msg)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "enter");
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        return;
    }

    delegator->Print(msg);
}

std::string JsTestRunner::GetTestRunnerPath(const std::shared_ptr<AbilityDelegatorArgs> &args)
{
    std::string result;
    result.append(args->GetTestModuleName());
    if (!args->GetTestRunnerClassName().empty()) {
        if (args->GetTestRunnerClassName().find("/") == std::string::npos) {
            result.append(LOWERCASETESTRUNNER);
        }
        result.append(args->GetTestRunnerClassName());
    } else {
        result.append("/");
        result.append(args->GetTestRunnerPath());
        size_t pos = result.rfind(".");
        if (pos != std::string::npos) {
            result.erase(pos);
        }
    }
    return result;
}
}  // namespace RunnerRuntime
}  // namespace OHOS
