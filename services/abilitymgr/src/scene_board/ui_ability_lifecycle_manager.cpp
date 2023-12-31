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

#include "scene_board/ui_ability_lifecycle_manager.h"

#include "ability_manager_service.h"
#include "ability_util.h"
#include "errors.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "iability_info_callback.h"
#include "session/host/include/zidl/session_interface.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr char EVENT_KEY_UID[] = "UID";
constexpr char EVENT_KEY_PID[] = "PID";
constexpr char EVENT_KEY_MESSAGE[] = "MSG";
constexpr char EVENT_KEY_PACKAGE_NAME[] = "PACKAGE_NAME";
constexpr char EVENT_KEY_PROCESS_NAME[] = "PROCESS_NAME";
const std::string DLP_INDEX = "ohos.dlp.params.index";
#ifdef SUPPORT_ASAN
const int KILL_TIMEOUT_MULTIPLE = 45;
#else
const int KILL_TIMEOUT_MULTIPLE = 3;
#endif
}
int UIAbilityLifecycleManager::StartUIAbility(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    HILOG_DEBUG("Call.");
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        HILOG_ERROR("sessionInfo is invalid.");
        return ERR_INVALID_VALUE;
    }
    auto sessionToken = iface_cast<Rosen::ISession>(sessionInfo->sessionToken);
    auto descriptor = Str16ToStr8(sessionToken->GetDescriptor());
    if (descriptor != "OHOS.ISession") {
        HILOG_ERROR("token's Descriptor: %{public}s", descriptor.c_str());
        return ERR_INVALID_VALUE;
    }
    abilityRequest.sessionInfo = sessionInfo;

    std::shared_ptr<AbilityRecord> uiAbilityRecord = nullptr;
    auto iter = sessionAbilityMap_.find(sessionInfo->persistentId);
    if (iter != sessionAbilityMap_.end()) {
        uiAbilityRecord = iter->second;
        uiAbilityRecord->SetWant(abilityRequest.want);
        uiAbilityRecord->SetIsNewWant(true);
    } else {
        if (sessionInfo->startSetting != nullptr) {
            HILOG_DEBUG("startSetting is valid.");
            abilityRequest.startSetting = sessionInfo->startSetting;
        }
        uiAbilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    }
    CHECK_POINTER_AND_RETURN(uiAbilityRecord, ERR_INVALID_VALUE);

    if (uiAbilityRecord->GetPendingState() == AbilityState::FOREGROUND) {
        HILOG_DEBUG("pending state is FOREGROUND.");
        uiAbilityRecord->SetPendingState(AbilityState::FOREGROUND);
        return ERR_OK;
    } else {
        HILOG_DEBUG("pending state is not FOREGROUND.");
        uiAbilityRecord->SetPendingState(AbilityState::FOREGROUND);
    }

    ReportEventToSuspendManager(abilityRequest.abilityInfo);
    UpdateAbilityRecordLaunchReason(abilityRequest, uiAbilityRecord);
    NotifyAbilityToken(uiAbilityRecord->GetToken(), abilityRequest);

    uiAbilityRecord->AddCallerRecord(sessionInfo->callerToken, sessionInfo->requestCode);
    if (iter == sessionAbilityMap_.end()) {
        sessionAbilityMap_.emplace(sessionInfo->persistentId, uiAbilityRecord);
    }
    uiAbilityRecord->ProcessForegroundAbility();
    if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED && !specifiedInfoQueue_.empty()) {
        SpecifiedInfo specifiedInfo = specifiedInfoQueue_.front();
        specifiedInfoQueue_.pop();
        uiAbilityRecord->SetSpecifiedFlag(specifiedInfo.flag);
        specifiedAbilityMap_.emplace(specifiedInfo, uiAbilityRecord);
    }
    return ERR_OK;
}

int UIAbilityLifecycleManager::AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler,
    const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (!IsContainsAbilityInner(token)) {
        return ERR_INVALID_VALUE;
    }
    auto&& abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    HILOG_DEBUG("AbilityMS attach abilityThread, name is %{public}s.", abilityRecord->GetAbilityInfo().name.c_str());

    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecord->GetAbilityRecordId());

    abilityRecord->SetScheduler(scheduler);
    if (abilityRecord->IsStartedByCall()) {
        if (abilityRecord->GetWant().GetBoolParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, false)) {
            abilityRecord->SetStartToForeground(true);
            DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token);
        } else {
            abilityRecord->SetStartToBackground(true);
            MoveToBackground(abilityRecord);
        }
        return ERR_OK;
    }

    if (abilityRecord->IsNeedToCallRequest()) {
        abilityRecord->CallRequest();
    }

    DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token);
    return ERR_OK;
}

void UIAbilityLifecycleManager::OnAbilityRequestDone(const sptr<IRemoteObject> &token, int32_t state) const
{
    HILOG_DEBUG("Ability request state %{public}d done.", state);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    AppAbilityState abilityState = DelayedSingleton<AppScheduler>::GetInstance()->ConvertToAppAbilityState(state);
    if (abilityState == AppAbilityState::ABILITY_STATE_FOREGROUND) {
        auto&& abilityRecord = Token::GetAbilityRecordByToken(token);
        CHECK_POINTER(abilityRecord);
        std::string element = abilityRecord->GetWant().GetElement().GetURI();
        HILOG_DEBUG("Ability is %{public}s, start to foreground.", element.c_str());
        abilityRecord->ForegroundAbility();
    }
}

int UIAbilityLifecycleManager::AbilityTransactionDone(const sptr<IRemoteObject> &token, int state,
    const PacMap &saveData)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int targetState = AbilityRecord::ConvertLifeCycleToAbilityState(static_cast<AbilityLifeCycleState>(state));
    std::string abilityState = AbilityRecord::ConvertAbilityState(static_cast<AbilityState>(targetState));
    HILOG_INFO("AbilityTransactionDone, state: %{public}s.", abilityState.c_str());

    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    auto abilityRecord = GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_DEBUG("ability: %{public}s, state: %{public}s", element.c_str(), abilityState.c_str());

    if (targetState == AbilityState::BACKGROUND) {
        abilityRecord->SaveAbilityState(saveData);
    }

    return DispatchState(abilityRecord, targetState);
}

