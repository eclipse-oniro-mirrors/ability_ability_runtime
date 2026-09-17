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

#include "continuation_manager_stage.h"

#include "ability_continuation_interface.h"
#include "ability_manager_client.h"
#ifdef NO_RUNTIME_EMULATOR
#include "app_event.h"
#include "app_event_processor_mgr.h"
#endif
#include "bool_wrapper.h"
#include "continuation_handler.h"
#include "distributed_client.h"
#include "hilog_tag_wrapper.h"
#include "operation_builder.h"
#include "string_ex.h"
#include "string_wrapper.h"
#include "ui_ability.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
#ifdef NO_RUNTIME_EMULATOR
using namespace OHOS::HiviewDFX;
#endif
namespace {
constexpr int TIMEOUT_MS_WAIT_DMS_NOTIFY_CONTINUATION_COMPLETE = 25000;
constexpr int TIMEOUT_MS_WAIT_REMOTE_NOTIFY_BACK = 6000;
const std::string PAGE_STACK_PROPERTY_NAME = "pageStack";
const std::string SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME = "ohos.extra.param.key.supportContinuePageStack";
const int32_t CONTINUE_ABILITY_REJECTED = 29360197;
const int32_t CONTINUE_SAVE_DATA_FAILED = 29360198;
const int32_t CONTINUE_ON_CONTINUE_FAILED = 29360199;
const int32_t CONTINUE_ON_CONTINUE_HANDLE_FAILED = 29360300;
const int32_t CONTINUE_ON_CONTINUE_MISMATCH = 29360204;
#ifdef NO_RUNTIME_EMULATOR
constexpr int32_t TRIGGER_COND_TIMEOUT = 90;
constexpr int32_t TRIGGER_COND_ROW = 30;
constexpr int32_t EVENT_RESULT_SUCCESS = 0;
constexpr int32_t EVENT_RESULT_FAIL = 1;
#endif
#ifdef SUPPORT_GRAPHICS
const int32_t CONTINUE_GET_CONTENT_FAILED = 29360200;
#endif
}

ContinuationManagerStage::ContinuationManagerStage() : progressState_(ProgressState::INITIAL) {}

bool ContinuationManagerStage::Init(const std::shared_ptr<AbilityRuntime::UIAbility> &ability,
    const sptr<IRemoteObject> &continueToken, const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<ContinuationHandlerStage> &continuationHandler)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null ability");
        return false;
    }
    ability_ = ability;

    std::shared_ptr<AbilityRuntime::UIAbility> abilityTmp = nullptr;
    abilityTmp = ability_.lock();
    if (abilityTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null abilityTmp");
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

ContinuationState ContinuationManagerStage::GetContinuationState()
{
    return continuationState_;
}

std::string ContinuationManagerStage::GetOriginalDeviceId()
{
    return originalDeviceId_;
}

void ContinuationManagerStage::ContinueAbilityWithStack(const std::string &deviceId, uint32_t versionCode)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
    HandleContinueAbilityWithStack(deviceId, versionCode);
}

bool ContinuationManagerStage::HandleContinueAbilityWithStack(const std::string &deviceId, uint32_t versionCode)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
    if (!CheckAbilityToken()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "CheckAbilityToken failed");
        return false;
    }

    InitMainHandlerIfNeed();
    wptr<IRemoteObject> continueTokenWeak(continueToken_);
    auto task = [continuationHandlerWeak = continuationHandler_, continueTokenWeak, deviceId, versionCode]() {
        auto continuationHandler = continuationHandlerWeak.lock();
        if (continuationHandler == nullptr) {
            TAG_LOGE(AAFwkTag::CONTINUATION, "null continuationHandler");
            return;
        }

        auto continueToken = continueTokenWeak.promote();
        if (continueToken == nullptr) {
            TAG_LOGE(AAFwkTag::CONTINUATION, "null continueToken");
            return;
        }
        continuationHandler->HandleStartContinuationWithStack(continueToken, deviceId, versionCode);
    };
    if (!mainHandler_->PostTask(task)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "PostTask failed");
        return false;
    }
    return true;
}

