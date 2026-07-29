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

#include "ability_runtime_error_util.h"

#include <map>
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::map<int32_t, int32_t> ERROR_CODE_MAP = {
    { ERR_OK, ERR_OK }
};

const std::map<int32_t, std::string> ERROR_MSG_MAP = {
    { ERR_OK,
        "success" },
    { ERR_ABILITY_RUNTIME_EXTERNAL_NO_SUCH_ABILITY_NAME,
        "Input error. The specified ability name not exsit." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_NOT_SUPPORT_OPERATION,
        "Incorrect ability type." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_NO_SUCH_ID,
        "The specified ID does not exist." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_VISIBILITY_VERIFICATION_FAILED,
        "Cannot start an invisible component." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_CROSS_USER_OPERATION,
        "Cannot cross user operations." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY,
        "Service busy. There are concurrent tasks. Try again later." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_CROWDTEST_APP_EXPIRATION,
        "Crowdtest App Expiration." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_WUKONG_MODE,
        "Cannot start ability in wukong mode." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_OPERATION_WITH_CONTINUE_FLAG,
        "Cannot operation with continue flag." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST,
        "Context does not exist." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_ABILITY_ALREADY_AT_TOP,
        "Ability is already at the top." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_CONNECTION_NOT_EXIST,
        "Connection not exist." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_CONNECTION_STATE_ABNORMAL,
        "Connection state is abnormal." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_TIMEOUT,
        "Service timeout." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_APP_UNDER_CONTROL,
        "Application is under control." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR,
        "Internal error." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_NETWORK_ERROR,
        "Network error. The network is abnormal." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_FREE_INSTALL_NOT_SUPPORT,
        "Free install not support. The application does not support free install." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_NOT_TOP_ABILITY,
        "Not top ability. The application is not top ability." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_FREE_INSTALL_BUSY,
        "The installation-free service is busy. Try again later." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_FREE_INSTALL_TIMEOUT,
        "Free install timeout." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_CANNOT_FREE_INSTALL_OTHER_ABILITY,
        "Cannot free install other ability." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_NOT_SUPPORT_CROSS_DEVICE_FREE_INSTALL,
        "Not support cross device free install." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_EXECUTE_SHELL_COMMAND_FAILED,
        "Execute shell command failed." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT,
        "Invalid wantAgent object." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_WANTAGENT_NOT_FOUND,
        "The wantAgent object does not exist." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_NOT_ABILITY_CONTEXT,
        "The context is not ability context." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_NO_SUCH_URI_ABILITY,
        "Input error. The specified uri does not exist." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_FA_NOT_SUPPORT_OPERATION,
        "Ability type error. The specified ability type is wrong." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_CALLER_RELEASED,
        "The caller has been released." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_CALLEE_INVALID,
        "The callee does not exist." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_RELEASE_ERROR,
        "Release error. The caller does not call any callee." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_METHOED_REGISTERED,
        "The method has been registered." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_METHOED_NOT_REGISTERED,
        "The method has not been registered." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_NO_SUCH_MISSION,
        "Mission id error. The specified mission id does not exist." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_NO_SUCH_MISSION_LISTENER,
        "The specified mission listener does not exist." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_NO_SUCH_BUNDLENAME,
        "The bundle does not exist or no patch has been applied." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_NO_SUCH_HQF,
        "The specified hqf is invalid. Hqf may not exist or inaccessible." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_DEPLOY_HQF_FAILED,
        "Deploy hqf failed." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_SWITCH_HQF_FAILED,
        "Switch hqf failed." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_DELETE_HQF_FAILED,
        "Failed to remove the patch package." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_LOAD_PATCH_FAILED,
        "Load patch failed." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_UNLOAD_PATCH_FAILED,
        "Unload patch failed." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_QUICK_FIX_INTERNAL_ERROR,
        "Internal error." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_NO_ACCESS_PERMISSION,
        "The application does not have permission to call the interface." },
    { ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP,
        "The application is not system-app, can not use system-api." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER,
        "Invalid input parameter." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_NO_SUCH_SYSCAP,
        "The specified SystemCapability name was not found." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_START_ABILITY_WAITTING,
        "Another ability is being started. Wait until it finishes starting." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_NOT_SUPPORT_CROSS_APP_START,
        "Redirection to a third-party application is not allowed in API version greater than 11." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_CANNOT_MATCH_ANY_COMPONENT,
        "No matching ability is found." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_URI_FLAG,
        "Invalid URI flag." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_URI_TYPE,
        "Invalid URI type, only support file uri" },
    { ERR_ABILITY_RUNTIME_EXTERNAL_GRANT_URI_PERMISSION,
        "Sandbox application can not grant URI permission." },
    { ERR_ABILITY_RUNTIME_OPERATION_NOT_SUPPORTED,
        "Operation not supported." },
    { ERR_ABILITY_RUNTIME_CHILD_PROCESS_NUMBER_EXCEEDS_UPPER_BOUND,
        "The number of child processes exceeds the upper limit." },
    { ERR_ABILITY_RUNTIME_RESTART_APP_INCORRECT_ABILITY,
        "The target to restart does not belong to the current application or is not a UIAbility." },
    { ERR_ABILITY_RUNTIME_RESTART_APP_FREQUENT,
        "Restart too frequently. Try again at least 3s later." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_NOT_SYSTEM_HSP,
        "The input bundleName and moduleName is not system HSP" },
    { ERR_ABILITY_RUNTIME_EXTERNAL_WANTAGENT_CANCELED,
        "The WantAgent has been canceled." },
    { ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_MODULENAME,
        "Invalid moduleName." },
};
}

bool AbilityRuntimeErrorUtil::Throw(napi_env env, int32_t errCode, const std::string &errMessage)
{
    std::string eMes = errMessage;
    if (eMes.empty()) {
        eMes = ERROR_MSG_MAP.at(errCode);
    }
    napi_value error = nullptr;
    napi_create_error(env, CreateJsValue(env, errCode), CreateJsValue(env, eMes), &error);
    if (error == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "create error");
        return false;
    }
    napi_throw(env, error);
    return true;
}

bool AbilityRuntimeErrorUtil::ThrowByInternalErrCode(napi_env env, int32_t errCode)
{
    if (ERROR_CODE_MAP.find(errCode) == ERROR_CODE_MAP.end()) {
        TAG_LOGE(AAFwkTag::DEFAULT, "Invalid errCode");
        return false;
    }
    return Throw(env, ERROR_CODE_MAP.at(errCode));
}

napi_value AbilityRuntimeErrorUtil::CreateErrorByInternalErrCode(napi_env env, int32_t errCode)
{
    if (ERROR_CODE_MAP.find(errCode) == ERROR_CODE_MAP.end()) {
        TAG_LOGE(AAFwkTag::DEFAULT, "Invalid errCode");
        return nullptr;
    }
    int32_t externalErrCode = ERROR_CODE_MAP.at(errCode);
    return CreateJsError(env, externalErrCode, ERROR_MSG_MAP.at(externalErrCode));
}

std::string AbilityRuntimeErrorUtil::GetErrMessage(int32_t errCode)
{
    auto iter = ERROR_MSG_MAP.find(errCode);
    std::string errMsg = (iter != ERROR_MSG_MAP.end()) ? iter->second : "";
    return errMsg;
}
} // namespace AbilityRuntime
} // namespace OHOS
