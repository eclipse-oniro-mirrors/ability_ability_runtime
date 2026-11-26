/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "intent_preprocessor.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ability_manager_errors.h"
#include "accesstoken_kit.h"
#include "array_wrapper.h"
#include "hilog_tag_wrapper.h"
#include "intent_client.h"
#include "ipc_skeleton.h"
#include "want.h"
#include "want_params_wrapper.h"
#ifdef SUPPORT_SCREEN
#include "ui_content.h"
#endif // SUPPORT_SCREEN

namespace OHOS {
namespace AbilityRuntime {
namespace {
// Privacy-center consent dialog (client-side modal UIExtension, sys/commonUI).
constexpr const char* SYSTEM_CONSENT_BUNDLE_NAME = "com.huawei.hmos.security.privacycenter";
constexpr const char* SYSTEM_CONSENT_ABILITY_NAME = "CollaborationAccessAbility";
constexpr const char* UI_EXTENSION_TYPE_KEY = "ability.want.params.uiExtensionType";
constexpr const char* UI_EXTENSION_TYPE_VALUE = "sys/commonUI";
// Want params handed to the consent dialog so it can render "caller X requests
// to invoke capability Y". Keys follow the privacy-center contract and are all
// prefixed with "ohos.hmos.security.privacycenter.".
constexpr const char* CONSENT_PARAM_CALLER_BUNDLE_NAME = "ohos.hmos.security.privacycenter.callerBundleName";
constexpr const char* CONSENT_PARAM_TARGET_BUNDLE_NAME = "ohos.hmos.security.privacycenter.targetBundleNameList";
// App clone index of the caller / target list; invokePublicFunction only targets
// the default (non-cloned) instance, so the single entry is 0.
constexpr const char* CONSENT_PARAM_CALLER_APP_INDEX = "ohos.hmos.security.privacycenter.callerAppIndex";
constexpr const char* CONSENT_PARAM_TARGET_APP_INDEX = "ohos.hmos.security.privacycenter.targetAppIndexList";
// Intent execute record id returned by the server-side pre-check; the consent
// dialog uses it to query the invoking caller bundle via
// getInsightIntentCallerBundleName. The public ArkTS surface types this id as
// int, so the uint64 record id is narrowed once when boxing it into the Want.
constexpr const char* CONSENT_PARAM_INTENT_ID = "ohos.hmos.security.privacycenter.intentId";
// Result key the dialog writes back: a single-element WantParams array whose
// resultCode field carries the final decision (see ParseGrantResult).
constexpr const char* CONSENT_PARAM_GRANT_RESULT = "ohos.hmos.security.privacycenter.grantResult";
// Field inside the grantResult WantParams; 0 = granted, otherwise declined.
constexpr const char* CONSENT_RESULT_CODE = "resultCode";
constexpr int32_t CONSENT_RESULT_GRANTED = 0;
} // namespace

// Extract the grant result from the dialog's returned Want. The privacy center
// writes grantResult as a WantParams array with one element (dump: "W1{{...}}")
// whose resultCode field is a typed int, so plain GetBoolParam always sees the
// default. Fail-closed: anything we cannot parse counts as declined.
bool IntentPreprocessor::ParseGrantResult(const AAFwk::Want &result)
{
    const auto &allParams = result.GetParams().GetParams();
    auto it = allParams.find(CONSENT_PARAM_GRANT_RESULT);
    if (it == allParams.end() || it->second == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "grant result: param missing");
        return false;
    }
    AAFwk::IInterface *raw = it->second.GetRefPtr();
    AAFwk::WantParams grantParams;
    AAFwk::IArray *array = AAFwk::IArray::Query(raw);
    if (array != nullptr) {
        long length = 0;
        if (array->GetLength(length) != ERR_OK || length <= 0) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "grant result: empty array, length=%{public}ld", length);
            return false;
        }
        sptr<AAFwk::IInterface> element;
        if (array->Get(0, element) != ERR_OK || element == nullptr) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "grant result: get element 0 failed");
            return false;
        }
        AAFwk::IWantParams *inner = AAFwk::IWantParams::Query(element.GetRefPtr());
        if (inner == nullptr) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "grant result: element 0 is not WantParams");
            return false;
        }
        grantParams = AAFwk::WantParamWrapper::Unbox(inner);
    } else {
        AAFwk::IWantParams *inner = AAFwk::IWantParams::Query(raw);
        if (inner == nullptr) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "grant result: unexpected param type");
            return false;
        }
        grantParams = AAFwk::WantParamWrapper::Unbox(inner);
    }
    if (grantParams.GetParams().find(CONSENT_RESULT_CODE) == grantParams.GetParams().end()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "grant result: resultCode missing");
        return false;
    }
    int32_t resultCode = grantParams.GetIntParam(CONSENT_RESULT_CODE, -1);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "grant result: resultCode=%{public}d", resultCode);
    return resultCode == CONSENT_RESULT_GRANTED;
}

