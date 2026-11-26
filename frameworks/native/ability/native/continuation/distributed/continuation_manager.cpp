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

#include "continuation_manager.h"

#include "ability.h"
#include "ability_continuation_interface.h"
#include "ability_manager_client.h"
#include "bool_wrapper.h"
#include "continuation_handler.h"
#include "distributed_client.h"
#include "hilog_tag_wrapper.h"
#include "string_ex.h"
#include "string_wrapper.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
const int ContinuationManager::TIMEOUT_MS_WAIT_DMS_NOTIFY_CONTINUATION_COMPLETE = 25000;
const int ContinuationManager::TIMEOUT_MS_WAIT_REMOTE_NOTIFY_BACK = 6000;
const std::string PAGE_STACK_PROPERTY_NAME = "pageStack";
const std::string SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME = "ohos.extra.param.key.supportContinuePageStack";
const int32_t CONTINUE_ABILITY_REJECTED = 29360197;
const int32_t CONTINUE_SAVE_DATA_FAILED = 29360198;
const int32_t CONTINUE_ON_CONTINUE_FAILED = 29360199;
const int32_t CONTINUE_ON_CONTINUE_HANDLE_FAILED = 29360300;
const int32_t CONTINUE_ON_CONTINUE_MISMATCH = 29360204;
#ifdef SUPPORT_GRAPHICS
const int32_t CONTINUE_GET_CONTENT_FAILED = 29360200;
#endif
ContinuationManager::ContinuationManager()
{
    progressState_ = ProgressState::INITIAL;
}

bool ContinuationManager::Init(const std::shared_ptr<Ability> &ability, const sptr<IRemoteObject> &continueToken,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<ContinuationHandler> &continuationHandler)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "begin");
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null ability");
        return false;
    }
    ability_ = ability;

    std::shared_ptr<Ability> abilityTmp = nullptr;
    abilityTmp = ability_.lock();
    if (abilityTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null ability");
        return false;
    }

    if (abilityTmp->GetAbilityInfo() == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null abilityInfo");
        return false;
    }
    abilityInfo_ = abilityTmp->GetAbilityInfo();

    if (continueToken == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null continueToken");
        return false;
    }
    continueToken_ = continueToken;

    continuationHandler_ = continuationHandler;
    return true;
}

ContinuationState ContinuationManager::GetContinuationState()
{
    return continuationState_;
}

std::string ContinuationManager::GetOriginalDeviceId()
{
    return originalDeviceId_;
}

void ContinuationManager::ContinueAbilityWithStack(const std::string &deviceId, uint32_t versionCode)
{
    HandleContinueAbilityWithStack(deviceId, versionCode);
}

bool ContinuationManager::HandleContinueAbilityWithStack(const std::string &deviceId, uint32_t versionCode)
{
    if (!CheckAbilityToken()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "checkAbilityToken failed");
        return false;
    }

    sptr<IRemoteObject> continueToken = continueToken_;
    std::shared_ptr<ContinuationHandler> continuationHandler = continuationHandler_.lock();
    if (continuationHandler == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null continuationHandler");
        return false;
    }

    InitMainHandlerIfNeed();
    auto task = [continuationHandler, continueToken, deviceId, versionCode]() {
        continuationHandler->HandleStartContinuationWithStack(continueToken, deviceId, versionCode);
    };
    if (!mainHandler_->PostTask(task)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "postTask failed");
        return false;
    }
    return true;
}

int32_t ContinuationManager::OnStartAndSaveData(WantParams &wantParams)
{
    std::shared_ptr<Ability> ability = nullptr;
    ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null ability");
        return ERR_INVALID_VALUE;
    }

    if (!ability->OnStartContinuation()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability rejected");
        return CONTINUE_ABILITY_REJECTED;
    }
    if (!ability->OnSaveData(wantParams)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "SaveData failed");
        return CONTINUE_SAVE_DATA_FAILED;
    }
    return ERR_OK;
}

