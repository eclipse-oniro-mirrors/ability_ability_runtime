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

#include "lifecycle_deal.h"

#include "ability_record.h"
#include "ability_util.h"

namespace OHOS {
namespace AAFwk {
LifecycleDeal::LifecycleDeal()
{}

LifecycleDeal::~LifecycleDeal()
{}

void LifecycleDeal::SetScheduler(const sptr<IAbilityScheduler> &scheduler)
{
    std::unique_lock<std::shared_mutex> lock(schedulerMutex_);
    abilityScheduler_ = scheduler;
}

sptr<IAbilityScheduler> LifecycleDeal::GetScheduler()
{
    std::shared_lock<std::shared_mutex> lock(schedulerMutex_);
    return abilityScheduler_;
}

void LifecycleDeal::Activate(const Want &want, LifeCycleStateInfo &stateInfo)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "call");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "caller %{public}s, %{public}s",
        stateInfo.caller.bundleName.c_str(),
        stateInfo.caller.abilityName.c_str());
    stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_ACTIVE;
    abilityScheduler->ScheduleAbilityTransaction(want, stateInfo);
}

void LifecycleDeal::Inactivate(const Want &want, LifeCycleStateInfo &stateInfo,
    sptr<SessionInfo> sessionInfo)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "call");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_INACTIVE;
    abilityScheduler->ScheduleAbilityTransaction(want, stateInfo, sessionInfo);
}

void LifecycleDeal::MoveToBackground(const Want &want, LifeCycleStateInfo &stateInfo)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_BACKGROUND;
    abilityScheduler->ScheduleAbilityTransaction(want, stateInfo);
}

void LifecycleDeal::ConnectAbility(const Want &want)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "call");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    abilityScheduler->ScheduleConnectAbility(want);
}

void LifecycleDeal::DisconnectAbility(const Want &want)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "call");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    abilityScheduler->ScheduleDisconnectAbility(want);
}

void LifecycleDeal::Terminate(const Want &want, LifeCycleStateInfo &stateInfo, sptr<SessionInfo> sessionInfo)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_INITIAL;
    abilityScheduler->ScheduleAbilityTransaction(want, stateInfo, sessionInfo);
}

void LifecycleDeal::CommandAbility(const Want &want, bool reStart, int startId)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "startId:%{public}d", startId);
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    abilityScheduler->ScheduleCommandAbility(want, reStart, startId);
}

void LifecycleDeal::CommandAbilityWindow(const Want &want, const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    abilityScheduler->ScheduleCommandAbilityWindow(want, sessionInfo, winCmd);
}

void LifecycleDeal::SaveAbilityState()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    abilityScheduler->ScheduleSaveAbilityState();
}

void LifecycleDeal::RestoreAbilityState(const PacMap &inState)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    abilityScheduler->ScheduleRestoreAbilityState(inState);
}

bool LifecycleDeal::ForegroundNew(const Want &want, LifeCycleStateInfo &stateInfo,
    sptr<SessionInfo> sessionInfo)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER_AND_RETURN(abilityScheduler, false);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "caller %{public}s, %{public}s",
        stateInfo.caller.bundleName.c_str(),
        stateInfo.caller.abilityName.c_str());
    stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_FOREGROUND_NEW;
    return abilityScheduler->ScheduleAbilityTransaction(want, stateInfo, sessionInfo);
}

void LifecycleDeal::BackgroundNew(const Want &want, LifeCycleStateInfo &stateInfo,
    sptr<SessionInfo> sessionInfo)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "caller %{public}s, %{public}s",
        stateInfo.caller.bundleName.c_str(),
        stateInfo.caller.abilityName.c_str());
    stateInfo.state = AbilityLifeCycleState::ABILITY_STATE_BACKGROUND_NEW;
    abilityScheduler->ScheduleAbilityTransaction(want, stateInfo, sessionInfo);
}

void LifecycleDeal::ContinueAbility(const std::string& deviceId, uint32_t versionCode)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    abilityScheduler->ContinueAbility(deviceId, versionCode);
}

void LifecycleDeal::NotifyContinuationResult(int32_t result)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    abilityScheduler->NotifyContinuationResult(result);
}

void LifecycleDeal::ShareData(const int32_t &uniqueId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "uniqueId is %{public}d.", uniqueId);
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    abilityScheduler->ScheduleShareData(uniqueId);
}

bool LifecycleDeal::PrepareTerminateAbility()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    auto abilityScheduler = GetScheduler();
    if (abilityScheduler == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null abilityScheduler");
        return false;
    }
    return abilityScheduler->SchedulePrepareTerminateAbility();
}

void LifecycleDeal::UpdateSessionToken(sptr<IRemoteObject> sessionToken)
{
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    abilityScheduler->UpdateSessionToken(sessionToken);
}

void LifecycleDeal::ScheduleCollaborate(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    abilityScheduler->ScheduleCollaborate(want);
}

void LifecycleDeal::NotifyAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
    const std::string &message, int32_t resultCode)
{
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    abilityScheduler->ScheduleAbilityRequestFailure(requestId, element, message, resultCode);
}

void LifecycleDeal::NotifyAbilityRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element)
{
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    abilityScheduler->ScheduleAbilityRequestSuccess(requestId, element);
}

void LifecycleDeal::NotifyAbilitiesRequestDone(const std::string &requestKey, int32_t resultCode)
{
    auto abilityScheduler = GetScheduler();
    CHECK_POINTER(abilityScheduler);
    abilityScheduler->ScheduleAbilitiesRequestDone(requestKey, resultCode);
}
}  // namespace AAFwk
}  // namespace OHOS
