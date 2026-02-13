/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "js_ability_stage.h"

#include "ability_delegator_registry.h"
#include "event_report.h"
#include "freeze_util.h"
#include "hilog_tag_wrapper.h"
#include "js_ability_stage_context.h"
#include "js_context_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_startup_config.h"
#include "napi_common_configuration.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "ohos_application.h"
#include "startup_config_instance.h"
#include "startup_manager.h"
#include "startup_task_instance.h"
#include "hitrace_meter.h"
#include "application_env.h"
#include <algorithm>
#include <cstring>
#include <exception>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

namespace OHOS {
namespace AbilityRuntime {
constexpr const char* PROFILE_FILE_PREFIX = "$profile:";
constexpr const char* STARTUP_TASKS = "startupTasks";
constexpr const char* NAME = "name";
constexpr const char* SRC_ENTRY = "srcEntry";
constexpr const char* DEPENDENCIES = "dependencies";
constexpr const char* EXCLUDE_FROM_AUTO_START = "excludeFromAutoStart";
constexpr const char* RUN_ON_THREAD = "runOnThread";
constexpr const char* WAIT_ON_MAIN_THREAD = "waitOnMainThread";
constexpr const char* CONFIG_ENTRY = "configEntry";
constexpr const char *TASKPOOL = "taskPool";
constexpr const char *TASKPOOL_LOWER = "taskpool";
constexpr const char *CALLBACK_SUCCESS = "success";
constexpr const int32_t ARGC_ONE = 1;
namespace {
void *DetachNewBaseContext(napi_env, void *nativeObject, void *)
{
    auto *origContext = static_cast<std::weak_ptr<Context> *>(nativeObject);
    if (origContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "origContext is null");
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "New detached base context");
    auto *detachNewContext = new (std::nothrow) std::weak_ptr<Context>(*origContext);
    return detachNewContext;
}

void DetachFinalizeBaseContext(void *detachedObject, void *)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Finalizer detached base context");
    delete static_cast<std::weak_ptr<Context> *>(detachedObject);
}

void RegisterStopPreloadSoCallback(JsRuntime& jsRuntime)
{
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    if (startupManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null startupManager");
        return;
    }
    if (!startupManager->HasAppStartupConfig()) {
        // no app startup config, no need to register stop preload so callback.
        return;
    }
    jsRuntime.SetStopPreloadSoCallback([]()-> void {
        std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
        if (startupManager == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null startupManager");
            return;
        }
        startupManager->StopAutoPreloadSoTask();
    });
}

napi_value OnPrepareTerminatePromiseCallback(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::APPKIT, "OnPrepareTerminatePromiseCallback begin");
    void *data = nullptr;
    size_t argc = ARGC_MAX_COUNT;
    napi_value argv[ARGC_MAX_COUNT] = {nullptr};
    NAPI_CALL_NO_THROW(napi_get_cb_info(env, info, &argc, argv, nullptr, &data), nullptr);
    auto *callbackInfo =
        static_cast<AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult> *>(data);
    int prepareTermination = 0;
    if (callbackInfo == nullptr || (argc > 0 && !ConvertFromJsValue(env, argv[0], prepareTermination))) {
        TAG_LOGE(AAFwkTag::APPKIT, "null callbackInfo or unwrap prepareTermination result failed");
        return nullptr;
    }
    AppExecFwk::OnPrepareTerminationResult result = { prepareTermination, true };
    callbackInfo->Call(result);
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult>::Destroy(callbackInfo);
    data = nullptr;
    TAG_LOGI(AAFwkTag::APPKIT, "OnPrepareTerminatePromiseCallback end");
    return nullptr;
}

napi_value OnStringPromiseCallback(napi_env env, napi_callback_info info)
{
    void *data = nullptr;
    size_t argc = ARGC_MAX_COUNT;
    napi_value argv[ARGC_MAX_COUNT] = {nullptr};
    NAPI_CALL_NO_THROW(napi_get_cb_info(env, info, &argc, argv, nullptr, &data), nullptr);
    auto *callbackInfo =
        static_cast<AppExecFwk::AbilityTransactionCallbackInfo<std::string> *>(data);
    std::string flag;
    if (callbackInfo == nullptr || (argc > 0 && !ConvertFromJsValue(env, argv[0], flag))) {
        TAG_LOGE(AAFwkTag::APPKIT, "null callbackInfo or unwrap flag result failed");
        return nullptr;
    }
    callbackInfo->Call(flag);
    AppExecFwk::AbilityTransactionCallbackInfo<std::string>::Destroy(callbackInfo);
    data = nullptr;
    return nullptr;
}
} // namespace