int UIAbilityLifecycleManager::NotifySCBToStartUIAbility(const AbilityRequest &abilityRequest)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Call.");
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    auto isSpecified = (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED);
    if (isSpecified) {
        EnqueueAbilityToFront(abilityRequest);
        DelayedSingleton<AppScheduler>::GetInstance()->StartSpecifiedAbility(
            abilityRequest.want, abilityRequest.abilityInfo);
        return ERR_OK;
    }
    auto sessionInfo = CreateSessionInfo(abilityRequest);
    sessionInfo->requestCode = abilityRequest.requestCode;
    sessionInfo->persistentId = GetPersistentIdByAbilityRequest(abilityRequest);
    return NotifySCBPendingActivation(sessionInfo, abilityRequest.callerToken);
}

int UIAbilityLifecycleManager::DispatchState(const std::shared_ptr<AbilityRecord> &abilityRecord, int state)
{
    switch (state) {
        case AbilityState::INITIAL: {
            return DispatchTerminate(abilityRecord);
        }
        case AbilityState::BACKGROUND: {
            return DispatchBackground(abilityRecord);
        }
        case AbilityState::FOREGROUND: {
            return DispatchForeground(abilityRecord, true);
        }
        case AbilityState::FOREGROUND_FAILED:
        case AbilityState::FOREGROUND_INVALID_MODE:
        case AbilityState::FOREGROUND_WINDOW_FREEZED: {
            return DispatchForeground(abilityRecord, false, static_cast<AbilityState>(state));
        }
        default: {
            HILOG_WARN("Don't support transiting state: %{public}d", state);
            return ERR_INVALID_VALUE;
        }
    }
}

int UIAbilityLifecycleManager::DispatchForeground(const std::shared_ptr<AbilityRecord> &abilityRecord, bool success,
    AbilityState state)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    auto taskHandler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER_AND_RETURN_LOG(taskHandler, ERR_INVALID_VALUE, "Fail to get AbilityTaskHandler.");
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (!abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING)) {
        HILOG_ERROR("DispatchForeground Ability transition life state error. expect %{public}d, actual %{public}d",
            AbilityState::FOREGROUNDING,
            abilityRecord->GetAbilityState());
        return ERR_INVALID_VALUE;
    }

    handler->RemoveEvent(AbilityManagerService::FOREGROUND_TIMEOUT_MSG, abilityRecord->GetAbilityRecordId());
    auto self(weak_from_this());
    if (success) {
        HILOG_INFO("foreground succeeded.");
        auto task = [self, abilityRecord]() {
            auto selfObj = self.lock();
            if (!selfObj) {
                HILOG_WARN("mgr is invalid.");
                return;
            }
            selfObj->CompleteForegroundSuccess(abilityRecord);
        };
        taskHandler->SubmitTask(task);
    } else {
        auto task = [self, abilityRecord, state]() {
            auto selfObj = self.lock();
            if (!selfObj) {
                HILOG_WARN("Mission list mgr is invalid.");
                return;
            }
            if (state == AbilityState::FOREGROUND_WINDOW_FREEZED) {
                HILOG_INFO("Window was freezed.");
                if (abilityRecord != nullptr) {
                    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
                    DelayedSingleton<AppScheduler>::GetInstance()->MoveToBackground(abilityRecord->GetToken());
                }
                return;
            }
            selfObj->HandleForegroundFailed(abilityRecord, state);
        };
        taskHandler->SubmitTask(task);
    }
    return ERR_OK;
}

int UIAbilityLifecycleManager::DispatchBackground(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityTaskHandler.");
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (!abilityRecord->IsAbilityState(AbilityState::BACKGROUNDING)) {
        HILOG_ERROR("Ability transition life state error. actual %{public}d", abilityRecord->GetAbilityState());
        return ERR_INVALID_VALUE;
    }

    // remove background timeout task.
    handler->CancelTask("background_" + std::to_string(abilityRecord->GetAbilityRecordId()));
    auto self(shared_from_this());
    auto task = [self, abilityRecord]() { self->CompleteBackground(abilityRecord); };
    handler->SubmitTask(task);

    return ERR_OK;
}

int UIAbilityLifecycleManager::DispatchTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (abilityRecord->GetAbilityState() != AbilityState::TERMINATING) {
        HILOG_ERROR("DispatchTerminate error, ability state is %{public}d", abilityRecord->GetAbilityState());
        return INNER_ERR;
    }

    // remove terminate timeout task.
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityTaskHandler.");
    handler->CancelTask("terminate_" + std::to_string(abilityRecord->GetAbilityRecordId()));
    auto self(shared_from_this());
    auto task = [self, abilityRecord]() { self->CompleteTerminate(abilityRecord); };
    handler->SubmitTask(task);

    return ERR_OK;
}

void UIAbilityLifecycleManager::CompleteForegroundSuccess(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);

    CHECK_POINTER(abilityRecord);
    // ability do not save window mode
    abilityRecord->RemoveWindowMode();
    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_DEBUG("ability: %{public}s", element.c_str());
    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);

    // new version. started by caller, scheduler call request
    if (abilityRecord->IsStartedByCall() && abilityRecord->IsStartToForeground() && abilityRecord->IsReady()) {
        HILOG_DEBUG("call request after completing foreground state");
        abilityRecord->CallRequest();
        abilityRecord->SetStartToForeground(false);
    }

    if (abilityRecord->GetPendingState() == AbilityState::BACKGROUND) {
        abilityRecord->SetMinimizeReason(true);
        MoveToBackground(abilityRecord);
    } else if (abilityRecord->GetPendingState() == AbilityState::FOREGROUND) {
        HILOG_DEBUG("not continuous startup.");
        abilityRecord->SetPendingState(AbilityState::INITIAL);
    }
}

void UIAbilityLifecycleManager::HandleForegroundFailed(const std::shared_ptr<AbilityRecord> &ability,
    AbilityState state)
{
    HILOG_DEBUG("state: %{public}d.", static_cast<int32_t>(state));
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (ability == nullptr) {
        HILOG_ERROR("ability record is nullptr.");
        return;
    }

    if (!ability->IsAbilityState(AbilityState::FOREGROUNDING)) {
        HILOG_ERROR("this ability is not foregrounding state.");
        return;
    }

    EraseAbilityRecord(ability);
    // foreground failed, notify appMs force terminate the ability
    DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(ability->GetToken());
}

