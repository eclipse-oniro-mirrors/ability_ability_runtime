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

#include "ets_ability_delegator_utils.h"

#include <map>
#include "ets_ability_delegator.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityDelegatorEts {
namespace {
constexpr const char* ABILITY_DELEGATOR_CLASS_NAME = "application.AbilityDelegator.AbilityDelegatorInner";
constexpr const char* RECORD_CLASS_NAME = "std.core.Record";
constexpr const char* VOID_CONTEXT = ":C{application.Context.Context}";
constexpr const char* STRING_NUMBER_ASYNCCALLBACK = "C{std.core.String}lC{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char* STRING_VOID = "C{std.core.String}:";
constexpr const char* MONITOR_ASYNCCALLBACK_VOID =
    "C{application.AbilityMonitor.AbilityMonitor}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char* WANT_ASYNCCALLBACK = "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char* ARGS_ABILITY_DELEGATOR_CLASS_NAME =
    "application.abilityDelegatorArgs.AbilityDelegatorArgsInner";
constexpr const char* SIGNATURE_MONITOR_VOID = "C{application.AbilityMonitor.AbilityMonitor}:";
constexpr const char* INTEROP_MONITOR_VOID =
    "C{application.InteropAbilityMonitor.InteropAbilityMonitor}:";
constexpr const char* SIGNATURE_MONITOR_NUMBER_ASYNCCALLBACK =
    "C{application.AbilityMonitor.AbilityMonitor}lC{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char* STAGEMONITOR_ASYNCCALLBACK_VOID =
    "C{application.AbilityStageMonitor.AbilityStageMonitor}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char* STAGEMONITOR_VOID =
    "C{application.AbilityStageMonitor.AbilityStageMonitor}:";
constexpr const char* SIGNATURE_STAGEMONITOR_NUMBER_ASYNCCALLBACK =
    "C{application.AbilityStageMonitor.AbilityStageMonitor}lC{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char* STAGEMONITOR_UIABILITY_ASYNCCALLBACK =
    "C{@ohos.app.ability.UIAbility.UIAbility}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char* STAGEMONITOR_UIABILITY_CHECK =
    "C{@ohos.app.ability.UIAbility.UIAbility}:";
constexpr const char* STRING_ASYNCCALLBACK_VOID = "C{std.core.String}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char* STRING_UIABILITY_NUMBER = "C{@ohos.app.ability.UIAbility.UIAbility}:i";
constexpr const char* STRING_CALLBACK_UIABILITY =
    "C{utils.AbilityUtils.AsyncCallbackWrapper}:C{@ohos.app.ability.UIAbility.UIAbility}";
constexpr const char* RECORD_GET_NAME =
    "X{C{std.core.BaseEnum}C{std.core.Numeric}C{std.core.String}}:Y";
constexpr const char* RECORD_SET_NAME =
    "X{C{std.core.BaseEnum}C{std.core.Numeric}C{std.core.String}}Y:";
}

auto GetBaseNativeFunctions()
{
    return std::array {
        ani_native_function {"getAppContext", VOID_CONTEXT,
            reinterpret_cast<void *>(EtsAbilityDelegator::GetAppContext)},
        ani_native_function {"nativeExecuteShellCommand", STRING_NUMBER_ASYNCCALLBACK,
            reinterpret_cast<void *>(EtsAbilityDelegator::ExecuteShellCommand)},
        ani_native_function {"nativeFinishTest", STRING_NUMBER_ASYNCCALLBACK,
            reinterpret_cast<void *>(EtsAbilityDelegator::FinishTest)},
        ani_native_function {"printSync", STRING_VOID,
            reinterpret_cast<void *>(EtsAbilityDelegator::PrintSync)},
        ani_native_function {"nativeStartAbility", WANT_ASYNCCALLBACK,
            reinterpret_cast<void *>(EtsAbilityDelegator::StartAbility)},
        ani_native_function {"printAsync", STRING_ASYNCCALLBACK_VOID,
            reinterpret_cast<void *>(EtsAbilityDelegator::Print)},
        ani_native_function {"getAbilityState", STRING_UIABILITY_NUMBER,
            reinterpret_cast<void *>(EtsAbilityDelegator::GetAbilityState)},
        ani_native_function {"nativeGetCurrentTopAbility", STRING_CALLBACK_UIABILITY,
            reinterpret_cast<void *>(EtsAbilityDelegator::GetCurrentTopAbility)},
        ani_native_function {"doAbilityForegroundAsync", STAGEMONITOR_UIABILITY_ASYNCCALLBACK,
            reinterpret_cast<void *>(EtsAbilityDelegator::DoAbilityForeground)},
        ani_native_function {"doAbilityForegroundOrBackgroundCheck", STAGEMONITOR_UIABILITY_CHECK,
            reinterpret_cast<void *>(EtsAbilityDelegator::DoAbilityForegroundOrBackgroundCheck)},
        ani_native_function {"doAbilityBackgroundAsync", STAGEMONITOR_UIABILITY_ASYNCCALLBACK,
            reinterpret_cast<void *>(EtsAbilityDelegator::DoAbilityBackground)},
        ani_native_function {"addInteropAbilityMonitorSync", INTEROP_MONITOR_VOID,
            reinterpret_cast<void *>(EtsAbilityDelegator::AddInteropAbilityMonitorSync)},
        ani_native_function {"removeInteropAbilityMonitorSync", INTEROP_MONITOR_VOID,
            reinterpret_cast<void *>(EtsAbilityDelegator::RemoveInteropAbilityMonitorSync)}
    };
}

