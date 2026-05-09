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

#include "ets_ability_delegator.h"

#include <mutex>
#include <sstream>
#include "ability_delegator_registry.h"
#include "ability_stage_monitor.h"
#include "ani_common_want.h"
#include "ani_enum_convert.h"
#include "ets_ability_monitor.h"
#include "ets_context_utils.h"
#include "ets_error_utils.h"
#include "ets_native_reference.h"
#include "hilog_tag_wrapper.h"
#include "shell_cmd_result.h"

namespace OHOS {
namespace AbilityDelegatorEts {

using namespace OHOS::AbilityRuntime;

std::map<std::shared_ptr<ETSNativeReference>, std::shared_ptr<EtsAbilityMonitor>> g_monitorRecord;
std::map<std::shared_ptr<ETSNativeReference>, std::shared_ptr<EtsAbilityStageMonitor>> g_stageMonitorRecord;
std::map<std::weak_ptr<ETSNativeReference>, sptr<IRemoteObject>, std::owner_less<>> g_abilityRecord;
std::mutex g_mtxMonitorRecord;
std::mutex g_mtxStageMonitorRecord;
std::mutex g_mutexAbilityRecord;
std::map<std::shared_ptr<ETSNativeReference>, std::shared_ptr<EtsInteropAbilityMonitor>> g_interopMonitorRecord;
std::mutex g_mtxInteropMonitorRecord;

namespace {
constexpr const char* CONTEXT_CLASS_NAME = "application.Context.Context";
constexpr const char* SHELL_CMD_RESULT_CLASS_NAME = "application.shellCmdResult.ShellCmdResultImpl";
constexpr const char* ABILITY_STAGE_CLASS_NAME = "@ohos.app.ability.AbilityStage.AbilityStage";
constexpr int COMMON_FAILED = 16000100;
constexpr ani_double INVALID_LIFECYCLE_STATE = -1;
}

EtsAbilityDelegator::EtsAbilityDelegator()
{
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (delegator) {
        auto clearFunc = [](const std::shared_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &baseProperty) {
            auto property = std::static_pointer_cast<AppExecFwk::EtsDelegatorAbilityProperty>(baseProperty);
            if (!property) {
                TAG_LOGE(AAFwkTag::DELEGATOR, "invalid property type");
                return;
            }

            std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
            for (auto it = g_abilityRecord.begin(); it != g_abilityRecord.end();) {
                if (it->second == property->token_) {
                    it = g_abilityRecord.erase(it);
                    continue;
                }
                ++it;
            }
        };

        delegator->RegisterClearFunc(clearFunc);
    }
}

EtsAbilityDelegator::~EtsAbilityDelegator() = default;

ani_object EtsAbilityDelegator::SetAppContext(ani_env *env, const std::shared_ptr<AbilityRuntime::Context> &context)
{
    if (env == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null env or context");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_object contextObj = nullptr;
    ani_method method = nullptr;
    ani_status status = env->FindClass(CONTEXT_CLASS_NAME, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "FindClass status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &contextObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Object_New status: %{public}d", status);
        return nullptr;
    }

    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::Context>(context);
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null workContext");
        return nullptr;
    }
    ani_long nativeContextLong = reinterpret_cast<ani_long>(workContext);
    if (!ContextUtil::SetNativeContextLong(env, contextObj, nativeContextLong)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "SetNativeContextLong failed");
        delete workContext;
        workContext = nullptr;
        return nullptr;
    }
    return contextObj;
}
ani_object EtsAbilityDelegator::WrapShellCmdResult(ani_env *env, std::unique_ptr<AppExecFwk::ShellCmdResult> result)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "WrapShellCmdResult called");
    if (result == nullptr || env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "result or env is null");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    status = env->FindClass(SHELL_CMD_RESULT_CLASS_NAME, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "find AbilityDelegator failed status: %{public}d", status);
        return nullptr;
    }
    ani_method method = nullptr;
    status = env->Class_FindMethod(cls, "<ctor>", ":", &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod ctor failed status: %{public}d", status);
        return nullptr;
    }
    ani_object object = nullptr;
    if (env->Object_New(cls, method, &object) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Object_New failed status: %{public}d", status);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::DELEGATOR, "Object_New success");
    ani_field filed = nullptr;
    status = env->Class_FindField(cls, "stdResult", &filed);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindField failed status: %{public}d", status);
    }
    ani_string aniStringVal = nullptr;
    std::string strResult = result->GetStdResult();
    status = env->String_NewUTF8(strResult.c_str(), strResult.size(), &aniStringVal);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "String_NewUTF8 failed status: %{public}d", status);
    }
    status = env->Object_SetField_Ref(object, filed, aniStringVal);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "set strResult failed status: %{public}d", status);
    }
    int32_t exitCode = result->GetExitCode();
    status = env->Object_SetPropertyByName_Int(object, "exitCode", exitCode);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "set exitCode failed status: %{public}d", status);
    }
    return object;
}

ani_object EtsAbilityDelegator::GetAppContext(ani_env *env, [[maybe_unused]]ani_object object, ani_class clss)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "GetAppContext call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_object nullobj = nullptr;
    if (ANI_OK != env->FindClass(CONTEXT_CLASS_NAME, &cls)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "FindClass Context Failed");
        return nullobj;
    }
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (!delegator) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        return nullobj;
    }
    std::shared_ptr<AbilityRuntime::Context> context = delegator->GetAppContext();
    if (!context) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null context");
        return nullobj;
    }
    ani_object contextObj = SetAppContext(env, context);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null contextObj");
        return nullobj;
    }
    ContextUtil::CreateEtsBaseContext(env, cls, contextObj, context);
    TAG_LOGD(AAFwkTag::DELEGATOR, "GetAppContext end");
    return contextObj;
}

