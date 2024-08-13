/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "ability_manager_stub.h"

#include "ability_manager_errors.h"
#include "ability_manager_radar.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "status_bar_delegate_interface.h"
#include <iterator>

namespace OHOS {
namespace AAFwk {
using AutoStartupInfo = AbilityRuntime::AutoStartupInfo;
namespace {
const std::u16string extensionDescriptor = u"ohos.aafwk.ExtensionManager";
constexpr int32_t CYCLE_LIMIT = 1000;
constexpr int32_t MAX_KILL_PROCESS_PID_COUNT = 100;
} // namespace
AbilityManagerStub::AbilityManagerStub()
{}

AbilityManagerStub::~AbilityManagerStub()
{}

int AbilityManagerStub::OnRemoteRequestInnerFirst(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    AbilityManagerInterfaceCode interfaceCode = static_cast<AbilityManagerInterfaceCode>(code);
    if (interfaceCode == AbilityManagerInterfaceCode::TERMINATE_ABILITY) {
        return TerminateAbilityInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::MINIMIZE_ABILITY) {
        return MinimizeAbilityInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::ATTACH_ABILITY_THREAD) {
        return AttachAbilityThreadInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::ABILITY_TRANSITION_DONE) {
        return AbilityTransitionDoneInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::ABILITY_WINDOW_CONFIG_TRANSITION_DONE) {
        return AbilityWindowConfigTransitionDoneInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::CONNECT_ABILITY_DONE) {
        return ScheduleConnectAbilityDoneInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::DISCONNECT_ABILITY_DONE) {
        return ScheduleDisconnectAbilityDoneInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::COMMAND_ABILITY_DONE) {
        return ScheduleCommandAbilityDoneInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::COMMAND_ABILITY_WINDOW_DONE) {
        return ScheduleCommandAbilityWindowDoneInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::ACQUIRE_DATA_ABILITY) {
        return AcquireDataAbilityInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::RELEASE_DATA_ABILITY) {
        return ReleaseDataAbilityInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::BACK_TO_CALLER_UIABILITY) {
        return BackToCallerInner(data, reply);
    }
    return ERR_CODE_NOT_EXIST;
}

int AbilityManagerStub::OnRemoteRequestInnerSecond(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    AbilityManagerInterfaceCode interfaceCode = static_cast<AbilityManagerInterfaceCode>(code);
    if (interfaceCode == AbilityManagerInterfaceCode::KILL_PROCESS) {
        return KillProcessInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::UNINSTALL_APP) {
        return UninstallAppInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::UPGRADE_APP) {
        return UpgradeAppInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::START_ABILITY) {
        return StartAbilityInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::START_ABILITY_ADD_CALLER) {
        return StartAbilityAddCallerInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::START_ABILITY_WITH_SPECIFY_TOKENID) {
        return StartAbilityInnerSpecifyTokenId(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::START_ABILITY_AS_CALLER_BY_TOKEN) {
        return StartAbilityAsCallerByTokenInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::START_ABILITY_AS_CALLER_FOR_OPTIONS) {
        return StartAbilityAsCallerForOptionInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::START_UI_SESSION_ABILITY_ADD_CALLER) {
        return StartAbilityByUIContentSessionAddCallerInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::START_UI_SESSION_ABILITY_FOR_OPTIONS) {
        return StartAbilityByUIContentSessionForOptionsInner(data, reply);
    }
    return ERR_CODE_NOT_EXIST;
}

int AbilityManagerStub::OnRemoteRequestInnerThird(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    AbilityManagerInterfaceCode interfaceCode = static_cast<AbilityManagerInterfaceCode>(code);
    if (interfaceCode == AbilityManagerInterfaceCode::START_ABILITY_BY_INSIGHT_INTENT) {
        return StartAbilityByInsightIntentInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::CONNECT_ABILITY) {
        return ConnectAbilityInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::DISCONNECT_ABILITY) {
        return DisconnectAbilityInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::STOP_SERVICE_ABILITY) {
        return StopServiceAbilityInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::DUMP_STATE) {
        return DumpStateInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::DUMPSYS_STATE) {
        return DumpSysStateInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::START_ABILITY_FOR_SETTINGS) {
        return StartAbilityForSettingsInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::CONTINUE_MISSION) {
        return ContinueMissionInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::CONTINUE_MISSION_OF_BUNDLENAME) {
        return ContinueMissionOfBundleNameInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::CONTINUE_ABILITY) {
        return ContinueAbilityInner(data, reply);
    }
    return ERR_CODE_NOT_EXIST;
}

int AbilityManagerStub::OnRemoteRequestInnerFourth(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    AbilityManagerInterfaceCode interfaceCode = static_cast<AbilityManagerInterfaceCode>(code);
    if (interfaceCode == AbilityManagerInterfaceCode::START_CONTINUATION) {
        return StartContinuationInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::NOTIFY_COMPLETE_CONTINUATION) {
        return NotifyCompleteContinuationInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::NOTIFY_CONTINUATION_RESULT) {
        return NotifyContinuationResultInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::SEND_RESULT_TO_ABILITY) {
        return SendResultToAbilityInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::REGISTER_REMOTE_MISSION_LISTENER) {
        return RegisterRemoteMissionListenerInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::REGISTER_REMOTE_ON_LISTENER) {
        return RegisterRemoteOnListenerInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::REGISTER_REMOTE_OFF_LISTENER) {
        return RegisterRemoteOffListenerInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::UNREGISTER_REMOTE_MISSION_LISTENER) {
        return UnRegisterRemoteMissionListenerInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::START_ABILITY_FOR_OPTIONS) {
        return StartAbilityForOptionsInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::START_SYNC_MISSIONS) {
        return StartSyncRemoteMissionsInner(data, reply);
    }
    return ERR_CODE_NOT_EXIST;
}

int AbilityManagerStub::OnRemoteRequestInnerFifth(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    AbilityManagerInterfaceCode interfaceCode = static_cast<AbilityManagerInterfaceCode>(code);
    if (interfaceCode == AbilityManagerInterfaceCode::STOP_SYNC_MISSIONS) {
        return StopSyncRemoteMissionsInner(data, reply);
    }
#ifdef ABILITY_COMMAND_FOR_TEST
    if (interfaceCode == AbilityManagerInterfaceCode::FORCE_TIMEOUT) {
        return ForceTimeoutForTestInner(data, reply);
    }
#endif
    if (interfaceCode == AbilityManagerInterfaceCode::FREE_INSTALL_ABILITY_FROM_REMOTE) {
        return FreeInstallAbilityFromRemoteInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::ADD_FREE_INSTALL_OBSERVER) {
        return AddFreeInstallObserverInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::CONNECT_ABILITY_WITH_TYPE) {
        return ConnectAbilityWithTypeInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::ABILITY_RECOVERY) {
        return ScheduleRecoverAbilityInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::ABILITY_RECOVERY_ENABLE) {
        return EnableRecoverAbilityInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::ABILITY_RECOVERY_SUBMITINFO) {
        return SubmitSaveRecoveryInfoInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::CLEAR_RECOVERY_PAGE_STACK) {
        return ScheduleClearRecoveryPageStackInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::MINIMIZE_UI_ABILITY_BY_SCB) {
        return MinimizeUIAbilityBySCBInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::CLOSE_UI_ABILITY_BY_SCB) {
        return CloseUIAbilityBySCBInner(data, reply);
    }
    return ERR_CODE_NOT_EXIST;
}

int AbilityManagerStub::OnRemoteRequestInnerSixth(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    AbilityManagerInterfaceCode interfaceCode = static_cast<AbilityManagerInterfaceCode>(code);
    if (interfaceCode == AbilityManagerInterfaceCode::REGISTER_COLLABORATOR) {
        return RegisterIAbilityManagerCollaboratorInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::UNREGISTER_COLLABORATOR) {
        return UnregisterIAbilityManagerCollaboratorInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::REGISTER_APP_DEBUG_LISTENER) {
        return RegisterAppDebugListenerInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::UNREGISTER_APP_DEBUG_LISTENER) {
        return UnregisterAppDebugListenerInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::ATTACH_APP_DEBUG) {
        return AttachAppDebugInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::DETACH_APP_DEBUG) {
        return DetachAppDebugInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::IS_ABILITY_CONTROLLER_START) {
        return IsAbilityControllerStartInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::EXECUTE_INTENT) {
        return ExecuteIntentInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::EXECUTE_INSIGHT_INTENT_DONE) {
        return ExecuteInsightIntentDoneInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::OPEN_FILE) {
        return OpenFileInner(data, reply);
    }
    return ERR_CODE_NOT_EXIST;
}

int AbilityManagerStub::OnRemoteRequestInnerSeventh(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    AbilityManagerInterfaceCode interfaceCode = static_cast<AbilityManagerInterfaceCode>(code);
    if (interfaceCode == AbilityManagerInterfaceCode::GET_PENDING_WANT_SENDER) {
        return GetWantSenderInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::SEND_PENDING_WANT_SENDER) {
        return SendWantSenderInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::CANCEL_PENDING_WANT_SENDER) {
        return CancelWantSenderInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::GET_PENDING_WANT_UID) {
        return GetPendingWantUidInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::GET_PENDING_WANT_USERID) {
        return GetPendingWantUserIdInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::GET_PENDING_WANT_BUNDLENAME) {
        return GetPendingWantBundleNameInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::GET_PENDING_WANT_CODE) {
        return GetPendingWantCodeInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::GET_PENDING_WANT_TYPE) {
        return GetPendingWantTypeInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::REGISTER_CANCEL_LISTENER) {
        return RegisterCancelListenerInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::UNREGISTER_CANCEL_LISTENER) {
        return UnregisterCancelListenerInner(data, reply);
    }
    return ERR_CODE_NOT_EXIST;
}

int AbilityManagerStub::OnRemoteRequestInnerEighth(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    AbilityManagerInterfaceCode interfaceCode = static_cast<AbilityManagerInterfaceCode>(code);
    if (interfaceCode == AbilityManagerInterfaceCode::GET_PENDING_REQUEST_WANT) {
        return GetPendingRequestWantInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::GET_PENDING_WANT_SENDER_INFO) {
        return GetWantSenderInfoInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::GET_APP_MEMORY_SIZE) {
        return GetAppMemorySizeInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::IS_RAM_CONSTRAINED_DEVICE) {
        return IsRamConstrainedDeviceInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::LOCK_MISSION_FOR_CLEANUP) {
        return LockMissionForCleanupInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::UNLOCK_MISSION_FOR_CLEANUP) {
        return UnlockMissionForCleanupInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::SET_SESSION_LOCKED_STATE) {
        return SetLockedStateInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::REGISTER_MISSION_LISTENER) {
        return RegisterMissionListenerInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::UNREGISTER_MISSION_LISTENER) {
        return UnRegisterMissionListenerInner(data, reply);
    }
    return ERR_CODE_NOT_EXIST;
}

int AbilityManagerStub::OnRemoteRequestInnerNinth(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    AbilityManagerInterfaceCode interfaceCode = static_cast<AbilityManagerInterfaceCode>(code);
    if (interfaceCode == AbilityManagerInterfaceCode::GET_MISSION_INFOS) {
        return GetMissionInfosInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::GET_MISSION_INFO_BY_ID) {
        return GetMissionInfoInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::CLEAN_MISSION) {
        return CleanMissionInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::CLEAN_ALL_MISSIONS) {
        return CleanAllMissionsInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::MOVE_MISSION_TO_FRONT) {
        return MoveMissionToFrontInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::MOVE_MISSION_TO_FRONT_BY_OPTIONS) {
        return MoveMissionToFrontByOptionsInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::MOVE_MISSIONS_TO_FOREGROUND) {
        return MoveMissionsToForegroundInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::MOVE_MISSIONS_TO_BACKGROUND) {
        return MoveMissionsToBackgroundInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::START_CALL_ABILITY) {
        return StartAbilityByCallInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::CALL_REQUEST_DONE) {
        return CallRequestDoneInner(data, reply);
    }
    return ERR_CODE_NOT_EXIST;
}

int AbilityManagerStub::OnRemoteRequestInnerTenth(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    AbilityManagerInterfaceCode interfaceCode = static_cast<AbilityManagerInterfaceCode>(code);
    if (interfaceCode == AbilityManagerInterfaceCode::RELEASE_CALL_ABILITY) {
        return ReleaseCallInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::START_USER) {
        return StartUserInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::STOP_USER) {
        return StopUserInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::LOGOUT_USER) {
        return LogoutUserInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::GET_ABILITY_RUNNING_INFO) {
        return GetAbilityRunningInfosInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::GET_EXTENSION_RUNNING_INFO) {
        return GetExtensionRunningInfosInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::GET_PROCESS_RUNNING_INFO) {
        return GetProcessRunningInfosInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::SET_ABILITY_CONTROLLER) {
        return SetAbilityControllerInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::GET_MISSION_SNAPSHOT_INFO) {
        return GetMissionSnapshotInfoInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::IS_USER_A_STABILITY_TEST) {
        return IsRunningInStabilityTestInner(data, reply);
    }
    return ERR_CODE_NOT_EXIST;
}

int AbilityManagerStub::OnRemoteRequestInnerEleventh(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    AbilityManagerInterfaceCode interfaceCode = static_cast<AbilityManagerInterfaceCode>(code);
    if (interfaceCode == AbilityManagerInterfaceCode::ACQUIRE_SHARE_DATA) {
        return AcquireShareDataInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::SHARE_DATA_DONE) {
        return ShareDataDoneInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::GET_ABILITY_TOKEN) {
        return GetAbilityTokenByCalleeObjInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::FORCE_EXIT_APP) {
        return ForceExitAppInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::RECORD_APP_EXIT_REASON) {
        return RecordAppExitReasonInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::RECORD_PROCESS_EXIT_REASON) {
        return RecordProcessExitReasonInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::REGISTER_SESSION_HANDLER) {
        return RegisterSessionHandlerInner(data, reply);
    }
#ifdef ABILITY_COMMAND_FOR_TEST
    if (interfaceCode == AbilityManagerInterfaceCode::BLOCK_ABILITY) {
        return BlockAbilityInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::BLOCK_AMS_SERVICE) {
        return BlockAmsServiceInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::BLOCK_APP_SERVICE) {
        return BlockAppServiceInner(data, reply);
    }
#endif
    return ERR_CODE_NOT_EXIST;
}

int AbilityManagerStub::OnRemoteRequestInnerTwelveth(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    AbilityManagerInterfaceCode interfaceCode = static_cast<AbilityManagerInterfaceCode>(code);
    if (interfaceCode == AbilityManagerInterfaceCode::START_USER_TEST) {
        return StartUserTestInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::FINISH_USER_TEST) {
        return FinishUserTestInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::GET_TOP_ABILITY_TOKEN) {
        return GetTopAbilityTokenInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::CHECK_UI_EXTENSION_IS_FOCUSED) {
        return CheckUIExtensionIsFocusedInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::DELEGATOR_DO_ABILITY_FOREGROUND) {
        return DelegatorDoAbilityForegroundInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::DELEGATOR_DO_ABILITY_BACKGROUND) {
        return DelegatorDoAbilityBackgroundInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::DO_ABILITY_FOREGROUND) {
        return DoAbilityForegroundInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::DO_ABILITY_BACKGROUND) {
        return DoAbilityBackgroundInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::GET_MISSION_ID_BY_ABILITY_TOKEN) {
        return GetMissionIdByTokenInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::GET_TOP_ABILITY) {
        return GetTopAbilityInner(data, reply);
    }
    return ERR_CODE_NOT_EXIST;
}

int AbilityManagerStub::OnRemoteRequestInnerThirteenth(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    AbilityManagerInterfaceCode interfaceCode = static_cast<AbilityManagerInterfaceCode>(code);
    if (interfaceCode == AbilityManagerInterfaceCode::GET_ELEMENT_NAME_BY_TOKEN) {
        return GetElementNameByTokenInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::DUMP_ABILITY_INFO_DONE) {
        return DumpAbilityInfoDoneInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::START_EXTENSION_ABILITY) {
        return StartExtensionAbilityInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::STOP_EXTENSION_ABILITY) {
        return StopExtensionAbilityInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::UPDATE_MISSION_SNAPSHOT_FROM_WMS) {
        return UpdateMissionSnapShotFromWMSInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::REGISTER_CONNECTION_OBSERVER) {
        return RegisterConnectionObserverInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::UNREGISTER_CONNECTION_OBSERVER) {
        return UnregisterConnectionObserverInner(data, reply);
    }
#ifdef WITH_DLP
    if (interfaceCode == AbilityManagerInterfaceCode::GET_DLP_CONNECTION_INFOS) {
        return GetDlpConnectionInfosInner(data, reply);
    }
#endif // WITH_DLP
    if (interfaceCode == AbilityManagerInterfaceCode::MOVE_ABILITY_TO_BACKGROUND) {
        return MoveAbilityToBackgroundInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::MOVE_UI_ABILITY_TO_BACKGROUND) {
        return MoveUIAbilityToBackgroundInner(data, reply);
    }
    return ERR_CODE_NOT_EXIST;
}

int AbilityManagerStub::OnRemoteRequestInnerFourteenth(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    AbilityManagerInterfaceCode interfaceCode = static_cast<AbilityManagerInterfaceCode>(code);
    if (interfaceCode == AbilityManagerInterfaceCode::SET_MISSION_CONTINUE_STATE) {
        return SetMissionContinueStateInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::PREPARE_TERMINATE_ABILITY_BY_SCB) {
        return PrepareTerminateAbilityBySCBInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::REQUESET_MODAL_UIEXTENSION) {
        return RequestModalUIExtensionInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::GET_UI_EXTENSION_ROOT_HOST_INFO) {
        return GetUIExtensionRootHostInfoInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::GET_UI_EXTENSION_SESSION_INFO) {
        return GetUIExtensionSessionInfoInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::PRELOAD_UIEXTENSION_ABILITY) {
        return PreloadUIExtensionAbilityInner(data, reply);
    }
    return ERR_CODE_NOT_EXIST;
}

int AbilityManagerStub::OnRemoteRequestInnerFifteenth(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    AbilityManagerInterfaceCode interfaceCode = static_cast<AbilityManagerInterfaceCode>(code);
#ifdef SUPPORT_GRAPHICS
    if (interfaceCode == AbilityManagerInterfaceCode::SET_MISSION_LABEL) {
        return SetMissionLabelInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::SET_MISSION_ICON) {
        return SetMissionIconInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::REGISTER_WMS_HANDLER) {
        return RegisterWindowManagerServiceHandlerInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::COMPLETEFIRSTFRAMEDRAWING) {
        return CompleteFirstFrameDrawingInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::START_UI_EXTENSION_ABILITY) {
        return StartUIExtensionAbilityInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::MINIMIZE_UI_EXTENSION_ABILITY) {
        return MinimizeUIExtensionAbilityInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::TERMINATE_UI_EXTENSION_ABILITY) {
        return TerminateUIExtensionAbilityInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::CONNECT_UI_EXTENSION_ABILITY) {
        return ConnectUIExtensionAbilityInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::PREPARE_TERMINATE_ABILITY) {
        return PrepareTerminateAbilityInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::GET_DIALOG_SESSION_INFO) {
        return GetDialogSessionInfoInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::SEND_DIALOG_RESULT) {
        return SendDialogResultInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::REGISTER_ABILITY_FIRST_FRAME_STATE_OBSERVER) {
        return RegisterAbilityFirstFrameStateObserverInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::UNREGISTER_ABILITY_FIRST_FRAME_STATE_OBSERVER) {
        return UnregisterAbilityFirstFrameStateObserverInner(data, reply);
    }
#endif
    return ERR_CODE_NOT_EXIST;
}

int AbilityManagerStub::OnRemoteRequestInnerSixteenth(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    AbilityManagerInterfaceCode interfaceCode = static_cast<AbilityManagerInterfaceCode>(code);
#ifdef SUPPORT_GRAPHICS
    if (interfaceCode == AbilityManagerInterfaceCode::COMPLETE_FIRST_FRAME_DRAWING_BY_SCB) {
        return CompleteFirstFrameDrawingBySCBInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::START_UI_EXTENSION_ABILITY_EMBEDDED) {
        return StartUIExtensionAbilityEmbeddedInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::START_UI_EXTENSION_CONSTRAINED_EMBEDDED) {
        return StartUIExtensionConstrainedEmbeddedInner(data, reply);
    }
#endif
    if (interfaceCode == AbilityManagerInterfaceCode::REQUEST_DIALOG_SERVICE) {
        return HandleRequestDialogService(data, reply);
    };
    if (interfaceCode == AbilityManagerInterfaceCode::REPORT_DRAWN_COMPLETED) {
        return HandleReportDrawnCompleted(data, reply);
    };
    if (interfaceCode == AbilityManagerInterfaceCode::QUERY_MISSION_VAILD) {
        return IsValidMissionIdsInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::VERIFY_PERMISSION) {
        return VerifyPermissionInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::START_UI_ABILITY_BY_SCB) {
        return StartUIAbilityBySCBInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::SET_ROOT_SCENE_SESSION) {
        return SetRootSceneSessionInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::CALL_ABILITY_BY_SCB) {
        return CallUIAbilityBySCBInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::START_SPECIFIED_ABILITY_BY_SCB) {
        return StartSpecifiedAbilityBySCBInner(data, reply);
    }
    return ERR_CODE_NOT_EXIST;
}

int AbilityManagerStub::OnRemoteRequestInnerSeventeenth(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    AbilityManagerInterfaceCode interfaceCode = static_cast<AbilityManagerInterfaceCode>(code);
    if (interfaceCode == AbilityManagerInterfaceCode::NOTIFY_SAVE_AS_RESULT) {
        return NotifySaveAsResultInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::SET_SESSIONMANAGERSERVICE) {
        return SetSessionManagerServiceInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::UPDATE_SESSION_INFO) {
        return UpdateSessionInfoBySCBInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::REGISTER_STATUS_BAR_DELEGATE) {
        return RegisterStatusBarDelegateInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::KILL_PROCESS_WITH_PREPARE_TERMINATE) {
        return KillProcessWithPrepareTerminateInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::REGISTER_AUTO_STARTUP_SYSTEM_CALLBACK) {
        return RegisterAutoStartupSystemCallbackInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::UNREGISTER_AUTO_STARTUP_SYSTEM_CALLBACK) {
        return UnregisterAutoStartupSystemCallbackInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::SET_APPLICATION_AUTO_STARTUP) {
        return SetApplicationAutoStartupInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::CANCEL_APPLICATION_AUTO_STARTUP) {
        return CancelApplicationAutoStartupInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::QUERY_ALL_AUTO_STARTUP_APPLICATION) {
        return QueryAllAutoStartupApplicationsInner(data, reply);
    }
    return ERR_CODE_NOT_EXIST;
}

int AbilityManagerStub::OnRemoteRequestInnerEighteenth(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    AbilityManagerInterfaceCode interfaceCode = static_cast<AbilityManagerInterfaceCode>(code);
    if (interfaceCode == AbilityManagerInterfaceCode::GET_CONNECTION_DATA) {
        return GetConnectionDataInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::SET_APPLICATION_AUTO_STARTUP_BY_EDM) {
        return SetApplicationAutoStartupByEDMInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::CANCEL_APPLICATION_AUTO_STARTUP_BY_EDM) {
        return CancelApplicationAutoStartupByEDMInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::START_ABILITY_FOR_RESULT_AS_CALLER) {
        return StartAbilityForResultAsCallerInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::START_ABILITY_FOR_RESULT_AS_CALLER_FOR_OPTIONS) {
        return StartAbilityForResultAsCallerForOptionsInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::GET_FOREGROUND_UI_ABILITIES) {
        return GetForegroundUIAbilitiesInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::RESTART_APP) {
        return RestartAppInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::OPEN_ATOMIC_SERVICE) {
        return OpenAtomicServiceInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::IS_EMBEDDED_OPEN_ALLOWED) {
        return IsEmbeddedOpenAllowedInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::REQUEST_ASSERT_FAULT_DIALOG) {
        return RequestAssertFaultDialogInner(data, reply);
    }
    return ERR_CODE_NOT_EXIST;
}


int AbilityManagerStub::OnRemoteRequestInnerNineteenth(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    AbilityManagerInterfaceCode interfaceCode = static_cast<AbilityManagerInterfaceCode>(code);
    if (interfaceCode == AbilityManagerInterfaceCode::NOTIFY_DEBUG_ASSERT_RESULT) {
        return NotifyDebugAssertResultInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::CHANGE_ABILITY_VISIBILITY) {
        return ChangeAbilityVisibilityInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::CHANGE_UI_ABILITY_VISIBILITY_BY_SCB) {
        return ChangeUIAbilityVisibilityBySCBInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::START_SHORTCUT) {
        return StartShortcutInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::SET_RESIDENT_PROCESS_ENABLE) {
        return SetResidentProcessEnableInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::GET_ABILITY_STATE_BY_PERSISTENT_ID) {
        return GetAbilityStateByPersistentIdInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::TRANSFER_ABILITY_RESULT) {
        return TransferAbilityResultForExtensionInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::NOTIFY_FROZEN_PROCESS_BY_RSS) {
        return NotifyFrozenProcessByRSSInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::PRE_START_MISSION) {
        return PreStartMissionInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::CLEAN_UI_ABILITY_BY_SCB) {
        return CleanUIAbilityBySCBInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::OPEN_LINK) {
        return OpenLinkInner(data, reply);
    }
    if (interfaceCode == AbilityManagerInterfaceCode::TERMINATE_MISSION) {
        return TerminateMissionInner(data, reply);
    }
    return ERR_CODE_NOT_EXIST;
}

int AbilityManagerStub::OnRemoteRequestInner(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    int retCode = ERR_OK;
    retCode = HandleOnRemoteRequestInnerFirst(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = HandleOnRemoteRequestInnerSecond(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    TAG_LOGW(AAFwkTag::ABILITYMGR, "default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int AbilityManagerStub::HandleOnRemoteRequestInnerFirst(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    int retCode = ERR_OK;
    retCode = OnRemoteRequestInnerFirst(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerSecond(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerThird(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerFourth(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerFifth(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerSixth(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerSeventh(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerEighth(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerNinth(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerTenth(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    return ERR_CODE_NOT_EXIST;
}

int AbilityManagerStub::HandleOnRemoteRequestInnerSecond(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    int retCode = ERR_OK;
    retCode = OnRemoteRequestInnerEleventh(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerTwelveth(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerThirteenth(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerFourteenth(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerFifteenth(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerSixteenth(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerSeventeenth(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerEighteenth(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    retCode = OnRemoteRequestInnerNineteenth(code, data, reply, option);
    if (retCode != ERR_CODE_NOT_EXIST) {
        return retCode;
    }
    return ERR_CODE_NOT_EXIST;
}

int AbilityManagerStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Received code : %{public}d", code);
    std::u16string abilityDescriptor = AbilityManagerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (abilityDescriptor != remoteDescriptor && extensionDescriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "local descriptor is unequal to remote");
        return ERR_INVALID_STATE;
    }

    return OnRemoteRequestInner(code, data, reply, option);
}

int AbilityManagerStub::GetTopAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    bool isNeedLocalDeviceId = data.ReadBool();
    AppExecFwk::ElementName result = GetTopAbility(isNeedLocalDeviceId);
    if (result.GetDeviceID().empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "GetTopAbilityInner is nullptr");
    }
    reply.WriteParcelable(&result);
    return NO_ERROR;
}

int AbilityManagerStub::GetElementNameByTokenInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    bool isNeedLocalDeviceId = data.ReadBool();
    AppExecFwk::ElementName result = GetElementNameByToken(token, isNeedLocalDeviceId);
    if (result.GetDeviceID().empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "GetElementNameByTokenInner is nullptr");
    }
    reply.WriteParcelable(&result);
    return NO_ERROR;
}

int AbilityManagerStub::MoveAbilityToBackgroundInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = nullptr;
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }
    int32_t result = MoveAbilityToBackground(token);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write result failed");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::MoveUIAbilityToBackgroundInner(MessageParcel &data, MessageParcel &reply)
{
    const sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "token is nullptr.");
        return IPC_STUB_ERR;
    }
    int32_t result = MoveUIAbilityToBackground(token);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write result failed.");
        return IPC_STUB_ERR;
    }
    return NO_ERROR;
}

int AbilityManagerStub::TerminateAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = nullptr;
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }
    int resultCode = data.ReadInt32();
    Want *resultWant = data.ReadParcelable<Want>();
    bool flag = data.ReadBool();
    int32_t result;
    if (flag) {
        result = TerminateAbility(token, resultCode, resultWant);
    } else {
        result = CloseAbility(token, resultCode, resultWant);
    }
    reply.WriteInt32(result);
    if (resultWant != nullptr) {
        delete resultWant;
    }
    return NO_ERROR;
}

int AbilityManagerStub::BackToCallerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = nullptr;
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }
    int resultCode = data.ReadInt32();
    Want *resultWant = data.ReadParcelable<Want>();
    int64_t callerRequestCode = data.ReadInt64();
    int32_t result = BackToCallerAbilityWithResult(token, resultCode, resultWant, callerRequestCode);
    reply.WriteInt32(result);
    if (resultWant != nullptr) {
        delete resultWant;
    }
    return NO_ERROR;
}

int AbilityManagerStub::TerminateUIExtensionAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<SessionInfo> extensionSessionInfo = nullptr;
    if (data.ReadBool()) {
        extensionSessionInfo = data.ReadParcelable<SessionInfo>();
    }
    int resultCode = data.ReadInt32();
    Want *resultWant = data.ReadParcelable<Want>();
    int32_t result = TerminateUIExtensionAbility(extensionSessionInfo, resultCode, resultWant);
    reply.WriteInt32(result);
    if (resultWant != nullptr) {
        delete resultWant;
    }
    return NO_ERROR;
}

int AbilityManagerStub::SendResultToAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    int requestCode = data.ReadInt32();
    int resultCode = data.ReadInt32();
    Want *resultWant = data.ReadParcelable<Want>();
    if (resultWant == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "resultWant is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t result = SendResultToAbility(requestCode, resultCode, *resultWant);
    reply.WriteInt32(result);
    if (resultWant != nullptr) {
        delete resultWant;
    }
    return NO_ERROR;
}

int AbilityManagerStub::MinimizeAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    auto token = data.ReadRemoteObject();
    auto fromUser = data.ReadBool();
    int32_t result = MinimizeAbility(token, fromUser);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::MinimizeUIExtensionAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<SessionInfo> extensionSessionInfo = nullptr;
    if (data.ReadBool()) {
        extensionSessionInfo = data.ReadParcelable<SessionInfo>();
    }
    auto fromUser = data.ReadBool();
    int32_t result = MinimizeUIExtensionAbility(extensionSessionInfo, fromUser);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::MinimizeUIAbilityBySCBInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<SessionInfo> sessionInfo = nullptr;
    if (data.ReadBool()) {
        sessionInfo = data.ReadParcelable<SessionInfo>();
    }
    bool fromUser = data.ReadBool();
    uint32_t sceneFlag = data.ReadUint32();
    int32_t result = MinimizeUIAbilityBySCB(sessionInfo, fromUser, sceneFlag);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::AttachAbilityThreadInner(MessageParcel &data, MessageParcel &reply)
{
    auto scheduler = iface_cast<IAbilityScheduler>(data.ReadRemoteObject());
    if (scheduler == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "scheduler is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto token = data.ReadRemoteObject();
    int32_t result = AttachAbilityThread(scheduler, token);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::AbilityTransitionDoneInner(MessageParcel &data, MessageParcel &reply)
{
    auto token = data.ReadRemoteObject();
    int targetState = data.ReadInt32();
    std::unique_ptr<PacMap> saveData(data.ReadParcelable<PacMap>());
    if (!saveData) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "save data is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t result = AbilityTransitionDone(token, targetState, *saveData);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::AbilityWindowConfigTransitionDoneInner(MessageParcel &data, MessageParcel &reply)
{
    auto token = data.ReadRemoteObject();
    std::unique_ptr<WindowConfig> windowConfig(data.ReadParcelable<WindowConfig>());
    if (!windowConfig) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "windowConfig is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t result = AbilityWindowConfigTransitionDone(token, *windowConfig);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::ScheduleConnectAbilityDoneInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> remoteObject = nullptr;
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }
    if (data.ReadBool()) {
        remoteObject = data.ReadRemoteObject();
    }
    int32_t result = ScheduleConnectAbilityDone(token, remoteObject);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::ScheduleDisconnectAbilityDoneInner(MessageParcel &data, MessageParcel &reply)
{
    auto token = data.ReadRemoteObject();
    int32_t result = ScheduleDisconnectAbilityDone(token);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::ScheduleCommandAbilityDoneInner(MessageParcel &data, MessageParcel &reply)
{
    auto token = data.ReadRemoteObject();
    int32_t result = ScheduleCommandAbilityDone(token);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::ScheduleCommandAbilityWindowDoneInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    sptr<SessionInfo> sessionInfo = data.ReadParcelable<SessionInfo>();
    int32_t winCmd = data.ReadInt32();
    int32_t abilityCmd = data.ReadInt32();
    int32_t result = ScheduleCommandAbilityWindowDone(token, sessionInfo,
        static_cast<WindowCommand>(winCmd), static_cast<AbilityCommand>(abilityCmd));
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::AcquireDataAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Uri> uri = std::make_unique<Uri>(data.ReadString());
    bool tryBind = data.ReadBool();
    sptr<IRemoteObject> callerToken = data.ReadRemoteObject();
    sptr<IAbilityScheduler> result = AcquireDataAbility(*uri, tryBind, callerToken);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "acquire data ability %{public}s", result ? "ok" : "failed");
    if (result) {
        reply.WriteRemoteObject(result->AsObject());
    } else {
        reply.WriteParcelable(nullptr);
    }
    return NO_ERROR;
}

int AbilityManagerStub::ReleaseDataAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    auto scheduler = iface_cast<IAbilityScheduler>(data.ReadRemoteObject());
    if (scheduler == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "scheduler is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto callerToken = data.ReadRemoteObject();
    int32_t result = ReleaseDataAbility(scheduler, callerToken);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "release data ability ret = %d", result);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::KillProcessInner(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = Str16ToStr8(data.ReadString16());
    bool clearPageStack = data.ReadBool();
    int result = KillProcess(bundleName, clearPageStack);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "remove stack error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::UninstallAppInner(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = Str16ToStr8(data.ReadString16());
    int32_t uid = data.ReadInt32();
    int32_t appIndex = data.ReadInt32();
    int32_t result = UninstallApp(bundleName, uid, appIndex);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "remove stack error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::UpgradeAppInner(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = Str16ToStr8(data.ReadString16());
    int32_t uid = data.ReadInt32();
    std::string exitMsg = Str16ToStr8(data.ReadString16());
    int32_t appIndex = data.ReadInt32();
    int32_t result = UpgradeApp(bundleName, uid, exitMsg, appIndex);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "UpgradeAppInner error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbility(*want, userId, requestCode);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityInnerSpecifyTokenId(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr.");
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    int32_t specifyTokenId = data.ReadInt32();
    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbilityWithSpecifyTokenId(*want, callerToken, specifyTokenId, userId, requestCode);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityByUIContentSessionAddCallerInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
        if (callerToken == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "callerToken is nullptr");
            return ERR_INVALID_VALUE;
        }
    }

    sptr<SessionInfo> sessionInfo = nullptr;
    if (data.ReadBool()) {
        sessionInfo = data.ReadParcelable<SessionInfo>();
        if (sessionInfo == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "sessionInfo is nullptr");
            return ERR_INVALID_VALUE;
        }
    }

    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbilityByUIContentSession(*want, callerToken, sessionInfo, userId, requestCode);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityByUIContentSessionForOptionsInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::unique_ptr<StartOptions> startOptions(data.ReadParcelable<StartOptions>());
    if (startOptions == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "startOptions is nullptr");
        return ERR_INVALID_VALUE;
    }
    startOptions->processOptions = nullptr;
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
        if (callerToken == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "callerToken is nullptr");
            return ERR_INVALID_VALUE;
        }
    }
    sptr<SessionInfo> sessionInfo = nullptr;
    if (data.ReadBool()) {
        sessionInfo = data.ReadParcelable<SessionInfo>();
        if (sessionInfo == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "sessionInfo is nullptr");
            return ERR_INVALID_VALUE;
        }
    }
    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbilityByUIContentSession(*want, *startOptions,
        callerToken, sessionInfo, userId, requestCode);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::StartExtensionAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    int32_t userId = data.ReadInt32();
    int32_t extensionType = data.ReadInt32();
    int32_t result = StartExtensionAbility(*want, callerToken, userId,
        static_cast<AppExecFwk::ExtensionAbilityType>(extensionType));
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::RequestModalUIExtensionInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t result = RequestModalUIExtension(*want);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::PreloadUIExtensionAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr.");
        return ERR_INVALID_VALUE;
    }
    std::string hostBundleName = Str16ToStr8(data.ReadString16());
    int32_t userId = data.ReadInt32();
    int32_t result = PreloadUIExtensionAbility(*want, hostBundleName, userId);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::ChangeAbilityVisibilityInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read ability token failed.");
        return ERR_NULL_OBJECT;
    }

    bool isShow = data.ReadBool();
    int result = ChangeAbilityVisibility(token, isShow);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write result failed.");
        return ERR_NATIVE_IPC_PARCEL_FAILED;
    }
    return NO_ERROR;
}

int AbilityManagerStub::ChangeUIAbilityVisibilityBySCBInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<SessionInfo> sessionInfo = data.ReadParcelable<SessionInfo>();
    if (!sessionInfo) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read sessionInfo failed.");
        return ERR_NULL_OBJECT;
    }

    bool isShow = data.ReadBool();
    int result = ChangeUIAbilityVisibilityBySCB(sessionInfo, isShow);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write result failed.");
        return ERR_NATIVE_IPC_PARCEL_FAILED;
    }
    return NO_ERROR;
}

