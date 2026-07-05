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
#include "ets_ability_stage.h"

#include <algorithm>
#include <cstring>
#include <exception>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>

#include "ability_delegator_registry.h"
#include "ani_common_configuration.h"
#include "ani_common_want.h"
#include "ani_enum_convert.h"
#include "configuration_convertor.h"
#include "ets_ability_stage_context.h"
#include "ets_startup_config.h"
#include "ets_startup_task.h"
#include "event_report.h"
#include "freeze_util.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ohos_application.h"
#include "stage_context_transfer.h"
#include "startup_config_instance.h"
#include "startup_manager.h"
#include "startup_task_instance.h"
#include "startup_task_utils.h"
#include "application_env.h"

#ifdef WINDOWS_PLATFORM
#define ETS_EXPORT __declspec(dllexport)
#else
#define ETS_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* CALLBACK_SUCCESS = "success";
constexpr const char* ABILITY_STAGE_CLASS_NAME = "@ohos.app.ability.AbilityStage.AbilityStage";
constexpr const char* ABILITY_STAGE_SYNC_METHOD_NAME = "C{@ohos.app.ability.Want.Want}:C{std.core.String}";
constexpr const char* ABILITY_STAGE_ASYNC_METHOD_NAME = "C{@ohos.app.ability.Want.Want}:z";
constexpr const char* MEMORY_LEVEL_ENUM_NAME =
    "@ohos.app.ability.AbilityConstant.AbilityConstant.MemoryLevel";
constexpr const char *PREPARE_TERMINATION_CLASS_NAME =
    ":C{@ohos.app.ability.AbilityConstant.AbilityConstant.PrepareTermination}";
constexpr const char *PREPARE_TERMINATION_PROMISE_CALLBACK_METHOD_NAME =
    "C{@ohos.app.ability.AbilityConstant.AbilityConstant.PrepareTermination}:";

void OnPrepareTerminatePromiseCallback(ani_env* env, ani_object aniObj, ani_object dataObj)
{
    TAG_LOGD(AAFwkTag::APPKIT, "OnPrepareTerminatePromiseCallback called");

    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or null aniObj");
        return;
    }
    ani_long callbackPoint = 0;
    ani_status status = ANI_ERROR;
    if ((status = env->Object_GetFieldByName_Long(aniObj, "prepareTerminationCallbackPoint", &callbackPoint)) !=
        ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        return;
    }
    auto *callbackInfo =
        reinterpret_cast<AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult> *>(
            callbackPoint);
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null callbackInfo");
        return;
    }
    ani_boolean isUndefined = false;
    if ((status = env->Reference_IsUndefined(dataObj, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to check undefined status : %{public}d", status);
        return;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::APPKIT, "onPrepareTermination unimplemented");
        return;
    }
    ani_int result = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, dataObj, result)) {
        TAG_LOGE(AAFwkTag::APPKIT, "EnumConvert_EtsToNative param err");
        return;
    }
    AppExecFwk::OnPrepareTerminationResult prepareTerminationResult = { result, true };
    callbackInfo->Call(prepareTerminationResult);
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult>::Destroy(callbackInfo);

    if ((status = env->Object_SetFieldByName_Long(aniObj, "prepareTerminationCallbackPoint",
        static_cast<ani_long>(0))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status : %{public}d", status);
        return;
    }
}
} // namespace

AbilityStage *ETSAbilityStage::Create(
    const std::unique_ptr<Runtime>& runtime, const AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null runtime");
        return nullptr;
    }
    FreezeUtil::GetInstance().AddAppLifecycleEvent(0, "ETSAbilityStage::Create");

    auto& etsRuntime = static_cast<ETSRuntime&>(*runtime);
    return new (std::nothrow) ETSAbilityStage(etsRuntime);
}

ETSAbilityStage::ETSAbilityStage(ETSRuntime &etsRuntime) : etsRuntime_(etsRuntime)
{}

ETSAbilityStage::~ETSAbilityStage()
{
    TAG_LOGI(AAFwkTag::APPKIT, "destructor");
    auto context = GetContext();
    if (context != nullptr) {
        context->Unbind();
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
    }
}

