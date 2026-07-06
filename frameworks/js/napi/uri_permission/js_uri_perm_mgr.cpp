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

#include "js_uri_perm_mgr.h"

#include "ability_business_error.h"
#include "ability_manager_errors.h"
#include "ability_runtime_error_util.h"
#include "hilog_tag_wrapper.h"
#ifdef HIVIEWDFX_RUNTIME_API_METRICS
#include "histogram_plugin_macros.h"
#endif
#include "ipc_skeleton.h"
#include "js_error_utils.h"
#include "napi_common_util.h"
#include "parameters.h"
#include "tokenid_kit.h"
#include "uri.h"
#include "uri_permission_manager_client.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t ARG_COUNT_TWO = 2;
constexpr int32_t ARG_COUNT_THREE = 3;
constexpr int32_t ARG_COUNT_FOUR = 4;
constexpr int32_t ARG_INDEX_ZERO = 0;
constexpr int32_t ARG_INDEX_ONE = 1;
constexpr int32_t ARG_INDEX_TWO = 2;
constexpr int32_t ARG_INDEX_THREE = 3;

struct UriPermissionParam {
    std::string uriStr;
    int32_t flag = 0;
    std::string bundleName;
    int32_t appIndex = 0;
    bool hasAppIndex = false;
    std::string key;
    int32_t callerTokenId = 0;
    int32_t targetTokenId = 0;
};

static void ResolveGrantUriPermissionTask(napi_env env, NapiAsyncTask &task, int32_t errCode)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "ResolveGrantUriPermissionTask");
    HandleScope handleScope(env);
    if (errCode == ERR_OK) {
        task.ResolveWithNoError(env, CreateJsUndefined(env));
        return;
    }
    if (errCode == AAFwk::CHECK_PERMISSION_FAILED || errCode == AAFwk::ERR_CODE_INVALID_URI_FLAG ||
        errCode == AAFwk::ERR_CODE_INVALID_URI_TYPE || errCode == AAFwk::ERR_CODE_GRANT_URI_PERMISSION) {
        task.Reject(env, CreateJsErrorByNativeErr(env, errCode, "ohos.permission.PROXY_AUTHORIZATION_URI"));
        return;
    }
    task.Reject(env, CreateJsError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
        GetInnerErrorMsg(AbilityInnerErrorMsg::GRANT_URI_PERMISSION_FAILED)));
    return;
}

static void ResolveGrantUriPermissionWithAppIndexTask(napi_env env, NapiAsyncTask &task, int32_t errCode)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "ResolveGrantUriPermissionWithAppIndexTask");
    HandleScope handleScope(env);
    if (errCode == ERR_OK) {
        task.ResolveWithNoError(env, CreateJsUndefined(env));
        return;
    }
    if (errCode == AAFwk::CHECK_PERMISSION_FAILED || errCode == AAFwk::ERR_CODE_INVALID_URI_FLAG ||
        errCode == AAFwk::ERR_CODE_INVALID_URI_TYPE || errCode == AAFwk::ERR_CODE_GRANT_URI_PERMISSION ||
        errCode == AAFwk::ERR_GET_TARGET_BUNDLE_INFO_FAILED) {
        task.Reject(env, CreateJsErrorByNativeErr(env, errCode, "ohos.permission.PROXY_AUTHORIZATION_URI"));
        return;
    }
    task.Reject(env, CreateJsError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
        GetInnerErrorMsg(AbilityInnerErrorMsg::GRANT_URI_PERMISSION_FAILED)));
    return;
}

static void ResolveRevokeUriPermissionTask(napi_env env, NapiAsyncTask &task, int32_t errCode)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "ResolveRevokeUriPermissionTask");
    HandleScope handleScope(env);
    if (errCode == ERR_OK) {
        task.ResolveWithNoError(env, CreateJsUndefined(env));
        return;
    }
    if (errCode == AAFwk::ERR_CODE_INVALID_URI_TYPE || errCode == AAFwk::ERR_CODE_GRANT_URI_PERMISSION) {
        task.Reject(env, CreateJsErrorByNativeErr(env, errCode));
        return;
    }
    task.Reject(env, CreateJsError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
        GetInnerErrorMsg(AbilityInnerErrorMsg::REVOKE_URI_PERMISSION_FAILED)));
    return;
}

