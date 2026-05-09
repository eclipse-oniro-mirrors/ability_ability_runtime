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

#include "js_embeddable_ui_ability_context.h"

#include <chrono>
#include <cstdint>
#include <string>

#include "ability_manager_client.h"
#include "event_handler.h"
#include "hilog_tag_wrapper.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_ability.h"
#include "napi_common_want.h"
#include "napi_common_util.h"
#include "napi_common_start_options.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
#define CHECK_POINTER_RETURN(env, object)            \
    if (!(object)) {                                 \
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");           \
        return CreateJsUndefined(env);               \
    }

namespace {
const std::string ERR_MSG_NOT_SUPPORT = "Not support the interface in embedded screen mode of atomic service.";

bool IsEmbeddableStart(int32_t screenMode)
{
    return screenMode == AAFwk::EMBEDDED_FULL_SCREEN_MODE ||
        screenMode == AAFwk::EMBEDDED_HALF_SCREEN_MODE;
}
}

JsEmbeddableUIAbilityContext::JsEmbeddableUIAbilityContext(const std::shared_ptr<AbilityContext>& uiAbiContext,
    const std::shared_ptr<UIExtensionContext>& uiExtContext, int32_t screenMode)
{
    jsAbilityContext_ = std::make_shared<JsAbilityContext>(uiAbiContext);
    jsUIExtensionContext_ = std::make_shared<JsUIExtensionContext>(uiExtContext);
    screenMode_ = screenMode;
}

void JsEmbeddableUIAbilityContext::Finalizer(napi_env env, void* data, void* hint)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    std::unique_ptr<JsEmbeddableUIAbilityContext>(static_cast<JsEmbeddableUIAbilityContext*>(data));
}

napi_value JsEmbeddableUIAbilityContext::StartAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbility);
}

napi_value JsEmbeddableUIAbilityContext::OpenLink(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnOpenLink);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityForResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityForResult);
}

napi_value JsEmbeddableUIAbilityContext::ConnectAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnConnectAbility);
}

napi_value JsEmbeddableUIAbilityContext::DisconnectAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnDisconnectAbility);
}

napi_value JsEmbeddableUIAbilityContext::TerminateSelf(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnTerminateSelf);
}

napi_value JsEmbeddableUIAbilityContext::TerminateSelfWithResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnTerminateSelfWithResult);
}

napi_value JsEmbeddableUIAbilityContext::BackToCallerAbilityWithResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnBackToCallerAbilityWithResult);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityAsCaller(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityAsCaller);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityWithAccount);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityByCall(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityByCall);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityForResultWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityForResultWithAccount);
}

napi_value JsEmbeddableUIAbilityContext::StartServiceExtensionAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartExtensionAbility);
}

napi_value JsEmbeddableUIAbilityContext::StartServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartExtensionAbilityWithAccount);
}

napi_value JsEmbeddableUIAbilityContext::StopServiceExtensionAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStopExtensionAbility);
}

napi_value JsEmbeddableUIAbilityContext::StopServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStopExtensionAbilityWithAccount);
}

napi_value JsEmbeddableUIAbilityContext::ConnectAbilityWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnConnectAbilityWithAccount);
}

napi_value JsEmbeddableUIAbilityContext::RestoreWindowStage(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnRestoreWindowStage);
}

napi_value JsEmbeddableUIAbilityContext::IsTerminating(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnIsTerminating);
}

napi_value JsEmbeddableUIAbilityContext::StartRecentAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartRecentAbility);
}

napi_value JsEmbeddableUIAbilityContext::RequestDialogService(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnRequestDialogService);
}

napi_value JsEmbeddableUIAbilityContext::ReportDrawnCompleted(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnReportDrawnCompleted);
}

napi_value JsEmbeddableUIAbilityContext::SetMissionContinueState(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnSetMissionContinueState);
}

napi_value JsEmbeddableUIAbilityContext::StartAbilityByType(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartAbilityByType);
}

napi_value JsEmbeddableUIAbilityContext::MoveAbilityToBackground(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnMoveAbilityToBackground);
}

napi_value JsEmbeddableUIAbilityContext::RequestModalUIExtension(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnRequestModalUIExtension);
}

napi_value JsEmbeddableUIAbilityContext::RequestModalUIExtensionWithAccount(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnRequestModalUIExtensionWithAccount);
}

napi_value JsEmbeddableUIAbilityContext::OpenAtomicService(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnOpenAtomicService);
}