int AbilityManagerStub::StartUIExtensionAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<SessionInfo> extensionSessionInfo = nullptr;
    if (data.ReadBool()) {
        extensionSessionInfo = data.ReadParcelable<SessionInfo>();
        if (extensionSessionInfo == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "read extensionSessionInfo failed.");
            return ERR_NULL_OBJECT;
        }
        // To ensure security, this attribute must be rewritten.
        extensionSessionInfo->uiExtensionUsage = UIExtensionUsage::MODAL;
    }

    int32_t userId = data.ReadInt32();

    int32_t result = StartUIExtensionAbility(extensionSessionInfo, userId);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::StartUIExtensionAbilityEmbeddedInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<SessionInfo> extensionSessionInfo = nullptr;
    if (data.ReadBool()) {
        extensionSessionInfo = data.ReadParcelable<SessionInfo>();
        if (extensionSessionInfo == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "read extensionSessionInfo failed.");
            return ERR_NULL_OBJECT;
        }
        // To ensure security, this attribute must be rewritten.
        extensionSessionInfo->uiExtensionUsage = UIExtensionUsage::EMBEDDED;
    }

    int32_t userId = data.ReadInt32();

    int32_t result = StartUIExtensionAbility(extensionSessionInfo, userId);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::StartUIExtensionConstrainedEmbeddedInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<SessionInfo> extensionSessionInfo = nullptr;
    if (data.ReadBool()) {
        extensionSessionInfo = data.ReadParcelable<SessionInfo>();
        if (extensionSessionInfo == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "read extensionSessionInfo failed.");
            return ERR_NULL_OBJECT;
        }
        // To ensure security, this attribute must be rewritten.
        extensionSessionInfo->uiExtensionUsage = UIExtensionUsage::CONSTRAINED_EMBEDDED;
    }

    int32_t userId = data.ReadInt32();

    int32_t result = StartUIExtensionAbility(extensionSessionInfo, userId);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::StopExtensionAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr.");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    int32_t userId = data.ReadInt32();
    int32_t extensionType = data.ReadInt32();
    int32_t result =
        StopExtensionAbility(*want, callerToken, userId, static_cast<AppExecFwk::ExtensionAbilityType>(extensionType));
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityAddCallerInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr.");
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }

    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbility(*want, callerToken, userId, requestCode);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityAsCallerByTokenInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr!");
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IRemoteObject> asCallerSourceToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    if (data.ReadBool()) {
        asCallerSourceToken =  data.ReadRemoteObject();
    }
    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbilityAsCaller(*want, callerToken, asCallerSourceToken, userId, requestCode);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityAsCallerForOptionInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    StartOptions *startOptions = data.ReadParcelable<StartOptions>();
    if (startOptions == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "startOptions is nullptr");
        return ERR_INVALID_VALUE;
    }
    startOptions->processOptions = nullptr;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IRemoteObject> asCallerSourceToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    if (data.ReadBool()) {
        asCallerSourceToken =  data.ReadRemoteObject();
    }
    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbilityAsCaller(*want, *startOptions, callerToken, asCallerSourceToken, userId, requestCode);
    reply.WriteInt32(result);
    delete startOptions;
    return NO_ERROR;
}

