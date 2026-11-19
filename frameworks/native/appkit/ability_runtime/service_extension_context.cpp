/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "service_extension_context.h"

#include "ability_connection.h"
#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t ServiceExtensionContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("ServiceExtensionContext"));
const std::string START_ABILITY_TYPE = "ABILITY_INNER_START_WITH_ACCOUNT";

int32_t ServiceExtensionContext::ILLEGAL_REQUEST_CODE(-1);

ErrCode ServiceExtensionContext::StartAbility(const AAFwk::Want &want) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPKIT, "ability:%{public}s", want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed %{public}d", err);
    }
    return err;
}

ErrCode ServiceExtensionContext::StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPKIT, "ability:%{public}s", want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_,
        ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed %{public}d", err);
    }
    return err;
}

ErrCode ServiceExtensionContext::StartAbilityAsCaller(const AAFwk::Want &want) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPKIT, "ability:%{public}s",
        want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->
    StartAbilityAsCaller(want, token_, nullptr, ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed %{public}d", err);
    }
    return err;
}

ErrCode ServiceExtensionContext::StartAbilityAsCaller(const AAFwk::Want &want,
    const AAFwk::StartOptions &startOptions) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPKIT, "ability:%{public}s.",
        want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbilityAsCaller(want, startOptions, token_, nullptr,
        ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed %{public}d", err);
    }
    return err;
}

std::shared_ptr<LocalCallContainer> ServiceExtensionContext::GetLocalCallContainer() const
{
    std::lock_guard lock(callContainerMutex_);
    return localCallContainer_;
}

ErrCode ServiceExtensionContext::StartAbilityByCall(
    const AAFwk::Want& want, const std::shared_ptr<CallerCallBack> &callback, int32_t accountId)
{
    TAG_LOGD(AAFwkTag::APPKIT, "begin");
    std::shared_ptr<LocalCallContainer> localCallContainer;
    {
        std::lock_guard lock(callContainerMutex_);
        if (localCallContainer_ == nullptr) {
            localCallContainer_ = std::make_shared<LocalCallContainer>();
        }
        localCallContainer = localCallContainer_;
    }
    return localCallContainer->StartAbilityByCallInner(want, callback, token_, accountId);
}

ErrCode ServiceExtensionContext::ReleaseCall(const std::shared_ptr<CallerCallBack> &callback) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "begin");
    auto localCallContainer = GetLocalCallContainer();
    if (localCallContainer == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null localCallContainer");
        return ERR_INVALID_VALUE;
    }
    return localCallContainer->ReleaseCall(callback);
}

void ServiceExtensionContext::ClearFailedCallConnection(const std::shared_ptr<CallerCallBack> &callback) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "begin");
    auto localCallContainer = GetLocalCallContainer();
    if (localCallContainer == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null localCallContainer");
        return;
    }
    localCallContainer->ClearFailedCallConnection(callback);
}

ErrCode ServiceExtensionContext::ConnectAbility(
    const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const
{
    TAG_LOGI(AAFwkTag::APPKIT, "Connect c:%{public}s, t:%{public}s",
        GetAbilityInfo() == nullptr ? "" : GetAbilityInfo()->name.c_str(), want.GetElement().GetAbilityName().c_str());
    ErrCode ret =
        ConnectionManager::GetInstance().ConnectAbility(token_, want, connectCallback);
    TAG_LOGD(AAFwkTag::APPKIT, "ConnectAbility ErrorCode = %{public}d", ret);
    return ret;
}

ErrCode ServiceExtensionContext::StartAbilityWithAccount(const AAFwk::Want &want, int32_t accountId) const
{
    std::string callerName = "";
    if (GetAbilityInfo() != nullptr) {
        callerName = GetAbilityInfo()->name;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "accountId: %{public}d, ability: %{public}s/%{public}s, caller: %{public}s",
        accountId, want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(),
        callerName.c_str());
    (const_cast<Want &>(want)).SetParam(START_ABILITY_TYPE, true);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(
        want, token_, ILLEGAL_REQUEST_CODE, accountId);
    TAG_LOGD(AAFwkTag::APPKIT, "ret=%{public}d", err);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed %{public}d", err);
    }
    return err;
}

