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

#include "js_auto_fill_manager.h"

#include "ability_business_error.h"
#include "auto_fill_manager.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "js_error_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr size_t ARGC_ONE = 1;
} // namespace

void JsAutoFillManager::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "called");
    std::unique_ptr<JsAutoFillManager>(static_cast<JsAutoFillManager *>(data));
}

napi_value JsAutoFillManager::RequestAutoSave(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAutoFillManager, OnRequestAutoSave);
}

napi_value JsAutoFillManager::OnRequestAutoSave(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "called");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    napi_value instanceIdValue = nullptr;
    if (napi_get_named_property(env, info.argv[INDEX_ZERO], "instanceId_", &instanceIdValue) != napi_ok) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "get function by name failed");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
            "Parameter error. Get instance id failed.");
        return CreateJsUndefined(env);
    }
    int32_t instanceId = -1;
    if (!ConvertFromJsValue(env, instanceIdValue, instanceId)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Failed to parse type.");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
            "Parameter error. Parse instance id failed.");
        return CreateJsUndefined(env);
    }

    auto saveCallback = GetCallbackByInstanceId(instanceId);
    if (saveCallback != nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "There are other requests in progress.");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    auto autoSaveMangerFunc = [this](const int32_t arg) { this->OnRequestAutoSaveDone(arg); };
    saveCallback = std::make_shared<JsAutoSaveRequestCallback>(env, instanceId, autoSaveMangerFunc);
    if (saveCallback == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null saveCallback");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    if (info.argc != ARGC_ONE) {
        if (!CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
            TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Second input parameter error.");
            ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
                "Parameter error. The second parameter is not of type callback.");
            return CreateJsUndefined(env);
        }
        saveCallback->Register(info.argv[INDEX_ONE]);
    }
    OnRequestAutoSaveInner(env, instanceId, saveCallback);
    return CreateJsUndefined(env);
}

void JsAutoFillManager::OnRequestAutoSaveInner(napi_env env, int32_t instanceId,
    const std::shared_ptr<JsAutoSaveRequestCallback> &saveRequestCallback)
{
#ifdef SUPPORT_GRAPHICS
    auto uiContent = Ace::UIContent::GetUIContent(instanceId);
    if (uiContent == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null uiContent");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    if (uiContent->CheckNeedAutoSave()) {
        AutoFill::AutoFillRequest request;
        uiContent->DumpViewData(request.viewData, request.autoFillType);
        request.autoFillCommand = AutoFill::AutoFillCommand::SAVE;
        AbilityRuntime::AutoFill::AutoFillResult result;
        auto ret = AutoFillManager::GetInstance().RequestAutoSave(uiContent, request, saveRequestCallback, result);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Request auto save error[%{public}d].", ret);
            ThrowError(env, GetJsErrorCodeByNativeError(ret));
            return;
        }
        std::lock_guard<std::mutex> lock(mutexLock_);
        saveRequestObject_.emplace(instanceId, saveRequestCallback);
    }
#endif // SUPPORT_GRAPHICS
}

std::shared_ptr<JsAutoSaveRequestCallback> JsAutoFillManager::GetCallbackByInstanceId(int32_t instanceId)
{
    std::lock_guard<std::mutex> lock(mutexLock_);
    auto iter = saveRequestObject_.find(instanceId);
    if (iter != saveRequestObject_.end()) {
        return iter->second.lock();
    }
    return nullptr;
}

void JsAutoFillManager::OnRequestAutoSaveDone(int32_t instanceId)
{
    std::lock_guard<std::mutex> lock(mutexLock_);
    auto iter = saveRequestObject_.find(instanceId);
    if (iter != saveRequestObject_.end()) {
        saveRequestObject_.erase(iter);
    }
}