bool ContinuationManager::IsContinuePageStack(const WantParams &wantParams)
{
    auto value = wantParams.GetParam(SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME);
    IBoolean *ao = IBoolean::Query(value);
    if (ao != nullptr) {
        return AAFwk::Boolean::Unbox(ao);
    }
    return true;
}

int32_t ContinuationManager::OnContinueAndGetContent(WantParams &wantParams)
{
    std::shared_ptr<Ability> ability = nullptr;
    ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null ability");
        return ERR_INVALID_VALUE;
    }

    int32_t status = ability->OnContinue(wantParams);
    switch (status) {
        case OnContinueResult::AGREE:
#ifdef SUPPORT_GRAPHICS
            if (IsContinuePageStack(wantParams)) {
                bool ret = GetContentInfo(wantParams);
                if (!ret) {
                    TAG_LOGE(AAFwkTag::CONTINUATION, "GetContentInfo failed");
                    return CONTINUE_GET_CONTENT_FAILED;
                }
            }
#endif
            return ERR_OK;
        case OnContinueResult::REJECT:
            TAG_LOGE(AAFwkTag::CONTINUATION, "app reject");
            return CONTINUE_ON_CONTINUE_FAILED;
        case OnContinueResult::MISMATCH:
            TAG_LOGE(AAFwkTag::CONTINUATION, "version mismatch");
            return CONTINUE_ON_CONTINUE_MISMATCH;
        case OnContinueResult::ON_CONTINUE_ERR:
            TAG_LOGE(AAFwkTag::CONTINUATION, "OnContinue handle failed");
            return CONTINUE_ON_CONTINUE_HANDLE_FAILED;
        default:
            TAG_LOGE(AAFwkTag::CONTINUATION, "invalid status");
            return CONTINUE_ON_CONTINUE_HANDLE_FAILED;
    }
}

int32_t ContinuationManager::OnContinue(WantParams &wantParams)
{
    auto ability = ability_.lock();
    auto abilityInfo = abilityInfo_.lock();
    if (ability == nullptr || abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null ability or abilityInfo");
        return ERR_INVALID_VALUE;
    }

    bool stageBased = abilityInfo->isStageBasedModel;
    if (!stageBased) {
        return OnStartAndSaveData(wantParams);
    } else {
        return OnContinueAndGetContent(wantParams);
    }
}

#ifdef SUPPORT_SCREEN
bool ContinuationManager::GetContentInfo(WantParams &wantParams)
{
    std::shared_ptr<Ability> ability = nullptr;
    ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null ability");
        return false;
    }

    std::string pageStack = ability->GetContentInfo();
    if (pageStack.empty()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "GetContentInfo failed");
        return false;
    }
    wantParams.SetParam(PAGE_STACK_PROPERTY_NAME, String::Box(pageStack));

    return true;
}
#endif

void ContinuationManager::ContinueAbility(bool reversible, const std::string &deviceId)
{
    if (CheckContinuationIllegal()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ability not available to continueAbility");
        return;
    }

    if (progressState_ != ProgressState::INITIAL) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "progressState_: %{public}d", progressState_);
        return;
    }

    if (continuationState_ != ContinuationState::LOCAL_RUNNING) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "current state: %{public}d", continuationState_);
        return;
    }

    if (HandleContinueAbility(reversible, deviceId)) {
        reversible_ = reversible;
        ChangeProcessState(ProgressState::WAITING_SCHEDULE);
    }
}

bool ContinuationManager::ReverseContinueAbility()
{
    if (progressState_ != ProgressState::INITIAL) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "progressState_: %{public}d", progressState_);
        return false;
    }

    if (continuationState_ != ContinuationState::REMOTE_RUNNING) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "continuation state: %{public}d", continuationState_);
        return false;
    }

    std::shared_ptr<ContinuationHandler> continuationHandler = continuationHandler_.lock();
    if (continuationHandler == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null continuationHandler_");
        return false;
    }

    bool requestSuccess = continuationHandler->ReverseContinueAbility();
    if (requestSuccess) {
        ChangeProcessState(ProgressState::WAITING_SCHEDULE);
        RestoreStateWhenTimeout(TIMEOUT_MS_WAIT_REMOTE_NOTIFY_BACK, ProgressState::WAITING_SCHEDULE);
    }
    return requestSuccess;
}