int32_t ContinuationManagerStage::OnStartAndSaveData(WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
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

bool ContinuationManagerStage::IsContinuePageStack(const WantParams &wantParams)
{
    auto value = wantParams.GetParam(SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME);
    IBoolean *ao = IBoolean::Query(value);
    if (ao != nullptr) {
        return AAFwk::Boolean::Unbox(ao);
    }
    return true;
}

int32_t ContinuationManagerStage::OnContinueAndGetContent(WantParams &wantParams, bool &isAsyncOnContinue,
    const AbilityInfo &abilityInfo)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null ability");
        return ERR_INVALID_VALUE;
    }
    int32_t status = ability->OnContinue(wantParams, isAsyncOnContinue, abilityInfo);
    switch (status) {
        case OnContinueResult::AGREE:
#ifdef SUPPORT_GRAPHICS
            if (!isAsyncOnContinue && IsContinuePageStack(wantParams)) {
                bool ret = GetContentInfo(wantParams);
                if (!ret) {
                    TAG_LOGE(AAFwkTag::CONTINUATION, "GetContentInfo failed!");
                    return CONTINUE_GET_CONTENT_FAILED;
                }
            }
#endif
            TAG_LOGI(AAFwkTag::CONTINUATION, "GetContentInfo has been successfully");
            return ERR_OK;
        case OnContinueResult::REJECT:
            TAG_LOGE(AAFwkTag::CONTINUATION, "app reject");
            return CONTINUE_ON_CONTINUE_FAILED;
        case OnContinueResult::MISMATCH:
            TAG_LOGE(AAFwkTag::CONTINUATION, "OnContinue version mismatch.");
            return CONTINUE_ON_CONTINUE_MISMATCH;
        case OnContinueResult::ON_CONTINUE_ERR:
            TAG_LOGE(AAFwkTag::CONTINUATION, "OnContinue handle failed");
            return CONTINUE_ON_CONTINUE_HANDLE_FAILED;
        default:
            TAG_LOGE(AAFwkTag::CONTINUATION, "invalid status");
            return CONTINUE_ON_CONTINUE_HANDLE_FAILED;
    }
}

#ifdef NO_RUNTIME_EMULATOR
static int64_t AddProcessor()
{
    HiAppEvent::ReportConfig config;
    config.name = "ha_app_event";
    config.appId = "com_hmos_sdk_ocg";
    config.routeInfo = "AUTO";
    config.triggerCond.timeout = TRIGGER_COND_TIMEOUT;
    config.triggerCond.row = TRIGGER_COND_ROW;
    config.eventConfigs.clear();
    {
        HiAppEvent::EventConfig event1;
        event1.domain = "api_diagnostic";
        event1.name = "api_exec_end";
        event1.isRealTime = false;
        config.eventConfigs.push_back(event1);
    }
    {
        HiAppEvent::EventConfig event2;
        event2.domain = "api_diagnostic";
        event2.name = "api_called_stat";
        event2.isRealTime = true;
        config.eventConfigs.push_back(event2);
    }
    {
        HiAppEvent::EventConfig event3;
        event3.domain = "api_diagnostic";
        event3.name = "api_called_stat_cnt";
        event3.isRealTime = true;
        config.eventConfigs.push_back(event3);
    }
    return HiAppEvent::AppEventProcessorMgr::AddProcessor(config);
}

static void WriteEndEvent(const std::string& transId, const int result, const int errCode, const time_t beginTime,
    int64_t processorId)
{
    HiAppEvent::Event event("api_diagnostic", "api_exec_end", HiAppEvent::BEHAVIOR);
    event.AddParam("transId", transId);
    event.AddParam("result", result);
    event.AddParam("error_code", errCode);
    event.AddParam("api_name", std::string("onContinue"));
    event.AddParam("sdk_name", std::string("AbilityKit"));
    event.AddParam("begin_time", beginTime);
    event.AddParam("end_time", time(nullptr));
    if (processorId > 0) {
        Write(event);
    }
}
#endif

int32_t ContinuationManagerStage::OnContinue(WantParams &wantParams, bool &isAsyncOnContinue,
    const AbilityInfo &tmpAbilityInfo)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
#ifdef NO_RUNTIME_EMULATOR
    int64_t processorId = -1;
    processorId = AddProcessor();
    TAG_LOGI(AAFwkTag::CONTINUATION, "Add processor start.Processor id is %{public}" PRId64, processorId);
    time_t beginTime = time(nullptr);
    std::string transId = std::string("transId_") + std::to_string(std::rand());
