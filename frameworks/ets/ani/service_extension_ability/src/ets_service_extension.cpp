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

#include "ets_service_extension.h"

#include "ability_business_error.h"
#include "ability_handler.h"
#include "ability_info.h"
#include "ability_manager_client.h"
#include "ani_common_configuration.h"
#include "ani_common_want.h"
#include "remote_object_taihe_ani.h"
#include "configuration_utils.h"
#include "display_util.h"
#include "ets_extension_common.h"
#include "ets_service_extension_context.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "insight_intent_execute_param.h"
#include "insight_intent_execute_result.h"
#include "insight_intent_executor_info.h"
#include "insight_intent_executor_mgr.h"
#include "js_service_extension_context.h"
#ifdef SUPPORT_GRAPHICS
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "window_scene.h"
#endif

#ifdef WINDOWS_PLATFORM
#define ETS_EXPORT __declspec(dllexport)
#else
#define ETS_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
constexpr const int ANI_ALREADY_BINDED = 8;
constexpr const char *CLASSNAME_SERVICE_ABILITY = "@ohos.app.ability.ServiceExtensionAbility.ServiceExtensionAbility";
constexpr const char *SERVICE_EXTENSION_CONTEXT_CLASS_NAME =
    "application.ServiceExtensionContext.ServiceExtensionContext";
constexpr const char *NATIVE_ONCONNECT_CALLBACK_SIGNATURE = "C{@ohos.rpc.rpc.RemoteObject}:z";
constexpr const char *ON_CREATE_SIGNATURE = "C{@ohos.app.ability.Want.Want}:";
constexpr const char *VOID_SIGNATURE = ":";
constexpr const char *CHECK_PROMISE_SIGNATURE = "Y:z";
constexpr const char *CALL_PROMISE_SIGNATURE = "C{std.core.Promise}:z";
constexpr const char *ON_DISCONNECT_SIGNATURE = "C{@ohos.app.ability.Want.Want}:z";
constexpr const char *ON_REQUEST_SIGNATURE = "C{@ohos.app.ability.Want.Want}i:";
constexpr const char *ON_CONFIGURATION_UPDATE_SIGNATURE = "C{@ohos.app.ability.Configuration.Configuration}:";
constexpr const char *ON_DUMP_SIGNATURE = "C{std.core.Array}:C{std.core.Array}";

void DisconnectPromiseCallback(ani_env *env, ani_object aniObj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "DisconnectPromiseCallback");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    ani_long disconnectCallback = 0;
    ani_status status = ANI_ERROR;
    if ((status = env->Object_GetFieldByName_Long(aniObj, "disconnectCallback", &disconnectCallback)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return;
    }
    auto *callbackInfo = reinterpret_cast<AppExecFwk::AbilityTransactionCallbackInfo<> *>(disconnectCallback);
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null callbackInfo");
        return;
    }
    callbackInfo->Call();
    AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
}

bool ConnectPromiseCallback(ani_env *env, ani_object aniObj, ani_object obj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "ConnectPromiseCallback");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return false;
    }
    ani_long connectCallback = 0;
    ani_status status = ANI_ERROR;
    if ((status = env->Object_GetFieldByName_Long(aniObj, "connectCallback", &connectCallback)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return false;
    }
    auto remoteObject = AniGetNativeRemoteObject(env, obj);
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null remoteObject");
    }
    auto *callbackInfo =
        reinterpret_cast<AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *>(connectCallback);
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null callbackInfo");
        return false;
    }

    callbackInfo->Call(remoteObject);
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>>::Destroy(callbackInfo);
    return true;
}
} // namespace

EtsServiceExtension *EtsServiceExtension::Create(const std::unique_ptr<Runtime> &runtime)
{
    return new (std::nothrow) EtsServiceExtension(static_cast<ETSRuntime &>(*runtime));
}

EtsServiceExtension::EtsServiceExtension(ETSRuntime &etsRuntime) : etsRuntime_(etsRuntime) {}
EtsServiceExtension::~EtsServiceExtension()
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "EtsServiceExtension destory");
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    if (shellContextRef_ && shellContextRef_->aniRef) {
        env->GlobalReference_Delete(shellContextRef_->aniRef);
    }
}

void EtsServiceExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "EtsServiceExtension init");
    if ((token == nullptr) || (application == nullptr) || (handler == nullptr) || (record == nullptr)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "init failed, some obj null");
        return;
    }
    ServiceExtension::Init(record, application, handler, token);
    if (Extension::abilityInfo_ == nullptr || Extension::abilityInfo_->srcEntrance.empty()) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "EtsServiceExtension Init abilityInfo error");
        return;
    }
    std::string srcPath(Extension::abilityInfo_->moduleName + "/");
    srcPath.append(Extension::abilityInfo_->srcEntrance);
    auto pos = srcPath.rfind(".");
    if (pos != std::string::npos) {
        srcPath.erase(pos);
        srcPath.append(".abc");
    }
    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    etsObj_ = etsRuntime_.LoadModule(moduleName, srcPath, abilityInfo_->hapPath,
        abilityInfo_->compileMode == AppExecFwk::CompileMode::ES_MODULE, false, abilityInfo_->srcEntrance);
    if (etsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsObj");
        return;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    std::array functions = {
        ani_native_function {
            "nativeOnDisconnectCallback", VOID_SIGNATURE, reinterpret_cast<void *>(DisconnectPromiseCallback) },
        ani_native_function { "nativeOnConnectCallback", NATIVE_ONCONNECT_CALLBACK_SIGNATURE,
            reinterpret_cast<void *>(ConnectPromiseCallback) },
    };
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass(CLASSNAME_SERVICE_ABILITY, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status: %{public}d", status);
        return;
    }
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Class_BindNativeMethods is fail %{public}d", status);
        return;
    }
    BindContext(env, record->GetWant());
    SetExtensionCommon(EtsExtensionCommon::Create(
        etsRuntime_, static_cast<AppExecFwk::ETSNativeReference &>(*etsObj_), shellContextRef_));
    handler_ = handler;
    auto context = GetContext();
    auto appContext = Context::GetApplicationContext();
    if (context != nullptr && appContext != nullptr) {
        auto appConfig = appContext->GetConfiguration();
        if (appConfig != nullptr) {
            TAG_LOGD(AAFwkTag::SERVICE_EXT, "Original config dump: %{public}s", appConfig->GetName().c_str());
            context->SetConfiguration(std::make_shared<Configuration>(*appConfig));
        }
    }
    ListenWMS();
}

void EtsServiceExtension::ListenWMS()
{
#ifdef SUPPORT_GRAPHICS
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "RegisterDisplayListener");
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null SaMgr");
        return;
    }

    auto etsServiceExtension = std::static_pointer_cast<EtsServiceExtension>(shared_from_this());
    displayListener_ = sptr<EtsServiceExtensionDisplayListener>::MakeSptr(etsServiceExtension);
    if (displayListener_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null displayListener");
        return;
    }

    auto context = GetContext();
    if (context == nullptr || context->GetToken() == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        return;
    }

    saStatusChangeListener_ =
        sptr<SystemAbilityStatusChangeListener>::MakeSptr(displayListener_, context->GetToken());
    if (saStatusChangeListener_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null saStatusChangeListener");
        return;
    }

    auto ret = abilityManager->SubscribeSystemAbility(WINDOW_MANAGER_SERVICE_ID, saStatusChangeListener_);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "subscribe system ability error:%{public}d.", ret);
    }
#endif
}
#ifdef SUPPORT_GRAPHICS
void EtsServiceExtension::SystemAbilityStatusChangeListener::OnAddSystemAbility(int32_t systemAbilityId,
    const std::string& deviceId)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "systemAbilityId: %{public}d add", systemAbilityId);
    if (systemAbilityId == WINDOW_MANAGER_SERVICE_ID) {
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "RegisterDisplayInfoChangedListener");
        Rosen::WindowManager::GetInstance().RegisterDisplayInfoChangedListener(token_, tmpDisplayListener_);
    }
}
#endif //SUPPORT_GRAPHICS