auto GetMonitorNativeFunctions()
{
    return std::array {
        ani_native_function {"nativeAddAbilityMonitor", MONITOR_ASYNCCALLBACK_VOID,
            reinterpret_cast<void *>(EtsAbilityDelegator::AddAbilityMonitor)},
        ani_native_function {"addAbilityMonitorCheck", SIGNATURE_MONITOR_VOID,
            reinterpret_cast<void *>(EtsAbilityDelegator::AddAbilityMonitorCheck)},
        ani_native_function {"addAbilityMonitorSync", SIGNATURE_MONITOR_VOID,
            reinterpret_cast<void *>(EtsAbilityDelegator::AddAbilityMonitorSync)},
        ani_native_function {"removeAbilityMonitorAsync", MONITOR_ASYNCCALLBACK_VOID,
            reinterpret_cast<void *>(EtsAbilityDelegator::RemoveAbilityMonitor)},
        ani_native_function {"removeAbilityMonitorCheck", SIGNATURE_MONITOR_VOID,
            reinterpret_cast<void *>(EtsAbilityDelegator::RemoveAbilityMonitorCheck)},
        ani_native_function {"removeAbilityMonitorSync", SIGNATURE_MONITOR_VOID,
            reinterpret_cast<void *>(EtsAbilityDelegator::RemoveAbilityMonitorSync)},
        ani_native_function {"waitAbilityMonitorAsync", SIGNATURE_MONITOR_NUMBER_ASYNCCALLBACK,
            reinterpret_cast<void *>(EtsAbilityDelegator::WaitAbilityMonitor)},
        ani_native_function {"addAbilityStageMonitorAsync", STAGEMONITOR_ASYNCCALLBACK_VOID,
            reinterpret_cast<void *>(EtsAbilityDelegator::AddAbilityStageMonitor)},
        ani_native_function {"addAbilityStageMonitorSync", STAGEMONITOR_VOID,
            reinterpret_cast<void *>(EtsAbilityDelegator::AddAbilityStageMonitorSync)},
        ani_native_function {"addAbilityStageMonitorCheck", STAGEMONITOR_VOID,
            reinterpret_cast<void *>(EtsAbilityDelegator::AddAbilityStageMonitorCheck)},
        ani_native_function {"removeAbilityStageMonitorAsync", STAGEMONITOR_ASYNCCALLBACK_VOID,
            reinterpret_cast<void *>(EtsAbilityDelegator::RemoveAbilityStageMonitor)},
        ani_native_function {"removeAbilityStageMonitorSync", STAGEMONITOR_VOID,
            reinterpret_cast<void *>(EtsAbilityDelegator::RemoveAbilityStageMonitorSync)},
        ani_native_function {"removeAbilityStageMonitorCheck", STAGEMONITOR_VOID,
            reinterpret_cast<void *>(EtsAbilityDelegator::RemoveAbilityStageMonitorCheck)},
        ani_native_function {"waitAbilityStageMonitorAsync", SIGNATURE_STAGEMONITOR_NUMBER_ASYNCCALLBACK,
            reinterpret_cast<void *>(EtsAbilityDelegator::WaitAbilityStageMonitor)}
    };
}

template<typename T>
bool BindNativeArray(ani_env *aniEnv, ani_class abilityDelegator, const T &functions)
{
    ani_status status = aniEnv->Class_BindNativeMethods(abilityDelegator, functions.data(), functions.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_BindNativeMethods failed status: %{public}d", status);
        return false;
    }
    return true;
}

