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

#include "js_fill_request_callback.h"

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "hilog_tag_wrapper.h"
#include "int_wrapper.h"
#include "ipc_skeleton.h"
#include "js_auto_fill_extension_util.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "napi_common_util.h"
#include "native_engine.h"
#include "native_value.h"
#include "tokenid_kit.h"
#include "ui_content.h"
#include "view_data.h"
#include "want.h"
#include "window.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr const char *WANT_PARAMS_VIEW_DATA = "ohos.ability.params.viewData";
constexpr const char *WANT_PARAMS_AUTO_FILL_CMD = "ohos.ability.params.autoFillCmd";
constexpr const char *WANT_PARAMS_AUTO_FILL_CMD_AUTOFILL = "autofill";
constexpr const char *WANT_PARAMS_UPDATE_POPUP_WIDTH = "ohos.ability.params.popupWidth";
constexpr const char *WANT_PARAMS_UPDATE_POPUP_HEIGHT = "ohos.ability.params.popupHeight";
constexpr const char *WANT_PARAMS_UPDATE_POPUP_PLACEMENT = "ohos.ability.params.popupPlacement";
constexpr const char *CONFIG_POPUP_SIZE = "popupSize";
constexpr const char *CONFIG_POPUP_PLACEMENT = "placement";
constexpr const char *WANT_PARAMS_FILL_CONTENT = "ohos.ability.params.fillContent";
constexpr const char *ERROR_MSG_INVALID_EMPTY = "JsonString is empty.";
constexpr const char *ERROR_MSG_VIEWDATA_INVALID = "The type of storeld, must be ViewData.";
constexpr const char *ERROR_MSG_AUTOFILLPOPUPCONFIG_INVALID =
    "The type of storeld, must be AutoFillPopupConfig.";
constexpr const char *ERROR_MSG_PARAMETER_INVALID =
    "The storeld can consist of only letters, digits, and underscores(_), and cannot exceed 128 characters.";
} // namespace

JsFillRequestCallback::JsFillRequestCallback(
    const sptr<AAFwk::SessionInfo> &sessionInfo, const sptr<Rosen::Window> &uiWindow)
    : sessionInfo_(sessionInfo), uiWindow_(uiWindow)
{}

void JsFillRequestCallback::Finalizer(napi_env env, void* data, void *hint)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    std::unique_ptr<JsFillRequestCallback>(static_cast<JsFillRequestCallback*>(data));
}

napi_value JsFillRequestCallback::FillRequestSuccess(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsFillRequestCallback, OnFillRequestSuccess);
}

napi_value JsFillRequestCallback::FillRequestFailed(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsFillRequestCallback, OnFillRequestFailed);
}

napi_value JsFillRequestCallback::FillRequestCanceled(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsFillRequestCallback, OnFillRequestCanceled);
}

napi_value JsFillRequestCallback::FillRequestAutoFillPopupConfig(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsFillRequestCallback, OnFillRequestAutoFillPopupConfig);
}

napi_value JsFillRequestCallback::OnFillRequestSuccess(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    if (info.argc < ARGC_ONE || !IsTypeForNapiValue(env, info.argv[INDEX_ZERO], napi_object)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "parse viewData failed");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM), ERROR_MSG_VIEWDATA_INVALID);
        SendResultCodeAndViewData(
            JsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_FAILED_INVALID_PARAM, "");
        return CreateJsUndefined(env);
    }

    FillResponse response;
    JsAutoFillExtensionUtil::UnwrapFillResponse(env, info.argv[INDEX_ZERO], response);
    std::string jsonString = response.viewData.ToJsonString();
    if (jsonString.empty()) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "empty jsonString");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        SendResultCodeAndViewData(
            JsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_FAILED_INVALID_PARAM, "");
        return CreateJsUndefined(env);
    }
    
    SendResultCodeAndViewData(JsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_SUCESS, jsonString);
    return CreateJsUndefined(env);
}

napi_value JsFillRequestCallback::OnFillRequestFailed(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    SendResultCodeAndViewData(JsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_FAILED, "");
    return CreateJsUndefined(env);
}

napi_value JsFillRequestCallback::OnFillRequestCanceled(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    if (info.argc < ARGC_ONE) {
        SendResultCodeAndViewData(JsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_CANCEL, "");
        return CreateJsUndefined(env);
    }
    if (!IsTypeForNapiValue(env, info.argv[INDEX_ZERO], napi_string)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "parse fillContent failed");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM), ERROR_MSG_PARAMETER_INVALID);
        SendResultCodeAndViewData(
            JsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_FAILED_INVALID_PARAM, "");
        return CreateJsUndefined(env);
    }
    std::string jsonString = UnwrapStringFromJS(env, info.argv[INDEX_ZERO], "");
    if (jsonString.empty()) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "empty jsonString");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM), ERROR_MSG_INVALID_EMPTY);
        SendResultCodeAndViewData(
            JsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_FAILED_INVALID_PARAM, "");
        return CreateJsUndefined(env);
    }
    SendResultCodeAndViewData(JsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_CANCEL, jsonString);
    return CreateJsUndefined(env);
}

