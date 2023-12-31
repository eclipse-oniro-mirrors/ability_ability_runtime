/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_MGR_IPC_INTERFACE_CODE_H
#define OHOS_ABILITY_RUNTIME_APP_MGR_IPC_INTERFACE_CODE_H

/* SAID:501 */
namespace OHOS {
namespace AppExecFwk {
enum class AppMgrInterfaceCode {
    // please add new code to the bottom in order to prevent some unexpected BUG
    APP_ATTACH_APPLICATION = 0,
    APP_APPLICATION_FOREGROUNDED,
    APP_APPLICATION_BACKGROUNDED,
    APP_APPLICATION_TERMINATED,
    APP_CHECK_PERMISSION,
    APP_ABILITY_CLEANED,
    APP_GET_MGR_INSTANCE,
    APP_CLEAR_UP_APPLICATION_DATA,
    APP_GET_ALL_RUNNING_PROCESSES,
    APP_GET_RUNNING_PROCESSES_BY_USER_ID,
    APP_ADD_ABILITY_STAGE_INFO_DONE,
    STARTUP_RESIDENT_PROCESS,
    REGISTER_APPLICATION_STATE_OBSERVER,
    UNREGISTER_APPLICATION_STATE_OBSERVER,
    GET_FOREGROUND_APPLICATIONS,
    START_USER_TEST_PROCESS,
    FINISH_USER_TEST,
    SCHEDULE_ACCEPT_WANT_DONE,
    BLOCK_APP_SERVICE,
    APP_GET_ABILITY_RECORDS_BY_PROCESS_ID,
    START_RENDER_PROCESS,
    ATTACH_RENDER_PROCESS,
    GET_RENDER_PROCESS_TERMINATION_STATUS,
    GET_CONFIGURATION,
    UPDATE_CONFIGURATION,
    REGISTER_CONFIGURATION_OBSERVER,
    UNREGISTER_CONFIGURATION_OBSERVER,
    APP_NOTIFY_MEMORY_LEVEL,
    GET_APP_RUNNING_STATE,
    NOTIFY_LOAD_REPAIR_PATCH,
    NOTIFY_HOT_RELOAD_PAGE,
    SET_CONTINUOUSTASK_PROCESS,
    NOTIFY_UNLOAD_REPAIR_PATCH,
    PRE_START_NWEBSPAWN_PROCESS,
    APP_GET_PROCESS_RUNNING_INFORMATION,
    IS_SHARED_BUNDLE_RUNNING,
    DUMP_HEAP_MEMORY_PROCESS,
    START_NATIVE_PROCESS_FOR_DEBUGGER,
    NOTIFY_APP_FAULT,
    NOTIFY_APP_FAULT_BY_SA,
    JUDGE_SANDBOX_BY_PID,
    GET_BUNDLE_NAME_BY_PID,
    APP_GET_ALL_RENDER_PROCESSES,
};
} // AppExecFwk
} // OHOS
#endif // OHOS_ABILITY_RUNTIME_APP_MGR_IPC_INTERFACE_CODE_H