ErrCode IntentPreprocessor::PreprocessIntentExecute(const std::shared_ptr<AbilityContext> &context,
    const std::string &targetBundleName, const std::string &targetIntentName, DecideCallback onDecided)
{
    if (context == nullptr || onDecided == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "preprocess intent execute: null context or callback");
        return ERR_INVALID_VALUE;
    }
    auto self = std::shared_ptr<IntentPreprocessor>(
        new IntentPreprocessor(context, targetBundleName, targetIntentName, onDecided));
    return self->Mount();
}

IntentPreprocessor::IntentPreprocessor(std::shared_ptr<AbilityContext> context, std::string targetBundleName,
    std::string targetIntentName, DecideCallback onDecided)
    : context_(std::move(context)), targetBundleName_(std::move(targetBundleName)),
      targetIntentName_(std::move(targetIntentName)), onDecided_(std::move(onDecided))
{}

ErrCode IntentPreprocessor::Mount()
{
    int32_t err = AAFwk::IntentClient::GetInstance().PreRegisterIntentRecord(
        targetBundleName_, targetIntentName_, intentId_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "invokePublicFunction pre-check failed: %{public}d", err);
        return err;
    }

    // Automation-control shortcut: skip the consent dialog only on an explicit
    // pre-grant; the decision is settled synchronously and the caller submits
    // the execution with the pre-registered record id.
    if (TryAutomationControlShortcut()) {
        Settle(true);
        return ERR_OK;
    }

    err = ShowConsentDialog();
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "create consent dialog failed: %{public}d", err);
        // The dialog could not be mounted: drop the pre-registered record so it
        // does not linger until eviction.
        AAFwk::IntentClient::GetInstance().RemoveIntentRecord(intentId_);
        intentId_ = 0;
        return err;
    }
    return ERR_OK;
}

bool IntentPreprocessor::TryAutomationControlShortcut() const
{
    Security::AccessToken::AccessTokenID controllerTokenId =
        static_cast<Security::AccessToken::AccessTokenID>(IPCSkeleton::GetSelfTokenID());
    Security::AccessToken::HapTokenInfo callerHapInfo;
    if (Security::AccessToken::AccessTokenKit::GetHapTokenInfo(controllerTokenId, callerHapInfo) !=
        Security::AccessToken::RET_SUCCESS) {
        return false;
    }
    Security::AccessToken::AccessTokenID targetTokenId =
        Security::AccessToken::AccessTokenKit::GetHapTokenID(callerHapInfo.userID, targetBundleName_, 0);
    if (targetTokenId == Security::AccessToken::INVALID_TOKENID) {
        return false;
    }
    std::vector<Security::AccessToken::AccessTokenID> targetList{targetTokenId};
    std::vector<Security::AccessToken::AutomationControlAccessDecision> decisions;
    int32_t checkRet = Security::AccessToken::AccessTokenKit::CheckAutomationControlAccess(
        controllerTokenId, targetList, decisions);
    if (checkRet != Security::AccessToken::RET_SUCCESS || decisions.size() != 1) {
        return false;
    }
    if (decisions[0] != Security::AccessToken::AutomationControlAccessDecision::NO_POPUP_NEEDED_GRANTED) {
        return false;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "invokePublicFunction: automation control pre-granted, skip dialog");
    return true;
}