#endif
    int32_t ret = 0;
    auto ability = ability_.lock();
    auto abilityInfo = abilityInfo_.lock();
    if (ability == nullptr || abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null ability or abilityInfo");
#ifdef NO_RUNTIME_EMULATOR
        WriteEndEvent(transId, EVENT_RESULT_FAIL, ERR_INVALID_VALUE, beginTime, processorId);
#endif
        return ERR_INVALID_VALUE;
    }

    bool stageBased = abilityInfo->isStageBasedModel;
    if (!stageBased) {
        ret = OnStartAndSaveData(wantParams);
#ifdef NO_RUNTIME_EMULATOR
        int32_t result = (ret == ERR_OK) ? EVENT_RESULT_SUCCESS : EVENT_RESULT_FAIL;
        WriteEndEvent(transId, result, ret, beginTime, processorId);
#endif
        return ret;
    }
    ret = OnContinueAndGetContent(wantParams, isAsyncOnContinue, tmpAbilityInfo);
#ifdef NO_RUNTIME_EMULATOR
    int32_t result = (ret == ERR_OK) ? EVENT_RESULT_SUCCESS : EVENT_RESULT_FAIL;
    WriteEndEvent(transId, result, ret, beginTime, processorId);
#endif
    return ret;
}

#ifdef SUPPORT_SCREEN
bool ContinuationManagerStage::GetContentInfo(WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null ability");
        return false;
    }

    std::string pageStack = ability->GetContentInfo();
    if (pageStack.empty()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "GetContentInfo failed");
        return false;
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "pageStack: %{public}s", pageStack.c_str());
    wantParams.SetParam(PAGE_STACK_PROPERTY_NAME, String::Box(pageStack));
    return true;
}
#endif

void ContinuationManagerStage::ContinueAbility(bool reversible, const std::string &deviceId)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
    if (CheckContinuationIllegal()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability not available to continueAbility");
        return;
    }

    if (progressState_ != ProgressState::INITIAL) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "progressState_: %{public}d", progressState_);
        return;
    }

    if (continuationState_ != ContinuationState::LOCAL_RUNNING) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "continuation state %{public}d", continuationState_);
        return;
    }

    if (HandleContinueAbility(reversible, deviceId)) {
        reversible_ = reversible;
        ChangeProcessState(ProgressState::WAITING_SCHEDULE);
    }
}

bool ContinuationManagerStage::ReverseContinueAbility()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "begin");
    if (progressState_ != ProgressState::INITIAL) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "progressState_: %{public}d", progressState_);
        return false;
    }

    if (continuationState_ != ContinuationState::REMOTE_RUNNING) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "continuation state: %{public}d", continuationState_);
        return false;
    }

    std::shared_ptr<ContinuationHandlerStage> continuationHandler = continuationHandler_.lock();
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

bool ContinuationManagerStage::StartContinuation()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "begin");
    ChangeProcessState(ProgressState::IN_PROGRESS);
    bool result = DoScheduleStartContinuation();
    if (!result) {
        ChangeProcessState(ProgressState::INITIAL);
    }
    return result;
}

bool ContinuationManagerStage::SaveData(WantParams &saveData)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "begin");
    bool result = DoScheduleSaveData(saveData);
    if (!result) {
        ChangeProcessState(ProgressState::INITIAL);
    } else {
        RestoreStateWhenTimeout(TIMEOUT_MS_WAIT_DMS_NOTIFY_CONTINUATION_COMPLETE, ProgressState::IN_PROGRESS);
    }
    return result;
}

bool ContinuationManagerStage::RestoreData(
    const WantParams &restoreData, bool reversible, const std::string &originalDeviceId)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
    ChangeProcessState(ProgressState::IN_PROGRESS);
    bool result = DoScheduleRestoreData(restoreData);
    if (reversible) {
        continuationState_ = ContinuationState::REPLICA_RUNNING;
    }
    originalDeviceId_ = originalDeviceId;
    ChangeProcessState(ProgressState::INITIAL);
    return result;
}

void ContinuationManagerStage::NotifyCompleteContinuation(
    const std::string &originDeviceId, int sessionId, bool success, const sptr<IRemoteObject> &reverseScheduler)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
    AAFwk::AbilityManagerClient::GetInstance()->NotifyCompleteContinuation(originDeviceId, sessionId, success);
}

void ContinuationManagerStage::CompleteContinuation(int result)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
    if (CheckContinuationIllegal()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability not available to complete continuation");
        return;
    }

    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
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

bool ContinuationManagerStage::RestoreFromRemote(const WantParams &restoreData)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
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

bool ContinuationManagerStage::NotifyRemoteTerminated()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
    continuationState_ = ContinuationState::LOCAL_RUNNING;
    ChangeProcessState(ProgressState::INITIAL);

    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null ability");
        return false;
    }

    ability->OnRemoteTerminated();
    return true;
}