static void ResolveRevokeUriPermissionWithAppIndexTask(napi_env env, NapiAsyncTask &task, int32_t errCode)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "ResolveRevokeUriPermissionWithAppIndexTask");
    HandleScope handleScope(env);
    if (errCode == ERR_OK) {
        task.ResolveWithNoError(env, CreateJsUndefined(env));
        return;
    }
    if (errCode == AAFwk::ERR_CODE_INVALID_URI_TYPE || errCode == AAFwk::ERR_CODE_GRANT_URI_PERMISSION ||
        errCode == AAFwk::ERR_GET_TARGET_BUNDLE_INFO_FAILED) {
        task.Reject(env, CreateJsErrorByNativeErr(env, errCode));
        return;
    }
    task.Reject(env, CreateJsError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
        GetInnerErrorMsg(AbilityInnerErrorMsg::REVOKE_URI_PERMISSION_FAILED)));
    return;
}

static void ResolveGrantUriPermissionByKeyAsCallerTask(napi_env env, NapiAsyncTask &task, int32_t errCode)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "ResolveGrantUriPermissionByKeyAsCallerTask");
    HandleScope handleScope(env);
    if (errCode == ERR_OK) {
        task.ResolveWithNoError(env, CreateJsUndefined(env));
        return;
    }
    task.Reject(env, CreateJsErrorByNativeErr(env, errCode, "ohos.permission.GRANT_URI_PERMISSION_AS_CALLER",
        GetInnerErrorMsg(AbilityInnerErrorMsg::GRANT_URI_PERMISSION_AS_CALLER_FAILED)));
    return;
}

static void ResolveGrantUriPermissionByKeyTask(napi_env env, NapiAsyncTask &task, int32_t errCode)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "ResolveGrantUriPermissionByKeyTask");
    HandleScope handleScope(env);
    if (errCode == ERR_OK) {
        task.ResolveWithNoError(env, CreateJsUndefined(env));
        return;
    }
    task.Reject(env, CreateJsErrorByNativeErr(env, errCode, "",
        GetInnerErrorMsg(AbilityInnerErrorMsg::GRANT_URI_PERMISSION_BY_KEY_FAILED)));
    return;
}

static bool ParseGrantUriPermissionParams(napi_env env, const NapiCallbackInfo &info, UriPermissionParam &param)
{
    // only support 3 or 4 params
    if (info.argc < ARG_COUNT_THREE) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "too few parameters");
        ThrowTooFewParametersError(env);
        return false;
    }
    if (!OHOS::AppExecFwk::UnwrapStringFromJS2(env, info.argv[0], param.uriStr)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "invalid uriStr");
        ThrowInvalidParamError(env, "Parse param uri failed, uri must be string.");
        return false;
    }
    if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(env, info.argv[1], param.flag)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "flag invalid.");
        ThrowInvalidParamError(env, "Parse param flag failed, flag must be number.");
        return false;
    }
    if (!OHOS::AppExecFwk::UnwrapStringFromJS2(env, info.argv[ARG_COUNT_TWO], param.bundleName)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "targetBundleName invalid");
        ThrowInvalidParamError(env, "Parse param targetBundleName failed, targetBundleName must be string.");
        return false;
    }
    if (info.argc >= ARG_COUNT_FOUR) {
        // process param index or callback
        if (CheckTypeForNapiValue(env, info.argv[ARG_COUNT_THREE], napi_function)) {
            return true;
        }
        if (!CheckTypeForNapiValue(env, info.argv[ARG_COUNT_THREE], napi_number) ||
            !OHOS::AppExecFwk::UnwrapInt32FromJS2(env, info.argv[ARG_COUNT_THREE], param.appIndex)) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "appIndex invalid");
            ThrowInvalidParamError(env, "Parse param appCloneIndex failed, appCloneIndex must be number.");
            return false;
        }
        if (param.appIndex < 0) {
            ThrowInvalidParamError(env, "Param appCloneIndex is invalid, the value less than 0.");
            return false;
        }
        param.hasAppIndex = true;
    }
    return true;
}

