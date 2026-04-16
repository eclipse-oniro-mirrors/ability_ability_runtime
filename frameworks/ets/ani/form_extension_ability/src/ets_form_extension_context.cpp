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

#include "ets_form_extension_context.h"

#include <algorithm>
#include <iterator>

#include "ani_common_configuration.h"
#include "ani_common_want.h"
#include "ets_context_utils.h"
#include "ets_error_utils.h"
#include "ets_extension_context.h"
#include "form_extension_context.h"
#include "hilog_tag_wrapper.h"
#include "remote_object_taihe_ani.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr ani_long ERROR_LONG_VALUE = -1;
constexpr const char *FORM_EXTENSION_CONTEXT_CLASS_NAME = "application.FormExtensionContext.FormExtensionContext";
constexpr const char *CLEANER_CLASS_NAME = "application.FormExtensionContext.Cleaner";
constexpr const char *SIGNATURE_START_ABILITY_FORM_EXTENSION =
    "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *SIGNATURE_CONNECT_FORM_EXTENSION =
    "C{@ohos.app.ability.Want.Want}C{ability.connectOptions.ConnectOptions}:l";
constexpr const char *SIGNATURE_DISCONNECT_FORM_EXTENSION = "lC{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *SIGNATURE_CHECK_WANT = "C{@ohos.app.ability.Want.Want}:";
constexpr const char *SIGNATURE_CHECK_CONNECTION = ":";
constexpr int32_t ARGC_ONE = 1;
constexpr int32_t ARGC_TWO = 2;

std::map<EtsConnectionKey, sptr<ETSFormExtensionConnection>, ets_key_compare> g_connects;
std::mutex g_connectsMutex_;
int64_t g_serialNumber = 0;

bool BindNativeMethods(ani_env *env, ani_class &cls)
{
    ani_status status = ANI_ERROR;
    std::array functions = {
        ani_native_function { "nativeStartAbility", SIGNATURE_START_ABILITY_FORM_EXTENSION,
            reinterpret_cast<void *>(ETSFormExtensionContext::StartAbility) },
        ani_native_function { "nativeConnectServiceExtensionAbility", SIGNATURE_CONNECT_FORM_EXTENSION,
            reinterpret_cast<void *>(ETSFormExtensionContext::ConnectAbility) },
        ani_native_function { "nativeDisconnectServiceExtensionAbility", SIGNATURE_DISCONNECT_FORM_EXTENSION,
            reinterpret_cast<void *>(ETSFormExtensionContext::DisconnectAbility) },
        ani_native_function { "nativeCheckWant", SIGNATURE_CHECK_WANT,
            reinterpret_cast<void *>(ETSFormExtensionContext::CheckWant) },
        ani_native_function { "nativeCheckConnectionAbility", SIGNATURE_CHECK_CONNECTION,
            reinterpret_cast<void *>(ETSFormExtensionContext::CheckConnectionAbility) },
    };
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "bind method status: %{public}d", status);
        return false;
    }
    ani_class cleanerCls = nullptr;
    status = env->FindClass(CLEANER_CLASS_NAME, &cleanerCls);
    if (status != ANI_OK || cleanerCls == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to find class, status: %{public}d", status);
        return false;
    }
    std::array CleanerMethods = {
        ani_native_function { "clean", nullptr, reinterpret_cast<void *>(ETSFormExtensionContext::Finalizer) },
    };
    if ((status = env->Class_BindNativeMethods(cleanerCls, CleanerMethods.data(), CleanerMethods.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "bind method status: %{public}d", status);
        return false;
    }
    return true;
}
bool CheckConnectionParam(ani_object connectOptionsObj,
    sptr<ETSFormExtensionConnection>& connection, AAFwk::Want& want)
{
    connection->SetConnectionRef(connectOptionsObj);
    EtsConnectionKey key;
    key.id = g_serialNumber;
    key.want = want;
    connection->SetConnectionId(key.id);
    {
        std::lock_guard<std::mutex> lock(g_connectsMutex_);
        g_connects.emplace(key, connection);
    }
    if (g_serialNumber < INT32_MAX) {
        g_serialNumber++;
    } else {
        g_serialNumber = 0;
    }
    TAG_LOGD(AAFwkTag::FORM_EXT, "not find connection");
    return true;
}

