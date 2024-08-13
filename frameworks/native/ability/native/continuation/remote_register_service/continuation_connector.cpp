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

#include "continuation_connector.h"

#include "continuation_device_callback_proxy.h"
#include "hilog_tag_wrapper.h"
#include "remote_register_service_proxy.h"

namespace OHOS {
namespace AppExecFwk {
sptr<ContinuationConnector> ContinuationConnector::instance_ = nullptr;
std::mutex ContinuationConnector::mutex_;
const std::string ContinuationConnector::CONNECTOR_DEVICE_ID("");
const std::string ContinuationConnector::CONNECTOR_BUNDLE_NAME("com.ohos.controlcenter");
const std::string ContinuationConnector::CONNECTOR_ABILITY_NAME(
    "com.ohos.controlcenter.fatransfer.service.FeatureAbilityRegisterService");

ContinuationConnector::ContinuationConnector(const std::weak_ptr<Context> &context) : context_(context)
{}

/**
 * @brief get singleton of Class ContinuationConnector
 *
 * @param context: the running context for appcontext
 *
 * @return The singleton of ContinuationConnector
 */
sptr<ContinuationConnector> ContinuationConnector::GetInstance(const std::weak_ptr<Context> &context)
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = sptr<ContinuationConnector>(new (std::nothrow) ContinuationConnector(context));
        }
    }
    return instance_;
}

/**
 * @brief This method is called back to receive the connection result after an ability calls the
 * Ability#connectAbility(Want, IAbilityConnection) method to connect it to a Service ability.
 *
 * @param element: Indicates information about the connected Service ability.
 * @param remote: Indicates the remote proxy object of the Service ability.
 * @param resultCode: Indicates the connection result code. The value 0 indicates a successful connection, and any other
 * value indicates a connection failure.
 */
void ContinuationConnector::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "remote is nullptr");
        return;
    }
    remoteRegisterService_ = iface_cast<RemoteRegisterServiceProxy>(remoteObject);
    if (remoteRegisterService_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "remoteRegisterService_ is nullptr");
        return;
    }
    isConnected_.store(true);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto &iter : continuationRequestList_) {
            iter->Execute();
        }
        continuationRequestList_.clear();
    }
}

/**
 * @brief This method is called back to receive the disconnection result after the connected Service ability crashes or
 * is killed. If the Service ability exits unexpectedly, all its connections are disconnected, and each ability
 * previously connected to it will call onAbilityDisconnectDone.
 *
 * @param element: Indicates information about the disconnected Service ability.
 * @param resultCode: Indicates the disconnection result code. The value 0 indicates a successful disconnection, and any
 * other value indicates a disconnection failure.
 */
void ContinuationConnector::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    remoteRegisterService_ = nullptr;
    isConnected_.store(false);
}

/**
 * @brief bind remote ability of RemoteRegisterService.
 *
 * @param request: request for continuation.
 */
void ContinuationConnector::BindRemoteRegisterAbility(const std::shared_ptr<AppExecFwk::ContinuationRequest> &request)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    std::shared_ptr tmpcontext = context_.lock();
    if (tmpcontext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "tmpcontext is nullptr");
        return;
    }
    if (request == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "request is nullptr");
        return;
    }
    if (IsAbilityConnected()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "remote register bounded");
        request->Execute();
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        continuationRequestList_.push_back(request);
    }
    BindRemoteRegisterAbility();
}

/**
 * @brief unbind remote ability of RemoteRegisterService.
 */
void ContinuationConnector::UnbindRemoteRegisterAbility()
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    std::shared_ptr tmpcontext = context_.lock();
    if (tmpcontext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "tmpcontext is nullptr");
        return;
    }

    tmpcontext->DisconnectAbility(this);
    isConnected_.store(false);
    remoteRegisterService_ = nullptr;
}

/**
 * @brief check whether connected to remote register service.
 *
 * @return bool true if connected, otherwise false.
 */
bool ContinuationConnector::IsAbilityConnected()
{
    return isConnected_.load();
}

/**
 * @brief unregister to control center continuation register service.
 *
 * @param token token from register return value.
 *
 * @return bool result of unregister.
 */
bool ContinuationConnector::Unregister(int token)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    if (remoteRegisterService_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "remoteRegisterService_ is nullptr");
        return false;
    }

    return remoteRegisterService_->Unregister(token);
}

/**
 * @brief notify continuation status to control center continuation register service.
 *
 * @param token token from register.
 * @param deviceId device id.
 * @param status device status.
 *
 * @return bool result of updateConnectStatus.
 */
bool ContinuationConnector::UpdateConnectStatus(int token, const std::string &deviceId, int status)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    if (remoteRegisterService_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "remoteRegisterService_ is nullptr");
        return false;
    }

    return remoteRegisterService_->UpdateConnectStatus(token, deviceId, status);
}

/**
 * @brief notify control center continuation register service to show device list.
 *
 * @param token token from register
 * @param parameter filter with supported device list.
 * @return bool result of showDeviceList.
 */
bool ContinuationConnector::ShowDeviceList(int token, const AppExecFwk::ExtraParams &parameter)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    if (remoteRegisterService_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "remoteRegisterService_ is nullptr");
        return false;
    }
    return remoteRegisterService_->ShowDeviceList(token, parameter);
}

/**
 * @brief register to control center continuation register service.
 *
 * @param context ability context.
 * @param bundleName bundle name of ability.
 * @param parameter filter with supported device list.
 * @param callback callback for device connect and disconnect.
 *
 * @return int token.
 */
int ContinuationConnector::Register(std::weak_ptr<Context> &context, std::string bundleName,
    const AppExecFwk::ExtraParams &parameter, std::shared_ptr<IContinuationDeviceCallback> &callback)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    std::shared_ptr pcontext = context.lock();
    if (pcontext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "pcontext is nullptr");
        return -1;
    }
    if (remoteRegisterService_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "remoteRegisterService_ is nullptr");
        return -1;
    }
    sptr<IRemoteObject> token = pcontext->GetToken();
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "token is nullptr");
        return -1;
    }

    sptr<ContinuationDeviceCallbackProxy> callBackSptr(new (std::nothrow) ContinuationDeviceCallbackProxy(callback));

    return remoteRegisterService_->Register(bundleName, token, parameter, callBackSptr);
}

/**
 * @brief bind remote ability of RemoteRegisterService.
 */
void ContinuationConnector::BindRemoteRegisterAbility()
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    std::shared_ptr tmpcontext = context_.lock();
    if (tmpcontext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "tmpcontext is nullptr");
        return;
    }
    Want want;
    want.SetElementName(CONNECTOR_DEVICE_ID, CONNECTOR_BUNDLE_NAME, CONNECTOR_ABILITY_NAME);
    want.AddFlags(Want::FLAG_NOT_OHOS_COMPONENT);
    tmpcontext->ConnectAbility(want, this);
}
}  // namespace AppExecFwk
}  // namespace OHOS