bool ContinuationManager::StartContinuation()
{
    ChangeProcessState(ProgressState::IN_PROGRESS);
    bool result = DoScheduleStartContinuation();
    if (!result) {
        ChangeProcessState(ProgressState::INITIAL);
    }
    return result;
}

bool ContinuationManager::SaveData(WantParams &saveData)
{
    bool result = DoScheduleSaveData(saveData);
    if (!result) {
        ChangeProcessState(ProgressState::INITIAL);
    } else {
        RestoreStateWhenTimeout(TIMEOUT_MS_WAIT_DMS_NOTIFY_CONTINUATION_COMPLETE, ProgressState::IN_PROGRESS);
    }
    return result;
}

bool ContinuationManager::RestoreData(
    const WantParams &restoreData, bool reversible, const std::string &originalDeviceId)
{
    ChangeProcessState(ProgressState::IN_PROGRESS);
    bool result = DoScheduleRestoreData(restoreData);
    if (reversible) {
        continuationState_ = ContinuationState::REPLICA_RUNNING;
    }
    originalDeviceId_ = originalDeviceId;
    ChangeProcessState(ProgressState::INITIAL);
    return result;
}

void ContinuationManager::NotifyCompleteContinuation(
    const std::string &originDeviceId, int sessionId, bool success, const sptr<IRemoteObject> &reverseScheduler)
{
    AAFwk::AbilityManagerClient::GetInstance()->NotifyCompleteContinuation(
        originDeviceId, sessionId, success);
}

void ContinuationManager::CompleteContinuation(int result)
{
    if (CheckContinuationIllegal()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ability not available to complete continuation");
        return;
    }

    std::shared_ptr<Ability> ability = nullptr;
    ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null ability");
        return;
    }

    if (result == 0 && reversible_) {
        continuationState_ = ContinuationState::REMOTE_RUNNING;
    }
    ChangeProcessState(ProgressState::INITIAL);

    ability->OnCompleteContinuation(result);

    if (!reversible_) {
        ability->TerminateAbility();
    }
}

bool ContinuationManager::RestoreFromRemote(const WantParams &restoreData)
{
    ChangeProcessState(ProgressState::IN_PROGRESS);
    bool result = DoRestoreFromRemote(restoreData);
    /*
     * No matter what the result is, we should reset the status. Because even it fail, we can do
     * nothing but let the user send another reverse continuation request again.
     */
    ChangeProcessState(ProgressState::INITIAL);
    if (result) {
        continuationState_ = ContinuationState::LOCAL_RUNNING;
    }
    return result;
}

bool ContinuationManager::NotifyRemoteTerminated()
{
    continuationState_ = ContinuationState::LOCAL_RUNNING;
    ChangeProcessState(ProgressState::INITIAL);

    std::shared_ptr<Ability> ability = nullptr;
    ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null ability");
        return false;
    }

    ability->OnRemoteTerminated();
    return true;
}

bool ContinuationManager::CheckContinuationIllegal()
{
    std::shared_ptr<Ability> ability = nullptr;
    ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null ability");
        return false;
    }

    if (ability->GetState() >= AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "state wrong: %{public}d", ability->GetState());
        return true;
    }
    return false;
}

bool ContinuationManager::HandleContinueAbility(bool reversible, const std::string &deviceId)
{
    if (!CheckAbilityToken()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "CheckAbilityToken failed");
        return false;
    }

    sptr<IRemoteObject> continueToken = continueToken_;
    std::shared_ptr<ContinuationHandler> continuationHandler = continuationHandler_.lock();
    if (continuationHandler == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null continuationHandler");
        return false;
    }
    continuationHandler->SetReversible(reversible);

    InitMainHandlerIfNeed();
    auto task = [continuationHandler, continueToken, deviceId]() {
        continuationHandler->HandleStartContinuation(continueToken, deviceId);
    };
    if (!mainHandler_->PostTask(task)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "PostTask failed");
        return false;
    }
    return true;
}