void ETSAbilityStage::Init(const std::shared_ptr<Context> &context,
    const std::weak_ptr<AppExecFwk::OHOSApplication> application)
{
    AbilityStage::Init(context, application);
    SetShellContextRef(context);
    if (!BindNativeMethods()) {
        TAG_LOGE(AAFwkTag::APPKIT, "BindNativeMethods failed");
    }
}

void ETSAbilityStage::LoadModule(const AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::APPKIT, "AbilityStage::LoadModule");
    std::string srcPath(hapModuleInfo.name);
    std::string moduleName(hapModuleInfo.moduleName);
    moduleName.append("::").append("AbilityStage");

    srcPath.append("/");
    if (!hapModuleInfo.srcEntrance.empty()) {
        srcPath.append(hapModuleInfo.srcEntrance);
        auto pos = srcPath.rfind(".");
        if (pos != std::string::npos) {
            srcPath.erase(pos);
            srcPath.append(".abc");
        }
    }
    if (hapModuleInfo.srcEntrance.empty()) {
        return;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "entry path: %{public}s", hapModuleInfo.srcEntrance.c_str());
    bool esModule = hapModuleInfo.compileMode == AppExecFwk::CompileMode::ES_MODULE;
    etsAbilityStageObj_ = etsRuntime_.LoadModule(moduleName, srcPath,
        hapModuleInfo.hapPath, esModule, false, hapModuleInfo.srcEntrance);
    SetEtsAbilityStage();
}

void ETSAbilityStage::OnCreate(const AAFwk::Want &want) const
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPKIT, "OnCreate called");
    AbilityStage::OnCreate(want);

    FreezeUtil::GetInstance().AddAppLifecycleEvent(0, "ETSAbilityStage::OnCreate begin");
    CallObjectMethod(false, "onCreate", ":");
    FreezeUtil::GetInstance().AddAppLifecycleEvent(0, "ETSAbilityStage::OnCreate end");
    ClearAppPreload();

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (delegator) {
        delegator->PostPerformStageStart(CreateStageProperty());
    }
}

void ETSAbilityStage::OnDestroy() const
{
    TAG_LOGD(AAFwkTag::APPKIT, "OnDestroy called");
    AbilityStage::OnDestroy();
    CallObjectMethod(false, "onDestroy", ":");
}

std::string ETSAbilityStage::OnAcceptWant(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<std::string> *callbackInfo, bool &isAsync)
{
    TAG_LOGD(AAFwkTag::APPKIT, "OnAcceptWant called");
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "callbackInfo nullptr");
        return "";
    }
    AbilityStage::OnAcceptWant(want, callbackInfo, isAsync);

    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return "";
    }
    if (etsAbilityStageObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsAbilityStageObj_");
        return "";
    }
    ani_long acceptCallbackPoint = reinterpret_cast<ani_long>(callbackInfo);
    ani_status status = env->Object_SetFieldByName_Long(etsAbilityStageObj_->aniObj, "acceptCallbackPoint",
        acceptCallbackPoint);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_SetFieldByName_Long status: %{public}d", status);
        return "";
    }
    std::string methodName = "callOnAcceptWant";
    if (CallAcceptOrRequestAsync(env, want, methodName, isAsync)) {
        TAG_LOGD(AAFwkTag::APPKIT, "callOnAcceptWant is implemented");
        return CALLBACK_SUCCESS;
    }
    methodName = "onAcceptWant";
    isAsync = false;
    if (CallAcceptOrRequestSync(env, want, methodName, callbackInfo)) {
        return CALLBACK_SUCCESS;
    }
    return "";
}

std::string ETSAbilityStage::OnNewProcessRequest(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<std::string> *callbackInfo, bool &isAsync)
{
    TAG_LOGD(AAFwkTag::APPKIT, "OnNewProcessRequest called");
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "callbackInfo nullptr");
        return "";
    }
    AbilityStage::OnNewProcessRequest(want, callbackInfo, isAsync);

    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return "";
    }
    if (etsAbilityStageObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsAbilityStageObj_");
        return "";
    }
    ani_long newProcessRequestCallbackPoint = reinterpret_cast<ani_long>(callbackInfo);
    ani_status status = env->Object_SetFieldByName_Long(etsAbilityStageObj_->aniObj, "newProcessRequestCallbackPoint",
        newProcessRequestCallbackPoint);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_SetFieldByName_Long status: %{public}d", status);
        return "";
    }
    std::string methodName = "callOnNewProcessRequest";
    if (CallAcceptOrRequestAsync(env, want, methodName, isAsync)) {
        TAG_LOGD(AAFwkTag::APPKIT, "callOnNewProcessRequest is implemented");
        return CALLBACK_SUCCESS;
    }
    methodName = "onNewProcessRequest";
    isAsync = false;
    if (CallAcceptOrRequestSync(env, want, methodName, callbackInfo)) {
        return CALLBACK_SUCCESS;
    }
    return "";
}

