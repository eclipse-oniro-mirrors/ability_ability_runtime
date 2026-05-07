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

#include "js_ability_delegator_utils.h"

#include <map>
#include "hilog_tag_wrapper.h"
#include "js_ability_monitor.h"
#include "napi/native_common.h"

namespace OHOS {
namespace AbilityDelegatorJs {
using namespace OHOS::AbilityRuntime;
napi_value CreateJsAbilityDelegator(napi_env env)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null objValue");
        return nullptr;
    }

    std::unique_ptr<JSAbilityDelegator> jsAbilityDelegator = std::make_unique<JSAbilityDelegator>();
    napi_wrap(env, objValue, jsAbilityDelegator.release(), JSAbilityDelegator::Finalizer, nullptr, nullptr);

    const char *moduleName = "JSAbilityDelegator";
    BindNativeFunction(env, objValue, "addAbilityMonitor", moduleName, JSAbilityDelegator::AddAbilityMonitor);
    BindNativeFunction(env, objValue, "addAbilityStageMonitor",
        moduleName, JSAbilityDelegator::AddAbilityStageMonitor);
    BindNativeFunction(env, objValue, "removeAbilityMonitor", moduleName, JSAbilityDelegator::RemoveAbilityMonitor);
    BindNativeFunction(env, objValue, "removeAbilityStageMonitor",
        moduleName, JSAbilityDelegator::RemoveAbilityStageMonitor);
    BindNativeFunction(env, objValue, "waitAbilityMonitor", moduleName, JSAbilityDelegator::WaitAbilityMonitor);
    BindNativeFunction(env, objValue, "waitAbilityStageMonitor",
        moduleName, JSAbilityDelegator::WaitAbilityStageMonitor);
    BindNativeFunction(env, objValue, "getAppContext", moduleName, JSAbilityDelegator::GetAppContext);
    BindNativeFunction(env, objValue, "getAbilityState", moduleName, JSAbilityDelegator::GetAbilityState);
    BindNativeFunction(env, objValue, "getCurrentTopAbility", moduleName, JSAbilityDelegator::GetCurrentTopAbility);
    BindNativeFunction(env, objValue, "startAbility", moduleName, JSAbilityDelegator::StartAbility);
    BindNativeFunction(env, objValue, "doAbilityForeground", moduleName, JSAbilityDelegator::DoAbilityForeground);
    BindNativeFunction(env, objValue, "doAbilityBackground", moduleName, JSAbilityDelegator::DoAbilityBackground);
    BindNativeFunction(env, objValue, "print", moduleName, JSAbilityDelegator::Print);
    BindNativeFunction(env, objValue, "printSync", moduleName, JSAbilityDelegator::PrintSync);
    BindNativeFunction(env, objValue, "executeShellCommand", moduleName, JSAbilityDelegator::ExecuteShellCommand);
    BindNativeFunction(env, objValue, "finishTest", moduleName, JSAbilityDelegator::FinishTest);
    BindNativeFunction(env, objValue, "addAbilityMonitorSync", moduleName, JSAbilityDelegator::AddAbilityMonitorSync);
    BindNativeFunction(env, objValue, "addAbilityStageMonitorSync",
                       moduleName, JSAbilityDelegator::AddAbilityStageMonitorSync);
    BindNativeFunction(env, objValue, "removeAbilityMonitorSync", moduleName,
                       JSAbilityDelegator::RemoveAbilityMonitorSync);
    BindNativeFunction(env, objValue, "removeAbilityStageMonitorSync",
                       moduleName, JSAbilityDelegator::RemoveAbilityStageMonitorSync);
    BindNativeFunction(env, objValue, "setMockList", moduleName, JSAbilityDelegator::SetMockList);
    BindNativeFunction(env, objValue, "addInteropAbilityMonitorSync", moduleName,
        JSAbilityDelegator::AddInteropAbilityMonitorSync);
    BindNativeFunction(env, objValue, "removeInteropAbilityMonitorSync", moduleName,
        JSAbilityDelegator::RemoveInteropAbilityMonitorSync);
    return handleEscape.Escape(objValue);
}

napi_value SetAbilityDelegatorArgumentsPara(napi_env env, const std::map<std::string, std::string> &paras)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null objValue");
        return nullptr;
    }

    auto iter = paras.begin();
    for (; iter != paras.end(); ++iter) {
        napi_set_named_property(env, objValue, iter->first.c_str(),
            CreateJsValue(env, iter->second));
    }
    return handleEscape.Escape(objValue);
}

napi_value CreateJsAbilityDelegatorArguments(
    napi_env env, const std::shared_ptr<AbilityDelegatorArgs> &abilityDelegatorArgs)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null objValue");
        return CreateJsNull(env);
    }

    napi_set_named_property(env, objValue, "bundleName",
        CreateJsValue(env, abilityDelegatorArgs->GetTestBundleName()));
    napi_set_named_property(env, objValue, "parameters",
        SetAbilityDelegatorArgumentsPara(env, abilityDelegatorArgs->GetTestParam()));
    napi_set_named_property(env, objValue, "testCaseNames",
        CreateJsValue(env, abilityDelegatorArgs->GetTestCaseName()));
    napi_set_named_property(env, objValue, "testRunnerClassName",
        CreateJsValue(env, abilityDelegatorArgs->GetTestRunnerClassName()));

    return handleEscape.Escape(objValue);
}

napi_value CreateJsShellCmdResult(napi_env env, std::unique_ptr<ShellCmdResult> &shellResult)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called");

    HandleEscape handleEscape(env);
    if (!shellResult) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null shellResult");
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null objValue");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "stdResult", CreateJsValue(env, shellResult->GetStdResult()));
    napi_set_named_property(env, objValue, "exitCode", CreateJsValue(env, shellResult->GetExitCode()));

    return handleEscape.Escape(objValue);
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