int AbilityManagerStub::ConnectAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr.");
        return ERR_INVALID_VALUE;
    }
    sptr<IAbilityConnection> callback = nullptr;
    sptr<IRemoteObject> token = nullptr;
    if (data.ReadBool()) {
        callback = iface_cast<IAbilityConnection>(data.ReadRemoteObject());
    }
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }
    int32_t userId = data.ReadInt32();
    int32_t result = ConnectAbilityCommon(*want, callback, token, AppExecFwk::ExtensionAbilityType::SERVICE, userId);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::ConnectAbilityWithTypeInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s, want is nullptr!", __func__);
        return ERR_INVALID_VALUE;
    }
    sptr<IAbilityConnection> callback = nullptr;
    sptr<IRemoteObject> token = nullptr;
    if (data.ReadBool()) {
        callback = iface_cast<IAbilityConnection>(data.ReadRemoteObject());
    }
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }
    int32_t userId = data.ReadInt32();
    AppExecFwk::ExtensionAbilityType extensionType = static_cast<AppExecFwk::ExtensionAbilityType>(data.ReadInt32());
    bool isQueryExtensionOnly = data.ReadBool();
    int32_t result = ConnectAbilityCommon(*want, callback, token, extensionType, userId, isQueryExtensionOnly);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::ConnectUIExtensionAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s, want is nullptr", __func__);
        return ERR_INVALID_VALUE;
    }
    sptr<IAbilityConnection> callback = nullptr;
    if (data.ReadBool()) {
        callback = iface_cast<IAbilityConnection>(data.ReadRemoteObject());
    }
    sptr<SessionInfo> sessionInfo = nullptr;
    if (data.ReadBool()) {
        sessionInfo = data.ReadParcelable<SessionInfo>();
    }
    int32_t userId = data.ReadInt32();

    sptr<UIExtensionAbilityConnectInfo> connectInfo = nullptr;
    if (data.ReadBool()) {
        connectInfo = data.ReadParcelable<UIExtensionAbilityConnectInfo>();
    }

    int32_t result = ConnectUIExtensionAbility(*want, callback, sessionInfo, userId, connectInfo);
    if (connectInfo != nullptr && !reply.WriteParcelable(connectInfo)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "connectInfo write failed.");
    }

    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::DisconnectAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IAbilityConnection> callback = iface_cast<IAbilityConnection>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callback is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t result = DisconnectAbility(callback);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "disconnect ability ret = %d", result);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::StopServiceAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t userId = data.ReadInt32();
    sptr<IRemoteObject> token = nullptr;
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }
    int32_t result = StopServiceAbility(*want, userId, token);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::DumpSysStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> result;
    std::string args = Str16ToStr8(data.ReadString16());
    std::vector<std::string> argList;

    auto isClient = data.ReadBool();
    auto isUserID = data.ReadBool();
    auto UserID = data.ReadInt32();
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return ERR_INVALID_VALUE;
    }
    DumpSysState(args, result, isClient, isUserID, UserID);
    reply.WriteInt32(result.size());
    for (auto stack : result) {
        reply.WriteString16(Str8ToStr16(stack));
    }
    return NO_ERROR;
}