static bool ParseRevokeUriPermissionParams(napi_env env, const NapiCallbackInfo &info, UriPermissionParam &param)
{
    // only support 2 or 3 params
    if (info.argc < ARG_COUNT_TWO) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Invalid args");
        ThrowTooFewParametersError(env);
        return false;
    }
    if (!OHOS::AppExecFwk::UnwrapStringFromJS2(env, info.argv[0], param.uriStr)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "invalid uriStr");
        ThrowInvalidParamError(env, "Parse param uri failed, uri must be string.");
        return false;
    }
    if (!OHOS::AppExecFwk::UnwrapStringFromJS2(env, info.argv[1], param.bundleName)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "bundleName invalid");
        ThrowInvalidParamError(env, "Parse param bundleName failed, bundleName must be string.");
        return false;
    }
    if (info.argc >= ARG_COUNT_THREE) {
        // process param index or callback
        if (CheckTypeForNapiValue(env, info.argv[ARG_COUNT_TWO], napi_function)) {
            return true;
        }
        if (!CheckTypeForNapiValue(env, info.argv[ARG_COUNT_TWO], napi_number) ||
            !OHOS::AppExecFwk::UnwrapInt32FromJS2(env, info.argv[ARG_COUNT_TWO], param.appIndex)) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "appCloneIndex invalid");
            ThrowInvalidParamError(env, "Parse param appCloneIndex failed, appCloneIndex must be number.");
            return false;
        }
        if (param.appIndex < 0) {
            ThrowInvalidParamError(env, "Param appCloneIndex is invalid, the value less than 0.");
            return false;
        }
        param.hasAppIndex = true;
    }
    return true;
}

static bool ParseGrantUriPermissionByKeyParams(napi_env env, const NapiCallbackInfo &info,
    UriPermissionParam &param)
{
    // key, flag, callerTokenId, targetTokenId
    if (info.argc < ARG_COUNT_THREE) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Too few args");
        ThrowTooFewParametersError(env);
        return false;
    }
    if (!OHOS::AppExecFwk::UnwrapStringFromJS2(env, info.argv[ARG_INDEX_ZERO], param.key)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "invalid key");
        ThrowInvalidParamError(env, "Failed to parse param key, key must be string.");
        return false;
    }
    if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(env, info.argv[ARG_INDEX_ONE], param.flag)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "invalid flag");
        ThrowInvalidParamError(env, "Failed to parse param flag, flag must be number.");
        return false;
    }
    if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(env, info.argv[ARG_INDEX_TWO], param.targetTokenId)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "invalid targetTokenId");
        ThrowInvalidParamError(env, "Failed to parse param targetTokenId, targetTokenId must be number.");
        return false;
    }
    return true;
}

