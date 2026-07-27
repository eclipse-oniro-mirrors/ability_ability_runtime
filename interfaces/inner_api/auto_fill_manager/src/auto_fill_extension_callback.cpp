/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "auto_fill_extension_callback.h"

#include "auto_fill_error.h"
#include "auto_fill_manager.h"
#include "auto_fill_manager_util.h"
#include "hilog_tag_wrapper.h"
#include "int_wrapper.h"
#include "string_wrapper.h"
#include "view_data.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
#ifdef SUPPORT_GRAPHICS
constexpr const char* WANT_PARAMS_VIEW_DATA_KEY = "ohos.ability.params.viewData";
constexpr const char* WANT_PARAMS_AUTO_FILL_CMD_KEY = "ohos.ability.params.autoFillCmd";
constexpr const char* WANT_PARAMS_FILL_CONTENT = "ohos.ability.params.fillContent";
constexpr const char* WANT_PARAMS_CUSTOM_DATA_KEY = "ohos.ability.params.customData";
constexpr const char* WANT_PARAMS_AUTO_FILL_EVENT_KEY = "ability.want.params.AutoFillEvent";
constexpr const char* WANT_PARAMS_UPDATE_POPUP_WIDTH = "ohos.ability.params.popupWidth";
constexpr const char* WANT_PARAMS_UPDATE_POPUP_HEIGHT = "ohos.ability.params.popupHeight";
constexpr const char* WANT_PARAMS_UPDATE_POPUP_PLACEMENT = "ohos.ability.params.popupPlacement";
constexpr const char* WANT_PARAMS_EXTENSION_TYPE_KEY = "ability.want.params.uiExtensionType";
constexpr const char* WANT_PARAMS_EXTENSION_TYPE = "autoFill/password";
constexpr const char* WANT_PARAMS_SMART_EXTENSION_TYPE = "autoFill/smart";
constexpr const char* WANT_PARAMS_AUTO_FILL_TYPE_KEY = "ability.want.params.AutoFillType";
constexpr const char* WANT_PARAMS_AUTO_FILL_POPUP_WINDOW_KEY = "ohos.ability.params.popupWindow";
#endif // SUPPORT_GRAPHICS
} // namespace

#ifdef SUPPORT_GRAPHICS
AutoFillExtensionCallback::AutoFillExtensionCallback()
{
    callbackId_ = GenerateCallbackId();
}

void AutoFillExtensionCallback::OnResult(int32_t errCode, const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called, result code: %{public}d", errCode);
    AutoFillManager::GetInstance().RemoveEvent(callbackId_);
    CloseUIExtension();
    if (autoFillWindowType_ == AutoFill::AutoFillWindowType::POPUP_WINDOW) {
        isOnResult_ = true;
        want_ = want;
        errCode_ = errCode;
        return;
    }

    if (errCode == AutoFill::AUTO_FILL_SUCCESS) {
        SendAutoFillSuccess(want);
    } else {
        auto resultCode = (errCode == AutoFill::AUTO_FILL_CANCEL) ?
            AutoFill::AUTO_FILL_CANCEL : AutoFill::AUTO_FILL_FAILED;
        SendAutoFillFailed(resultCode, want);
    }
}

void AutoFillExtensionCallback::OnRelease(int32_t errCode)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called, result code: %{public}d", errCode);
    AutoFillManager::GetInstance().RemoveEvent(callbackId_);
    CloseUIExtension();
    if (errCode != 0) {
        SendAutoFillFailed(AutoFill::AUTO_FILL_RELEASE_FAILED);
    }
}

void AutoFillExtensionCallback::OnError(int32_t errCode, const std::string &name, const std::string &message)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called, errcode: %{public}d, name: %{public}s, message: %{public}s",
        errCode, name.c_str(), message.c_str());
    if (name.compare("extension_node_transparent") == 0) {
        return;
    }
    AutoFillManager::GetInstance().RemoveEvent(callbackId_);
    CloseUIExtension();
    if (errCode != 0) {
        SendAutoFillFailed(AutoFill::AUTO_FILL_ON_ERROR);
    }
}