void ETSAbilityStage::OnAcceptWantCallback(ani_env *env, ani_object aniObj, ani_string aniResult)
{
    TAG_LOGD(AAFwkTag::APPKIT, "OnAcceptWantCallback called");

    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or null aniObj");
        return;
    }
    ani_long callbackPoint = 0;
    ani_status status = ANI_ERROR;
    if ((status = env->Object_GetFieldByName_Long(aniObj, "acceptCallbackPoint", &callbackPoint)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        return;
    }
    auto *callbackInfo = reinterpret_cast<AppExecFwk::AbilityTransactionCallbackInfo<std::string> *>(callbackPoint);
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null callbackInfo");
        return;
    }
    std::string resultString = "";
    if (!AppExecFwk::GetStdString(env, aniResult, resultString)) {
        TAG_LOGE(AAFwkTag::APPKIT, "fail to get resultString");
        return;
    }
    callbackInfo->Call(resultString);
    AppExecFwk::AbilityTransactionCallbackInfo<std::string>::Destroy(callbackInfo);

    if ((status = env->Object_SetFieldByName_Long(aniObj, "acceptCallbackPoint",
        static_cast<ani_long>(0))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status : %{public}d", status);
        return;
    }
}

void ETSAbilityStage::OnNewProcessRequestCallback(ani_env *env, ani_object aniObj, ani_string aniResult)
{
    TAG_LOGD(AAFwkTag::APPKIT, "OnNewProcessRequestCallback called");

    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or null aniObj");
        return;
    }
    ani_long callbackPoint = 0;
    ani_status status = ANI_ERROR;
    if ((status = env->Object_GetFieldByName_Long(aniObj, "newProcessRequestCallbackPoint",
        &callbackPoint)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        return;
    }
    auto *callbackInfo = reinterpret_cast<AppExecFwk::AbilityTransactionCallbackInfo<std::string> *>(callbackPoint);
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null callbackInfo");
        return;
    }
    std::string resultString = "";
    if (!AppExecFwk::GetStdString(env, aniResult, resultString)) {
        TAG_LOGE(AAFwkTag::APPKIT, "fail to get resultString");
        return;
    }
    callbackInfo->Call(resultString);
    AppExecFwk::AbilityTransactionCallbackInfo<std::string>::Destroy(callbackInfo);

    if ((status = env->Object_SetFieldByName_Long(aniObj, "newProcessRequestCallbackPoint",
        static_cast<ani_long>(0))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status : %{public}d", status);
        return;
    }
}

void ETSAbilityStage::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    TAG_LOGD(AAFwkTag::APPKIT, "OnConfigurationUpdated called");
    AbilityStage::OnConfigurationUpdated(configuration);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env nullptr");
        return;
    }
    auto application = application_.lock();
    if (application == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "application is null");
        return;
    }
    auto fullConfig = application->GetConfiguration();
    if (!fullConfig) {
        TAG_LOGE(AAFwkTag::APPKIT, "null fullConfig");
        return;
    }
    ETSAbilityStageContext::ConfigurationUpdated(env, fullConfig);
    ani_object configObj = OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    if (configObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null configObj");
        return;
    }
    CallObjectMethod(false, "onConfigurationUpdate", "C{@ohos.app.ability.Configuration.Configuration}:", configObj);
}

void ETSAbilityStage::OnMemoryLevel(int32_t level)
{
    TAG_LOGD(AAFwkTag::APPKIT, "OnMemoryLevel called");
    AbilityStage::OnMemoryLevel(level);

    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env nullptr");
        return;
    }
    ani_enum_item memoryLevelItem {};
    OHOS::AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(env,
        MEMORY_LEVEL_ENUM_NAME, level, memoryLevelItem);

    CallObjectMethod(false, "onMemoryLevel", "C{@ohos.app.ability.AbilityConstant.AbilityConstant.MemoryLevel}:",
        memoryLevelItem);
    TAG_LOGD(AAFwkTag::APPKIT, "end");
}