static bool ParseGrantUriPermissionByKeyAsCallerParams(napi_env env, const NapiCallbackInfo &info,
    UriPermissionParam &param)
{
    // key, flag, callerTokenId, targetTokenId
    if (info.argc < ARG_COUNT_FOUR) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Too few args");
        ThrowTooFewParametersError(env);
        return false;
    }
    if (!OHOS::AppExecFwk::UnwrapStringFromJS2(env, info.argv[ARG_INDEX_ZERO], param.key)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "invalid key");
        ThrowInvalidParamError(env, "Failed to parse param key, key must be string.");
        return false;
    }
    if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(env, info.argv[ARG_INDEX_ONE], param.flag)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "invalid flag");
        ThrowInvalidParamError(env, "Failed to parse param flag, flag must be number.");
        return false;
    }
    if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(env, info.argv[ARG_INDEX_TWO], param.callerTokenId)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "invalid callerTokenId");
        ThrowInvalidParamError(env, "Failed to parse param callerTokenId, callerTokenId must be number.");
        return false;
    }
    if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(env, info.argv[ARG_INDEX_THREE], param.targetTokenId)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "invalid targetTokenId");
        ThrowInvalidParamError(env, "Failed to Parse param targetTokenId, targetTokenId must be number.");
        return false;
    }
    return true;
}
}
class JsUriPermMgr {
public:
    JsUriPermMgr() = default;
    ~JsUriPermMgr() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "call");
        std::unique_ptr<JsUriPermMgr>(static_cast<JsUriPermMgr*>(data));
    }

    static napi_value GrantUriPermission(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsUriPermMgr, OnGrantUriPermission);
    }

    static napi_value RevokeUriPermission(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsUriPermMgr, OnRevokeUriPermission);
    }

    static napi_value GrantUriPermissionByKeyAsCaller(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsUriPermMgr, OnGrantUriPermissionByKeyAsCaller);
    }

    static napi_value GrantUriPermissionByKey(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsUriPermMgr, OnGrantUriPermissionByKey);
    }

