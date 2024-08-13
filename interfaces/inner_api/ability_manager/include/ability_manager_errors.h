/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_ERRORS_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_ERRORS_H

#include <map>

#include "errors.h"

namespace OHOS {
namespace AAFwk {
enum {
    /**
     *  Module type: Ability Manager Service side
     */
    ABILITY_MODULE_TYPE_SERVICE = 0,
    /**
     *  Module type: ABility Kit side
     */
    ABILITY_MODULE_TYPE_KIT = 1,
    /**
     *  Module type: Ability  connection state kit side
     */
    ABILITY_MODULE_TYPE_CONNECTION_STATE_KIT = 2
};

// offset of aafwk error, only be used in this file.
constexpr ErrCode AAFWK_SERVICE_ERR_OFFSET = ErrCodeOffset(SUBSYS_AAFWK, ABILITY_MODULE_TYPE_SERVICE);

enum {
    /**
     * Result(2097152) for StartAbility: An error of the Want could not be resolved
     * to ability info from BMS or DistributedMS.
     */
    RESOLVE_ABILITY_ERR = AAFWK_SERVICE_ERR_OFFSET,
    /**
     * Result(2097153) for Connect: An error of the get ability service.
     */
    GET_ABILITY_SERVICE_FAILED,
    /**
     * Result(2097154) for Connect State: An error of the the ability service not connect.
     */
    ABILITY_SERVICE_NOT_CONNECTED,
    /**
     * Result(2097155) for StartAbility: An error of the Want could not be resolved
     * to app info from BMS or DistributedMS.
     */
    RESOLVE_APP_ERR,
    /**
     * Result(2097156) for StartAbility: The ability to start is already at the top.
     */
    ABILITY_EXISTED,
    /**
     * Result(2097157) for StartAbility: An error to create mission stack.
     */
    CREATE_MISSION_STACK_FAILED,
    /**
     * Result(2097158) for StartAbility: An error to create ability record.
     */
    CREATE_ABILITY_RECORD_FAILED,
    /**
     * Result(2097159) for StartAbility: The ability to start is waiting.
     */
    START_ABILITY_WAITING,
    /**
     * Result(2097160) for TerminateAbility: Don't allow to terminate launcher.
     */
    TERMINATE_LAUNCHER_DENIED,
    /**
     * Result(2097161) for DisconnectAbility: Connection not exist.
     */
    CONNECTION_NOT_EXIST,
    /**
     * Result(2097162) for DisconnectAbility:Connection is invalid state.
     */
    INVALID_CONNECTION_STATE,
    /**
     * Result(2097163) for LoadctAbility:LoadAbility timeout.
     */
    LOAD_ABILITY_TIMEOUT,
    /**
     * Result(2097164) for DisconnectAbility:Connection timeout.
     */
    CONNECTION_TIMEOUT,
    /**
     * Result(2097165) for start service: An error of the get BundleManagerService.
     */
    GET_BUNDLE_MANAGER_SERVICE_FAILED,
    /**
     * Result(2097166) for Remove mission: An error of removing mission.
     */
    REMOVE_MISSION_FAILED,
    /**
     * Result(2097167) for All: An error occurs in server.
     */
    INNER_ERR,
    /**
     * Result(2097168) for Get recent mission: get recent missions failed
     */
    GET_RECENT_MISSIONS_FAILED,
    /**
     * Result(2097169) for Remove stack: Don't allow to remove stack which has launcher ability.
     */
    REMOVE_STACK_LAUNCHER_DENIED,
    /**
     * Result(2097170) for ConnectAbility:target ability is not service ability.
     */
    TARGET_ABILITY_NOT_SERVICE,
    /**
     * Result(2097171) for TerminateAbility:target service has a record of connect. It cannot be stopped.
     */
    TERMINATE_SERVICE_IS_CONNECTED,
    /**
     * Result(2097172) for StartAbility:The ability to start is already activating..
     */
    START_SERVICE_ABILITY_ACTIVATING,
    /**
     * Result(2097173) for move mission to top: An error of moving stack.
     */
    MOVE_MISSION_FAILED,
    /**
     * Result(2097174) for kill process: An error of kill process.
     */
    KILL_PROCESS_FAILED,
    /**
     * Result(2097175) for uninstall app: An error of uninstall app.
     */
    UNINSTALL_APP_FAILED,
    /**
     * Result(2097176) for terminate ability result: An error of terminate service.
     */
    TERMINATE_ABILITY_RESULT_FAILED,
    /**
     * Result(2097177) for check permission failed.
     */
    CHECK_PERMISSION_FAILED,