int AbilityManagerStub::DumpStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> result;
    std::string args = Str16ToStr8(data.ReadString16());
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return ERR_INVALID_VALUE;
    }
    DumpState(args, result);
    reply.WriteInt32(result.size());
    for (auto stack : result) {
        reply.WriteString16(Str8ToStr16(stack));
    }
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityForSettingsInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    AbilityStartSetting *abilityStartSetting = data.ReadParcelable<AbilityStartSetting>();
    if (abilityStartSetting == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityStartSetting is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbility(*want, *abilityStartSetting, callerToken, userId, requestCode);
    reply.WriteInt32(result);
    delete abilityStartSetting;
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityForOptionsInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr.");
        return ERR_INVALID_VALUE;
    }
    StartOptions *startOptions = data.ReadParcelable<StartOptions>();
    if (startOptions == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "startOptions is nullptr.");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbility(*want, *startOptions, callerToken, userId, requestCode);
    reply.WriteInt32(result);
    delete startOptions;
    return NO_ERROR;
}

int AbilityManagerStub::CloseUIAbilityBySCBInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<SessionInfo> sessionInfo = nullptr;
    if (data.ReadBool()) {
        sessionInfo = data.ReadParcelable<SessionInfo>();
    }
    int32_t result = CloseUIAbilityBySCB(sessionInfo);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::GetWantSenderInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<WantSenderInfo> wantSenderInfo(data.ReadParcelable<WantSenderInfo>());
    if (wantSenderInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "wantSenderInfo is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    sptr<IWantSender> wantSender = GetWantSender(*wantSenderInfo, callerToken);
    if (!reply.WriteRemoteObject(((wantSender == nullptr) ? nullptr : wantSender->AsObject()))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to reply wantSender instance to client, for write parcel error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::SendWantSenderInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::unique_ptr<SenderInfo> senderInfo(data.ReadParcelable<SenderInfo>());
    if (senderInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "senderInfo is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t result = SendWantSender(wantSender, *senderInfo);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::CancelWantSenderInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }
    CancelWantSender(wantSender);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingWantUidInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    int32_t uid = GetPendingWantUid(wantSender);
    reply.WriteInt32(uid);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingWantUserIdInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    int32_t userId = GetPendingWantUserId(wantSender);
    reply.WriteInt32(userId);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingWantBundleNameInner(MessageParcel &data, MessageParcel &reply)
{
    auto remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ReadRemoteObject is nullptr");
        return ERR_INVALID_VALUE;
    }

    sptr<IWantSender> wantSender = iface_cast<IWantSender>(remote);
    if (wantSender == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::string bundleName = GetPendingWantBundleName(wantSender);
    reply.WriteString16(Str8ToStr16(bundleName));
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingWantCodeInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    int32_t code = GetPendingWantCode(wantSender);
    reply.WriteInt32(code);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingWantTypeInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "wantSender is nullptr.");
        return ERR_INVALID_VALUE;
    }

    int32_t type = GetPendingWantType(wantSender);
    reply.WriteInt32(type);
    return NO_ERROR;
}

int AbilityManagerStub::RegisterCancelListenerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> sender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (sender == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sender is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IWantReceiver> receiver = iface_cast<IWantReceiver>(data.ReadRemoteObject());
    if (receiver == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "receiver is nullptr");
        return ERR_INVALID_VALUE;
    }
    RegisterCancelListener(sender, receiver);
    return NO_ERROR;
}