void EtsAbilityDelegator::ExecuteShellCommand(ani_env *env, [[maybe_unused]]ani_object object,
    ani_string cmd, ani_long timeoutSecs, ani_object callback)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "ExecuteShellCommand called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null env");
        return;
    }
    std::string stdCmd = "";
    if (!AppExecFwk::GetStdString(env, cmd, stdCmd)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GetStdString Failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
            "Parse parameters failed, cmd must be string and timeout must be number.");
        return;
    }
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (!delegator) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "delegator is nullptr");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    auto result = delegator->ExecuteShellCommand(stdCmd, static_cast<int64_t>(timeoutSecs));
    ani_object objValue = WrapShellCmdResult(env, std::move(result));
    if (objValue == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null objValue");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            COMMON_FAILED, "executeShellCommand failed."), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, nullptr, objValue);
    return;
}

void EtsAbilityDelegator::FinishTest(ani_env *env, [[maybe_unused]]ani_object object,
    ani_string msg, ani_long code, ani_object callback)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null env");
        return;
    }
    std::string stdMsg = "";
    if (!AppExecFwk::GetStdString(env, msg, stdMsg)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GetStdString Failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
            "Parse parameters failed, msg must be string and code must be number.");
        return;
    }
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (!delegator) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "FinishTest delegator is null");
        AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env,
            COMMON_FAILED, "Calling FinishTest failed."), nullptr);
        return;
    } else {
        delegator->FinishUserTest(stdMsg, static_cast<int64_t>(code));
    }
    AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),
        nullptr);
    return;
}

void EtsAbilityDelegator::PrintSync(ani_env *env, [[maybe_unused]]ani_class aniClass, ani_string msg)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "PrintSync");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return;
    }
    std::string strMsg = "";
    if (!AppExecFwk::GetStdString(env, msg, strMsg)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GetStdString Failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
            "Parse msg failed, msg must be string.");
        return;
    }
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (delegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        return;
    }
    delegator->Print(strMsg);
    return;
}

void EtsAbilityDelegator::AddAbilityMonitorCheck(ani_env *env, ani_object aniObj, ani_object monitorObj)
{
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null env or aniObj");
        return;
    }
    std::shared_ptr<EtsAbilityMonitor> monitorImpl = nullptr;
    if (!ParseMonitorPara(env, monitorObj, monitorImpl)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
            "Parse param monitor failed, monitor must be Monitor.");
    }
}

void EtsAbilityDelegator::AddAbilityMonitor(ani_env *env, [[maybe_unused]]ani_class aniClass,
    ani_object monitorObj, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return;
    }
    std::shared_ptr<EtsAbilityMonitor> monitorImpl = nullptr;
    if (!ParseMonitorPara(env, monitorObj, monitorImpl)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "ParseMonitorPara failed");
        return;
    }
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (delegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env,
            COMMON_FAILED, "Calling AddAbilityMonitor failed."), nullptr);
        return;
    }
    delegator->AddAbilityMonitor(monitorImpl);
    AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),
        nullptr);
    return;
}

void EtsAbilityDelegator::AddAbilityMonitorSync(ani_env *env, [[maybe_unused]]ani_class aniClass,
    ani_object monitorObj)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "AddAbilityMonitorSync");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return;
    }
    std::shared_ptr<EtsAbilityMonitor> monitorImpl = nullptr;
    if (!ParseMonitorPara(env, monitorObj, monitorImpl)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "ParseMonitorPara failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
            "Parse param monitor failed, monitor must be Monitor.");
        return;
    }
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (delegator) {
        delegator->AddAbilityMonitor(monitorImpl);
    } else {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, COMMON_FAILED, "Calling AddAbilityMonitorSync failed.");
    }
    return;
}

void EtsAbilityDelegator::StartAbility(ani_env *env, [[maybe_unused]]ani_object object,
    ani_object wantObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "StartAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "UnwrapWant failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
            "Parse want failed, want must be Want.");
        return;
    }
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (delegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateError(env, COMMON_FAILED, "startAbility failed."),
            nullptr);
        return;
    }
    int result = delegator->StartAbility(want);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "start ability failed: %{public}d", result);
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, result, "startAbility failed."),
            nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),
        nullptr);
    return;
}

ani_ref EtsAbilityDelegator::GetCurrentTopAbility(ani_env* env, [[maybe_unused]]ani_class aniClass,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    ani_object objValue = nullptr;
    int32_t resultCode = COMMON_FAILED;
    std::string resultMsg = "Calling GetCurrentTopAbility failed.";
    do {
        auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(
            AbilityRuntime::Runtime::Language::ETS);
        if (delegator == nullptr) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "delegator is nullptr");
            break;
        }
        auto property = delegator->GetCurrentTopAbility();
        auto etsbaseProperty = std::static_pointer_cast<AppExecFwk::EtsDelegatorAbilityProperty>(property);
        if (etsbaseProperty == nullptr) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "property is nullptr");
            break;
        }
        auto ability = etsbaseProperty->object_.lock();
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "invalid property");
            break;
        }
        resultCode = 0;
        resultMsg = "";
        std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
        g_abilityRecord.emplace(etsbaseProperty->object_, etsbaseProperty->token_);
        objValue = ability->aniObj;
    } while (0);
    ani_ref callbackRef = nullptr;
    auto status = env->GlobalReference_Create(callback, &callbackRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Create Global ref for delegator failed %{public}d", status);
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return objValue;
    }
    AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef),
        AbilityRuntime::EtsErrorUtil::CreateError(env, resultCode, resultMsg), objValue);
    return objValue;
}

