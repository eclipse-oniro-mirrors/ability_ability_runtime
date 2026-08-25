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
#include <memory>
#include <unordered_map>

#include "ability_business_error.h"
#include "app_mgr_client.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "res_sched_client.h"
#include "res_type.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
const std::string RES_SCHED_CLIENT_SO = "libressched_client.z.so";

napi_value CreateJsHyperSnapErrorInfo(napi_env env, const AppExecFwk::HyperSnapErrorRecord &record)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);

    napi_value codeValue = nullptr;
    napi_create_int32(env, static_cast<int32_t>(record.code), &codeValue);
    napi_set_named_property(env, object, "code", codeValue);

    napi_value msgValue = nullptr;
    napi_create_string_utf8(env, record.msg.c_str(), NAPI_AUTO_LENGTH, &msgValue);
    napi_set_named_property(env, object, "msg", msgValue);

    napi_value timeStampValue = nullptr;
    napi_create_int64(env, record.occurTimeStamp, &timeStampValue);
    napi_set_named_property(env, object, "occurTimeStamp", timeStampValue);
    return object;
}

bool SetEnumItem(napi_env env, napi_value object, const char *name, int32_t value)
{
    napi_value itemName = nullptr;
    napi_value itemValue = nullptr;
    if (napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &itemName) != napi_ok ||
        napi_create_int32(env, value, &itemValue) != napi_ok ||
        napi_set_property(env, object, itemName, itemValue) != napi_ok ||
        napi_set_property(env, object, itemValue, itemName) != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "SetEnumItem failed, name:%{public}s", name);
        return false;
    }
    return true;
}

// Enums declared in the d.ts are runtime values; for ArkTS dynamic apps the module
// object comes from this native Init, so each enum must be attached to exports.
napi_value InitHyperSnapErrorTypeEnum(napi_env env)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (!SetEnumItem(env, object, "CREATE_SNAPSHOT",
        static_cast<int32_t>(AppExecFwk::HyperSnapErrorType::CREATE_SNAPSHOT)) ||
        !SetEnumItem(env, object, "FORK_FROM_SNAPSHOT",
        static_cast<int32_t>(AppExecFwk::HyperSnapErrorType::FORK_FROM_SNAPSHOT))) {
        return nullptr;
    }
    return object;
}

napi_value InitHyperSnapErrorCodeEnum(napi_env env)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (!SetEnumItem(env, object, "ERR_OK",
        static_cast<int32_t>(AppExecFwk::HyperSnapErrorCode::ERR_OK)) ||
        !SetEnumItem(env, object, "ERR_SYSTEM_INNER",
        static_cast<int32_t>(AppExecFwk::HyperSnapErrorCode::ERR_SYSTEM_INNER)) ||
        !SetEnumItem(env, object, "ERR_SNAPSHOT_EXIST",
        static_cast<int32_t>(AppExecFwk::HyperSnapErrorCode::ERR_SNAPSHOT_EXIST)) ||
        !SetEnumItem(env, object, "ERR_PROCESS_IS_RUNNING",
        static_cast<int32_t>(AppExecFwk::HyperSnapErrorCode::ERR_PROCESS_IS_RUNNING)) ||
        !SetEnumItem(env, object, "ERR_SNAPSHOT_PROCESS_IS_DIED",
        static_cast<int32_t>(AppExecFwk::HyperSnapErrorCode::ERR_SNAPSHOT_PROCESS_IS_DIED)) ||
        !SetEnumItem(env, object, "ERR_SNAPSHOT_IS_INTERRUPTED",
        static_cast<int32_t>(AppExecFwk::HyperSnapErrorCode::ERR_SNAPSHOT_IS_INTERRUPTED)) ||
        !SetEnumItem(env, object, "ERR_EXISTS_ILLEGAL_BINDER",
        static_cast<int32_t>(AppExecFwk::HyperSnapErrorCode::ERR_EXISTS_ILLEGAL_BINDER)) ||
        !SetEnumItem(env, object, "ERR_LAST_PROCESS_NOT_FULLY_EXITED",
        static_cast<int32_t>(AppExecFwk::HyperSnapErrorCode::ERR_LAST_PROCESS_NOT_FULLY_EXITED))) {
        return nullptr;
    }
    return object;
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

    napi_value OnGetLastError(napi_env env, size_t argc, napi_value* argv)
    {
        TAG_LOGD(AAFwkTag::APPKIT, "OnGetLastError");
        // only support 1 params
        if (argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::APPKIT, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        int32_t errType = 0;
        if (!ConvertFromJsValue(env, argv[INDEX_ZERO], errType) ||
            (errType != static_cast<int32_t>(AppExecFwk::HyperSnapErrorType::CREATE_SNAPSHOT) &&
            errType != static_cast<int32_t>(AppExecFwk::HyperSnapErrorType::FORK_FROM_SNAPSHOT))) {
            TAG_LOGE(AAFwkTag::APPKIT, "get errType failed");
            ThrowInvalidParamError(env, "Parse param errType failed, must be a valid HyperSnapErrorType.");
            return CreateJsUndefined(env);
        }

        // The service fills `record` regardless of whether an error exists (code == ERR_OK means
        // no error recorded); only an IPC/system failure makes the call itself fail.
        auto record = std::make_shared<AppExecFwk::HyperSnapErrorRecord>();
        auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute =
            [errType, innerErrorCode, record]() {
                auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
                if (appMgrClient == nullptr) {
                    TAG_LOGW(AAFwkTag::APPKIT, "null appMgrClient");
                    *innerErrorCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
                    return;
                }
                *innerErrorCode = appMgrClient->GetHyperSnapLastError(errType, *record);
            };
        NapiAsyncTask::CompleteCallback complete =
            [innerErrorCode, record](napi_env env, NapiAsyncTask &task, int32_t status) {
                if (*innerErrorCode == ERR_OK) {
                    task.ResolveWithNoError(env, CreateJsHyperSnapErrorInfo(env, *record));
                } else {
                    task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
                }
            };
        napi_value lastParam = nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsHyperSnapManager::OnGetLastError",
            env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return result;
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

    napi_value hyperSnapErrorType = InitHyperSnapErrorTypeEnum(env);
    if (hyperSnapErrorType == nullptr || napi_set_named_property(env, exportObj,
        "HyperSnapErrorType", hyperSnapErrorType) != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to export HyperSnapErrorType");
        return nullptr;
    }

    napi_value hyperSnapErrorCode = InitHyperSnapErrorCodeEnum(env);
    if (hyperSnapErrorCode == nullptr || napi_set_named_property(env, exportObj,
        "HyperSnapErrorCode", hyperSnapErrorCode) != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to export HyperSnapErrorCode");
        return nullptr;
    }

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
}  // namespace AbilityRuntime
}  // namespace OHOS