ErrCode ServiceExtensionContext::StartAbilityWithAccount(
    const AAFwk::Want &want, int32_t accountId, const AAFwk::StartOptions &startOptions) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "begin");
    (const_cast<Want &>(want)).SetParam(START_ABILITY_TYPE, true);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_,
        ILLEGAL_REQUEST_CODE, accountId);
    TAG_LOGD(AAFwkTag::APPKIT, "End calling StartAbilityWithAccount. ret=%{public}d", err);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed %{public}d", err);
    }
    return err;
}

ErrCode ServiceExtensionContext::StartUIAbilities(const std::vector<AAFwk::Want> &wantList,
    const std::string &requestKey)
{
    TAG_LOGD(AAFwkTag::APPKIT, "call StartUIAbilities");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartUIAbilities(wantList, requestKey, token_);
    return err;
}

ErrCode ServiceExtensionContext::StartServiceExtensionAbility(const AAFwk::Want &want, int32_t accountId) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "begin");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartExtensionAbility(
        want, token_, accountId, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed %{public}d", err);
    }
    return err;
}

ErrCode ServiceExtensionContext::StartUIServiceExtensionAbility(const AAFwk::Want &want, int32_t accountId) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "begin");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartExtensionAbility(
        want, token_, accountId, AppExecFwk::ExtensionAbilityType::UI_SERVICE);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed %{public}d", err);
    }
    return err;
}

ErrCode ServiceExtensionContext::StopServiceExtensionAbility(const AAFwk::Want& want, int32_t accountId) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "begin");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StopExtensionAbility(
        want, token_, accountId, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed %{public}d", err);
    }
    return err;
}

ErrCode ServiceExtensionContext::ConnectAbilityWithAccount(
    const AAFwk::Want &want, int32_t accountId, const sptr<AbilityConnectCallback> &connectCallback) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "begin");
    ErrCode ret =
        ConnectionManager::GetInstance().ConnectAbilityWithAccount(token_, want, accountId, connectCallback);
    TAG_LOGI(AAFwkTag::APPKIT, "ConAbiWithAcc:%{public}d", ret);
    return ret;
}

ErrCode ServiceExtensionContext::DisconnectAbility(const AAFwk::Want &want,
    const sptr<AbilityConnectCallback> &connectCallback, int32_t accountId) const
{
    TAG_LOGI(AAFwkTag::APPKIT, "DisCon %{public}s,%{public}s",
        GetAbilityInfo() == nullptr ? "" : GetAbilityInfo()->name.c_str(), want.GetElement().GetAbilityName().c_str());
    ErrCode ret =
        ConnectionManager::GetInstance().DisconnectAbility(token_, want, connectCallback, accountId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "ret=%{public}d", ret);
    }
    return ret;
}

ErrCode ServiceExtensionContext::TerminateAbility()
{
    TAG_LOGI(AAFwkTag::APPKIT, "begin");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, nullptr);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed %{public}d", err);
    }
    return err;
}

ErrCode ServiceExtensionContext::RequestModalUIExtension(const Want &want)
{
    TAG_LOGI(AAFwkTag::APPKIT, "begin");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->RequestModalUIExtension(want);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed %{public}d", err);
    }
    return err;
}

AppExecFwk::AbilityType ServiceExtensionContext::GetAbilityInfoType() const
{
    std::shared_ptr<AppExecFwk::AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null info");
        return AppExecFwk::AbilityType::UNKNOWN;
    }

    return info->type;
}

ErrCode ServiceExtensionContext::AddFreeInstallObserver(const sptr<AbilityRuntime::IFreeInstallObserver> &observer)
{
    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->AddFreeInstallObserver(token_, observer);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "ret: %{public}d", ret);
    }
    return ret;
}

ErrCode ServiceExtensionContext::PreStartMission(const std::string& bundleName, const std::string& moduleName,
    const std::string& abilityName, const std::string& startTime)
{
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->PreStartMission(
        bundleName, moduleName, abilityName, startTime);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed %{public}d", err);
    }
    return err;
}

