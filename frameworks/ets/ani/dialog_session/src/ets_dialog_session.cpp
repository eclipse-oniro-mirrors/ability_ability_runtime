/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ets_dialog_session.h"

#include "ability_business_error.h"
#include "ability_manager_client.h"
#include "ani_common_ability_state_data.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "ets_dialog_session_utils.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"
#include "int_wrapper.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* ETS_DIALOG_SESSION_NAMESPACE = "@ohos.app.ability.dialogSession.dialogSession";
}

static void SendDialogResult(
    ani_env *env, ani_string etsDialogSessionId, ani_object wantObj, ani_boolean etsIsAllow, ani_object callback)
{
    TAG_LOGD(AAFwkTag::DIALOG, "call SendDialogResult");
    std::string dialogSessionId = "";
    ani_object aniObject;

    if (!AppExecFwk::GetStdString(env, etsDialogSessionId, dialogSessionId)) {
        TAG_LOGE(AAFwkTag::DIALOG, "Failed unwrap dialogSessionId");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "Parameter error: dialogSessionId must be a string.");
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::DIALOG, "Failed unwrap want");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "Parameter error: want must be a Want.");
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    bool isAllow = false;
    if (etsIsAllow != 1 && etsIsAllow != 0) {
        TAG_LOGE(AAFwkTag::DIALOG, "Failed unwrap isAllow");
        aniObject = EtsErrorUtil::CreateInvalidParamError(env, "Parameter error: isAllow must be a Boolean.");
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    isAllow = static_cast<bool>(etsIsAllow);
    auto errorCode =
        AAFwk::AbilityManagerClient::GetInstance()->SendDialogResult(want, dialogSessionId, isAllow);
    AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, errorCode, "",
        GetInnerErrorMsg(AbilityInnerErrorMsg::SEND_DIALOG_RESULT_FAILED)), nullptr);
}

static ani_object GetDialogSessionInfo(ani_env *env, ani_string etsDialogSessionId)
{
    TAG_LOGD(AAFwkTag::DIALOG, "call GetDialogSessionInfo");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null env");
        return nullptr;
    }
    std::string dialogSessionId = "";
    if (!AppExecFwk::GetStdString(env, etsDialogSessionId, dialogSessionId)) {
        TAG_LOGE(AAFwkTag::DIALOG, "Failed unwrap dialogSessionId");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error: dialogSessionId must be a valid string.");
        return AppExecFwk::CreateEtsNull(env);
    }

    sptr<AAFwk::DialogSessionInfo> dialogSessionInfo;
#ifdef SUPPORT_SCREEN
    auto errCode =
        AAFwk::AbilityManagerClient::GetInstance()->GetDialogSessionInfo(dialogSessionId, dialogSessionInfo);
    if (errCode != ERR_OK || dialogSessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::DIALOG,
            "GetDialogSessionInfo failed with incorrect return value or empty dialogSessionInfo");
        return AppExecFwk::CreateEtsNull(env);
    }
#endif // SUPPORT_SCREEN
    ani_object dialogSessionInfoObj = AppExecFwk::WrapDialogSessionInfo(env, *dialogSessionInfo);
    if (dialogSessionInfoObj == nullptr) {
        EtsErrorUtil::ThrowError(env, EtsErrorUtil::CreateErrorByNativeErr(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER), "",
            GetInnerErrorMsg(AbilityInnerErrorMsg::GET_DIALOG_SESSION_INFO_FAILED)));
        return AppExecFwk::CreateEtsNull(env);
    }
    return dialogSessionInfoObj;
}

void EtsDialogSessionInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::DIALOG, "call EtsDialogSessionInit");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DIALOG, "null env");
        return;
    }
    ani_status status = ANI_ERROR;
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::DIALOG, "ResetError failed");
    }
    ani_namespace ns = nullptr;
    status = env->FindNamespace(ETS_DIALOG_SESSION_NAMESPACE, &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DIALOG, "FindNamespace abilityManager failed status : %{public}d", status);
        return;
    }
    std::array methods = {
        ani_native_function {"getDialogSessionInfo", nullptr,
            reinterpret_cast<void *>(GetDialogSessionInfo)},
        ani_native_function {"nativeSendDialogResult", nullptr,
            reinterpret_cast<void *>(SendDialogResult) },
    };
    status = env->Namespace_BindNativeFunctions(ns, methods.data(), methods.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DIALOG, "Namespace_BindNativeFunctions failed status : %{public}d", status);
    }
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::DIALOG, "ResetError failed");
    }
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::DIALOG, "in DialogSessionEts.ANI_Constructor");
    if (vm == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::DIALOG, "null vm or result");
        return ANI_INVALID_ARGS;
    }

    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DIALOG, "GetEnv failed, status=%{public}d", status);
        return ANI_NOT_FOUND;
    }
    EtsDialogSessionInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::DIALOG, "DialogSessionEts.ANI_Constructor finished");
    return ANI_OK;
}
}
} // namespace AbilityRuntime
} // namespace OHOS