bool JsAbilityStage::UseCommonChunk(const AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    for (auto &md: hapModuleInfo.metadata) {
        if (md.name == "USE_COMMON_CHUNK") {
            if (md.value != "true") {
                TAG_LOGW(AAFwkTag::APPKIT, "USE_COMMON_CHUNK = %s{public}s", md.value.c_str());
                return false;
            }
            return true;
        }
    }
    return false;
}

std::shared_ptr<AbilityStage> JsAbilityStage::Create(
    const std::unique_ptr<Runtime>& runtime, const AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    if (runtime == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "null runtime");
        return nullptr;
    }
    FreezeUtil::GetInstance().AddAppLifecycleEvent(0, "JsAbilityStage::Create");
    auto& jsRuntime = static_cast<JsRuntime&>(*runtime);
    RegisterStopPreloadSoCallback(jsRuntime);
    return std::make_shared<JsAbilityStage>(jsRuntime);
}

JsAbilityStage::JsAbilityStage(JsRuntime& jsRuntime) : jsRuntime_(jsRuntime)
{}

JsAbilityStage::~JsAbilityStage()
{
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }

    jsRuntime_.FreeNativeReference(std::move(jsAbilityStageObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
}

void JsAbilityStage::Init(const std::shared_ptr<Context> &context,
    const std::weak_ptr<AppExecFwk::OHOSApplication> application)
{
    AbilityStage::Init(context, application);
    SetShellContextRef(context);
}

void JsAbilityStage::LoadModule(const AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::APPKIT, "LoadModule");
    std::string srcPath(hapModuleInfo.name);
    std::string moduleName(hapModuleInfo.moduleName);
    moduleName.append("::").append("AbilityStage");
    bool commonChunkFlag = UseCommonChunk(hapModuleInfo);
    /* temporary compatibility api8 + config.json */
    if (!hapModuleInfo.isModuleJson) {
        srcPath.append("/assets/js/");
        if (hapModuleInfo.srcPath.empty()) {
            srcPath.append("AbilityStage.abc");
        } else {
            srcPath.append(hapModuleInfo.srcPath);
            srcPath.append("/AbilityStage.abc");
        }
        std::string key(moduleName);
        key.append("::");
        key.append(srcPath);
        std::unique_ptr<NativeReference> moduleObj = nullptr;
        if (jsRuntime_.PopPreloadObj(key, moduleObj)) {
            jsAbilityStageObj_ = std::move(moduleObj);
        } else {
            auto moduleObj = jsRuntime_.LoadModule(moduleName, srcPath, hapModuleInfo.hapPath,
                hapModuleInfo.compileMode == AppExecFwk::CompileMode::ES_MODULE, commonChunkFlag);
            jsAbilityStageObj_ = std::move(moduleObj);
        }
    } else if (!hapModuleInfo.srcEntrance.empty()) {
        srcPath.append("/");
        srcPath.append(hapModuleInfo.srcEntrance);
        srcPath.erase(srcPath.rfind("."));
        srcPath.append(".abc");
        std::unique_ptr<NativeReference> moduleObj = jsRuntime_.LoadModule(moduleName, srcPath, hapModuleInfo.hapPath,
            hapModuleInfo.compileMode == AppExecFwk::CompileMode::ES_MODULE, commonChunkFlag);
        TAG_LOGD(AAFwkTag::APPKIT, "srcPath is %{public}s", srcPath.c_str());
        jsAbilityStageObj_ = std::move(moduleObj);
    }
    SetJsAbilityStage();
}

