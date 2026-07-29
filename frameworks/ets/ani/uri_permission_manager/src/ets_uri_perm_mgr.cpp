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

#include "ets_uri_perm_mgr.h"

#include "ability_business_error.h"
#include "ability_manager_errors.h"
#include "ability_runtime_error_util.h"
#include "ani_common_util.h"
#include "ani_enum_convert.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "napi_common_util.h"
#include "parameters.h"
#include "tokenid_kit.h"
#include "uri.h"
#include "uri_permission_manager_client.h"
#include "ets_error_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const int32_t ERR_OK = 0;
constexpr const int32_t ERR_FAILURE = -1;
constexpr const char* NOT_SYSTEM_APP = "The application is not system-app, can not use system-api.";

ani_object CreateDouble(ani_env *env, int32_t res)
{
    if (env == nullptr) {
        return nullptr;
    }
    static const char *className = "std.core.Double";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "create double error");
        return nullptr;
    }

    if (cls == nullptr) {
        return nullptr;
    }
    ani_method ctor;
    env->Class_FindMethod(cls, "<ctor>", "d:", &ctor);
    if (ctor == nullptr) {
        return nullptr;
    }
    ani_object obj;
    env->Object_New(cls, ctor, &obj, ani_double(res));
    return obj;
}

}

static std::string GetStdString(ani_env* env, ani_string str)
{
    if (env == nullptr) {
        return std::string();
    }
    std::string result;
    ani_size sz {};
    env->String_GetUTF8Size(str, &sz);
    result.resize(sz + 1);
    env->String_GetUTF8SubString(str, 0, sz, result.data(), result.size(), &sz);
    result.resize(sz);
    return result;
}

static void grantUriPermissionCallbackSync([[maybe_unused]]ani_env *env,
    ani_string uri, ani_enum_item flagEnum, ani_string targetName, ani_int appCloneIndex, ani_object callback)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "grantUriPermissionCallbackSync run");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "env null");
        return;
    }
    if (appCloneIndex < 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "appCloneIndex invalid");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
            "Param appCloneIndex is invalid, the value less than 0."),
            nullptr);
        return;
    }
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    ani_object etsErrCode = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK);
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "app not system-app");
        etsErrCode = EtsErrorUtil::CreateError(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP), NOT_SYSTEM_APP);
        AppExecFwk::AsyncCallback(env, callback, etsErrCode, nullptr);
        return;
    }
    std::string uriStr = GetStdString(env, uri);
    Uri uriVec(uriStr);
    ani_int flag = 0;
    AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, flagEnum, flag);
    int32_t flagId = static_cast<int32_t>(flag);
    std::string targetBundleName = GetStdString(env, targetName);
    int32_t errCode = AAFwk::UriPermissionManagerClient::GetInstance().GrantUriPermission(uriVec, flagId,
        targetBundleName, appCloneIndex);
    if (errCode != ERR_OK) {
        etsErrCode = EtsErrorUtil::CreateErrorByNativeErr(env, errCode, "",
            GetInnerErrorMsg(AbilityInnerErrorMsg::GRANT_URI_PERMISSION_FAILED));
    }
    
    AppExecFwk::AsyncCallback(env, callback, etsErrCode, nullptr);
}

static void revokeUriPermissionCallbackSync([[maybe_unused]]ani_env *env,
    ani_string uri, ani_string targetName, ani_int appCloneIndex, ani_object callback)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "revokeUriPermissionCallbackSync run");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "env null");
        return;
    }
    if (appCloneIndex < 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "appCloneIndex invalid");
        AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
            "Param appCloneIndex is invalid, the value less than 0."),
            nullptr);
        return;
    }
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    ani_object etsErrCode = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK);
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "app not system-app");
        etsErrCode = EtsErrorUtil::CreateError(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP), NOT_SYSTEM_APP);
        AppExecFwk::AsyncCallback(env, callback, etsErrCode, nullptr);
        return;
    }
    std::string uriStr = GetStdString(env, uri);
    Uri uriVec(uriStr);
    std::string targetBundleName = GetStdString(env, targetName);
    int32_t errCode = AAFwk::UriPermissionManagerClient::GetInstance().RevokeUriPermissionManually(uriVec,
        targetBundleName, appCloneIndex);
    if (errCode != ERR_OK) {
        etsErrCode = EtsErrorUtil::CreateErrorByNativeErr(env, errCode, "",
            GetInnerErrorMsg(AbilityInnerErrorMsg::REVOKE_URI_PERMISSION_FAILED));
    }
    AppExecFwk::AsyncCallback(env, callback, etsErrCode, nullptr);
}