bool EtsServiceExtension::HasScreenDensityBeenSet(std::shared_ptr<Global::Resource::ResourceManager> resourceManager)
{
    TAG_LOGD(AAFwkTag::ABILITY, "call HasScreenDensityBeenSet");
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null resConfig");
        return false;
    }
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null resourceManager");
        return false;
    }
    resourceManager->GetResConfig(*resConfig);
    return resConfig->GetScreenDensityDpi() != Global::Resource::ScreenDensity::SCREEN_DENSITY_NOT_SET;
}

void EtsServiceExtension::OnStart(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnStart");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "env not found Ability.ets");
        return;
    }
    auto context = GetContext();
    if (context != nullptr) {
#ifdef SUPPORT_GRAPHICS
        int32_t displayId = AAFwk::DisplayUtil::GetDefaultDisplayId();
        displayId = want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, displayId);
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "displayId %{public}d", displayId);
        auto configUtils = std::make_shared<ConfigurationUtils>();
        if (!HasScreenDensityBeenSet(context->GetResourceManager())) {
            TAG_LOGD(AAFwkTag::ABILITY, "call InitDisplayConfig");
            configUtils->InitDisplayConfig(displayId, context->GetConfiguration(), context->GetResourceManager());
        }
#endif //SUPPORT_GRAPHICS
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null wantRef");
        return;
    }

    CallObjectMethod(false, "onCreate", ON_CREATE_SIGNATURE, wantRef);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "end");
}

void EtsServiceExtension::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnStop");
    ServiceExtension::OnStop();
    CallObjectMethod(false, "onDestroy", VOID_SIGNATURE);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "end");
}

sptr<IRemoteObject> EtsServiceExtension::OnConnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnConnect");
    Extension::OnConnect(want);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return nullptr;
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null wantRef");
        return nullptr;
    }
    ani_ref result = CallObjectMethod(true, "onConnect", nullptr, wantRef);
    auto obj = reinterpret_cast<ani_object>(result);
    auto remoteObj = AniGetNativeRemoteObject(env, obj);
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "remoteObj null");
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "end");
    return remoteObj;
}