std::shared_ptr<AbilityRecord> UIAbilityLifecycleManager::GetAbilityRecordByToken(const sptr<IRemoteObject> &token)
    const
{
    if (token == nullptr) {
        HILOG_ERROR("nullptr.");
        return nullptr;
    }

    for (auto ability : terminateAbilityList_) {
        if (ability && token == ability->GetToken()->AsObject()) {
            return ability;
        }
    }

    for (auto iter = sessionAbilityMap_.begin(); iter != sessionAbilityMap_.end(); iter++) {
        if (iter->second != nullptr && iter->second->GetToken()->AsObject() == token) {
            return iter->second;
        }
    }
    return nullptr;
}

bool UIAbilityLifecycleManager::IsContainsAbility(const sptr<IRemoteObject> &token) const
{
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    return IsContainsAbilityInner(token);
}

bool UIAbilityLifecycleManager::IsContainsAbilityInner(const sptr<IRemoteObject> &token) const
{
    for (auto iter = sessionAbilityMap_.begin(); iter != sessionAbilityMap_.end(); iter++) {
        if (iter->second != nullptr && iter->second->GetToken()->AsObject() == token) {
            return true;
        }
    }
    return false;
}

void UIAbilityLifecycleManager::EraseAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    if (abilityRecord == nullptr) {
        return;
    }

    for (auto iter = sessionAbilityMap_.begin(); iter != sessionAbilityMap_.end(); iter++) {
        if (iter->second != nullptr && iter->second->GetToken()->AsObject() == abilityRecord->GetToken()->AsObject()) {
            sessionAbilityMap_.erase(iter);
            break;
        }
    }
}

void UIAbilityLifecycleManager::EraseSpecifiedAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    for (auto iter = specifiedAbilityMap_.begin(); iter != specifiedAbilityMap_.end(); iter++) {
        auto abilityInfo = abilityRecord->GetAbilityInfo();
        if (iter->second != nullptr && iter->second->GetToken()->AsObject() == abilityRecord->GetToken()->AsObject() &&
            iter->first.abilityName == abilityInfo.name && iter->first.bundleName == abilityInfo.bundleName &&
            iter->first.flag == abilityRecord->GetSpecifiedFlag()) {
            specifiedAbilityMap_.erase(iter);
            break;
        }
    }
}

void UIAbilityLifecycleManager::UpdateAbilityRecordLaunchReason(
    const AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &abilityRecord) const
{
    if (abilityRecord == nullptr) {
        HILOG_WARN("input record is nullptr.");
        return;
    }

    if (abilityRequest.IsContinuation()) {
        abilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_CONTINUATION);
        return;
    }

    if (abilityRequest.IsAppRecovery() || abilityRecord->GetRecoveryInfo()) {
        abilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_APP_RECOVERY);
        return;
    }

    abilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_START_ABILITY);
    return;
}

std::shared_ptr<AbilityRecord> UIAbilityLifecycleManager::GetUIAbilityRecordBySessionInfo(
    const sptr<SessionInfo> &sessionInfo)
{
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    CHECK_POINTER_AND_RETURN(sessionInfo, nullptr);
    CHECK_POINTER_AND_RETURN(sessionInfo->sessionToken, nullptr);
    auto sessionToken = iface_cast<Rosen::ISession>(sessionInfo->sessionToken);
    std::string descriptor = Str16ToStr8(sessionToken->GetDescriptor());
    if (descriptor != "OHOS.ISession") {
        HILOG_ERROR("failed, input token is not a sessionToken, token->GetDescriptor(): %{public}s",
            descriptor.c_str());
        return nullptr;
    }

    auto iter = sessionAbilityMap_.find(sessionInfo->persistentId);
    if (iter != sessionAbilityMap_.end()) {
        return iter->second;
    }
    return nullptr;
}

int UIAbilityLifecycleManager::MinimizeUIAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HILOG_DEBUG("call");
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("ability record is null");
        return ERR_INVALID_VALUE;
    }
    HILOG_INFO("abilityInfoName:%{public}s", abilityRecord->GetAbilityInfo().name.c_str());
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    if (!abilityRecord->IsAbilityState(AbilityState::FOREGROUND)) {
        HILOG_ERROR("ability state is not foreground");
        return ERR_OK;
    }
    MoveToBackground(abilityRecord);
    return ERR_OK;
}

void UIAbilityLifecycleManager::MoveToBackground(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("ability record is null");
        return;
    }
    abilityRecord->SetIsNewWant(false);
    auto self(weak_from_this());
    auto task = [abilityRecord, self]() {
        auto selfObj = self.lock();
        if (selfObj == nullptr) {
            HILOG_WARN("UIAbilityLifecycleManager is invalid");
            return;
        }
        HILOG_ERROR("UIAbilityLifecycleManager move to background timeout");
        selfObj->PrintTimeOutLog(abilityRecord, AbilityManagerService::BACKGROUND_TIMEOUT_MSG);
        selfObj->CompleteBackground(abilityRecord);
    };
    abilityRecord->BackgroundAbility(task);
}

int UIAbilityLifecycleManager::ResolveLocked(const AbilityRequest &abilityRequest)
{
    HILOG_INFO("ability_name:%{public}s", abilityRequest.want.GetElement().GetURI().c_str());

    if (!abilityRequest.IsCallType(AbilityCallType::CALL_REQUEST_TYPE)) {
        HILOG_ERROR("%{public}s, resolve ability_name:", __func__);
        return RESOLVE_CALL_ABILITY_INNER_ERR;
    }

    return CallAbilityLocked(abilityRequest);
}