void AutoFillExtensionCallback::HandleReloadInModal(const AAFwk::WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "called");
    SetModalUIExtensionProxy(nullptr);

    auto oldWindowType = autoFillWindowType_;
    auto oldSessionId = sessionId_;
    int32_t resultCode = ReloadInModal(wantParams);
    if (resultCode != AutoFill::AUTO_FILL_SUCCESS) {
        SendAutoFillFailed(resultCode);
    }

    auto uiContent = GetUIContent();
    if (uiContent == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null uiContent");
        return;
    }

    if (oldWindowType == AutoFill::AutoFillWindowType::POPUP_WINDOW) {
        isReloadInModal_ = true;
        uiContent->DestroyCustomPopupUIExtension(oldSessionId);
    } else {
        TAG_LOGW(AAFwkTag::AUTOFILLMGR, "Window type not popup, can not be destroyed");
    }
}

int32_t AutoFillExtensionCallback::ReloadInModal(const AAFwk::WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "called");
    std::lock_guard<std::mutex> lock(closeMutex_);
    if (autoFillWindowType_ != AutoFill::AutoFillWindowType::POPUP_WINDOW) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "not popup window, not reload any more");
        return AutoFill::AUTO_FILL_PREVIOUS_REQUEST_NOT_FINISHED;
    }
    auto uiContent = GetUIContent();
    if (uiContent == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null uiContent");
        return AutoFill::AUTO_FILL_OBJECT_IS_NULL;
    }

    AutoFillManager::GetInstance().SetTimeOutEvent(callbackId_);
    AAFwk::Want want;
    want.SetParam(WANT_PARAMS_AUTO_FILL_CMD_KEY, static_cast<int32_t>(AutoFill::AutoFillCommand::RELOAD_IN_MODAL));
    want.SetParam(WANT_PARAMS_CUSTOM_DATA_KEY, wantParams.GetStringParam(WANT_PARAMS_CUSTOM_DATA_KEY));
    isSmartAutoFill_ ? want.SetParam(WANT_PARAMS_EXTENSION_TYPE_KEY, std::string(WANT_PARAMS_SMART_EXTENSION_TYPE)) :
        want.SetParam(WANT_PARAMS_EXTENSION_TYPE_KEY, std::string(WANT_PARAMS_EXTENSION_TYPE));
    want.SetParam(WANT_PARAMS_AUTO_FILL_TYPE_KEY, static_cast<int32_t>(request_.autoFillType));
    want.SetParam(WANT_PARAMS_VIEW_DATA_KEY, request_.viewData.ToJsonString());
    want.SetParam(WANT_PARAMS_AUTO_FILL_POPUP_WINDOW_KEY, false);
    Ace::ModalUIExtensionCallbacks callback;
    AutoFillManager::GetInstance().BindModalUIExtensionCallback(shared_from_this(), callback);
    Ace::ModalUIExtensionConfig config;
    config.isAsyncModalBinding = true;
    int32_t sessionId = 0;
    sessionId = uiContent->CreateModalUIExtension(want, callback, config);
    if (sessionId == 0) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Create ui extension failed");
        AutoFillManager::GetInstance().RemoveEvent(callbackId_);
        return AutoFill::AUTO_FILL_CREATE_MODULE_UI_EXTENSION_FAILED;
    }
    SetSessionId(sessionId);
    SetWindowType(AutoFill::AutoFillWindowType::MODAL_WINDOW);
    return AutoFill::AUTO_FILL_SUCCESS;
}

void AutoFillExtensionCallback::OnReceive(const AAFwk::WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "called");
    int32_t cmdValue = wantParams.GetIntParam(WANT_PARAMS_AUTO_FILL_CMD_KEY, 0);
    if (cmdValue == static_cast<int32_t>(AutoFill::AutoFillCommand::RELOAD_IN_MODAL)) {
        HandleReloadInModal(wantParams);
        return;
    } else if (cmdValue == static_cast<int32_t>(AutoFill::AutoFillCommand::RESIZE)) {
        UpdateCustomPopupConfig(wantParams);
    }
    if (wantParams.GetIntParam(WANT_PARAMS_AUTO_FILL_EVENT_KEY, 0) == AutoFill::AUTO_FILL_CANCEL_TIME_OUT) {
        AutoFillManager::GetInstance().RemoveEvent(callbackId_);
    }
}