int AbilityManagerStub::UnregisterCancelListenerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> sender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (sender == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sender is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IWantReceiver> receiver = iface_cast<IWantReceiver>(data.ReadRemoteObject());
    if (receiver == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "receiver is nullptr");
        return ERR_INVALID_VALUE;
    }
    UnregisterCancelListener(sender, receiver);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingRequestWantInner(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    int32_t result = GetPendingRequestWant(wantSender, want);
    if (result != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetPendingRequestWant is failed");
        return ERR_INVALID_VALUE;
    }
    reply.WriteParcelable(want.get());
    return NO_ERROR;
}

int AbilityManagerStub::GetWantSenderInfoInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<WantSenderInfo> info(data.ReadParcelable<WantSenderInfo>());
    int32_t result = GetWantSenderInfo(wantSender, info);
    if (result != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetWantSenderInfo is failed");
        return ERR_INVALID_VALUE;
    }
    reply.WriteParcelable(info.get());
    return NO_ERROR;
}

int AbilityManagerStub::GetAppMemorySizeInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = GetAppMemorySize();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "GetAppMemorySizeInner result %{public}d", result);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetAppMemorySize error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::IsRamConstrainedDeviceInner(MessageParcel &data, MessageParcel &reply)
{
    auto result = IsRamConstrainedDevice();
    if (!reply.WriteBool(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::ContinueMissionInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "called");
    std::string srcDeviceId = data.ReadString();
    std::string dstDeviceId = data.ReadString();
    int32_t missionId = data.ReadInt32();
    sptr<IRemoteObject> callback = data.ReadRemoteObject();
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ContinueMissionInner callback readParcelable failed.");
        return ERR_NULL_OBJECT;
    }
    std::unique_ptr<WantParams> wantParams(data.ReadParcelable<WantParams>());
    if (wantParams == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ContinueMissionInner wantParams readParcelable failed.");
        return ERR_NULL_OBJECT;
    }
    int32_t result = ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, *wantParams);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ContinueMissionInner result = %{public}d.", result);
    return result;
}

int AbilityManagerStub::ContinueMissionOfBundleNameInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "amsStub %{public}s called!", __func__);
    ContinueMissionInfo continueMissionInfo;
    continueMissionInfo.srcDeviceId = data.ReadString();
    continueMissionInfo.dstDeviceId = data.ReadString();
    continueMissionInfo.bundleName = data.ReadString();
    sptr<IRemoteObject> callback = data.ReadRemoteObject();
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ContinueMissionInner callback readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    std::unique_ptr<WantParams> wantParams(data.ReadParcelable<WantParams>());
    if (wantParams == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ContinueMissionInner wantParams readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    continueMissionInfo.wantParams = *wantParams;
    continueMissionInfo.srcBundleName = data.ReadString();
    continueMissionInfo.continueType = data.ReadString();
    int32_t result = ContinueMission(continueMissionInfo, callback);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ContinueMissionInner result = %{public}d", result);
    return result;
}

int AbilityManagerStub::ContinueAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    int32_t missionId = data.ReadInt32();
    uint32_t versionCode = data.ReadUint32();
    AAFWK::ContinueRadar::GetInstance().SaveDataContinue("ContinueAbility");
    int32_t result = ContinueAbility(deviceId, missionId, versionCode);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ContinueAbilityInner result = %{public}d", result);
    return result;
}

int AbilityManagerStub::StartContinuationInner(MessageParcel &data, MessageParcel &reply)
{
    AAFWK::ContinueRadar::GetInstance().SaveDataRes("GetContentInfo");
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "StartContinuationInner want readParcelable failed!");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> abilityToken = data.ReadRemoteObject();
    if (abilityToken == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get abilityToken failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t status = data.ReadInt32();
    int32_t result = StartContinuation(*want, abilityToken, status);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "StartContinuationInner result = %{public}d", result);

    return result;
}

int AbilityManagerStub::NotifyCompleteContinuationInner(MessageParcel &data, MessageParcel &reply)
{
    std::string devId = data.ReadString();
    int32_t sessionId = data.ReadInt32();
    bool isSuccess = data.ReadBool();

    NotifyCompleteContinuation(devId, sessionId, isSuccess);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "NotifyCompleteContinuationInner end");
    return NO_ERROR;
}

int AbilityManagerStub::NotifyContinuationResultInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t missionId = data.ReadInt32();
    int32_t continuationResult = data.ReadInt32();

    int32_t result = NotifyContinuationResult(missionId, continuationResult);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "StartContinuationInner result = %{public}d", result);
    return result;
}

int AbilityManagerStub::LockMissionForCleanupInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t id = data.ReadInt32();
    int result = LockMissionForCleanup(id);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AbilityManagerStub: lock mission failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::UnlockMissionForCleanupInner(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int32_t id = data.ReadInt32();
    int result = UnlockMissionForCleanup(id);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AbilityManagerStub: unlock mission failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::SetLockedStateInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t sessionId = data.ReadInt32();
    bool flag = data.ReadBool();
    SetLockedState(sessionId, flag);
    return NO_ERROR;
}

int AbilityManagerStub::RegisterMissionListenerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IMissionListener> listener = iface_cast<IMissionListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "stub register mission listener, listener is nullptr.");
        return ERR_INVALID_VALUE;
    }

    int32_t result = RegisterMissionListener(listener);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::UnRegisterMissionListenerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IMissionListener> listener = iface_cast<IMissionListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "stub unregister mission listener, listener is nullptr.");
        return ERR_INVALID_VALUE;
    }

    int32_t result = UnRegisterMissionListener(listener);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::GetMissionInfosInner(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string deviceId = Str16ToStr8(data.ReadString16());
    int numMax = data.ReadInt32();
    std::vector<MissionInfo> missionInfos;
    int32_t result = GetMissionInfos(deviceId, numMax, missionInfos);
    reply.WriteInt32(missionInfos.size());
    for (auto &it : missionInfos) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return result;
}

int AbilityManagerStub::GetMissionInfoInner(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    MissionInfo info;
    std::string deviceId = Str16ToStr8(data.ReadString16());
    int32_t missionId = data.ReadInt32();
    int result = GetMissionInfo(deviceId, missionId, info);
    if (!reply.WriteParcelable(&info)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetMissionInfo error");
        return ERR_INVALID_VALUE;
    }

    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetMissionInfo result error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::CleanMissionInner(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int32_t missionId = data.ReadInt32();
    int result = CleanMission(missionId);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CleanMission failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::CleanAllMissionsInner(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int result = CleanAllMissions();
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CleanAllMissions failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::MoveMissionToFrontInner(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int32_t missionId = data.ReadInt32();
    int result = MoveMissionToFront(missionId);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "MoveMissionToFront failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::GetMissionIdByTokenInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    int32_t missionId = GetMissionIdByToken(token);
    if (!reply.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetMissionIdByToken write missionId failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::MoveMissionToFrontByOptionsInner(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int32_t missionId = data.ReadInt32();
    std::unique_ptr<StartOptions> startOptions(data.ReadParcelable<StartOptions>());
    if (startOptions == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "startOptions is nullptr");
        return ERR_INVALID_VALUE;
    }
    startOptions->processOptions = nullptr;
    int result = MoveMissionToFront(missionId, *startOptions);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "MoveMissionToFront failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::MoveMissionsToForegroundInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    std::vector<int32_t> missionIds;
    data.ReadInt32Vector(&missionIds);
    int32_t topMissionId = data.ReadInt32();
    int32_t errCode = MoveMissionsToForeground(missionIds, topMissionId);
    if (!reply.WriteInt32(errCode)) {
        return ERR_INVALID_VALUE;
    }
    return errCode;
}

int AbilityManagerStub::MoveMissionsToBackgroundInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    std::vector<int32_t> missionIds;
    std::vector<int32_t> result;

    data.ReadInt32Vector(&missionIds);
    int32_t errCode = MoveMissionsToBackground(missionIds, result);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s is called. resultSize: %{public}zu", __func__, result.size());
    if (!reply.WriteInt32Vector(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s is called. WriteInt32Vector Failed", __func__);
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(errCode)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityByCallInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityManagerStub::StartAbilityByCallInner begin.");
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }

    auto callback = iface_cast<IAbilityConnection>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callback is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }

    int32_t accountId = data.ReadInt32();
    int32_t result = StartAbilityByCall(*want, callback, callerToken, accountId);

    TAG_LOGD(AAFwkTag::ABILITYMGR, "resolve call ability ret = %d", result);

    reply.WriteInt32(result);

    TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityManagerStub::StartAbilityByCallInner end.");

    return NO_ERROR;
}

int AbilityManagerStub::StartUIAbilityBySCBInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<SessionInfo> sessionInfo = nullptr;
    if (data.ReadBool()) {
        sessionInfo = data.ReadParcelable<SessionInfo>();
    }
    uint32_t sceneFlag = data.ReadUint32();
    bool isColdStart = false;
    int32_t result = StartUIAbilityBySCB(sessionInfo, isColdStart, sceneFlag);
    reply.WriteBool(isColdStart);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::CallRequestDoneInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    sptr<IRemoteObject> callStub = data.ReadRemoteObject();
    CallRequestDone(token, callStub);
    return NO_ERROR;
}

int AbilityManagerStub::ReleaseCallInner(MessageParcel &data, MessageParcel &reply)
{
    auto callback = iface_cast<IAbilityConnection>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callback is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::unique_ptr<AppExecFwk::ElementName> element(data.ReadParcelable<AppExecFwk::ElementName>());
    if (element == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callback stub receive element is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t result = ReleaseCall(callback, *element);

    TAG_LOGD(AAFwkTag::ABILITYMGR, "release call ability ret = %d", result);

    reply.WriteInt32(result);

    return NO_ERROR;
}

int AbilityManagerStub::StartUserInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();
    sptr<IUserCallback> callback = nullptr;
    if (data.ReadBool()) {
        callback = iface_cast<IUserCallback>(data.ReadRemoteObject());
    } else {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callback is invalid value.");
        return ERR_INVALID_VALUE;
    }
    int result = StartUser(userId, callback);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "StartUser failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::StopUserInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();
    sptr<IUserCallback> callback = nullptr;
    if (data.ReadBool()) {
        callback = iface_cast<IUserCallback>(data.ReadRemoteObject());
    }
    int result = StopUser(userId, callback);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "StopUser failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::LogoutUserInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();
    int result = LogoutUser(userId);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "LogoutUser failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::GetAbilityRunningInfosInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<AbilityRunningInfo> abilityRunningInfos;
    auto result = GetAbilityRunningInfos(abilityRunningInfos);
    reply.WriteInt32(abilityRunningInfos.size());
    for (auto &it : abilityRunningInfos) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return result;
}

int AbilityManagerStub::GetExtensionRunningInfosInner(MessageParcel &data, MessageParcel &reply)
{
    auto upperLimit = data.ReadInt32();
    std::vector<ExtensionRunningInfo> infos;
    auto result = GetExtensionRunningInfos(upperLimit, infos);
    reply.WriteInt32(infos.size());
    for (auto &it : infos) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return result;
}

int AbilityManagerStub::GetProcessRunningInfosInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<AppExecFwk::RunningProcessInfo> infos;
    auto result = GetProcessRunningInfos(infos);
    reply.WriteInt32(infos.size());
    for (auto &it : infos) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::StartSyncRemoteMissionsInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    bool fixConflict = data.ReadBool();
    int64_t tag = data.ReadInt64();
    int result = StartSyncRemoteMissions(deviceId, fixConflict, tag);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "StartSyncRemoteMissionsInner failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::StopSyncRemoteMissionsInner(MessageParcel &data, MessageParcel &reply)
{
    int result = StopSyncRemoteMissions(data.ReadString());
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "StopSyncRemoteMissionsInner failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::RegisterRemoteMissionListenerInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    if (deviceId.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AbilityManagerStub: RegisterRemoteMissionListenerInner deviceId empty!");
        return INVALID_PARAMETERS_ERR;
    }
    sptr<IRemoteMissionListener> listener = iface_cast<IRemoteMissionListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AbilityManagerStub: RegisterRemoteMissionListenerInner listener"
            "readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = RegisterMissionListener(deviceId, listener);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "AbilityManagerStub: RegisterRemoteMissionListenerInner result = %{public}d",
        result);
    return result;
}