sptr<IRemoteObject> EtsServiceExtension::OnConnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnConnect");
    Extension::OnConnect(want);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return nullptr;
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null wantRef");
        return nullptr;
    }
    ani_long connectCallback = (ani_long)callbackInfo;
    ani_status status = ANI_ERROR;
    ani_field callbackField = nullptr;
    if ((status = env->Class_FindField(etsObj_->aniCls, "connectCallback", &callbackField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_SetField_Long(etsObj_->aniObj, callbackField, connectCallback)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return nullptr;
    }
    ani_ref aniRemoteRef = CallObjectMethod(true, "onConnect", nullptr, wantRef);
    auto aniRemoteobj = reinterpret_cast<ani_object>(aniRemoteRef);
    ani_method method {};
    if ((status = env->Class_FindMethod(etsObj_->aniCls, "checkPromise", CHECK_PROMISE_SIGNATURE, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return nullptr;
    }
    ani_boolean isPromise = ANI_FALSE;
    if ((status = env->Object_CallMethod_Boolean(etsObj_->aniObj, method, &isPromise, aniRemoteobj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return nullptr;
    }
    if (!isPromise) {
        isAsyncCallback = false;
        auto remoteObj = AniGetNativeRemoteObject(env, aniRemoteobj);
        if (remoteObj == nullptr) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "null remoteObj");
        }
        return remoteObj;
    }
    return OnConnectInner(env, aniRemoteobj, isAsyncCallback);
}

sptr<IRemoteObject> EtsServiceExtension::OnConnectInner(ani_env *env, ani_object &aniRemoteobj, bool &isAsyncCallback)
{
    ani_status status = ANI_ERROR;
    ani_method method {};
    if ((status = env->Class_FindMethod(etsObj_->aniCls, "callPromise", CALL_PROMISE_SIGNATURE, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return nullptr;
    }
    ani_boolean callResult = ANI_FALSE;
    if ((status = env->Object_CallMethod_Boolean(etsObj_->aniObj, method, &callResult, aniRemoteobj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return nullptr;
    }
    isAsyncCallback = callResult;
    return nullptr;
}

void EtsServiceExtension::OnDisconnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnDisconnect");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null wantRef");
        return;
    }
    CallObjectMethod(false, "onDisconnect", ON_DISCONNECT_SIGNATURE, wantRef);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "end");
}

void EtsServiceExtension::OnDisconnect(
    const AAFwk::Want &want, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnDisconnect");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null wantRef");
        return;
    }
    if (callbackInfo == nullptr) {
        isAsyncCallback = false;
        OnDisconnect(want);
        return;
    }
    ani_long disconnectCallback = (ani_long)callbackInfo;

    ani_field field = nullptr;
    ani_status status = env->Class_FindField(etsObj_->aniCls, "disconnectCallback", &field);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return;
    }
    if (field == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null field");
        return;
    }
    if ((status = env->Object_SetField_Long(etsObj_->aniObj, field, disconnectCallback)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return;
    }
    ani_method method {};
    if ((status = env->Class_FindMethod(etsObj_->aniCls, "callOnDisconnect", ON_DISCONNECT_SIGNATURE, &method))
        != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return;
    }
    ani_boolean callResult = ANI_FALSE;
    if ((status = env->Object_CallMethod_Boolean(etsObj_->aniObj, method, &callResult, wantRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return;
    }
    isAsyncCallback = callResult;
}

void EtsServiceExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnCommand");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null wantRef");
        return;
    }
    ani_int iStartId = static_cast<ani_int>(startId);
    CallObjectMethod(false, "onRequest", ON_REQUEST_SIGNATURE, wantRef, iStartId);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "end");
    return;
}

bool EtsServiceExtension::HandleInsightIntent(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "HandleInsightIntent called");
    auto callback = std::make_unique<InsightIntentExecutorAsyncCallback>();
    callback.reset(InsightIntentExecutorAsyncCallback::Create());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null callback");
        return false;
    }
    auto executeParam = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    bool ret = AppExecFwk::InsightIntentExecuteParam::GenerateFromWant(want, *executeParam);
    if (!ret) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Generate execute param failed");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return false;
    }
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Insight bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s"
        "insightIntentName: %{public}s, executeMode: %{public}d, intentId: %{public}" PRIu64 "",
        executeParam->bundleName_.c_str(), executeParam->moduleName_.c_str(), executeParam->abilityName_.c_str(),
        executeParam->insightIntentName_.c_str(), executeParam->executeMode_, executeParam->insightIntentId_);
    auto asyncCallback = [weak = weak_from_this(), intentId = executeParam->insightIntentId_]
        (AppExecFwk::InsightIntentExecuteResult result) {
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "intentId %{public}" PRIu64"", intentId);
        auto extension = weak.lock();
        if (extension == nullptr) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "null extension");
            return;
        }
        auto ret = extension->OnInsightIntentExecuteDone(intentId, result);
        if (!ret) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "OnInsightIntentExecuteDone failed");
        }
    };
    callback->Push(asyncCallback);
    InsightIntentExecutorInfo executorInfo;
    const WantParams &wantParams = want.GetParams();
    std::string arkTSMode = wantParams.GetStringParam(AppExecFwk::INSIGHT_INTENT_ARKTS_MODE);
    if (arkTSMode == AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2) {
        ret = GetInsightIntentExecutorInfo(want, executeParam, executorInfo);
        if (!ret) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "Get Intent executor failed");
            InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
            return false;
        }
        ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
            etsRuntime_, executorInfo, std::move(callback));
        if (!ret) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "Execute insight intent failed");
            return false;
        }
        return true;
    } else {
        auto& jsRuntime = etsRuntime_.GetJsRuntime();
        if (jsRuntime == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null jsRuntime");
            return false;
        }
        if (abilityInfo_) {
            auto &jsRuntimePoint = (static_cast<AbilityRuntime::JsRuntime &>(*jsRuntime));
            jsRuntimePoint.UpdateModuleNameAndAssetPath(abilityInfo_->moduleName);
        }
        ret = GetInsightIntentExecutorInfo(want, executeParam, executorInfo);
        if (!ret) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "Get Intent executor failed");
            InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
                static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
            return false;
        }
        ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
            *jsRuntime, executorInfo, std::move(callback));
        if (!ret) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "Execute insight intent failed");
            return false;
        }
        return true;
    }
}