void FindConnection(AAFwk::Want& want,
    sptr<ETSFormExtensionConnection>& connection, int64_t& connectId)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "connection:%{public}d", static_cast<int32_t>(connectId));
    std::lock_guard<std::mutex> lock(g_connectsMutex_);
    auto item = std::find_if(g_connects.begin(),
        g_connects.end(),
        [&connectId](const auto &obj) {
            return connectId == obj.first.id;
        });
    if (item != g_connects.end()) {
        // match id
        want = item->first.want;
        connection = item->second;
        TAG_LOGD(AAFwkTag::FORM_EXT, "ability not exist");
    }
    return;
}

void RemoveConnection(int64_t connectId)
{
    std::lock_guard<std::mutex> lock(g_connectsMutex_);
    auto item = std::find_if(g_connects.begin(), g_connects.end(),
    [&connectId](const auto &obj) {
        return connectId == obj.first.id;
    });
    if (item != g_connects.end()) {
        TAG_LOGD(AAFwkTag::FORM_EXT, "ability exist");
        if (item->second) {
            item->second->RemoveConnectionObject();
        }
        g_connects.erase(item);
    } else {
        TAG_LOGD(AAFwkTag::FORM_EXT, "ability not exist");
    }
}
} // namespace

void ETSFormExtensionContext::Finalizer(ani_env *env, ani_object obj)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "Finalizer");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return;
    }
    ani_long nativeEtsContextPtr;
    if (env->Object_GetFieldByName_Long(obj, "nativeEtsContext", &nativeEtsContextPtr) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to get nativeEtsContext");
        return;
    }
    if (nativeEtsContextPtr != 0) {
        delete reinterpret_cast<ETSFormExtensionContext *>(nativeEtsContextPtr);
    }
}

void ETSFormExtensionContext::StartAbility(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object call)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "StartAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return;
    }
    auto etsFormExtensionContext = ETSFormExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsFormExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null etsFormExtensionContext");
        return;
    }
    etsFormExtensionContext->OnStartAbility(env, aniObj, wantObj, call);
}

ani_long ETSFormExtensionContext::ConnectAbility(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_object connectOptionsObj)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "ConnectAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return ERROR_LONG_VALUE;
    }
    auto etsFormExtensionContext = ETSFormExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsFormExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null etsFormExtensionContext");
        return ERROR_LONG_VALUE;
    }
    return etsFormExtensionContext->OnConnectAbility(env, aniObj, wantObj, connectOptionsObj);
}

void ETSFormExtensionContext::DisconnectAbility(ani_env *env, ani_object aniObj, ani_long connectId,
    ani_object callback)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "DisconnectAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return;
    }
    auto etsFormExtensionContext = ETSFormExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsFormExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null etsFormExtensionContext");
        return;
    }
    etsFormExtensionContext->OnDisconnectAbility(env, aniObj, connectId, callback);
}

void ETSFormExtensionContext::CheckConnectionAbility(ani_env *env, ani_object aniObj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return;
    }
    auto etsFormExtensionContext = ETSFormExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsFormExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null etsFormExtensionContext");
        return;
    }
    etsFormExtensionContext->OnCheckConnectionAbility(env, aniObj);
}

void ETSFormExtensionContext::OnCheckConnectionAbility(ani_env *env, ani_object aniObj)
{
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "not system app");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
}

ETSFormExtensionContext *ETSFormExtensionContext::GetEtsAbilityContext(ani_env *env, ani_object aniObj)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "GetEtsAbilityContext");
    ani_class cls = nullptr;
    ani_long nativeContextLong;
    ani_field contextField = nullptr;
    ani_status status = ANI_ERROR;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(FORM_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to find class, status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindField(cls, "nativeEtsContext", &contextField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to find filed, status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_GetField_Long(aniObj, contextField, &nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to get filed, status : %{public}d", status);
        return nullptr;
    }
    auto weakContext = reinterpret_cast<ETSFormExtensionContext *>(nativeContextLong);
    return weakContext;
}

void ETSFormExtensionContext::OnStartAbility(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object call)
{
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "unwrap want failed");
        EtsErrorUtil::ThrowError(env, (int32_t)AbilityErrorCode::ERROR_CODE_INVALID_PARAM,
            "Parameter error. The type of \"want\" must be Want.");
        return;
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "Start bundle: %{public}s ability: %{public}s",
        want.GetBundle().c_str(),
        want.GetElement().GetAbilityName().c_str());
    auto context = context_.lock();
    if (!context) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null context");
        AppExecFwk::AsyncCallback(env, call,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }
    ErrCode innerErrorCode = context->StartAbility(want);
    if (innerErrorCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Start failed: %{public}d", innerErrorCode);
        AppExecFwk::AsyncCallback(env, call,
            EtsErrorUtil::CreateErrorByNativeErr(env, (int32_t)innerErrorCode), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, call, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
}

void ETSFormExtensionContext::CheckWant(ani_env *env, ani_object aniObj, ani_object wantObj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "unwrap want failed");
        EtsErrorUtil::ThrowError(env, (int32_t)AbilityErrorCode::ERROR_CODE_INVALID_PARAM,
            "Parameter error. The type of \"want\" must be Want.");
        return;
    }
}

