/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "js_ui_service_host_proxy.h"

#include "ability_business_error.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "js_error_utils.h"
#include "napi_common_want.h"
#include "permission_constants.h"
#include "tokenid_kit.h"
#include "ui_service_host_proxy.h"

namespace OHOS {
namespace AAFwk {
using namespace AbilityRuntime;

static constexpr int32_t INDEX_ZERO = 0;
static constexpr int32_t ARGC_ONE = 1;

napi_ref JsUIServiceHostProxy::CreateJsUIServiceHostProxy(napi_env env, const sptr<IRemoteObject>& impl)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "called");
    HandleScope handleScope(env);
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null object");
        return nullptr;
    }

    std::unique_ptr<JsUIServiceHostProxy> proxy = std::make_unique<JsUIServiceHostProxy>(impl);
    napi_ref nref = nullptr;
    napi_status status = napi_wrap(env, object, proxy.release(), JsUIServiceHostProxy::Finalizer, nullptr, &nref);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "napi_wrap failed %{public}d", status);
    }
    const char *moduleName = "JsUIServiceHostProxy";
    BindNativeFunction(env, object, "sendData", moduleName, JsUIServiceHostProxy::SendData);
    return nref;
}

void JsUIServiceHostProxy::Finalizer(napi_env env, void* data, void* hint)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "called");
    std::unique_ptr<JsUIServiceHostProxy>(static_cast<JsUIServiceHostProxy*>(data));
}

JsUIServiceHostProxy::JsUIServiceHostProxy(const sptr<IRemoteObject>& impl)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "called");
    if (impl != nullptr) {
        proxy_ = iface_cast<OHOS::AAFwk::IUIServiceHost>(impl);
    }
    if (proxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null proxy");
    }
}

JsUIServiceHostProxy::~JsUIServiceHostProxy()
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "called");
    proxy_ = nullptr;
}

bool JsUIServiceHostProxy::CheckCallerIsSystemApp()
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "not allow");
        return false;
    }
    return true;
}

napi_value JsUIServiceHostProxy::SendData(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsUIServiceHostProxy, OnSendData);
}

napi_value JsUIServiceHostProxy::OnSendData(napi_env env, NapiCallbackInfo& info)
{
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "CheckCallerIsSystemApp failed");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }
    if (proxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null proxy_");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::UI_SERVICE_PROXY_NOT_FOUND));
        return CreateJsUndefined(env);
    }
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    AAFwk::WantParams params;
    bool result = AppExecFwk::UnwrapWantParams(env, info.argv[INDEX_ZERO], params);
    if (!result) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "UnwrapWantParams failed");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM), "Data verification failed");
        return CreateJsUndefined(env);
    }

    int32_t ret = proxy_->SendData(params);
    if (ret != static_cast<int32_t>(AbilityErrorCode::ERROR_OK)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "SendData failed");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::SEND_DATA_FAILED));
    }
    return CreateJsUndefined(env);
}
}
}
