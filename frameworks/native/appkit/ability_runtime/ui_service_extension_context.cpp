/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "ui_service_extension_context.h"

#include <atomic>

#include "ability_business_error.h"
#include "ability_connection.h"
#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include <native_engine/native_engine.h>
#include "ui_content.h"
#include "connection_manager.h"
#include "string_wrapper.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t UIServiceExtensionContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("UIServiceExtensionContext"));
const std::string UIEXTENSION_TARGET_TYPE_KEY = "ability.want.params.uiExtensionTargetType";
const std::string FLAG_AUTH_READ_URI_PERMISSION = "ability.want.params.uriPermissionFlag";

int32_t UIServiceExtensionContext::ILLEGAL_REQUEST_CODE(-1);

ErrCode UIServiceExtensionContext::StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "ability:%{public}s", want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_,
        ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "failed %{public}d", err);
    }
    return err;
}

ErrCode UIServiceExtensionContext::TerminateSelf()
{
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, nullptr);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "failed %{public}d", err);
    }
    return err;
}

void UIServiceExtensionContext::SetWindow(sptr<Rosen::Window> window)
{
    window_ = window;
}

sptr<Rosen::Window> UIServiceExtensionContext::GetWindow()
{
    return window_;
}

Ace::UIContent* UIServiceExtensionContext::GetUIContent()
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "called");
    if (window_ == nullptr) {
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "null window_");
        return nullptr;
    }
    return window_->GetUIContent();
}

ErrCode UIServiceExtensionContext::StartAbilityByType(
    const std::string &type, AAFwk::WantParams &wantParam, std::shared_ptr<UIExtensionCallback> uiExtensionCallback)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "StartAbilityByType begin.");
    if (uiExtensionCallback == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null uiExtensionCallback");
        return ERR_INVALID_VALUE;
    }
    auto uiContent = GetUIContent();
    if (uiContent == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null uiContent");
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

    auto errorFired = std::make_shared<std::atomic<bool>>(false);
    OHOS::Ace::ModalUIExtensionCallbacks callback;
    OHOS::Ace::ModalUIExtensionConfig config;
    callback.onError = [uiExtensionCallback](int32_t arg, const std::string &str1,
        const std::string &str2) {
        uiExtensionCallback->OnError(arg);
    };
    callback.onAbilityErrorCode = [uiExtensionCallback, errorFired](const OHOS::Ace::UIExtensionOperationPhase& phase,
        int32_t errorCode) {
        if (phase != OHOS::Ace::UIExtensionOperationPhase::FOREGROUND && errorCode == 0) {
            return;
        }
        bool expected = false;
        if (!errorFired->compare_exchange_strong(expected, true)) {
            if (errorCode != 0) {
                TAG_LOGW(AAFwkTag::UISERVC_EXT, "error %{public}d dropped, callback already fired", errorCode);
            }
            return;
        }
        uiExtensionCallback->OnAbilityByTypeResult(errorCode);
    };
    callback.onRelease = [uiExtensionCallback, errorFired](int32_t arg) {
        bool expected = false;
        if (errorFired->compare_exchange_strong(expected, true)) {
            uiExtensionCallback->OnAbilityByTypeResult(0);
        }
        uiExtensionCallback->OnRelease(arg);
    };
    callback.onResult = [uiExtensionCallback](int32_t arg1, const OHOS::AAFwk::Want arg2) {
        uiExtensionCallback->OnResult(arg1, arg2);
    };
    callback.onReceive = [uiExtensionCallback](const AAFwk::WantParams& data) {
        if (data.HasParam("onRequestSuccess")) {
            auto successParam = data.GetWantParams("onRequestSuccess");
            auto elementName = successParam.GetStringParam("name");
            uiExtensionCallback->OnRequestSuccess(elementName);
        } else if (data.HasParam("onRequestFailure")) {
            auto failureParam = data.GetWantParams("onRequestFailure");
            auto elementName = failureParam.GetStringParam("name");
            int32_t failureCode = failureParam.GetIntParam("failureCode", 0);
            if (failureCode != 0 && failureCode != 1) {
                TAG_LOGE(AAFwkTag::CONTEXT, "Invalid failureCode: %{public}d", failureCode);
                return;
            }
            std::string failureMsg = failureParam.GetStringParam("failureMessage");
            uiExtensionCallback->OnRequestFailure(elementName, failureCode, failureMsg);
        }
    };
    int32_t sessionId = uiContent->CreateModalUIExtension(want, callback, config);
    if (sessionId == 0) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "sessionId zero");
        return ERR_INVALID_VALUE;
    }
    uiExtensionCallback->SetUIContent(uiContent);
    uiExtensionCallback->SetSessionId(sessionId);
    return ERR_OK;
}

ErrCode UIServiceExtensionContext::ConnectServiceExtensionAbility(
    const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const
{
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "Connect ability begin, ability:%{public}s.",
            want.GetElement().GetAbilityName().c_str());
        ErrCode ret =
            ConnectionManager::GetInstance().ConnectAbility(token_, want, connectCallback);
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "ServiceExtensionContext::ConnectAbility ErrorCode = %{public}d", ret);
        return ret;
}

ErrCode UIServiceExtensionContext::DisConnectServiceExtensionAbility(const AAFwk::Want &want,
    const sptr<AbilityConnectCallback> &connectCallback, int32_t accountId) const
{
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "begin.");
        ErrCode ret =
            ConnectionManager::GetInstance().DisconnectAbility(token_, want, connectCallback, accountId);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "ret=%{public}d", ret);
        }
        TAG_LOGI(AAFwkTag::UISERVC_EXT, "end");
        return ret;
}

}  // namespace AbilityRuntime
}  // namespace OHOS