ani_long ETSFormExtensionContext::OnConnectAbility(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_object connectOptionsObj)
{
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "not system app");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return ERROR_LONG_VALUE;
    }
    ani_vm *etsVm = nullptr;
    if (env->GetVM(&etsVm) != ANI_OK || etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to getVM");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return ERROR_LONG_VALUE;
    }
    AAFwk::Want want;
    sptr<ETSFormExtensionConnection> connection = new ETSFormExtensionConnection(etsVm);
    if (!AppExecFwk::UnwrapWant(env, wantObj, want) ||
        !CheckConnectionParam(connectOptionsObj, connection, want)) {
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return ERROR_LONG_VALUE;
    }
    int64_t connectId = connection->GetConnectionId();
    auto context = context_.lock();
    if (!context) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null context");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        RemoveConnection(connectId);
        return ERROR_LONG_VALUE;
    }
    auto innerErrorCode = context->ConnectAbility(want, connection);
    int32_t errcode = static_cast<int32_t>(GetJsErrorCodeByNativeError(innerErrorCode));
    if (errcode) {
        connection->CallEtsFailed(errcode);
        RemoveConnection(connectId);
        return ERROR_LONG_VALUE;
    }
    return connectId;
}

void ETSFormExtensionContext::OnDisconnectAbility(ani_env *env, ani_object aniObj, ani_long connectId,
    ani_object callback)
{
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "not system app");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
    AAFwk::Want want;
    sptr<ETSFormExtensionConnection> connection = nullptr;
    FindConnection(want, connection, connectId);
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null context");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }
    if (!connection) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null connection");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INNER), nullptr);
        return;
    }
    auto innerErrorCode = context->DisconnectAbility(want, connection);
    if (innerErrorCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Disconnect failed: %{public}d", innerErrorCode);
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateErrorByNativeErr(env, (int32_t)innerErrorCode), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
}

ani_object CreateEtsFormExtensionContext(ani_env *env, std::shared_ptr<FormExtensionContext> &context)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "CreateEtsFormExtensionContext call");
    if (env == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env or context");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass(FORM_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to find class, status: %{public}d", status);
        return nullptr;
    }
    if (!BindNativeMethods(env, cls)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to BindNativeMethods");
        return nullptr;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, "<ctor>", "l:", &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to find constructor, status : %{public}d", status);
        return nullptr;
    }
    std::unique_ptr<ETSFormExtensionContext> workContext = std::make_unique<ETSFormExtensionContext>(context);
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to create etsFormExtensionContext");
        return nullptr;
    }
    auto formContextPtr = new std::weak_ptr<FormExtensionContext> (workContext->GetAbilityContext());
    if (formContextPtr == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "formContextPtr is nullptr");
        return nullptr;
    }
    ani_object contextObj = nullptr;
    if ((status = env->Object_New(cls, method, &contextObj, (ani_long)workContext.release())) != ANI_OK ||
        contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to create object, status : %{public}d", status);
        return nullptr;
    }
    if (!ContextUtil::SetNativeContextLong(env, contextObj, (ani_long)(formContextPtr))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to setNativeContextLong ");
        return nullptr;
    }
    ContextUtil::CreateEtsBaseContext(env, cls, contextObj, context);
    CreateEtsExtensionContext(env, cls, contextObj, context, context->GetAbilityInfo());
    return contextObj;
}

ETSFormExtensionConnection::ETSFormExtensionConnection(ani_vm *etsVm) : etsVm_(etsVm) {}

ETSFormExtensionConnection::~ETSFormExtensionConnection()
{
    RemoveConnectionObject();
}

void ETSFormExtensionConnection::SetConnectionId(int32_t id)
{
    connectionId_ = id;
}

void ETSFormExtensionConnection::RemoveConnectionObject()
{
    if (etsVm_ != nullptr && stsConnectionRef_ != nullptr) {
        ani_env *env = nullptr;
        if (etsVm_->GetEnv(ANI_VERSION_1, &env) == ANI_OK && env != nullptr) {
            env->GlobalReference_Delete(stsConnectionRef_);
            stsConnectionRef_ = nullptr;
        }
    }
}

