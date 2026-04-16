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

#include "app_scheduler_host.h"
#include "ability_info.h"
#include "ability_manager_errors.h"
#include "appexecfwk_errors.h"
#include "hitrace_meter.h"
#include "hilog_tag_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AppExecFwk {
AppSchedulerHost::AppSchedulerHost()
{
    InitMemberFuncMap();
}

void AppSchedulerHost::InitMemberFuncMap() {}

AppSchedulerHost::~AppSchedulerHost()
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerHost destruction");
}

int AppSchedulerHost::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerHost::OnReceived, code = %{public}u, flags= %{public}d.", code,
        option.GetFlags());
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::u16string descriptor = AppSchedulerHost::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::APPMGR, "local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }
    return OnRemoteRequestInner(code, data, reply, option);
}

int32_t AppSchedulerHost::OnRemoteRequestInner(uint32_t code, MessageParcel &data,
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
    TAG_LOGD(AAFwkTag::APPMGR, "AppSchedulerHost::OnRemoteRequest end");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t AppSchedulerHost::OnRemoteRequestInnerFirst(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    switch (static_cast<uint32_t>(code)) {
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_FOREGROUND_APPLICATION_TRANSACTION):
            return HandleScheduleForegroundApplication(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_BACKGROUND_APPLICATION_TRANSACTION):
            return HandleScheduleBackgroundApplication(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_TERMINATE_APPLICATION_TRANSACTION):
            return HandleScheduleTerminateApplication(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_LOWMEMORY_APPLICATION_TRANSACTION):
            return HandleScheduleLowMemory(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_SHRINK_MEMORY_APPLICATION_TRANSACTION):
            return HandleScheduleShrinkMemory(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_MEMORYLEVEL_APPLICATION_TRANSACTION):
            return HandleScheduleMemoryLevel(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_LAUNCH_ABILITY_TRANSACTION):
            return HandleScheduleLaunchAbility(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_CLEAN_ABILITY_TRANSACTION):
            return HandleScheduleCleanAbility(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_LAUNCH_APPLICATION_TRANSACTION):
            return HandleScheduleLaunchApplication(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_PROFILE_CHANGED_TRANSACTION):
            return HandleScheduleProfileChanged(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_CONFIGURATION_UPDATED):
            return HandleScheduleConfigurationUpdated(data, reply);
    }
    return INVALID_FD;
}

int32_t AppSchedulerHost::OnRemoteRequestInnerSecond(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    switch (static_cast<uint32_t>(code)) {
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_PROCESS_SECURITY_EXIT_TRANSACTION):
            return HandleScheduleProcessSecurityExit(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_ABILITY_STAGE_INFO):
            return HandleScheduleAbilityStage(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_CLEAR_PAGE_STACK):
            return HandleScheduleClearPageStack(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_ACCEPT_WANT):
            return HandleScheduleAcceptWant(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_PREPARE_TERMINATE):
            return HandleSchedulePrepareTerminate(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NEW_PROCESS_REQUEST):
            return HandleScheduleNewProcessRequest(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NOTIFY_LOAD_REPAIR_PATCH):
            return HandleNotifyLoadRepairPatch(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NOTIFY_HOT_RELOAD_PAGE):
            return HandleNotifyHotReloadPage(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NOTIFY_UNLOAD_REPAIR_PATCH):
            return HandleNotifyUnLoadRepairPatch(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_UPDATE_APPLICATION_INFO_INSTALLED):
            return HandleScheduleUpdateApplicationInfoInstalled(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_HEAPMEMORY_APPLICATION_TRANSACTION):
            return HandleScheduleHeapMemory(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NOTIFY_FAULT):
            return HandleNotifyAppFault(data, reply);
    }
    return INVALID_FD;
}

