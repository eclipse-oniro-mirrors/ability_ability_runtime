/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "app_mgr_stub.h"

#include "ability_info.h"
#include "ability_manager_errors.h"
#include "app_jsheap_mem_info.h"
#include "app_cjheap_mem_info.h"
#include "app_malloc_info.h"
#include "app_mgr_proxy.h"
#include "app_scheduler_interface.h"
#include "app_utils.h"
#include "appexecfwk_errors.h"
#include "bundle_info.h"
#ifdef SUPPORT_CHILD_PROCESS
#include "child_process_request.h"
#endif // SUPPORT_CHILD_PROCESS
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "iapp_state_callback.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iremote_object.h"
#include "memory_level_info.h"
#include "running_process_info.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
constexpr int32_t CYCLE_LIMIT = 1000;
constexpr int32_t MAX_PROCESS_STATE_COUNT = 1000;
constexpr int32_t MAX_BACKGROUND_APP_COUNT = 1000;

AppMgrStub::AppMgrStub() {}

AppMgrStub::~AppMgrStub() {}

int AppMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppMgrStub::OnReceived, code = %{public}u, flags= %{public}d.", code,
        option.GetFlags());
    std::u16string descriptor = AppMgrStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::APPMGR, "local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }
    return OnRemoteRequestInner(code, data, reply, option);
}

int32_t AppMgrStub::OnRemoteRequestInner(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    int retCode = ERR_OK;
    retCode = OnRemoteRequestInnerFirst(code, data, reply, option);
    if (retCode != INVALID_FD) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerSecond(code, data, reply, option);
    if (retCode != INVALID_FD) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerThird(code, data, reply, option);
    if (retCode != INVALID_FD) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerFourth(code, data, reply, option);
    if (retCode != INVALID_FD) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerFifth(code, data, reply, option);
    if (retCode != INVALID_FD) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerSixth(code, data, reply, option);
    if (retCode != INVALID_FD) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerSeventh(code, data, reply, option);
    if (retCode != INVALID_FD) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerEighth(code, data, reply, option);
    if (retCode != INVALID_FD) {
        return retCode;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "AppMgrStub::OnRemoteRequest end");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t AppMgrStub::OnRemoteRequestInnerFirst(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    switch (static_cast<uint32_t>(code)) {
        case static_cast<uint32_t>(AppMgrInterfaceCode::APP_ATTACH_APPLICATION):
            return HandleAttachApplication(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::PRELOAD_APPLICATION):
            return HandlePreloadApplication(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::APP_APPLICATION_FOREGROUNDED):
            return HandleApplicationForegrounded(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::APP_APPLICATION_BACKGROUNDED):
            return HandleApplicationBackgrounded(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::APP_APPLICATION_TERMINATED):
            return HandleApplicationTerminated(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::APP_ABILITY_CLEANED):
            return HandleAbilityCleaned(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_MGR_INSTANCE):
            return HandleGetAmsMgr(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::APP_CLEAR_UP_APPLICATION_DATA):
            return HandleClearUpApplicationData(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_ALL_RUNNING_PROCESSES):
            return HandleGetAllRunningProcesses(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::APP_NOTIFY_MEMORY_LEVEL):
            return HandleNotifyMemoryLevel(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::APP_NOTIFY_PROC_MEMORY_LEVEL):
            return HandleNotifyProcMemoryLevel(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_RUNNING_PROCESSES_BY_USER_ID):
            return HandleGetProcessRunningInfosByUserId(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::APP_ADD_ABILITY_STAGE_INFO_DONE):
            return HandleAddAbilityStageDone(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::STARTUP_RESIDENT_PROCESS):
            return HandleStartupResidentProcess(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::REGISTER_APPLICATION_STATE_OBSERVER):
            return HandleRegisterApplicationStateObserver(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::UNREGISTER_APPLICATION_STATE_OBSERVER):
            return HandleUnregisterApplicationStateObserver(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::PRELOAD_MODULE_FINISHED):
            return HandlePreloadModuleFinished(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::REGISTER_APPLICATION_STATE_OBSERVER_WITH_FILTER):
            return HandleRegisterApplicationStateObserverWithFilter(data, reply);
    }
    return INVALID_FD;
}

int32_t AppMgrStub::OnRemoteRequestInnerSecond(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    switch (static_cast<uint32_t>(code)) {
        case static_cast<uint32_t>(AppMgrInterfaceCode::GET_FOREGROUND_APPLICATIONS):
            return HandleGetForegroundApplications(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::START_USER_TEST_PROCESS):
            return HandleStartUserTestProcess(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::FINISH_USER_TEST):
            return HandleFinishUserTest(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::SCHEDULE_ACCEPT_WANT_DONE):
            return HandleScheduleAcceptWantDone(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::SCHEDULE_NEW_PROCESS_REQUEST_DONE):
            return HandleScheduleNewProcessRequestDone(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_ABILITY_RECORDS_BY_PROCESS_ID):
            return HandleGetAbilityRecordsByProcessID(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::PRE_START_NWEBSPAWN_PROCESS):
            return HandlePreStartNWebSpawnProcess(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::START_RENDER_PROCESS):
            return HandleStartRenderProcess(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::ATTACH_RENDER_PROCESS):
            return HandleAttachRenderProcess(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::GET_RENDER_PROCESS_TERMINATION_STATUS):
            return HandleGetRenderProcessTerminationStatus(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::GET_CONFIGURATION):
            return HandleGetConfiguration(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::UPDATE_CONFIGURATION):
            return HandleUpdateConfiguration(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::REGISTER_CONFIGURATION_OBSERVER):
            return HandleRegisterConfigurationObserver(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::UNREGISTER_CONFIGURATION_OBSERVER):
            return HandleUnregisterConfigurationObserver(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_PROCESS_RUNNING_INFORMATION):
            return HandleGetProcessRunningInformation(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::DUMP_HEAP_MEMORY_PROCESS):
            return HandleDumpHeapMemory(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::DUMP_JSHEAP_MEMORY_PROCESS):
            return HandleDumpJsHeapMemory(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::DUMP_CJHEAP_MEMORY_PROCESS):
            return HandleDumpCjHeapMemory(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::GET_RUNNING_MULTIAPP_INFO_BY_BUNDLENAME):
            return HandleGetRunningMultiAppInfoByBundleName(data, reply);
    }
    return INVALID_FD;
}

int32_t AppMgrStub::OnRemoteRequestInnerThird(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    switch (static_cast<uint32_t>(code)) {
        case static_cast<uint32_t>(AppMgrInterfaceCode::GET_APP_RUNNING_STATE):
            return HandleGetAppRunningStateByBundleName(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_LOAD_REPAIR_PATCH):
            return HandleNotifyLoadRepairPatch(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_HOT_RELOAD_PAGE):
            return HandleNotifyHotReloadPage(data, reply);
    #ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
        case static_cast<uint32_t>(AppMgrInterfaceCode::SET_CONTINUOUSTASK_PROCESS):
            return HandleSetContinuousTaskProcess(data, reply);
    #endif
        case static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_UNLOAD_REPAIR_PATCH):
            return HandleNotifyUnLoadRepairPatch(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::IS_SHARED_BUNDLE_RUNNING):
            return HandleIsSharedBundleRunning(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::START_NATIVE_PROCESS_FOR_DEBUGGER):
            return HandleStartNativeProcessForDebugger(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_APP_FAULT):
            return HandleNotifyFault(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::GET_All_RUNNING_INSTANCE_KEYS_BY_BUNDLENAME):
            return HandleGetAllRunningInstanceKeysByBundleName(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::GET_All_RUNNING_INSTANCE_KEYS_BY_SELF):
            return HandleGetAllRunningInstanceKeysBySelf(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::UPDATE_CONFIGURATION_MULTI_USER):
            return HandleUpdateConfigurationMultiUser(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::GET_CONFIGURATION_BY_USERID):
            return HandleGetConfigurationByUserId(data, reply);
    }
    return INVALID_FD;
}