    /**
     * Result(2097178) for no found abilityrecord by caller
     */
    NO_FOUND_ABILITY_BY_CALLER,

    /**
     * Result(2097179) for ability visible attribute is false.
     */
    ABILITY_VISIBLE_FALSE_DENY_REQUEST,

    /**
     * Result(2097180) for caller is not systemapp.
     */
    CALLER_ISNOT_SYSTEMAPP,

    /**
     * Result(2097181) for get bundleName by uid fail.
     */
    GET_BUNDLENAME_BY_UID_FAIL,

    /**
     * Result(2097182) for mission not found.
     */
    MISSION_NOT_FOUND,

    /**
     * Result(2097183) for get bundle info fail.
     */
    GET_BUNDLE_INFO_FAILED,

    /**
     * Result(2097184) for KillProcess: keep alive process can not be killed
     */
    KILL_PROCESS_KEEP_ALIVE,

    /**
     * Result(2097185) for clear the application data fail.
     */
    CLEAR_APPLICATION_DATA_FAIL,

    // for call ability
    /**
     * Result(2097186) for resolve ability failed, there is no permissions
     */
    RESOLVE_CALL_NO_PERMISSIONS,

    /**
     * Result(2097187) for resolve ability failed, target ability not page or singleton
     */
    RESOLVE_CALL_ABILITY_TYPE_ERR,

    /**
     * Result(2097188) for resolve ability failed, resolve failed.
     */
    RESOLVE_CALL_ABILITY_INNER_ERR,

    /**
     * Result(2097189) for resolve ability failed, resolve failed.
     */
    RESOLVE_CALL_ABILITY_VERSION_ERR,

    /**
     * Result(2097190) for release ability failed, release failed.
     */
    RELEASE_CALL_ABILITY_INNER_ERR,

    /**
     * Result(2097191) for register remote mission listener fail.
     */
    REGISTER_REMOTE_MISSION_LISTENER_FAIL,

    /**
     * Result(2097192) for unregister remote mission listener fail.
     */
    UNREGISTER_REMOTE_MISSION_LISTENER_FAIL,

    /**
     * Result(2097193) for invalid userid.
     */
    INVALID_USERID_VALUE,

    /**
     * Result(2097194) for start user test fail.
     */
    START_USER_TEST_FAIL,

    /**
     * Result(2097195) for send usr1 sig to the process of not response fail.
     */
    SEND_USR1_SIG_FAIL,

    /**
     * Result(2097196) for hidump fail.
     */
    ERR_AAFWK_HIDUMP_ERROR,

    /**
     * Result(2097197) for hidump params are invalid.
     */
    ERR_AAFWK_HIDUMP_INVALID_ARGS,

    /**
     * Result(2097198) for parcel fail.
     */
    ERR_AAFWK_PARCEL_FAIL,

    /**
     * Result(2097199) for for implicit start ability is failed.
     */
    ERR_IMPLICIT_START_ABILITY_FAIL,

    /**
     * Result(2097200) for instance reach to upper limit.
     */
    ERR_REACH_UPPER_LIMIT,

    /**
     * Result(2097201) for window mode.
     */
    ERR_AAFWK_INVALID_WINDOW_MODE,

    /**
     * Result(2097202) for wrong interface call.
     */
    ERR_WRONG_INTERFACE_CALL,

    /**
     * Result(2097203) for crowdtest expired.
     */
    ERR_CROWDTEST_EXPIRED,

    /**
     * Result(2097204) for application abnormal.
     */
    ERR_APP_CONTROLLED,