void JsAbilityStage::OnCreate(const AAFwk::Want &want) const
{
    AbilityStage::OnCreate(want);

    if (!jsAbilityStageObj_) {
        TAG_LOGW(AAFwkTag::APPKIT, "Not found AbilityStage.js");
        ClearAppPreload();
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "get object failed");
        return;
    }

    napi_value methodOnCreate = nullptr;
    napi_get_named_property(env, obj, "onCreate", &methodOnCreate);
    if (methodOnCreate == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null methodOnCreate");
        return;
    }
    FreezeUtil::GetInstance().AddAppLifecycleEvent(0, "JsAbilityStage::OnCreate begin");
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    napi_call_function(env, obj, methodOnCreate, 0, nullptr, nullptr);
    FreezeUtil::GetInstance().AddAppLifecycleEvent(0, "JsAbilityStage::OnCreate end");
    ClearAppPreload();

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        delegator->PostPerformStageStart(CreateStageProperty());
    }
}

void JsAbilityStage::OnDestroy() const
{
    TAG_LOGD(AAFwkTag::APPKIT, "Called");
    AbilityStage::OnDestroy();

    if (!jsAbilityStageObj_) {
        TAG_LOGW(AAFwkTag::APPKIT, "Not found AbilityStage.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "get object failed");
        return;
    }

    napi_value methodOnDestroy = nullptr;
    napi_get_named_property(env, obj, "onDestroy", &methodOnDestroy);
    if (methodOnDestroy == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null methodOnDestroy");
        return;
    }
    napi_call_function(env, obj, methodOnDestroy, 0, nullptr, nullptr);
}

bool JsAbilityStage::OnPrepareTerminate(
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult> *callbackInfo,
    bool &isAsync) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    AbilityStage::OnPrepareTerminate(callbackInfo, isAsync);

    if (!jsAbilityStageObj_) {
        TAG_LOGW(AAFwkTag::APPKIT, "Not found AbilityStage.js");
        return false;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Fail to get AbilityStage object");
        return false;
    }

    if (CallOnPrepareTerminateAsync(env, callbackInfo, isAsync)) {
        TAG_LOGI(AAFwkTag::APPKIT, "onPrepareTerminationAsync is implemented");
        return true;
    }
    isAsync = false;
    return CallOnPrepareTerminate(env, callbackInfo);
}

bool JsAbilityStage::CallOnPrepareTerminate(napi_env env,
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult> *callbackInfo) const
{
    TAG_LOGI(AAFwkTag::APPKIT, "sync call");
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "callbackInfo nullptr");
        return false;
    }
    bool hasCaughtException = false;
    napi_value result = CallObjectMethod("onPrepareTermination", hasCaughtException, nullptr, 0);
    if (result == nullptr) {
        TAG_LOGI(AAFwkTag::APPKIT, "onPrepareTermination unimplemented");
        return false;
    }
    int32_t prepareTermination = 0;
    if (!ConvertFromJsValue(env, result, prepareTermination)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Fail to unwrap prepareTermination result");
    }
    AppExecFwk::OnPrepareTerminationResult prepareTerminationResult = { prepareTermination, true };
    callbackInfo->Call(prepareTerminationResult);
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult>::Destroy(callbackInfo);
    return true;
}

bool JsAbilityStage::CallOnPrepareTerminateAsync(napi_env env,
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult> *callbackInfo,
    bool &isAsync) const
{
    isAsync = false;
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "callbackInfo nullptr");
        return false;
    }
    bool hasCaughtException = false;
    napi_value result = CallObjectMethod("onPrepareTerminationAsync", hasCaughtException, nullptr, 0);
    if (result == nullptr || !CheckTypeForNapiValue(env, result, napi_object)) {
        TAG_LOGI(AAFwkTag::APPKIT, "onPrepareTerminationAsync unimplemented");
        return false;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "onPrepareTerminationAsync implemented");
    bool isPromise = false;
    napi_is_promise(env, result, &isPromise);
    if (!isPromise) {
        TAG_LOGI(AAFwkTag::APPKIT, "result not promise");
        // the async func is implemented but the user's returned value is wrong
        return true;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "async call");
    bool callResult = false;
    do {
        napi_value then = nullptr;
        napi_get_named_property(env, result, "then", &then);
        if (then == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null then");
            break;
        }
        bool isCallable = false;
        napi_is_callable(env, then, &isCallable);
        if (!isCallable) {
            TAG_LOGE(AAFwkTag::APPKIT, "not callable property then");
            break;
        }
        napi_value promiseCallback = nullptr;
        napi_create_function(env, "promiseCallback", strlen("promiseCallback"),
            OnPrepareTerminatePromiseCallback, callbackInfo, &promiseCallback);
        napi_value argv[1] = { promiseCallback };
        napi_call_function(env, result, then, 1, argv, nullptr);
        callResult = true;
    } while (false);
    if (!callResult) {
        TAG_LOGE(AAFwkTag::APPKIT, "call promise error");
        return true;
    }
    isAsync = true;
    return true;
}