int32_t AppMgrStub::OnRemoteRequestInnerFourth(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    switch (static_cast<uint32_t>(code)) {
    #ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
        case static_cast<uint32_t>(AppMgrInterfaceCode::SET_CONTINUOUSTASK_PROCESS):
            return HandleSetContinuousTaskProcess(data, reply);
    #endif
        case static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_APP_FAULT_BY_SA):
            return HandleNotifyFaultBySA(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::SET_APPFREEZE_FILTER):
            return HandleSetAppFreezeFilter(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::GET_BUNDLE_NAME_BY_PID):
            return HandleGetBundleNameByPid(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::GET_RUNNING_PROCESS_INFO_BY_PID):
            return HandleGetRunningProcessInfoByPid(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::GET_RUNNING_PROCESS_INFO_BY_CHILD_PROCESS_PID):
            return HandleGetRunningProcessInfoByChildProcessPid(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_ALL_RENDER_PROCESSES):
            return HandleGetAllRenderProcesses(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::GET_PROCESS_MEMORY_BY_PID):
            return HandleGetProcessMemoryByPid(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::IS_TERMINATING_BY_PID):
            return HandleIsTerminatingByPid(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::IS_SPECIFIED_MODULE_LOADED):
            return HandleIsSpecifiedModuleLoaded(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::SIGN_RESTART_PROCESS):
            return HandleSignRestartProcess(data, reply);
    }
    return INVALID_FD;
}

int32_t AppMgrStub::OnRemoteRequestInnerFifth(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    switch (static_cast<uint32_t>(code)) {
    #ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
        case static_cast<uint32_t>(AppMgrInterfaceCode::SET_CONTINUOUSTASK_PROCESS):
            return HandleSetContinuousTaskProcess(data, reply);
    #endif
        case static_cast<uint32_t>(AppMgrInterfaceCode::GET_PIDS_BY_BUNDLENAME):
            return HandleGetRunningProcessInformation(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::CHANGE_APP_GC_STATE):
            return HandleChangeAppGcState(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_PAGE_SHOW):
            return HandleNotifyPageShow(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_PAGE_HIDE):
            return HandleNotifyPageHide(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::REGISTER_APP_RUNNING_STATUS_LISTENER):
            return HandleRegisterAppRunningStatusListener(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::UNREGISTER_APP_RUNNING_STATUS_LISTENER):
            return HandleUnregisterAppRunningStatusListener(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::REGISTER_APP_FOREGROUND_STATE_OBSERVER):
            return HandleRegisterAppForegroundStateObserver(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::UNREGISTER_APP_FOREGROUND_STATE_OBSERVER):
            return HandleUnregisterAppForegroundStateObserver(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::REGISTER_ABILITY_FOREGROUND_STATE_OBSERVER):
            return HandleRegisterAbilityForegroundStateObserver(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::UNREGISTER_ABILITY_FOREGROUND_STATE_OBSERVER):
            return HandleUnregisterAbilityForegroundStateObserver(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::IS_APPLICATION_RUNNING):
            return HandleIsApplicationRunning(data, reply);
    #ifdef SUPPORT_CHILD_PROCESS
        case static_cast<uint32_t>(AppMgrInterfaceCode::START_CHILD_PROCESS):
            return HandleStartChildProcess(data, reply);
    #endif // SUPPORT_CHILD_PROCESS
        case static_cast<uint32_t>(AppMgrInterfaceCode::REGISTER_NATIVE_CHILD_EXIT_NOTIFY):
            return HandleRegisterNativeChildExitNotify(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::UNREGISTER_NATIVE_CHILD_EXIT_NOTIFY):
            return HandleUnregisterNativeChildExitNotify(data, reply);
    }
    return INVALID_FD;
}

int32_t AppMgrStub::OnRemoteRequestInnerSixth(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    switch (static_cast<uint32_t>(code)) {
    #ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
        case static_cast<uint32_t>(AppMgrInterfaceCode::SET_CONTINUOUSTASK_PROCESS):
            return HandleSetContinuousTaskProcess(data, reply);
    #endif
    #ifdef SUPPORT_CHILD_PROCESS
        case static_cast<uint32_t>(AppMgrInterfaceCode::GET_CHILD_PROCCESS_INFO_FOR_SELF):
            return HandleGetChildProcessInfoForSelf(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::ATTACH_CHILD_PROCESS):
            return HandleAttachChildProcess(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::EXIT_CHILD_PROCESS_SAFELY):
            return HandleExitChildProcessSafely(data, reply);
    #endif // SUPPORT_CHILD_PROCESS
        case static_cast<uint32_t>(AppMgrInterfaceCode::IS_FINAL_APP_PROCESS):
            return HandleIsFinalAppProcess(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::APP_CLEAR_UP_APPLICATION_DATA_BY_SELF):
            return HandleClearUpApplicationDataBySelf(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::REGISTER_RENDER_STATUS_OBSERVER):
            return HandleRegisterRenderStateObserver(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::UNREGISTER_RENDER_STATUS_OBSERVER):
            return HandleUnregisterRenderStateObserver(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::UPDATE_RENDER_STATUS):
            return HandleUpdateRenderState(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::SIGN_RESTART_APP_FLAG):
            return HandleSignRestartAppFlag(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::GET_APP_RUNNING_UNIQUE_ID_BY_PID):
            return HandleGetAppRunningUniqueIdByPid(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::GET_ALL_UI_EXTENSION_ROOT_HOST_PID):
            return HandleGetAllUIExtensionRootHostPid(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::GET_ALL_UI_EXTENSION_PROVIDER_PID):
            return HandleGetAllUIExtensionProviderPid(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::IS_APP_RUNNING_BY_BUNDLE_NAME_AND_USER_ID):
            return HandleIsAppRunningByBundleNameAndUserId(data, reply);
    }
    return INVALID_FD;
}