bool ContinuationManagerStage::CheckContinuationIllegal()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null ability");
        return false;
    }

    if (ability->GetState() >= AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "state is wrong: %{public}d.", ability->GetState());
        return true;
    }
    return false;
}

bool ContinuationManagerStage::HandleContinueAbility(bool reversible, const std::string &deviceId)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");

    if (!CheckAbilityToken()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "CheckAbilityToken failed");
        return false;
    }

    std::shared_ptr<ContinuationHandlerStage> continuationHandler = continuationHandler_.lock();
    if (continuationHandler == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null continuationHandler");
        return false;
    }
    continuationHandler->SetReversible(reversible);

    InitMainHandlerIfNeed();
    if (mainHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null mainHandler_");
        return false;
    }
    wptr<IRemoteObject> continueTokeWeak(continueToken_);
    auto task = [continuationHandlerWeak = continuationHandler_, continueTokeWeak, deviceId]() {
        auto continuationHandler = continuationHandlerWeak.lock();
        if (continuationHandler == nullptr) {
            TAG_LOGE(AAFwkTag::CONTINUATION, "null continuationHandler");
            return;
        }

        auto continueToken = continueTokeWeak.promote();
        if (continueToken == nullptr) {
            TAG_LOGE(AAFwkTag::CONTINUATION, "null continueToken");
            return;
        }
        continuationHandler->HandleStartContinuation(continueToken, deviceId);
    };
    if (!mainHandler_->PostTask(task)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "PostTask failed");
        return false;
    }

    return true;
}

ContinuationManagerStage::ProgressState ContinuationManagerStage::GetProcessState()
{
    return progressState_;
}

void ContinuationManagerStage::ChangeProcessState(const ProgressState &newState)
{
    progressState_ = newState;
}

void ContinuationManagerStage::ChangeProcessStateToInit()
{
    if (mainHandler_ != nullptr) {
        mainHandler_->RemoveTask("Restore_State_When_Timeout");
        TAG_LOGD(AAFwkTag::CONTINUATION, "Restore_State_When_Timeout task removed");
    }
    ChangeProcessState(ProgressState::INITIAL);
}

void ContinuationManagerStage::RestoreStateWhenTimeout(long timeoutInMs, const ProgressState &preState)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
    InitMainHandlerIfNeed();
    if (mainHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null mainHandler_");
        return;
    }

    auto timeoutTask = [continuationManager = shared_from_this(), preState]() {
        TAG_LOGD(AAFwkTag::CONTINUATION,
            "preState = %{public}d, currentState = %{public}d", preState, continuationManager->GetProcessState());
        if (preState == continuationManager->GetProcessState()) {
            continuationManager->ChangeProcessState(ProgressState::INITIAL);
        }
    };
    mainHandler_->PostTask(timeoutTask, "Restore_State_When_Timeout", timeoutInMs);
}

void ContinuationManagerStage::InitMainHandlerIfNeed()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
    if (mainHandler_ == nullptr) {
        TAG_LOGD(AAFwkTag::CONTINUATION, "Try to init main handler");
        std::lock_guard<std::mutex> lock_l(lock_);
        if ((mainHandler_ == nullptr) && (EventRunner::GetMainEventRunner() != nullptr)) {
            mainHandler_ = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
        }
    }
}

bool ContinuationManagerStage::CheckAbilityToken()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
    if (continueToken_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null continueToken");
        return false;
    }
    return true;
}

void ContinuationManagerStage::CheckDmsInterfaceResult(int result, const std::string &interfaceName)
{
}

bool ContinuationManagerStage::DoScheduleStartContinuation()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
    if (CheckContinuationIllegal()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability not available to startContinuation");
        return false;
    }

    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
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

bool ContinuationManagerStage::DoScheduleSaveData(WantParams &saveData)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
    if (CheckContinuationIllegal()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability not available to save data");
        return false;
    }

    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
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

bool ContinuationManagerStage::DoScheduleRestoreData(const WantParams &restoreData)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
    if (CheckContinuationIllegal()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability not available to restore data");
        return false;
    }

    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
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
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability restore data failed");
    }
    return ret;
}

bool ContinuationManagerStage::DoRestoreFromRemote(const WantParams &restoreData)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin");
    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
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
        TAG_LOGE(AAFwkTag::CONTINUATION, "OnRestoreData failed");
    }
    return ret;
}
} // namespace AppExecFwk
} // namespace OHOS