std::string JsAbilityStage::OnAcceptWant(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<std::string> *callbackInfo, bool &isAsync)
{
    TAG_LOGD(AAFwkTag::APPKIT, "OnAcceptWant called");
    AbilityStage::OnAcceptWant(want, callbackInfo, isAsync);

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    if (!jsAbilityStageObj_) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not found AbilityStage.js");
        return "";
    }
    std::string methodName = "onAcceptWantAsync";
    if (CallAcceptOrRequestAsync(env, want, methodName, callbackInfo, isAsync)) {
        TAG_LOGD(AAFwkTag::APPKIT, "onAcceptWantAsync is implemented");
        return CALLBACK_SUCCESS;
    }
    methodName = "onAcceptWant";
    isAsync = false;
    if (CallAcceptOrRequestSync(env, want, methodName, callbackInfo)) {
        return CALLBACK_SUCCESS;
    }
    return "";
}

bool JsAbilityStage::CallAcceptOrRequestSync(napi_env env, const AAFwk::Want &want, std::string &methodName,
    AppExecFwk::AbilityTransactionCallbackInfo<std::string> *callbackInfo) const
{
    HandleScope handleScope(env);
    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "get object failed");
        return false;
    }
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value methodOnAcceptWant = nullptr;
    napi_status status = napi_get_named_property(env, obj, methodName.c_str(), &methodOnAcceptWant);
    if (status != napi_ok || methodOnAcceptWant == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null %{public}s", methodName.c_str());
        return false;
    }
    napi_value argv[] = { napiWant };
    napi_value flagNative;
    status = napi_call_function(env, obj, methodOnAcceptWant, 1, argv, &flagNative);
    if (status != napi_ok || flagNative == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to call %{public}s", methodName.c_str());
        return false;
    }
    if (callbackInfo) {
        std::string resultString = AppExecFwk::UnwrapStringFromJS(env, flagNative);
        callbackInfo->Call(resultString);
        AppExecFwk::AbilityTransactionCallbackInfo<std::string>::Destroy(callbackInfo);
        return true;
    }
    return false;
}

bool JsAbilityStage::CallAcceptOrRequestAsync(napi_env env, const AAFwk::Want &want, std::string &methodName,
    AppExecFwk::AbilityTransactionCallbackInfo<std::string> *callbackInfo, bool &isAsync)  const
{
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "callbackInfo nullptr");
        return false;
    }
    HandleScope handleScope(env);
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    bool hasCaughtException = false;
    napi_value result = CallObjectMethod(methodName.c_str(), hasCaughtException, &napiWant, ARGC_ONE);
    if (hasCaughtException) {
        std::string result;
        callbackInfo->Call(result);
        AppExecFwk::AbilityTransactionCallbackInfo<std::string>::Destroy(callbackInfo);
        return true;
    }
    if (result == nullptr || !CheckTypeForNapiValue(env, result, napi_object)) {
        TAG_LOGI(AAFwkTag::APPKIT, "%{public}s unimplemented", methodName.c_str());
        return false;
    }
    bool isPromise = false;
    napi_is_promise(env, result, &isPromise);
    if (!isPromise) {
        TAG_LOGD(AAFwkTag::APPKIT, "result not promise");
        return true;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "async call");
    bool callResult = false;
    do {
        napi_value then = nullptr;
        napi_get_named_property(env, result, "then", &then);
        if (then == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null then");
            break;
        }
        bool isCallable = false;
        napi_is_callable(env, then, &isCallable);
        if (!isCallable) {
            TAG_LOGE(AAFwkTag::APPKIT, "not callable property then");
            break;
        }
        napi_value promiseCallback = nullptr;
        napi_create_function(env, "promiseCallback", strlen("promiseCallback"),
            OnStringPromiseCallback, callbackInfo, &promiseCallback);
        napi_value argv[1] = { promiseCallback };
        napi_call_function(env, result, then, 1, argv, nullptr);
        callResult = true;
    } while (false);

    if (!callResult) {
        TAG_LOGE(AAFwkTag::APPKIT, "call promise error");
        return true;
    }
    isAsync = true;
    return true;
}