void ETSAbilityStage::OnLaunchFromHyperSnap()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPKIT, "OnLaunchFromHyperSnap called");
    AbilityStage::OnLaunchFromHyperSnap();

    FreezeUtil::GetInstance().AddAppLifecycleEvent(0, "ETSAbilityStage::OnLaunchFromHyperSnap begin");
    CallObjectMethod(false, "onLaunchFromHyperSnap", ":");
    FreezeUtil::GetInstance().AddAppLifecycleEvent(0, "ETSAbilityStage::OnLaunchFromHyperSnap end");
}

void ETSAbilityStage::OnAboutToCreateAbility(bool &isAsyncCallback,
    const std::function<void(const std::shared_ptr<AbilityRuntime::Context> &)> &callback) const
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPKIT, "OnAboutToCreateAbility called");
    AbilityStage::OnAboutToCreateAbility(isAsyncCallback, callback);

    FreezeUtil::GetInstance().AddAppLifecycleEvent(0, "ETSAbilityStage::OnAboutToCreateAbility begin");
    CallObjectMethod(false, "onAboutToCreateAbility", ":");
    FreezeUtil::GetInstance().AddAppLifecycleEvent(0, "ETSAbilityStage::OnAboutToCreateAbility end");
}

bool ETSAbilityStage::CallObjectMethod(bool withResult, const char *name, const char *signature, ...) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    TAG_LOGD(AAFwkTag::APPKIT, "CallObjectMethod: name:%{public}s", name);
    if (etsAbilityStageObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "etsAbilityStageObj_ nullptr");
        return false;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env nullptr");
        return false;
    }
    ani_status status = ANI_OK;
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(etsAbilityStageObj_->aniCls, name, signature, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        return false;
    }
    env->ResetError();
    if (withResult) {
        ani_boolean res = ANI_FALSE;
        va_list args;
        va_start(args, signature);
        if ((status = env->Object_CallMethod_Boolean_V(etsAbilityStageObj_->aniObj, method, &res, args)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
            etsRuntime_.HandleUncaughtError();
        }
        va_end(args);
        return res;
    }
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(etsAbilityStageObj_->aniObj, method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        etsRuntime_.HandleUncaughtError();
        va_end(args);
        return false;
    }
    va_end(args);
    return false;
}

ani_object ETSAbilityStage::CallObjectMethod(const char *name, const char *signature, ...) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    TAG_LOGD(AAFwkTag::APPKIT, "ETSAbilityStage call ets, name: %{public}s", name);
    if (etsAbilityStageObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsAbilityStageObj_");
        return nullptr;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return nullptr;
    }
    auto obj = etsAbilityStageObj_->aniObj;
    auto cls = etsAbilityStageObj_->aniCls;
    ani_status status = ANI_ERROR;

    ani_method method {};
    if ((status = env->Class_FindMethod(cls, name, signature, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status : %{public}d", status);
        env->ResetError();
        return nullptr;
    }
    ani_ref res {};
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Ref_V(obj, method, &res, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status : %{public}d", status);
        etsRuntime_.HandleUncaughtError();
        va_end(args);
        return nullptr;
    }
    va_end(args);
    return reinterpret_cast<ani_object>(res);
}

std::shared_ptr<AppExecFwk::EtsDelegatorAbilityStageProperty> ETSAbilityStage::CreateStageProperty() const
{
    auto property = std::make_shared<AppExecFwk::EtsDelegatorAbilityStageProperty>();
    property->moduleName_ = GetHapModuleProp("name");
    property->srcEntrance_ = GetHapModuleProp("srcEntrance");
    return property;
}

std::string ETSAbilityStage::GetHapModuleProp(const std::string &propName) const
{
    auto context = GetContext();
    if (!context) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return std::string();
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (!hapModuleInfo) {
        TAG_LOGE(AAFwkTag::APPKIT, "null hapModuleInfo");
        return std::string();
    }
    if (propName.compare("name") == 0) {
        return hapModuleInfo->name;
    }
    if (propName.compare("srcEntrance") == 0) {
        return hapModuleInfo->srcEntrance;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "name = %{public}s", propName.c_str());
    return std::string();
}

bool ETSAbilityStage::CallAcceptOrRequestSync(ani_env *env, const AAFwk::Want &want, std::string &methodName,
    AppExecFwk::AbilityTransactionCallbackInfo<std::string> *callbackInfo) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "CallAcceptOrRequestSync called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return false;
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null wantRef");
        return false;
    }
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null callbackInfo");
        return false;
    }
    if (etsAbilityStageObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsAbilityObj_");
        return false;
    }
    ani_method method = nullptr;
    ani_status status = env->Class_FindMethod(etsAbilityStageObj_->aniCls, methodName.c_str(),
        ABILITY_STAGE_SYNC_METHOD_NAME, &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindMethod status: %{public}d, or null method", status);
        return false;
    }

    ani_value args[] = { { .r = wantRef } };
    ani_ref result = nullptr;
    if ((status = env->Object_CallMethod_Ref_A(etsAbilityStageObj_->aniObj, method, &result, args)) != ANI_OK ||
        result == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "CallMethod status: %{public}d, or null result", status);
        return false;
    }

    std::string resultString = "";
    if (!AppExecFwk::GetStdString(env, reinterpret_cast<ani_string>(result), resultString)) {
        TAG_LOGE(AAFwkTag::APPKIT, "fail to get resultString");
        return false;
    }
    callbackInfo->Call(resultString);
    AppExecFwk::AbilityTransactionCallbackInfo<std::string>::Destroy(callbackInfo);
    return true;
}