int AbilityManagerStub::RegisterRemoteOnListenerInner(MessageParcel &data, MessageParcel &reply)
{
    std::string type = data.ReadString();
    if (type.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AbilityManagerStub: RegisterRemoteOnListenerInner type empty!");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteOnListener> listener = iface_cast<IRemoteOnListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AbilityManagerStub: RegisterRemoteOnListenerInner listener"
            "readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = RegisterOnListener(type, listener);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "AbilityManagerStub: RegisterRemoteOnListenerInner result = %{public}d", result);
    return result;
}

int AbilityManagerStub::RegisterRemoteOffListenerInner(MessageParcel &data, MessageParcel &reply)
{
    std::string type = data.ReadString();
    if (type.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AbilityManagerStub: RegisterRemoteOffListenerInner type empty!");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteOnListener> listener = iface_cast<IRemoteOnListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AbilityManagerStub: RegisterRemoteOffListenerInner listener"
            "readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = RegisterOffListener(type, listener);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "AbilityManagerStub: RegisterRemoteOffListenerInner result = %{public}d", result);
    return result;
}

int AbilityManagerStub::UnRegisterRemoteMissionListenerInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    if (deviceId.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AbilityManagerStub: UnRegisterRemoteMissionListenerInner deviceId empty!");
        return INVALID_PARAMETERS_ERR;
    }
    sptr<IRemoteMissionListener> listener = iface_cast<IRemoteMissionListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AbilityManagerStub: UnRegisterRemoteMissionListenerInner listener"
            "readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = UnRegisterMissionListener(deviceId, listener);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "AbilityManagerStub: UnRegisterRemoteMissionListenerInner result = %{public}d",
        result);
    return result;
}

int AbilityManagerStub::RegisterSnapshotHandlerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<ISnapshotHandler> handler = iface_cast<ISnapshotHandler>(data.ReadRemoteObject());
    if (handler == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "snapshot: AbilityManagerStub read snapshot handler failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = RegisterSnapshotHandler(handler);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "snapshot: AbilityManagerStub register snapshot handler result = %{public}d",
        result);
    return result;
}

int AbilityManagerStub::GetMissionSnapshotInfoInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    int32_t missionId = data.ReadInt32();
    bool isLowResolution = data.ReadBool();
    MissionSnapshot missionSnapshot;
    int32_t result = GetMissionSnapshot(deviceId, missionId, missionSnapshot, isLowResolution);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "snapshot: AbilityManagerStub get snapshot result = %{public}d", result);
    if (!reply.WriteParcelable(&missionSnapshot)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetMissionSnapshot error");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetMissionSnapshot result error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::SetAbilityControllerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<AppExecFwk::IAbilityController> controller =
        iface_cast<AppExecFwk::IAbilityController>(data.ReadRemoteObject());
    if (controller == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AbilityManagerStub: setAbilityControllerInner controller"
            "readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    bool imAStabilityTest = data.ReadBool();
    int32_t result = SetAbilityController(controller, imAStabilityTest);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "AbilityManagerStub: setAbilityControllerInner result = %{public}d", result);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "setAbilityControllerInner failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::IsRunningInStabilityTestInner(MessageParcel &data, MessageParcel &reply)
{
    bool result = IsRunningInStabilityTest();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "AbilityManagerStub: IsRunningInStabilityTest result = %{public}d", result);
    if (!reply.WriteBool(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "IsRunningInStabilityTest failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::StartUserTestInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto observer = data.ReadRemoteObject();
    int32_t result = StartUserTest(*want, observer);
    reply.WriteInt32(result);
    return result;
}

int AbilityManagerStub::FinishUserTestInner(MessageParcel &data, MessageParcel &reply)
{
    std::string msg = data.ReadString();
    int64_t resultCode = data.ReadInt64();
    std::string bundleName = data.ReadString();
    int32_t result = FinishUserTest(msg, resultCode, bundleName);
    reply.WriteInt32(result);
    return result;
}

int AbilityManagerStub::GetTopAbilityTokenInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token;
    auto result = GetTopAbility(token);
    if (!reply.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "data write failed.");
        return ERR_INVALID_VALUE;
    }
    reply.WriteInt32(result);

    return NO_ERROR;
}

int AbilityManagerStub::CheckUIExtensionIsFocusedInner(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uiExtensionTokenId = data.ReadUint32();
    bool isFocused = false;
    auto result = CheckUIExtensionIsFocused(uiExtensionTokenId, isFocused);
    if (result == ERR_OK) {
        if (!reply.WriteBool(isFocused)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "reply write failed.");
            return ERR_INVALID_VALUE;
        }
    }
    return result;
}

int AbilityManagerStub::DelegatorDoAbilityForegroundInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    auto result = DelegatorDoAbilityForeground(token);
    reply.WriteInt32(result);

    return NO_ERROR;
}

int AbilityManagerStub::DelegatorDoAbilityBackgroundInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    auto result = DelegatorDoAbilityBackground(token);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag)
{
    return 0;
}

int AbilityManagerStub::DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag)
{
    return 0;
}

int AbilityManagerStub::DoAbilityForegroundInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    uint32_t flag = data.ReadUint32();
    auto result = DoAbilityForeground(token, flag);
    reply.WriteInt32(result);

    return NO_ERROR;
}

int AbilityManagerStub::DoAbilityBackgroundInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    uint32_t flag = data.ReadUint32();
    auto result = DoAbilityBackground(token, flag);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::RegisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer)
{
    // should implement in child.
    return NO_ERROR;
}

int AbilityManagerStub::UnregisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer)
{
    // should implement in child
    return NO_ERROR;
}

#ifdef WITH_DLP
int AbilityManagerStub::GetDlpConnectionInfos(std::vector<AbilityRuntime::DlpConnectionInfo> &infos)
{
    // should implement in child
    return NO_ERROR;
}
#endif // WITH_DLP

int AbilityManagerStub::GetConnectionData(std::vector<AbilityRuntime::ConnectionData> &infos)
{
    // should implement in child
    return NO_ERROR;
}

#ifdef ABILITY_COMMAND_FOR_TEST
int AbilityManagerStub::ForceTimeoutForTestInner(MessageParcel &data, MessageParcel &reply)
{
    std::string abilityName = Str16ToStr8(data.ReadString16());
    std::string state = Str16ToStr8(data.ReadString16());
    int result = ForceTimeoutForTest(abilityName, state);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "force ability timeout error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::BlockAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t abilityRecordId = data.ReadInt32();
    int32_t result = BlockAbility(abilityRecordId);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::BlockAmsServiceInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = BlockAmsService();
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::BlockAppServiceInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = BlockAppService();
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}
#endif

int AbilityManagerStub::FreeInstallAbilityFromRemoteInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    want->SetParam(FROM_REMOTE_KEY, true);

    auto callback = data.ReadRemoteObject();
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callback is nullptr");
        return ERR_INVALID_VALUE;
    }

    int32_t userId = data.ReadInt32();
    int32_t requestCode = data.ReadInt32();
    int32_t result = FreeInstallAbilityFromRemote(*want, callback, userId, requestCode);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::AddFreeInstallObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
        if (callerToken == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "caller token is nullptr.");
            return ERR_INVALID_VALUE;
        }
    }
    sptr<AbilityRuntime::IFreeInstallObserver> observer =
        iface_cast<AbilityRuntime::IFreeInstallObserver>(data.ReadRemoteObject());
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "observer is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t result = AddFreeInstallObserver(callerToken, observer);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::DumpAbilityInfoDoneInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> infos;
    data.ReadStringVector(&infos);
    sptr<IRemoteObject> callerToken = data.ReadRemoteObject();
    int32_t result = DumpAbilityInfoDone(infos, callerToken);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::UpdateMissionSnapShotFromWMSInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read ability token failed.");
        return ERR_NULL_OBJECT;
    }
#ifdef SUPPORT_SCREEN
    std::shared_ptr<Media::PixelMap> pixelMap(data.ReadParcelable<Media::PixelMap>());
    if (pixelMap == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read pixelMap failed.");
        return ERR_NULL_OBJECT;
    }
    UpdateMissionSnapShot(token, pixelMap);
#endif // SUPPORT_SCREEN
    return NO_ERROR;
}

int AbilityManagerStub::EnableRecoverAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "EnableRecoverAbilityInner read ability token failed.");
        return ERR_NULL_OBJECT;
    }
    EnableRecoverAbility(token);
    return NO_ERROR;
}

int AbilityManagerStub::ScheduleClearRecoveryPageStackInner(MessageParcel &data, MessageParcel &reply)
{
    ScheduleClearRecoveryPageStack();
    return NO_ERROR;
}

int AbilityManagerStub::SubmitSaveRecoveryInfoInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SubmitSaveRecoveryInfoInner read ability token failed.");
        return ERR_NULL_OBJECT;
    }
    SubmitSaveRecoveryInfo(token);
    return NO_ERROR;
}

int AbilityManagerStub::HandleRequestDialogService(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> callerToken = data.ReadRemoteObject();
    if (!callerToken) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callerToken is invalid.");
        return ERR_INVALID_VALUE;
    }

    int32_t result = RequestDialogService(*want, callerToken);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::HandleReportDrawnCompleted(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    sptr<IRemoteObject> callerToken = data.ReadRemoteObject();
    if (callerToken == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callerToken is invalid.");
        return ERR_INVALID_VALUE;
    }

    auto result = ReportDrawnCompleted(callerToken);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::AcquireShareDataInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t missionId = data.ReadInt32();
    sptr<IAcquireShareDataCallback> shareData = iface_cast<IAcquireShareDataCallback>(data.ReadRemoteObject());
    if (!shareData) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "shareData read failed.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = AcquireShareData(missionId, shareData);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::ShareDataDoneInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ShareDataDone read ability token failed.");
        return ERR_NULL_OBJECT;
    }
    int32_t resultCode = data.ReadInt32();
    int32_t uniqueId = data.ReadInt32();
    std::shared_ptr<WantParams> wantParam(data.ReadParcelable<WantParams>());
    if (!wantParam) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "wantParam read failed.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = ShareDataDone(token, resultCode, uniqueId, *wantParam);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::GetAbilityTokenByCalleeObjInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> callStub = data.ReadRemoteObject();
    if (!callStub) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetAbilityToken read call stub failed.");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> result;
    GetAbilityTokenByCalleeObj(callStub, result);
    reply.WriteRemoteObject(result);
    return NO_ERROR;
}

int AbilityManagerStub::ScheduleRecoverAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ScheduleRecoverAbility read ability token failed.");
        return ERR_NULL_OBJECT;
    }

    int reason = data.ReadInt32();
    Want *want = data.ReadParcelable<Want>();
    ScheduleRecoverAbility(token, reason, want);
    if (want != nullptr) {
        delete want;
    }
    return NO_ERROR;
}

int AbilityManagerStub::RegisterConnectionObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<AbilityRuntime::IConnectionObserver> observer = iface_cast<AbilityRuntime::IConnectionObserver>(
        data.ReadRemoteObject());
    if (!observer) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "RegisterConnectionObserverInner read observer failed.");
        return ERR_NULL_OBJECT;
    }

    return RegisterObserver(observer);
}

int AbilityManagerStub::UnregisterConnectionObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<AbilityRuntime::IConnectionObserver> observer = iface_cast<AbilityRuntime::IConnectionObserver>(
        data.ReadRemoteObject());
    if (!observer) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "UnregisterConnectionObserverInner read observer failed.");
        return ERR_NULL_OBJECT;
    }

    return UnregisterObserver(observer);
}