int32_t AppMgrStub::OnRemoteRequestInnerSeventh(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    switch (static_cast<uint32_t>(code)) {
    #ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
        case static_cast<uint32_t>(AppMgrInterfaceCode::SET_CONTINUOUSTASK_PROCESS):
            return HandleSetContinuousTaskProcess(data, reply);
    #endif
        case static_cast<uint32_t>(AppMgrInterfaceCode::UPDATE_CONFIGURATION_BY_BUNDLE_NAME):
            return HandleUpdateConfigurationByBundleName(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_MEMORY_SIZE_STATE_CHANGED):
            return HandleNotifyMemorySizeStateChanged(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::SET_SUPPORTED_PROCESS_CACHE_SELF):
            return HandleSetSupportedProcessCacheSelf(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::SET_SUPPORTED_PROCESS_CACHE):
            return HandleSetSupportedProcessCache(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::IS_PROCESS_CACHE_SUPPORTED):
            return HandleIsProcessCacheSupported(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::SET_PROCESS_CACHE_ENABLE):
            return HandleSetProcessCacheEnable(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_RUNNING_PROCESSES_BY_BUNDLE_TYPE):
            return HandleGetRunningProcessesByBundleType(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::SET_APP_ASSERT_PAUSE_STATE_SELF):
            return HandleSetAppAssertionPauseState(data, reply);
    #ifdef SUPPORT_CHILD_PROCESS
        case static_cast<uint32_t>(AppMgrInterfaceCode::CREATE_NATIVE_CHILD_PROCESS):
            return HandleCreateNativeChildProcess(data, reply);
    #endif // SUPPORT_CHILD_PROCESS
        case static_cast<uint32_t>(AppMgrInterfaceCode::SAVE_BROWSER_CHANNEL):
            return HandleSaveBrowserChannel(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::IS_APP_RUNNING):
            return HandleIsAppRunning(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::IS_APP_RUNNING_BY_USER_ID):
            return HandleIsAppRunningByUserId(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::CHECK_CALLING_IS_USER_TEST_MODE):
            return HandleCheckCallingIsUserTestMode(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_PROCESS_DEPENDED_ON_WEB):
            return HandleNotifyProcessDependedOnWeb(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::KILL_PROCESS_DEPENDED_ON_WEB):
            return HandleKillProcessDependedOnWeb(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::RESTART_RESIDENT_PROCESS_DEPENDED_ON_WEB):
            return HandleRestartResidentProcessDependedOnWeb(data, reply);
    #ifdef SUPPORT_CHILD_PROCESS
        case static_cast<uint32_t>(AppMgrInterfaceCode::GET_ALL_CHILDREN_PROCESSES):
            return HandleGetAllChildrenProcesses(data, reply);
    #endif // SUPPORT_CHILD_PROCESS
        case static_cast<uint32_t>(AppMgrInterfaceCode::GET_SUPPORTED_PROCESS_CACHE_PIDS):
            return HandleGetSupportedProcessCachePids(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::REGISTER_KIA_INTERCEPTOR):
            return HandleRegisterKiaInterceptor(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::CHECK_IS_KIA_PROCESS):
            return HandleCheckIsKiaProcess(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::KILL_APP_SELF_WITH_INSTANCE_KEY):
            return HandleKillAppSelfWithInstanceKey(data, reply);
    }
    return INVALID_FD;
}

int32_t AppMgrStub::OnRemoteRequestInnerEighth(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    switch (static_cast<uint32_t>(code)) {
        case static_cast<uint32_t>(AppMgrInterfaceCode::UPDATE_PROCESS_MEMORY_STATE):
            return HandleUpdateProcessMemoryState(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::LAUNCH_ABILITY):
            return HandleLaunchAbility(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::UPDATE_CONFIGURATION_POLICY):
            return HandleUpdateConfigurationForBackgroundApp(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::PROMOTE_CURRENT_TO_CANDIDATE_MASTER_PROCESS):
            return HandlePromoteCurrentToCandidateMasterProcess(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::DEMOTE_CURRENT_FROM_CANDIDATE_MASTER_PROCESS):
            return HandleDemoteCurrentFromCandidateMasterProcess(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::EXIT_MASTER_PROCESS_ROLE):
            return HandleExitMasterProcessRole(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::QUERY_RUNNING_SHARED_BUNDLES):
            return HandleQueryRunningSharedBundles(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::SET_SPECIFIED_PROCESS_REQUEST_ID):
            return HandleSetSpecifiedProcessRequestId(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::ALLOW_SCB_PROCESS_MOVE_TO_BACKGROUND):
            return HandleAllowScbProcessMoveToBackground(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::KILL_PROCESS_BY_PID_FOR_EXIT):
            return HandleKillProcessByPidForExit(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::KILL_CHILD_PROCESS_BY_PID):
            return HandleKillChildProcessByPid(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::PRELOAD_EXTENSION):
            return HandlePreloadExtension(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::LOCK_PROCESS_CACHE):
            return HandleLockProcessCache(data, reply);
        case static_cast<uint32_t>(AppMgrInterfaceCode::SET_PROCESS_PREPARE_EXIT):
            return HandleSetProcessPrepareExit(data, reply);
    }
    return INVALID_FD;
}

int32_t AppMgrStub::HandleAttachApplication(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> client = data.ReadRemoteObject();
    if (client == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remote object null");
    }
    AttachApplication(client);
    return NO_ERROR;
}

int32_t AppMgrStub::HandlePreloadApplication(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::string bundleName = Str16ToStr8(data.ReadString16());
    int32_t userId = data.ReadInt32();
    int32_t preloadMode = data.ReadInt32();
    int32_t appIndex = data.ReadInt32();
    auto result = PreloadApplication(bundleName, userId, static_cast<AppExecFwk::PreloadMode>(preloadMode), appIndex);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Stub HandlePreloadApplication Write result failed.");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandlePreloadModuleFinished(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "HandlePreloadModuleFinished called");
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t recordId = data.ReadInt32();
    PreloadModuleFinished(recordId);
    return NO_ERROR;
}


int32_t AppMgrStub::HandleApplicationForegrounded(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    ApplicationForegrounded(data.ReadInt32());
    return NO_ERROR;
}

int32_t AppMgrStub::HandleApplicationBackgrounded(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    ApplicationBackgrounded(data.ReadInt32());
    return NO_ERROR;
}

int32_t AppMgrStub::HandleApplicationTerminated(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    ApplicationTerminated(data.ReadInt32());
    return NO_ERROR;
}

