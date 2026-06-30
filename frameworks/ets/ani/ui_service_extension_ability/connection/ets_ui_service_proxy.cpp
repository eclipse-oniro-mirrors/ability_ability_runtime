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

#include "ets_ui_service_proxy.h"

#include "ability_business_error.h"
#include "ani_common_want.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
using namespace AbilityRuntime;

constexpr const char *UI_SERVICE_PROXY_CLASS_NAME = "application.UIServiceProxy.UIServiceProxyImpl";

ani_object EtsUIServiceProxy::CreateEtsUIServiceProxy(
    ani_env *env, const sptr<IRemoteObject> &impl, int64_t connectionId, const sptr<IRemoteObject> &hostProxy)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "CreateEtsUIServiceProxy called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null env");
        return nullptr;
    }
    ani_object object = nullptr;
    ani_class cls {};
    ani_status status = env->FindClass(UI_SERVICE_PROXY_CLASS_NAME, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindClass failed status: %{public}d", status);
        return nullptr;
    }
    ani_method method = nullptr;
    status = env->Class_FindMethod(cls, "<ctor>", ":", &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "call Class_FindMethod ctor failed");
        return nullptr;
    }
    status = env->Object_New(cls, method, &object);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "call Object_New failed");
        return nullptr;
    }
    ani_field field = nullptr;
    if ((status = env->Class_FindField(cls, "nativeServiceProxy", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    std::unique_ptr<EtsUIServiceProxy> proxy = std::make_unique<EtsUIServiceProxy>(impl, hostProxy);
    proxy->SetConnectionId(connectionId);

    ani_long nativeServiceProxyLong = reinterpret_cast<ani_long>(proxy.release());
    if ((status = env->Object_SetField_Long(object, field, nativeServiceProxyLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }

    std::array functions = {
        ani_native_function {"nativeSendData", nullptr,
            reinterpret_cast<void *>(EtsUIServiceProxy::SendData)}};

    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::APPKIT, "Class_BindNativeMethods status: %{public}d", status);
        return nullptr;
    }
    return object;
}

ani_object EtsUIServiceProxy::CreateEmptyProxyObject(ani_env *env)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "CreateEmptyProxyObject called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return nullptr;
    }
    ani_object object = nullptr;
    ani_class cls {};
    ani_status status = env->FindClass(UI_SERVICE_PROXY_CLASS_NAME, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindClass failed status: %{public}d", status);
        return nullptr;
    }
    ani_method method = nullptr;
    status = env->Class_FindMethod(cls, "<ctor>", ":", &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "call Class_FindMethod ctor failed");
        return nullptr;
    }
    status = env->Object_New(cls, method, &object);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "call Object_New abilityStageCtxCls failed");
        return nullptr;
    }
    return object;
}

EtsUIServiceProxy::EtsUIServiceProxy(const sptr<IRemoteObject> &impl, const sptr<IRemoteObject> &hostProxy)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "EtsUIServiceProxy called");
    proxy_ = iface_cast<OHOS::AAFwk::IUIService>(impl);
    hostProxy_ = hostProxy;
    if (proxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null proxy");
    }
}

EtsUIServiceProxy::~EtsUIServiceProxy()
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "EtsUIServiceProxy destroyed");
    proxy_ = nullptr;
    hostProxy_ = nullptr;
}

EtsUIServiceProxy* EtsUIServiceProxy::GetEtsUIServiceProxy(ani_env *env, ani_object obj)
{
    if (env == nullptr || obj == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null env or obj");
        return nullptr;
    }
    EtsUIServiceProxy *serviceProxy = nullptr;
    ani_status status = ANI_ERROR;
    ani_long nativeServiceProxyLong = 0;
    if ((status = env->Object_GetFieldByName_Long(obj, "nativeServiceProxy", &nativeServiceProxyLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "status: %{public}d", status);
        return nullptr;
    }
    serviceProxy = reinterpret_cast<EtsUIServiceProxy *>(nativeServiceProxyLong);
    if (serviceProxy == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "serviceProxy null");
        return nullptr;
    }
    return serviceProxy;
}

void EtsUIServiceProxy::SendData(ani_env *env, ani_object obj, ani_object data)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "SendData called");
    EtsUIServiceProxy* etsUIServiceProxy = GetEtsUIServiceProxy(env, obj);
    if (etsUIServiceProxy == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "GetEtsUIServiceProxy failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    etsUIServiceProxy->OnSendData(env, data);
}

void EtsUIServiceProxy::OnSendData(ani_env *env, ani_object data)
{
    if (proxy_ == nullptr || hostProxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null proxy_ or hostProxy_");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::UI_SERVICE_PROXY_NOT_FOUND));
        return;
    }
    AAFwk::WantParams params;
    bool result = AppExecFwk::UnwrapWantParams(env, data, params);
    if (!result) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "UnwrapWantParams failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return;
    }

    int32_t ret = proxy_->SendData(hostProxy_, params);
    if (ret != static_cast<int32_t>(AbilityErrorCode::ERROR_OK)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "proxy_->SendData failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::SEND_DATA_FAILED));
    }
}
}  // namespace AAFwk
}  // namespace OHOS
