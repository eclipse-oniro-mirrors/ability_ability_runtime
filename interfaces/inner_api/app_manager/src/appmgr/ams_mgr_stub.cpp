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

#include "ams_mgr_stub.h"
#include "ability_manager_errors.h"
#include "ability_info.h"
#include "app_debug_listener_interface.h"
#include "app_mgr_proxy.h"
#include "app_scheduler_interface.h"
#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "iapp_state_callback.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iremote_object.h"
#include "param.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t MAX_APP_DEBUG_COUNT = 100;
constexpr int32_t MAX_KILL_PROCESS_PID_COUNT = 100;
}

AmsMgrStub::AmsMgrStub()
{
    CreateMemberFuncMap();
}

AmsMgrStub::~AmsMgrStub() {}

void AmsMgrStub::CreateMemberFuncMap() {}

int AmsMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (code != static_cast<uint32_t>(IAmsMgr::Message::Get_BUNDLE_NAME_BY_PID)) {
        TAG_LOGI(AAFwkTag::APPMGR, "OnReceived, code: %{public}u, flags: %{public}d", code,
            option.GetFlags());
    }
    std::u16string descriptor = AmsMgrStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid descriptor");
        return ERR_INVALID_STATE;
    }
    return OnRemoteRequestInner(code, data, reply, option);
}