napi_value JsEmbeddableUIAbilityContext::ShowAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnShowAbility);
}

napi_value JsEmbeddableUIAbilityContext::HideAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnHideAbility);
}

napi_value JsEmbeddableUIAbilityContext::SetRestoreEnabled(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnSetRestoreEnabled);
}

napi_value JsEmbeddableUIAbilityContext::SetColorMode(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnSetColorMode);
}

napi_value JsEmbeddableUIAbilityContext::StartUIServiceExtension(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnStartUIServiceExtension);
}

napi_value JsEmbeddableUIAbilityContext::ConnectUIServiceExtension(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnConnectUIServiceExtension);
}

napi_value JsEmbeddableUIAbilityContext::RestartAppWithWindow(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnRestartAppWithWindow);
}

napi_value JsEmbeddableUIAbilityContext::DisconnectUIServiceExtension(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnDisconnectUIServiceExtension);
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbility(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGI(AAFwkTag::UI_EXT, "Start ability in embedded screen mode");
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnStartAbility(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStartAbility(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnOpenLink(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGI(AAFwkTag::UI_EXT, "Start openLink in embedded screen mode");
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnOpenLink(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnOpenLink(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbilityForResult(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGI(AAFwkTag::UI_EXT, "Start ability for result in embedded screen mode");
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnStartAbilityForResult(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStartAbilityForResult(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnConnectAbility(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGI(AAFwkTag::UI_EXT, "Connect ability in embedded screen mode");
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnConnectAbility(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnConnectAbility(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnDisconnectAbility(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGI(AAFwkTag::UI_EXT, "Disconnect ability in embedded screen mode");
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnDisconnectAbility(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnDisconnectAbility(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnTerminateSelf(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGI(AAFwkTag::UI_EXT, "TerminateSelf in embedded screen mode");
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnTerminateSelf(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnTerminateSelf(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnTerminateSelfWithResult(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGI(AAFwkTag::UI_EXT, "TerminateSelfWithResult ability in embedded screen mode");
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnTerminateSelfWithResult(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnTerminateSelfWithResult(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnBackToCallerAbilityWithResult(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "BackToCallerAbilityWithResult in embedded screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnBackToCallerAbilityWithResult(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbilityAsCaller(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Start ability as caller in embedded screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStartAbilityAsCaller(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Start ability with account in embedded screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStartAbilityWithAccount(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbilityByCall(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Start ability by caller in embedded screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStartAbilityByCall(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbilityForResultWithAccount(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Start ability for result in embedded screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStartAbilityForResultWithAccount(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStartExtensionAbility(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Start extension in embedded screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStartExtensionAbility(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStartExtensionAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Start extension with account in embedded screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStartExtensionAbilityWithAccount(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStopExtensionAbility(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Stop extension in embedded screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStopExtensionAbility(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStopExtensionAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Stop extension with account in embedded screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStopExtensionAbilityWithAccount(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnConnectAbilityWithAccount(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Connect ability with account in embedded screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnConnectAbilityWithAccount(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnRestoreWindowStage(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Restore window stage with account in embedded screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnRestoreWindowStage(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnIsTerminating(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnIsTerminating(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnIsTerminating(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStartRecentAbility(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Start recent ability in embedded screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStartRecentAbility(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnRequestDialogService(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Request dialog service in embedded screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnRequestDialogService(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnReportDrawnCompleted(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGI(AAFwkTag::UI_EXT, "Report Drawn Completed in half screen mode");
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnReportDrawnCompleted(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnReportDrawnCompleted(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnSetMissionContinueState(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Set mission continue state in embedded screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnSetMissionContinueState(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStartAbilityByType(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnStartAbilityByType(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStartAbilityByType(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnMoveAbilityToBackground(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "OnMoveAbilityToBackground in half screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnMoveAbilityToBackground(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnRequestModalUIExtension(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "OnRequestModalUIExtension in half screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnRequestModalUIExtension(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnRequestModalUIExtensionWithAccount(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "OnRequestModalUIExtensionWithAccount in half screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnRequestModalUIExtensionWithAccount(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnOpenAtomicService(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGI(AAFwkTag::UI_EXT, "OpenAtomicService in embedded screen mode");
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnOpenAtomicService(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnOpenAtomicService(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnShowAbility(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "OnShowAbility in half screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnShowAbility(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnHideAbility(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "OnHideAbility in half screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnHideAbility(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnSetRestoreEnabled(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "OnSetRestoreEnabled in half screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnSetRestoreEnabled(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnSetColorMode(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnSetColorMode(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnSetColorMode(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnStartUIServiceExtension(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnStartUIServiceExtension(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnStartUIServiceExtension(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnConnectUIServiceExtension(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnConnectUIServiceExtension(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnConnectUIServiceExtension(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnDisconnectUIServiceExtension(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        CHECK_POINTER_RETURN(env, jsUIExtensionContext_);
        return jsUIExtensionContext_->OnDisconnectUIServiceExtension(env, info);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnDisconnectUIServiceExtension(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnRestartAppWithWindow(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "OnRestartAppWithWindow in half screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnRestartAppWithWindow(env, info);
}

#ifdef SUPPORT_GRAPHICS
napi_value JsEmbeddableUIAbilityContext::SetMissionLabel(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "called");
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnSetMissionLabel);
}

napi_value JsEmbeddableUIAbilityContext::SetMissionIcon(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "called");
    GET_NAPI_INFO_AND_CALL(env, info, JsEmbeddableUIAbilityContext, OnSetMissionIcon);
}

napi_value JsEmbeddableUIAbilityContext::OnSetMissionLabel(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Set mission label in embedded screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnSetMissionLabel(env, info);
}

napi_value JsEmbeddableUIAbilityContext::OnSetMissionIcon(napi_env env, NapiCallbackInfo& info)
{
    if (IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Set mission icon in embedded screen mode");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), ERR_MSG_NOT_SUPPORT);
        return CreateJsUndefined(env);
    }
    CHECK_POINTER_RETURN(env, jsAbilityContext_);
    return jsAbilityContext_->OnSetMissionIcon(env, info);
}
#endif

void JsEmbeddableUIAbilityContext::WrapJsUIAbilityContext(napi_env env,
    std::shared_ptr<AbilityContext> uiAbiContext, napi_value &objValue, int32_t screenMode)
{
    if (uiAbiContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiAbiContext");
        return;
    }
    HandleScope handleScope(env);
    objValue = CreateJsBaseContext(env, uiAbiContext);
    std::unique_ptr<JsEmbeddableUIAbilityContext> jsContext = std::make_unique<JsEmbeddableUIAbilityContext>(
        uiAbiContext, nullptr, screenMode);
    napi_wrap(env, objValue, jsContext.release(), Finalizer, nullptr, nullptr);

    auto abilityInfo = uiAbiContext->GetAbilityInfo();
    if (abilityInfo != nullptr) {
        napi_set_named_property(env, objValue, "abilityInfo", CreateJsAbilityInfo(env, *abilityInfo));
    }

    auto configuration = uiAbiContext->GetConfiguration();
    if (configuration != nullptr) {
        napi_set_named_property(env, objValue, "config", CreateJsConfiguration(env, *configuration));
    }
}

void JsEmbeddableUIAbilityContext::WrapJsUIExtensionContext(napi_env env,
    std::shared_ptr<UIExtensionContext> uiExtContext, napi_value &objValue, int32_t screenMode)
{
    if (uiExtContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiExtContext");
        return;
    }
    HandleScope handleScope(env);
    objValue = CreateJsBaseContext(env, uiExtContext);
    std::unique_ptr<JsEmbeddableUIAbilityContext> jsContext = std::make_unique<JsEmbeddableUIAbilityContext>(
        nullptr, uiExtContext, screenMode);
    napi_wrap(env, objValue, jsContext.release(), Finalizer, nullptr, nullptr);

    auto abilityInfo = uiExtContext->GetAbilityInfo();
    if (abilityInfo != nullptr) {
        napi_set_named_property(env, objValue, "abilityInfo", CreateJsAbilityInfo(env, *abilityInfo));
    }

    auto configuration = uiExtContext->GetConfiguration();
    if (configuration != nullptr) {
        napi_set_named_property(env, objValue, "config", CreateJsConfiguration(env, *configuration));
    }
}

napi_value JsEmbeddableUIAbilityContext::CreateJsEmbeddableUIAbilityContext(napi_env env,
    std::shared_ptr<AbilityContext> uiAbiContext, std::shared_ptr<UIExtensionContext> uiExtContext, int32_t screenMode)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    HandleEscape handleEscape(env);
    napi_value objValue = nullptr;
    if (screenMode == AAFwk::JUMP_SCREEN_MODE) {
        WrapJsUIAbilityContext(env, uiAbiContext, objValue, screenMode);
    } else if (IsEmbeddableStart(screenMode)) {
        WrapJsUIExtensionContext(env, uiExtContext, objValue, screenMode);
    }

    const char* moduleName = "JsEmbeddableUIAbilityContext";
    BindNativeFunction(env, objValue, "startAbility", moduleName, StartAbility);
    BindNativeFunction(env, objValue, "openLink", moduleName, OpenLink);
    BindNativeFunction(env, objValue, "startAbilityForResult", moduleName, StartAbilityForResult);
    BindNativeFunction(env, objValue, "connectServiceExtensionAbility", moduleName, ConnectAbility);
    BindNativeFunction(env, objValue, "disconnectServiceExtensionAbility", moduleName, DisconnectAbility);
    BindNativeFunction(env, objValue, "terminateSelf", moduleName, TerminateSelf);
    BindNativeFunction(env, objValue, "terminateSelfWithResult", moduleName, TerminateSelfWithResult);
    BindNativeFunction(env, objValue, "backToCallerAbilityWithResult", moduleName, BackToCallerAbilityWithResult);
    BindNativeFunction(env, objValue, "startAbilityAsCaller", moduleName, StartAbilityAsCaller);
    BindNativeFunction(env, objValue, "startAbilityWithAccount", moduleName, StartAbilityWithAccount);
    BindNativeFunction(env, objValue, "startAbilityByCall", moduleName, StartAbilityByCall);
    BindNativeFunction(env, objValue, "startAbilityForResultWithAccount", moduleName,
        StartAbilityForResultWithAccount);
    BindNativeFunction(env, objValue, "startServiceExtensionAbility", moduleName, StartServiceExtensionAbility);
    BindNativeFunction(env, objValue, "startServiceExtensionAbilityWithAccount", moduleName,
        StartServiceExtensionAbilityWithAccount);
    BindNativeFunction(env, objValue, "stopServiceExtensionAbility", moduleName, StopServiceExtensionAbility);
    BindNativeFunction(env, objValue, "stopServiceExtensionAbilityWithAccount", moduleName,
        StopServiceExtensionAbilityWithAccount);
    BindNativeFunction(env, objValue, "connectServiceExtensionAbilityWithAccount", moduleName,
        ConnectAbilityWithAccount);
    BindNativeFunction(env, objValue, "restoreWindowStage", moduleName, RestoreWindowStage);
    BindNativeFunction(env, objValue, "isTerminating", moduleName, IsTerminating);
    BindNativeFunction(env, objValue, "startRecentAbility", moduleName, StartRecentAbility);
    BindNativeFunction(env, objValue, "requestDialogService", moduleName, RequestDialogService);
    BindNativeFunction(env, objValue, "reportDrawnCompleted", moduleName, ReportDrawnCompleted);
    BindNativeFunction(env, objValue, "setMissionContinueState", moduleName, SetMissionContinueState);
    BindNativeFunction(env, objValue, "startAbilityByType", moduleName, StartAbilityByType);
    BindNativeFunction(env, objValue, "requestModalUIExtension", moduleName, RequestModalUIExtension);
    BindNativeFunction(env, objValue, "requestModalUIExtensionWithAccount", moduleName,
        RequestModalUIExtensionWithAccount);
    BindNativeFunction(env, objValue, "openAtomicService", moduleName, OpenAtomicService);
    BindNativeFunction(env, objValue, "showAbility", moduleName, ShowAbility);
    BindNativeFunction(env, objValue, "hideAbility", moduleName, HideAbility);
    BindNativeFunction(env, objValue, "setRestoreEnabled", moduleName, SetRestoreEnabled);
    BindNativeFunction(env, objValue, "setColorMode", moduleName, SetColorMode);
    BindNativeFunction(env, objValue, "startUIServiceExtensionAbility", moduleName, StartUIServiceExtension);
    BindNativeFunction(env, objValue, "connectUIServiceExtensionAbility", moduleName, ConnectUIServiceExtension);
    BindNativeFunction(env, objValue, "disconnectUIServiceExtensionAbility", moduleName, DisconnectUIServiceExtension);
    BindNativeFunction(env, objValue, "restartApp", moduleName, RestartAppWithWindow);

#ifdef SUPPORT_GRAPHICS
    BindNativeFunction(env, objValue, "setMissionLabel", moduleName, SetMissionLabel);
    BindNativeFunction(env, objValue, "setMissionIcon", moduleName, SetMissionIcon);
    BindNativeFunction(env, objValue, "moveAbilityToBackground", moduleName, MoveAbilityToBackground);
#endif
    return handleEscape.Escape(objValue);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