void EtsAbilityDelegator::RemoveAbilityMonitorCheck(ani_env *env, ani_object aniObj, ani_object monitorObj)
{
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null env or aniObj");
        return;
    }
    std::shared_ptr<EtsAbilityMonitor> monitorImpl = nullptr;
    if (!ParseMonitorPara(env, monitorObj, monitorImpl)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
            "Parse monitor failed, removeAbilityMonitor must be Monitor.");
    }
}

void EtsAbilityDelegator::RemoveAbilityMonitor(ani_env *env, [[maybe_unused]]ani_class aniClass,
    ani_object monitorObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "RemoveAbilityMonitor called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return;
    }
    std::shared_ptr<EtsAbilityMonitor> monitorImpl = nullptr;
    if (!ParseMonitorPara(env, monitorObj, monitorImpl)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "ParseMonitorPara failed");
        return;
    }
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (delegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateError(env, COMMON_FAILED, "Calling RemoveAbilityMonitor failed."),
            nullptr);
        return;
    } else {
        delegator->RemoveAbilityMonitor(monitorImpl);
        CleanAndFindMonitorRecord(env, monitorObj);
    }
    AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),
        nullptr);
    return;
}

void EtsAbilityDelegator::RemoveAbilityMonitorSync(ani_env *env, [[maybe_unused]]ani_class aniClass,
    ani_object monitorObj)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "RemoveAbilityMonitorSync called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return;
    }
    std::shared_ptr<EtsAbilityMonitor> monitorImpl = nullptr;
    if (!ParseMonitorPara(env, monitorObj, monitorImpl)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "ParseMonitorPara failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
            "Parse monitor failed, RemoveAbilityMonitorSync must be Monitor.");
        return;
    }
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (delegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, COMMON_FAILED, "Calling RemoveAbilityMonitorSync failed.");
        return;
    }
    delegator->RemoveAbilityMonitor(monitorImpl);
    CleanAndFindMonitorRecord(env, monitorObj);
    return;
}

void EtsAbilityDelegator::WaitAbilityMonitor(ani_env *env, [[maybe_unused]]ani_class aniClass,
    ani_object monitorObj, ani_long timeout, ani_object callback)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "WaitAbilityMonitor called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return;
    }
    std::shared_ptr<EtsAbilityMonitor> monitorImpl = nullptr;
    if (!ParseMonitorPara(env, monitorObj, monitorImpl)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return;
    }
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (delegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    std::shared_ptr<BaseDelegatorAbilityProperty> property = (static_cast<int64_t>(timeout) > 0) ?
            delegator->WaitAbilityMonitor(monitorImpl, static_cast<int64_t>(timeout)) :
            delegator->WaitAbilityMonitor(monitorImpl);
    ani_object resultAniOj = nullptr;
    auto etsbaseProperty = std::static_pointer_cast<AppExecFwk::EtsDelegatorAbilityProperty>(property);
    if (!etsbaseProperty || etsbaseProperty->object_.expired()) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid etsbaseProperty");
        AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env, COMMON_FAILED,
            "Calling WaitAbilityMonitor failed."),
            resultAniOj);
        return;
    }
    std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
    if (etsbaseProperty->object_.lock() == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "etsbaseProperty->object_ is null");
        AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env, COMMON_FAILED,
            "Calling WaitAbilityMonitor failed."),
            resultAniOj);
        return;
    }
    g_abilityRecord.emplace(etsbaseProperty->object_, etsbaseProperty->token_);
    resultAniOj = etsbaseProperty->object_.lock()->aniObj;
    AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),
        resultAniOj);
    return;
}

void EtsAbilityDelegator::AddAbilityStageMonitor(ani_env *env, [[maybe_unused]]ani_class aniClass,
    ani_object stageMonitorObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "AddAbilityStageMonitor called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return;
    }
    bool isExisted = false;
    std::shared_ptr<EtsAbilityStageMonitor> stageMonitor = nullptr;
    if (!ParseStageMonitorPara(env, stageMonitorObj, stageMonitor, isExisted)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return;
    }
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (delegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateError(env, COMMON_FAILED, "Calling AddAbilityStageMonitor failed."),
            nullptr);
        return;
    }
    delegator->AddAbilityStageMonitor(stageMonitor);
    if (!isExisted) {
        AddStageMonitorRecord(env, stageMonitorObj, stageMonitor);
    }
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),
        nullptr);
    return;
}

void EtsAbilityDelegator::AddAbilityStageMonitorCheck(ani_env *env, ani_object aniObj, ani_object stageMonitorObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "AddAbilityStageMonitorCheck");
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null env or aniObj");
        return;
    }
    bool isExisted = false;
    std::shared_ptr<EtsAbilityStageMonitor> stageMonitor = nullptr;
    if (!ParseStageMonitorPara(env, stageMonitorObj, stageMonitor, isExisted)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
            "Parse parameters failed, monitor must be Monitor and isExited must be boolean.");
    }
}

void EtsAbilityDelegator::AddAbilityStageMonitorSync(ani_env *env, [[maybe_unused]]ani_class aniClass,
    ani_object stageMonitorObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "AddAbilityStageMonitorSync called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return;
    }
    bool isExisted = false;
    std::shared_ptr<EtsAbilityStageMonitor> stageMonitor = nullptr;
    if (!ParseStageMonitorPara(env, stageMonitorObj, stageMonitor, isExisted)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
            "Parse param monitor failed, monitor must be Monitor.");
        return;
    }
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (delegator) {
        delegator->AddAbilityStageMonitor(stageMonitor);
        if (!isExisted) {
        AddStageMonitorRecord(env, stageMonitorObj, stageMonitor);
        }
    } else {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, COMMON_FAILED, "Calling AddAbilityStageMonitorSync failed.");
    }
    return;
}