void ETSFormExtensionConnection::CallEtsFailed(int32_t errorCode)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "CallEtsFailed");
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null etsVm");
        return;
    }
    if (stsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null stsConnectionRef_");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_OK;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to get env, status: %{public}d", status);
        return;
    }
    ani_ref funRef;
    if ((status = env->Object_GetPropertyByName_Ref(reinterpret_cast<ani_object>(stsConnectionRef_),
        "onFailed", &funRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "get onFailed failed status : %{public}d", status);
        return;
    }
    if (!AppExecFwk::IsValidProperty(env, funRef)) {
        TAG_LOGI(AAFwkTag::FORM_EXT, "invalid onFailed property");
        return;
    }
    ani_object errorCodeObj = AppExecFwk::CreateInt(env, errorCode);
    if (errorCodeObj == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null errorCodeObj");
        return;
    }
    ani_ref result;
    std::vector<ani_ref> argv = { errorCodeObj };
    if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_ONE, argv.data(),
        &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to call onFailed, status: %{public}d", status);
    }
}

void ETSFormExtensionConnection::SetConnectionRef(ani_object connectOptionsObj)
{
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "etsVm_ is nullptr");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status: %{public}d", status);
        return;
    }
    if ((status = env->GlobalReference_Create(connectOptionsObj, &stsConnectionRef_)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status: %{public}d", status);
    }
}

void ETSFormExtensionConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int32_t resultCode)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnAbilityConnectDone");
    if (etsVm_ == nullptr || stsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null stsConnectionRef or etsVm");
        return;
    }
    ani_env *env = AttachCurrentThread();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "GetEnv failed");
        return;
    }
    ani_ref refElement = AppExecFwk::WrapElementName(env, element);
    if (refElement == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null refElement");
        DetachCurrentThread();
        return;
    }
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null remoteObject");
        DetachCurrentThread();
        return;
    }
    ani_object refRemoteObject = ANI_ohos_rpc_CreateJsRemoteObject(env, remoteObject);
    if (refRemoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null refRemoteObject");
        DetachCurrentThread();
        return;
    }
    ani_status status = ANI_ERROR;
    ani_ref funRef;
    if ((status = env->Object_GetPropertyByName_Ref(reinterpret_cast<ani_object>(stsConnectionRef_),
        "onConnect", &funRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "get onConnect failed status : %{public}d", status);
        return;
    }
    if (!AppExecFwk::IsValidProperty(env, funRef)) {
        TAG_LOGI(AAFwkTag::FORM_EXT, "invalid onConnect property");
        return;
    }
    ani_ref result;
    std::vector<ani_ref> argv = { refElement, refRemoteObject};
    if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_TWO, argv.data(),
        &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to call onConnect, status: %{public}d", status);
    }
    DetachCurrentThread();
}

void ETSFormExtensionConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnAbilityDisconnectDone");
    if (etsVm_ == nullptr || stsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null stsConnectionRef or etsVm");
        return;
    }
    ani_env *env = AttachCurrentThread();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "GetEnv failed");
        return;
    }
    ani_ref refElement = AppExecFwk::WrapElementName(env, element);
    if (refElement == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null refElement");
        DetachCurrentThread();
        return;
    }
    ani_status status = ANI_ERROR;
    ani_ref funRef;
    if ((status = env->Object_GetPropertyByName_Ref(reinterpret_cast<ani_object>(stsConnectionRef_),
        "onDisconnect", &funRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "get onDisconnect failed status : %{public}d", status);
        return;
    }
    if (!AppExecFwk::IsValidProperty(env, funRef)) {
        TAG_LOGI(AAFwkTag::FORM_EXT, "invalid onDisconnect property");
        return;
    }
    ani_ref result;
    std::vector<ani_ref> argv = { refElement };
    if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_ONE, argv.data(),
        &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to call onDisconnect, status: %{public}d", status);
    }
    DetachCurrentThread();
}

ani_env *ETSFormExtensionConnection::AttachCurrentThread()
{
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) == ANI_OK) {
        return env;
    }
    ani_option interopEnabled { "--interop=disable", nullptr };
    ani_options aniArgs { 1, &interopEnabled };
    if ((status = etsVm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status: %{public}d", status);
        return nullptr;
    }
    isAttachThread_ = true;
    return env;
}

void ETSFormExtensionConnection::DetachCurrentThread()
{
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null etsVm");
        return;
    }
    if (isAttachThread_) {
        etsVm_->DetachCurrentThread();
        isAttachThread_ = false;
    }
}
} // namespace AbilityRuntime
} // namespace OHOS