/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "ui_extension_context.h"

#include "ability_business_error.h"
#include "ability_manager_client.h"
#include "bool_wrapper.h"
#include "configuration.h"
#include "configuration_convertor.h"
#include "connection_manager.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "string_wrapper.h"
#include "ui_content.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t UIExtensionContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("UIExtensionContext"));
int UIExtensionContext::ILLEGAL_REQUEST_CODE(-1);
constexpr const char* REQUEST_COMPONENT_TERMINATE_KEY = "ohos.param.key.requestComponentTerminate";
constexpr const char* UIEXTENSION_TARGET_TYPE_KEY = "ability.want.params.uiExtensionTargetType";
constexpr const char* FLAG_AUTH_READ_URI_PERMISSION = "ability.want.params.uriPermissionFlag";

ErrCode UIExtensionContext::StartAbility(const AAFwk::Want &want) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "begin, ability:%{public}s", want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret = %{public}d", err);
    }
    return err;
}

ErrCode UIExtensionContext::StartAbility(const AAFwk::Want &want, int requestCode) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "begin, requestCode:%{public}d", requestCode);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret = %{public}d", err);
    }
    return err;
}

ErrCode UIExtensionContext::StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "begin, ability:%{public}s", want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_,
        ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret = %{public}d", err);
    }
    return err;
}

ErrCode UIExtensionContext::StartUIAbilitiesInSplitWindowMode(int32_t primaryWindowId,
    const AAFwk::Want &secondaryWant) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartUIAbilitiesInSplitWindowMode(
        primaryWindowId, secondaryWant, token_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret = %{public}d", err);
    }
    return err;
}

ErrCode UIExtensionContext::StartUIServiceExtension(const AAFwk::Want& want, int32_t accountId) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "Start UIServiceExtension begin");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartExtensionAbility(
        want, token_, accountId, AppExecFwk::ExtensionAbilityType::UI_SERVICE);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret = %{public}d", err);
    }
    return err;
}

ErrCode UIExtensionContext::TerminateSelf()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, nullptr);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret = %{public}d", err);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "TerminateSelf end");
    return err;
}

ErrCode UIExtensionContext::ConnectAbility(
    const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin, ability:%{public}s",
        want.GetElement().GetAbilityName().c_str());
    ErrCode ret =
        ConnectionManager::GetInstance().ConnectAbility(token_, want, connectCallback);
    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtensionContext::ConnectAbility ErrorCode = %{public}d", ret);
    return ret;
}

ErrCode UIExtensionContext::ConnectUIServiceExtensionAbility(
    const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin, ability:%{public}s",
        want.GetElement().GetAbilityName().c_str());
    ErrCode ret =
        ConnectionManager::GetInstance().ConnectUIServiceExtensionAbility(token_, want, connectCallback);
    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtensionContext::ConnectUIServiceExtensionAbility ErrorCode = %{public}d", ret);
    return ret;
}

ErrCode UIExtensionContext::DisconnectAbility(
    const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    ErrCode ret =
        ConnectionManager::GetInstance().DisconnectAbility(token_, want, connectCallback);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret = %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
    return ret;
}

ErrCode UIExtensionContext::StartServiceExtensionAbility(const AAFwk::Want& want, int32_t accountId)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "Start service extension %{public}s/%{public}s, accountId: %{public}d",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), accountId);
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->StartExtensionAbility(
        want, token_, accountId, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Start service extension failed %{public}d", ret);
    }
    return ret;
}

ErrCode UIExtensionContext::StartUIAbilities(const std::vector<AAFwk::Want> &wantList, const std::string &requestKey)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "call StartUIAbilities");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartUIAbilities(wantList, requestKey, token_);
    return err;
}

ErrCode UIExtensionContext::StartAbilityForResult(const AAFwk::Want &want, int requestCode, RuntimeTask &&task)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    {
        std::lock_guard<std::mutex> lock(mutexlock_);
        resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    }
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret=%{public}d", err);
        OnAbilityResultInner(requestCode, err, want);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
    return err;
}

void UIExtensionContext::InsertResultCallbackTask(int requestCode, RuntimeTask &&task)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    {
        std::lock_guard<std::mutex> lock(mutexlock_);
        resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    }
}