    /**
     * Result(2097205) for invalid caller.
     */
    ERR_INVALID_CALLER,

    /**
     * Result(2097206) for not allowed continuation flag.
     */
    ERR_INVALID_CONTINUATION_FLAG,

    /**
     * Result(2097207) for not allowed to cross user.
     */
    ERR_CROSS_USER,

    /**
     * Result(2097208) for not granted for static permission.
     */
    ERR_STATIC_CFG_PERMISSION,

    /**
     * Result(2097209) for non-system-app use system-api.
     */
    ERR_NOT_SYSTEM_APP,

    /**
     * Result(2097210) for ecological rule control.
     */
    ERR_ECOLOGICAL_CONTROL_STATUS,

    /**
     * Result(2097211) for app jump interceptor.
     */
    ERR_APP_JUMP_INTERCEPTOR_STATUS,

    /**
     * Result(2097212) for URI flag invalid.
     */
    ERR_CODE_INVALID_URI_FLAG,

    /**
     * Result(2097213) for URI type invalid.
     */
    ERR_CODE_INVALID_URI_TYPE,

    /**
     * Result(2097214) for start not self application.
     */
    ERR_NOT_SELF_APPLICATION,

    /**
     * Result(2097215) for edm application abnormal.
     */
    ERR_EDM_APP_CONTROLLED,

    /**
     * Result(2097216) for sandbox application grant URI permission.
     */
    ERR_CODE_GRANT_URI_PERMISSION,

    /**
     * Result(2097217) for collaborator is empty.
     */
    ERR_COLLABORATOR_NOT_REGISTER,

    /**
     * Result(2097218) for collaborator is empty.
     */
    ERR_COLLABORATOR_NOTIFY_FAILED,

    /**
     * Result(2097219) for prouct application boot setting.
     */
    ERR_NOT_SUPPORTED_PRODUCT_TYPE,
    /**
     * Result(2097220) for starting invalid component.
     */
    ERR_INSIGHT_INTENT_START_INVALID_COMPONENT,

    /**
     * Result(2097221) for developer mode.
     */
    ERR_NOT_DEVELOPER_MODE,

    /**
     * Result(2097222) for get active ability list empty when record exit reason.
     */
    ERR_GET_ACTIVE_ABILITY_LIST_EMPTY,

    /**
     * Result(2097223) for query highest priority ability.
     */
    ERR_QUERY_HIGHEST_PRIORITY_ABILITY,

    /**
     * Result(2097224) for the target to restart does not belong to the current app or is not a UIAbility.
     */
    ERR_RESTART_APP_INCORRECT_ABILITY,

    /**
     * Result(2097225) for restart too frequently. Try again at least 10s later.
     */
    ERR_RESTART_APP_FREQUENT,

    /**
     * Result(2097226) for connect ERMS service failed.
     */
    ERR_CONNECT_ERMS_FAILED,

    /**
     * Result(2097227) for ability is not foreground state.
     */
    ERR_ABILITY_NOT_FOREGROUND,

    /**
     * Result(2097228) for in wukong mode, ability can not move to foreground or background.
     */
    ERR_WUKONG_MODE_CANT_MOVE_STATE,

    /**
     * Result(2097229) for operation not supported on current device.
     */
    ERR_OPERATION_NOT_SUPPORTED_ON_CURRENT_DEVICE,

    /**
     * Result(2097230) for capability not support.
     */
    ERR_CAPABILITY_NOT_SUPPORT,

    /**
     * Result(2097231) for not allow implicit start.
     */
    ERR_NOT_ALLOW_IMPLICIT_START,

    /**
     * Result(2097232) for start options check failed.
     */
    ERR_START_OPTIONS_CHECK_FAILED,

    /**
     * Result(2097233) for ability already running.
     */
    ERR_ABILITY_ALREADY_RUNNING,

    /**
     * Native error(2097234) for not self application.
     */
    ERR_NATIVE_NOT_SELF_APPLICATION,