void EtsAbilityDelegator::RemoveAbilityStageMonitor(ani_env *env, [[maybe_unused]]ani_class aniClass,
    ani_object stageMonitorObj, ani_object callback)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "RemoveAbilityStageMonitor called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return;
    }
    bool isExisted = false;
    std::shared_ptr<EtsAbilityStageMonitor> stageMonitor = nullptr;
    if (!ParseStageMonitorPara(env, stageMonitorObj, stageMonitor, isExisted)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return;
    }
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (delegator == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateError(env, COMMON_FAILED, "Calling RemoveAbilityStageMonitor failed."),
            nullptr);
        return;
    }
    delegator->RemoveAbilityStageMonitor(stageMonitor);
    AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),
        nullptr);
    if (isExisted) {
        RemoveStageMonitorRecord(env, stageMonitorObj);
    }
    return;
}

void EtsAbilityDelegator::RemoveAbilityStageMonitorCheck(ani_env *env, ani_object aniObj, ani_object stageMonitorObj)
{
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null env or aniObj");
        return;
    }
    bool isExisted = false;
    std::shared_ptr<EtsAbilityStageMonitor> stageMonitor = nullptr;
    if (!ParseStageMonitorPara(env, stageMonitorObj, stageMonitor, isExisted)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
            "Parse monitor failed, removeAbilityMonitor must be Monitor.");
    }
}


void EtsAbilityDelegator::RemoveAbilityStageMonitorSync(ani_env *env, [[maybe_unused]]ani_class aniClass,
    ani_object stageMonitorObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "RemoveAbilityStageMonitorSync called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return;
    }
    bool isExisted = false;
    std::shared_ptr<EtsAbilityStageMonitor> stageMonitor = nullptr;
    if (!ParseStageMonitorPara(env, stageMonitorObj, stageMonitor, isExisted)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
            "Parse monitor failed, removeAbilityMonitor must be Monitor.");
        return;
    }
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (delegator) {
        delegator->RemoveAbilityStageMonitor(stageMonitor);
    } else {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, COMMON_FAILED, "Calling RemoveAbilityStageMonitorSync failed.");
    }

    if (isExisted) {
        RemoveStageMonitorRecord(env, stageMonitorObj);
    }
    return;
}

void EtsAbilityDelegator::WaitAbilityStageMonitor(ani_env *env, [[maybe_unused]]ani_class aniClass,
    ani_object stageMonitorObj, ani_long timeout, ani_object callback)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "WaitAbilityStageMonitor called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
            "Parse remoteObject failed, remoteObject must be RemoteObject.");
        return;
    }
    std::shared_ptr<EtsAbilityStageMonitor> stageMonitor = nullptr;
    if (!ParseWaitAbilityStageMonitorPara(env, stageMonitorObj, stageMonitor)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return;
    }
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (!delegator) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    std::shared_ptr<BaseDelegatorAbilityStageProperty> result;
    result = (static_cast<int64_t>(timeout) > 0) ?
            delegator->WaitAbilityStageMonitor(stageMonitor, static_cast<int64_t>(timeout)) :
            delegator->WaitAbilityStageMonitor(stageMonitor);
    int resultCode = 0;
    ani_object resultAniOj = nullptr;
    auto etsbaseProperty = std::static_pointer_cast<AppExecFwk::EtsDelegatorAbilityStageProperty>(result);
    if (CheckPropertyValue(env, resultCode, resultAniOj, etsbaseProperty)) {
        resultAniOj = etsbaseProperty->object_.lock()->aniObj;
    } else {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateError(env, resultCode, "Calling WaitAbilityStageMonitor failed."),
            resultAniOj);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),
        resultAniOj);
    return;
}

void EtsAbilityDelegator::DoAbilityForegroundOrBackgroundCheck(ani_env *env, [[maybe_unused]]ani_object object,
    ani_object abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "DoAbilityForegroundCheck called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return;
    }
    sptr<OHOS::IRemoteObject> remoteObject = nullptr;
    if (!ParseAbilityCommonPara(env, abilityObj, remoteObject)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
            "Parse remoteObject failed, remoteObject must be RemoteObject.");
        return;
    }
}

void EtsAbilityDelegator::DoAbilityForeground(ani_env *env, [[maybe_unused]]ani_object object,
    ani_object abilityObj, ani_object callback)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "DoAbilityForeground called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return;
    }
    sptr<OHOS::IRemoteObject> remoteObject = nullptr;
    if (!ParseAbilityCommonPara(env, abilityObj, remoteObject)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
            "Parse remoteObject failed, remoteObject must be RemoteObject.");
        return;
    }
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (!delegator) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env, COMMON_FAILED,
            "Calling DoAbilityForeground failed."), nullptr);
        return;
    } else {
        if (!delegator->DoAbilityForeground(remoteObject)) {
            AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env, COMMON_FAILED,
                "Calling DoAbilityForeground failed."), nullptr);
            return;
        }
    }
    AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),
        nullptr);
    return;
}

void EtsAbilityDelegator::DoAbilityBackground(ani_env *env, [[maybe_unused]]ani_object object,
    ani_object abilityObj, ani_object callback)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "DoAbilityBackground called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return;
    }
    sptr<OHOS::IRemoteObject> remoteObject = nullptr;
    if (!ParseAbilityCommonPara(env, abilityObj, remoteObject)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return;
    }
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (!delegator) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env, COMMON_FAILED,
            "Calling DoAbilityBackground failed."), nullptr);
        return;
    } else {
        if (!delegator->DoAbilityBackground(remoteObject)) {
            AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env, COMMON_FAILED,
            "Calling DoAbilityForeground failed."), nullptr);
            return;
        }
    }
    AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),
        nullptr);
    return;
}

