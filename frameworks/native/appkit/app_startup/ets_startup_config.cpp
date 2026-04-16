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
#include "ets_startup_config.h"

#include "ani_common_util.h"
#include "ani_common_want.h"
#include "ets_error_utils.h"
#include "ets_runtime.h"
#include "hilog_tag_wrapper.h"

#ifdef WINDOWS_PLATFORM
#define ETS_EXPORT __declspec(dllexport)
#else
#define ETS_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *STARTUP_CONFIG_CLASS_NAME = "@ohos.app.appstartup.StartupConfig.StartupConfigImpl";
constexpr const char *STARTUP_CONFIG_ENTRY_CLASS_NAME = "@ohos.app.appstartup.StartupConfigEntry.StartupConfigEntry";
constexpr const char *STARTUP_CONFIG_ENTRY_SIGNATURE_ON_REQUEST_CUSTOM_MATCH_RULE =
    "C{@ohos.app.ability.Want.Want}:C{std.core.String}";
constexpr const char *STARTUP_LISTEN_SIGNATURE_ON_COMPLETED = "C{@ohos.base.BusinessError}:";
constexpr int32_t DEFAULT_AWAIT_TIMEOUT_MS = 10000;
constexpr int32_t ARGC_ONE = 1;
}
ETSStartupConfig::ETSStartupConfig(ani_vm *etsVm) : StartupConfig(), etsVm_(etsVm)
{}

ETSStartupConfig::~ETSStartupConfig() = default;

int32_t ETSStartupConfig::Init(Runtime &runtime, std::shared_ptr<Context> context, const std::string &srcEntry,
    std::shared_ptr<AAFwk::Want> want)
{
    auto &etsRuntime = static_cast<ETSRuntime&>(runtime);
    auto configEntryRef = LoadSrcEntry(etsRuntime, context, srcEntry);
    if (configEntryRef == nullptr || configEntryRef->aniRef == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null configEntry or aniRef");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    ani_ref configEntry = configEntryRef->aniRef;
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null etsVm_");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "GetEnv failed");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    ani_status status = ANI_ERROR;
    ani_ref config = nullptr;
    if ((status = env->Object_CallMethodByName_Ref(reinterpret_cast<ani_object>(configEntry), "onConfig",
        ":C{@ohos.app.appstartup.StartupConfig.StartupConfig}", &config)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "Object_CallMethodByName_Ref onConfig failed");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    if (config == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null config");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    InitAwaitTimeout(env, reinterpret_cast<ani_object>(config));
    InitListener(env, reinterpret_cast<ani_object>(config));
    InitCustomization(env, reinterpret_cast<ani_object>(configEntry), want);
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
    return ERR_OK;
}

int32_t ETSStartupConfig::Init(ani_object config)
{
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null etsVm_");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "GetEnv failed");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    if (config == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null config");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    ani_boolean isUndefined = ANI_TRUE;
    env->Reference_IsUndefined(reinterpret_cast<ani_ref>(config), &isUndefined);
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::STARTUP, "undefined config");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    InitAwaitTimeout(env, config);
    InitListener(env, config);
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
    return ERR_OK;
}

std::unique_ptr<AppExecFwk::ETSNativeReference> ETSStartupConfig::LoadSrcEntry(ETSRuntime &etsRuntime,
    std::shared_ptr<Context> context, const std::string &srcEntry)
{
    TAG_LOGD(AAFwkTag::STARTUP, "LoadSrcEntry call, srcEntry: %{private}s", srcEntry.c_str());
    if (srcEntry.empty()) {
        TAG_LOGE(AAFwkTag::STARTUP, "srcEntry invalid");
        return nullptr;
    }
    if (!context) {
        TAG_LOGE(AAFwkTag::STARTUP, "null context");
        return nullptr;
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (!hapModuleInfo) {
        TAG_LOGE(AAFwkTag::STARTUP, "null hapModuleInfo");
        return nullptr;
    }

    std::string moduleNameWithStartupConfig(hapModuleInfo->moduleName + "::startupConfig");
    std::string srcPath(srcEntry);
    auto pos = srcPath.rfind('.');
    if (pos != std::string::npos) {
        srcPath.erase(pos);
        srcPath.append(".abc");
    }
    bool esModule = hapModuleInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE;
    return etsRuntime.LoadModule(moduleNameWithStartupConfig, srcPath, hapModuleInfo->hapPath, esModule, false,
        srcEntry);
}

bool ETSStartupConfig::GetTimeoutMs(ani_env *env, ani_object config, int32_t &timeoutMs)
{
    ani_class cls = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null env");
        return false;
    }
    if (config == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null config");
        return false;
    }
    ani_status status = env->FindClass("@ohos.app.appstartup.StartupConfig.StartupConfig", &cls);
    if (status != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "Findclass failed, status: %{public}d", status);
        return false;
    }
    ani_method method = nullptr;
    status = env->Class_FindMethod(cls, "<get>timeoutMs", nullptr, &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "Class_FindMethod failed, status: %{public}d", status);
        return false;
    }
    ani_ref iTimeoutMsRef = 0;
    status = env->Object_CallMethod_Ref(config, method, &iTimeoutMsRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "Object_CallMethod failed, status: %{public}d", status);
        return false;
    }

    ani_boolean isUndefined = ANI_TRUE;
    if ((status = env->Reference_IsUndefined(iTimeoutMsRef, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "Reference_IsUndefined failed, status: %{public}d", status);
        return false;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::STARTUP, "undefined");
        return false;
    }
    ani_int aniInt = 0;
    if ((status = env->Object_CallMethodByName_Int(
        reinterpret_cast<ani_object>(iTimeoutMsRef), "toInt", nullptr, &aniInt)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "Object_CallMethodByName failed, status: %{public}d", status);
        return false;
    }
    timeoutMs = static_cast<int32_t>(aniInt);
    return true;
}