ErrCode IntentPreprocessor::ShowConsentDialog()
{
#ifdef SUPPORT_SCREEN
    auto uiContent = context_->GetUIContent();
    if (uiContent == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "permission dialog: null uiContent");
        return ERR_INVALID_VALUE;
    }

    AAFwk::Want want;
    want.SetElementName(SYSTEM_CONSENT_BUNDLE_NAME, SYSTEM_CONSENT_ABILITY_NAME);
    want.SetParam(UI_EXTENSION_TYPE_KEY, std::string(UI_EXTENSION_TYPE_VALUE));
    want.SetParam(CONSENT_PARAM_CALLER_BUNDLE_NAME, context_->GetBundleName());
    want.SetParam(CONSENT_PARAM_TARGET_BUNDLE_NAME, std::vector<std::string> { targetBundleName_ });
    want.SetParam(CONSENT_PARAM_CALLER_APP_INDEX, 0);
    want.SetParam(CONSENT_PARAM_TARGET_APP_INDEX, std::vector<int> { 0 });
    want.SetParam(CONSENT_PARAM_INTENT_ID, intentId_);

    auto self = shared_from_this();
    Ace::ModalUIExtensionCallbacks callbacks;
    callbacks.onResult = [self](int32_t code, const AAFwk::Want &result) {
        self->OnResult(code, result);
    };
    callbacks.onError = [self](int32_t code, const std::string &name, const std::string &message) {
        self->OnError(code, name, message);
    };
    callbacks.onRelease = [self](int32_t code) {
        self->OnRelease(code);
    };

    Ace::ModalUIExtensionConfig config;
    config.isProhibitBack = true;

    sessionId_ = uiContent->CreateModalUIExtension(want, callbacks, config);
    if (sessionId_ == 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "permission dialog: create failed");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
#else
    TAG_LOGE(AAFwkTag::CLI_TOOL, "permission dialog: screen not supported");
    return ERR_INVALID_VALUE;
#endif // SUPPORT_SCREEN
}

void IntentPreprocessor::OnResult(int32_t code, const AAFwk::Want &result)
{
    Close();
    Settle(ParseGrantResult(result));
}

void IntentPreprocessor::OnError(int32_t code, const std::string &name, const std::string &message)
{
    TAG_LOGE(AAFwkTag::CLI_TOOL, "permission dialog error: %{public}d, %{public}s, %{public}s", code,
        name.c_str(), message.c_str());
    Close();
    Settle(false);
}

void IntentPreprocessor::OnRelease(int32_t code)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "permission dialog released without result: %{public}d", code);
    Close();
    Settle(false);
}

void IntentPreprocessor::Close()
{
    if (sessionId_ == 0) {
        return;
    }
    int32_t sessionId = sessionId_;
    sessionId_ = 0;
    auto uiContent = context_->GetUIContent();
    if (uiContent != nullptr) {
        uiContent->CloseModalUIExtension(sessionId);
    }
}

void IntentPreprocessor::Settle(bool granted)
{
    if (settled_.exchange(true)) {
        return;
    }
    int32_t decidedId = granted ? intentId_ : 0;
    if (!granted) {
        // Drop the pre-registered record so a declined or dismissed dialog does
        // not linger until eviction.
        AAFwk::IntentClient::GetInstance().RemoveIntentRecord(intentId_);
        intentId_ = 0;
    }
    if (onDecided_) {
        onDecided_(granted, decidedId);
    }
}

} // namespace AbilityRuntime
} // namespace OHOS