void EtsAbilityDelegator::Print(ani_env *env, [[maybe_unused]]ani_object object,
    ani_string msg, ani_object callback)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "Print called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return;
    }
    std::string strMsg = "";
    if (!AppExecFwk::GetStdString(env, msg, strMsg)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GetStdString Failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
            "Parse parameters msg failed, msg must be string.");
        return;
    }
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (!delegator) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env, COMMON_FAILED,
        "print failed."), nullptr);
        return;
    }
    delegator->Print(strMsg);
    AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),
        nullptr);
    return;
}

ani_int EtsAbilityDelegator::GetAbilityState(ani_env *env, [[maybe_unused]]ani_object object, ani_object abilityObj)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "GetAbilityState called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return INVALID_LIFECYCLE_STATE;
    }
    sptr<OHOS::IRemoteObject> remoteObject = nullptr;
    if (!ParseAbilityCommonPara(env, abilityObj, remoteObject)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return INVALID_LIFECYCLE_STATE;
    }

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (!delegator) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, COMMON_FAILED);
        return INVALID_LIFECYCLE_STATE;
    }
    AbilityDelegator::AbilityState lifeState = delegator->GetAbilityState(remoteObject);
    AbilityLifecycleState abilityLifeState = AbilityLifecycleState::UNINITIALIZED;
    AbilityLifecycleStateToEts(lifeState, abilityLifeState);
    return static_cast<ani_int>(abilityLifeState);
}

void EtsAbilityDelegator::AbilityLifecycleStateToEts(
    const AbilityDelegator::AbilityState &lifeState, AbilityLifecycleState &abilityLifeState)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "lifeState: %{public}d", static_cast<int32_t>(lifeState));
    switch (lifeState) {
        case AbilityDelegator::AbilityState::STARTED:
            abilityLifeState = AbilityLifecycleState::CREATE;
            break;
        case AbilityDelegator::AbilityState::FOREGROUND:
            abilityLifeState = AbilityLifecycleState::FOREGROUND;
            break;
        case AbilityDelegator::AbilityState::BACKGROUND:
            abilityLifeState = AbilityLifecycleState::BACKGROUND;
            break;
        case AbilityDelegator::AbilityState::STOPPED:
            abilityLifeState = AbilityLifecycleState::DESTROY;
            break;
        default:
            abilityLifeState = AbilityLifecycleState::UNINITIALIZED;
            break;
    }
}

bool EtsAbilityDelegator::CheckMonitorPara(ani_env *env, ani_object monitorObj)
{
    if (env == nullptr || monitorObj == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env or monitorObj is nullptr");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_boolean isUndefined = true;
    if ((status = env->Reference_IsUndefined(reinterpret_cast<ani_ref>(monitorObj), &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Reference_IsUndefined status: %{public}d", status);
        return false;
    }
    ani_boolean isNull = true;
    if ((status = env->Reference_IsNullishValue(reinterpret_cast<ani_ref>(monitorObj), &isNull)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Reference_IsNullishValue status: %{public}d", status);
        return false;
    }
    if (isUndefined || isNull) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "monitorObj is undefined or null");
        return false;
    }
    return true;
}

bool EtsAbilityDelegator::ParseMonitorPara(ani_env *env, ani_object monitorObj,
    std::shared_ptr<EtsAbilityMonitor> &monitorImpl)
{
    if (env == nullptr || monitorObj == nullptr || !CheckMonitorPara(env, monitorObj)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env or monitorObj is nullptr");
        return false;
    }
    {
        std::unique_lock<std::mutex> lck(g_mtxMonitorRecord);
        TAG_LOGI(AAFwkTag::DELEGATOR, "monitorRecord size: %{public}zu", g_monitorRecord.size());
        for (auto iter = g_monitorRecord.begin(); iter != g_monitorRecord.end();) {
            if (iter->first.get() == nullptr) {
                TAG_LOGE(AAFwkTag::DELEGATOR, "monitorRecord is nullptr");
                iter = g_monitorRecord.erase(iter);
                continue;
            }
            std::shared_ptr<ETSNativeReference> etsMonitor = iter->first;
            if (etsMonitor == nullptr) {
                TAG_LOGE(AAFwkTag::DELEGATOR, "etsMonitor is nullptr");
                iter = g_monitorRecord.erase(iter);
                continue;
            }
            ani_boolean result = false;
            ani_status status = env->Reference_StrictEquals(reinterpret_cast<ani_ref>(monitorObj),
                etsMonitor->aniRef, &result);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::DELEGATOR, "Reference_StrictEquals failed status: %{public}d", status);
            }
            if (result) {
                monitorImpl = iter->second;
                return monitorImpl ? true : false;
            }
            ++iter;
        }
    }
    if (!ParseMonitorParaInner(env, monitorObj, monitorImpl)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "ParseMonitorParaInner failed");
        return false;
    }
    return true;
}

