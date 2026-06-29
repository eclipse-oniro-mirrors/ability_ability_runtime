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

#include "ets_ui_service_host_proxy.h"

#include "ability_business_error.h"
#include "ani_common_want.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "permission_constants.h"
#include "tokenid_kit.h"
#include "ui_service_host_proxy.h"

namespace OHOS {
namespace AAFwk {
using namespace AbilityRuntime;

constexpr const char *UI_SERVICE_HOST_PROXY_CLASS_NAME = "application.UIServiceHostProxy.UIServiceHostProxyImpl";

ani_object EtsUIServiceHostProxy::CreateEtsUIServiceHostProxy(ani_env *env, const sptr<IRemoteObject> &impl)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "CreateEtsUIServiceHostProxy called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null env");
        return nullptr;
    }
    ani_object object = nullptr;
    ani_class cls {};
    ani_status status = env->FindClass(UI_SERVICE_HOST_PROXY_CLASS_NAME, &cls);
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
    ani_field field = nullptr;
    if ((status = env->Class_FindField(cls, "nativeServiceHostProxy", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    std::unique_ptr<EtsUIServiceHostProxy> proxy = std::make_unique<EtsUIServiceHostProxy>(impl);
    ani_long nativeServiceHostProxyLong = reinterpret_cast<ani_long>(proxy.release());
    if ((status = env->Object_SetField_Long(object, field, nativeServiceHostProxyLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    std::array functions = {ani_native_function {"nativeSendData", nullptr,
        reinterpret_cast<void *>(EtsUIServiceHostProxy::SendData)}};
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::APPKIT, "Class_BindNativeMethods status: %{public}d", status);
        return nullptr;
    }
    return object;
}

EtsUIServiceHostProxy::EtsUIServiceHostProxy(const sptr<IRemoteObject> &impl)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "EtsUIServiceHostProxy called");
    if (impl != nullptr) {
        proxy_ = iface_cast<OHOS::AAFwk::IUIServiceHost>(impl);
    }
    if (proxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null proxy");
    }
}

EtsUIServiceHostProxy::~EtsUIServiceHostProxy()
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "EtsUIServiceHostProxy destroyed");
    proxy_ = nullptr;
}

bool EtsUIServiceHostProxy::CheckCallerIsSystemApp()
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "not allow");
        return false;
    }
    return true;
}

EtsUIServiceHostProxy* EtsUIServiceHostProxy::GetEtsUIServiceHostProxy(ani_env *env, ani_object obj)
{
    if (env == nullptr || obj == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null env or obj");
        return nullptr;
    }
    EtsUIServiceHostProxy *serviceHostProxy = nullptr;
    ani_status status = ANI_ERROR;
    ani_long nativeServiceHostProxyLong = 0;
    if ((status = env->Object_GetFieldByName_Long(obj, "nativeServiceHostProxy",
        &nativeServiceHostProxyLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "status: %{public}d", status);
        return nullptr;
    }
    serviceHostProxy = reinterpret_cast<EtsUIServiceHostProxy *>(nativeServiceHostProxyLong);
    if (serviceHostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "serviceHostProxy null");
        return nullptr;
    }
    return serviceHostProxy;
}

void EtsUIServiceHostProxy::SendData(ani_env *env, ani_object obj, ani_object data)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "SendData called");
    EtsUIServiceHostProxy* etsProxy = GetEtsUIServiceHostProxy(env, obj);
    if (etsProxy == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "GetEtsUIServiceHostProxy failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    etsProxy->OnSendData(env, data);
}

void EtsUIServiceHostProxy::OnSendData(ani_env *env, ani_object data)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null env");
        return;
    }
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "CheckCallerIsSystemApp failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
    if (proxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null proxy_");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::UI_SERVICE_PROXY_NOT_FOUND));
        return;
    }
    AAFwk::WantParams params;
    bool result = AppExecFwk::UnwrapWantParams(env, data, params);
    if (!result) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "UnwrapWantParams failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(
            env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM), "Data verification failed");
        return;
    }

    int32_t ret = proxy_->SendData(params);
    if (ret != static_cast<int32_t>(AbilityErrorCode::ERROR_OK)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "SendData failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::SEND_DATA_FAILED));
    }
}
}
}