bool ETSAbilityStage::CallAcceptOrRequestAsync(ani_env *env, const AAFwk::Want &want, std::string &methodName,
    bool &isAsync) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "CallAcceptOrRequestAsync called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return false;
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null wantRef");
        return false;
    }
    if (etsAbilityStageObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsAbilityObj_");
        return false;
    }
    ani_method method = nullptr;
    ani_status status = env->Class_FindMethod(etsAbilityStageObj_->aniCls, methodName.c_str(),
        ABILITY_STAGE_ASYNC_METHOD_NAME, &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindMethod status: %{public}d, or null method", status);
        return false;
    }

    ani_value args[] = { { .r = wantRef } };
    ani_boolean res = ANI_FALSE;
    if ((status = env->Object_CallMethod_Boolean_A(etsAbilityStageObj_->aniObj, method, &res, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "CallMethod status: %{public}d", status);
        return false;
    }
    isAsync = res;
    return isAsync;
}

bool ETSAbilityStage::BindNativeMethods()
{
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return false;
    }
    std::array functions = {
        ani_native_function{
            "nativeOnAcceptWantCallback", "C{std.core.String}:",
            reinterpret_cast<void *>(ETSAbilityStage::OnAcceptWantCallback)},
        ani_native_function{
            "nativeOnNewProcessRequestCallback", "C{std.core.String}:",
            reinterpret_cast<void *>(ETSAbilityStage::OnNewProcessRequestCallback)},
        ani_native_function{"nativeOnPrepareTerminatePromiseCallback",
            PREPARE_TERMINATION_PROMISE_CALLBACK_METHOD_NAME,
            reinterpret_cast<void *>(OnPrepareTerminatePromiseCallback)},
    };
    ani_class cls {};
    ani_status status = env->FindClass(ABILITY_STAGE_CLASS_NAME, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindClass failed status: %{public}d", status);
        return false;
    }
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Class_BindNativeMethods status: %{public}d", status);
        return false;
    }
    return true;
}

void ETSAbilityStage::SetShellContextRef(std::shared_ptr<Context> context)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "context nullptr");
        return;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "env nullptr");
        return;
    }

    ani_object contextObj = ETSAbilityStageContext::CreateEtsAbilityStageContext(env, context);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "CreateEtsAbilityStageContext failed");
        return;
    }
    ani_ref contextGlobalRef = nullptr;
    ani_status status = ANI_OK;
    if ((status = env->GlobalReference_Create(contextObj, &contextGlobalRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "GlobalReference_Create failed status: %{public}d", status);
        return;
    }
    shellContextRef_ = std::make_shared<AppExecFwk::ETSNativeReference>();
    shellContextRef_->aniObj = reinterpret_cast<ani_object>(contextGlobalRef);
    shellContextRef_->aniRef = contextGlobalRef;
}