ContinuationManager::ProgressState ContinuationManager::GetProcessState()
{
    return progressState_;
}

void ContinuationManager::ChangeProcessState(const ProgressState &newState)
{
    progressState_ = newState;
}


void ContinuationManager::ChangeProcessStateToInit()
{
    if (mainHandler_ != nullptr) {
        mainHandler_->RemoveTask("Restore_State_When_Timeout");
        TAG_LOGI(AAFwkTag::CONTINUATION, "Restore_State_When_Timeout task removed");
    }
    ChangeProcessState(ProgressState::INITIAL);
}

void ContinuationManager::RestoreStateWhenTimeout(long timeoutInMs, const ProgressState &preState)
{
    InitMainHandlerIfNeed();

    auto timeoutTask = [continuationManager = shared_from_this(), preState]() {
        TAG_LOGI(AAFwkTag::CONTINUATION, "preState = %{public}d, currentState = %{public}d",
            preState, continuationManager->GetProcessState());
        if (preState == continuationManager->GetProcessState()) {
            continuationManager->ChangeProcessState(ProgressState::INITIAL);
        }
    };
    mainHandler_->PostTask(timeoutTask, "Restore_State_When_Timeout", timeoutInMs);
}

void ContinuationManager::InitMainHandlerIfNeed()
{
    if (mainHandler_ == nullptr) {
        TAG_LOGW(AAFwkTag::CONTINUATION, "null mainHandler");
        std::lock_guard<std::mutex> lock_l(lock_);
        if ((mainHandler_ == nullptr) && (EventRunner::GetMainEventRunner() != nullptr)) {
            mainHandler_ = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
        }
    }
}

bool ContinuationManager::CheckAbilityToken()
{
    if (continueToken_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null continueToken_");
        return false;
    }
    return true;
}

void ContinuationManager::CheckDmsInterfaceResult(int result, const std::string &interfaceName)
{
}

bool ContinuationManager::DoScheduleStartContinuation()
{
    if (CheckContinuationIllegal()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ability not available to startContinuation");
        return false;
    }

    std::shared_ptr<Ability> ability = nullptr;
    ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null ability");
        return false;
    }
    if (!ability->OnStartContinuation()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "StartContinuation failed");
        return false;
    }
    return true;
}

bool ContinuationManager::DoScheduleSaveData(WantParams &saveData)
{
    if (CheckContinuationIllegal()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability not available to save data");
        return false;
    }

    std::shared_ptr<Ability> ability = nullptr;
    ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null ability");
        return false;
    }

    WantParams abilitySaveData;
    bool ret = ability->OnSaveData(abilitySaveData);
    for (std::string key : abilitySaveData.KeySet()) {
        saveData.SetParam(key, abilitySaveData.GetParam(key).GetRefPtr());
    }

    if (!ret) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability save data failed");
    }
    return ret;
}

bool ContinuationManager::DoScheduleRestoreData(const WantParams &restoreData)
{
    if (CheckContinuationIllegal()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability not available to restore data");
        return false;
    }

    std::shared_ptr<Ability> ability = nullptr;
    ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null ability");
        return false;
    }

    WantParams abilityRestoreData;
    for (std::string key : restoreData.KeySet()) {
        abilityRestoreData.SetParam(key, restoreData.GetParam(key).GetRefPtr());
    }

    bool ret = ability->OnRestoreData(abilityRestoreData);
    if (!ret) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ability restore data failed");
    }
    return ret;
}

bool ContinuationManager::DoRestoreFromRemote(const WantParams &restoreData)
{
    std::shared_ptr<Ability> ability = nullptr;
    ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null ability");
        return false;
    }

    WantParams abilityRestoreData;
    for (std::string key : restoreData.KeySet()) {
        abilityRestoreData.SetParam(key, restoreData.GetParam(key).GetRefPtr());
    }

    bool ret = ability->OnRestoreData(abilityRestoreData);
    if (!ret) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ability restore data failed");
    }
    return ret;
}
}  // namespace AppExecFwk
}  // namespace OHOS
