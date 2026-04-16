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
#include <unordered_map>

#include "ability_business_error.h"
#include "hilog_tag_wrapper.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "res_sched_client.h"
#include "res_type.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
const std::string RES_SCHED_CLIENT_SO = "libressched_client.z.so";

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
    TAG_LOGD(AAFwkTag::APPKIT, "JsHyperSnapManager end");
    return CreateJsUndefined(env);
}
}  //namespace AbilityRuntime
}  //namespace OHOS