bool BindFunctions(ani_env *aniEnv, ani_class abilityDelegator)
{
    if (aniEnv == nullptr) {
        return false;
    }
    auto baseFuncs = GetBaseNativeFunctions();
    auto monitorFuncs = GetMonitorNativeFunctions();
    if (!BindNativeArray(aniEnv, abilityDelegator, baseFuncs)) {
        return false;
    }
    return BindNativeArray(aniEnv, abilityDelegator, monitorFuncs);
}

ani_object CreateEtsAbilityDelegator(ani_env *aniEnv)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "CreateEtsAbilityDelegator");
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null aniEnv");
        return nullptr;
    }
    ani_class abilityDelegator = nullptr;
    ani_status status = ANI_ERROR;
    status = aniEnv->FindClass(ABILITY_DELEGATOR_CLASS_NAME, &abilityDelegator);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "find AbilityDelegatorInner failed status: %{public}d", status);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::DELEGATOR, "find AbilityDelegator success");

    if (!BindFunctions(aniEnv, abilityDelegator)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "BindFunctions failed");
        return nullptr;
    }

    ani_method method = nullptr;
    status = aniEnv->Class_FindMethod(abilityDelegator, "<ctor>", ":", &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod ctor failed status: %{public}d", status);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::DELEGATOR, "Class_FindMethod ctor success");

    ani_object object = nullptr;
    if (aniEnv->Object_New(abilityDelegator, method, &object) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Object_New failed status: %{public}d", status);
        return nullptr;
    }

    TAG_LOGD(AAFwkTag::DELEGATOR, "CreateEtsAbilityDelegator success");
    return object;
}

void SetBundleName(ani_env *aniEnv, ani_class arguments, ani_object argumentObject, const std::string &bundleName)
{
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null aniEnv");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_string aniStr = nullptr;
    // Get a ani_string from std::string
    status = aniEnv->String_NewUTF8(bundleName.c_str(), bundleName.length(), &aniStr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "String_NewUTF8 failed status: %{public}d", status);
        return;
    }
    TAG_LOGD(AAFwkTag::DELEGATOR, "String_NewUTF8 success");

    // find the setter method
    ani_method nameSetter = nullptr;
    status = aniEnv->Class_FindMethod(arguments, "%%set-bundleName", nullptr, &nameSetter);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod failed status: %{public}d", status);
        return;
    }
    TAG_LOGD(AAFwkTag::DELEGATOR, "Class_FindMethod success");

    // call set bundleName(n:string)
    status = aniEnv->Object_CallMethod_Void(argumentObject, nameSetter, aniStr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Object_CallMethod_Void failed status: %{public}d", status);
        return;
    }
    TAG_LOGD(AAFwkTag::DELEGATOR, "Object_CallMethod_Void success");
}

void SetParameters(ani_env *aniEnv, ani_class arguments, ani_object argumentObject,
    const std::map<std::string, std::string> &paras)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "SetParameters begin");
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null aniEnv");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_class recordCls = nullptr;
    status = aniEnv->FindClass(RECORD_CLASS_NAME, &recordCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "FindClass failed status: %{public}d", status);
        return;
    }
    ani_method recordGetMethod = nullptr;
    status = aniEnv->Class_FindMethod(recordCls, "$_get", RECORD_GET_NAME, &recordGetMethod);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod failed status: %{public}d", status);
        return;
    }
    ani_method recordSetMethod = nullptr;
    status = aniEnv->Class_FindMethod(recordCls, "$_set", RECORD_SET_NAME, &recordSetMethod);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod failed status: %{public}d", status);
        return;
    }
    ani_ref parameterRef = nullptr;
    status = aniEnv->Object_CallMethodByName_Ref(argumentObject, "%%get-parameters", ":C{std.core.Record}",
        &parameterRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Object_CallMethodByName_Ref failed status: %{public}d", status);
        return;
    }
    ani_object parameterObject = static_cast<ani_object>(parameterRef);
    for (auto iter = paras.begin(); iter != paras.end(); ++iter) {
        std::string key = iter->first;
        std::string value = iter->second;
        ani_string ani_key = nullptr;
        ani_string ani_value = nullptr;
        status = aniEnv->String_NewUTF8(key.c_str(), key.length(), &ani_key);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "String_NewUTF8 key failed status: %{public}d", status);
            return;
        }
        status = aniEnv->String_NewUTF8(value.c_str(), value.length(), &ani_value);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "String_NewUTF8 value failed status: %{public}d", status);
            return;
        }
        status = aniEnv->Object_CallMethod_Void(parameterObject, recordSetMethod, ani_key, ani_value);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "Object_CallMethod_Void failed status: %{public}d", status);
            return;
        }
    }
    TAG_LOGD(AAFwkTag::DELEGATOR, "SetParameters end");
}