bool EtsAbilityDelegator::ParseMonitorParaInner(ani_env *env, ani_object monitorObj,
    std::shared_ptr<EtsAbilityMonitor> &monitorImpl)
{
    if (env == nullptr || monitorObj == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env or monitorObj is nullptr");
        return false;
    }

    std::shared_ptr<EtsAbilityMonitor> abilityMonitor = nullptr;
    std::string strAbilityName = "";
    if (!GetStringProperty(env, monitorObj, "abilityName", strAbilityName)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GetStringProperty failed");
        return false;
    }
    std::string strModuleName = "";
    if (!GetStringProperty(env, monitorObj, "moduleName", strModuleName)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GetStringProperty failed");
        abilityMonitor = std::make_shared<EtsAbilityMonitor>(strAbilityName);
    } else {
        abilityMonitor = std::make_shared<EtsAbilityMonitor>(strAbilityName, strModuleName);
    }
    abilityMonitor->SetEtsAbilityMonitor(env, monitorObj);
    monitorImpl = abilityMonitor;
    ani_status status = ANI_ERROR;
    ani_ref monitorRef = nullptr;
    if ((status = env->GlobalReference_Create(monitorObj, &monitorRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GlobalReference_Create failed, status: %{public}d", status);
        return false;
    }
    std::shared_ptr<ETSNativeReference> reference = std::make_shared<ETSNativeReference>();
    if (reference != nullptr) {
        reference->aniRef = monitorRef;
        reference->aniObj = monitorObj;
    }
    std::unique_lock<std::mutex> lck(g_mtxMonitorRecord);
    g_monitorRecord.emplace(reference, monitorImpl);
    return true;
}

bool EtsAbilityDelegator::ParseStageMonitorPara(ani_env *env, ani_object stageMonitorObj,
    std::shared_ptr<EtsAbilityStageMonitor> &stageMonitor, bool &isExisted)
{
    if (env == nullptr || stageMonitorObj == nullptr || !CheckMonitorPara(env, stageMonitorObj)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env or stageMonitorObj is nullptr");
        return false;
    }
    isExisted = false;
    {
        std::unique_lock<std::mutex> lck(g_mtxStageMonitorRecord);
        TAG_LOGI(AAFwkTag::DELEGATOR, "stageMonitorRecord size: %{public}zu", g_stageMonitorRecord.size());
        for (auto iter = g_stageMonitorRecord.begin(); iter != g_stageMonitorRecord.end();) {
            if (iter->first.get() == nullptr) {
                TAG_LOGE(AAFwkTag::DELEGATOR, "stagemonitorRecord is nullptr");
                iter = g_stageMonitorRecord.erase(iter);
                continue;
            }
            std::shared_ptr<ETSNativeReference> etsMonitor = iter->first;
            if (etsMonitor == nullptr) {
                TAG_LOGE(AAFwkTag::DELEGATOR, "etsMonitor is nullptr");
                iter = g_stageMonitorRecord.erase(iter);
                continue;
            }
            ani_boolean result = false;
            ani_status status = env->Reference_StrictEquals(reinterpret_cast<ani_ref>(stageMonitorObj),
                etsMonitor->aniRef, &result);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::DELEGATOR, "Reference_StrictEquals failed status: %{public}d", status);
            }
            if (result) {
                TAG_LOGW(AAFwkTag::DELEGATOR, "abilityStage monitor exist");
                isExisted = true;
                stageMonitor = iter->second;
                return stageMonitor ? true : false;
            }
            ++iter;
        }
    }
    if (!ParseStageMonitorParaInner(env, stageMonitorObj, stageMonitor)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "ParseStageMonitorParaInner failed");
        return false;
    }
    return true;
}

bool EtsAbilityDelegator::ParseStageMonitorParaInner(ani_env *env, ani_object stageMonitorObj,
    std::shared_ptr<EtsAbilityStageMonitor> &stageMonitor)
{
    if (env == nullptr || stageMonitorObj == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env or stageMonitorObj is nullptr");
        return false;
    }
    std::string strModuleName = "";
    if (!GetStringProperty(env, stageMonitorObj, "moduleName", strModuleName)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GetStringProperty failed");
        return false;
    }
    std::string srcEntrance = "";
    if (!GetStringProperty(env, stageMonitorObj, "srcEntrance", strModuleName)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GetStringProperty failed");
        return false;
    }
    TAG_LOGD(AAFwkTag::DELEGATOR, "srcEntrance %{public}s ", srcEntrance.c_str());
    stageMonitor = std::make_shared<EtsAbilityStageMonitor>(strModuleName, srcEntrance);
    return true;
}

void EtsAbilityDelegator::AddStageMonitorRecord(ani_env *env, ani_object stageMonitorObj,
    const std::shared_ptr<EtsAbilityStageMonitor> &stageMonitor)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "AddStageMonitorRecord called");
    if (env == nullptr || stageMonitor == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env or stageMonitor is nullptr");
        return;
    }
    if (!AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_ref stageMonitorRef = nullptr;
    if ((status = env->GlobalReference_Create(stageMonitorObj, &stageMonitorRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GlobalReference_Create failed, status: %{public}d", status);
        return;
    }
    std::shared_ptr<ETSNativeReference> reference = std::make_shared<ETSNativeReference>();
    reference->aniObj = stageMonitorObj;
    reference->aniRef = stageMonitorRef;
    {
        std::unique_lock<std::mutex> lck(g_mtxStageMonitorRecord);
        g_stageMonitorRecord.emplace(reference, stageMonitor);
        TAG_LOGI(AAFwkTag::DELEGATOR, "end, size: %{public}zu", g_stageMonitorRecord.size());
    }
}

void EtsAbilityDelegator::RemoveStageMonitorRecord(ani_env *env, ani_object stageMonitorObj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return;
    }
    if (!AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        return;
    }
    std::unique_lock<std::mutex> lck(g_mtxStageMonitorRecord);
    for (auto iter = g_stageMonitorRecord.begin(); iter != g_stageMonitorRecord.end();) {
        if (iter->first.get() == nullptr) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "stagemonitorRecord is nullptr");
            iter = g_stageMonitorRecord.erase(iter);
            continue;
        }
        std::shared_ptr<ETSNativeReference> etsMonitor = iter->first;
        if (etsMonitor == nullptr) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "etsMonitor is nullptr");
            iter = g_stageMonitorRecord.erase(iter);
            continue;
        }
        ani_boolean result = false;
        ani_status status = env->Reference_StrictEquals(reinterpret_cast<ani_ref>(stageMonitorObj),
            etsMonitor->aniRef, &result);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "Reference_StrictEquals failed status: %{public}d", status);
        }
        if (result) {
            status = env->GlobalReference_Delete(etsMonitor->aniRef);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::DELEGATOR, "GlobalReference_Delete failed status: %{public}d", status);
            }
            g_stageMonitorRecord.erase(iter);
            TAG_LOGI(AAFwkTag::DELEGATOR, "end, size: %{public}zu", g_stageMonitorRecord.size());
            break;
        }
        ++iter;
    }
}