void ETSStartupConfig::InitAwaitTimeout(ani_env *env, ani_object config)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null env");
        return;
    }
    if (config == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null config");
        return;
    }

    int32_t awaitTimeoutNum = DEFAULT_AWAIT_TIMEOUT_MS;
    if (!GetTimeoutMs(env, config, awaitTimeoutNum)) {
        TAG_LOGI(AAFwkTag::STARTUP, "no timeoutMs or failed");
    }

    if (awaitTimeoutNum <= 0) {
        TAG_LOGE(AAFwkTag::STARTUP, "invalid awaitTimeoutNum");
        awaitTimeoutNum = DEFAULT_AWAIT_TIMEOUT_MS;
    }
    TAG_LOGD(AAFwkTag::STARTUP, "set awaitTimeoutMs to %{public}d", awaitTimeoutNum);
    awaitTimeoutMs_ = awaitTimeoutNum;
}

void ETSStartupConfig::InitListener(ani_env *env, ani_object config)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null env");
        return;
    }
    if (config == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null config");
        return;
    }
    ani_ref listener = nullptr;
    if (!AppExecFwk::GetRefProperty(env, config, "startupListener", listener)) {
        TAG_LOGE(AAFwkTag::STARTUP, "startupListener GetPropertyRef failed");
        return;
    }

    ani_ref gl = nullptr;
    // todo delete gl
    env->GlobalReference_Create(reinterpret_cast<ani_object>(listener), &gl);
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "get vm failed");
        return;
    }
    OnCompletedCallbackFunc onCompletedCallback =
        [etsVm = aniVM, listener = gl](const std::shared_ptr<StartupTaskResult> &result) {
        if (etsVm == nullptr || listener == nullptr || result == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "null vm or listener or result");
            return;
        }
        bool isAttachThread = false;
        ani_env *env = AppExecFwk::AttachAniEnv(etsVm, isAttachThread);
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "env null");
            return;
        }
        ani_object resultValue = ETSStartupConfig::BuildResult(env, result);
        if (resultValue == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "resultValue null");
            return;
        }
        ani_status status = ANI_ERROR;
        ani_ref funRef = nullptr;
        if ((status = env->Object_GetPropertyByName_Ref(reinterpret_cast<ani_object>(listener),
            "onCompleted", &funRef)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::STARTUP, "Object_GetPropertyByName_Ref failed, status: %{public}d", status);
        }
        if (!AppExecFwk::IsValidProperty(env, funRef)) {
            TAG_LOGE(AAFwkTag::STARTUP, "funRef invalid");
        }
        std::vector<ani_ref> argv = { resultValue };
        ani_ref voidReturn = nullptr;
        if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_ONE, argv.data(),
            &voidReturn)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::STARTUP, "FunctionalObject_Call failed, status: %{public}d", status);
        }
        AppExecFwk::DetachAniEnv(etsVm, isAttachThread);
    };
    listener_ = std::make_shared<StartupListener>(onCompletedCallback);
}

void ETSStartupConfig::InitCustomization(ani_env *env, ani_object configEntry, std::shared_ptr<AAFwk::Want> want)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null env");
        return;
    }
    if (configEntry == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null config");
        return;
    }
    if (!want) {
        TAG_LOGD(AAFwkTag::STARTUP, "want is null");
        return;
    }
    ani_object wantAniObj = nullptr;
    wantAniObj = AppExecFwk::WrapWant(env, *want);

    ani_status status = ANI_ERROR;
    ani_ref callResult = nullptr;
    if ((status = env->Object_CallMethodByName_Ref(reinterpret_cast<ani_object>(configEntry),
        "onRequestCustomMatchRule", STARTUP_CONFIG_ENTRY_SIGNATURE_ON_REQUEST_CUSTOM_MATCH_RULE,
        &callResult, wantAniObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "Object_CallMethodByName_Ref onRequestCustomMatchRule failed");
        return;
    }
    if (callResult == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null config");
        return;
    }

    std::string customization = "";
    if (!AppExecFwk::GetStdString(env, reinterpret_cast<ani_string>(callResult), customization)) {
        TAG_LOGE(AAFwkTag::STARTUP, "GetStdString failed");
        return;
    }
    customization_ = customization;
}

ani_object ETSStartupConfig::BuildResult(ani_env *env, const std::shared_ptr<StartupTaskResult> &result)
{
    if (result == nullptr) {
        TAG_LOGI(AAFwkTag::STARTUP, "result null");
        return EtsErrorUtil::CreateError(env, ERR_STARTUP_INTERNAL_ERROR,
            StartupUtils::GetErrorMessage(ERR_STARTUP_INTERNAL_ERROR));
    }
    ani_ref undefRef = nullptr;
    if (result->GetResultCode() != ERR_OK) {
        return EtsErrorUtil::CreateError(env, result->GetResultCode(), result->GetResultMessage());
    }
    return EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK);
}

extern "C" ETS_EXPORT StartupConfig* OHOS_CreateEtsStartupConfig(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null env");
        return nullptr;
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "get vm failed");
    }
    return new (std::nothrow) ETSStartupConfig(aniVM);
}
}
}