void ETSAbilityStage::SetEtsAbilityStage()
{
    if (!etsAbilityStageObj_) {
        TAG_LOGE(AAFwkTag::APPKIT, "etsAbilityStageObj_ null");
        ClearAppPreload();
        return;
    }
    if (shellContextRef_ == nullptr || shellContextRef_->aniRef == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "shellContextRef_ or shellContextRef_->aniRef null");
        return;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env nullptr");
        return;
    }

    ani_status status = ANI_OK;
    if (env->Object_SetFieldByName_Ref(etsAbilityStageObj_->aniObj, "context", shellContextRef_->aniRef) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_SetFieldByName_Ref context failed");
    }
}

int32_t ETSAbilityStage::RunAutoStartupTask(const std::function<void()> &callback, std::shared_ptr<AAFwk::Want> want,
    bool &isAsyncCallback, const std::shared_ptr<Context> &stageContext, bool preAbilityStageLoad)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPKIT, "RunAutoStartupTask, pre:%{public}d", preAbilityStageLoad);
    isAsyncCallback = false;
    auto context = GetContext();
    if (!context) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return ERR_INVALID_VALUE;
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (!hapModuleInfo) {
        TAG_LOGE(AAFwkTag::APPKIT, "null hapModuleInfo");
        return ERR_INVALID_VALUE;
    }
    if (hapModuleInfo->moduleType != AppExecFwk::ModuleType::ENTRY &&
        hapModuleInfo->moduleType != AppExecFwk::ModuleType::FEATURE) {
        TAG_LOGD(AAFwkTag::APPKIT, "not entry module or feature module");
        return ERR_INVALID_VALUE;
    }
    if (hapModuleInfo->appStartup.empty()) {
        TAG_LOGD(AAFwkTag::APPKIT, "module no app startup config");
        return ERR_INVALID_VALUE;
    }
    if (!shellContextRef_) {
        SetShellContextRef(stageContext);
    }
    int32_t result = RegisterAppStartupTask(hapModuleInfo, want);
    if (result != ERR_OK) {
        return result;
    }
    return RunAutoStartupTaskInner(callback, want, isAsyncCallback, hapModuleInfo->name, preAbilityStageLoad);
}

int32_t ETSAbilityStage::RegisterAppStartupTask(std::shared_ptr<AppExecFwk::HapModuleInfo> hapModuleInfo,
    std::shared_ptr<AAFwk::Want> want)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (isStartupTaskRegistered_) {
        TAG_LOGD(AAFwkTag::APPKIT, "app startup task already registered");
        return ERR_OK;
    }
    isStartupTaskRegistered_ = true;
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    if (startupManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null startupManager");
        return ERR_INVALID_VALUE;
    }
    bool needRunAutoStartupTask = false;
    int32_t result = startupManager->LoadAppStartupTaskConfig(needRunAutoStartupTask);
    if (result != ERR_OK) {
        return result;
    }
    result = startupManager->RunLoadModuleStartupConfigTask(needRunAutoStartupTask, hapModuleInfo);
    if (result != ERR_OK) {
        return result;
    }
    if (!needRunAutoStartupTask) {
        return ERR_OK;
    }
    if (hapModuleInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null hapModuleInfo");
        return ERR_INVALID_VALUE;
    }
    auto configEntry = startupManager->GetPendingConfigEntry();
    if (!LoadEtsStartupConfig(configEntry, want, hapModuleInfo->moduleName, hapModuleInfo->moduleType)) {
        TAG_LOGE(AAFwkTag::APPKIT, "load ets appStartup config failed.");
        return ERR_INVALID_VALUE;
    }
    return RegisterEtsStartupTask(hapModuleInfo);
}

bool ETSAbilityStage::LoadEtsStartupConfig(const std::pair<std::string, std::string> &configEntry,
    std::shared_ptr<AAFwk::Want> want, const std::string &moduleName, AppExecFwk::ModuleType moduleType)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto application = application_.lock();
    if (application == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null application");
        return false;
    }
    auto arkTSMode = configEntry.second;
    auto startupConfig = StartupConfigInstance::CreateStartupConfig(application->GetRuntime(), arkTSMode);
    if (startupConfig == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null startupConfig");
        return false;
    }
    auto &runtime = StartupTaskInstance::GetSpecifiedRuntime(application->GetRuntime(), arkTSMode);
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null runtime");
        return false;
    }
    auto srcEntry = configEntry.first;
    if (startupConfig->Init(*runtime, GetContext(), srcEntry, want) != ERR_OK) {
        return false;
    }
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    if (startupManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null startupManager");
        return false;
    }
    startupManager->SetModuleConfig(startupConfig, moduleName, moduleType == AppExecFwk::ModuleType::ENTRY);
    return true;
}