bool EtsAbilityDelegator::ParseWaitAbilityStageMonitorPara(ani_env *env, ani_object stageMonitorObj,
    std::shared_ptr<EtsAbilityStageMonitor> &stageMonitor)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "ParseWaitAbilityStageMonitorPara called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return false;
    }
    bool isExisted = false;
    if (!ParseStageMonitorPara(env, stageMonitorObj, stageMonitor, isExisted)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "invalid params");
        return false;
    }
    if (!isExisted) {
        AddStageMonitorRecord(env, stageMonitorObj, stageMonitor);
    }
    return true;
}

bool EtsAbilityDelegator::ParseAbilityCommonPara(ani_env *env, ani_object abilityObj,
    sptr<OHOS::IRemoteObject> &remoteObject)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "g_abilityRecord size: %{public}zu", g_abilityRecord.size());
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return false;
    }
    std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
    for (auto iter = g_abilityRecord.begin(); iter != g_abilityRecord.end();) {
        if (iter->first.expired()) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "g_abilityRecord expired");
            iter = g_abilityRecord.erase(iter);
            continue;
        }
        std::shared_ptr<ETSNativeReference> etsMonitor = iter->first.lock();
        if (etsMonitor == nullptr) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "etsMonitor is null");
            iter = g_abilityRecord.erase(iter);
            continue;
        }
        ani_boolean result = false;
        ani_status status = env->Reference_StrictEquals(etsMonitor->aniRef,
            reinterpret_cast<ani_ref>(abilityObj), &result);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "Reference_StrictEquals failed status: %{public}d", status);
        }
        if (result) {
            remoteObject = iter->second;
            TAG_LOGI(AAFwkTag::DELEGATOR, "ability exist");
            return remoteObject ? true : false;
        }
        ++iter;
    }
    TAG_LOGE(AAFwkTag::DELEGATOR, "ability not exist");
    remoteObject = nullptr;
    return false;
}

bool EtsAbilityDelegator::CheckPropertyValue(ani_env *env, int &resultCode, ani_object &resultAniOj,
    std::shared_ptr<AppExecFwk::EtsDelegatorAbilityStageProperty> etsProperty)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return false;
    }
    if (!etsProperty || etsProperty->object_.expired()) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "waitAbilityStageMonitor failed");
        resultCode = COMMON_FAILED;
        ani_class cls = nullptr;
        ani_status status = env->FindClass(ABILITY_STAGE_CLASS_NAME, &cls);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "find AbilityDelegator failed status: %{public}d", status);
        }
        ani_method method = nullptr;
        status = env->Class_FindMethod(cls, "<ctor>", ":", &method);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod ctor failed status: %{public}d", status);
        }
        if (env->Object_New(cls, method, &resultAniOj) != ANI_OK) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "Object_New failed status: %{public}d", status);
        }
        return false;
    }
    return true;
}

void EtsAbilityDelegator::CleanAndFindMonitorRecord(ani_env *env, ani_object monitorObj)
{
    if (env == nullptr || monitorObj == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env or monitorObj is nullptr");
        return;
    }
    std::unique_lock<std::mutex> lck(g_mtxMonitorRecord);
    for (auto iter = g_monitorRecord.begin(); iter != g_monitorRecord.end();) {
        if (iter->first.get() == nullptr) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "monitorRecord is nullptr");
            iter = g_monitorRecord.erase(iter);
            continue;
        }
        std::shared_ptr<ETSNativeReference> etsMonitor = iter->first;
        if (etsMonitor == nullptr) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "etsMonitor is nullptr");
            iter = g_monitorRecord.erase(iter);
            continue;
        }
        ani_boolean result = false;
        ani_status status = env->Reference_StrictEquals(reinterpret_cast<ani_ref>(monitorObj),
            etsMonitor->aniRef, &result);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "Reference_StrictEquals failed status: %{public}d", status);
        }
        if (result) {
            status = env->GlobalReference_Delete(etsMonitor->aniRef);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::DELEGATOR, "GlobalReference_Delete failed status: %{public}d", status);
            }
            g_monitorRecord.erase(iter);
            break;
        }
        ++iter;
    }
}

void EtsAbilityDelegator::AddInteropAbilityMonitorSync(ani_env *env, [[maybe_unused]]ani_class aniClass,
    ani_object monitorObj)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "AddInteropAbilityMonitorSync called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return;
    }
    std::shared_ptr<EtsInteropAbilityMonitor> monitorImpl = nullptr;
    if (!ParseInteropMonitorPara(env, monitorObj, monitorImpl)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "ParseInteropMonitorPara failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
            "Parse param monitor failed, monitor must be InteropAbilityMonitor.");
        return;
    }
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(
        AbilityRuntime::Runtime::Language::ETS);
    if (delegator) {
        monitorImpl->SetLanguage(AbilityRuntime::Runtime::Language::ETS);
        delegator->AddInteropAbilityMonitor(monitorImpl);
    } else {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, COMMON_FAILED,
            "Calling AddInteropAbilityMonitorSync failed.");
    }
    return;
}