napi_value JsFillRequestCallback::OnFillRequestAutoFillPopupConfig(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Non-system app forbidden to call");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }
    if (uiWindow_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null uiWindow");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::UI_WINDOW_NULL));
        return CreateJsUndefined(env);
    }

    if (info.argc < ARGC_ONE || !IsTypeForNapiValue(env, info.argv[INDEX_ZERO], napi_object)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "parse resize failed");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
            ERROR_MSG_AUTOFILLPOPUPCONFIG_INVALID);
        return CreateJsUndefined(env);
    }
    AAFwk::WantParams wantParams;
    wantParams.SetParam(WANT_PARAMS_AUTO_FILL_CMD, AAFwk::Integer::Box(AutoFillCommand::RESIZE));
    auto isValueChanged = SetPopupConfigToWantParams(env, info, wantParams);
    if (isValueChanged) {
        auto ret = uiWindow_->TransferExtensionData(wantParams);
        if (ret != Rosen::WMError::WM_OK) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Transfer ability result failed");
            ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
                GetInnerErrorMsg(AbilityInnerErrorMsg::TRANSFER_EXTENSION_DATA_FAILED));
        }
    }
    return CreateJsUndefined(env);
}

bool JsFillRequestCallback::SetPopupConfigToWantParams(
    napi_env env, NapiCallbackInfo& info, AAFwk::WantParams& wantParams)
{
    AbilityRuntime::HandleScope handleScope(env);
    napi_value jsValue = nullptr;
    bool isValueChanged = false;
    jsValue = GetPropertyValueByPropertyName(env, info.argv[INDEX_ZERO], CONFIG_POPUP_SIZE, napi_object);
    if (jsValue) {
        PopupSize popupSize;
        JsAutoFillExtensionUtil::UnwrapPopupSize(env, jsValue, popupSize);
        wantParams.SetParam(WANT_PARAMS_UPDATE_POPUP_WIDTH, AAFwk::Integer::Box(popupSize.width));
        wantParams.SetParam(WANT_PARAMS_UPDATE_POPUP_HEIGHT, AAFwk::Integer::Box(popupSize.height));
        isValueChanged = true;
    }

    jsValue = nullptr;
    jsValue = GetPropertyValueByPropertyName(env, info.argv[INDEX_ZERO], CONFIG_POPUP_PLACEMENT, napi_number);
    if (jsValue) {
        int popupPlacement = 0;
        napi_get_value_int32(env, jsValue, &popupPlacement);
        wantParams.SetParam(WANT_PARAMS_UPDATE_POPUP_PLACEMENT, AAFwk::Integer::Box(popupPlacement));
        isValueChanged = true;
    }
    return isValueChanged;
}

void JsFillRequestCallback::SendResultCodeAndViewData(
    const JsAutoFillExtensionUtil::AutoFillResultCode &resultCode, const std::string &jsString)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    if (uiWindow_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null uiWindow");
        return;
    }

    AAFwk::Want want;
    if (resultCode == JsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_SUCESS) {
        want.SetParam(WANT_PARAMS_VIEW_DATA, jsString);
        want.SetParam(WANT_PARAMS_AUTO_FILL_CMD, WANT_PARAMS_AUTO_FILL_CMD_AUTOFILL);
    }

    if (resultCode == JsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_CANCEL) {
        want.SetParam(WANT_PARAMS_FILL_CONTENT, jsString);
    }

    auto ret = uiWindow_->TransferAbilityResult(resultCode, want);
    if (ret != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "TransferAbilityResult failed");
        return;
    }

    auto errorCode = AAFwk::AbilityManagerClient::GetInstance()->TerminateUIExtensionAbility(sessionInfo_);
    if (errorCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "TerminateUIExtensionAbility error: %{public}d", errorCode);
    }
}

napi_value JsFillRequestCallback::CreateJsFillRequestCallback(napi_env env,
    const sptr<AAFwk::SessionInfo> &sessionInfo, const sptr<Rosen::Window> &uiWindow)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    HandleEscape handleEscape(env);
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null object");
        return CreateJsUndefined(env);
    }

    std::unique_ptr<JsFillRequestCallback> jsSession =
        std::make_unique<JsFillRequestCallback>(sessionInfo, uiWindow);
    napi_wrap(env, object, jsSession.release(), Finalizer, nullptr, nullptr);

    const char *moduleName = "JsFillRequestCallback";
    BindNativeFunction(env, object, "onSuccess", moduleName, FillRequestSuccess);
    BindNativeFunction(env, object, "onFailure", moduleName, FillRequestFailed);
    BindNativeFunction(env, object, "onCancel", moduleName, FillRequestCanceled);
    BindNativeFunction(env, object, "setAutoFillPopupConfig", moduleName, FillRequestAutoFillPopupConfig);
    return handleEscape.Escape(object);
}
} // namespace AbilityRuntime
} // namespace OHOS