ErrCode ServiceExtensionContext::OpenLink(const AAFwk::Want &want, int requestCode, bool hideFailureTipDialog)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    return AAFwk::AbilityManagerClient::GetInstance()->OpenLink(want, token_, -1, requestCode, hideFailureTipDialog);
}

ErrCode ServiceExtensionContext::OpenAtomicService(const AAFwk::Want &want, const AAFwk::StartOptions &options)
{
    TAG_LOGD(AAFwkTag::APPKIT, "OpenAtomicService called");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->OpenAtomicService(const_cast<AAFwk::Want&>(want),
        options, token_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "OpenAtomicService ret=%{public}d", err);
    }
    return err;
}

ErrCode ServiceExtensionContext::AddCompletionHandlerForAtomicService(const std::string &requestId,
    OnAtomicRequestSuccess onRequestSucc, OnAtomicRequestFailure onRequestFail, const std::string &appId)
{
    if (onRequestSucc == nullptr || onRequestFail == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "either func is null");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard lock(onRequestResultMutex_);
    for (auto iter = onAtomicRequestResults_.begin(); iter != onAtomicRequestResults_.end(); iter++) {
        if ((*iter)->requestId_ == requestId) {
            TAG_LOGI(AAFwkTag::APPKIT, "requestId=%{public}s already exists", requestId.c_str());
            return ERR_OK;
        }
    }
    onAtomicRequestResults_.emplace_back(std::make_shared<OnAtomicRequestResult>(
        requestId, appId, onRequestSucc, onRequestFail));
    return ERR_OK;
}

ErrCode ServiceExtensionContext::AddCompletionHandlerForOpenLink(const std::string &requestId,
    OnRequestResult onRequestSucc, OnRequestResult onRequestFail)
{
    if (onRequestSucc == nullptr || onRequestFail == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "either func is null");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard lock(onOpenLinkRequestResultMutex_);
    for (auto iter = onOpenLinkRequestResults_.begin(); iter != onOpenLinkRequestResults_.end(); iter++) {
        if ((*iter)->requestId_ == requestId) {
            TAG_LOGW(AAFwkTag::APPKIT, "requestId=%{public}s already exists", requestId.c_str());
            return ERR_OK;
        }
    }
    onOpenLinkRequestResults_.emplace_back(std::make_shared<AAFwk::OnOpenLinkRequestResult>(
        requestId, onRequestSucc, onRequestFail));
    return ERR_OK;
}

void ServiceExtensionContext::OnOpenLinkRequestSuccess(const std::string &requestId,
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
        TAG_LOGI(AAFwkTag::APPKIT, "requestId=%{public}s, call onRequestSuccess", requestId.c_str());
        openLinkResult->onRequestSuccess_(element, message);
        return;
    }

    TAG_LOGE(AAFwkTag::APPKIT, "requestId=%{public}s not exist", requestId.c_str());
}

void ServiceExtensionContext::OnOpenLinkRequestFailure(const std::string &requestId,
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
        TAG_LOGI(AAFwkTag::APPKIT, "requestId=%{public}s, call onRequestFailure", requestId.c_str());
        openLinkResult->onRequestFailure_(element, message);
        return;
    }

    TAG_LOGE(AAFwkTag::APPKIT, "requestId=%{public}s not exist", requestId.c_str());
}

void ServiceExtensionContext::OnRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element,
    const std::string &message)
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
        TAG_LOGI(AAFwkTag::APPKIT, "requestId=%{public}s, call onRequestSuccess", requestId.c_str());
        atomicResult->onRequestSuccess_(atomicResult->appId_);
        return;
    }
    OnOpenLinkRequestSuccess(requestId, element, message);
}

void ServiceExtensionContext::OnRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
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
        TAG_LOGI(AAFwkTag::APPKIT, "requestId=%{public}s, call onRequestFailure", requestId.c_str());
        int32_t failureCode = 0;
        std::string failureMessage;
        GetFailureInfoByMessage(message, failureCode, failureMessage, resultCode);
        atomicResult->onRequestFailure_(atomicResult->appId_, failureCode, failureMessage);
        return;
    }

    OnOpenLinkRequestFailure(requestId, element, message);
}

void ServiceExtensionContext::GetFailureInfoByMessage(
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
}  // namespace AbilityRuntime
}  // namespace OHOS