int32_t AppSchedulerHost::OnRemoteRequestInnerThird(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    switch (static_cast<uint32_t>(code)) {
        case static_cast<uint32_t>(IAppScheduler::Message::APP_GC_STATE_CHANGE):
            return HandleScheduleChangeAppGcState(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_ATTACH_APP_DEBUG):
            return HandleAttachAppDebug(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_DETACH_APP_DEBUG):
            return HandleDetachAppDebug(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_JSHEAP_MEMORY_APPLICATION_TRANSACTION):
            return HandleScheduleJsHeapMemory(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_CJHEAP_MEMORY_APPLICATION_TRANSACTION):
            return HandleScheduleCjHeapMemory(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_MEM_APPLICATION_TRANSACTION):
            return HandleScheduleMem(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_DUMP_IPC_START):
            return HandleScheduleDumpIpcStart(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_DUMP_IPC_STOP):
            return HandleScheduleDumpIpcStop(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_DUMP_IPC_STAT):
            return HandleScheduleDumpIpcStat(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_DUMP_FFRT):
            return HandleScheduleDumpFfrt(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_DUMP_ARKWEB):
            return HandleScheduleDumpArkWeb(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_CACHE_PROCESS):
            return HandleScheduleCacheProcess(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::WATCHDOG_BACKGROUND_STATUS_TRANSACTION):
            return HandleSetWatchdogBackgroundStatus(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::ON_LOAD_ABILITY_FINISHED):
            return HandleOnLoadAbilityFinished(data, reply);
        case static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_UPDATE_WORK_PROCESS_INFO):
            return HandleScheduleUpdateWorkProcessInfo(data, reply);
    }
    return INVALID_FD;
}