void UIExtensionContext::RemoveResultCallbackTask(int requestCode)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    {
        std::lock_guard<std::mutex> lock(mutexlock_);
        resultCallbacks_.erase(requestCode);
    }
}

ErrCode UIExtensionContext::StartAbilityForResult(
    const AAFwk::Want &want, const AAFwk::StartOptions &startOptions, int requestCode, RuntimeTask &&task)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    {
        std::lock_guard<std::mutex> lock(mutexlock_);
        resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    }
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_, requestCode);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret=%{public}d", err);
        OnAbilityResultInner(requestCode, err, want);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
    return err;
}

ErrCode UIExtensionContext::StartAbilityForResultAsCaller(const AAFwk::Want &want, int requestCode, RuntimeTask &&task)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    {
        std::lock_guard<std::mutex> lock(mutexlock_);
        resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    }
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbilityForResultAsCaller(want, token_, requestCode);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret = %{public}d", err);
        OnAbilityResultInner(requestCode, err, want);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "End");
    return err;
}

ErrCode UIExtensionContext::StartAbilityForResultAsCaller(
    const AAFwk::Want &want, const AAFwk::StartOptions &startOptions, int requestCode, RuntimeTask &&task)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    {
        std::lock_guard<std::mutex> lock(mutexlock_);
        resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    }
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbilityForResultAsCaller(
        want, startOptions, token_, requestCode);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret = %{public}d", err);
        OnAbilityResultInner(requestCode, err, want);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "End");
    return err;
}

ErrCode UIExtensionContext::ReportDrawnCompleted()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->ReportDrawnCompleted(token_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret=%{public}d", err);
    }
    return err;
}

void UIExtensionContext::OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &resultData)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    std::lock_guard<std::mutex> lock(mutexlock_);
    auto callback = resultCallbacks_.find(requestCode);
    if (callback != resultCallbacks_.end()) {
        if (callback->second) {
            callback->second(resultCode, resultData, false);
        }
        resultCallbacks_.erase(requestCode);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

AppExecFwk::AbilityType UIExtensionContext::GetAbilityInfoType() const
{
    std::shared_ptr<AppExecFwk::AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "null info");
        return AppExecFwk::AbilityType::UNKNOWN;
    }

    return info->type;
}

void UIExtensionContext::OnAbilityResultInner(int requestCode, int resultCode, const AAFwk::Want &resultData)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    std::lock_guard<std::mutex> lock(mutexlock_);
    auto callback = resultCallbacks_.find(requestCode);
    if (callback != resultCallbacks_.end()) {
        if (callback->second) {
            callback->second(resultCode, resultData, true);
        }
        resultCallbacks_.erase(requestCode);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

int32_t UIExtensionContext::GetScreenMode() const
{
    return screenMode_;
}

void UIExtensionContext::SetScreenMode(int32_t screenMode)
{
    screenMode_ = screenMode;
}

int UIExtensionContext::GenerateCurRequestCode()
{
    std::lock_guard lock(requestCodeMutex_);
    curRequestCode_ = (curRequestCode_ == INT_MAX) ? 0 : (curRequestCode_ + 1);
    return curRequestCode_;
}
#ifdef SUPPORT_SCREEN
void UIExtensionContext::SetWindow(sptr<Rosen::Window> window)
{
    window_ = window;
}
sptr<Rosen::Window> UIExtensionContext::GetWindow()
{
    return window_;
}
Ace::UIContent* UIExtensionContext::GetUIContent()
{
    TAG_LOGI(AAFwkTag::UI_EXT, "called");
    if (window_ == nullptr) {
        return nullptr;
    }
    return window_->GetUIContent();
}
#endif // SUPPORT_SCREEN
ErrCode UIExtensionContext::OpenAtomicService(AAFwk::Want& want, const AAFwk::StartOptions &options, int requestCode,
    RuntimeTask &&task)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->OpenAtomicService(want, options, token_, requestCode);
    if (err != ERR_OK && err != AAFwk::START_ABILITY_WAITING) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret = %{public}d", err);
        OnAbilityResultInner(requestCode, err, want);
    }
    return err;
}