private:
    napi_value OnGrantUriPermission(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::URIPERMMGR, "start");
    #ifdef HIVIEWDFX_RUNTIME_API_METRICS
        HISTOGRAM_BOOLEAN("AbilityKit.uriPermissionManager.grantUriPermission", 1);
    #endif
        HandleEscape handleEscape(env);
        UriPermissionParam param;
        if (!ParseGrantUriPermissionParams(env, info, param)) {
            return CreateJsUndefined(env);
        }
        auto selfToken = IPCSkeleton::GetSelfTokenID();
        if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "app not system-app");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
            return CreateJsUndefined(env);
        }
        auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute =
            [param, innerErrCode]() {
                Uri uri(param.uriStr);
                *innerErrCode = AAFwk::UriPermissionManagerClient::GetInstance().GrantUriPermission(uri,
                    param.flag, param.bundleName, param.appIndex);
        };
        NapiAsyncTask::CompleteCallback complete =
        [param, innerErrCode](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (*innerErrCode != ERR_OK) {
                TAG_LOGE(AAFwkTag::URIPERMMGR, "GrantUriPermission ret: %{public}d", *innerErrCode);
            }
            if (param.hasAppIndex) {
                return ResolveGrantUriPermissionWithAppIndexTask(env, task, *innerErrCode);
            }
            return ResolveGrantUriPermissionTask(env, task, *innerErrCode);
        };
        napi_value lastParam = (info.argc >= ARG_COUNT_FOUR && !param.hasAppIndex) ?
            info.argv[ARG_COUNT_THREE] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsUriPermMgr::OnGrantUriPermission",
            env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnGrantUriPermissionByKey(napi_env env, const NapiCallbackInfo &info)
    {
        TAG_LOGD(AAFwkTag::URIPERMMGR, "OnGrantUriPermissionByKey start");
        HandleEscape handleEscape(env);
        UriPermissionParam param;
        if (!ParseGrantUriPermissionByKeyParams(env, info, param)) {
            return CreateJsUndefined(env);
        }
        auto selfToken = IPCSkeleton::GetSelfTokenID();
        if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "app not system-app");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
            return CreateJsUndefined(env);
        }
        auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute =
            [param, innerErrCode]() {
                *innerErrCode = AAFwk::UriPermissionManagerClient::GetInstance().GrantUriPermissionByKey(
                    param.key, param.flag, param.targetTokenId);
        };
        NapiAsyncTask::CompleteCallback complete =
        [innerErrCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            if (*innerErrCode != ERR_OK) {
                TAG_LOGE(AAFwkTag::URIPERMMGR, "GrantUriPermissionByKey fail:%{public}d", *innerErrCode);
            }
            ResolveGrantUriPermissionByKeyTask(env, task, *innerErrCode);
        };
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsUriPermMgr::OnGrantUriPermissionByKey",
            env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnGrantUriPermissionByKeyAsCaller(napi_env env, const NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::URIPERMMGR, "OnGrantUriPermissionByKeyAsCaller start");
        HandleEscape handleEscape(env);
        UriPermissionParam param;
        if (!ParseGrantUriPermissionByKeyAsCallerParams(env, info, param)) {
            return CreateJsUndefined(env);
        }
        auto selfToken = IPCSkeleton::GetSelfTokenID();
        if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "app not system-app");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
            return CreateJsUndefined(env);
        }
        auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute =
            [param, innerErrCode]() {
                *innerErrCode = AAFwk::UriPermissionManagerClient::GetInstance().GrantUriPermissionByKeyAsCaller(
                    param.key, param.flag, param.callerTokenId, param.targetTokenId);
        };
        NapiAsyncTask::CompleteCallback complete =
        [innerErrCode](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (*innerErrCode != ERR_OK) {
                TAG_LOGE(AAFwkTag::URIPERMMGR, "GrantUriPermissionByKeyAsCaller fail:%{public}d", *innerErrCode);
            }
            ResolveGrantUriPermissionByKeyAsCallerTask(env, task, *innerErrCode);
        };
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsUriPermMgr::OnGrantUriPermissionByKeyAsCaller",
            env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnRevokeUriPermission(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::URIPERMMGR, "start");
        HandleEscape handleEscape(env);
        UriPermissionParam param;
        if (!ParseRevokeUriPermissionParams(env, info, param)) {
            return CreateJsUndefined(env);
        }
        auto selfToken = IPCSkeleton::GetSelfTokenID();
        if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "app not system-app");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
            return CreateJsUndefined(env);
        }
        auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute = [param, innerErrCode]() {
            Uri uri(param.uriStr);
            *innerErrCode = AAFwk::UriPermissionManagerClient::GetInstance().RevokeUriPermissionManually(uri,
                param.bundleName, param.appIndex);
        };
        NapiAsyncTask::CompleteCallback complete =
        [param, innerErrCode](napi_env env, NapiAsyncTask& task, int32_t status) {
            if (*innerErrCode != ERR_OK) {
                TAG_LOGE(AAFwkTag::URIPERMMGR, "RevokeUriPermissionManually ret: %{public}d", *innerErrCode);
            }
            if (param.hasAppIndex) {
                return ResolveRevokeUriPermissionWithAppIndexTask(env, task, *innerErrCode);
            }
            return ResolveRevokeUriPermissionTask(env, task, *innerErrCode);
        };
        napi_value lastParam = (info.argc >= ARG_COUNT_THREE && !param.hasAppIndex) ?
            info.argv[ARG_COUNT_TWO] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsUriPermMgr::OnRevokeUriPermission",
            env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }
};

napi_value CreateJsUriPermMgr(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "call");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "Invalid param");
        return nullptr;
    }

    std::unique_ptr<JsUriPermMgr> jsUriPermMgr = std::make_unique<JsUriPermMgr>();
    napi_wrap(env, exportObj, jsUriPermMgr.release(), JsUriPermMgr::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsUriPermMgr";
    BindNativeFunction(env, exportObj, "grantUriPermission", moduleName, JsUriPermMgr::GrantUriPermission);
    BindNativeFunction(env, exportObj, "revokeUriPermission", moduleName, JsUriPermMgr::RevokeUriPermission);
    BindNativeFunction(env, exportObj, "grantUriPermissionByKey", moduleName, JsUriPermMgr::GrantUriPermissionByKey);
    BindNativeFunction(env, exportObj, "grantUriPermissionByKeyAsCaller", moduleName,
        JsUriPermMgr::GrantUriPermissionByKeyAsCaller);
    return CreateJsUndefined(env);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