void AutoFillExtensionCallback::UpdateCustomPopupConfig(const AAFwk::WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "called");
    AutoFill::AutoFillCustomConfig autoFillCustomConfig = request_.config;
    if (wantParams.HasParam(WANT_PARAMS_UPDATE_POPUP_WIDTH) &&
        wantParams.HasParam(WANT_PARAMS_UPDATE_POPUP_HEIGHT)) {
        AutoFill::PopupSize popupSize;
        popupSize.width = wantParams.GetIntParam(WANT_PARAMS_UPDATE_POPUP_WIDTH, 0);
        popupSize.height = wantParams.GetIntParam(WANT_PARAMS_UPDATE_POPUP_HEIGHT, 0);
        autoFillCustomConfig.targetSize = popupSize;
    }
    if (wantParams.HasParam(WANT_PARAMS_UPDATE_POPUP_PLACEMENT)) {
        autoFillCustomConfig.placement =
            static_cast<AutoFill::PopupPlacement>(wantParams.GetIntParam(WANT_PARAMS_UPDATE_POPUP_PLACEMENT, 0));
    }
    {
        std::lock_guard<std::mutex> lock(requestCallbackMutex_);
        if (fillCallback_ != nullptr) {
            fillCallback_->onPopupConfigWillUpdate(autoFillCustomConfig);
        }
    }
    Ace::CustomPopupUIExtensionConfig popupConfig;
    AutoFillManagerUtil::ConvertToPopupUIExtensionConfig(autoFillCustomConfig, popupConfig);
    auto uiContent = GetUIContent();
    if (uiContent == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null uiContent");
        return;
    }
    uiContent->UpdateCustomPopupUIExtension(popupConfig);
}

void AutoFillExtensionCallback::onRemoteReady(const std::shared_ptr<Ace::ModalUIExtensionProxy> &modalUIExtensionProxy)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "called");
    if (modalUIExtensionProxy == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null proxy");
        return;
    }
    SetModalUIExtensionProxy(modalUIExtensionProxy);
    if (request_.onUIExtensionProxyReady) {
        request_.onUIExtensionProxyReady();
    }
}

void AutoFillExtensionCallback::onDestroy()
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "called");
    if (isReloadInModal_) {
        isReloadInModal_ = false;
        return;
    }
    if (isOnResult_ && autoFillWindowType_ == AutoFill::AutoFillWindowType::POPUP_WINDOW) {
        isOnResult_ = false;
        if (errCode_ == AutoFill::AUTO_FILL_SUCCESS) {
            SendAutoFillSuccess(want_);
        } else {
            auto resultCode = (errCode_ == AutoFill::AUTO_FILL_CANCEL) ?
                AutoFill::AUTO_FILL_CANCEL : AutoFill::AUTO_FILL_FAILED;
            SendAutoFillFailed(resultCode);
        }
        return;
    }
    CloseUIExtension();
    SendAutoFillFailed(AutoFill::AUTO_FILL_FAILED);
}

void AutoFillExtensionCallback::SetFillRequestCallback(const std::shared_ptr<IFillRequestCallback> &callback)
{
    std::lock_guard<std::mutex> lock(requestCallbackMutex_);
    fillCallback_ = callback;
}

void AutoFillExtensionCallback::SetSaveRequestCallback(const std::shared_ptr<ISaveRequestCallback> &callback)
{
    std::lock_guard<std::mutex> lock(requestCallbackMutex_);
    saveCallback_ = callback;
}

void AutoFillExtensionCallback::SetSessionId(int32_t sessionId)
{
    sessionId_= sessionId;
}

void AutoFillExtensionCallback::SetInstanceId(int32_t instanceId)
{
    instanceId_.store(instanceId);
}

int32_t AutoFillExtensionCallback::GetInstanceId()
{
    return instanceId_.load();
}

Ace::UIContent* AutoFillExtensionCallback::GetUIContent()
{
    return Ace::UIContent::GetUIContent(GetInstanceId());
}

void AutoFillExtensionCallback::SetWindowType(const AutoFill::AutoFillWindowType &autoFillWindowType)
{
    autoFillWindowType_ = autoFillWindowType;
}

AutoFill::AutoFillWindowType AutoFillExtensionCallback::GetWindowType() const
{
    return autoFillWindowType_;
}

void AutoFillExtensionCallback::SetAutoFillRequest(const AutoFill::AutoFillRequest &request)
{
    request_ = request;
}

void AutoFillExtensionCallback::SetExtensionType(bool isSmartAutoFill)
{
    isSmartAutoFill_ = isSmartAutoFill;
}

void AutoFillExtensionCallback::HandleTimeOut()
{
    CloseUIExtension();
    SendAutoFillFailed(AutoFill::AUTO_FILL_REQUEST_TIME_OUT);
}

