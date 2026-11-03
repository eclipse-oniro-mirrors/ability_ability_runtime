/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "call_record.h"

#include "ability_util.h"
#include "ability_manager_service.h"
#include "ffrt.h"
#include "global_constant.h"

namespace OHOS {
namespace AAFwk {
int64_t CallRecord::callRecordId = 0;

CallRecord::CallRecord(const int32_t callerUid, const std::shared_ptr<AbilityRecord> &targetService,
    const sptr<IAbilityConnection> &connCallback, const sptr<IRemoteObject> &callToken)
    : state_(CallState::INIT),
      callerUid_(callerUid),
      service_(targetService),
      connCallback_(connCallback),
      callerToken_(callToken)
{
    recordId_ = callRecordId++;
    startTime_ = AbilityUtil::SystemTimeMillis();
}

CallRecord::~CallRecord()
{
    if (callRemoteObject_ && callDeathRecipient_) {
        callRemoteObject_->RemoveDeathRecipient(callDeathRecipient_);
    }
}

std::shared_ptr<CallRecord> CallRecord::CreateCallRecord(const int32_t callerUid,
    const std::shared_ptr<AbilityRecord> &targetService, const sptr<IAbilityConnection> &connCallback,
    const sptr<IRemoteObject> &callToken)
{
    auto callRecord = std::make_shared<CallRecord>(callerUid, targetService, connCallback, callToken);
    CHECK_POINTER_AND_RETURN(callRecord, nullptr);
    callRecord->SetCallState(CallState::INIT);
    return callRecord;
}

void CallRecord::SetCallStub(const sptr<IRemoteObject> &call)
{
    CHECK_POINTER(call);
    if (callRemoteObject_) {
        // Already got callRemoteObject, just return
        return;
    }
    callRemoteObject_ = call;

    TAG_LOGD(AAFwkTag::ABILITYMGR, "SetCallStub complete.");

    if (callDeathRecipient_ == nullptr) {
        std::weak_ptr<CallRecord> callRecord = shared_from_this();
        auto callStubDied = [wptr = callRecord] (const wptr<IRemoteObject> &remote) {
            auto call = wptr.lock();
            if (call == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "null call");
                return;
            }

            call->OnCallStubDied(remote);
        };
        callDeathRecipient_ =
                new AbilityCallRecipient(callStubDied);
    }

    if (!callRemoteObject_->AddDeathRecipient(callDeathRecipient_)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "addDeathRecipient failed");
    }
}

sptr<IRemoteObject> CallRecord::GetCallStub()
{
    return callRemoteObject_;
}

void CallRecord::SetConCallBack(const sptr<IAbilityConnection> &connCallback)
{
    connCallback_ = connCallback;
}

sptr<IAbilityConnection> CallRecord::GetConCallBack() const
{
    return connCallback_;
}

AppExecFwk::ElementName CallRecord::GetTargetServiceName() const
{
    std::shared_ptr<AbilityRecord> tmpService = service_.lock();
    if (tmpService) {
        const AppExecFwk::AbilityInfo &abilityInfo = tmpService->GetAbilityInfo();
        AppExecFwk::ElementName element(abilityInfo.deviceId, abilityInfo.bundleName,
            abilityInfo.name, abilityInfo.moduleName);
        return element;
    }
    return AppExecFwk::ElementName();
}

sptr<IRemoteObject> CallRecord::GetCallerToken() const
{
    return callerToken_;
}