void SetTestCaseNames(ani_env *aniEnv, ani_class arguments, ani_object argumentObject, const std::string &testcaseNames)
{
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null aniEnv");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_string aniStr = nullptr;
    status = aniEnv->String_NewUTF8(testcaseNames.c_str(), testcaseNames.length(), &aniStr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "String_NewUTF8 failed status: %{public}d", status);
        return;
    }
    TAG_LOGD(AAFwkTag::DELEGATOR, "String_NewUTF8 success");

    // find the setter method
    ani_method nameSetter = nullptr;
    status = aniEnv->Class_FindMethod(arguments, "%%set-testCaseNames", nullptr, &nameSetter);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod failed status: %{public}d", status);
        return;
    }
    TAG_LOGD(AAFwkTag::DELEGATOR, "Class_FindMethod success");

    // call set testcaseNames(n:string)
    status = aniEnv->Object_CallMethod_Void(argumentObject, nameSetter, aniStr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Object_CallMethod_Void failed status: %{public}d", status);
        return;
    }
    TAG_LOGD(AAFwkTag::DELEGATOR, "Object_CallMethod_Void success");
}

void SetTestRunnerClassName(ani_env *aniEnv, ani_class arguments, ani_object argumentObject,
    const std::string &className)
{
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null aniEnv");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_string aniStr = nullptr;
    status = aniEnv->String_NewUTF8(className.c_str(), className.length(), &aniStr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "String_NewUTF8 failed status: %{public}d", status);
        return;
    }
    TAG_LOGD(AAFwkTag::DELEGATOR, "String_NewUTF8 success");

    // find the setter method
    ani_method nameSetter = nullptr;
    status = aniEnv->Class_FindMethod(arguments, "%%set-testRunnerClassName", nullptr, &nameSetter);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod failed status: %{public}d", status);
        return;
    }
    TAG_LOGD(AAFwkTag::DELEGATOR, "Class_FindMethod success");

    // call set testRunnerClassName(n:string)
    status = aniEnv->Object_CallMethod_Void(argumentObject, nameSetter, aniStr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Object_CallMethod_Void failed status: %{public}d", status);
        return;
    }
    TAG_LOGD(AAFwkTag::DELEGATOR, "Object_CallMethod_Void success");
}

ani_object CreateEtsAbilityDelegatorArguments(
    ani_env *aniEnv, const std::shared_ptr<AppExecFwk::AbilityDelegatorArgs> abilityDelegatorArgs)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "CreateEtsAbilityDelegatorArguments");
    if (aniEnv == nullptr || abilityDelegatorArgs == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null aniEnv or abilityDelegatorArgs");
        return nullptr;
    }
    ani_class arguments = nullptr;
    ani_status status = ANI_ERROR;
    status = aniEnv->FindClass(ARGS_ABILITY_DELEGATOR_CLASS_NAME, &arguments);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "find abilityDelegatorArgs failed status: %{public}d", status);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::DELEGATOR, "find AbilityDelegatorArgs success");

    ani_method method = nullptr;
    status = aniEnv->Class_FindMethod(arguments, "<ctor>", ":", &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod ctor failed status: %{public}d", status);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::DELEGATOR, "Class_FindMethod ctor success");

    ani_object argumentObject = nullptr;
    status = aniEnv->Object_New(arguments, method, &argumentObject);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Object_New failed status: %{public}d", status);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::DELEGATOR, "Object_New success");

    std::string bundleName = abilityDelegatorArgs->GetTestBundleName();
    SetBundleName(aniEnv, arguments, argumentObject, bundleName);

    std::string testcaseName = abilityDelegatorArgs->GetTestCaseName();
    SetTestCaseNames(aniEnv, arguments, argumentObject, testcaseName);

    std::string className = abilityDelegatorArgs->GetTestRunnerClassName();
    SetTestRunnerClassName(aniEnv, arguments, argumentObject, className);

    auto parameters = abilityDelegatorArgs->GetTestParam();
    SetParameters(aniEnv, arguments, argumentObject, parameters);

    return argumentObject;
}
} // namespace AbilityDelegatorEts
} // namespace OHOS