int32_t ETSAbilityStage::RegisterEtsStartupTask(std::shared_ptr<AppExecFwk::HapModuleInfo> hapModuleInfo)
{
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    if (startupManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null startupManager");
        return ERR_INVALID_VALUE;
    }
    if (hapModuleInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null hapModuleInfo");
        return ERR_INVALID_VALUE;
    }
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null shellContextRef_");
        return ERR_INVALID_VALUE;
    }
    auto application = application_.lock();
    if (application == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null application");
        return ERR_INVALID_VALUE;
    }
    const std::vector<StartupTaskInfo> startupTaskInfos = startupManager->GetStartupTaskInfos(hapModuleInfo->name);
    for (const auto& item : startupTaskInfos) {
        auto startupTask = StartupTaskInstance::CreateStartupTask(application->GetRuntime(), item.arkTSMode, item,
            startupManager->EnableLazyLoadingAppStartupTasks());
        if (startupTask == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "crate null startupTask");
            return ERR_INVALID_VALUE;
        }
        StartupTaskUtils::UpdateStartupTaskContextRef(GetNapiEnv(), startupTask, GetContext(),
            shellContextRef_->aniRef);
        startupTask->SetDependencies(item.dependencies);
        startupTask->SetIsExcludeFromAutoStart(item.excludeFromAutoStart);
        startupTask->SetCallCreateOnMainThread(item.callCreateOnMainThread);
        startupTask->SetWaitOnMainThread(item.waitOnMainThread);
        startupTask->SetModuleName(item.moduleName);
        startupTask->SetModuleType(item.moduleType);
        startupTask->SetMatchRules(std::move(item.matchRules));
        startupTask->SetPreAbilityStageLoad(item.preAbilityStageLoad);
        startupManager->RegisterAppStartupTask(item.name, startupTask);
    }
    return ERR_OK;
}

int32_t ETSAbilityStage::RunAutoStartupTaskInner(const std::function<void()> &callback,
    std::shared_ptr<AAFwk::Want> want, bool &isAsyncCallback, const std::string &moduleName, bool preAbilityStageLoad)
{
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    if (startupManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null startupManager");
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<StartupTaskManager> startupTaskManager = nullptr;
    int32_t result = startupManager->BuildAutoAppStartupTaskManager(want, startupTaskManager, moduleName,
        preAbilityStageLoad);
    if (result != ERR_OK) {
        return result;
    }
    if (startupTaskManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null startupTaskManager");
        return ERR_INVALID_VALUE;
    }
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null shellContextRef_");
        return ERR_INVALID_VALUE;
    }
    auto tasks = startupTaskManager->GetStartupTasks();
    UpdateStartupTasks(tasks);
    result = startupTaskManager->Prepare();
    if (result != ERR_OK) {
        return result;
    }
    auto runAutoStartupCallback = std::make_shared<OnCompletedCallback>(
        [callback](const std::shared_ptr<StartupTaskResult> &) {
            TAG_LOGI(AAFwkTag::APPKIT, "mainThreadAwaitCallback");
            callback();
        });
    const auto timeoutCallback = [moduleName]() {
        auto startupManager = DelayedSingleton<StartupManager>::GetInstance();
        if (startupManager == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null startupManager");
            return;
        }
        AAFwk::EventInfo eventInfo;
        eventInfo.errCode = ERR_STARTUP_TIMEOUT;
        eventInfo.errMsg = "Auto task timeout.";
        eventInfo.bundleName = startupManager->GetBundleName();
        eventInfo.appIndex = startupManager->GetAppIndex();
        eventInfo.moduleName = moduleName;
        eventInfo.userId = startupManager->GetUid() / AppExecFwk::Constants::BASE_USER_RANGE;
        AAFwk::EventReport::SendAppStartupErrorEvent(
            AAFwk::EventName::APP_STARTUP_ERROR, HISYSEVENT_FAULT, eventInfo);
    };
    startupTaskManager->SetTimeoutCallback(timeoutCallback);
    result = startupTaskManager->Run(runAutoStartupCallback);
    if (result != ERR_OK) {
        isAsyncCallback = runAutoStartupCallback->IsCalled();
        return result;
    }
    isAsyncCallback = true;
    return ERR_OK;
}