bool CallRecord::SchedulerConnectDone()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Scheduler Connect Done by callback. id:%{public}d, %{public}d",
        recordId_, isPrelaunch_);
    std::shared_ptr<AbilityRecord> tmpService = service_.lock();
    auto remoteObject = callRemoteObject_;
    auto callback = connCallback_;
    if (!remoteObject || !callback || !tmpService) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callstub or connCallback null");
        return false;
    }

    const AppExecFwk::AbilityInfo &abilityInfo = tmpService->GetAbilityInfo();
    AppExecFwk::ElementName element(abilityInfo.deviceId, abilityInfo.bundleName,
        abilityInfo.name, abilityInfo.moduleName);
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER_AND_RETURN(handler, false);
    ffrt::submit([callback, remoteObject, launchMode = abilityInfo.launchMode, element]() {
        callback->OnAbilityConnectDone(element,
            remoteObject, static_cast<int32_t>(launchMode));
        }, ffrt::task_attr().timeout(AbilityRuntime::GlobalConstant::DEFAULT_FFRT_TASK_TIMEOUT));
    state_ = CallState::REQUESTED;

    TAG_LOGD(AAFwkTag::ABILITYMGR, "element: %{public}s, mode: %{public}d. connectstate:%{public}d.",
        element.GetURI().c_str(), static_cast<int32_t>(abilityInfo.launchMode), state_);
    return true;
}

bool CallRecord::SchedulerDisconnectDone()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Scheduler disconnect Done by callback. id:%{public}d", recordId_);
    std::shared_ptr<AbilityRecord> tmpService = service_.lock();
    auto callback = connCallback_;
    if (!callback || !tmpService) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callstub or connCallback null");
        return false;
    }

    const AppExecFwk::AbilityInfo &abilityInfo = tmpService->GetAbilityInfo();
    AppExecFwk::ElementName element(abilityInfo.deviceId, abilityInfo.bundleName,
        abilityInfo.name, abilityInfo.moduleName);
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER_AND_RETURN(handler, false);
    ffrt::submit([callback, element, id = recordId_]() {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "OnAbilityDisconnectDone begin, recordId:%{public}d, reason:0", id);
        callback->OnAbilityDisconnectDone(element,  ERR_OK);
        }, ffrt::task_attr().timeout(AbilityRuntime::GlobalConstant::DEFAULT_FFRT_TASK_TIMEOUT));

    return true;
}

void CallRecord::OnCallStubDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "callstub is died. id:%{public}d begin", recordId_);

    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER(abilityManagerService);
    auto handler = abilityManagerService->GetTaskHandler();
    CHECK_POINTER(handler);
    auto task = [abilityManagerService, callRecord = shared_from_this()]() {
        abilityManagerService->OnCallConnectDied(callRecord);
    };
    handler->SubmitTask(task);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "callstub is died. id:%{public}d, end", recordId_);
}

int32_t CallRecord::GetUserId() const
{
    std::shared_ptr<AbilityRecord> tmpService = service_.lock();
    if (tmpService == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "AbilityRecord is invalid");
        return -1;
    }
    return tmpService->GetOwnerMissionUserId();
}

void CallRecord::Dump(std::vector<std::string> &info) const
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "CallRecord::Dump is called");

    std::string tempstr = "            CallRecord";
    tempstr += " ID #" + std::to_string (recordId_) + "\n";
    tempstr += "              caller";
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken_);
    if (abilityRecord) {
        AppExecFwk::ElementName element(
            abilityRecord->GetAbilityInfo().deviceId, abilityRecord->GetAbilityInfo().bundleName,
            abilityRecord->GetAbilityInfo().name, abilityRecord->GetAbilityInfo().moduleName);
        tempstr += " uri [" + element.GetURI() + "]" + "\n";
    }

    std::string state = (state_ == CallState::INIT ? "INIT" :
                        state_ == CallState::REQUESTING ? "REQUESTING" : "REQUESTED");
    tempstr += "              state #" + state;
    tempstr += " start time [" + std::to_string (startTime_) + "]";
    info.emplace_back(tempstr);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "CallRecord::Dump is called1");
}

int32_t CallRecord::GetCallerUid() const
{
    return callerUid_;
}

bool CallRecord::IsCallState(const CallState &state) const
{
    return (state_ == state);
}

void CallRecord::SetCallState(const CallState &state)
{
    state_ = state;
}

void CallRecord::SetPrelaunchFlag(bool isPrelaunch)
{
    isPrelaunch_ = isPrelaunch;
}

bool CallRecord::IsPrelaunch() const
{
    return isPrelaunch_;
}

int CallRecord::GetCallRecordId() const
{
    return recordId_;
}
}  // namespace AAFwk
}  // namespace OHOS