int32_t AppSchedulerHost::HandleScheduleForegroundApplication(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    ScheduleForegroundApplication();
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleBackgroundApplication(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    ScheduleBackgroundApplication();
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleTerminateApplication(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    auto isLastProcess = data.ReadBool();
    ScheduleTerminateApplication(isLastProcess);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleLowMemory(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    ScheduleLowMemory();
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleShrinkMemory(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    ScheduleShrinkMemory(data.ReadInt32());
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleMemoryLevel(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    ScheduleMemoryLevel(data.ReadInt32());
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleHeapMemory(MessageParcel &data, MessageParcel &reply)
{
    int32_t pid = data.ReadInt32();
    struct OHOS::AppExecFwk::MallocInfo mallocInfo;
    ScheduleHeapMemory(pid, mallocInfo);
    reply.WriteParcelable(&mallocInfo);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleJsHeapMemory(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::unique_ptr<JsHeapDumpInfo> info(data.ReadParcelable<JsHeapDumpInfo>());
    if (!info) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<JsHeapDumpInfo> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    ScheduleJsHeapMemory(*info);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleCjHeapMemory(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::unique_ptr<CjHeapDumpInfo> info(data.ReadParcelable<CjHeapDumpInfo>());
    if (!info) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<CjHeapDumpInfo> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    ScheduleCjHeapMemory(*info);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleMem(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::unique_ptr<MemDumpInfo> info(data.ReadParcelable<MemDumpInfo>());
    if (!info) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<MemDumpInfo> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    std::string dumpResult;
    ScheduleMem(*info, dumpResult);
    if (info->isSync) {
        if (!reply.WriteString(dumpResult)) {
            TAG_LOGE(AAFwkTag::APPMGR, "write dumpResult failed");
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleLaunchAbility(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::unique_ptr<AbilityInfo> abilityInfo(data.ReadParcelable<AbilityInfo>());
    if (!abilityInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<AbilityInfo> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    sptr<IRemoteObject> token = nullptr;
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }

    std::shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    auto abilityRecordId = data.ReadInt32();
    std::shared_ptr<AppUpdateInfo> updateInfo;
    if (data.ReadBool()) {
        updateInfo.reset(data.ReadParcelable<AppUpdateInfo>());
    }
    ScheduleLaunchAbility(*abilityInfo, token, want, abilityRecordId, updateInfo);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleCleanAbility(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    bool isCacheProcess = data.ReadBool();
    ScheduleCleanAbility(token, isCacheProcess);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleLaunchApplication(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::unique_ptr<AppLaunchData> launchData(data.ReadParcelable<AppLaunchData>());
    std::unique_ptr<Configuration> config(data.ReadParcelable<Configuration>());
    if (!launchData) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<launchData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!config) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<Configuration> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    ScheduleLaunchApplication(*launchData, *config);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleUpdateApplicationInfoInstalled(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::unique_ptr<ApplicationInfo> appInfo(data.ReadParcelable<ApplicationInfo>());
    auto moduleName = data.ReadString();
    if (!appInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<ApplicationInfo> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    ScheduleUpdateApplicationInfoInstalled(*appInfo, moduleName);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleAbilityStage(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::unique_ptr<HapModuleInfo> abilityStage(data.ReadParcelable<HapModuleInfo>());
    if (!abilityStage) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<launchData> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    ScheduleAbilityStage(*abilityStage);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleProfileChanged(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::unique_ptr<Profile> profile(data.ReadParcelable<Profile>());
    if (!profile) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<Profile> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    ScheduleProfileChanged(*profile);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleConfigurationUpdated(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HITRACE_METER(HITRACE_TAG_APP);
    std::unique_ptr<Configuration> configuration(data.ReadParcelable<Configuration>());
    if (!configuration) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<Configuration> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    uint8_t reason = data.ReadUint8();
    ScheduleConfigurationUpdated(*configuration, static_cast<ConfigUpdateReason>(reason));
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleProcessSecurityExit(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    ScheduleProcessSecurityExit();
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleClearPageStack(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    ScheduleClearPageStack();
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleAcceptWant(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    auto want = std::shared_ptr<AAFwk::Want>(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto moduleName = data.ReadString();
    ScheduleAcceptWant(*want, moduleName);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleSchedulePrepareTerminate(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    auto moduleName = data.ReadString();
    SchedulePrepareTerminate(moduleName);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleNewProcessRequest(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call.");
    HITRACE_METER(HITRACE_TAG_APP);
    auto want = std::shared_ptr<AAFwk::Want>(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto moduleName = data.ReadString();
    ScheduleNewProcessRequest(*want, moduleName);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleNotifyLoadRepairPatch(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::string bundleName = data.ReadString();
    auto callback = iface_cast<IQuickFixCallback>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Callback is null.");
        return ERR_INVALID_VALUE;
    }
    auto recordId = data.ReadInt32();
    ScheduleNotifyLoadRepairPatch(bundleName, callback, recordId);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleNotifyHotReloadPage(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    auto callback = iface_cast<IQuickFixCallback>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Callback is null.");
        return ERR_INVALID_VALUE;
    }
    auto recordId = data.ReadInt32();
    ScheduleNotifyHotReloadPage(callback, recordId);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleNotifyUnLoadRepairPatch(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::string bundleName = data.ReadString();
    auto callback = iface_cast<IQuickFixCallback>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Callback is null.");
        return ERR_INVALID_VALUE;
    }
    auto recordId = data.ReadInt32();
    ScheduleNotifyUnLoadRepairPatch(bundleName, callback, recordId);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleNotifyAppFault(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<FaultData> faultData(data.ReadParcelable<FaultData>());
    if (faultData == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<FaultData> failed");
        return ERR_INVALID_VALUE;
    }

    int32_t result = ScheduleNotifyAppFault(*faultData);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleChangeAppGcState(MessageParcel &data, MessageParcel &reply)
{
    int32_t state = data.ReadInt32();
    uint64_t tid = data.ReadUint64();
    int32_t result = ScheduleChangeAppGcState(state, tid);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleAttachAppDebug(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    auto isDebugFromLocal = data.ReadBool();
    AttachAppDebug(isDebugFromLocal);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleDetachAppDebug(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    DetachAppDebug();
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleDumpIpcStart(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::string result;
    ScheduleDumpIpcStart(result);
    if (!reply.WriteString(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write string of ScheduleDumpIpcStart result");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleDumpIpcStop(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::string result;
    ScheduleDumpIpcStop(result);
    if (!reply.WriteString(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write string of ScheduleDumpIpcStop result");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleDumpIpcStat(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::string result;
    ScheduleDumpIpcStat(result);
    if (!reply.WriteString(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write string of ScheduleDumpIpcStat result");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleCacheProcess(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    ScheduleCacheProcess();
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleDumpFfrt(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::string result;
    ScheduleDumpFfrt(result);
    if (!reply.WriteString(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write string of ScheduleDumpFfrt result");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleDumpArkWeb(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    auto customArgs = data.ReadString();
    std::string result;
    ScheduleDumpArkWeb(customArgs, result);
    if (!reply.WriteString(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write ScheduleDumpArkWeb result fail");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleSetWatchdogBackgroundStatus(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    bool status = data.ReadBool();
    SetWatchdogBackgroundStatus(status);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleOnLoadAbilityFinished(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    uint64_t callbackId = data.ReadUint64();
    int32_t pid = data.ReadInt32();
    OnLoadAbilityFinished(callbackId, pid);
    return NO_ERROR;
}

int32_t AppSchedulerHost::HandleScheduleUpdateWorkProcessInfo(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    auto updateInfo = std::shared_ptr<AppUpdateInfo>(data.ReadParcelable<AppUpdateInfo>());
    ScheduleUpdateWorkProcessInfo(updateInfo);
    return NO_ERROR;
}
}  // namespace AppExecFwk
}  // namespace OHOS