static void grantUriPermissionByKeyCallbackSync([[maybe_unused]]ani_env *env,
    ani_string uri, ani_enum_item flagEnum, ani_int targetTokenId, ani_object callback)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "grantUriPermissionByKeyCallbackSync start");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "env null");
        return;
    }
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    ani_object etsErrCode = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK);
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "app not system-app");
        etsErrCode = EtsErrorUtil::CreateError(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP), NOT_SYSTEM_APP);
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
    std::string uriStr = GetStdString(env, uri);
    ani_int flag = 0;
    AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, flagEnum, flag);
    int32_t result = ERR_OK;
    int32_t errCode =
        AAFwk::UriPermissionManagerClient::GetInstance().GrantUriPermissionByKey(uriStr, flag, targetTokenId);
    if (errCode != ERR_OK) {
        result = ERR_FAILURE;
        TAG_LOGE(AAFwkTag::DELEGATOR, "GrantUriPermissionByKey failed status: %{public}d", errCode);
        etsErrCode = EtsErrorUtil::CreateErrorByNativeErr(env, errCode, "",
            GetInnerErrorMsg(AbilityInnerErrorMsg::GRANT_URI_PERMISSION_BY_KEY_FAILED));
        AppExecFwk::AsyncCallback(env, callback, etsErrCode, CreateDouble(env, result));
        return;
    }
    etsErrCode = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK);
    AppExecFwk::AsyncCallback(env, callback, etsErrCode, nullptr);
}

static void grantUriPermissionByKeyAsCallerCallbackSync([[maybe_unused]]ani_env *env,
    ani_string uri, ani_enum_item flagEnum, ani_int callerTokenId, ani_int targetTokenId, ani_object callback)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "grantUriPermissionByKeyAsCallerCallbackSync start");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "env null");
        return;
    }
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    ani_object etsErrCode = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK);
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "app not system-app");
        etsErrCode = EtsErrorUtil::CreateError(env,
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP), NOT_SYSTEM_APP);
        AppExecFwk::AsyncCallback(env, callback, etsErrCode, CreateDouble(env, ERR_FAILURE));
        return;
    }
    std::string uriStr = GetStdString(env, uri);
    ani_int flag = 0;
    AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, flagEnum, flag);
    int32_t result = ERR_OK;
    int32_t errCode = AAFwk::UriPermissionManagerClient::GetInstance().GrantUriPermissionByKeyAsCaller(
        uriStr, flag, callerTokenId, targetTokenId);
    if (errCode != ERR_OK) {
        result = ERR_FAILURE;
        etsErrCode = EtsErrorUtil::CreateErrorByNativeErr(env, errCode, "",
            GetInnerErrorMsg(AbilityInnerErrorMsg::GRANT_URI_PERMISSION_AS_CALLER_FAILED));
    }
    AppExecFwk::AsyncCallback(env, callback, etsErrCode, CreateDouble(env, result));
}

void EtsUriPermissionManagerInit(ani_env *env)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "EtsUriPermissionManagerInit call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Invalid param");
        return;
    }
    ani_namespace ns;
    const char* targetNamespace = "@ohos.application.uriPermissionManager.uriPermissionManager";
    if (env->FindNamespace(targetNamespace, &ns) != ANI_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "FindNamespace failed");
    }
    if (ns == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "ns null");
        return;
    }
    std::array functions = {
        ani_native_function {
            "grantUriPermissionCallbackSync",
            "C{std.core.String}C{@ohos.app.ability.wantConstant.wantConstant.Flags}C{std.core.String}i"
            "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void*>(grantUriPermissionCallbackSync)
        },
        ani_native_function {
            "revokeUriPermissionCallbackSync",
            "C{std.core.String}C{std.core.String}i"
            "C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void*>(revokeUriPermissionCallbackSync)
        },
        ani_native_function {
            "grantUriPermissionByKeyCallbackSync",
            nullptr,
            reinterpret_cast<void*>(grantUriPermissionByKeyCallbackSync)
        },
        ani_native_function {
            "grantUriPermissionByKeyAsCallerCallbackSync",
            nullptr,
            reinterpret_cast<void*>(grantUriPermissionByKeyAsCallerCallbackSync)
        }
    };
    TAG_LOGI(AAFwkTag::URIPERMMGR, "EtsUriPermissionManagerInit bind functions");
    if (env->Namespace_BindNativeFunctions(ns, functions.data(), functions.size()) != ANI_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Namespace_BindNativeFunctions failed");
    };
    TAG_LOGI(AAFwkTag::URIPERMMGR, "EtsUriPermissionManagerInit end");
}

extern "C"{
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "ANI_Constructor");
    if (vm == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "vm null");
        return ANI_ERROR;
    }
    if (result == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "result null");
        return ANI_ERROR;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "GetEnv failed status: %{public}d", status);
        return ANI_NOT_FOUND;
    }

    EtsUriPermissionManagerInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGI(AAFwkTag::URIPERMMGR, "ANI_Constructor finish");
    return ANI_OK;
}
}
} // namespace AbilityRuntime
} // namespace OHOS