ani_ref EtsServiceExtension::CallObjectMethod(bool withResult, const char *name, const char *signature, ...)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(etsObj_->aniCls, name, signature, &method)) != ANI_OK) {
        return nullptr;
    }
    if (method == nullptr) {
        return nullptr;
    }
    ani_ref res = nullptr;
    va_list args;
    if (withResult) {
        va_start(args, signature);
        if ((status = env->Object_CallMethod_Ref_V(etsObj_->aniObj, method, &res, args)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
            va_end(args);
            return nullptr;
        }
        va_end(args);
        return res;
    }
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(etsObj_->aniObj, method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
    }
    va_end(args);
    return nullptr;
}

ani_object EtsServiceExtension::CreateETSContext(ani_env *env, std::shared_ptr<ServiceExtensionContext> context)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "CreateETSContext");
    return CreateEtsServiceExtensionContext(env, context);
}

void EtsServiceExtension::BindContext(ani_env *env, std::shared_ptr<AAFwk::Want> want)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "BindContext");
    if (env == nullptr || want == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Want info is null or env is null");
        return;
    }
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to get context");
        return;
    }
    ani_object contextObj = CreateETSContext(env, context);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null contextObj");
        return;
    }
    // bind EtsServiceExtension
    ani_field contextField;
    auto status = env->Class_FindField(etsObj_->aniCls, "context", &contextField);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Class_GetField context failed");
        return;
    }
    ani_ref contextRef = nullptr;
    if (env->GlobalReference_Create(contextObj, &contextRef) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "GlobalReference_Create contextObj failed");
        return;
    }
    if (env->Object_SetField_Ref(etsObj_->aniObj, contextField, contextRef) != ANI_OK) {
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "Object_SetField_Ref contextObj failed");
    }
    shellContextRef_ = std::make_shared<AppExecFwk::ETSNativeReference>();
    shellContextRef_->aniObj = contextObj;
    shellContextRef_->aniRef = contextRef;
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "BindContext end");
}

void EtsServiceExtension::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnConfigurationUpdated");
    ServiceExtension::OnConfigurationUpdated(configuration);
    ConfigurationUpdated();
}