void EtsAbilityDelegator::RemoveInteropAbilityMonitorSync(ani_env *env, [[maybe_unused]]ani_class aniClass,
    ani_object monitorObj)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "RemoveInteropAbilityMonitorSync called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env is nullptr");
        return;
    }
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(
        AbilityRuntime::Runtime::Language::ETS);
    auto monitor = CleanAndFindInteropMonitorRecord(env, monitorObj);
    if (delegator && monitor) {
        delegator->RemoveInteropAbilityMonitor(monitor);
    } else if (!delegator) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null delegator");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, COMMON_FAILED,
            "Calling RemoveInteropAbilityMonitorSync failed.");
    }
    return;
}

bool EtsAbilityDelegator::ParseInteropMonitorPara(ani_env *env, ani_object monitorObj,
    std::shared_ptr<EtsInteropAbilityMonitor> &monitorImpl)
{
    if (env == nullptr || monitorObj == nullptr || !CheckMonitorPara(env, monitorObj)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env or monitorObj is nullptr");
        return false;
    }
    {
        std::unique_lock<std::mutex> lck(g_mtxInteropMonitorRecord);
        TAG_LOGI(AAFwkTag::DELEGATOR, "interopMonitorRecord size: %{public}zu", g_interopMonitorRecord.size());
        for (auto iter = g_interopMonitorRecord.begin(); iter != g_interopMonitorRecord.end();) {
            if (iter->first.get() == nullptr) {
                TAG_LOGE(AAFwkTag::DELEGATOR, "interopMonitorRecord is nullptr");
                iter = g_interopMonitorRecord.erase(iter);
                continue;
            }
            std::shared_ptr<ETSNativeReference> etsMonitor = iter->first;
            if (etsMonitor == nullptr) {
                TAG_LOGE(AAFwkTag::DELEGATOR, "etsMonitor is nullptr");
                iter = g_interopMonitorRecord.erase(iter);
                continue;
            }
            ani_boolean result = false;
            ani_status status = env->Reference_StrictEquals(reinterpret_cast<ani_ref>(monitorObj),
                etsMonitor->aniRef, &result);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::DELEGATOR, "Reference_StrictEquals failed status: %{public}d", status);
            }
            if (result) {
                TAG_LOGW(AAFwkTag::DELEGATOR, "interop monitor exist");
                monitorImpl = iter->second;
                return monitorImpl ? true : false;
            }
            ++iter;
        }
    }
    if (!ParseInteropMonitorParaInner(env, monitorObj, monitorImpl)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "ParseInteropMonitorParaInner failed");
        return false;
    }
    return true;
}

bool EtsAbilityDelegator::ParseInteropMonitorParaInner(ani_env *env, ani_object monitorObj,
    std::shared_ptr<EtsInteropAbilityMonitor> &monitorImpl)
{
    std::string strAbilityName = "";
    if (!GetStringProperty(env, monitorObj, "abilityName", strAbilityName)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GetStringProperty abilityName failed");
        return false;
    }
    std::string strModuleName = "";
    auto interopMonitor = std::make_shared<EtsInteropAbilityMonitor>(strAbilityName);
    if (GetStringProperty(env, monitorObj, "moduleName", strModuleName)) {
        interopMonitor = std::make_shared<EtsInteropAbilityMonitor>(strAbilityName, strModuleName);
    }
    interopMonitor->SetEtsInteropAbilityMonitor(env, monitorObj);
    auto *napiEnvPtr = reinterpret_cast<napi_env>(AppExecFwk::AbilityDelegatorRegistry::GetNapiEnv());
    if (napiEnvPtr != nullptr) {
        interopMonitor->SetNapiEnv(napiEnvPtr);
    }
    monitorImpl = interopMonitor;

    ani_ref monitorRef = nullptr;
    if (env->GlobalReference_Create(monitorObj, &monitorRef) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GlobalReference_Create failed");
        return false;
    }
    auto reference = std::make_shared<ETSNativeReference>();
    if (reference != nullptr) {
        reference->aniRef = monitorRef;
        reference->aniObj = monitorObj;
    }
    {
        std::unique_lock<std::mutex> lck(g_mtxInteropMonitorRecord);
        g_interopMonitorRecord.emplace(reference, monitorImpl);
    }
    return true;
}

std::shared_ptr<EtsInteropAbilityMonitor> EtsAbilityDelegator::CleanAndFindInteropMonitorRecord(
    ani_env *env, ani_object monitorObj)
{
    if (env == nullptr || monitorObj == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "env or monitorObj is nullptr");
        return nullptr;
    }
    std::unique_lock<std::mutex> lck(g_mtxInteropMonitorRecord);
    for (auto iter = g_interopMonitorRecord.begin(); iter != g_interopMonitorRecord.end();) {
        if (iter->first.get() == nullptr) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "interopMonitorRecord is nullptr");
            iter = g_interopMonitorRecord.erase(iter);
            continue;
        }
        std::shared_ptr<ETSNativeReference> etsMonitor = iter->first;
        if (etsMonitor == nullptr) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "etsMonitor is nullptr");
            iter = g_interopMonitorRecord.erase(iter);
            continue;
        }
        ani_boolean result = false;
        ani_status status = env->Reference_StrictEquals(reinterpret_cast<ani_ref>(monitorObj),
            etsMonitor->aniRef, &result);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "Reference_StrictEquals failed status: %{public}d", status);
        }
        if (result) {
            status = env->GlobalReference_Delete(etsMonitor->aniRef);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::DELEGATOR, "GlobalReference_Delete failed status: %{public}d", status);
            }
            auto monitor = iter->second;
            g_interopMonitorRecord.erase(iter);
            return monitor;
        }
        ++iter;
    }
    return nullptr;
}
} // namespace AbilityDelegatorEts
} // namespace OHOS