int UIAbilityLifecycleManager::CallAbilityLocked(const AbilityRequest &abilityRequest)
{
    HILOG_DEBUG("Call.");
    std::lock_guard<ffrt::mutex> guard(sessionLock_);

    // Get target uiAbility record.
    std::shared_ptr<AbilityRecord> uiAbilityRecord;
    auto persistentId = GetPersistentIdByAbilityRequest(abilityRequest);
    if (persistentId == 0) {
        uiAbilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    } else {
        uiAbilityRecord = sessionAbilityMap_.at(persistentId);
    }

    uiAbilityRecord->AddCallerRecord(abilityRequest.callerToken, abilityRequest.requestCode);
    uiAbilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_CALL);
    NotifyAbilityToken(uiAbilityRecord->GetToken(), abilityRequest);

    // new version started by call type
    auto ret = ResolveAbility(uiAbilityRecord, abilityRequest);
    if (ret == ResolveResultType::OK_HAS_REMOTE_OBJ) {
        HILOG_DEBUG("target ability has been resolved.");
        if (abilityRequest.want.GetBoolParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, false)) {
            HILOG_DEBUG("target ability needs to be switched to foreground.");
            auto sessionInfo = CreateSessionInfo(abilityRequest);
            sessionInfo->persistentId = persistentId;
            sessionInfo->state = CallToState::FOREGROUND;
            DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(uiAbilityRecord->GetToken());
            return NotifySCBPendingActivation(sessionInfo, abilityRequest.callerToken);
        }
    } else if (ret == ResolveResultType::NG_INNER_ERROR) {
        HILOG_ERROR("resolve failed, error: %{public}d.", RESOLVE_CALL_ABILITY_INNER_ERR);
        return RESOLVE_CALL_ABILITY_INNER_ERR;
    }

    auto sessionInfo = CreateSessionInfo(abilityRequest);
    sessionInfo->persistentId = persistentId;
    sessionInfo->uiAbilityId = uiAbilityRecord->GetAbilityRecordId();
    if (abilityRequest.want.GetBoolParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, false)) {
        sessionInfo->state = CallToState::FOREGROUND;
    } else {
        sessionInfo->state = CallToState::BACKGROUND;
    }
    HILOG_DEBUG("Notify scb's abilityId is %{public}" PRIu64 ".", sessionInfo->uiAbilityId);
    tmpAbilityMap_.emplace(uiAbilityRecord->GetAbilityRecordId(), uiAbilityRecord);
    return NotifySCBPendingActivation(sessionInfo, abilityRequest.callerToken);
}

void UIAbilityLifecycleManager::CallUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo)
{
    HILOG_DEBUG("Call.");
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        HILOG_ERROR("sessionInfo is invalid.");
        return;
    }
    auto sessionToken = iface_cast<Rosen::ISession>(sessionInfo->sessionToken);
    auto descriptor = Str16ToStr8(sessionToken->GetDescriptor());
    if (descriptor != "OHOS.ISession") {
        HILOG_ERROR("token's Descriptor: %{public}s", descriptor.c_str());
        return;
    }

    HILOG_DEBUG("SCB output abilityId is %{public}" PRIu64 ".", sessionInfo->uiAbilityId);
    auto search = tmpAbilityMap_.find(sessionInfo->uiAbilityId);
    if (search == tmpAbilityMap_.end()) {
        HILOG_WARN("Not found UIAbility.");
        return;
    }
    auto uiAbilityRecord = search->second;
    if (uiAbilityRecord == nullptr) {
        HILOG_ERROR("UIAbility not exist.");
        return;
    }
    auto sessionSearch = sessionAbilityMap_.find(sessionInfo->persistentId);
    if (sessionSearch != sessionAbilityMap_.end()) {
        HILOG_ERROR("Session already exist.");
        return;
    }

    sessionAbilityMap_.emplace(sessionInfo->persistentId, uiAbilityRecord);
    tmpAbilityMap_.erase(search);
    uiAbilityRecord->SetSessionInfo(sessionInfo);

    uiAbilityRecord->LoadAbility();
}

sptr<SessionInfo> UIAbilityLifecycleManager::CreateSessionInfo(const AbilityRequest &abilityRequest) const
{
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    sessionInfo->callerToken = abilityRequest.callerToken;
    sessionInfo->want = abilityRequest.want;
    if (abilityRequest.startSetting != nullptr) {
        sessionInfo->startSetting = abilityRequest.startSetting;
    }
    return sessionInfo;
}

int UIAbilityLifecycleManager::NotifySCBPendingActivation(sptr<SessionInfo> &sessionInfo,
    const sptr<IRemoteObject> &token) const
{
    auto abilityRecord = GetAbilityRecordByToken(token);
    if (abilityRecord == nullptr) {
        CHECK_POINTER_AND_RETURN(rootSceneSession_, ERR_INVALID_VALUE);
        HILOG_INFO("Call PendingSessionActivation by callerSession.");
        return static_cast<int>(rootSceneSession_->PendingSessionActivation(sessionInfo));
    } else {
        auto callerSessionInfo = abilityRecord->GetSessionInfo();
        CHECK_POINTER_AND_RETURN(callerSessionInfo, ERR_INVALID_VALUE);
        CHECK_POINTER_AND_RETURN(callerSessionInfo->sessionToken, ERR_INVALID_VALUE);
        auto callerSession = iface_cast<Rosen::ISession>(callerSessionInfo->sessionToken);
        HILOG_INFO("Call PendingSessionActivation by rootSceneSession.");
        return static_cast<int>(callerSession->PendingSessionActivation(sessionInfo));
    }
}

int UIAbilityLifecycleManager::ResolveAbility(
    const std::shared_ptr<AbilityRecord> &targetAbility, const AbilityRequest &abilityRequest) const
{
    HILOG_DEBUG("targetAbilityRecord resolve call record.");
    CHECK_POINTER_AND_RETURN(targetAbility, ResolveResultType::NG_INNER_ERROR);

    ResolveResultType result = targetAbility->Resolve(abilityRequest);
    switch (result) {
        case ResolveResultType::NG_INNER_ERROR:
        case ResolveResultType::OK_HAS_REMOTE_OBJ:
            return result;
        default:
            break;
    }

    if (targetAbility->IsReady()) {
        HILOG_DEBUG("targetAbility is ready, directly scheduler call request.");
        targetAbility->CallRequest();
        return ResolveResultType::OK_HAS_REMOTE_OBJ;
    }

    HILOG_DEBUG("targetAbility need to call request after lifecycle.");
    return result;
}