int32_t AppMgrStub::HandleAbilityCleaned(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    AbilityCleaned(token);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetAmsMgr(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t result = NO_ERROR;
    sptr<IAmsMgr> amsMgr = GetAmsMgr();
    if (!amsMgr) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilitymgr instance is nullptr");
        result = ERR_NO_INIT;
    } else {
        if (!reply.WriteRemoteObject(amsMgr->AsObject())) {
            TAG_LOGE(AAFwkTag::APPMGR, "failed to reply abilitymgr instance to client, for write parcel error");
            result = ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleClearUpApplicationData(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::string bundleName = data.ReadString();
    int32_t appCloneIndex = data.ReadInt32();
    int32_t userId = data.ReadInt32();
    int32_t result = ClearUpApplicationData(bundleName, appCloneIndex, userId);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleClearUpApplicationDataBySelf(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t userId = data.ReadInt32();
    int32_t result = ClearUpApplicationDataBySelf(userId);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetAllRunningProcesses(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::vector<RunningProcessInfo> info;
    auto result = GetAllRunningProcesses(info);
    reply.WriteInt32(info.size());
    for (auto &it : info) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetRunningMultiAppInfoByBundleName(MessageParcel &data, MessageParcel &reply)
{
    if (AAFwk::AppUtils::GetInstance().IsForbidStart()) {
        TAG_LOGW(AAFwkTag::APPMGR, "forbid start: HandleGetRunningMultiAppInfoByBundleName");
        return AAFwk::INNER_ERR;
    }
    std::string bundleName = data.ReadString();
    RunningMultiAppInfo info;
    int32_t result = GetRunningMultiAppInfoByBundleName(bundleName, info);
    if (!reply.WriteParcelable(&info)) {
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetAllRunningInstanceKeysBySelf(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> instanceKeys;
    int32_t result = GetAllRunningInstanceKeysBySelf(instanceKeys);
    if (!reply.WriteStringVector(instanceKeys)) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to write isntanceKeys");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetAllRunningInstanceKeysByBundleName(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = data.ReadString();
    int32_t userId = data.ReadInt32();
    std::vector<std::string> instanceKeys;
    int32_t result = GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys, userId);
    if (!reply.WriteStringVector(instanceKeys)) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed to write isntanceKeys");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetRunningProcessesByBundleType(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t bundleType = data.ReadInt32();
    std::vector<RunningProcessInfo> info;
    auto result = GetRunningProcessesByBundleType(static_cast<BundleType>(bundleType), info);
    reply.WriteInt32(info.size());
    for (auto &it : info) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetProcessRunningInfosByUserId(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t userId = data.ReadInt32();
    std::vector<RunningProcessInfo> info;
    auto result = GetProcessRunningInfosByUserId(info, userId);
    reply.WriteInt32(info.size());
    for (auto &it : info) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "AppMgrStub::HandleGetAllRunningProcesses end");
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetAllRenderProcesses(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::vector<RenderProcessInfo> info;
    auto result = GetAllRenderProcesses(info);
    reply.WriteInt32(info.size());
    for (auto &it : info) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

#ifdef SUPPORT_CHILD_PROCESS
int32_t AppMgrStub::HandleGetAllChildrenProcesses(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::vector<ChildProcessInfo> info;
    auto result = GetAllChildrenProcesses(info);
    reply.WriteInt32(info.size());
    for (auto &it : info) {
        if (!reply.WriteParcelable(&it)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Write ChildProcessInfo faild, child pid=%{public}d", it.pid);
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write result faild");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}
#endif // SUPPORT_CHILD_PROCESS

int32_t AppMgrStub::HandleIsTerminatingByPid(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t pid = data.ReadInt32();
    bool isTerminating = false;
    auto result = IsTerminatingByPid(pid, isTerminating);
    if (!reply.WriteBool(isTerminating)) {
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetProcessRunningInformation(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    RunningProcessInfo info;
    auto result = GetProcessRunningInformation(info);
    if (!reply.WriteParcelable(&info)) {
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleAddAbilityStageDone(MessageParcel &data, MessageParcel &reply)
{
    int32_t recordId = data.ReadInt32();
    AddAbilityStageDone(recordId);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleNotifyMemoryLevel(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t level = data.ReadInt32();
    auto result = NotifyMemoryLevel(level);
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleNotifyProcMemoryLevel(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::unique_ptr<MemoryLevelInfo> memoryLevelInfo(data.ReadParcelable<MemoryLevelInfo>());
    if (memoryLevelInfo.get() == nullptr) {
        return ERR_INVALID_VALUE;
    }
    auto result = NotifyProcMemoryLevel(memoryLevelInfo->GetProcLevelMap());
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleDumpHeapMemory(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppMgrStub::HandleDumpHeapMemory.");
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t pid = data.ReadInt32();
    struct OHOS::AppExecFwk::MallocInfo mallocInfo;
    auto result = DumpHeapMemory(pid, mallocInfo);
    if (result != NO_ERROR) {
        return result;
    }
    reply.WriteParcelable(&mallocInfo);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleDumpJsHeapMemory(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppMgrStub::HandleDumpJsHeapMemory.");
    HITRACE_METER(HITRACE_TAG_APP);
    std::unique_ptr<JsHeapDumpInfo> info(data.ReadParcelable<JsHeapDumpInfo>());
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppMgrStub read configuration error");
        return ERR_INVALID_VALUE;
    }
    auto result = DumpJsHeapMemory(*info);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write result error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleDumpCjHeapMemory(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppMgrStub::HandleDumpCjHeapMemory.");
    HITRACE_METER(HITRACE_TAG_APP);
    std::unique_ptr<CjHeapDumpInfo> info(data.ReadParcelable<CjHeapDumpInfo>());
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppMgrStub read configuration error");
        return ERR_INVALID_VALUE;
    }
    auto result = DumpCjHeapMemory(*info);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write result error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleStartupResidentProcess(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    int32_t infoSize = data.ReadInt32();
    if (infoSize > CYCLE_LIMIT) {
        TAG_LOGE(AAFwkTag::APPMGR, "infoSize is too large");
        return ERR_INVALID_VALUE;
    }
    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<AppExecFwk::BundleInfo> bundleInfo(data.ReadParcelable<AppExecFwk::BundleInfo>());
        if (!bundleInfo) {
            TAG_LOGE(AAFwkTag::APPMGR, "Read Parcelable infos failed.");
            return ERR_INVALID_VALUE;
        }
        bundleInfos.emplace_back(*bundleInfo);
    }
    StartupResidentProcess(bundleInfos);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleRegisterApplicationStateObserver(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> bundleNameList;
    auto callback = iface_cast<AppExecFwk::IApplicationStateObserver>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Callback is null.");
        return ERR_INVALID_VALUE;
    }
    data.ReadStringVector(&bundleNameList);
    int32_t result = RegisterApplicationStateObserver(callback, bundleNameList);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleUnregisterApplicationStateObserver(MessageParcel &data, MessageParcel &reply)
{
    auto callback = iface_cast<AppExecFwk::IApplicationStateObserver>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Callback is null.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = UnregisterApplicationStateObserver(callback);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleRegisterAbilityForegroundStateObserver(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto callback = iface_cast<AppExecFwk::IAbilityForegroundStateObserver>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Callback is null.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = RegisterAbilityForegroundStateObserver(callback);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleUnregisterAbilityForegroundStateObserver(MessageParcel &data, MessageParcel &reply)
{
    auto callback = iface_cast<AppExecFwk::IAbilityForegroundStateObserver>(data.ReadRemoteObject());
    int32_t result = UnregisterAbilityForegroundStateObserver(callback);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetForegroundApplications(MessageParcel &data, MessageParcel &reply)
{
    std::vector<AppStateData> appStateDatas;
    int32_t result = GetForegroundApplications(appStateDatas);
    reply.WriteInt32(appStateDatas.size());
    for (auto &it : appStateDatas) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return result;
}

int32_t AppMgrStub::HandleStartUserTestProcess(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto observer = data.ReadRemoteObject();
    std::unique_ptr<BundleInfo> bundleInfo(data.ReadParcelable<BundleInfo>());
    if (bundleInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t userId = data.ReadInt32();
    int32_t result = StartUserTestProcess(*want, observer, *bundleInfo, userId);
    reply.WriteInt32(result);
    return result;
}

int32_t AppMgrStub::HandleFinishUserTest(MessageParcel &data, MessageParcel &reply)
{
    std::string msg = data.ReadString();
    int64_t resultCode = data.ReadInt64();
    std::string bundleName = data.ReadString();
    int32_t result = FinishUserTest(msg, resultCode, bundleName);
    reply.WriteInt32(result);
    return result;
}

int32_t AppMgrStub::RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer,
    const std::vector<std::string> &bundleNameList)
{
    return NO_ERROR;
}

int32_t AppMgrStub::UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    return NO_ERROR;
}

int32_t AppMgrStub::GetForegroundApplications(std::vector<AppStateData> &list)
{
    return NO_ERROR;
}

int32_t AppMgrStub::HandleScheduleAcceptWantDone(MessageParcel &data, MessageParcel &reply)
{
    auto recordId = data.ReadInt32();
    AAFwk::Want *want = data.ReadParcelable<AAFwk::Want>();
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto flag = data.ReadString();

    ScheduleAcceptWantDone(recordId, *want, flag);
    delete want;
    return NO_ERROR;
}

int32_t AppMgrStub::HandleScheduleNewProcessRequestDone(MessageParcel &data, MessageParcel &reply)
{
    auto recordId = data.ReadInt32();
    AAFwk::Want *want = data.ReadParcelable<AAFwk::Want>();
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto flag = data.ReadString();

    ScheduleNewProcessRequestDone(recordId, *want, flag);
    delete want;
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetAbilityRecordsByProcessID(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t pid = data.ReadInt32();
    std::vector<sptr<IRemoteObject>> tokens;
    auto result = GetAbilityRecordsByProcessID(pid, tokens);
    reply.WriteInt32(tokens.size());
    for (auto &it : tokens) {
        if (!reply.WriteRemoteObject(it)) {
            TAG_LOGE(AAFwkTag::APPMGR, "failed to write query result.");
            return ERR_FLATTEN_OBJECT;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandlePreStartNWebSpawnProcess(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::APPMGR, "HandlePreNWebSpawnProcess");
    int32_t result = PreStartNWebSpawnProcess();
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write result error.");
        return ERR_INVALID_VALUE;
    }
    return result;
}

int32_t AppMgrStub::HandleStartRenderProcess(MessageParcel &data, MessageParcel &reply)
{
    std::string renderParam = data.ReadString();
    int32_t ipcFd = data.ReadFileDescriptor();
    int32_t sharedFd = data.ReadFileDescriptor();
    int32_t crashFd = data.ReadFileDescriptor();
    int32_t renderPid = 0;
    bool isGPU = data.ReadBool();
    int32_t result =
        StartRenderProcess(renderParam, ipcFd, sharedFd, crashFd, renderPid, isGPU);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write result error.");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(renderPid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write renderPid error.");
        return ERR_INVALID_VALUE;
    }
    return result;
}

int32_t AppMgrStub::HandleAttachRenderProcess(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    sptr<IRemoteObject> scheduler = data.ReadRemoteObject();
    AttachRenderProcess(scheduler);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleSaveBrowserChannel(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> browser = data.ReadRemoteObject();
    SaveBrowserChannel(browser);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetRenderProcessTerminationStatus(MessageParcel &data, MessageParcel &reply)
{
    int32_t renderPid = data.ReadInt32();
    if (renderPid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid renderPid value");
        return ERR_INVALID_VALUE;
    }
    int status = 0;
    int32_t result = GetRenderProcessTerminationStatus(renderPid, status);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write result error.");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(status)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write status error.");
        return ERR_INVALID_VALUE;
    }
    return result;
}

int32_t AppMgrStub::HandleGetConfiguration(MessageParcel &data, MessageParcel &reply)
{
    Configuration config;
    int ret = GetConfiguration(config);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppMgrStub GetConfiguration error");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteParcelable(&config)) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppMgrStub GetConfiguration error");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetConfigurationByUserId(MessageParcel& data, MessageParcel& reply)
{
    int32_t userId = -1;
    if (!data.ReadInt32(userId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to read userId from parcel");
        return ERR_INVALID_DATA;
    }

    Configuration config;
    int32_t ret = GetConfiguration(config, userId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppMgrStub GetConfiguration error");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteParcelable(&config)) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppMgrStub GetConfiguration error");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleUpdateConfiguration(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Configuration> config(data.ReadParcelable<Configuration>());
    if (!config) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppMgrStub read configuration error");
        return ERR_INVALID_VALUE;
    }
    int32_t userId = data.ReadInt32();
    int32_t ret = UpdateConfiguration(*config, userId);
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleUpdateConfigurationMultiUser(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Configuration> config(data.ReadParcelable<Configuration>());
    if (!config) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppMgrStub read configuration error");
        return ERR_INVALID_VALUE;
    }

    std::vector<int32_t> userIds;
    if (!data.ReadInt32Vector(&userIds)) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppMgrStub read userIds error");
        return ERR_INVALID_DATA;
    }
    int32_t ret = UpdateConfigurationByUserIds(*config, userIds);
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleUpdateConfigurationForBackgroundApp(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::vector<BackgroundAppInfo> appInfos;
    uint32_t size = data.ReadUint32();
    if (size <= 0 || size > MAX_BACKGROUND_APP_COUNT) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid size");
        return ERR_INVALID_VALUE;
    }
    appInfos.resize(size);
    if (appInfos.capacity() < size) {
        TAG_LOGE(AAFwkTag::APPMGR, "Resource allocation error");
        return ERR_NO_MEMORY;
    }
    for (auto &item : appInfos) {
        std::unique_ptr<BackgroundAppInfo> tmpInfo(data.ReadParcelable<BackgroundAppInfo>());
        if (tmpInfo == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "tmpInfo null");
            return ERR_INVALID_VALUE;
        }
        item = *tmpInfo;
    }
    std::unique_ptr<AppExecFwk::ConfigurationPolicy> policy(data.ReadParcelable<AppExecFwk::ConfigurationPolicy>());
    if (policy == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "policy null");
        return ERR_INVALID_VALUE;
    }
    int32_t userId = data.ReadInt32();
    int32_t result = UpdateConfigurationForBackgroundApp(appInfos, *policy, userId);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleUpdateConfigurationByBundleName(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Configuration> config(data.ReadParcelable<Configuration>());
    if (!config) {
        TAG_LOGE(AAFwkTag::APPMGR, "AppMgrStub read configuration error");
        return ERR_INVALID_VALUE;
    }
    std::string name = data.ReadString();
    auto appIndex = data.ReadInt32();
    int32_t ret = UpdateConfigurationByBundleName(*config, name, appIndex);
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleRegisterConfigurationObserver(MessageParcel &data, MessageParcel &reply)
{
    auto observer = iface_cast<AppExecFwk::IConfigurationObserver>(data.ReadRemoteObject());
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Observer is null.");
        return ERR_INVALID_VALUE;
    }
    int32_t userId = data.ReadInt32();
    int32_t result = RegisterConfigurationObserver(observer, userId);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleUnregisterConfigurationObserver(MessageParcel &data, MessageParcel &reply)
{
    auto observer = iface_cast<AppExecFwk::IConfigurationObserver>(data.ReadRemoteObject());
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Observer is null.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = UnregisterConfigurationObserver(observer);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetAppRunningStateByBundleName(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::string bundleName = data.ReadString();
    auto ret = GetAppRunningStateByBundleName(bundleName);
    if (!reply.WriteBool(ret)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleNotifyLoadRepairPatch(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::string bundleName = data.ReadString();
    auto callback = iface_cast<IQuickFixCallback>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Callback is null.");
        return ERR_INVALID_VALUE;
    }
    auto ret = NotifyLoadRepairPatch(bundleName, callback);
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleNotifyHotReloadPage(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::string bundleName = data.ReadString();
    auto callback = iface_cast<IQuickFixCallback>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Callback is null.");
        return ERR_INVALID_VALUE;
    }
    auto ret = NotifyHotReloadPage(bundleName, callback);
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
int32_t AppMgrStub::HandleSetContinuousTaskProcess(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    int32_t pid = data.ReadInt32();
    bool isContinuousTask = data.ReadBool();
    auto ret = SetContinuousTaskProcess(pid, isContinuousTask);
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}
#endif

int32_t AppMgrStub::HandleNotifyUnLoadRepairPatch(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::string bundleName = data.ReadString();
    auto callback = iface_cast<IQuickFixCallback>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Callback is null.");
        return ERR_INVALID_VALUE;
    }
    auto ret = NotifyUnLoadRepairPatch(bundleName, callback);
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleIsSharedBundleRunning(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::string bundleName = data.ReadString();
    uint32_t versionCode = data.ReadUint32();
    bool result = IsSharedBundleRunning(bundleName, versionCode);
    if (!reply.WriteBool(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleStartNativeProcessForDebugger(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::unique_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto result = StartNativeProcessForDebugger(*want);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetBundleNameByPid(MessageParcel &data, MessageParcel &reply)
{
    int32_t pid = data.ReadInt32();
    std::string bundleName;
    int32_t uid;
    auto result = GetBundleNameByPid(pid, bundleName, uid);
    if (result != ERR_OK) {
        return result;
    }

    if (!reply.WriteString(bundleName)) {
        return ERR_INVALID_VALUE;
    }

    if (!reply.WriteInt32(uid)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetRunningProcessInfoByPid(MessageParcel &data, MessageParcel &reply)
{
    RunningProcessInfo processInfo;
    auto pid = static_cast<pid_t>(data.ReadInt32());
    auto result = GetRunningProcessInfoByPid(pid, processInfo);
    if (!reply.WriteParcelable(&processInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetRunningProcessInfoByChildProcessPid(MessageParcel &data, MessageParcel &reply)
{
    RunningProcessInfo processInfo;
    auto childPid = static_cast<pid_t>(data.ReadInt32());
    auto result = GetRunningProcessInfoByChildProcessPid(childPid, processInfo);
    if (!reply.WriteParcelable(&processInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleNotifyFault(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<FaultData> faultData(data.ReadParcelable<FaultData>());
    if (faultData == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<FaultData> failed");
        return ERR_INVALID_VALUE;
    }

    int32_t result = NotifyAppFault(*faultData);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleNotifyFaultBySA(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AppFaultDataBySA> faultData(data.ReadParcelable<AppFaultDataBySA>());
    if (faultData == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<AppFaultDataBySA> failed");
        return ERR_INVALID_VALUE;
    }

    int32_t result = NotifyAppFaultBySA(*faultData);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleSetAppFreezeFilter(MessageParcel &data, MessageParcel &reply)
{
    int32_t pid = data.ReadInt32();
    bool result = SetAppFreezeFilter(pid);
    if (!reply.WriteBool(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetProcessMemoryByPid(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int32_t pid = data.ReadInt32();
    int32_t memorySize = 0;
    auto result = GetProcessMemoryByPid(pid, memorySize);
    if (!reply.WriteInt32(memorySize)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Memory size write failed.");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetRunningProcessInformation(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::string bundleName = data.ReadString();
    int32_t userId = data.ReadInt32();
    std::vector<RunningProcessInfo> info;
    auto result = GetRunningProcessInformation(bundleName, userId, info);
    reply.WriteInt32(info.size());
    for (auto &it : info) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleChangeAppGcState(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t pid = data.ReadInt32();
    int32_t state = data.ReadInt32();
    uint64_t tid = data.ReadUint64();
    int32_t ret = ChangeAppGcState(pid, state, tid);
    reply.WriteInt32(ret);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleNotifyPageShow(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    std::unique_ptr<PageStateData> pageStateData(data.ReadParcelable<PageStateData>());
    if (token == nullptr || pageStateData == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "read data failed");
        return ERR_INVALID_VALUE;
    }

    auto result = NotifyPageShow(token, *pageStateData);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleNotifyPageHide(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    std::unique_ptr<PageStateData> pageStateData(data.ReadParcelable<PageStateData>());
    if (token == nullptr || pageStateData == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "read data failed");
        return ERR_INVALID_VALUE;
    }

    auto result = NotifyPageHide(token, *pageStateData);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleRegisterAppRunningStatusListener(MessageParcel &data, MessageParcel &reply)
{
    auto listener = data.ReadRemoteObject();
    if (listener == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Read data failed.");
        return ERR_INVALID_VALUE;
    }

    auto result = RegisterAppRunningStatusListener(listener);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleUnregisterAppRunningStatusListener(MessageParcel &data, MessageParcel &reply)
{
    auto listener = data.ReadRemoteObject();
    if (listener == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Read data failed.");
        return ERR_INVALID_VALUE;
    }

    auto result = UnregisterAppRunningStatusListener(listener);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleRegisterAppForegroundStateObserver(MessageParcel &data, MessageParcel &reply)
{
    auto callback = iface_cast<AppExecFwk::IAppForegroundStateObserver>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Callback is null.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = RegisterAppForegroundStateObserver(callback);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleUnregisterAppForegroundStateObserver(MessageParcel &data, MessageParcel &reply)
{
    auto callback = iface_cast<AppExecFwk::IAppForegroundStateObserver>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Callback is null.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = UnregisterAppForegroundStateObserver(callback);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleIsApplicationRunning(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::string bundleName = data.ReadString();
    bool isRunning = false;
    int32_t result = IsApplicationRunning(bundleName, isRunning);
    if (!reply.WriteBool(isRunning)) {
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleIsAppRunning(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::string bundleName = data.ReadString();
    bool isRunning = false;
    int32_t appCloneIndex = data.ReadInt32();
    int32_t result = IsAppRunning(bundleName, appCloneIndex, isRunning);
    if (!reply.WriteBool(isRunning)) {
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleIsAppRunningByBundleNameAndUserId(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::string bundleName = data.ReadString();
    bool isRunning = false;
    int32_t userId = data.ReadInt32();
    int32_t result = IsAppRunningByBundleNameAndUserId(bundleName, userId, isRunning);
    if (!reply.WriteBool(isRunning)) {
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleIsAppRunningByUserId(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::string bundleName = data.ReadString();
    bool isRunning = false;
    int32_t appCloneIndex = data.ReadInt32();
    int32_t userId = data.ReadInt32();
    int32_t result = IsAppRunning(bundleName, appCloneIndex, userId, isRunning);
    if (!reply.WriteBool(isRunning)) {
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

#ifdef SUPPORT_CHILD_PROCESS
int32_t AppMgrStub::HandleStartChildProcess(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::unique_ptr<ChildProcessRequest> request(data.ReadParcelable<ChildProcessRequest>());
    if (!request) {
        return IPC_STUB_ERR;
    }
    int32_t childPid = 0;
    int32_t result = StartChildProcess(childPid, *request);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write result error.");
        return IPC_STUB_ERR;
    }
    if (result == ERR_OK && !reply.WriteInt32(childPid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write childPid error.");
        return IPC_STUB_ERR;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetChildProcessInfoForSelf(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    ChildProcessInfo info;
    auto result = GetChildProcessInfoForSelf(info);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write result error.");
        return ERR_INVALID_VALUE;
    }
    if (result == ERR_OK && !reply.WriteParcelable(&info)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleAttachChildProcess(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    sptr<IRemoteObject> scheduler = data.ReadRemoteObject();
    AttachChildProcess(scheduler);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleExitChildProcessSafely(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    ExitChildProcessSafely();
    return NO_ERROR;
}
#endif // SUPPORT_CHILD_PROCESS

int32_t AppMgrStub::HandleRegisterNativeChildExitNotify(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto callback = iface_cast<AppExecFwk::INativeChildNotify>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Callback is null.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = RegisterNativeChildExitNotify(callback);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleUnregisterNativeChildExitNotify(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto callback = iface_cast<AppExecFwk::INativeChildNotify>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Callback is null.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = UnregisterNativeChildExitNotify(callback);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleIsFinalAppProcess(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (!reply.WriteBool(IsFinalAppProcess())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write bool result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleRegisterRenderStateObserver(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto callback = iface_cast<AppExecFwk::IRenderStateObserver>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Callback is null.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = RegisterRenderStateObserver(callback);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleUnregisterRenderStateObserver(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto callback = iface_cast<AppExecFwk::IRenderStateObserver>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Callback is null.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = UnregisterRenderStateObserver(callback);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleUpdateRenderState(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    int32_t pid = data.ReadInt32();
    int32_t state = data.ReadInt32();
    int32_t result = UpdateRenderState(pid, state);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleSignRestartAppFlag(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto uid = data.ReadInt32();
    auto instanceKey = data.ReadString();
    auto ret = SignRestartAppFlag(uid, instanceKey);
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write ret error.");
        return IPC_STUB_ERR;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleSignRestartProcess(MessageParcel &data, MessageParcel &reply)
{
    auto uid = data.ReadInt32();
    auto ret = SignRestartProcess(uid);
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write ret error.");
        return IPC_STUB_ERR;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetAppRunningUniqueIdByPid(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    int32_t pid = data.ReadInt32();
    std::string appRunningUniqueId;
    int32_t result = GetAppRunningUniqueIdByPid(pid, appRunningUniqueId);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write result error.");
        return IPC_STUB_ERR;
    }
    if (result == ERR_OK && !reply.WriteString(appRunningUniqueId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetAppRunningUniqueIdByPid err or Write appRunningUniqueId error.");
        return IPC_STUB_ERR;
    }

    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetAllUIExtensionRootHostPid(MessageParcel &data, MessageParcel &reply)
{
    pid_t pid = data.ReadInt32();
    std::vector<pid_t> hostPids;
    auto result = GetAllUIExtensionRootHostPid(pid, hostPids);
    reply.WriteInt32(hostPids.size());
    for (auto &it : hostPids) {
        if (!reply.WriteInt32(it)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Write host pid failed.");
            return ERR_INVALID_VALUE;
        }
    }

    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write result failed.");
        return ERR_INVALID_VALUE;
    }

    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetAllUIExtensionProviderPid(MessageParcel &data, MessageParcel &reply)
{
    pid_t hostPid = data.ReadInt32();
    std::vector<pid_t> providerPids;
    auto result = GetAllUIExtensionProviderPid(hostPid, providerPids);
    reply.WriteInt32(providerPids.size());
    for (auto &it : providerPids) {
        if (!reply.WriteInt32(it)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Write provider pid failed.");
            return ERR_INVALID_VALUE;
        }
    }

    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write result failed.");
        return ERR_INVALID_VALUE;
    }

    return NO_ERROR;
}

int32_t AppMgrStub::HandleNotifyMemorySizeStateChanged(MessageParcel &data, MessageParcel &reply)
{
    int32_t memorySizeState = data.ReadInt32();
    int result = NotifyMemorySizeStateChanged(memorySizeState);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write result error.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleSetSupportedProcessCache(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    bool isSupport = data.ReadBool();
    int32_t pid = data.ReadInt32();
    auto ret = SetSupportedProcessCache(pid, isSupport);
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write ret error.");
        return IPC_STUB_ERR;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleIsProcessCacheSupported(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "HandleIsProcessCacheSupported called");
    int32_t pid = data.ReadInt32();
    bool isSupported = false;
    auto ret = IsProcessCacheSupported(pid, isSupported);
    if (!reply.WriteBool(isSupported)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write isSupported error.");
        return AAFwk::ERR_WRITE_BOOL_FAILED;
    }
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write ret error.");
        return AAFwk::ERR_WRITE_RESULT_CODE_FAILED;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleSetProcessCacheEnable(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "HandleSetProcessCacheEnable called");
    int32_t pid = data.ReadInt32();
    bool enable = data.ReadBool();
    auto ret = SetProcessCacheEnable(pid, enable);
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write ret error.");
        return AAFwk::ERR_WRITE_RESULT_CODE_FAILED;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleLockProcessCache(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "HandleLockProcessCache called");
    int32_t pid = data.ReadInt32();
    bool isLock = data.ReadBool();
    auto ret = LockProcessCache(pid, isLock);
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write ret error.");
        return AAFwk::ERR_WRITE_RESULT_CODE_FAILED;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleSetAppAssertionPauseState(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    bool flag = data.ReadBool();
    SetAppAssertionPauseState(flag);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleSetSupportedProcessCacheSelf(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    bool isSupport = data.ReadBool();
    auto ret = SetSupportedProcessCacheSelf(isSupport);
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write ret error.");
        return IPC_STUB_ERR;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleCheckCallingIsUserTestMode(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    pid_t pid = data.ReadInt32();
    bool isUserTest = false;
    int32_t ret = CheckCallingIsUserTestMode(pid, isUserTest);
    if (!reply.WriteBool(isUserTest)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write isUserTest error.");
        return IPC_STUB_ERR;
    }
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write ret error.");
        return IPC_STUB_ERR;
    }
    return NO_ERROR;
}

#ifdef SUPPORT_CHILD_PROCESS
int32_t AppMgrStub::HandleCreateNativeChildProcess(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called");
    std::string libName = data.ReadString();
    sptr<IRemoteObject> callback = data.ReadRemoteObject();
    std::unique_ptr<ChildProcessRequest> request(data.ReadParcelable<ChildProcessRequest>());
    if (!request) {
        return IPC_STUB_ERR;
    }

    auto result = CreateNativeChildProcess(libName, callback, *request);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write result failed.");
        return IPC_STUB_ERR;
    }
    return NO_ERROR;
}
#endif // SUPPORT_CHILD_PROCESS

int32_t AppMgrStub::HandleNotifyProcessDependedOnWeb(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    int32_t ret = NotifyProcessDependedOnWeb();
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write ret error.");
        return IPC_STUB_ERR;
    }

    return NO_ERROR;
}

int32_t AppMgrStub::HandleKillProcessDependedOnWeb(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    KillProcessDependedOnWeb();
    return NO_ERROR;
}

int32_t AppMgrStub::HandleRestartResidentProcessDependedOnWeb(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    RestartResidentProcessDependedOnWeb();
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetSupportedProcessCachePids(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::string bundleName = data.ReadString();
    std::vector<int32_t> pidList;
    auto result = GetSupportedProcessCachePids(bundleName, pidList);
    reply.WriteInt32(pidList.size());
    for (auto it : pidList) {
        if (!reply.WriteInt32(it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::GetSupportedProcessCachePids(const std::string &bundleName,
    std::vector<int32_t> &pidList)
{
    return NO_ERROR;
}

int32_t AppMgrStub::HandleRegisterKiaInterceptor(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    sptr<IKiaInterceptor> interceptor = iface_cast<IKiaInterceptor>(data.ReadRemoteObject());
    if (interceptor == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "interceptor is nullptr.");
        return ERR_INVALID_VALUE;
    }

    reply.WriteInt32(RegisterKiaInterceptor(interceptor));
    return NO_ERROR;
}

int32_t AppMgrStub::HandleCheckIsKiaProcess(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    int pid = data.ReadInt32();
    bool isKia = false;
    int result = CheckIsKiaProcess(pid, isKia);
    reply.WriteInt32(result);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "failed,result=%{public}d.", result);
        return result;
    }
    reply.WriteBool(isKia);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleKillAppSelfWithInstanceKey(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    auto instanceKey = data.ReadString();
    auto clearPageStack = data.ReadBool();
    auto reason = data.ReadString();
    auto result = KillAppSelfWithInstanceKey(instanceKey, clearPageStack, reason);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleKillProcessByPidForExit(MessageParcel &data, MessageParcel &reply)
{
    auto pid = data.ReadInt32();
    auto reason = data.ReadString();
    auto result = KillProcessByPidForExit(pid, reason);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleIsSpecifiedModuleLoaded(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::unique_ptr<AbilityInfo> abilityInfo(data.ReadParcelable<AbilityInfo>());
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityInfo is nullptr");
        return ERR_INVALID_VALUE;
    }

    bool exist = false;
    bool isDebug = false;
    auto ret = IsSpecifiedModuleLoaded(*want, *abilityInfo, exist, isDebug);
    if (ret == ERR_OK) {
        reply.WriteBool(exist);
        reply.WriteBool(isDebug);
    }
    return ret;
}

ErrCode AppMgrStub::HandleUpdateProcessMemoryState(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    TAG_LOGD(AAFwkTag::APPMGR, "HandleUpdateProcessMemoryState,callingPid=%{public}d", IPCSkeleton::GetCallingPid());
    std::vector<ProcessMemoryState> states;
    auto size = data.ReadUint32();
    if (size <= 0 || size > MAX_PROCESS_STATE_COUNT) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid size");
        return ERR_INVALID_VALUE;
    }
    for (uint32_t i = 0; i < size; i++) {
        std::unique_ptr<ProcessMemoryState> tmpState(data.ReadParcelable<ProcessMemoryState>());
        if (tmpState == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "tmpState is nullptr.");
            return ERR_INVALID_VALUE;
        }
        states.emplace_back(*tmpState);
    }
    auto ret = UpdateProcessMemoryState(states);
    reply.WriteInt32(ret);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleLaunchAbility(MessageParcel &data, MessageParcel &reply)
{
    sptr token = data.ReadRemoteObject();
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "read data failed.");
        return ERR_INVALID_VALUE;
    }
    auto result = LaunchAbility(token);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandlePromoteCurrentToCandidateMasterProcess(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    bool isInsertToHead = data.ReadBool();
    auto ret = PromoteCurrentToCandidateMasterProcess(isInsertToHead);
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write ret error.");
        return IPC_STUB_ERR;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleDemoteCurrentFromCandidateMasterProcess(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    int32_t ret = DemoteCurrentFromCandidateMasterProcess();
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write ret error.");
        return IPC_STUB_ERR;
    }

    return NO_ERROR;
}

int32_t AppMgrStub::HandleExitMasterProcessRole(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    int32_t ret = ExitMasterProcessRole();
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write ret error.");
        return IPC_STUB_ERR;
    }

    return NO_ERROR;
}

int32_t AppMgrStub::HandleSetSpecifiedProcessRequestId(MessageParcel &data, MessageParcel &reply)
{
    auto recordId = data.ReadInt32();
    auto requestId = data.ReadInt32();
    SetSpecifiedProcessRequestId(recordId, requestId);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleQueryRunningSharedBundles(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "HandleQueryRunningSharedBundles call");
    pid_t pid = data.ReadInt32();
    std::map<std::string, uint32_t> sharedBundles;

    int32_t result = QueryRunningSharedBundles(pid, sharedBundles);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write result fail");
        return AAFwk::ERR_WRITE_RESULT_CODE_FAILED;
    }
    if (result != ERR_OK) {
        return NO_ERROR;
    }

    if (!reply.WriteInt32(static_cast<int32_t>(sharedBundles.size()))) {
        TAG_LOGE(AAFwkTag::APPMGR, "write size fail");
        return IPC_STUB_ERR;
    }
    for (const auto &item : sharedBundles) {
        if (!reply.WriteString(item.first) || !reply.WriteUint32(item.second)) {
            TAG_LOGE(AAFwkTag::APPMGR, "write item fail");
            return IPC_STUB_ERR;
        }
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleRegisterApplicationStateObserverWithFilter(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> bundleNameList;
    auto callback = iface_cast<IApplicationStateObserver>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Callback is null.");
        return ERR_INVALID_VALUE;
    }
    data.ReadStringVector(&bundleNameList);
    std::unique_ptr<AppStateFilter> appStateFilter(data.ReadParcelable<AppStateFilter>());
    if (appStateFilter == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appStateFilter is nullptr");
        return ERR_INVALID_VALUE;
    }
    bool isUsingFilter = data.ReadBool();
    int32_t result = RegisterApplicationStateObserverWithFilter(callback,
        bundleNameList, *appStateFilter, isUsingFilter);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleAllowScbProcessMoveToBackground(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call");
    AllowScbProcessMoveToBackground();
    return NO_ERROR;
}

int32_t AppMgrStub::HandleKillChildProcessByPid(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "HandleKillChildProcessByPid call");
    pid_t pid = data.ReadInt32();

    int32_t result = KillChildProcessByPid(pid);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write result fail");
        return AAFwk::ERR_WRITE_RESULT_CODE_FAILED;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandlePreloadExtension(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    TAG_LOGD(AAFwkTag::APPMGR, "Stub HandlePreloadExtension called.");
    std::unique_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t appIndex = data.ReadInt32();
    int32_t userId = data.ReadInt32();
    auto result = PreloadExtension(*want, appIndex, userId);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Stub HandlePreloadExtension Write result failed.");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    return NO_ERROR;
}


int32_t AppMgrStub::HandleSetProcessPrepareExit(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "HandleSetProcessPrepareExit call");
    pid_t pid = data.ReadInt32();
    SetProcessPrepareExit(pid);
    return NO_ERROR;
}
}  // namespace AppExecFwk
}  // namespace OHOS