#ifdef WITH_DLP
int AbilityManagerStub::GetDlpConnectionInfosInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<AbilityRuntime::DlpConnectionInfo> infos;
    auto result = GetDlpConnectionInfos(infos);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write result failed");
        return ERR_INVALID_VALUE;
    }

    if (!reply.WriteInt32(infos.size())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write infos size failed");
        return ERR_INVALID_VALUE;
    }

    for (auto &item : infos) {
        if (!reply.WriteParcelable(&item)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "write info item failed");
            return ERR_INVALID_VALUE;
        }
    }

    return ERR_OK;
}
#endif // WITH_DLP

int AbilityManagerStub::GetConnectionDataInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<AbilityRuntime::ConnectionData> connectionData;
    auto result = GetConnectionData(connectionData);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write result failed");
        return ERR_INVALID_VALUE;
    }

    if (!reply.WriteInt32(connectionData.size())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write infos size failed");
        return ERR_INVALID_VALUE;
    }

    for (auto &item : connectionData) {
        if (!reply.WriteParcelable(&item)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "write info item failed");
            return ERR_INVALID_VALUE;
        }
    }

    return ERR_OK;
}

int AbilityManagerStub::SetMissionContinueStateInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionContinueStateInner read ability token failed.");
        return ERR_NULL_OBJECT;
    }

    int32_t state = data.ReadInt32();
    int result = SetMissionContinueState(token, static_cast<AAFwk::ContinueState>(state));
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionContinueState failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

#ifdef SUPPORT_SCREEN
int AbilityManagerStub::SetMissionLabelInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionLabelInner read ability token failed.");
        return ERR_NULL_OBJECT;
    }

    std::string label = Str16ToStr8(data.ReadString16());
    int result = SetMissionLabel(token, label);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionLabel failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::SetMissionIconInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionIconInner read ability token failed.");
        return ERR_NULL_OBJECT;
    }

    std::shared_ptr<Media::PixelMap> icon(data.ReadParcelable<Media::PixelMap>());
    if (!icon) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionIconInner read icon failed.");
        return ERR_NULL_OBJECT;
    }

    int result = SetMissionIcon(token, icon);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionIcon failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::RegisterWindowManagerServiceHandlerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWindowManagerServiceHandler> handler = iface_cast<IWindowManagerServiceHandler>(data.ReadRemoteObject());
    if (handler == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s read WMS handler failed!", __func__);
        return ERR_NULL_OBJECT;
    }
    bool animationEnabled = data.ReadBool();
    return RegisterWindowManagerServiceHandler(handler, animationEnabled);
}

int AbilityManagerStub::CompleteFirstFrameDrawingInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    sptr<IRemoteObject> abilityToken = data.ReadRemoteObject();
    if (abilityToken == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s read abilityToken failed!", __func__);
        return ERR_NULL_OBJECT;
    }
    CompleteFirstFrameDrawing(abilityToken);
    return NO_ERROR;
}

int AbilityManagerStub::CompleteFirstFrameDrawingBySCBInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    int32_t sessionId = data.ReadInt32();
    CompleteFirstFrameDrawing(sessionId);
    return NO_ERROR;
}

int AbilityManagerStub::PrepareTerminateAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    sptr<IRemoteObject> token = nullptr;
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }
    sptr<IPrepareTerminateCallback> callback = iface_cast<IPrepareTerminateCallback>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callback is nullptr");
        return ERR_NULL_OBJECT;
    }
    int result = PrepareTerminateAbility(token, callback);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "end faild. err: %{public}d", result);
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::GetDialogSessionInfoInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    std::string dialogSessionId = data.ReadString();
    sptr<DialogSessionInfo> info;
    int result = GetDialogSessionInfo(dialogSessionId, info);
    if (result != ERR_OK || info == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not find dialogSessionInfo");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteParcelable(info)) {
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::SendDialogResultInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::string dialogSessionId = data.ReadString();
    bool isAllow = data.ReadBool();
    int result = SendDialogResult(*want, dialogSessionId, isAllow);
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::RegisterAbilityFirstFrameStateObserverInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto callback = iface_cast<AppExecFwk::IAbilityFirstFrameStateObserver>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Callback is null.");
        return ERR_INVALID_VALUE;
    }

    std::string targetBundleName = data.ReadString();
    auto ret = RegisterAbilityFirstFrameStateObserver(callback, targetBundleName);
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write ret error.");
        return IPC_STUB_ERR;
    }
    return NO_ERROR;
}

int AbilityManagerStub::UnregisterAbilityFirstFrameStateObserverInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto callback = iface_cast<AppExecFwk::IAbilityFirstFrameStateObserver>(data.ReadRemoteObject());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Callback is null.");
        return ERR_INVALID_VALUE;
    }
    auto ret = UnregisterAbilityFirstFrameStateObserver(callback);
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write ret error.");
        return IPC_STUB_ERR;
    }
    return NO_ERROR;
}
#endif

int32_t AbilityManagerStub::IsValidMissionIdsInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    std::vector<int32_t> missionIds;
    std::vector<MissionValidResult> results;

    data.ReadInt32Vector(&missionIds);
    auto err = IsValidMissionIds(missionIds, results);
    if (err != ERR_OK) {
        results.clear();
    }

    if (!reply.WriteInt32(err)) {
        return ERR_INVALID_VALUE;
    }

    reply.WriteInt32(static_cast<int32_t>(results.size()));
    for (auto &item : results) {
        if (!reply.WriteParcelable(&item)) {
            return ERR_INVALID_VALUE;
        }
    }
    return NO_ERROR;
}

int AbilityManagerStub::VerifyPermissionInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "VerifyPermission call.");
    std::string permission = data.ReadString();
    int32_t pid = data.ReadInt32();
    int32_t uid = data.ReadInt32();

    auto result = VerifyPermission(permission, pid, uid);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "VerifyPermission failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::ForceExitAppInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t pid = data.ReadInt32();
    std::unique_ptr<ExitReason> exitReason(data.ReadParcelable<ExitReason>());
    if (!exitReason) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "exitReason is nullptr.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = ForceExitApp(pid, *exitReason);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write result failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::RecordAppExitReasonInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<ExitReason> exitReason(data.ReadParcelable<ExitReason>());
    if (!exitReason) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "exitReason is nullptr.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = RecordAppExitReason(*exitReason);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write result failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::RecordProcessExitReasonInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t pid = data.ReadInt32();
    std::unique_ptr<ExitReason> exitReason(data.ReadParcelable<ExitReason>());
    if (!exitReason) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "exitReason is nullptr.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = RecordProcessExitReason(pid, *exitReason);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write result failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::SetRootSceneSessionInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call.");
    auto rootSceneSession = data.ReadRemoteObject();
    if (rootSceneSession == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read rootSceneSession failed.");
        return ERR_INVALID_VALUE;
    }
    SetRootSceneSession(rootSceneSession);
    return NO_ERROR;
}

int AbilityManagerStub::CallUIAbilityBySCBInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call.");
    sptr<SessionInfo> sessionInfo = nullptr;
    if (data.ReadBool()) {
        sessionInfo = data.ReadParcelable<SessionInfo>();
    }
    bool isColdStart = false;
    CallUIAbilityBySCB(sessionInfo, isColdStart);
    reply.WriteBool(isColdStart);
    return NO_ERROR;
}

int32_t AbilityManagerStub::StartSpecifiedAbilityBySCBInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call.");
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    StartSpecifiedAbilityBySCB(*want);
    return NO_ERROR;
}

int AbilityManagerStub::NotifySaveAsResultInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (!want) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    int resultCode = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = NotifySaveAsResult(*want, resultCode, requestCode);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::SetSessionManagerServiceInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> sessionManagerService = data.ReadRemoteObject();
    if (!sessionManagerService) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetSessionManagerServiceInner read ability token failed.");
        return ERR_NULL_OBJECT;
    }
    SetSessionManagerService(sessionManagerService);
    return NO_ERROR;
}

int32_t AbilityManagerStub::RegisterIAbilityManagerCollaboratorInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t type = data.ReadInt32();
    sptr<IAbilityManagerCollaborator> collaborator = iface_cast<IAbilityManagerCollaborator>(data.ReadRemoteObject());
    if (collaborator == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read collaborator failed.");
        return ERR_NULL_OBJECT;
    }
    int32_t ret = RegisterIAbilityManagerCollaborator(type, collaborator);
    reply.WriteInt32(ret);
    return NO_ERROR;
}

int32_t AbilityManagerStub::UnregisterIAbilityManagerCollaboratorInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t type = data.ReadInt32();
    int32_t ret = UnregisterIAbilityManagerCollaborator(type);
    reply.WriteInt32(ret);
    return NO_ERROR;
}

int AbilityManagerStub::PrepareTerminateAbilityBySCBInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call.");
    sptr<SessionInfo> sessionInfo = nullptr;
    if (data.ReadBool()) {
        sessionInfo = data.ReadParcelable<SessionInfo>();
    }
    bool isPrepareTerminate = false;
    auto result = PrepareTerminateAbilityBySCB(sessionInfo, isPrepareTerminate);
    if (result == ERR_OK) {
        if (!reply.WriteBool(isPrepareTerminate)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "reply write failed.");
            return ERR_INVALID_VALUE;
        }
    }
    return result;
}

int32_t AbilityManagerStub::RegisterStatusBarDelegateInner(MessageParcel &data, MessageParcel &reply)
{
    auto delegate = iface_cast<AbilityRuntime::IStatusBarDelegate>(data.ReadRemoteObject());
    if (delegate == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "delegate is nullptr.");
        return ERR_NULL_OBJECT;
    }
    int32_t result = RegisterStatusBarDelegate(delegate);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AbilityManagerStub::KillProcessWithPrepareTerminateInner(MessageParcel &data, MessageParcel &reply)
{
    auto size = data.ReadUint32();
    if (size == 0 || size > MAX_KILL_PROCESS_PID_COUNT) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid size.");
        return ERR_INVALID_VALUE;
    }
    std::vector<int32_t> pids;
    for (uint32_t i = 0; i < size; i++) {
        pids.emplace_back(data.ReadInt32());
    }
    int32_t result = KillProcessWithPrepareTerminate(pids);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "KillProcessWithPrepareTerminate failed.");
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::RegisterAutoStartupSystemCallbackInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> callback = data.ReadRemoteObject();
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Callback is nullptr.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = RegisterAutoStartupSystemCallback(callback);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AbilityManagerStub::UnregisterAutoStartupSystemCallbackInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> callback = data.ReadRemoteObject();
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Callback is nullptr.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = UnregisterAutoStartupSystemCallback(callback);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AbilityManagerStub::SetApplicationAutoStartupInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<AutoStartupInfo> info = data.ReadParcelable<AutoStartupInfo>();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Info is nullptr.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = SetApplicationAutoStartup(*info);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AbilityManagerStub::CancelApplicationAutoStartupInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<AutoStartupInfo> info = data.ReadParcelable<AutoStartupInfo>();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Info is nullptr.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = CancelApplicationAutoStartup(*info);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AbilityManagerStub::QueryAllAutoStartupApplicationsInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<AutoStartupInfo> infoList;
    auto result = QueryAllAutoStartupApplications(infoList);
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }

    reply.WriteInt32(static_cast<int32_t>(infoList.size()));
    for (auto &info : infoList) {
        if (!reply.WriteParcelable(&info)) {
            return ERR_INVALID_VALUE;
        }
    }
    return NO_ERROR;
}

int AbilityManagerStub::RegisterSessionHandlerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> handler = data.ReadRemoteObject();
    if (handler == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "stub register session handler, handler is nullptr.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = RegisterSessionHandler(handler);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AbilityManagerStub::RegisterAppDebugListenerInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto appDebugLister = iface_cast<AppExecFwk::IAppDebugListener>(data.ReadRemoteObject());
    if (appDebugLister == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "App debug lister is nullptr.");
        return ERR_INVALID_VALUE;
    }

    auto result = RegisterAppDebugListener(appDebugLister);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::UnregisterAppDebugListenerInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto appDebugLister = iface_cast<AppExecFwk::IAppDebugListener>(data.ReadRemoteObject());
    if (appDebugLister == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "App debug lister is nullptr.");
        return ERR_INVALID_VALUE;
    }

    auto result = UnregisterAppDebugListener(appDebugLister);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::AttachAppDebugInner(MessageParcel &data, MessageParcel &reply)
{
    auto bundleName = data.ReadString();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Bundle name is empty.");
        return ERR_INVALID_VALUE;
    }

    auto result = AttachAppDebug(bundleName);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::DetachAppDebugInner(MessageParcel &data, MessageParcel &reply)
{
    auto bundleName = data.ReadString();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Bundle name is empty.");
        return ERR_INVALID_VALUE;
    }

    auto result = DetachAppDebug(bundleName);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::IsAbilityControllerStartInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr");
        return true;
    }
    bool result = IsAbilityControllerStart(*want);
    reply.WriteBool(result);
    return NO_ERROR;
}

