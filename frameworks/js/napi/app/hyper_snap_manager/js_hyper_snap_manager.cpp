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

#include "js_hyper_snap_manager.h"

#include <cstdint>
#include <dlfcn.h>
#include <new>
#include <unordered_map>

#include "ability_business_error.h"
#include "app_mgr_interface.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "res_sched_client.h"
#include "res_type.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
const std::string RES_SCHED_CLIENT_SO = "libressched_client.z.so";

// Obtain the AppMgrService proxy via SystemAbilityManager. The client side casts the SA
// remote object to IAppMgr; it never constructs AppMgrProxy directly.
sptr<AppExecFwk::IAppMgr> GetAppMgrInstance()
{
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetAppMgrInstance: system ability manager is null");
        return nullptr;
    }
    auto appObject = systemAbilityManager->GetSystemAbility(APP_MGR_SERVICE_ID);
    return iface_cast<AppExecFwk::IAppMgr>(appObject);
}

// Async context for getLastError (Promise-based). Created in OnGetLastError, filled in
// GetLastErrorExecute, consumed and deleted in GetLastErrorComplete.
struct GetLastErrorContext {
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    int32_t errType = 0;        // ErrorType: CREATE_SNAPSHOT(0) / FORK_FROM_SNAPSHOT(1)
    int32_t ipcResult = 0;      // IPC return code (ERR_OK on success)
    int32_t code = 0;           // HyperSnapErrorCode carried back to JS
    std::string msg;            // Error message carried back to JS
    std::string occurTimeStamp; // Timestamp (ms since boot, as string) carried back to JS
};

// Runs in the worker thread. No napi calls except logging.
void GetLastErrorExecute(napi_env env, void *data)
{
    auto *context = static_cast<GetLastErrorContext *>(data);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetLastErrorExecute: context is null");
        return;
    }

    auto appMgr = GetAppMgrInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetLastErrorExecute: failed to get AppMgrService proxy");
        context->ipcResult = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }

    // The service fills `record` regardless of whether an error exists (code == ERR_OK means
    // no error recorded); only an IPC/system failure makes the call itself fail.
    AppExecFwk::HyperSnapErrorRecord record;
    int32_t result = appMgr->GetHyperSnapLastError(context->errType, record);
    context->ipcResult = result;
    if (result == ERR_OK) {
        context->code = static_cast<int32_t>(record.code);
        context->msg = record.msg;
        context->occurTimeStamp = record.occurTimeStamp;
        TAG_LOGI(AAFwkTag::APPKIT, "GetLastErrorExecute: success, errType:%{public}d, code:%{public}d",
            context->errType, context->code);
    } else {
        TAG_LOGE(AAFwkTag::APPKIT, "GetLastErrorExecute: GetHyperSnapLastError failed, result:%{public}d", result);
    }
}

// Runs in the main thread. Per spec: on IPC success the promise resolves with
// HyperSnapErrorInfo (code may be 0 or a non-zero snapshot error code); it is only rejected
// for system/IPC failures. Parameter errors (401) are handled in OnGetLastError.
void GetLastErrorComplete(napi_env env, napi_status status, void *data)
{
    auto *context = static_cast<GetLastErrorContext *>(data);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetLastErrorComplete: context is null");
        return;
    }

    if (context->ipcResult == ERR_OK) {
        napi_value result = nullptr;
        napi_create_object(env, &result);

        napi_value codeValue = nullptr;
        napi_create_int32(env, context->code, &codeValue);
        napi_set_named_property(env, result, "code", codeValue);

        napi_value msgValue = nullptr;
        napi_create_string_utf8(env, context->msg.c_str(), NAPI_AUTO_LENGTH, &msgValue);
        napi_set_named_property(env, result, "msg", msgValue);

        napi_value timeStampValue = nullptr;
        napi_create_string_utf8(env, context->occurTimeStamp.c_str(), NAPI_AUTO_LENGTH, &timeStampValue);
        napi_set_named_property(env, result, "occurTimeStamp", timeStampValue);

        napi_resolve_deferred(env, context->deferred, result);
    } else {
        napi_value error = CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
        napi_reject_deferred(env, context->deferred, error);
    }

    napi_delete_async_work(env, context->work);
    delete context;
}

class JsHyperSnapManager final {
public:
    JsHyperSnapManager()
    {}

    ~JsHyperSnapManager() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGD(AAFwkTag::APPKIT, "called");
        std::unique_ptr<JsHyperSnapManager>(static_cast<JsHyperSnapManager*>(data));
    }

    static napi_value SetHyperSnapEnabled(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsHyperSnapManager, OnSetHyperSnapEnabled);
    }

    static napi_value RequestRebuildHyperSnap(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsHyperSnapManager, OnRequestRebuildHyperSnap);
    }

    static napi_value GetLastError(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsHyperSnapManager, OnGetLastError);
    }
