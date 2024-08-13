/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "ability_business_error.h"

#include <unordered_map>

#include "ability_manager_errors.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AAFwk;
namespace {
constexpr const char* TAG_PERMISSION = " permission:";
constexpr const char* ERROR_MSG_OK = "OK.";
constexpr const char* ERROR_MSG_PERMISSION_DENIED = "The application does not have permission to call the interface.";
constexpr const char* ERROR_MSG_NOT_SYSTEM_APP = "The application is not system-app, can not use system-api.";
constexpr const char* ERROR_MSG_INVALID_PARAM = "Invalid input parameter.";
constexpr const char* ERROR_MSG_CAPABILITY_NOT_SUPPORT = "Capability not support.";
constexpr const char* ERROR_MSG_INNER = "Internal error.";
constexpr const char* ERROR_MSG_RESOLVE_ABILITY = "The specified ability does not exist.";
constexpr const char* ERROR_MSG_INVALID_ABILITY_TYPE = "Incorrect ability type.";
constexpr const char* ERROR_MSG_INVALID_ID = "The specified ID does not exist.";
constexpr const char* ERROR_MSG_INVISIBLE = "Failed to start the invisible ability.";
constexpr const char* ERROR_MSG_STATIC_CFG_PERMISSION = "The specified process does not have the permission.";
constexpr const char* ERROR_MSG_CROSS_USER = "Cross-user operations are not allowed.";
constexpr const char* ERROR_MSG_SERVICE_BUSY = "Service busy. There are concurrent tasks. Try again later.";
constexpr const char* ERROR_MSG_CROWDTEST_EXPIRED = "The crowdtesting application expires.";
constexpr const char* ERROR_MSG_WUKONG_MODE = "An ability cannot be started or stopped in Wukong mode.";
constexpr const char* ERROR_MSG_CONTINUATION_FLAG = "The call with the continuation flag is forbidden.";
constexpr const char* ERROR_MSG_INVALID_CONTEXT = "The context does not exist.";
constexpr const char* ERROR_MSG_CONTROLLED = "The application is controlled.";
constexpr const char* ERROR_MSG_EDM_CONTROLLED = "The application is controlled by EDM.";
constexpr const char* ERROR_MSG_NETWORK_ABNORMAL = "Network error.";
constexpr const char* ERROR_MSG_NOT_SUPPORT_FREE_INSTALL = "Installation-free is not supported.";
constexpr const char* ERROR_MSG_NOT_TOP_ABILITY = "The ability is not on the top of the UI.";
constexpr const char* ERROR_MSG_FREE_INSTALL_TOO_BUSY =
    "The installation-free service is busy. Try again later.";
constexpr const char* ERROR_MSG_FREE_INSTALL_TIMEOUT = "Installation-free timed out.";
constexpr const char* ERROR_MSG_FREE_INSTALL_OTHERS = "Installation-free is not allowed for other applications.";
constexpr const char* ERROR_MSG_FREE_INSTALL_CROSS_DEVICE = "Cross-device installation-free is not supported.";
constexpr const char* ERROR_MSG_INVALID_URI_FLAG = "Invalid URI flag.";
constexpr const char* ERROR_MSG_INVALID_URI_TYPE = "Invalid URI type, only support file Uri.";
constexpr const char* ERROR_MSG_GRANT_URI_PERMISSION = "A sandbox application cannot grant URI permission.";
constexpr const char* ERROR_MSG_OPERATION_NOT_SUPPORTED = "Operation not supported.";
constexpr const char* ERROR_MSG_CHILD_PROCESS_NUMBER_EXCEEDS_UPPER_BOUND =
    "The number of child processes exceeds the upper limit.";
constexpr const char* ERROR_MSG_RESTART_APP_INCORRECT_ABILITY =
    "The target to restart does not belong to the current application or is not a UIAbility.";
constexpr const char* ERROR_MSG_RESTART_APP_FREQUENT = "Restart too frequently. Try again at least 10s later.";
constexpr const char* ERROR_MSG_INVALID_CALLER = "The caller has been released.";
constexpr const char* ERROR_MSG_NO_MISSION_ID = "The specified mission does not exist.";
constexpr const char* ERROR_MSG_NO_MISSION_LISTENER = "The specified mission listener does not exist.";
constexpr const char* ERROR_MSG_START_ABILITY_WAITTING =
    "Another ability is being started. Wait until it finishes starting.";
constexpr const char* ERROR_MSG_NOT_SELF_APPLICATION = "The target application is not the current application.";
constexpr const char* ERROR_MSG_ABILITY_NOT_FOREGROUND =
    "The API can be called only when the ability is running in the foreground.";
constexpr const char* ERROR_MSG_WUKONG_MODE_CANT_MOVE_STATE =
    "An ability cannot switch to the foreground or background in Wukong mode.";
constexpr const char* ERROR_MSG_START_OPTIONS_CHECK_FAILED = "The StartOptions check failed.";
constexpr const char* ERROR_MSG_ABILITY_ALREADY_RUNNING = "The ability is already running.";
constexpr const char* ERROR_MSG_NOT_SUPPORT_CROSS_APP_START =
    "Redirection to a third-party application is not allowed in API version 11 or later.";
constexpr const char* ERROR_MSG_CANNOT_MATCH_ANY_COMPONENT = "No matching ability is found.";
constexpr const char* ERROR_MSG_TARGET_BUNDLE_NOT_EXIST = "The bundle does not exist or no patch has been applied.";
constexpr const char* ERROR_MSG_NO_RESIDENT_PERMISSION =
    "The caller application can only set the resident status of the configured process.";
constexpr const char* ERROR_MSG_MULTI_APP_NOT_SUPPORTED = "App clone or multi-instance is not supported.";
constexpr const char* ERROR_MSG_APP_CLONE_INDEX_INVALID =
    "The target app clone with the specified index does not exist.";
constexpr const char* ERROR_MSG_NOT_APP_CLONE = "The target app is not Clone.";
constexpr const char* ERROR_MSG_EXTENSION_START_THIRD_PARTY_APP_CONTROLLED =
    "The extension can not start the specified third party application.";
constexpr const char* ERROR_MSG_EXTENSION_START_SERVICE_CONTROLLED = "The extension can not start the service.";
constexpr const char* ERROR_MSG_FREE_INSTALL_TASK_NOT_EXIST = "The target free install task does not exist.";

// follow ERR_BUNDLE_MANAGER_BUNDLE_NOT_EXIST of appexecfwk_errors.h in bundle_framework
constexpr int32_t ERR_BUNDLE_MANAGER_BUNDLE_NOT_EXIST = 8521220;

static std::unordered_map<AbilityErrorCode, const char*> ERR_CODE_MAP = {
    { AbilityErrorCode::ERROR_OK, ERROR_MSG_OK },
    { AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED, ERROR_MSG_PERMISSION_DENIED },
    { AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP, ERROR_MSG_NOT_SYSTEM_APP },
    { AbilityErrorCode::ERROR_CODE_INVALID_PARAM, ERROR_MSG_INVALID_PARAM },
    { AbilityErrorCode::ERROR_CODE_CAPABILITY_NOT_SUPPORT, ERROR_MSG_CAPABILITY_NOT_SUPPORT },
    { AbilityErrorCode::ERROR_CODE_INNER, ERROR_MSG_INNER },
    { AbilityErrorCode::ERROR_CODE_RESOLVE_ABILITY, ERROR_MSG_RESOLVE_ABILITY },
    { AbilityErrorCode::ERROR_CODE_INVALID_ABILITY_TYPE, ERROR_MSG_INVALID_ABILITY_TYPE },
    { AbilityErrorCode::ERROR_CODE_INVALID_ID, ERROR_MSG_INVALID_ID },
    { AbilityErrorCode::ERROR_CODE_NO_INVISIBLE_PERMISSION, ERROR_MSG_INVISIBLE },
    { AbilityErrorCode::ERROR_CODE_STATIC_CFG_PERMISSION, ERROR_MSG_STATIC_CFG_PERMISSION },
    { AbilityErrorCode::ERROR_CODE_CROSS_USER, ERROR_MSG_CROSS_USER },
    { AbilityErrorCode::ERROR_CODE_SERVICE_BUSY, ERROR_MSG_SERVICE_BUSY},
    { AbilityErrorCode::ERROR_CODE_CROWDTEST_EXPIRED, ERROR_MSG_CROWDTEST_EXPIRED },
    { AbilityErrorCode::ERROR_CODE_WUKONG_MODE, ERROR_MSG_WUKONG_MODE },
    { AbilityErrorCode::ERROR_CODE_CONTINUATION_FLAG, ERROR_MSG_CONTINUATION_FLAG },
    { AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT, ERROR_MSG_INVALID_CONTEXT },
    { AbilityErrorCode::ERROR_CODE_CONTROLLED, ERROR_MSG_CONTROLLED },
    { AbilityErrorCode::ERROR_CODE_EDM_CONTROLLED, ERROR_MSG_EDM_CONTROLLED },
    { AbilityErrorCode::ERROR_CODE_NETWORK_ABNORMAL, ERROR_MSG_NETWORK_ABNORMAL },
    { AbilityErrorCode::ERROR_CODE_NOT_SUPPORT_FREE_INSTALL, ERROR_MSG_NOT_SUPPORT_FREE_INSTALL },
    { AbilityErrorCode::ERROR_CODE_NOT_TOP_ABILITY, ERROR_MSG_NOT_TOP_ABILITY },
    { AbilityErrorCode::ERROR_CODE_FREE_INSTALL_TOO_BUSY, ERROR_MSG_FREE_INSTALL_TOO_BUSY },
    { AbilityErrorCode::ERROR_CODE_FREE_INSTALL_TIMEOUT, ERROR_MSG_FREE_INSTALL_TIMEOUT },
    { AbilityErrorCode::ERROR_CODE_FREE_INSTALL_OTHERS, ERROR_MSG_FREE_INSTALL_OTHERS },
    { AbilityErrorCode::ERROR_CODE_FREE_INSTALL_CROSS_DEVICE, ERROR_MSG_FREE_INSTALL_CROSS_DEVICE },
    { AbilityErrorCode::ERROR_CODE_INVALID_URI_FLAG, ERROR_MSG_INVALID_URI_FLAG },
    { AbilityErrorCode::ERROR_CODE_INVALID_URI_TYPE, ERROR_MSG_INVALID_URI_TYPE },
    { AbilityErrorCode::ERROR_CODE_GRANT_URI_PERMISSION, ERROR_MSG_GRANT_URI_PERMISSION },
    { AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED, ERROR_MSG_OPERATION_NOT_SUPPORTED },
    { AbilityErrorCode::ERROR_CODE_CHILD_PROCESS_NUMBER_EXCEEDS_UPPER_BOUND,
        ERROR_MSG_CHILD_PROCESS_NUMBER_EXCEEDS_UPPER_BOUND },
    { AbilityErrorCode::ERROR_CODE_RESTART_APP_INCORRECT_ABILITY, ERROR_MSG_RESTART_APP_INCORRECT_ABILITY },
    { AbilityErrorCode::ERROR_CODE_RESTART_APP_FREQUENT, ERROR_MSG_RESTART_APP_FREQUENT },
    { AbilityErrorCode::ERROR_CODE_INVALID_CALLER, ERROR_MSG_INVALID_CALLER },
    { AbilityErrorCode::ERROR_CODE_NO_MISSION_ID, ERROR_MSG_NO_MISSION_ID },
    { AbilityErrorCode::ERROR_CODE_NO_MISSION_LISTENER, ERROR_MSG_NO_MISSION_LISTENER },
    { AbilityErrorCode::ERROR_START_ABILITY_WAITTING, ERROR_MSG_START_ABILITY_WAITTING },
    { AbilityErrorCode::ERROR_NOT_SELF_APPLICATION, ERROR_MSG_NOT_SELF_APPLICATION },
    { AbilityErrorCode::ERROR_CODE_ABILITY_NOT_FOREGROUND, ERROR_MSG_ABILITY_NOT_FOREGROUND },
    { AbilityErrorCode::ERROR_CODE_WUKONG_MODE_CANT_MOVE_STATE, ERROR_MSG_WUKONG_MODE_CANT_MOVE_STATE },
    { AbilityErrorCode::ERROR_START_OPTIONS_CHECK_FAILED, ERROR_MSG_START_OPTIONS_CHECK_FAILED },
    { AbilityErrorCode::ERROR_ABILITY_ALREADY_RUNNING, ERROR_MSG_ABILITY_ALREADY_RUNNING },
    { AbilityErrorCode::ERROR_CODE_NOT_SUPPORT_CROSS_APP_START, ERROR_MSG_NOT_SUPPORT_CROSS_APP_START },
    { AbilityErrorCode::ERROR_CODE_CANNOT_MATCH_ANY_COMPONENT, ERROR_MSG_CANNOT_MATCH_ANY_COMPONENT },
    { AbilityErrorCode::ERROR_CODE_TARGET_BUNDLE_NOT_EXIST, ERROR_MSG_TARGET_BUNDLE_NOT_EXIST },
    { AbilityErrorCode::ERROR_CODE_NO_RESIDENT_PERMISSION, ERROR_MSG_NO_RESIDENT_PERMISSION },
    { AbilityErrorCode::ERROR_CODE_MULTI_APP_NOT_SUPPORTED, ERROR_MSG_MULTI_APP_NOT_SUPPORTED },
    { AbilityErrorCode::ERROR_APP_CLONE_INDEX_INVALID, ERROR_MSG_APP_CLONE_INDEX_INVALID },
    { AbilityErrorCode::ERROR_NOT_APP_CLONE, ERROR_MSG_NOT_APP_CLONE },
    { AbilityErrorCode::ERROR_CODE_EXTENSION_START_THIRD_PARTY_APP_CONTROLLED,
        ERROR_MSG_EXTENSION_START_THIRD_PARTY_APP_CONTROLLED },
    { AbilityErrorCode::ERROR_CODE_EXTENSION_START_SERVICE_CONTROLLED, ERROR_MSG_EXTENSION_START_SERVICE_CONTROLLED},
    { AbilityErrorCode::ERROR_CODE_BUNDLE_NAME_INVALID, ERROR_MSG_TARGET_BUNDLE_NOT_EXIST},
    { AbilityErrorCode::ERROR_CODE_FREE_INSTALL_TASK_NOT_EXIST, ERROR_MSG_FREE_INSTALL_TASK_NOT_EXIST }
};

static std::unordered_map<int32_t, AbilityErrorCode> INNER_TO_JS_ERROR_CODE_MAP {
    {0, AbilityErrorCode::ERROR_OK},
    {CHECK_PERMISSION_FAILED, AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED},
    {ERR_PERMISSION_DENIED, AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED},
    {ERR_NOT_SYSTEM_APP, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP},
    {RESOLVE_ABILITY_ERR, AbilityErrorCode::ERROR_CODE_RESOLVE_ABILITY},
    {ERR_WRONG_INTERFACE_CALL, AbilityErrorCode::ERROR_CODE_INVALID_ABILITY_TYPE},
    {TARGET_ABILITY_NOT_SERVICE, AbilityErrorCode::ERROR_CODE_INVALID_ABILITY_TYPE},
    {RESOLVE_CALL_ABILITY_TYPE_ERR, AbilityErrorCode::ERROR_CODE_INVALID_ABILITY_TYPE},
    {ABILITY_VISIBLE_FALSE_DENY_REQUEST, AbilityErrorCode::ERROR_CODE_NO_INVISIBLE_PERMISSION},
    {ERR_STATIC_CFG_PERMISSION, AbilityErrorCode::ERROR_CODE_STATIC_CFG_PERMISSION},
    {ERR_CROSS_USER, AbilityErrorCode::ERROR_CODE_CROSS_USER},
    {ERR_CROWDTEST_EXPIRED, AbilityErrorCode::ERROR_CODE_CROWDTEST_EXPIRED},
    {ERR_WOULD_BLOCK, AbilityErrorCode::ERROR_CODE_WUKONG_MODE},
    {ERR_INVALID_CONTINUATION_FLAG, AbilityErrorCode::ERROR_CODE_CONTINUATION_FLAG},
    {ERR_INVALID_CALLER, AbilityErrorCode::ERROR_CODE_INVALID_CALLER},
    {ERR_CODE_INVALID_URI_FLAG, AbilityErrorCode::ERROR_CODE_INVALID_URI_FLAG},
    {ERR_CODE_INVALID_URI_TYPE, AbilityErrorCode::ERROR_CODE_INVALID_URI_TYPE},
    {ERR_CODE_GRANT_URI_PERMISSION, AbilityErrorCode::ERROR_CODE_GRANT_URI_PERMISSION},
    {ERR_NOT_SELF_APPLICATION, AbilityErrorCode::ERROR_NOT_SELF_APPLICATION},
    // Installation-free error code transfer
    {HAP_PACKAGE_DOWNLOAD_TIMED_OUT, AbilityErrorCode::ERROR_CODE_NETWORK_ABNORMAL},
    {FA_PACKAGE_DOES_NOT_SUPPORT_FREE_INSTALL, AbilityErrorCode::ERROR_CODE_NOT_SUPPORT_FREE_INSTALL},
    {NOT_TOP_ABILITY, AbilityErrorCode::ERROR_CODE_NOT_TOP_ABILITY},
    {CONCURRENT_TASKS_WAITING_FOR_RETRY, AbilityErrorCode::ERROR_CODE_FREE_INSTALL_TOO_BUSY},
    {FREE_INSTALL_TIMEOUT, AbilityErrorCode::ERROR_CODE_FREE_INSTALL_TIMEOUT},
    {NOT_ALLOWED_TO_PULL_THIS_FA, AbilityErrorCode::ERROR_CODE_FREE_INSTALL_OTHERS},
    {MISSION_NOT_FOUND, AbilityErrorCode::ERROR_CODE_NO_MISSION_ID},
    {FA_TIMEOUT, AbilityErrorCode::ERROR_CODE_FREE_INSTALL_TIMEOUT},
    {NOT_SUPPORT_CROSS_DEVICE_FREE_INSTALL_PA, AbilityErrorCode::ERROR_CODE_FREE_INSTALL_CROSS_DEVICE},
    {TARGET_BUNDLE_NOT_EXIST, AbilityErrorCode::ERROR_CODE_RESOLVE_ABILITY},
    {DMS_PERMISSION_DENIED, AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED},
    {DMS_COMPONENT_ACCESS_PERMISSION_DENIED, AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED},
    {INVALID_PARAMETERS_ERR, AbilityErrorCode::ERROR_CODE_INVALID_PARAM},
    {DMS_ACCOUNT_ACCESS_PERMISSION_DENIED, AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED},
    {START_ABILITY_WAITING, AbilityErrorCode::ERROR_START_ABILITY_WAITTING},
    {ERR_APP_CONTROLLED, AbilityErrorCode::ERROR_CODE_CONTROLLED},
    {ERR_EDM_APP_CONTROLLED, AbilityErrorCode::ERROR_CODE_EDM_CONTROLLED},
    {ERR_INSIGHT_INTENT_START_INVALID_COMPONENT, AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED},
    {ERR_RESTART_APP_INCORRECT_ABILITY, AbilityErrorCode::ERROR_CODE_RESTART_APP_INCORRECT_ABILITY},
    {ERR_RESTART_APP_FREQUENT, AbilityErrorCode::ERROR_CODE_RESTART_APP_FREQUENT},
    {ERR_CAPABILITY_NOT_SUPPORT, AbilityErrorCode::ERROR_CODE_CAPABILITY_NOT_SUPPORT},
    {ERR_NOT_ALLOW_IMPLICIT_START, AbilityErrorCode::ERROR_CODE_RESOLVE_ABILITY},
    {ERR_START_OPTIONS_CHECK_FAILED, AbilityErrorCode::ERROR_START_OPTIONS_CHECK_FAILED},
    {ERR_ABILITY_ALREADY_RUNNING, AbilityErrorCode::ERROR_ABILITY_ALREADY_RUNNING},
    {ERR_BUNDLE_MANAGER_BUNDLE_NOT_EXIST, AbilityErrorCode::ERROR_CODE_INVALID_ID},
    {ERR_ABILITY_NOT_FOREGROUND, AbilityErrorCode::ERROR_CODE_ABILITY_NOT_FOREGROUND},
    {ERR_WUKONG_MODE_CANT_MOVE_STATE, AbilityErrorCode::ERROR_CODE_WUKONG_MODE_CANT_MOVE_STATE},
    {ERR_OPERATION_NOT_SUPPORTED_ON_CURRENT_DEVICE, AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED},
    {ERR_IMPLICIT_START_ABILITY_FAIL, AbilityErrorCode::ERROR_CODE_CANNOT_MATCH_ANY_COMPONENT},
    {ERR_START_OTHER_APP_FAILED, AbilityErrorCode::ERROR_CODE_NOT_SUPPORT_CROSS_APP_START},
    {ERR_TARGET_BUNDLE_NOT_EXIST, AbilityErrorCode::ERROR_CODE_TARGET_BUNDLE_NOT_EXIST},
    {ERR_NO_RESIDENT_PERMISSION, AbilityErrorCode::ERROR_CODE_NO_RESIDENT_PERMISSION},
    {ERR_MULTI_APP_NOT_SUPPORTED, AbilityErrorCode::ERROR_CODE_MULTI_APP_NOT_SUPPORTED},
    {ERR_APP_CLONE_INDEX_INVALID, AbilityErrorCode::ERROR_APP_CLONE_INDEX_INVALID},
    {EXTENSION_BLOCKED_BY_THIRD_PARTY_APP_FLAG,
        AbilityErrorCode::ERROR_CODE_EXTENSION_START_THIRD_PARTY_APP_CONTROLLED},
    {EXTENSION_BLOCKED_BY_SERVICE_LIST, AbilityErrorCode::ERROR_CODE_EXTENSION_START_SERVICE_CONTROLLED},
    {ERR_BUNDLE_NOT_EXIST, AbilityErrorCode::ERROR_CODE_BUNDLE_NAME_INVALID},
    {ERR_FREE_INSTALL_TASK_NOT_EXIST, AbilityErrorCode::ERROR_CODE_FREE_INSTALL_TASK_NOT_EXIST},
};
}

std::string GetErrorMsg(const AbilityErrorCode& errCode)
{
    auto it = ERR_CODE_MAP.find(errCode);
    if (it != ERR_CODE_MAP.end()) {
        return it->second;
    }

    return "";
}

std::string GetNoPermissionErrorMsg(const std::string& permission)
{
    return std::string(ERROR_MSG_PERMISSION_DENIED) + std::string(TAG_PERMISSION) + permission;
}

AbilityErrorCode GetJsErrorCodeByNativeError(int32_t errCode)
{
    auto it = INNER_TO_JS_ERROR_CODE_MAP.find(errCode);
    if (it != INNER_TO_JS_ERROR_CODE_MAP.end()) {
        return it->second;
    }

    return AbilityErrorCode::ERROR_CODE_INNER;
}
}  // namespace AbilityRuntime
}  // namespace OHOS