ErrCode UIExtensionContext::AddFreeInstallObserver(const sptr<IFreeInstallObserver> &observer)
{
    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->AddFreeInstallObserver(token_, observer);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret = %{public}d", ret);
    }
    return ret;
}

ErrCode UIExtensionContext::OpenLink(const AAFwk::Want &want, int requestCode, bool hideFailureTipDialog)
{
    return AAFwk::AbilityManagerClient::GetInstance()->OpenLink(want, token_, -1, requestCode, hideFailureTipDialog);
}

std::shared_ptr<Global::Resource::ResourceManager> UIExtensionContext::GetResourceManager() const
{
    if (abilityResourceMgr_) {
        return abilityResourceMgr_;
    }

    return ContextImpl::GetResourceManager();
}

void UIExtensionContext::SetAbilityResourceManager(
    std::shared_ptr<Global::Resource::ResourceManager> abilityResourceMgr)
{
    abilityResourceMgr_ = abilityResourceMgr;
}

void UIExtensionContext::RegisterAbilityConfigUpdateCallback(AbilityConfigUpdateCallback abilityConfigUpdateCallback)
{
    abilityConfigUpdateCallback_ = abilityConfigUpdateCallback;
}

std::shared_ptr<AppExecFwk::Configuration> UIExtensionContext::GetAbilityConfiguration() const
{
    return abilityConfiguration_;
}

void UIExtensionContext::SetAbilityConfiguration(const AppExecFwk::Configuration &config)
{
    if (!abilityConfiguration_) {
        abilityConfiguration_ = std::make_shared<AppExecFwk::Configuration>(config);
        TAG_LOGI(AAFwkTag::CONTEXT, "abilityConfiguration: %{public}s", abilityConfiguration_->GetName().c_str());
        return;
    }
    std::vector<std::string> changeKeyV;
    abilityConfiguration_->CompareDifferent(changeKeyV, config);
    if (!changeKeyV.empty()) {
        abilityConfiguration_->Merge(changeKeyV, config);
    }
    TAG_LOGI(AAFwkTag::CONTEXT, "abilityConfiguration: %{public}s", abilityConfiguration_->GetName().c_str());
}

void UIExtensionContext::SetAbilityColorMode(int32_t colorMode)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "SetAbilityColorMode colorMode: %{public}d", colorMode);
    if (colorMode < -1 || colorMode > 1) {
        TAG_LOGE(AAFwkTag::CONTEXT, "colorMode error");
        return;
    }
    AppExecFwk::Configuration config;

    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, AppExecFwk::GetColorModeStr(colorMode));
    config.AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP,
        AppExecFwk::ConfigurationInner::IS_SET_BY_APP);
    if (!abilityConfigUpdateCallback_) {
        TAG_LOGE(AAFwkTag::CONTEXT, "abilityConfigUpdateCallback_ nullptr");
        return;
    }
    abilityConfigUpdateCallback_(config);
}

void UIExtensionContext::RequestComponentTerminate()
{
    TAG_LOGD(AAFwkTag::CONTEXT, "RequestComponentTerminate called");
    if (!window_) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null window_");
        return;
    }
    AAFwk::WantParams params;
    params.SetParam(REQUEST_COMPONENT_TERMINATE_KEY, AAFwk::Boolean::Box(true));
    auto ret = window_->TransferExtensionData(params);
    if (ret != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "transfer extension data failed, ret:%{public}d", ret);
    }
}

bool UIExtensionContext::IsTerminating()
{
    return isTerminating_;
}

void UIExtensionContext::SetTerminating(bool state)
{
    isTerminating_ = state;
}