private:

    napi_value OnSetHyperSnapEnabled(napi_env env, const size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPKIT, "OnSetHyperSnapEnabled");
        if (argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::APPKIT, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        bool enableFlag = false;
        if (!ConvertFromJsValue(env, argv[INDEX_ZERO], enableFlag)) {
            TAG_LOGE(AAFwkTag::APPKIT, "parse support flag failed, not boolean");
            ThrowInvalidParamError(env, "Parse param enable failed, must be a boolean.");
            return CreateJsUndefined(env);
        }
        TAG_LOGD(AAFwkTag::APPKIT, "enableFlag is %{public}d", enableFlag);
        std::unordered_map<std::string, std::string> payload {
            { "enableFlag", enableFlag ? "1" : "0" },
        };
        std::unordered_map<std::string, std::string> reply;
        uint32_t resType = ResourceSchedule::ResType::RES_TYPE_CTRL_FORKALL_IMAGE_INTERFACE;
        int32_t errCode = ResourceSchedule::ResSchedClient::GetInstance().ReportSyncEvent(resType,
            ResourceSchedule::ResType::CtrlForkallImageInterfaceCode::SET_SUPPORT_MIRROR_PROCESS, payload, reply);
        if (errCode != 0) {
            TAG_LOGE(AAFwkTag::APPKIT, "set enable fail, %{public}d", errCode);
            ThrowError(env, AbilityErrorCode::ERROR_CODE_SEND_REQUEST_TO_SYSTEM_FAIL);
        }
        return CreateJsUndefined(env);
    }

    napi_value OnRequestRebuildHyperSnap(napi_env env, const size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPKIT, "OnRequestRebuildHyperSnap");
        uint32_t resType = ResourceSchedule::ResType::RES_TYPE_CTRL_FORKALL_IMAGE_INTERFACE;
        std::unordered_map<std::string, std::string> payload;
        std::unordered_map<std::string, std::string> reply;
        int32_t errCode = ResourceSchedule::ResSchedClient::GetInstance().ReportSyncEvent(resType,
            ResourceSchedule::ResType::CtrlForkallImageInterfaceCode::REBUILD_IMAGE, payload, reply);
        if (errCode != 0) {
            TAG_LOGE(AAFwkTag::APPKIT, "rebuild fail, %{public}d", errCode);
            ThrowError(env, AbilityErrorCode::ERROR_CODE_SEND_REQUEST_TO_SYSTEM_FAIL);
        }
        return CreateJsUndefined(env);
    }

    // getLastError(errType): Promise<HyperSnapErrorInfo>.
    napi_value OnGetLastError(napi_env env, const size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPKIT, "OnGetLastError");

        napi_deferred deferred = nullptr;
        napi_value promise = nullptr;
        napi_create_promise(env, &deferred, &promise);

        // Validate errType: must be CREATE_SNAPSHOT(0) or FORK_FROM_SNAPSHOT(1).
        int32_t errType = 0;
        bool paramValid = (argc >= ARGC_ONE) &&
            (napi_get_value_int32(env, argv[INDEX_ZERO], &errType) == napi_ok) &&
            ((errType == static_cast<int32_t>(AppExecFwk::ErrorType::CREATE_SNAPSHOT)) ||
                (errType == static_cast<int32_t>(AppExecFwk::ErrorType::FORK_FROM_SNAPSHOT)));
        if (!paramValid) {
            TAG_LOGE(AAFwkTag::APPKIT, "OnGetLastError: invalid errType parameter");
            napi_value error = CreateInvalidParamJsError(env, "Parameter error. errType must be a valid ErrorType.");
            napi_reject_deferred(env, deferred, error);
            return promise;
        }

        auto *context = new (std::nothrow) GetLastErrorContext();
        if (context == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "OnGetLastError: alloc context failed");
            napi_value error = CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
            napi_reject_deferred(env, deferred, error);
            return promise;
        }
        context->errType = errType;
        context->deferred = deferred;

        napi_value resourceName = nullptr;
        napi_create_string_utf8(env, "GetHyperSnapLastError", NAPI_AUTO_LENGTH, &resourceName);
        napi_status status = napi_create_async_work(env, nullptr, resourceName, GetLastErrorExecute,
            GetLastErrorComplete, static_cast<void *>(context), &context->work);
        if (status != napi_ok) {
            TAG_LOGE(AAFwkTag::APPKIT, "OnGetLastError: failed to create async work");
            delete context;
            napi_value error = CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
            napi_reject_deferred(env, deferred, error);
            return promise;
        }

        status = napi_queue_async_work_with_qos(env, context->work, napi_qos_user_initiated);
        if (status != napi_ok) {
            TAG_LOGE(AAFwkTag::APPKIT, "OnGetLastError: failed to queue async work");
            napi_delete_async_work(env, context->work);
            delete context;
            napi_value error = CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER);
            napi_reject_deferred(env, deferred, error);
            return promise;
        }

        return promise;
    }
};
} // namespace

napi_value JsHyperSnapManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "null env or exportObj");
        return nullptr;
    }

    std::unique_ptr<JsHyperSnapManager> jsHyperSnapManager =
        std::make_unique<JsHyperSnapManager>();
    napi_wrap(env, exportObj, jsHyperSnapManager.release(),
        JsHyperSnapManager::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsHyperSnapManager";
    BindNativeFunction(env, exportObj, "setHyperSnapEnabled", moduleName,
        JsHyperSnapManager::SetHyperSnapEnabled);
    BindNativeFunction(env, exportObj, "requestRebuildHyperSnap", moduleName,
        JsHyperSnapManager::RequestRebuildHyperSnap);
    BindNativeFunction(env, exportObj, "getLastError", moduleName,
        JsHyperSnapManager::GetLastError);
    TAG_LOGD(AAFwkTag::APPKIT, "JsHyperSnapManager end");
    return CreateJsUndefined(env);
}
}  //namespace AbilityRuntime
}  //namespace OHOS