std::string JsAbilityStage::OnNewProcessRequest(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<std::string> *callbackInfo, bool &isAsync)
{
    TAG_LOGD(AAFwkTag::APPKIT, "OnNewProcessRequest called");
    AbilityStage::OnNewProcessRequest(want, callbackInfo, isAsync);

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    if (!jsAbilityStageObj_) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not found AbilityStage.js");
        return "";
    }
    std::string methodName = "onNewProcessRequestAsync";
    if (CallAcceptOrRequestAsync(env, want, methodName, callbackInfo, isAsync)) {
        TAG_LOGD(AAFwkTag::APPKIT, "onNewProcessRequestAsync is implemented");
        return CALLBACK_SUCCESS;
    }
    methodName = "onNewProcessRequest";
    isAsync = false;
    if (CallAcceptOrRequestSync(env, want, methodName, callbackInfo)) {
        return CALLBACK_SUCCESS;
    }
    return "";
}

void JsAbilityStage::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    AbilityStage::OnConfigurationUpdated(configuration);

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    auto application = application_.lock();
    if (application == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null application");
        return;
    }
    // Notify Ability stage context
    auto fullConfig = application->GetConfiguration();
    if (!fullConfig) {
        TAG_LOGE(AAFwkTag::APPKIT, "null fullConfig");
        return;
    }
    JsAbilityStageContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    bool hasCaughtException = false;
    CallObjectMethod("onConfigurationUpdated", hasCaughtException, &napiConfiguration, ARGC_ONE);
    CallObjectMethod("onConfigurationUpdate", hasCaughtException, &napiConfiguration, ARGC_ONE);
}

void JsAbilityStage::OnMemoryLevel(int32_t level)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    AbilityStage::OnMemoryLevel(level);

    if (!jsAbilityStageObj_) {
        TAG_LOGW(AAFwkTag::APPKIT, "Not found stage");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "get object failed");
        return;
    }

    napi_value jsLevel = CreateJsValue(env, level);
    napi_value argv[] = { jsLevel };
    bool hasCaughtException = false;
    CallObjectMethod("onMemoryLevel", hasCaughtException, argv, ArraySize(argv));
    TAG_LOGD(AAFwkTag::APPKIT, "end");
}