    /**
     * Native error(2097235) for IPC parcel failed.
     */
    ERR_NATIVE_IPC_PARCEL_FAILED,

    /**
     * Native error(2097236) for ability not found.
     */
    ERR_NATIVE_ABILITY_NOT_FOUND,

    /**
     * Native error(2097237) for ability state check failed.
     */
    ERR_NATIVE_ABILITY_STATE_CHECK_FAILED,
    
    /**
     * Native error(2097238) for kill process not exist.
     */
    ERR_KILL_PROCESS_NOT_EXIST,

    /**
     * Native error(2097239) for start other app failed.
     */
    ERR_START_OTHER_APP_FAILED,

    /**
     * Native error(2097240) for memory size state unchanged.
     */
    ERR_NATIVE_MEMORY_SIZE_STATE_UNCHANGED,

    /**
     * Native error(2097241) for target bundle not exist.
     */
    ERR_TARGET_BUNDLE_NOT_EXIST,

    /**
     * Native error(2097242) for get launch ability info failed.
     */
    ERR_GET_LAUNCH_ABILITY_INFO_FAILED,

    /**
     * Native error(2097243) for check preload conditions failed.
     */
    ERR_CHECK_PRELOAD_CONDITIONS_FAILED,

    ERR_SET_SUPPORTED_PROCESS_CACHE_AGAIN,

    /**
     * Result(2097245) for size of uri list out of range.
     */
    ERR_URI_LIST_OUT_OF_RANGE,

    /**
     * Native error(2097246) for not allow preload by rss.
     */
    ERR_NOT_ALLOW_PRELOAD_BY_RSS,

    /**
     * Result(2097247) for get active extension list empty when record exit reason.
     */
    ERR_GET_ACTIVE_EXTENSION_LIST_EMPTY,

    /**
     * Result(2097248) for get ExtensionName by uid fail.
     */
    GET_EXTENSION_NAME_BY_UID_FAIL,

    /**
     * Native error(2097249) no resident process permissions set.
     */
    ERR_NO_RESIDENT_PERMISSION,

    /**
     * Result(2097250) for app clone index does not exist.
     */
    ERR_APP_CLONE_INDEX_INVALID,

    /**
     * Result(2097251) not support twin.
     */
    ERR_MULTI_APP_NOT_SUPPORTED,

    /**
     * Result(2097252) for unlock screen failed in developer mode.
     */
    ERR_UNLOCK_SCREEN_FAILED_IN_DEVELOPER_MODE,

    /*
     * Result(2097253) for block startup in lock screen.
     */
    ERR_BLOCK_START_FIRST_BOOT_SCREEN_UNLOCK = 2097253,

    /*
     * Result(2097254) for extension blocked by third party app flag
     */
    EXTENSION_BLOCKED_BY_THIRD_PARTY_APP_FLAG = 2097254,

    /*
     * Result(2097255) for extension blocked by service list
     */
    EXTENSION_BLOCKED_BY_SERVICE_LIST = 2097255,

    /*
     * Result(2097256) for non-app-provision mode
     */
    ERR_NOT_IN_APP_PROVISION_MODE = 2097256,
    
    /*
     * Result(2097257) for share file uri non-implicitly
     */
    ERR_SHARE_FILE_URI_NON_IMPLICITLY = 2097257,

    /**
     * Native error(2097258) for target bundle not exist.
     */
    ERR_BUNDLE_NOT_EXIST,

    /*
     * Result(2097259) for open link start abilty default ok.
     */
    ERR_OPEN_LINK_START_ABILITY_DEFAULT_OK = 2097259,

    /*
     * Result (2097260) for target free install task does not exist.
     */
    ERR_FREE_INSTALL_TASK_NOT_EXIST = 2097260,
    
    /*
     * Result (2097261) caller not exists.
     */
    ERR_CALLER_NOT_EXISTS = 2097261,
    
    /*
     * Result (2097262) Not support back to caller.
     */
    ERR_NOT_SUPPORT_BACK_TO_CALLER = 2097262,

    /*
     * Result (2097261) for killing process uid is foundation.
     */
    ERR_KILL_FOUNDATION_UID = 2097261,