int32_t AbilityManagerStub::ExecuteIntentInner(MessageParcel &data, MessageParcel &reply)
{
    uint64_t key = data.ReadUint64();
    sptr<IRemoteObject> callerToken = data.ReadRemoteObject();
    if (callerToken == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get remote object.");
        return ERR_INVALID_VALUE;
    }
    std::unique_ptr<InsightIntentExecuteParam> param(data.ReadParcelable<InsightIntentExecuteParam>());
    if (param == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "param is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto result = ExecuteIntent(key, callerToken, *param);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityForResultAsCallerInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The want is nullptr.");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    int32_t requestCode = data.ReadInt32();
    int32_t userId = data.ReadInt32();
    int32_t result = StartAbilityForResultAsCaller(*want, callerToken, requestCode, userId);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityForResultAsCallerForOptionsInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The want is nullptr.");
        return ERR_INVALID_VALUE;
    }
    std::unique_ptr<StartOptions> startOptions(data.ReadParcelable<StartOptions>());
    if (startOptions == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The startOptions is nullptr.");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    int32_t requestCode = data.ReadInt32();
    int32_t userId = data.ReadInt32();
    int32_t result = StartAbilityForResultAsCaller(*want, *startOptions, callerToken, requestCode, userId);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AbilityManagerStub::StartAbilityByInsightIntentInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> callerToken = nullptr;
    if (!data.ReadBool()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid caller token");
        return ERR_INVALID_VALUE;
    }
    callerToken = data.ReadRemoteObject();
    uint64_t intentId = data.ReadUint64();
    int32_t userId = data.ReadInt32();
    int32_t result = StartAbilityByInsightIntent(*want, callerToken, intentId, userId);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AbilityManagerStub::ExecuteInsightIntentDoneInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto token = data.ReadRemoteObject();
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get remote object.");
        return ERR_INVALID_VALUE;
    }

    auto intentId = data.ReadInt64();
    std::unique_ptr<InsightIntentExecuteResult> executeResult(data.ReadParcelable<InsightIntentExecuteResult>());
    if (!executeResult) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Execute result is nullptr");
        return ERR_INVALID_VALUE;
    }

    int32_t result = ExecuteInsightIntentDone(token, intentId, *executeResult);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AbilityManagerStub::SetApplicationAutoStartupByEDMInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<AutoStartupInfo> info = data.ReadParcelable<AutoStartupInfo>();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Info is nullptr.");
        return ERR_INVALID_VALUE;
    }
    auto flag = data.ReadBool();
    int32_t result = SetApplicationAutoStartupByEDM(*info, flag);
    return reply.WriteInt32(result);
}

int32_t AbilityManagerStub::CancelApplicationAutoStartupByEDMInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<AutoStartupInfo> info = data.ReadParcelable<AutoStartupInfo>();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Info is nullptr.");
        return ERR_INVALID_VALUE;
    }
    auto flag = data.ReadBool();
    int32_t result = CancelApplicationAutoStartupByEDM(*info, flag);
    return reply.WriteInt32(result);
}

int32_t AbilityManagerStub::OpenFileInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (!uri) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "To read uri failed.");
        return ERR_DEAD_OBJECT;
    }
    auto flag = data.ReadInt32();
    int fd = OpenFile(*uri, flag);
    reply.WriteFileDescriptor(fd);
    return ERR_OK;
}

int32_t AbilityManagerStub::RequestAssertFaultDialogInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Request to display assert fault dialog.");
    sptr<IRemoteObject> callback = data.ReadRemoteObject();
    std::unique_ptr<WantParams> wantParams(data.ReadParcelable<WantParams>());
    if (wantParams == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ContinueMissionInner wantParams readParcelable failed.");
        return ERR_NULL_OBJECT;
    }
    auto result = RequestAssertFaultDialog(callback, *wantParams);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write result failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::NotifyDebugAssertResultInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Notify user action result to assert fault process.");
    uint64_t assertSessionId = data.ReadUint64();
    int32_t status = data.ReadInt32();
    auto result = NotifyDebugAssertResult(assertSessionId, static_cast<AAFwk::UserStatus>(status));
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write result failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::GetForegroundUIAbilitiesInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    std::vector<AppExecFwk::AbilityStateData> abilityStateDatas;
    int32_t result = GetForegroundUIAbilities(abilityStateDatas);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get foreground uI abilities is failed.");
        return result;
    }
    auto infoSize = abilityStateDatas.size();
    if (infoSize > CYCLE_LIMIT) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Info size exceeds the limit.");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(infoSize)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write data size failed.");
        return ERR_INVALID_VALUE;
    }
    for (auto &it : abilityStateDatas) {
        if (!reply.WriteParcelable(&it)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Write parcelable failed.");
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write result failed.");
        return ERR_INVALID_VALUE;
    }
    return result;
}

int32_t AbilityManagerStub::UpdateSessionInfoBySCBInner(MessageParcel &data, MessageParcel &reply)
{
    auto size = data.ReadInt32();
    int32_t threshold = 512;
    if (size > threshold) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Size of vector too large.");
        return ERR_ENOUGH_DATA;
    }
    std::list<SessionInfo> sessionInfos;
    for (auto i = 0; i < size; i++) {
        std::unique_ptr<SessionInfo> info(data.ReadParcelable<SessionInfo>());
        if (info == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Read session info failed.");
            return ERR_NATIVE_IPC_PARCEL_FAILED;
        }
        sessionInfos.emplace_back(*info);
    }
    int32_t userId = data.ReadInt32();
    std::vector<int32_t> sessionIds;
    auto result = UpdateSessionInfoBySCB(sessionInfos, userId, sessionIds);
    if (result != ERR_OK) {
        return result;
    }
    size = static_cast<int32_t>(sessionIds.size());
    if (size > threshold) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Size of vector too large for sessionIds.");
        return ERR_ENOUGH_DATA;
    }
    reply.WriteInt32(size);
    for (auto index = 0; index < size; index++) {
        reply.WriteInt32(sessionIds[index]);
    }
    return ERR_OK;
}

int32_t AbilityManagerStub::GetUIExtensionRootHostInfoInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
        if (callerToken == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "caller token is nullptr.");
            return ERR_INVALID_VALUE;
        }
    }

    int32_t userId = data.ReadInt32();
    UIExtensionHostInfo hostInfo;
    auto result = GetUIExtensionRootHostInfo(callerToken, hostInfo, userId);
    if (!reply.WriteParcelable(&hostInfo)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write host info failed.");
        return ERR_INVALID_VALUE;
    }

    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write result failed.");
        return ERR_INVALID_VALUE;
    }

    return NO_ERROR;
}

int32_t AbilityManagerStub::GetUIExtensionSessionInfoInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
        if (callerToken == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "caller token is nullptr.");
            return ERR_INVALID_VALUE;
        }
    }

    int32_t userId = data.ReadInt32();
    UIExtensionSessionInfo uiExtensionSessionInfo;
    auto result = GetUIExtensionSessionInfo(callerToken, uiExtensionSessionInfo, userId);
    if (!reply.WriteParcelable(&uiExtensionSessionInfo)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write ui extension session info failed.");
        return ERR_INVALID_VALUE;
    }

    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write result failed.");
        return ERR_INVALID_VALUE;
    }

    return NO_ERROR;
}

int32_t AbilityManagerStub::RestartAppInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call.");
    std::unique_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr");
        return IPC_STUB_ERR;
    }
    bool isAppRecovery = data.ReadBool();
    auto result = RestartApp(*want, isAppRecovery);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to write result.");
        return IPC_STUB_ERR;
    }
    return ERR_OK;
}

int32_t AbilityManagerStub::OpenAtomicServiceInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::unique_ptr<StartOptions> options(data.ReadParcelable<StartOptions>());
    if (options == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "options is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    int32_t requestCode = data.ReadInt32();
    int32_t userId = data.ReadInt32();
    int32_t openRet = OpenAtomicService(*want, *options, callerToken, requestCode, userId);
    if (openRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Open atomic service to be failed.");
        return openRet;
    }
    if (!reply.WriteInt32(openRet)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write openRet failed.");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t AbilityManagerStub::SetResidentProcessEnableInner(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = data.ReadString();
    bool enable = data.ReadBool();
    auto result = SetResidentProcessEnabled(bundleName, enable);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write result failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::IsEmbeddedOpenAllowedInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
        if (callerToken == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "caller token is nullptr.");
            return ERR_INVALID_VALUE;
        }
    }

    std::string appId = data.ReadString();
    auto result = IsEmbeddedOpenAllowed(callerToken, appId);

    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write result failed.");
        return ERR_INVALID_VALUE;
    }

    return NO_ERROR;
}

int32_t AbilityManagerStub::StartShortcutInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    StartOptions *startOptions = data.ReadParcelable<StartOptions>();
    if (startOptions == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "startOptions is nullptr");
        return ERR_INVALID_VALUE;
    }
    startOptions->processOptions = nullptr;

    int32_t result = StartShortcut(*want, *startOptions);
    reply.WriteInt32(result);
    delete startOptions;
    return NO_ERROR;
}

int32_t AbilityManagerStub::GetAbilityStateByPersistentIdInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t persistentId = data.ReadInt32();
    bool state = false;
    int32_t result = GetAbilityStateByPersistentId(persistentId, state);
    if (result == ERR_OK) {
        if (!reply.WriteBool(state)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "reply write failed.");
            return IPC_STUB_ERR;
        }
    }
    return result;
}

int32_t AbilityManagerStub::TransferAbilityResultForExtensionInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> callerToken = data.ReadRemoteObject();
    int32_t resultCode = data.ReadInt32();
    sptr<Want> want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t result = TransferAbilityResultForExtension(callerToken, resultCode, *want);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AbilityManagerStub::NotifyFrozenProcessByRSSInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<int32_t> pidList;
    data.ReadInt32Vector(&pidList);
    int32_t uid = data.ReadInt32();
    NotifyFrozenProcessByRSS(pidList, uid);
    return NO_ERROR;
}

int32_t AbilityManagerStub::PreStartMissionInner(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = data.ReadString();
    std::string moduleName = data.ReadString();
    std::string abilityName = data.ReadString();
    std::string startTime = data.ReadString();
    int32_t result = PreStartMission(bundleName, moduleName, abilityName, startTime);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AbilityManagerStub::CleanUIAbilityBySCBInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<SessionInfo> sessionInfo = nullptr;
    if (data.ReadBool()) {
        sessionInfo = data.ReadParcelable<SessionInfo>();
    }
    int32_t result = CleanUIAbilityBySCB(sessionInfo);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AbilityManagerStub::OpenLinkInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<Want> want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want is nullptr.");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = data.ReadRemoteObject();
    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();

    int32_t result = OpenLink(*want, callerToken, userId, requestCode);
    if (result != NO_ERROR && result != ERR_OPEN_LINK_START_ABILITY_DEFAULT_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "OpenLink failed.");
    }
    reply.WriteInt32(result);
    return result;
}

int32_t AbilityManagerStub::TerminateMissionInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t missionId = data.ReadInt32();
    int32_t result = TerminateMission(missionId);
    if (result != NO_ERROR && result != ERR_OPEN_LINK_START_ABILITY_DEFAULT_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "OpenLink failed.");
    }
    reply.WriteInt32(result);
    return result;
}
} // namespace AAFwk
} // namespace OHOS