void UIAbilityLifecycleManager::NotifyAbilityToken(const sptr<IRemoteObject> &token,
    const AbilityRequest &abilityRequest) const
{
    auto abilityInfoCallback = iface_cast<AppExecFwk::IAbilityInfoCallback>(abilityRequest.abilityInfoCallback);
    if (abilityInfoCallback != nullptr) {
        abilityInfoCallback->NotifyAbilityToken(token, abilityRequest.want);
    }
}

void UIAbilityLifecycleManager::PrintTimeOutLog(const std::shared_ptr<AbilityRecord> &ability, uint32_t msgId)
{
    if (ability == nullptr) {
        HILOG_ERROR("failed, ability is nullptr");
        return;
    }

    AppExecFwk::RunningProcessInfo processInfo = {};
    DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByToken(ability->GetToken(), processInfo);
    if (processInfo.pid_ == 0) {
        HILOG_ERROR("failed, error: the ability[%{public}s], app may fork fail or not running.",
            ability->GetAbilityInfo().name.data());
        return;
    }
    std::string msgContent = "ability:" + ability->GetAbilityInfo().name + " ";
    switch (msgId) {
        case AbilityManagerService::LOAD_TIMEOUT_MSG:
            msgContent += "load timeout";
            break;
        case AbilityManagerService::FOREGROUND_TIMEOUT_MSG:
            msgContent += "foreground timeout";
            break;
        case AbilityManagerService::BACKGROUND_TIMEOUT_MSG:
            msgContent += "background timeout";
            break;
        case AbilityManagerService::TERMINATE_TIMEOUT_MSG:
            msgContent += "terminate timeout";
            break;
        default:
            return;
    }
    std::string eventType = "LIFECYCLE_TIMEOUT";
    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::AAFWK, eventType,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        EVENT_KEY_UID, processInfo.uid_,
        EVENT_KEY_PID, processInfo.pid_,
        EVENT_KEY_PACKAGE_NAME, ability->GetAbilityInfo().bundleName,
        EVENT_KEY_PROCESS_NAME, processInfo.processName_,
        EVENT_KEY_MESSAGE, msgContent);

    HILOG_WARN("LIFECYCLE_TIMEOUT: uid: %{public}d, pid: %{public}d, bundleName: %{public}s, abilityName: %{public}s,"
        "msg: %{public}s", processInfo.uid_, processInfo.pid_, ability->GetAbilityInfo().bundleName.c_str(),
        ability->GetAbilityInfo().name.c_str(), msgContent.c_str());
}

void UIAbilityLifecycleManager::CompleteBackground(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (abilityRecord->GetAbilityState() != AbilityState::BACKGROUNDING) {
        HILOG_ERROR("failed, ability state is %{public}d, it can't complete background.",
            abilityRecord->GetAbilityState());
        return;
    }
    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
    // notify AppMS to update application state.
    DelayedSingleton<AppScheduler>::GetInstance()->MoveToBackground(abilityRecord->GetToken());

    if (abilityRecord->GetPendingState() == AbilityState::FOREGROUND) {
        DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(abilityRecord->GetToken());
    } else if (abilityRecord->GetPendingState() == AbilityState::BACKGROUND) {
        HILOG_DEBUG("not continuous startup.");
        abilityRecord->SetPendingState(AbilityState::INITIAL);
    }

    // new version. started by caller, scheduler call request
    if (abilityRecord->IsStartedByCall() && abilityRecord->IsStartToBackground() && abilityRecord->IsReady()) {
        HILOG_DEBUG("call request after completing background state");
        abilityRecord->CallRequest();
        abilityRecord->SetStartToBackground(false);
    }

    // Abilities ahead of the one started were put in terminate list, we need to terminate them.
    auto self(shared_from_this());
    for (auto terminateAbility : terminateAbilityList_) {
        if (terminateAbility->GetAbilityState() == AbilityState::BACKGROUND) {
            auto timeoutTask = [terminateAbility, self]() {
                HILOG_WARN("Terminate ability timeout after background.");
                self->DelayCompleteTerminate(terminateAbility);
            };
            terminateAbility->Terminate(timeoutTask);
        }
    }
}

int UIAbilityLifecycleManager::CloseUIAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
    int resultCode, const Want *resultWant)
{
    HILOG_DEBUG("call");
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_INFO("call, from ability: %{public}s", element.c_str());
    if (abilityRecord->IsTerminating() && !abilityRecord->IsForeground()) {
        HILOG_INFO("ability is on terminating");
        return ERR_OK;
    }

    abilityRecord->SetTerminatingState();
    // save result to caller AbilityRecord
    if (resultWant != nullptr) {
        abilityRecord->SaveResultToCallers(resultCode, resultWant);
    } else {
        Want want;
        abilityRecord->SaveResultToCallers(-1, &want);
    }

    terminateAbilityList_.push_back(abilityRecord);
    EraseAbilityRecord(abilityRecord);
    abilityRecord->SendResultToCallers();

    if (abilityRecord->IsAbilityState(FOREGROUND) || abilityRecord->IsAbilityState(FOREGROUNDING)) {
        HILOG_DEBUG("current ability is active");
        abilityRecord->SetPendingState(AbilityState::BACKGROUND);
        MoveToBackground(abilityRecord);
        return ERR_OK;
    }

    // ability on background, schedule to terminate.
    if (abilityRecord->GetAbilityState() == AbilityState::BACKGROUND) {
        auto self(shared_from_this());
        auto task = [abilityRecord, self]() {
            HILOG_WARN("close ability by scb timeout");
            self->DelayCompleteTerminate(abilityRecord);
        };
        abilityRecord->Terminate(task);
    }
    return ERR_OK;
}

void UIAbilityLifecycleManager::DelayCompleteTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER(handler);

    PrintTimeOutLog(abilityRecord, AbilityManagerService::TERMINATE_TIMEOUT_MSG);

    auto timeoutTask = [self = shared_from_this(), abilityRecord]() {
        HILOG_INFO("emit delay complete terminate task.");
        self->CompleteTerminate(abilityRecord);
    };
    int killTimeout = AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * KILL_TIMEOUT_MULTIPLE;
    handler->SubmitTask(timeoutTask, "DELAY_KILL_PROCESS", killTimeout);
}