    /**
     * Native error(3000000) for target bundle not exist.
     */
    ERR_CODE_NOT_EXIST = 3000000,
};

enum {
    /**
     * Provides a list that does not contain any
     * recent missions that currently are not available to the user.
     */
    RECENT_IGNORE_UNAVAILABLE = 0x0002,
};

enum NativeFreeInstallError {
    FREE_INSTALL_OK = 0,
    /**
     * FA search failed.
     */
    FA_FREE_INSTALL_QUERY_ERROR = -1,

    /**
     * HAG query timeout.
     */
    HAG_QUERY_TIMEOUT = -4,

    /**
     * FA Network unavailable.
     */
    FA_NETWORK_UNAVAILABLE = -2,

    /**
     * FA internal system error.
     */
    FA_FREE_INSTALL_SERVICE_ERROR = 0x820101,

    /**
     * FA distribution center crash.
     */
    FA_CRASH = 0x820102,

    /**
     * FA distribution center processing timeout(30s).
     */
    FA_TIMEOUT = 0x820103,

    /**
     * BMS unknown exception.
     */
    UNKNOWN_EXCEPTION = 0x820104,

    /**
     * It is not supported to pull up PA across applications on the same device
     */
    NOT_SUPPORT_PA_ON_SAME_DEVICE = -11,

    /**
     * FA internal system error.
     */
    FA_INTERNET_ERROR = -3,

    /**
     * The user confirms to jump to the application market upgrade.
     */
    JUMP_TO_THE_APPLICATION_MARKET_UPGRADE = -8,

    /**
     * User gives up.
     */
    USER_GIVES_UP = -7,

    /**
     * Installation error in free installation.
     */
    INSTALLATION_ERROR_IN_FREE_INSTALL = -5,

    /**
     * HAP package download timed out.
     */
    HAP_PACKAGE_DOWNLOAD_TIMED_OUT = -9,

    /**
     * There are concurrent tasks, waiting for retry.
     */
    CONCURRENT_TASKS_WAITING_FOR_RETRY = -6,

    /**
     * FA package does not support free installation.
     */
    FA_PACKAGE_DOES_NOT_SUPPORT_FREE_INSTALL = -10,

    /**
     * The app is not allowed to pull this FA.
     */
    NOT_ALLOWED_TO_PULL_THIS_FA = -901,

    /**
     * Not support cross-device free install PA
     */
    NOT_SUPPORT_CROSS_DEVICE_FREE_INSTALL_PA = -12,

    /**
     * Free install timeout
     */
    FREE_INSTALL_TIMEOUT = 29360300,

    /**
     * Not top ability
     */
    NOT_TOP_ABILITY = 0x500001,

    /**
     * Target bundle name is not exist in targetBundleList.
     */
    TARGET_BUNDLE_NOT_EXIST = 0x500002,

    /**
     * Permission denied.
     */
    DMS_PERMISSION_DENIED = 29360157,

    /**
     * Result(29360176) for component access permission check failed.
     */
    DMS_COMPONENT_ACCESS_PERMISSION_DENIED = 29360176,

    /**
     * Invalid parameters.
     */
    INVALID_PARAMETERS_ERR = 29360128,

    /**
     * Remote DMS is not compatible.
     */
    REMOTE_DEVICE_NOT_COMPATIBLE = 502,

    /**
     * Remote service's device is offline.
     */
    DEVICE_OFFLINE_ERR = 29360142,

    /**
     * Result(29360175) for account access permission check failed.
     */
    DMS_ACCOUNT_ACCESS_PERMISSION_DENIED = 29360175,

    /**
     * Result(29360131) for remote invalid parameters.
     */
    INVALID_REMOTE_PARAMETERS_ERR = 29360131,

    /*
     * Result(29360205) for continue freeinstall failed.
     */
    CONTINUE_FREE_INSTALL_FAILED = 29360205,

    /**
     * Undefine error code.
     */
    UNDEFINE_ERROR_CODE = 3,
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_ERRORS_H
