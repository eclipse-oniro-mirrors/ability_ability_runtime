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

#include "ets_error_utils.h"

#include "ability_runtime_error_util.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *ERR_MSG_TOO_FEW_PARAM = "Parameter error. Too few parameters.";
constexpr const char *ERR_MSG_NOT_MAINTHREAD = "Caller error. Caller from non-main thread.";
constexpr const char *ERR_MSG_INVALID_NUM_PARAMS = "Parameter error. The number of parameters is invalid.";
constexpr const char *NOT_SYSTEM_APP = "The application is not system-app, can not use system-api.";
constexpr const char *BUSINESS_ERROR_CLASS = "@ohos.base.BusinessError";
constexpr const char *ERROR_CLASS_NAME = "std.core.Error";
constexpr const char* ERROR_MSG_TRANSFER_CLASS_NOT_FOUND = "Unable to find the class for transferring.";
constexpr int32_t ERROR_CODE_TRANSFER_CLASS_NOT_FOUND = 10200067;
} // namespace

void EtsErrorUtil::ThrowError(ani_env *env, ani_object err)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return;
    }
    env->ThrowError(static_cast<ani_error>(err));
}

void EtsErrorUtil::ThrowError(ani_env *env, int32_t errCode, const std::string &errorMsg)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return;
    }
    EtsErrorUtil::ThrowError(env, EtsErrorUtil::CreateError(env, errCode, errorMsg));
}

void EtsErrorUtil::ThrowError(ani_env *env, const AbilityErrorCode &err)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return;
    }
    EtsErrorUtil::ThrowError(env, EtsErrorUtil::CreateError(env, static_cast<int32_t>(err), GetErrorMsg(err)));
}

void EtsErrorUtil::ThrowRuntimeError(ani_env *env, int32_t errCode, const std::string &errMessage)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return;
    }
    std::string eMes = errMessage;
    if (eMes.empty()) {
        eMes = AbilityRuntimeErrorUtil::GetErrMessage(errCode);
    }
    EtsErrorUtil::ThrowError(env, EtsErrorUtil::CreateError(env, errCode, eMes));
}

void EtsErrorUtil::ThrowInvalidCallerError(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return;
    }
    EtsErrorUtil::ThrowError(env, EtsErrorUtil::CreateError(
        env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CALLER), ERR_MSG_NOT_MAINTHREAD));
}

void EtsErrorUtil::ThrowTooFewParametersError(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return;
    }
    EtsErrorUtil::ThrowError(env, EtsErrorUtil::CreateError(
        env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM), ERR_MSG_TOO_FEW_PARAM));
}

void EtsErrorUtil::ThrowInvalidNumParametersError(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return;
    }
    EtsErrorUtil::ThrowError(env, EtsErrorUtil::CreateError(
        env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM), ERR_MSG_INVALID_NUM_PARAMS));
}

void EtsErrorUtil::ThrowNoPermissionError(ani_env *env, const std::string &permission)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return;
    }
    EtsErrorUtil::ThrowError(
        env, EtsErrorUtil::CreateError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED),
        GetNoPermissionErrorMsg(permission)));
}

void EtsErrorUtil::ThrowNotSystemAppError(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return;
    }
    EtsErrorUtil::ThrowError(env, EtsErrorUtil::CreateError(
        env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP), NOT_SYSTEM_APP));
}

void EtsErrorUtil::ThrowEtsTransferClassError(ani_env *env)
{
    EtsErrorUtil::ThrowError(env, EtsErrorUtil::CreateError(
        env, static_cast<int32_t>(ERROR_CODE_TRANSFER_CLASS_NOT_FOUND), ERROR_MSG_TRANSFER_CLASS_NOT_FOUND));
}

void EtsErrorUtil::ThrowInvalidParamError(ani_env *env, const std::string &message)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return;
    }
    EtsErrorUtil::ThrowError(env, EtsErrorUtil::CreateInvalidParamError(env, message));
}

void EtsErrorUtil::ThrowErrorByNativeErr(ani_env *env, int32_t err)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return;
    }
    EtsErrorUtil::ThrowError(env, EtsErrorUtil::CreateErrorByNativeErr(env, err));
}

ani_object EtsErrorUtil::CreateError(ani_env *env, const AbilityErrorCode &err)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }
    return EtsErrorUtil::CreateError(env, static_cast<int32_t>(err), GetErrorMsg(err));
}

ani_object EtsErrorUtil::CreateInvalidParamError(ani_env *env, const std::string &message)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }
    return EtsErrorUtil::CreateError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM), message);
}

ani_object EtsErrorUtil::CreateNoPermissionError(ani_env *env, const std::string &permission)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }
    return EtsErrorUtil::CreateError(
        env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED), GetNoPermissionErrorMsg(permission));
}

ani_object EtsErrorUtil::CreateErrorByNativeErr(ani_env *env, int32_t err, const std::string &permission)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }
    auto errCode = GetJsErrorCodeByNativeError(err);
    auto errMsg = (errCode == AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED && !permission.empty())
                      ? GetNoPermissionErrorMsg(permission)
                      : GetErrorMsg(errCode);
    return EtsErrorUtil::CreateError(env, static_cast<int32_t>(errCode), errMsg);
}

ani_object EtsErrorUtil::WrapError(ani_env *env, const std::string &msg)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }
    ani_status status = ANI_ERROR;
    ani_string aniMsg = nullptr;
    if ((status = env->String_NewUTF8(msg.c_str(), msg.size(), &aniMsg)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "String_NewUTF8 failed %{public}d", status);
        return nullptr;
    }
    ani_ref undefRef;
    if ((status = env->GetUndefined(&undefRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetUndefined failed %{public}d", status);
        return nullptr;
    }
    ani_class cls = nullptr;
    if ((status = env->FindClass(ERROR_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "FindClass failed %{public}d", status);
        return nullptr;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, "<ctor>", "C{std.core.String}C{std.core.ErrorOptions}:", &method)) !=
        ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Class_FindMethod failed %{public}d", status);
        return nullptr;
    }
    ani_object obj = nullptr;
    if ((status = env->Object_New(cls, method, &obj, aniMsg, undefRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_New failed %{public}d", status);
        return nullptr;
    }
    return obj;
}

ani_object EtsErrorUtil::CreateError(ani_env *env, ani_int code, const std::string &msg)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }
    ani_status status = ANI_ERROR;
    ani_class cls = nullptr;
    if ((status = env->FindClass(BUSINESS_ERROR_CLASS, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "FindClass failed %{public}d", status);
        return nullptr;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, "<ctor>", "iC{std.core.Error}:", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Class_FindMethod failed %{public}d", status);
        return nullptr;
    }
    ani_object error = EtsErrorUtil::WrapError(env, msg);
    if (error == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "error nulll");
        return nullptr;
    }
    ani_object obj = nullptr;
    if ((status = env->Object_New(cls, method, &obj, code, error)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_New failed %{public}d", status);
        return nullptr;
    }
    return obj;
}
} // namespace AbilityRuntime
} // namespace OHOS