void UIAbilityLifecycleManager::CompleteTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);

    if (abilityRecord->GetAbilityState() != AbilityState::TERMINATING) {
        HILOG_ERROR("failed, %{public}s, ability is not terminating.", __func__);
        return;
    }
    abilityRecord->RemoveAbilityDeathRecipient();

    // notify AppMS terminate
    if (abilityRecord->TerminateAbility() != ERR_OK) {
        // Don't return here
        HILOG_ERROR("AppMS fail to terminate ability.");
    }
    EraseSpecifiedAbilityRecord(abilityRecord);
    terminateAbilityList_.remove(abilityRecord);
}

uint64_t UIAbilityLifecycleManager::GetPersistentIdByAbilityRequest(const AbilityRequest &abilityRequest) const
{
    if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED) {
        return GetReusedSpecifiedPersistentId(abilityRequest);
    }

    if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::STANDARD) {
        return GetReusedStandardPersistentId(abilityRequest);
    }

    if (abilityRequest.abilityInfo.launchMode != AppExecFwk::LaunchMode::SINGLETON) {
        HILOG_WARN("Launch mode is not singleton.");
        return 0;
    }

    for (const auto& [first, second] : sessionAbilityMap_) {
        if (CheckProperties(second, abilityRequest, AppExecFwk::LaunchMode::SINGLETON)) {
            HILOG_DEBUG("SINGLETON: find.");
            return first;
        }
    }

    HILOG_DEBUG("Not find existed ui ability.");
    return 0;
}

uint64_t UIAbilityLifecycleManager::GetReusedSpecifiedPersistentId(const AbilityRequest &abilityRequest) const
{
    if (abilityRequest.abilityInfo.launchMode != AppExecFwk::LaunchMode::SPECIFIED) {
        HILOG_WARN("Not SPECIFIED.");
        return 0;
    }

    // specified ability name and bundle name and module name and appIndex format is same as singleton.
    for (const auto& [first, second] : sessionAbilityMap_) {
        if (second->GetSpecifiedFlag() == abilityRequest.specifiedFlag &&
            CheckProperties(second, abilityRequest, AppExecFwk::LaunchMode::SPECIFIED)) {
            HILOG_DEBUG("SPECIFIED: find.");
            return first;
        }
    }
    return 0;
}

uint64_t UIAbilityLifecycleManager::GetReusedStandardPersistentId(const AbilityRequest &abilityRequest) const
{
    if (abilityRequest.abilityInfo.launchMode != AppExecFwk::LaunchMode::STANDARD) {
        HILOG_WARN("Not STANDARD.");
        return 0;
    }

    if (!abilityRequest.startRecent) {
        HILOG_WARN("startRecent is false.");
        return 0;
    }

    int64_t sessionTime = 0;
    uint64_t persistentId = 0;
    for (const auto& [first, second] : sessionAbilityMap_) {
        if (CheckProperties(second, abilityRequest, AppExecFwk::LaunchMode::STANDARD) &&
            second->GetRestartTime() >= sessionTime) {
            persistentId = first;
            sessionTime = second->GetRestartTime();
        }
    }
    return persistentId;
}

bool UIAbilityLifecycleManager::CheckProperties(const std::shared_ptr<AbilityRecord> &abilityRecord,
    const AbilityRequest &abilityRequest, AppExecFwk::LaunchMode launchMode) const
{
    const auto& abilityInfo = abilityRecord->GetAbilityInfo();
    return abilityInfo.launchMode == launchMode && abilityRequest.abilityInfo.name == abilityInfo.name &&
        abilityRequest.abilityInfo.bundleName == abilityInfo.bundleName &&
        abilityRequest.abilityInfo.moduleName == abilityInfo.moduleName &&
        abilityRequest.want.GetIntParam(DLP_INDEX, 0) == abilityRecord->GetAppIndex();
}

void UIAbilityLifecycleManager::ReportEventToSuspendManager(const AppExecFwk::AbilityInfo &abilityInfo) const
{
#ifdef EFFICIENCY_MANAGER_ENABLE
#endif // EFFICIENCY_MANAGER_ENABLE
}

void UIAbilityLifecycleManager::OnTimeOut(uint32_t msgId, int64_t abilityRecordId)
{
    HILOG_DEBUG("call, msgId is %{public}d", msgId);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    std::shared_ptr<AbilityRecord> abilityRecord;
    for (auto iter = sessionAbilityMap_.begin(); iter != sessionAbilityMap_.end(); iter++) {
        if (iter->second != nullptr && iter->second->GetAbilityRecordId() == abilityRecordId) {
            abilityRecord = iter->second;
            break;
        }
    }
    if (abilityRecord == nullptr) {
        HILOG_ERROR("failed, ability record is nullptr");
        return;
    }
    HILOG_DEBUG("call, msgId:%{public}d, name:%{public}s", msgId, abilityRecord->GetAbilityInfo().name.c_str());

    PrintTimeOutLog(abilityRecord, msgId);
    switch (msgId) {
        case AbilityManagerService::LOAD_TIMEOUT_MSG:
            HandleLoadTimeout(abilityRecord);
            break;
        case AbilityManagerService::FOREGROUND_TIMEOUT_MSG:
            HandleForegroundTimeout(abilityRecord);
            break;
        default:
            break;
    }
}

void UIAbilityLifecycleManager::SetRootSceneSession(const sptr<IRemoteObject> &rootSceneSession)
{
    HILOG_DEBUG("call");
    if (rootSceneSession == nullptr) {
        HILOG_ERROR("rootSceneSession is invalid.");
        return;
    }
    auto tmpSceneSession = iface_cast<Rosen::ISession>(rootSceneSession);
    auto descriptor = Str16ToStr8(tmpSceneSession->GetDescriptor());
    if (descriptor != "OHOS.ISession") {
        HILOG_ERROR("token's Descriptor: %{public}s", descriptor.c_str());
        return;
    }
    rootSceneSession_ = tmpSceneSession;
}