void ETSAbilityStage::UpdateStartupTasks(std::map<std::string, std::shared_ptr<StartupTask>> &tasks)
{
    for (auto &iter : tasks) {
        if (iter.second == nullptr) {
            continue;
        }
        if (iter.second->GetType() != AppStartupTask::TASK_TYPE_JS &&
            iter.second->GetType() != AppStartupTask::TASK_TYPE_ETS) {
            continue;
        }
        std::shared_ptr<AppStartupTask> appStartupTask = std::static_pointer_cast<AppStartupTask>(iter.second);
        if (appStartupTask->GetModuleType() != AppExecFwk::ModuleType::SHARED) {
            continue;
        }
        if (shellContextRef_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "shellContextRef_ null");
            continue;
        }
        StartupTaskUtils::UpdateStartupTaskContextRef(GetNapiEnv(), appStartupTask, GetContext(),
            shellContextRef_->aniRef);
    }
}

napi_env ETSAbilityStage::GetNapiEnv()
{
    auto &jsRuntimePtr = etsRuntime_.GetJsRuntime();
    if (jsRuntimePtr == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "jsRuntime is null");
        return nullptr;
    }
    auto &jsRuntime = static_cast<JsRuntime &>(*jsRuntimePtr);
    return jsRuntime.GetNapiEnv();
}

bool ETSAbilityStage::OnPrepareTerminate(
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult> *callbackInfo,
    bool &isAsync) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "OnPrepareTerminate called");
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "callbackInfo nullptr");
        return false;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return false;
    }
    if (etsAbilityStageObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null etsAbilityStageObj_");
        return false;
    }
    ani_long prepareTerminationCallbackPoint = reinterpret_cast<ani_long>(callbackInfo);
    ani_status status = env->Object_SetFieldByName_Long(etsAbilityStageObj_->aniObj, "prepareTerminationCallbackPoint",
        prepareTerminationCallbackPoint);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_SetFieldByName_Long status: %{public}d", status);
        return false;
    }
    if (CallOnPrepareTerminateAsync(callbackInfo, isAsync)) {
        TAG_LOGI(AAFwkTag::APPKIT, "onPrepareTerminationAsync is implemented");
        return true;
    }
    isAsync = false;
    return CallOnPrepareTerminate(callbackInfo);
}

bool ETSAbilityStage::CallOnPrepareTerminateAsync(
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult> *callbackInfo,
    bool &isAsync) const
{
    isAsync = false;
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "callbackInfo nullptr");
        return false;
    }
    isAsync = CallObjectMethod(true, "callOnPrepareTermination", ":z");
    return isAsync;
}

bool ETSAbilityStage::CallOnPrepareTerminate(
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult> *callbackInfo) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "CallOnPrepareTerminate call");
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "callbackInfo nullptr");
        return false;
    }
    ani_env *env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return false;
    }
    ani_object resObj = CallObjectMethod("onPrepareTermination", PREPARE_TERMINATION_CLASS_NAME);
    if (resObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "onPrepareTermination unimplemented");
        return false;
    }
    ani_int result = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, resObj, result)) {
        TAG_LOGE(AAFwkTag::APPKIT, "EnumConvert_EtsToNative param err");
        return false;
    }
    AppExecFwk::OnPrepareTerminationResult prepareTerminationResult = { result, true };
    callbackInfo->Call(prepareTerminationResult);
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult>::Destroy(callbackInfo);
    return true;
}
}  // namespace AbilityRuntime
}  // namespace OHOS

ETS_EXPORT extern "C" OHOS::AbilityRuntime::AbilityStage *OHOS_ETS_Ability_Stage_Create(
    const std::unique_ptr<OHOS::AbilityRuntime::Runtime> &runtime,
    const OHOS::AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    return OHOS::AbilityRuntime::ETSAbilityStage::Create(runtime, hapModuleInfo);
}