ErrCode UIExtensionContext::StartAbilityByType(const std::string &type,
    AAFwk::WantParams &wantParam, const std::shared_ptr<JsUIExtensionCallback> &uiExtensionCallbacks)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "StartAbilityByType begin.");
    if (uiExtensionCallbacks == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiExtensionCallbacks");
        return ERR_INVALID_VALUE;
    }
    auto uiContent = GetUIContent();
    if (uiContent == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiContent");
        return ERR_INVALID_VALUE;
    }
    wantParam.SetParam(UIEXTENSION_TARGET_TYPE_KEY, AAFwk::String::Box(type));
    AAFwk::Want want;
    want.SetParams(wantParam);
    if (wantParam.HasParam(FLAG_AUTH_READ_URI_PERMISSION)) {
        int32_t flag = wantParam.GetIntParam(FLAG_AUTH_READ_URI_PERMISSION, 0);
        want.SetFlags(flag);
        wantParam.Remove(FLAG_AUTH_READ_URI_PERMISSION);
    }

    OHOS::Ace::ModalUIExtensionCallbacks callback;
    OHOS::Ace::ModalUIExtensionConfig config;
    callback.onError = [uiExtensionCallbacks](int32_t arg, const std::string &str1, const std::string &str2) {
        uiExtensionCallbacks->OnError(arg);
    };
    callback.onRelease = [uiExtensionCallbacks](int32_t arg) {
        uiExtensionCallbacks->OnRelease(arg);
    };
    callback.onResult = [uiExtensionCallbacks](int32_t arg1, const OHOS::AAFwk::Want arg2) {
        uiExtensionCallbacks->OnResult(arg1, arg2);
    };

    int32_t sessionId = uiContent->CreateModalUIExtension(want, callback, config);
    if (sessionId == 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "createModalUIExtension fail");
        return ERR_INVALID_VALUE;
    }
    uiExtensionCallbacks->SetUIContent(uiContent);
    uiExtensionCallbacks->SetSessionId(sessionId);
    return ERR_OK;
}

ErrCode UIExtensionContext::AddCompletionHandlerForAtomicService(const std::string &requestId,
    OnAtomicRequestSuccess onRequestSucc, OnAtomicRequestFailure onRequestFail, const std::string &appId)
{
    if (onRequestSucc == nullptr || onRequestFail == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "either func is null");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard lock(onRequestResultMutex_);
    for (auto iter = onAtomicRequestResults_.begin(); iter != onAtomicRequestResults_.end(); iter++) {
        if ((*iter)->requestId_ == requestId) {
            TAG_LOGI(AAFwkTag::UI_EXT, "requestId=%{public}s already exists", requestId.c_str());
            return ERR_OK;
        }
    }
    onAtomicRequestResults_.emplace_back(std::make_shared<OnAtomicRequestResult>(
        requestId, appId, onRequestSucc, onRequestFail));
    return ERR_OK;
}

void UIExtensionContext::OnRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element,
    const std::string &message)
{
    std::shared_ptr<OnAtomicRequestResult> atomicResult = nullptr;
    {
        std::lock_guard<std::mutex> lock(onRequestResultMutex_);
        for (auto iter = onAtomicRequestResults_.begin(); iter != onAtomicRequestResults_.end(); iter++) {
            if ((*iter)->requestId_ == requestId) {
                atomicResult = *iter;
                onAtomicRequestResults_.erase(iter);
                break;
            }
        }
    }

    if (atomicResult != nullptr) {
        TAG_LOGI(AAFwkTag::CONTEXT, "requestId=%{public}s, call onRequestSuccess", requestId.c_str());
        atomicResult->onRequestSuccess_(atomicResult->appId_);
        return;
    }

    OnOpenLinkRequestSuccess(requestId, element, message);
}

void UIExtensionContext::OnRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
    const std::string &message, int32_t resultCode)
{
    std::shared_ptr<OnAtomicRequestResult> atomicResult = nullptr;
    {
        std::lock_guard lock(onRequestResultMutex_);
        for (auto iter = onAtomicRequestResults_.begin(); iter != onAtomicRequestResults_.end(); iter++) {
            if ((*iter)->requestId_ == requestId) {
                atomicResult = *iter;
                onAtomicRequestResults_.erase(iter);
                break;
            }
        }
    }

    if (atomicResult != nullptr) {
        TAG_LOGI(AAFwkTag::UI_EXT, "requestId=%{public}s, call onRequestFailure", requestId.c_str());
        int32_t failureCode = 0;
        std::string failureMessage;
        GetFailureInfoByMessage(message, failureCode, failureMessage, resultCode);
        atomicResult->onRequestFailure_(atomicResult->appId_, failureCode, failureMessage);
        return;
    }

    OnOpenLinkRequestFailure(requestId, element, message);
}