void UIAbilityLifecycleManager::NotifySCBToHandleException(const std::shared_ptr<AbilityRecord> &abilityRecord,
    int32_t errorCode, std::string errorReason)
{
    HILOG_DEBUG("call");
    if (abilityRecord == nullptr) {
        HILOG_ERROR("ability record is nullptr");
        return;
    }
    auto callerSessionInfo = abilityRecord->GetSessionInfo();
    CHECK_POINTER(callerSessionInfo);
    CHECK_POINTER(callerSessionInfo->sessionToken);
    auto callerSession = iface_cast<Rosen::ISession>(callerSessionInfo->sessionToken);
    HILOG_INFO("call notifySessionException");
    sptr<SessionInfo> info = abilityRecord->GetSessionInfo();
    info->errorCode = errorCode;
    info->errorReason = errorReason;
    callerSession->NotifySessionException(info);
    EraseAbilityRecord(abilityRecord);
}

void UIAbilityLifecycleManager::HandleLoadTimeout(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HILOG_DEBUG("call");
    if (abilityRecord == nullptr) {
        HILOG_ERROR("failed, ability record is nullptr");
        return;
    }
    NotifySCBToHandleException(abilityRecord,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_LOAD_TIMEOUT), "handleLoadTimeout");
    DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(abilityRecord->GetToken());
}

void UIAbilityLifecycleManager::HandleForegroundTimeout(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HILOG_DEBUG("call");
    if (abilityRecord == nullptr) {
        HILOG_ERROR("ability record is nullptr");
        return;
    }
    if (!abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING)) {
        HILOG_ERROR("this ability is not foregrounding state");
        return;
    }
    NotifySCBToHandleException(abilityRecord,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_FOREGROUND_TIMEOUT), "handleForegroundTimeout");
    DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(abilityRecord->GetToken());
    EraseSpecifiedAbilityRecord(abilityRecord);
}

void UIAbilityLifecycleManager::OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord)
{
    HILOG_DEBUG("call");
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("failed, ability record is nullptr");
        return;
    }
    terminateAbilityList_.push_back(abilityRecord);
    abilityRecord->SetAbilityState(AbilityState::TERMINATING);
    NotifySCBToHandleException(abilityRecord, static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_DIED),
        "onAbilityDied");
    DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(abilityRecord->GetToken());
    DispatchTerminate(abilityRecord);
    EraseSpecifiedAbilityRecord(abilityRecord);
}

void UIAbilityLifecycleManager::OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag)
{
    HILOG_DEBUG("call");
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (abilityQueue_.empty()) {
        return;
    }

    AbilityRequest abilityRequest = abilityQueue_.front();
    abilityQueue_.pop();
    if (abilityRequest.abilityInfo.launchMode != AppExecFwk::LaunchMode::SPECIFIED) {
        return;
    }
    auto callerAbility = GetAbilityRecordByToken(abilityRequest.callerToken);
    if (!flag.empty()) {
        abilityRequest.specifiedFlag = flag;
        auto persistentId = GetReusedSpecifiedPersistentId(abilityRequest);
        if (persistentId != 0) {
            auto abilityRecord = GetReusedSpecifiedAbility(want, flag);
            if (!abilityRecord) {
                return;
            }
            abilityRecord->SetWant(abilityRequest.want);
            abilityRecord->SetIsNewWant(true);
            UpdateAbilityRecordLaunchReason(abilityRequest, abilityRecord);
            if (callerAbility == nullptr) {
                callerAbility = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
            }
            MoveAbilityToFront(abilityRequest, abilityRecord, callerAbility);
            NotifyRestartSpecifiedAbility(abilityRequest, abilityRecord->GetToken());
            return;
        }
    }
    NotifyStartSpecifiedAbility(abilityRequest, want);
    StartAbilityBySpecifed(abilityRequest, callerAbility);
}

void UIAbilityLifecycleManager::StartSpecifiedAbilityBySCB(const Want &want, int32_t userId)
{
    HILOG_DEBUG("call");
    AbilityRequest abilityRequest;
    int result = DelayedSingleton<AbilityManagerService>::GetInstance()->GenerateAbilityRequest(
        want, DEFAULT_INVAL_VALUE, abilityRequest, nullptr, userId);
    if (result != ERR_OK) {
        HILOG_ERROR("cannot find generate ability request");
        return;
    }
    {
        std::lock_guard<ffrt::mutex> guard(sessionLock_);
        EnqueueAbilityToFront(abilityRequest);
    }
    DelayedSingleton<AppScheduler>::GetInstance()->StartSpecifiedAbility(
        abilityRequest.want, abilityRequest.abilityInfo);
}

std::shared_ptr<AbilityRecord> UIAbilityLifecycleManager::GetReusedSpecifiedAbility(const AAFwk::Want &want,
    const std::string &flag)
{
    auto element = want.GetElement();
    for (const auto& [first, second] : specifiedAbilityMap_) {
        if (flag == first.flag && element.GetAbilityName() == first.abilityName &&
            element.GetBundleName() == first.bundleName) {
            return second;
        }
    }
    return nullptr;
}

void UIAbilityLifecycleManager::EnqueueAbilityToFront(const AbilityRequest &abilityRequest)
{
    abilityQueue_.push(abilityRequest);
}

void UIAbilityLifecycleManager::NotifyRestartSpecifiedAbility(AbilityRequest &request,
    const sptr<IRemoteObject> &token)
{
    if (request.abilityInfoCallback == nullptr) {
        return;
    }
    sptr<AppExecFwk::IAbilityInfoCallback> abilityInfoCallback
        = iface_cast<AppExecFwk::IAbilityInfoCallback> (request.abilityInfoCallback);
    if (abilityInfoCallback != nullptr) {
        HILOG_DEBUG("%{public}s called.", __func__);
        abilityInfoCallback->NotifyRestartSpecifiedAbility(token);
    }
}

