/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
     *  Module type: Ability Kit side
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
     * Result(2097154) for Connect State: An error of the ability service not connect.
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
     * Result(2097219) for product application boot setting.
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
     * Result(2097225) for restart too frequently. Try again at least 3s later.
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
     * Result (2097261) for killing process uid is foundation.
     */
    ERR_KILL_FOUNDATION_UID = 2097261,

    /*
     * Result (2097262) caller not exists.
     */
    ERR_CALLER_NOT_EXISTS = 2097262,

    /*
     * Result (2097263) Not support back to caller.
     */
    ERR_NOT_SUPPORT_BACK_TO_CALLER = 2097263,

    /*
     * Result (2097264) for not support child process.
     */
    ERR_NOT_SUPPORT_CHILD_PROCESS = 2097264,

    /*
     * Result (2097265) for already in child process.
     */
    ERR_ALREADY_IN_CHILD_PROCESS = 2097265,

    /*
     * Result (2097266) for child process reach limit.
     */
    ERR_CHILD_PROCESS_REACH_LIMIT = 2097266,

    /**
     * Result(2097267) for check is debug app.
     */
    ERR_NOT_DEBUG_APP = 2097267,

    /*
     * Result (2097268) for native child process reach limit.
     */
    ERR_NOT_SUPPORT_NATIVE_CHILD_PROCESS = 2097268,

    /*
     * Result (2097269) for failed to get profile when execute insight intent.
     */
    ERR_INSIGHT_INTENT_GET_PROFILE_FAILED = 2097269,

    /*
     * Result (2097270) for all apps are blocked from starting due to resource shortage.
     */
    ERR_ALL_APP_START_BLOCKED = 2097270,

    /**
     * Result(2097271) not support multi-instance.
     */
    ERR_MULTI_INSTANCE_NOT_SUPPORTED = 2097271,

    /*
     * Result (2097272) for not support app instance key.
     */
    ERR_APP_INSTANCE_KEY_NOT_SUPPORT = 2097272,

    /*
     * Result (2097273) for reach the upper limit.
     */
    ERR_UPPER_LIMIT = 2097273,

    /*
     * Result (2097274) for not support to create a new instance.
     */
    ERR_CREATE_NEW_INSTANCE_NOT_SUPPORT = 2097274,

    /*
     * Result (2097275) for invalid app instance key.
     */
    ERR_INVALID_APP_INSTANCE_KEY = 2097275,

    /*
     * Result (2097276) for not support app clone.
     */
    ERR_NOT_SUPPORT_APP_CLONE = 2097276,

    /*
     * Result (2097277) for invalid extension type.
     */
    ERR_INVALID_EXTENSION_TYPE = 2097277,

    /*
     * Result (2097278) for replying failed while executing insight intent.
     */
    ERR_INSIGHT_INTENT_EXECUTE_REPLY_FAILED = 2097278,

    /*
     * Result (2097279) for get target bundle info failed.
     */
    ERR_GET_TARGET_BUNDLE_INFO_FAILED = 2097279,

    /*
     * Result (2097280) for UIAbility in starting state.
     */
    ERR_UI_ABILITY_IS_STARTING = 2097280,

    /*
     * Result (2097281) for setSessionManagerService failed.
     */
    SET_SMS_FAILED = 2097281,

    /*
     * Result (2097282) for get appSpawn client failed.
     */
    ERR_GET_SPAWN_CLIENT_FAILED = 2097282,

    /*
     * Result (2097283) for create start msg failed.
     */
    ERR_CREATE_START_MSG_FAILED = 2097283,

    /*
     * Result (2097284) for create spawn process failed.
     */
    ERR_SPAWN_PROCESS_FAILED = 2097284,

    /*
     * Result (2097285 - 2097295) for record exit reason failed.
     */
    ERR_RECORD_SIGNAL_REASON_FAILED = 2097285,
    ERR_GET_EXIT_INFO_FAILED = 2097286,
    ERR_NO_PERMISSION_CALLER = 2097287,
    ERR_NULL_APP_EXIT_REASON_HELPER = 2097288,
    ERR_READ_EXIT_REASON_FAILED = 2097289,
    ERR_IPC_PROXY_WRITE_FAILED = 2097290,
    ERR_INVALID_ACCESS_TOKEN = 2097291,
    ERR_GET_KV_STORE_HANDLE_FAILED = 2097292,

    ERR_WRITE_BOOL_FAILED = 2097295,
    ERR_WRITE_INTERFACE_TOKEN_FAILED = 2097296,
    ERR_WRITE_RESULT_CODE_FAILED = 2097297,
    ERR_READ_RESULT_PARCEL_FAILED = 2097298,
    ERR_NO_ALLOW_OUTSIDE_CALL = 2097299,
    ERR_APP_MGR_SERVICE_NOT_READY = 2097300,
    ERR_NULL_APP_RUNNING_MANAGER = 2097301,
    ERR_NULL_APP_MGR_SERVICE_INNER = 2097302,
    ERR_NULL_APP_MGR_PROXY = 2097303,
    ERR_NO_APP_RECORD = 2097304,

    /*
     * Result (2097306) for get connectManager by userId failed.
     */
    CONNECT_MAMAGER_NOT_FIND_BY_USERID = 2097306,

    /*
     * Result (2097307) for not containsAbility or not find abilityRecord by callerToken.
     */
    INVALID_CALLER_TOKEN = 2097307,

    /*
     * Result (2097308) for extension ability not exist.
     */
    EXTENSION_ABILITY_NOT_EXIST = 2097308,

    /*
     * Result (2097309) for extension ability info not query by uri.
     */
    EXTENSION_ABILITY_INFO_NOT_QUERY_BY_URI = 2097309,

    /*
     * Result (2097310) for cannot minimize or terminate except ui extension ability.
     */
    EXTENSION_TYPE_NOT_UI_EXTENSION = 2097310,

    /*
     * Result (2097311) for get Local deviceId failed.
     */
    GET_LOCAL_DEVICE_ID_FAILED = 2097311,

    /**
     * Result (2097312) for interceptor executer is nullptr.
     */
    ERR_NULL_INTERCEPTOR_EXECUTER = 2097312,

    /**
     * Result (2097313) for after check executer is nullptr.
     */
    ERR_NULL_AFTER_CHECK_EXECUTER = 2097313,

    /**
     * Result (2097314) for mission list manager is nullptr.
     */
    ERR_NULL_MISSION_LIST_MANAGER = 2097314,

    /**
     * Result (2097315) for invalid ability type.
     */
    ERR_ABILITY_TYPE_INVALID = 2097315,

    /**
     * Result (2097316) for ui ability manager is nullptr.
     */
    ERR_NULL_UI_ABILITY_MANAGER = 2097316,

    /**
     * Result (2097317) for session info is nullptr.
     */
    ERR_NULL_SESSION_INFO = 2097317,

    /**
     * Result (2097305) for extension starting ability controlled
     */
    ERR_EXTENSION_START_ABILITY_CONTROLEED = 2097305,

    /**
     * Result (2097318 - 2097328) for login and logout user.
     */
    ERR_LOGOUT_USER_TASK_HANDLE_NULL = 2097318,

    ERR_LOGOUT_USER_APP_MANAGER_NULL = 2097319,

    ERR_LOGOUT_USER_KILL_PROCESS_TIMEOUT = 2097320,

    ERR_NOT_HOOK = 2097321,
    ERR_FROM_WINDOW = 2097322,
    ERR_INVALID_CONTEXT = 2097323,

    INTENT_NOT_EXIST = 2097329,
    INTENT_STATE_NOT_EXECUTING = 2097330,

    /**
     * Native error(2097331) for kill app while starting.
     */

    ERR_KILL_APP_WHILE_STARTING = 2097331,
    
    /**
     * Result (2097332) for target not in app identifier allow list.
     */
    ERR_TARGET_NOT_IN_APP_IDENTIFIER_ALLOW_LIST = 2097332,

    /**
     * Result (2097333) for target has not been started yet.
     */
    ERR_TARGET_NOT_STARTED = 2097333,

    /**
     * Result (2097334) for app selector not exists.
     */
    ERR_APP_SELECTOR_NOT_EXISTS = 2097334,

    /**
     * Result (2097335) for canceled wantagent.
     */
    ERR_WANTAGENT_CANCELED = 2097335,

    /**
     * Result (2097336) for invalid main element type.
     */
    ERR_INVALID_MAIN_ELEMENT_TYPE = 2097336,
    
    /**
     * Result (2097337) for can not change keep alive status.
     */
    ERR_CHANGE_KEEP_ALIVE = 2097337,

    /**
     * Result (2097338) for target bundle is not in u1.
     */
    ERR_NO_U1 = 2097338,

    /**
     * Result (2097339) for kill foreground apps.
     */
    ERR_KILL_APP_WHILE_FOREGROUND = 2097339,

    /**
     * Result (2097340-2097350) for uri permission error result.
     */
    ERR_UPMS_GET_FILE_URIS_BY_KEY_FAILED = 2097340,

    ERR_UPMS_NO_PERMISSION_GRANT_URI = 2097341,

    ERR_UPMS_INVALID_CALLER_TOKENID = 2097342,

    ERR_UPMS_INVALID_TARGET_TOKENID = 2097343,

    ERR_UPMS_ADD_PRIVILEGED_FAILED = 2097344,

    ERR_UPMS_GET_ORI_URI_FAILED = 2097345,

    ERR_UPMS_NOT_FILE_URI = 2097346,

    ERR_UPMS_GRANT_URI_PERMISSION_FAILED = 2097347,

    ERR_UPMS_KEY_IS_NOT_CREATE_BY_CALLER = 2097348,

    ERR_UPMS_SERVICE_NOT_START = 2097349,

    /**
     * Result (2097351) target not in whitelist.
     */
    ERR_KIOSK_MODE_NOT_IN_WHITELIST = 2097351,

    /**
     * Result (2097352) already in kiosk mode.
     */
    ERR_ALREADY_IN_KIOSK_MODE = 2097352,

    /**
     * Result (2097353) system not in kiosk mode when required.
     */
    ERR_NOT_IN_KIOSK_MODE = 2097353,

    /**
     * Result (2097354) for application not in focus.
     */
    ERR_APP_NOT_IN_FOCUS = 2097354,

    ERR_WRITE_KIOSK_UPDATE_APP_LIST_FAILED = 2097355,

    ERR_WRITE_KIOSK_STATUS_FAILED = 2097356,

    /**
     * Result (2097357) for sa interceptor executer is nullptr.
     */
    ERR_NULL_SA_INTERCEPTOR_EXECUTER = 2097357,

    /**
     * Result (2097358) for sa interceptor write rule falied.
     */
    ERR_SA_INTERCEPTOR_WRITE_RULE_FAILED = 2097358,

    /**
     * Result (2097359) for sa interceptor descriptor mismatch.
     */
    ERR_SA_INTERCEPTOR_DESCRIPTOR_MISMATCH = 2097359,

    /**
     * Result (2097360) for sa interceptor write ipc params failed.
     */
    ERR_SA_INTERCEPTOR_WRITE_PARAMS_FAILED = 2097360,

    /*
     * Result(2097361) for ipc remote null.
     */
    ERR_NULL_IPC_REMOTE = 2097361,

    /*
     * Result(2097362) for ipc send request failed.
     */
    ERR_NULL_IPC_SEND_REQUST_FAILED = 2097362,

    /*
     * Result(2097363) for sa interceptor read ipc params failed.
     */
    ERR_SA_INTERCEPTOR_READ_PARAMS_FAILED = 2097363,

    /*
     * Result(2097364) Current process is not running a component configured with "isolationProcess".
     */
    ERR_NOT_ISOLATION_PROCESS = 2097364,

    /*
     * Result(2097365) Current process is already a master process, revocation is not supported.
     */
    ERR_ALREADY_MASTER_PROCESS = 2097365,

     /*
     * Result(2097366) Current process is not a candidate master process, no need to revocation.
     */
    ERR_NOT_CANDIDATE_MASTER_PROCESS = 2097366,

    /*
    * Result(2097367) App is uninstalling or upgrading and can not start new process.
    */
    ERR_UNINSTALLING_OR_UPGRADING_APP = 2097367,

    /*
     * Result(2097368) Not entry module when preload application.
     */
    ERR_PRELOAD_NOT_ENTRY_MODULE = 2097368,

    /**
     * Result (2097370-2097379) for StartUIAbilities Error.
     */
    START_UI_ABILITIES_NOT_SUPPORT_DLP = 2097370,

    START_UI_ABILITIES_NOT_SUPPORT_START_PLUGIN = 2097371,

    START_UI_ABILITIES_NOT_SUPPORT_CREATE_APP_INSTANCE_KEY = 2097372,

    START_UI_ABILITIES_NOT_SUPPORT_OPERATE_REMOTE = 2097373,

    START_UI_ABILITIES_NOT_SUPPORT_CROSS_USER = 2097374,

    START_UI_ABILITIES_NOT_SUPPORT_IMPLICIT_START = 2097375,

    START_UI_ABILITIES_ONLY_SUPPORT_UI_ABILITY = 2097376,

    START_UI_ABILITIES_INTERCEPTOR_CHECK_FAILED = 2097377,

    START_UI_ABILITIES_WAITING_SPECIFIED_CODE = 2097378,

    START_UI_ABILITIES_WANT_LIST_SIZE_ERROR = 2097379,

    /*
     * Result(2097380) for IPC write Int32 failed.
     */
    ERR_WRITE_INT32_FAILED = 2097380,

    ERR_WRITE_STRING_FAILED = 2097381,
    
    START_UI_ABILITIES_SPECIFIED_FAILED_ERROR = 2097382,
    
    ERR_PRELOAD_APP_RECORD_ALREADY_EXIST = 2097383,
    
    ERR_NOTIFY_PRELOAD_ABILITY_NO_CHANGE = 2097384,

    ERR_NOTIFY_PRELOAD_ABILITY_HAS_OTHER = 2097385,

    /**
     * Result (2097386) for the PID of the process created by APPSPAWN is invalid.
     */
    ERR_PROCESS_START_INVALID_PID = 2097386,

    /**
     * Result (2097387) for not expected caller native name.
     */
    ERR_NOT_EXPECTED_NATIVE_CALLER_NAME = 2097387,

    /**
     * Result (2097388) for app not configured preload phase.
     */
    ERR_INVALID_APP_PRELOAD_PHASE = 2097388,

    ERR_DO_CLOSURE_CALLBACK_FAILED = 2097397,

    ERR_DMS_START_REMOTE_FAILED = 2097398,

    ERR_PRELOAD_APP_DATA_ABILITIES_FAILED = 2097399,

    /**
     * Result (2097400) The current process is not a master process.
     */
    ERR_NOT_MASTER_PROCESS = 2097400,

    /**
     * Result (2097401) There is an unfinished onNewProcessRequest.
     */
    ERR_NOT_ON_NEW_PROCESS_REQUEST_DONE = 2097401,

    /**
     * Result (2097402) Generate uid failed.
     */
    ERR_GENERATE_UID_FAILED = 2097402,

    /**
     * Result (2097403) The args of fd is invalid.
     */
    ERR_INVALID_FD = 2097403,

    /**
     * Result (2098000-2098049) Error code refinement INVALID_VALUE.
     */
    ERR_REFINEMENT_INVALID_VALUE_BEGIN = 2098000,

    ERR_CONNECT_MANAGER_NULL_ABILITY_RECORD = 2098001,

    ERR_UI_ABILITY_MANAGER_NULL_ABILITY_RECORD = 2098002,

    ERR_NULL_APP_MGR_CLIENT = 2098004,

    ERR_ADD_START_CONTROL_PARAM_FAILED = 2098005,

    ERR_NULL_FREE_INSTALL_MANAGER = 2098006,

    ERR_GET_MISSION_ID_BY_ABILITY_TOKEN_FAILED = 2098007,

    ERR_INVALID_MISSION_ID = 2098008,
    
    ERR_NULL_SYS_DIALOG_SCHEDULER = 2098009,
    
    ERR_CHECK_CALL_FROM_BACKGROUND_FAILED = 2098010,
    
    ERR_TOO_LARGE_APPINDEXES = 2098011,

    ERR_FREQ_START_ABILITY = 2098012,

    ERR_CALLER_IS_KILLING = 2098013,
    
    ERR_NOTIFY_SCB_PENDING_ACTIVATION_FAILED = 2098014,

    ERR_FREE_INSTALL_REMOTE_WANT = 2098015,

    ERR_REFINEMENT_INVALID_VALUE_END = 2098049,

    /**
     * Result (2098050-2098099) Error code refinement INNERR_ERROR.
     */
    ERR_REFINEMENT_INNER_ERROR_BEGIN = 2098050,

    ERR_APP_MGR_TERMINATTE_ABILITY_FAILED = 2098051,

    ERR_REFINEMENT_INNER_ERROR_END = 2098099,

    /**
     * Result (2099100-2099149) Error code refinement RESOLVE_ABILITY_ERR.
     */
    ERR_REFINEMENT_RESOLVE_ABILITY_BEGIN = 2099100,

    ERR_CHECK_PLUGIN_NULL_RECORD = 2099101,

    ERR_CALLER_IS_PLUGIN_ABILITY = 2099102,

    ERR_CHECK_PLUGIN_INVALID_TYPE = 2099103,

    ERR_REFINEMENT_RESOLVE_ABILITY_END = 2099149,

    /**
     * Result (2099150-2099199) Error code refinement ERR_INVALID_CALLER.
     */
    ERR_REFINEMENT_INVALID_CALLER_BEGIN = 2099150,

    ERR_IS_NOT_SPECIFIED_SA = 2099151,

    ERR_REFINEMENT_INVALID_CALLER_END = 2099199,

    /**
     * Result (2099200) for main window of this ability does not exist.
     */
    ERR_MAIN_WINDOW_NOT_EXIST = 2099200,

    /**
     * Result (2099201) for specified ID does not exist.
     */
    ERR_CODE_INVALID_ID = 2099201,

    /**
     * Result (2099202) for self redirection is not allowed.
     */
    ERR_SELF_REDIRECTION_DISALLOWED = 2099202,

    /**
     * Result (2099210-2099410) Error code for agent runtime.
     */
    // Result (2099210) for agent manager proxy is nullptr.
    ERR_NULL_AGENT_MGR_PROXY = 2099210,

    // Result (2099211) for invalid agent card data.
    ERR_INVALID_AGENT_CARD_DATA = 2099211,

    // Result (2099212) for agent card list is out of range.
    ERR_AGENT_CARD_LIST_OUT_OF_RANGE = 2099212,

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

    /**
     * Native error(29360135) for target bundle has no main ability.
     */
    ERR_NO_MAIN_ABILITY = 29360135,

    /**
     * Native error(29360136) for target app has no status-bar ability.
     */
    ERR_NO_STATUS_BAR_ABILITY = 29360136,

    /**
     * Native error(29360137) for target app is not attached to a status bar.
     */
    ERR_NOT_ATTACHED_TO_STATUS_BAR = 29360137,

    /**
     * Result(29360138) for Connect State: An error of the BMS not connect.
     */
    BMS_NOT_CONNECTED = 29360138,

    /*
     * Result(29360205) for continue freeinstall failed.
     */
    CONTINUE_FREE_INSTALL_FAILED = 29360205,

    /*
     * Result(29360206) for atomic service minimized.
     */
    ATOMIC_SERVICE_MINIMIZED = 29360206,

    /* codes 29360210 - 29360220 are reserved for StartSelfUIAbility with startOptions */
    /*
     * Result(29360210) for write interface code failed.
     */
    ERR_WRITE_INTERFACE_CODE = 29360210,

    /*
     * Result(29360211) for write want failed.
     */
    ERR_WRITE_WANT = 29360211,

    /*
     * Result(29360212) for write startOptions failed.
     */
    ERR_WRITE_START_OPTIONS = 29360212,

    /*
     * Result(29360213) for read want failed.
     */
    ERR_READ_WANT = 29360213,

    /*
     * Result(29360214) for read startOptions failed.
     */
    ERR_READ_START_OPTIONS = 29360214,

    /*
     * Result(29360215) for write StartSelfUIAbility result failed.
     */
    ERR_WRITE_START_SELF_UI_ABILITY_RESULT = 29360215,

    /*
     * Result(29360216) for write caller token failed.
     */
    ERR_WRITE_CALLER_TOKEN_FAILED = 29360216,

    /*
     * Result(29360217) for caller is not atomic service.
     */
    ERR_CALLER_NOT_ATOMIC_SERVICE = 29360217,

    /*
     * Result(29360218) for failed to create instance key from SCB.
     */
    ERR_CREATE_INSTANCE_KEY_FAILED = 29360218,

    /*
     * Result(29360219) for write sa interceptor failed.
     */
    ERR_WRITE_SA_INTERCEPTOR_FAILED = 29360219,

    /*
     * Result(29360219) for SCB intercepted this startup attempt.
     */
    ERR_SCB_INTERCEPTION = 29360220,

    /*
     * Result(29360221) for write int failed.
     */
    ERR_WRITE_INT_FAILED = 29360221,

    /*
     * Result(29360222) for attaching ability failed.
     */
    ERR_ATTACH_ABILITY_THREAD_FAILED = 29360222,

    /*
     * Result(29360223) sa interceptor OnCheckStarting failed.
     */
    ERROR_SA_INTERCEPTOR_START_FAILED = 29360223,

    /*
     * Result(29360224) The UIAbility not belong to caller.
     */
    ERROR_UIABILITY_NOT_BELONG_TO_CALLER = 29360224,

    /*
     * Result(29360225) The UIAbility is already exist, can not start again.
     */
    ERROR_UIABILITY_IS_ALREADY_EXIST = 29360225,

    /*
     * Result (29360226) Invalid pid.
     */
    ERR_INVALID_PID = 29360226,

    /**
     * Undefine error code.
     */
    UNDEFINE_ERROR_CODE = 3,
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_ERRORS_H