void EtsServiceExtension::ConfigurationUpdated()
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "ConfigurationUpdated");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "env nullptr");
        return;
    }
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        return;
    }
    auto fullConfig = context->GetConfiguration();
    if (fullConfig == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null configuration");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_object aniConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    status = env->Object_CallMethodByName_Void(
        etsObj_->aniObj, "onConfigurationUpdate", ON_CONFIGURATION_UPDATE_SIGNATURE, aniConfiguration);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "CALL Object_CallMethod failed, status: %{public}d", status);
        return;
    }
    ani_ref contextRef = nullptr;
    if ((status = env->Object_GetFieldByName_Ref(etsObj_->aniObj, "context", &contextRef)) != ANI_OK ||
        contextRef == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to get field, status : %{public}d", status);
        return;
    }
    ani_class cls = nullptr;
    ani_field configField = nullptr;
    if ((status = env->FindClass(SERVICE_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to find class, status : %{public}d", status);
        return;
    }
    if ((status = env->Class_FindField(cls, "config", &configField)) != ANI_OK || configField == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to find field, status : %{public}d", status);
        return;
    }
    if ((status = env->Object_SetFieldByName_Ref(reinterpret_cast<ani_object>(contextRef), "config",
        aniConfiguration)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to set field, status : %{public}d", status);
        return;
    }
}

void EtsServiceExtension::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Dump");
    Extension::Dump(params, info);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "env nullptr");
        return;
    }
    ani_object arrayObj = nullptr;
    if (!WrapArrayString(env, arrayObj, params)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "WrapArrayString failed");
        arrayObj = nullptr;
        return;
    }
    if (etsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsObj_");
        return;
    }
    if (etsObj_->aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Not found ServiceExtension Obj");
        return;
    }
    if (etsObj_->aniCls == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Not found ServiceExtension class");
        return;
    }

    ani_status status = ANI_ERROR;
    ani_ref dumpInfoRef = nullptr;
    if ((status = env->Object_CallMethodByName_Ref(etsObj_->aniObj, "onDump", ON_DUMP_SIGNATURE, &dumpInfoRef,
        arrayObj)) != ANI_OK || dumpInfoRef == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Object_CallMethod_Ref FAILED: %{public}d", status);
        return;
    }
    std::vector<std::string> dumpInfoStrArray;
    ani_object dumpInfoObj = reinterpret_cast<ani_object>(dumpInfoRef);
    if (!UnwrapArrayString(env, dumpInfoObj, dumpInfoStrArray)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "UnwrapArrayString failed");
        return;
    }
    for (auto dumpInfoStr:dumpInfoStrArray) {
        info.push_back(dumpInfoStr);
    }
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Dump info size: %{public}zu", info.size());
}

#ifdef SUPPORT_GRAPHICS
void EtsServiceExtension::OnCreate(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "enter");
}

void EtsServiceExtension::OnDestroy(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "exit");
}

void EtsServiceExtension::OnDisplayInfoChange(const sptr<IRemoteObject>& token, Rosen::DisplayId displayId,
    float density, Rosen::DisplayOrientation orientation)
{
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "displayId: %{public}" PRIu64, displayId);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        return;
    }

    auto contextConfig = context->GetConfiguration();
    if (contextConfig == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null contextConfig");
        return;
    }

    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Config dump: %{public}s", contextConfig->GetName().c_str());
    bool configChanged = false;
    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateDisplayConfig(displayId, contextConfig, context->GetResourceManager(), configChanged);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Config dump after update: %{public}s", contextConfig->GetName().c_str());

    if (configChanged) {
        auto etsServiceExtension = std::static_pointer_cast<EtsServiceExtension>(shared_from_this());
        auto task = [etsServiceExtension]() {
            if (etsServiceExtension) {
                etsServiceExtension->ConfigurationUpdated();
            }
        };
        if (handler_ != nullptr) {
            handler_->PostTask(task, "EtsServiceExtension:OnChange");
        }
    }
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "finished");
}

void EtsServiceExtension::OnChange(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "displayId: %{public}" PRIu64"", displayId);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        return;
    }

    auto contextConfig = context->GetConfiguration();
    if (contextConfig == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null contextConfig");
        return;
    }

    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Config dump: %{public}s", contextConfig->GetName().c_str());
    bool configChanged = false;
    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateDisplayConfig(displayId, contextConfig, context->GetResourceManager(), configChanged);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Config dump after update: %{public}s", contextConfig->GetName().c_str());

    if (configChanged) {
        auto etsServiceExtension = std::static_pointer_cast<EtsServiceExtension>(shared_from_this());
        auto task = [etsServiceExtension]() {
            if (etsServiceExtension) {
                etsServiceExtension->ConfigurationUpdated();
            }
        };
        if (handler_ != nullptr) {
            handler_->PostTask(task, "EtsServiceExtension:OnChange");
        }
    }

    TAG_LOGD(AAFwkTag::SERVICE_EXT, "finished");
}
#endif
} // namespace AbilityRuntime
} // namespace OHOS

ETS_EXPORT extern "C" OHOS::AbilityRuntime::ServiceExtension *OHOS_ETS_Service_Extension_Create(
    const std::unique_ptr<OHOS::AbilityRuntime::Runtime> &runtime)
{
    return OHOS::AbilityRuntime::EtsServiceExtension::Create(runtime);
}