void UIAbilityLifecycleManager::NotifyStartSpecifiedAbility(AbilityRequest &abilityRequest, const AAFwk::Want &want)
{
    if (abilityRequest.abilityInfoCallback == nullptr) {
        return;
    }

    sptr<AppExecFwk::IAbilityInfoCallback> abilityInfoCallback
        = iface_cast<AppExecFwk::IAbilityInfoCallback> (abilityRequest.abilityInfoCallback);
    if (abilityInfoCallback != nullptr) {
        Want newWant = want;
        int32_t type = static_cast<int32_t>(abilityRequest.abilityInfo.type);
        newWant.SetParam("abilityType", type);
        sptr<Want> extraParam = new (std::nothrow) Want();
        abilityInfoCallback->NotifyStartSpecifiedAbility(abilityRequest.callerToken, newWant,
            abilityRequest.requestCode, extraParam);
        int32_t procCode = extraParam->GetIntParam(Want::PARAM_RESV_REQUEST_PROC_CODE, 0);
        if (procCode != 0) {
            abilityRequest.want.SetParam(Want::PARAM_RESV_REQUEST_PROC_CODE, procCode);
        }
        int32_t tokenCode = extraParam->GetIntParam(Want::PARAM_RESV_REQUEST_TOKEN_CODE, 0);
        if (tokenCode != 0) {
            abilityRequest.want.SetParam(Want::PARAM_RESV_REQUEST_TOKEN_CODE, tokenCode);
        }
    }
}

int UIAbilityLifecycleManager::MoveAbilityToFront(const AbilityRequest &abilityRequest,
    const std::shared_ptr<AbilityRecord> &abilityRecord, std::shared_ptr<AbilityRecord> callerAbility,
    std::shared_ptr<StartOptions> startOptions)
{
    HILOG_DEBUG("call");
    if (!abilityRecord) {
        HILOG_ERROR("get target ability record failed");
        return ERR_INVALID_VALUE;
    }
    sptr<SessionInfo> sessionInfo = abilityRecord->GetSessionInfo();
    sessionInfo->want = abilityRequest.want;
    SendSessionInfoToSCB(callerAbility, sessionInfo);
    abilityRecord->RemoveWindowMode();
    if (startOptions != nullptr) {
        abilityRecord->SetWindowMode(startOptions->GetWindowMode());
    }
    return ERR_OK;
}

int UIAbilityLifecycleManager::SendSessionInfoToSCB(std::shared_ptr<AbilityRecord> &callerAbility,
    sptr<SessionInfo> &sessionInfo)
{
    HILOG_DEBUG("call");
    if (callerAbility != nullptr) {
        auto callerSessionInfo = callerAbility->GetSessionInfo();
        if (callerSessionInfo != nullptr && callerSessionInfo->sessionToken != nullptr) {
            auto callerSession = iface_cast<Rosen::ISession>(callerSessionInfo->sessionToken);
            callerSession->PendingSessionActivation(sessionInfo);
        } else {
            CHECK_POINTER_AND_RETURN(rootSceneSession_, ERR_INVALID_VALUE);
            rootSceneSession_->PendingSessionActivation(sessionInfo);
        }
    } else {
        CHECK_POINTER_AND_RETURN(rootSceneSession_, ERR_INVALID_VALUE);
        rootSceneSession_->PendingSessionActivation(sessionInfo);
    }
    return ERR_OK;
}

int UIAbilityLifecycleManager::StartAbilityBySpecifed(const AbilityRequest &abilityRequest,
    std::shared_ptr<AbilityRecord> &callerAbility)
{
    HILOG_DEBUG("call");
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    sessionInfo->callerToken = abilityRequest.callerToken;
    sessionInfo->want = abilityRequest.want;
    sessionInfo->requestCode = abilityRequest.requestCode;
    SpecifiedInfo specifiedInfo;
    specifiedInfo.abilityName = abilityRequest.abilityInfo.name;
    specifiedInfo.bundleName = abilityRequest.abilityInfo.bundleName;
    specifiedInfo.flag = abilityRequest.specifiedFlag;
    specifiedInfoQueue_.push(specifiedInfo);

    SendSessionInfoToSCB(callerAbility, sessionInfo);
    return ERR_OK;
}

void UIAbilityLifecycleManager::CallRequestDone(const std::shared_ptr<AbilityRecord> &abilityRecord,
    const sptr<IRemoteObject> &callStub)
{
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("ability record is null.");
        return;
    }
    if (callStub == nullptr) {
        HILOG_ERROR("call stub is null.");
        return;
    }
    abilityRecord->CallRequestDone(callStub);
}

int UIAbilityLifecycleManager::ReleaseCallLocked(
    const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element)
{
    HILOG_DEBUG("release call ability.");

    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);

    std::lock_guard<ffrt::mutex> guard(sessionLock_);

    auto abilityRecords = GetAbilityRecordsByName(element);
    auto isExist = [connect] (const std::shared_ptr<AbilityRecord> &abilityRecord) {
        return abilityRecord->IsExistConnection(connect);
    };
    auto findRecord = std::find_if(abilityRecords.begin(), abilityRecords.end(), isExist);
    if (findRecord == abilityRecords.end()) {
        HILOG_ERROR("not found ability record by callback");
        return RELEASE_CALL_ABILITY_INNER_ERR;
    }
    auto abilityRecord = *findRecord;
    CHECK_POINTER_AND_RETURN(abilityRecord, RELEASE_CALL_ABILITY_INNER_ERR);

    if (!abilityRecord->ReleaseCall(connect)) {
        HILOG_ERROR("ability release call record failed.");
        return RELEASE_CALL_ABILITY_INNER_ERR;
    }
    return ERR_OK;
}

std::vector<std::shared_ptr<AbilityRecord>> UIAbilityLifecycleManager::GetAbilityRecordsByName(
    const AppExecFwk::ElementName &element)
{
    std::vector<std::shared_ptr<AbilityRecord>> records;
    for (const auto& [first, second] : sessionAbilityMap_) {
        auto &abilityInfo = second->GetAbilityInfo();
        AppExecFwk::ElementName localElement(abilityInfo.deviceId, abilityInfo.bundleName,
            abilityInfo.name, abilityInfo.moduleName);
        AppExecFwk::ElementName localElementNoModuleName(abilityInfo.deviceId,
            abilityInfo.bundleName, abilityInfo.name);
        if (localElement == element || localElementNoModuleName == element) {
            HILOG_DEBUG("find element %{public}s", localElement.GetURI().c_str());
            records.push_back(second);
        }
    }
    return records;
}
}  // namespace AAFwk
}  // namespace OHOS