int32_t AmsMgrStub::OnRemoteRequestInner(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    int retCode = ERR_OK;
    retCode = OnRemoteRequestInnerFirst(code, data, reply, option);
    if (retCode != AAFwk::ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerSecond(code, data, reply, option);
    if (retCode != AAFwk::ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerThird(code, data, reply, option);
    if (retCode != AAFwk::ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerFourth(code, data, reply, option);
    if (retCode != AAFwk::ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t AmsMgrStub::OnRemoteRequestInnerFirst(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    switch (static_cast<uint32_t>(code)) {
        case static_cast<uint32_t>(IAmsMgr::Message::LOAD_ABILITY):
            return HandleLoadAbility(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::TERMINATE_ABILITY):
            return HandleTerminateAbility(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::UPDATE_ABILITY_STATE):
            return HandleUpdateAbilityState(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::UPDATE_EXTENSION_STATE):
            return HandleUpdateExtensionState(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::REGISTER_APP_STATE_CALLBACK):
            return HandleRegisterAppStateCallback(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::KILL_PEOCESS_BY_ABILITY_TOKEN):
            return HandleKillProcessByAbilityToken(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESSES_BY_USERID):
            return HandleKillProcessesByUserId(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESS_WITH_ACCOUNT):
            return HandleKillProcessWithAccount(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION):
            return HandleKillApplication(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::ABILITY_ATTACH_TIMEOUT):
            return HandleAbilityAttachTimeOut(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::PREPARE_TERMINATE_ABILITY):
            return HandlePrepareTerminate(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION_BYUID):
            return HandleKillApplicationByUid(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION_SELF):
            return HandleKillApplicationSelf(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::GET_RUNNING_PROCESS_INFO_BY_TOKEN):
            return HandleGetRunningProcessInfoByToken(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_UNINSTALL_OR_UPGRADE_APP):
            return HandleNotifyUninstallOrUpgradeApp(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_UNINSTALL_OR_UPGRADE_APP_END):
            return HandleNotifyUninstallOrUpgradeAppEnd(data, reply);
    }
    return AAFwk::ERR_CODE_NOT_EXIST;
}

int32_t AmsMgrStub::OnRemoteRequestInnerSecond(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    switch (static_cast<uint32_t>(code)) {
        case static_cast<uint32_t>(IAmsMgr::Message::SET_ABILITY_FOREGROUNDING_FLAG):
            return HandleSetAbilityForegroundingFlagToAppRecord(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::START_SPECIFIED_ABILITY):
            return HandleStartSpecifiedAbility(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::REGISTER_START_SPECIFIED_ABILITY_RESPONSE):
            return HandleRegisterStartSpecifiedAbilityResponse(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::GET_APPLICATION_INFO_BY_PROCESS_ID):
            return HandleGetApplicationInfoByProcessID(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_APP_MGR_RECORD_EXIT_REASON):
            return HandleNotifyAppMgrRecordExitReason(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_APP_MGR_RECORD_EXIT_REASON_COMPABILITY):
            return HandleNotifyAppMgrRecordExitReasonCompability(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::UPDATE_APPLICATION_INFO_INSTALLED):
            return HandleUpdateApplicationInfoInstalled(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::ENABLE_START_PROCESS_FLAG_BY_USER_ID):
            return HandleSetEnableStartProcessFlagByUserId(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::Get_BUNDLE_NAME_BY_PID):
            return HandleGetBundleNameByPid(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::REGISTER_APP_DEBUG_LISTENER):
            return HandleRegisterAppDebugListener(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::UNREGISTER_APP_DEBUG_LISTENER):
            return HandleUnregisterAppDebugListener(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::ATTACH_APP_DEBUG):
            return HandleAttachAppDebug(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::DETACH_APP_DEBUG):
            return HandleDetachAppDebug(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::SET_APP_WAITING_DEBUG):
            return HandleSetAppWaitingDebug(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::CANCEL_APP_WAITING_DEBUG):
            return HandleCancelAppWaitingDebug(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::GET_WAITING_DEBUG_APP):
            return HandleGetWaitingDebugApp(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::IS_WAITING_DEBUG_APP):
            return HandleIsWaitingDebugApp(data, reply);
    }
    return AAFwk::ERR_CODE_NOT_EXIST;
}

int32_t AmsMgrStub::OnRemoteRequestInnerThird(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    switch (static_cast<uint32_t>(code)) {
        case static_cast<uint32_t>(IAmsMgr::Message::CLEAR_NON_PERSIST_WAITING_DEBUG_FLAG):
            return HandleClearNonPersistWaitingDebugFlag(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::REGISTER_ABILITY_DEBUG_RESPONSE):
            return HandleRegisterAbilityDebugResponse(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::IS_ATTACH_DEBUG):
            return HandleIsAttachDebug(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::CLEAR_PROCESS_BY_TOKEN):
            return HandleClearProcessByToken(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESSES_BY_PIDS):
            return HandleKillProcessesByPids(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::ATTACH_PID_TO_PARENT):
            return HandleAttachPidToParent(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::IS_MEMORY_SIZE_SUFFICIENT):
            return HandleIsMemorySizeSufficient(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::IS_NO_REQUIRE_BIG_MEMORY):
            return HandleIsNoRequireBigMemory(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::SET_KEEP_ALIVE_ENABLE_STATE):
            return HandleSetKeepAliveEnableState(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::ATTACHED_TO_STATUS_BAR):
            return HandleAttachedToStatusBar(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::UPDATE_CONFIGURATION):
            return 0;
        case static_cast<uint32_t>(IAmsMgr::Message::GET_CONFIGURATION):
            return 0;
        case static_cast<uint32_t>(IAmsMgr::Message::START_SPECIFIED_PROCESS):
            return 0;
        case static_cast<uint32_t>(IAmsMgr::Message::REGISTER_ABILITY_MS_DELEGATE):
            return 0;
        case static_cast<uint32_t>(IAmsMgr::Message::BLOCK_PROCESS_CACHE_BY_PIDS):
            return HandleBlockProcessCacheByPids(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::IS_KILLED_FOR_UPGRADE_WEB):
            return HandleIsKilledForUpgradeWeb(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::PREPARE_TERMINATE_APP):
            return HandlePrepareTerminateApp(data, reply);
    }
    return AAFwk::ERR_CODE_NOT_EXIST;
}

int32_t AmsMgrStub::OnRemoteRequestInnerFourth(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    switch (static_cast<uint32_t>(code)) {
        case static_cast<uint32_t>(IAmsMgr::Message::IS_PROCESS_CONTAINS_ONLY_UI_EXTENSION):
            return HandleIsProcessContainsOnlyUIAbility(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::FORCE_KILL_APPLICATION):
            return HandleForceKillApplication(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::CLEAN_UIABILITY_BY_USER_REQUEST):
            return HandleCleanAbilityByUserRequest(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::FORCE_KILL_APPLICATION_BY_ACCESS_TOKEN_ID):
            return HandleKillProcessesByAccessTokenId(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::IS_PROCESS_ATTACHED):
            return HandleIsProcessAttached(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::IS_CALLER_KILLING):
            return HandleIsCallerKilling(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::SET_KEEP_ALIVE_DKV):
            return HandleSetKeepAliveDkv(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::SET_KEEP_ALIVE_APP_SERVICE):
            return HandleSetKeepAliveAppService(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESSES_IN_BATCH):
            return HandleKillProcessesInBatch(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::PRELOAD_APPLICATION_BY_PHASE):
            return HandlePreloadApplicationByPhase(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_PRELOAD_ABILITY_STATE_CHANGED):
            return HandleNotifyPreloadAbilityStateChanged(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::CHECK_PRELOAD_APP_RECORD_EXIST):
            return HandleCheckPreloadAppRecordExist(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::VERIFY_KILL_PROCESS_PERMISSION):
            return HandleVerifyKillProcessPermission(data, reply);
        case static_cast<uint32_t>(IAmsMgr::Message::NOTIFY_LOAD_ABILITY_FINISHED):
            return HandleNotifyLoadAbilityFinished(data, reply);
    }
    return AAFwk::ERR_CODE_NOT_EXIST;
}

ErrCode AmsMgrStub::HandleLoadAbility(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::shared_ptr<AbilityInfo> abilityInfo(data.ReadParcelable<AbilityInfo>());
    if (!abilityInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<AbilityInfo> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    std::shared_ptr<ApplicationInfo> appInfo(data.ReadParcelable<ApplicationInfo>());
    if (!appInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<ApplicationInfo> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    std::shared_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (!want) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable want failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    std::shared_ptr<AbilityRuntime::LoadParam> loadParam(data.ReadParcelable<AbilityRuntime::LoadParam>());
    if (!loadParam) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable loadParam failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    LoadAbility(abilityInfo, appInfo, want, loadParam);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleNotifyLoadAbilityFinished(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t callingPid = data.ReadInt32();
    int32_t targetPid = data.ReadInt32();
    uint64_t callbackId = data.ReadUint64();
    NotifyLoadAbilityFinished(callingPid, targetPid, callbackId);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleTerminateAbility(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    bool clearMissionFlag = data.ReadBool();
    TerminateAbility(token, clearMissionFlag);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleUpdateAbilityState(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    int32_t state = data.ReadInt32();
    UpdateAbilityState(token, static_cast<AbilityState>(state));
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleUpdateExtensionState(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    int32_t state = data.ReadInt32();
    UpdateExtensionState(token, static_cast<ExtensionState>(state));
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleRegisterAppStateCallback(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IAppStateCallback> callback = nullptr;
    if (data.ReadBool()) {
        sptr<IRemoteObject> obj = data.ReadRemoteObject();
        callback = iface_cast<IAppStateCallback>(obj);
    }
    RegisterAppStateCallback(callback);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleKillProcessByAbilityToken(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();

    KillProcessByAbilityToken(token);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleKillProcessesByUserId(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t userId = data.ReadInt32();
    sptr<AAFwk::IUserCallback> callback = nullptr;
    bool isWithCallback = data.ReadBool();
    if (isWithCallback) {
        callback = iface_cast<AAFwk::IUserCallback>(data.ReadRemoteObject());
    }

    KillProcessesByUserId(userId, callback);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleKillProcessesByPids(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    auto size = data.ReadUint32();
    if (size == 0 || size > MAX_KILL_PROCESS_PID_COUNT) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid size");
        return ERR_INVALID_VALUE;
    }
    std::vector<int32_t> pids;
    for (uint32_t i = 0; i < size; i++) {
        pids.emplace_back(data.ReadInt32());
    }
    std::string reason = data.ReadString();
    bool subProcess = data.ReadBool();
    bool isKillPrecedeStart = data.ReadBool();
    int32_t ret = KillProcessesByPids(pids, reason, subProcess, isKillPrecedeStart);
    reply.WriteInt32(ret);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleAttachPidToParent(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    sptr<IRemoteObject> callerToken = data.ReadRemoteObject();
    AttachPidToParent(token, callerToken);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleKillProcessWithAccount(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::APPMGR, "enter");

    HITRACE_METER(HITRACE_TAG_APP);

    std::string bundleName = data.ReadString();
    int accountId = data.ReadInt32();
    bool clearPageStack = data.ReadBool();
    auto appIndex = data.ReadInt32();

    TAG_LOGI(AAFwkTag::APPMGR,
        "bundleName = %{public}s, accountId = %{public}d, clearPageStack = %{public}d",
        bundleName.c_str(), accountId, clearPageStack);

    int32_t result = KillProcessWithAccount(bundleName, accountId, clearPageStack, appIndex);
    reply.WriteInt32(result);

    TAG_LOGI(AAFwkTag::APPMGR, "end");

    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleKillProcessesInBatch(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::APPMGR, "enter");

    HITRACE_METER(HITRACE_TAG_APP);

    auto size = data.ReadUint32();
    TAG_LOGI(AAFwkTag::APPMGR, "pids.size=%{public}d", size);
    if (size == 0 || size > MAX_KILL_PROCESS_PID_COUNT) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid size");
        return ERR_INVALID_VALUE;
    }
    std::vector<int32_t> pids;
    for (uint32_t i = 0; i < size; i++) {
        pids.emplace_back(data.ReadInt32());
    }

    int32_t result = KillProcessesInBatch(pids);
    reply.WriteInt32(result);

    TAG_LOGI(AAFwkTag::APPMGR, "end");

    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleKillApplication(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::string bundleName = data.ReadString();
    bool clearPageStack = data.ReadBool();
    auto appIndex = data.ReadInt32();
    std::string reason = data.ReadString();

    TAG_LOGW(AAFwkTag::APPMGR,
        "KillApplication,callingPid=%{public}d,bundleName=%{public}s,clearPageStack=%{public}d,kill reason=%{public}s",
        IPCSkeleton::GetCallingPid(), bundleName.c_str(), clearPageStack, reason.c_str());

    int32_t result = KillApplication(bundleName, clearPageStack, appIndex, reason);
    reply.WriteInt32(result);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleForceKillApplication(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::string bundleName = data.ReadString();
    int userId = data.ReadInt32();
    int appIndex = data.ReadInt32();

    TAG_LOGI(AAFwkTag::APPMGR, "bundleName = %{public}s,userId=%{public}d,appIndex=%{public}d",
        bundleName.c_str(), userId, appIndex);

    int32_t result = ForceKillApplication(bundleName, userId, appIndex);
    reply.WriteInt32(result);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleKillProcessesByAccessTokenId(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    int accessTokenId = data.ReadInt32();

    TAG_LOGI(AAFwkTag::APPMGR, "accessTokenId=%{public}d", accessTokenId);

    int32_t result = KillProcessesByAccessTokenId(accessTokenId);
    reply.WriteInt32(result);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleKillApplicationByUid(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::string bundleName = data.ReadString();
    int uid = data.ReadInt32();
    std::string reason = data.ReadString();
    TAG_LOGW(AAFwkTag::APPMGR, "KillApplicationByUid,callingPid=%{public}d", IPCSkeleton::GetCallingPid());
    int32_t result = KillApplicationByUid(bundleName, uid, reason);
    reply.WriteInt32(result);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleNotifyUninstallOrUpgradeApp(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::string bundleName = data.ReadString();
    int32_t uid = data.ReadInt32();
    bool isUpgrade = data.ReadBool();
    TAG_LOGW(AAFwkTag::APPMGR, "NotifyUninstallOrUpgradeApp, callingPid=%{public}d", IPCSkeleton::GetCallingPid());
    int32_t result = NotifyUninstallOrUpgradeApp(bundleName, uid, isUpgrade);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "result write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleNotifyUninstallOrUpgradeAppEnd(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t uid = data.ReadInt32();
    TAG_LOGW(AAFwkTag::APPMGR, "NotifyUninstallOrUpgradeAppEnd, callingPid=%{public}d", IPCSkeleton::GetCallingPid());
    NotifyUninstallOrUpgradeAppEnd(uid);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleKillApplicationSelf(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    TAG_LOGW(AAFwkTag::APPMGR, "KillApplicationSelf,callingPid=%{public}d", IPCSkeleton::GetCallingPid());
    bool clearPageStack = data.ReadBool();
    std::string reason = data.ReadString();
    int32_t result = KillApplicationSelf(clearPageStack, reason);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "result write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleAbilityAttachTimeOut(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    AbilityAttachTimeOut(token);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandlePrepareTerminate(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    bool clearMissionFlag = data.ReadBool();
    PrepareTerminate(token, clearMissionFlag);
    return NO_ERROR;
}

void AmsMgrStub::UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state)
{}

int32_t AmsMgrStub::HandleGetRunningProcessInfoByToken(MessageParcel &data, MessageParcel &reply)
{
    RunningProcessInfo processInfo;
    auto token = data.ReadRemoteObject();
    GetRunningProcessInfoByToken(token, processInfo);
    if (reply.WriteParcelable(&processInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "process info write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleSetAbilityForegroundingFlagToAppRecord(MessageParcel &data, MessageParcel &reply)
{
    RunningProcessInfo processInfo;
    auto pid = static_cast<pid_t>(data.ReadInt32());
    SetAbilityForegroundingFlagToAppRecord(pid);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandlePrepareTerminateApp(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto pid = static_cast<pid_t>(data.ReadInt32());
    auto moduleName = data.ReadString();
    PrepareTerminateApp(pid, moduleName);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleStartSpecifiedAbility(MessageParcel &data, MessageParcel &reply)
{
    AAFwk::Want *want = data.ReadParcelable<AAFwk::Want>();
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }

    AbilityInfo *abilityInfo = data.ReadParcelable<AbilityInfo>();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityInfo is nullptr.");
        delete want;
        return ERR_INVALID_VALUE;
    }
    StartSpecifiedAbility(*want, *abilityInfo, data.ReadInt32(), data.ReadString());
    delete want;
    delete abilityInfo;
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleRegisterStartSpecifiedAbilityResponse(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    sptr<IStartSpecifiedAbilityResponse> response = iface_cast<IStartSpecifiedAbilityResponse>(obj);
    RegisterStartSpecifiedAbilityResponse(response);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleGetApplicationInfoByProcessID(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t pid = data.ReadInt32();
    AppExecFwk::ApplicationInfo application;
    bool debug;
    int32_t result = GetApplicationInfoByProcessID(pid, application, debug);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write result error.");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteParcelable(&application)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write application info failed");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteBool(debug)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write debug info failed");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleNotifyAppMgrRecordExitReason(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    int32_t pid = data.ReadInt32();
    int32_t reason = data.ReadInt32();
    std::string exitMsg = Str16ToStr8(data.ReadString16());
    int32_t result = NotifyAppMgrRecordExitReason(pid, reason, exitMsg);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write result failed.");
        return IPC_PROXY_ERR;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleNotifyAppMgrRecordExitReasonCompability(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    int32_t pid = data.ReadInt32();
    int32_t killId = data.ReadInt32();
    std::string killMsg = data.ReadString();
    std::string innerMsg = data.ReadString();
    int32_t result = NotifyAppMgrRecordExitReasonCompability(pid, killId, killMsg, innerMsg);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write result failed.");
        return IPC_PROXY_ERR;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleUpdateApplicationInfoInstalled(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::string bundleName = data.ReadString();
    int uid = data.ReadInt32();
    std::string moduleName = data.ReadString();
    int32_t result = UpdateApplicationInfoInstalled(bundleName, uid, moduleName, data.ReadBool());
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleSetEnableStartProcessFlagByUserId(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();
    bool enableStartProcess = data.ReadBool();
    SetEnableStartProcessFlagByUserId(userId, enableStartProcess);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleGetBundleNameByPid(MessageParcel &data, MessageParcel &reply)
{
    int32_t pid = data.ReadInt32();
    std::string bundleName;
    int32_t uid;
    GetBundleNameByPid(pid, bundleName, uid);

    reply.WriteString(bundleName);
    reply.WriteInt32(uid);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleRegisterAppDebugListener(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto appDebugLister = iface_cast<IAppDebugListener>(data.ReadRemoteObject());
    if (appDebugLister == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "App debug lister is null.");
        return ERR_INVALID_VALUE;
    }

    auto result = RegisterAppDebugListener(appDebugLister);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleUnregisterAppDebugListener(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto appDebugLister = iface_cast<IAppDebugListener>(data.ReadRemoteObject());
    if (appDebugLister == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "App debug lister is nullptr.");
        return ERR_INVALID_VALUE;
    }

    auto result = UnregisterAppDebugListener(appDebugLister);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleAttachAppDebug(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto bundleName = data.ReadString();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Bundle name is empty.");
        return ERR_INVALID_VALUE;
    }
    auto isDebugFromLocal = data.ReadBool();

    auto result = AttachAppDebug(bundleName, isDebugFromLocal);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleDetachAppDebug(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto bundleName = data.ReadString();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Bundle name is empty.");
        return ERR_INVALID_VALUE;
    }

    auto result = DetachAppDebug(bundleName);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleSetAppWaitingDebug(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto bundleName = data.ReadString();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Bundle name is empty.");
        return ERR_INVALID_VALUE;
    }
    auto isPersist = data.ReadBool();
    auto result = SetAppWaitingDebug(bundleName, isPersist);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleCancelAppWaitingDebug(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto result = CancelAppWaitingDebug();
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleGetWaitingDebugApp(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::vector<std::string> debugInfoList;
    auto result = GetWaitingDebugApp(debugInfoList);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }

    int32_t listSize = static_cast<int32_t>(debugInfoList.size());
    if (listSize > MAX_APP_DEBUG_COUNT) {
        TAG_LOGE(AAFwkTag::APPMGR, "Max app debug count is %{public}d.", listSize);
        return ERR_INVALID_VALUE;
    }

    if (!reply.WriteInt32(listSize)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write list size.");
        return ERR_INVALID_VALUE;
    }

    if (!reply.WriteStringVector(debugInfoList)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write string vector debug info list.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleIsWaitingDebugApp(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto bundleName = data.ReadString();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Bundle name is empty.");
        return ERR_INVALID_VALUE;
    }

    auto result = IsWaitingDebugApp(bundleName);
    if (!reply.WriteBool(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleSetKeepAliveEnableState(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto bundleName = data.ReadString();
    auto enable = data.ReadBool();
    auto uid = data.ReadInt32();
    SetKeepAliveEnableState(bundleName, enable, uid);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleSetKeepAliveDkv(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto bundleName = data.ReadString();
    auto enable = data.ReadBool();
    auto uid = data.ReadInt32();
    SetKeepAliveDkv(bundleName, enable, uid);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleSetKeepAliveAppService(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "HandleSetKeepAliveAppService called");
    auto bundleName = data.ReadString();
    auto enable = data.ReadBool();
    auto uid = data.ReadInt32();
    SetKeepAliveAppService(bundleName, enable, uid);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleClearNonPersistWaitingDebugFlag(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    ClearNonPersistWaitingDebugFlag();
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleRegisterAbilityDebugResponse(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto response = iface_cast<IAbilityDebugResponse>(data.ReadRemoteObject());
    if (response == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Response is nullptr.");
        return ERR_INVALID_VALUE;
    }

    auto result = RegisterAbilityDebugResponse(response);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleIsAttachDebug(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto bundleName = data.ReadString();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Bundle name is empty.");
        return ERR_INVALID_VALUE;
    }

    auto result = IsAttachDebug(bundleName);
    if (!reply.WriteBool(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleClearProcessByToken(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    ClearProcessByToken(token);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleIsMemorySizeSufficient(MessageParcel &data, MessageParcel &reply)
{
    auto result = IsMemorySizeSufficient();
    if (!reply.WriteBool(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleIsNoRequireBigMemory(MessageParcel &data, MessageParcel &reply)
{
    auto result = IsNoRequireBigMemory();
    if (!reply.WriteBool(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleAttachedToStatusBar(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    AttachedToStatusBar(token);
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleBlockProcessCacheByPids(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    auto size = data.ReadUint32();
    if (size == 0 || size > MAX_KILL_PROCESS_PID_COUNT) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid size.");
        return ERR_INVALID_VALUE;
    }
    std::vector<int32_t> pids;
    for (uint32_t i = 0; i < size; i++) {
        pids.emplace_back(data.ReadInt32());
    }

    BlockProcessCacheByPids(pids);
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleIsKilledForUpgradeWeb(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto bundleName = data.ReadString();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Bundle name is empty.");
        return ERR_INVALID_VALUE;
    }

    auto result = IsKilledForUpgradeWeb(bundleName);
    if (!reply.WriteBool(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

ErrCode AmsMgrStub::HandleCleanAbilityByUserRequest(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    auto result = CleanAbilityByUserRequest(token);
    if (!reply.WriteBool(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "fail to write the result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleIsProcessContainsOnlyUIAbility(MessageParcel &data, MessageParcel &reply)
{
    auto pid = data.ReadUint32();

    auto result = IsProcessContainsOnlyUIAbility(pid);
    if (!reply.WriteBool(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result in HandleIsProcessContainsOnlyUIAbility.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleIsProcessAttached(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    auto isAttached = IsProcessAttached(token);
    if (!reply.WriteBool(isAttached)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleIsCallerKilling(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    auto callerKey = data.ReadString();
    auto isCallerKilling = IsCallerKilling(callerKey);
    if (!reply.WriteBool(isCallerKilling)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandlePreloadApplicationByPhase(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    auto bundleName = data.ReadString();
    auto userId = data.ReadInt32();
    auto appIndex = data.ReadInt32();
    auto preloadPhase = data.ReadInt32();
    auto ret = PreloadApplicationByPhase(bundleName, userId, appIndex,
        static_cast<AppExecFwk::PreloadPhase>(preloadPhase));
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write ret");
        return AAFwk::ERR_WRITE_RESULT_CODE_FAILED;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleNotifyPreloadAbilityStateChanged(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    bool isPreForeground = data.ReadBool();
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null token");
        return AAFwk::INVALID_CALLER_TOKEN;
    }
    auto ret = NotifyPreloadAbilityStateChanged(token, isPreForeground);
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write ret");
        return AAFwk::ERR_WRITE_RESULT_CODE_FAILED;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleCheckPreloadAppRecordExist(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    auto bundleName = data.ReadString();
    auto userId = data.ReadInt32();
    auto appIndex = data.ReadInt32();
    bool isExist = false;
    auto ret = CheckPreloadAppRecordExist(bundleName, userId, appIndex, isExist);
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write ret");
        return AAFwk::ERR_WRITE_RESULT_CODE_FAILED;
    }
    if (ret == NO_ERROR && !reply.WriteBool(isExist)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write isExist");
        return AAFwk::ERR_WRITE_BOOL_FAILED;
    }
    return NO_ERROR;
}

int32_t AmsMgrStub::HandleVerifyKillProcessPermission(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto bundleName = data.ReadString();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Bundle name is empty.");
        return ERR_INVALID_VALUE;
    }

    auto result = VerifyKillProcessPermission(bundleName);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Fail to write result.");
        return AAFwk::ERR_WRITE_RESULT_CODE_FAILED;
    }
    return NO_ERROR;
}
}  // namespace AppExecFwk
}  // namespace OHOS