int32_t JsAbilityStage::RunAutoStartupTask(const std::function<void()> &callback, std::shared_ptr<AAFwk::Want> want,
    bool &isAsyncCallback, const std::shared_ptr<Context> &stageContext, bool preAbilityStageLoad)
{
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

int32_t JsAbilityStage::RegisterAppStartupTask(const std::shared_ptr<AppExecFwk::HapModuleInfo>& hapModuleInfo,
    std::shared_ptr<AAFwk::Want> want)
{
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
    jsRuntime_.UpdateModuleNameAndAssetPath(hapModuleInfo->moduleName);

    auto configEntry = startupManager->GetPendingConfigEntry();
    if (!LoadJsStartupConfig(configEntry, want, hapModuleInfo->moduleName, hapModuleInfo->moduleType)) {
        TAG_LOGE(AAFwkTag::APPKIT, "load js appStartup config failed.");
        return ERR_INVALID_VALUE;
    }
    return RegisterJsStartupTask(hapModuleInfo);
}

int32_t JsAbilityStage::RegisterJsStartupTask(std::shared_ptr<AppExecFwk::HapModuleInfo> hapModuleInfo)
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
        startupTask->UpdateContextRef(shellContextRef_);
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

int32_t JsAbilityStage::RunAutoStartupTaskInner(const std::function<void()> &callback,
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
    startupTaskManager->UpdateStartupTaskContextRef(shellContextRef_);
    result = startupTaskManager->Prepare();
    if (result != ERR_OK) {
        return result;
    }
    auto runAutoStartupCallback = std::make_shared<OnCompletedCallback>(
        [callback](const std::shared_ptr<StartupTaskResult> &) {
            TAG_LOGI(AAFwkTag::APPKIT, "OnCompletedCallback");
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

bool JsAbilityStage::LoadJsStartupConfig(const std::pair<std::string, std::string> &configEntry,
    std::shared_ptr<AAFwk::Want> want, const std::string &moduleName, AppExecFwk::ModuleType moduleType)
{
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

napi_value JsAbilityStage::CallObjectMethod(
    const char* name, bool &hasCaughtException, napi_value const * argv, size_t argc) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "call %{public}s", name);
    if (!jsAbilityStageObj_) {
        TAG_LOGW(AAFwkTag::APPKIT, "Not found AbilityStage.js");
        return nullptr;
    }

    HandleEscape handleEscape(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "get object failed");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        TAG_LOGE(AAFwkTag::APPKIT, "get '%{public}s' object failed", name);
        return nullptr;
    }

    napi_value result = nullptr;
    TryCatch tryCatch(env);
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    napi_status withResultStatus = napi_call_function(env, obj, method, argc, argv, &result);
    if (withResultStatus != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "JsAbilityStage call js, withResult failed: %{public}d", withResultStatus);
    }
    if (tryCatch.HasCaught()) {
        TAG_LOGE(AAFwkTag::APPKIT, "%{public}s exception occurred", name);
        reinterpret_cast<NativeEngine*>(env)->HandleUncaughtException();
        hasCaughtException = true;
    }
    return handleEscape.Escape(result);
}

std::shared_ptr<AppExecFwk::DelegatorAbilityStageProperty> JsAbilityStage::CreateStageProperty() const
{
    auto property = std::make_shared<AppExecFwk::DelegatorAbilityStageProperty>();
    property->moduleName_ = GetHapModuleProp("name");
    property->srcEntrance_ = GetHapModuleProp("srcEntrance");
    property->object_ = jsAbilityStageObj_;
    return property;
}

std::string JsAbilityStage::GetHapModuleProp(const std::string &propName) const
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
    TAG_LOGE(AAFwkTag::APPKIT, "name = %{public}s", propName.c_str());
    return std::string();
}

void JsAbilityStage::SetShellContextRef(std::shared_ptr<Context> context)
{
    if (!context) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value contextObj = CreateJsAbilityStageContext(env, context);
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(env, "application.AbilityStageContext", &contextObj, 1);
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null shellContextRef_");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "get object failed");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::Context>(context);
    auto coerceStatus = napi_coerce_to_native_binding_object(
        env, contextObj, DetachNewBaseContext, AttachAbilityStageContext, workContext, nullptr);
    if (coerceStatus != napi_ok) {
        TAG_LOGW(AAFwkTag::APPKIT, "coerce ability stage context failed: %{public}d", coerceStatus);
        delete workContext;
        return;
    }
    napi_add_detached_finalizer(env, contextObj, DetachFinalizeBaseContext, nullptr);
    context->Bind(jsRuntime_, shellContextRef_.get());

    napi_status status = napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void*) {
            TAG_LOGD(AAFwkTag::APPKIT, "Finalizer for weak_ptr ability stage context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Context>*>(data);
        }, nullptr, nullptr);
    if (status != napi_ok && workContext != nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "napi_wrap Failed: %{public}d", status);
        delete workContext;
        return;
    }
}

void JsAbilityStage::SetJsAbilityStage()
{
    if (!jsAbilityStageObj_) {
        TAG_LOGW(AAFwkTag::APPKIT, "null stage");
        return;
    }
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null shellContextRef_");
        return;
    }
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "get object failed");
        return;
    }
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "obj is nullptr");
        return;
    }
    napi_value contextObj = shellContextRef_->GetNapiValue();
    napi_set_named_property(env, obj, "context", contextObj);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