napi_value CreateJsAutoFillType(napi_env env)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "called");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    napi_set_named_property(env, objValue, "UNSPECIFIED", CreateJsValue(env, AbilityBase::AutoFillType::UNSPECIFIED));
    napi_set_named_property(env, objValue, "PASSWORD", CreateJsValue(env, AbilityBase::AutoFillType::PASSWORD));
    napi_set_named_property(env, objValue, "USER_NAME", CreateJsValue(env, AbilityBase::AutoFillType::USER_NAME));
    napi_set_named_property(env, objValue, "NEW_PASSWORD", CreateJsValue(env, AbilityBase::AutoFillType::NEW_PASSWORD));
    napi_set_named_property(env, objValue, "FULL_STREET_ADDRESS",
        CreateJsValue(env, AbilityBase::AutoFillType::FULL_STREET_ADDRESS));
    napi_set_named_property(env, objValue, "HOUSE_NUMBER", CreateJsValue(env, AbilityBase::AutoFillType::HOUSE_NUMBER));
    napi_set_named_property(env, objValue, "DISTRICT_ADDRESS",
        CreateJsValue(env, AbilityBase::AutoFillType::DISTRICT_ADDRESS));
    napi_set_named_property(env, objValue, "CITY_ADDRESS", CreateJsValue(env, AbilityBase::AutoFillType::CITY_ADDRESS));
    napi_set_named_property(env, objValue, "PROVINCE_ADDRESS",
        CreateJsValue(env, AbilityBase::AutoFillType::PROVINCE_ADDRESS));
    napi_set_named_property(env, objValue, "COUNTRY_ADDRESS",
        CreateJsValue(env, AbilityBase::AutoFillType::COUNTRY_ADDRESS));
    napi_set_named_property(env, objValue, "PERSON_FULL_NAME",
        CreateJsValue(env, AbilityBase::AutoFillType::PERSON_FULL_NAME));
    napi_set_named_property(env, objValue, "PERSON_LAST_NAME",
        CreateJsValue(env, AbilityBase::AutoFillType::PERSON_LAST_NAME));
    napi_set_named_property(env, objValue, "PERSON_FIRST_NAME",
        CreateJsValue(env, AbilityBase::AutoFillType::PERSON_FIRST_NAME));
    napi_set_named_property(env, objValue, "PHONE_NUMBER", CreateJsValue(env, AbilityBase::AutoFillType::PHONE_NUMBER));
    napi_set_named_property(env, objValue, "PHONE_COUNTRY_CODE",
        CreateJsValue(env, AbilityBase::AutoFillType::PHONE_COUNTRY_CODE));
    napi_set_named_property(env, objValue, "FULL_PHONE_NUMBER",
        CreateJsValue(env, AbilityBase::AutoFillType::FULL_PHONE_NUMBER));
    napi_set_named_property(env, objValue, "EMAIL_ADDRESS",
        CreateJsValue(env, AbilityBase::AutoFillType::EMAIL_ADDRESS));
    napi_set_named_property(env, objValue, "BANK_CARD_NUMBER",
        CreateJsValue(env, AbilityBase::AutoFillType::BANK_CARD_NUMBER));
    napi_set_named_property(env, objValue, "ID_CARD_NUMBER",
        CreateJsValue(env, AbilityBase::AutoFillType::ID_CARD_NUMBER));
    napi_set_named_property(env, objValue, "PRECISE_TIME", CreateJsValue(env, AbilityBase::AutoFillType::PRECISE_TIME));
    napi_set_named_property(env, objValue, "HOUR_AND_MINUTE",
        CreateJsValue(env, AbilityBase::AutoFillType::HOUR_AND_MINUTE));
    napi_set_named_property(env, objValue, "DATE", CreateJsValue(env, AbilityBase::AutoFillType::DATE));
    napi_set_named_property(env, objValue, "MONTH", CreateJsValue(env, AbilityBase::AutoFillType::MONTH));
    napi_set_named_property(env, objValue, "YEAR", CreateJsValue(env, AbilityBase::AutoFillType::YEAR));
    napi_set_named_property(env, objValue, "NICKNAME", CreateJsValue(env, AbilityBase::AutoFillType::NICKNAME));
    napi_set_named_property(env, objValue, "DETAIL_INFO_WITHOUT_STREET",
        CreateJsValue(env, AbilityBase::AutoFillType::DETAIL_INFO_WITHOUT_STREET));
    napi_set_named_property(env, objValue, "FORMAT_ADDRESS",
        CreateJsValue(env, AbilityBase::AutoFillType::FORMAT_ADDRESS));
    return objValue;
}

napi_value CreateJsPopupPlacement(napi_env env)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "called");
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    napi_set_named_property(env, objValue, "LEFT", CreateJsValue(env, AbilityBase::PopupPlacement::LEFT));
    napi_set_named_property(env, objValue, "RIGHT", CreateJsValue(env, AbilityBase::PopupPlacement::RIGHT));
    napi_set_named_property(env, objValue, "TOP", CreateJsValue(env, AbilityBase::PopupPlacement::TOP));
    napi_set_named_property(env, objValue, "BOTTOM", CreateJsValue(env, AbilityBase::PopupPlacement::BOTTOM));
    napi_set_named_property(env, objValue, "TOP_LEFT", CreateJsValue(env, AbilityBase::PopupPlacement::TOP_LEFT));
    napi_set_named_property(env, objValue, "TOP_RIGHT", CreateJsValue(env, AbilityBase::PopupPlacement::TOP_RIGHT));
    napi_set_named_property(env, objValue, "BOTTOM_LEFT", CreateJsValue(env, AbilityBase::PopupPlacement::BOTTOM_LEFT));
    napi_set_named_property(env, objValue, "BOTTOM_RIGHT",
        CreateJsValue(env, AbilityBase::PopupPlacement::BOTTOM_RIGHT));
    napi_set_named_property(env, objValue, "LEFT_TOP", CreateJsValue(env, AbilityBase::PopupPlacement::LEFT_TOP));
    napi_set_named_property(env, objValue, "LEFT_BOTTOM", CreateJsValue(env, AbilityBase::PopupPlacement::LEFT_BOTTOM));
    napi_set_named_property(env, objValue, "RIGHT_TOP", CreateJsValue(env, AbilityBase::PopupPlacement::RIGHT_TOP));
    napi_set_named_property(env, objValue, "RIGHT_BOTTOM",
        CreateJsValue(env, AbilityBase::PopupPlacement::RIGHT_BOTTOM));
    napi_set_named_property(env, objValue, "NONE", CreateJsValue(env, AbilityBase::PopupPlacement::NONE));
    return objValue;
}

napi_value JsAutoFillManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "called");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Env or exportObj nullptr.");
        return nullptr;
    }

    auto jsAbilityAutoFillManager = std::make_unique<JsAutoFillManager>();
    napi_wrap(env, exportObj, jsAbilityAutoFillManager.release(), JsAutoFillManager::Finalizer, nullptr, nullptr);

    napi_set_named_property(env, exportObj, "AutoFillType", CreateJsAutoFillType(env));

    napi_set_named_property(env, exportObj, "PopupPlacement", CreateJsPopupPlacement(env));

    const char *moduleName = "JsAutoFillManager";
    BindNativeFunction(env, exportObj, "requestAutoSave", moduleName, JsAutoFillManager::RequestAutoSave);

    return CreateJsUndefined(env);
}
} // namespace AbilityRuntime
} // namespace OHOS