uint32_t AutoFillExtensionCallback::GenerateCallbackId()
{
    static std::atomic<uint32_t> callbackId(0);
    uint32_t id = callbackId.fetch_add(1) + 1;
    // Handle overflow: if id wraps to 0, return 1 instead
    return (id != 0) ? id : 1;
}

uint32_t AutoFillExtensionCallback::GetCallbackId() const
{
    return callbackId_;
}

void AutoFillExtensionCallback::SetModalUIExtensionProxy(const std::shared_ptr<Ace::ModalUIExtensionProxy>& proxy)
{
    std::lock_guard<std::mutex> lock(proxyMutex_);
    modalUIExtensionProxy_ = proxy;
}

std::shared_ptr<Ace::ModalUIExtensionProxy> AutoFillExtensionCallback::GetModalUIExtensionProxy()
{
    std::lock_guard<std::mutex> lock(proxyMutex_);
    return modalUIExtensionProxy_;
}

void AutoFillExtensionCallback::UpdateCustomPopupUIExtension(const AbilityBase::ViewData &viewData)
{
    auto modalUIExtensionProxy = GetModalUIExtensionProxy();
    if (modalUIExtensionProxy == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null UIExtensionProxy");
        return;
    }
    AAFwk::WantParams wantParams;
    wantParams.SetParam(WANT_PARAMS_AUTO_FILL_CMD_KEY,
        AAFwk::Integer::Box(static_cast<int32_t>(AutoFill::AutoFillCommand::UPDATE)));
    wantParams.SetParam(WANT_PARAMS_VIEW_DATA_KEY, AAFwk::String::Box(viewData.ToJsonString()));
    modalUIExtensionProxy->SendData(wantParams);
}

void AutoFillExtensionCallback::SendAutoFillSuccess(const AAFwk::Want &want)
{
    TAG_LOGI(AAFwkTag::AUTOFILLMGR, "SendAutoFillSuccess");
    std::lock_guard<std::mutex> lock(requestCallbackMutex_);
    if (fillCallback_ != nullptr) {
        std::string dataStr = want.GetStringParam(WANT_PARAMS_VIEW_DATA_KEY);
        AbilityBase::ViewData viewData;
        viewData.FromJsonString(dataStr.c_str());
        fillCallback_->OnFillRequestSuccess(viewData);
        fillCallback_ = nullptr;
    }

    if (saveCallback_ != nullptr) {
        saveCallback_->OnSaveRequestSuccess();
        saveCallback_ = nullptr;
    }
    AutoFillManager::GetInstance().RemoveAutoFillExtensionCallback(callbackId_);
}

void AutoFillExtensionCallback::SendAutoFillFailed(int32_t errCode, const AAFwk::Want &want)
{
    TAG_LOGI(AAFwkTag::AUTOFILLMGR, "SendAutoFillFailed");
    std::lock_guard<std::mutex> lock(requestCallbackMutex_);
    if (fillCallback_ != nullptr) {
        std::string fillContent = want.GetStringParam(WANT_PARAMS_FILL_CONTENT);
        bool isPopup = (autoFillWindowType_ == AutoFill::AutoFillWindowType::POPUP_WINDOW);
        fillCallback_->OnFillRequestFailed(errCode, fillContent, isPopup);
        fillCallback_ = nullptr;
    }

    if (saveCallback_ != nullptr) {
        saveCallback_->OnSaveRequestFailed();
        saveCallback_ = nullptr;
    }
    AutoFillManager::GetInstance().RemoveAutoFillExtensionCallback(callbackId_);
}

void AutoFillExtensionCallback::CloseUIExtension()
{
    TAG_LOGI(AAFwkTag::AUTOFILLMGR, "CloseUIExtension");
    Ace::UIContent* uiContent = nullptr;
    {
        std::lock_guard<std::mutex> lock(closeMutex_);
        uiContent = GetUIContent();
        if (uiContent == nullptr) {
            TAG_LOGD(AAFwkTag::AUTOFILLMGR, "null uiContent");
            return;
        }
        SetInstanceId(-1);
    }

    if (autoFillWindowType_ == AutoFill::AutoFillWindowType::POPUP_WINDOW) {
        uiContent->DestroyCustomPopupUIExtension(sessionId_);
    } else if (autoFillWindowType_ == AutoFill::AutoFillWindowType::MODAL_WINDOW) {
        uiContent->CloseModalUIExtension(sessionId_);
    }
    SetModalUIExtensionProxy(nullptr);
}
#endif // SUPPORT_GRAPHICS
} // namespace AbilityRuntime
} // namespace OHOS