ErrCode UIExtensionContext::AddCompletionHandlerForOpenLink(const std::string &requestId,
    OnRequestResult onRequestSucc, OnRequestResult onRequestFail)
{
    if (onRequestSucc == nullptr || onRequestFail == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "either func is null");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard lock(onOpenLinkRequestResultMutex_);
    for (auto iter = onOpenLinkRequestResults_.begin(); iter != onOpenLinkRequestResults_.end(); iter++) {
        if ((*iter)->requestId_ == requestId) {
            TAG_LOGW(AAFwkTag::UI_EXT, "requestId=%{public}s already exists", requestId.c_str());
            return ERR_OK;
        }
    }
    onOpenLinkRequestResults_.emplace_back(std::make_shared<AAFwk::OnOpenLinkRequestResult>(
        requestId, onRequestSucc, onRequestFail));
    return ERR_OK;
}

void UIExtensionContext::OnOpenLinkRequestSuccess(const std::string &requestId,
    const AppExecFwk::ElementName &element, const std::string &message)
{
    std::shared_ptr<AAFwk::OnOpenLinkRequestResult> openLinkResult = nullptr;
    {
        std::lock_guard lock(onOpenLinkRequestResultMutex_);
        for (auto iter = onOpenLinkRequestResults_.begin(); iter != onOpenLinkRequestResults_.end(); iter++) {
            if ((*iter)->requestId_ == requestId) {
                openLinkResult = *iter;
                onOpenLinkRequestResults_.erase(iter);
                break;
            }
        }
    }
    if (openLinkResult != nullptr) {
        TAG_LOGI(AAFwkTag::UI_EXT, "requestId=%{public}s, call onRequestSuccess", requestId.c_str());
        openLinkResult->onRequestSuccess_(element, message);
        return;
    }

    TAG_LOGE(AAFwkTag::UI_EXT, "requestId=%{public}s not exist", requestId.c_str());
}

void UIExtensionContext::OnOpenLinkRequestFailure(const std::string &requestId,
    const AppExecFwk::ElementName &element, const std::string &message)
{
    if (requestId.empty()) {
        return;
    }
    std::shared_ptr<AAFwk::OnOpenLinkRequestResult> openLinkResult = nullptr;
    {
        std::lock_guard lock(onOpenLinkRequestResultMutex_);
        for (auto iter = onOpenLinkRequestResults_.begin(); iter != onOpenLinkRequestResults_.end(); iter++) {
            if ((*iter)->requestId_ == requestId) {
                openLinkResult = *iter;
                onOpenLinkRequestResults_.erase(iter);
                break;
            }
        }
    }
    if (openLinkResult != nullptr) {
        TAG_LOGI(AAFwkTag::UI_EXT, "requestId=%{public}s, call onRequestFailure", requestId.c_str());
        openLinkResult->onRequestFailure_(element, message);
        return;
    }

    TAG_LOGE(AAFwkTag::UI_EXT, "requestId=%{public}s not exist", requestId.c_str());
}

void UIExtensionContext::GetFailureInfoByMessage(
    const std::string &message, int32_t &failureCode, std::string &failureMessage, int32_t resultCode)
{
    if (resultCode == USER_CANCEL) {
        failureCode = static_cast<int32_t>(FailureCode::FAILURE_CODE_USER_CANCEL);
        failureMessage = "The user canceled this startup";
    } else if (message.find("User refused redirection") != std::string::npos) {
        failureCode = static_cast<int32_t>(FailureCode::FAILURE_CODE_USER_REFUSE);
        failureMessage = "User refused redirection";
    } else {
        failureCode = static_cast<int32_t>(FailureCode::FAILURE_CODE_SYSTEM_MALFUNCTION);
        failureMessage = "A system error occurred";
    }
}

int32_t UIExtensionContext::curRequestCode_ = 0;
std::mutex UIExtensionContext::requestCodeMutex_;
}  // namespace AbilityRuntime
}  // namespace OHOS
