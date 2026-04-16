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

#include "pending_want_manager.h"

#include "ability_manager_service.h"
#include "ability_util.h"
#include "distributed_client.h"
#include "hitrace_meter.h"
#include "permission_constants.h"
#include "session_manager_lite.h"
#include "utils/app_mgr_util.h"
#include "wm_common.h"

namespace OHOS {
namespace AAFwk {
using namespace OHOS::EventFwk;
using namespace std::chrono;
using namespace std::placeholders;
constexpr const char* PENDING_WANT_MANAGER = "PendingWantManager";

PendingWantManager::PendingWantManager()
{
    taskHandler_ = TaskHandlerWrap::CreateQueueHandler(PENDING_WANT_MANAGER);
}

PendingWantManager::~PendingWantManager()
{
}

sptr<IWantSender> PendingWantManager::GetWantSender(int32_t callingUid, int32_t uid, const bool isSystemApp,
    const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken, int32_t appIndex)
{
    TAG_LOGD(AAFwkTag::WANTAGENT, "begin");
    if (wantSenderInfo.type != static_cast<int32_t>(OperationType::SEND_COMMON_EVENT)) {
        if (callingUid != uid &&
            !isSystemApp &&
            !AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "cannot send");
            return nullptr;
        }
    }

    if (wantSenderInfo.type == static_cast<int32_t>(OperationType::START_SERVICE_EXTENSION) && !isSystemApp &&
        !AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "non-system app called");
        return nullptr;
    }

    WantSenderInfo info = wantSenderInfo;

    if (wantSenderInfo.type != static_cast<int32_t>(OperationType::SEND_COMMON_EVENT) &&
        !isSystemApp && !AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        for (auto it = info.allWants.begin(); it != info.allWants.end();) {
        if (info.bundleName != it->want.GetBundle()) {
                it = info.allWants.erase(it);
            } else {
                it->want.RemoveParam("ohos.extra.param.key.appCloneIndex");
                it++;
            }
        }
    }

    return GetWantSenderLocked(callingUid, uid, wantSenderInfo.userId, info, callerToken, appIndex);
}

void SendWantAgentNumberEvent(std::shared_ptr<PendingWantKey> pendingKey, int32_t wantAgentNumber)
{
    std::string bundleName = pendingKey->GetBundleName();
    std::int32_t appIndex = pendingKey->GetAppIndex();

    EventInfo eventInfo;
    eventInfo.callerBundleName = bundleName;
    eventInfo.callerUid = appIndex;
    eventInfo.wantAgentNumber = wantAgentNumber;
    EventReport::SendWantAgentEvent(EventName::WANTAGENT_NUMBER, HISYSEVENT_STATISTIC, eventInfo);
}

void PendingWantManager::HandleAddWantAgentNumber(std::shared_ptr<PendingWantKey> pendingKey)
{
    if (pendingKey == nullptr) {
        return;
    }
    TAG_LOGD(AAFwkTag::WANTAGENT, "add wantAgent number");
    std::string bundleName = pendingKey->GetBundleName();
    std::lock_guard<ffrt::mutex> locker(countMutex_);
    auto it = wantAgentCount_.find(bundleName);
    if (it == wantAgentCount_.end()) {
        wantAgentCount_.insert(std::make_pair(bundleName, AgentCount{1, 1, false}));
        return;
    }

    // not first add
    it->second.currentNumber = it->second.currentNumber + 1;
    if (it->second.DFXFlag) { // Not First DFX
        TAG_LOGD(AAFwkTag::WANTAGENT, "wantagent current number: %{public}d", it->second.currentNumber);
        if ((it->second.currentNumber > WANTAGENT_NUMBER_THRESHOLD) &&
            (it->second.currentNumber - it->second.latestMinNumber > WANTAGENT_FLOAT_THRESHOLD)) {
            if (it->second.currentNumber % DFX_INTERVAL == 0) {
                SendWantAgentNumberEvent(pendingKey, it->second.currentNumber);
            }
        }
    } else { // First DFX
        if (it->second.currentNumber > WANTAGENT_NUMBER_THRESHOLD) {
            SendWantAgentNumberEvent(pendingKey, it->second.currentNumber);
            it->second.DFXFlag = true;
        }
    }
}

void PendingWantManager::AddWantAgentNumber(std::shared_ptr<PendingWantKey> pendingKey)
{
    // check is third-party hap
    if (PermissionVerification::GetInstance()->IsSystemAppCall() ||
        PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGD(AAFwkTag::WANTAGENT, "systemCall");
        return;
    }
    HandleAddWantAgentNumber(pendingKey);
}

void PendingWantManager::HandleReduceWantAgentNumber(std::shared_ptr<PendingWantKey> pendingKey)
{
    if (pendingKey == nullptr) {
        return;
    }
    TAG_LOGD(AAFwkTag::WANTAGENT, "reduce wantAgent number");
    std::string bundleName = pendingKey->GetBundleName();
    std::lock_guard<ffrt::mutex> locker(countMutex_);
    auto it = wantAgentCount_.find(bundleName);
    if (it == wantAgentCount_.end()) {
        TAG_LOGD(AAFwkTag::WANTAGENT, "No WantAgent");
    } else {
        if (it->second.currentNumber == 1) {
            wantAgentCount_.erase(bundleName);
        } else {
            it->second.currentNumber = it->second.currentNumber - 1;
            it->second.latestMinNumber = it->second.currentNumber; // Record Latest Minimum WantAgent Number
        }
    }
}

void PendingWantManager::ReduceWantAgentNumber(std::shared_ptr<PendingWantKey> pendingKey)
{
    // check is third-party hap
    if (PermissionVerification::GetInstance()->IsSystemAppCall() ||
        PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGD(AAFwkTag::WANTAGENT, "systemCall");
        return;
    }
    HandleReduceWantAgentNumber(pendingKey);
}

sptr<IWantSender> PendingWantManager::GetWantSenderLocked(const int32_t callingUid, const int32_t uid,
    const int32_t userId, WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken, int32_t appIndex)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::WANTAGENT, "begin");

    bool needCreate = (static_cast<uint32_t>(wantSenderInfo.flags) &
        static_cast<uint32_t>(Flags::NO_BUILD_FLAG)) == 0;
    bool needCancel = (static_cast<uint32_t>(wantSenderInfo.flags) &
        static_cast<uint32_t>(Flags::CANCEL_PRESENT_FLAG)) != 0;
    bool needUpdate = (static_cast<uint32_t>(wantSenderInfo.flags) &
        static_cast<uint32_t>(Flags::UPDATE_PRESENT_FLAG)) != 0;

    std::shared_ptr<PendingWantKey> pendingKey = std::make_shared<PendingWantKey>();
    pendingKey->SetBundleName(wantSenderInfo.bundleName);
    pendingKey->SetRequestWho(wantSenderInfo.resultWho);
    pendingKey->SetRequestCode(wantSenderInfo.requestCode);
    pendingKey->SetFlags(wantSenderInfo.flags);
    pendingKey->SetUserId(wantSenderInfo.userId);
    pendingKey->SetType(wantSenderInfo.type);
    pendingKey->SetAppIndex(appIndex);
    if (wantSenderInfo.allWants.size() > 0) {
        pendingKey->SetRequestWant(wantSenderInfo.allWants.back().want);
        pendingKey->SetRequestResolvedType(wantSenderInfo.allWants.back().resolvedTypes);
        pendingKey->SetAllWantsInfos(wantSenderInfo.allWants);
    }
    std::lock_guard<ffrt::mutex> locker(mutex_);
    auto ref = GetPendingWantRecordByKey(pendingKey);
    if (ref != nullptr) {
        if (!needCancel) {
            if (needUpdate && wantSenderInfo.allWants.size() > 0) {
                ref->GetKey()->SetRequestWant(wantSenderInfo.allWants.back().want);
                ref->GetKey()->SetRequestResolvedType(wantSenderInfo.allWants.back().resolvedTypes);
                wantSenderInfo.allWants.back().want = ref->GetKey()->GetRequestWant();
                wantSenderInfo.allWants.back().resolvedTypes = ref->GetKey()->GetRequestResolvedType();
                ref->GetKey()->SetAllWantsInfos(wantSenderInfo.allWants);
                ref->SetCallerUid(callingUid);
            }
            return ref;
        }
        MakeWantSenderCanceledLocked(*ref);
        ReduceWantAgentNumber(ref->GetKey());
        wantRecords_.erase(ref->GetKey());
    }

    if (!needCreate) {
        return (ref != nullptr) ? ref : nullptr;
    }

    sptr<PendingWantRecord> rec =
        new (std::nothrow) PendingWantRecord(shared_from_this(), uid, IPCSkeleton::GetCallingTokenID(),
        callerToken, pendingKey);
    if (rec != nullptr) {
        rec->SetCallerUid(callingUid);
        pendingKey->SetCode(PendingRecordIdCreate());
        AddWantAgentNumber(pendingKey);
        wantRecords_.insert(std::make_pair(pendingKey, rec));
        TAG_LOGI(AAFwkTag::WANTAGENT,
            "wantRecords_ size %{public}zu, bundleName=%{public}s, flags=%{public}d, type=%{public}d, code=%{public}d",
            wantRecords_.size(), pendingKey->GetBundleName().c_str(), pendingKey->GetFlags(), pendingKey->GetType(),
            pendingKey->GetCode());
        return rec;
    }
    return nullptr;
}

void PendingWantManager::MakeWantSenderCanceledLocked(PendingWantRecord &record)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "cancel");

    record.SetCanceled();
    for (auto &callback : record.GetCancelCallbacks()) {
        callback->Send(record.GetKey()->GetRequestCode());
    }
    if (record.GetKey() != nullptr && record.GetKey()->GetCode() > 0) {
        record.GetKey()->ClearAllWantsInfosFd();
    }
}

sptr<PendingWantRecord> PendingWantManager::GetPendingWantRecordByKey(const std::shared_ptr<PendingWantKey> &key)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::WANTAGENT, "begin");
    for (const auto &item : wantRecords_) {
        const auto pendingKey = item.first;
        const auto pendingRecord = item.second;
        if ((pendingRecord != nullptr) && CheckPendingWantRecordByKey(pendingKey, key)) {
            return pendingRecord;
        }
    }
    return nullptr;
}

bool PendingWantManager::CheckPendingWantRecordByKey(
    const std::shared_ptr<PendingWantKey> &inputKey, const std::shared_ptr<PendingWantKey> &key)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!inputKey || !key) {
        TAG_LOGW(AAFwkTag::WANTAGENT, "inputKey or key null");
        return false;
    }
    if (inputKey->GetAppIndex() != key->GetAppIndex()) {
        return false;
    }
    if (inputKey->GetBundleName().compare(key->GetBundleName()) != 0) {
        return false;
    }
    if (inputKey->GetType() != key->GetType()) {
        return false;
    }
    if (inputKey->GetRequestWho().compare(key->GetRequestWho()) != 0) {
        return false;
    }
    if (inputKey->GetRequestCode() != key->GetRequestCode()) {
        return false;
    }

    if (inputKey->GetRequestResolvedType().compare(key->GetRequestResolvedType()) != 0) {
        return false;
    }
    if (inputKey->GetUserId() != key->GetUserId()) {
        return false;
    }

    if (!inputKey->IsEqualsRequestWant(key->GetRequestWantRef())) {
        return false;
    }

    return true;
}

int32_t PendingWantManager::SendWantSender(sptr<IWantSender> target,  SenderInfo &senderInfo)
{
    SenderInfo info = senderInfo;

    if (target == nullptr) {
        if (senderInfo.finishedReceiver != nullptr) {
            Want want;
            WantParams wantParams = {};
            senderInfo.finishedReceiver->PerformReceive(want, senderInfo.code, "canceled", wantParams, false, false, 0);
        }
        TAG_LOGE(AAFwkTag::WANTAGENT, "null sender");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> obj = target->AsObject();
    if (obj == nullptr || obj->IsProxyObject()) {
        if (senderInfo.finishedReceiver != nullptr) {
            Want want;
            WantParams wantParams = {};
            senderInfo.finishedReceiver->PerformReceive(want, senderInfo.code, "canceled", wantParams, false, false, 0);
        }
        TAG_LOGE(AAFwkTag::WANTAGENT, "target object null or a proxy");
        return ERR_INVALID_VALUE;
    }
    sptr<PendingWantRecord> record = static_cast<PendingWantRecord*>(target.GetRefPtr());
    if (!CheckPermission(record)) {
        if (senderInfo.finishedReceiver != nullptr) {
            Want want;
            record->BuildSendWant(info, want);
            WantParams wantParams = {};
            senderInfo.finishedReceiver->PerformReceive(want, senderInfo.code, "", wantParams, false, false, 0);
        }
        return ERR_INVALID_VALUE;
    }
    return record->SenderInner(senderInfo);
}

int32_t PendingWantManager::SendLocalWantSender(const SenderInfo &senderInfo)
{
    int32_t result = NO_ERROR;
    switch (senderInfo.operType) {
        case static_cast<int32_t>(OperationType::START_ABILITY): {
            result = PendingWantStartAbility(senderInfo.want, senderInfo.startOptions,
                senderInfo.callerToken, -1, senderInfo.uid, senderInfo.tokenId, -1);
            break;
        }
        case static_cast<int32_t>(OperationType::START_SERVICE): {
            result = PendingWantStartAbility(senderInfo.want, nullptr,
                senderInfo.callerToken, -1, senderInfo.uid, senderInfo.tokenId, -1);
            break;
        }
        default: {
            break;
        }
    }
    bool sendFinish = (senderInfo.finishedReceiver != nullptr);
    if (sendFinish && result != ERR_WANTAGENT_CANCELED) {
        WantParams wantParams = {};
        senderInfo.finishedReceiver->PerformReceive(senderInfo.want, senderInfo.code, "", wantParams, false, false, 0);
    }
    return result;
}

void PendingWantManager::CancelWantSender(const bool isSystemAppCall, const sptr<IWantSender> &sender)
{
    TAG_LOGD(AAFwkTag::WANTAGENT, "begin");

    if (sender == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null sender");
        return;
    }

    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall && !isSystemAppCall) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "cannot send");
        return;
    }

    sptr<IRemoteObject> obj = sender->AsObject();
    if (obj == nullptr || obj->IsProxyObject()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "target object null or a proxy");
        return;
    }
    sptr<PendingWantRecord> record = static_cast<PendingWantRecord*>(sender.GetRefPtr());
    CancelWantSenderLocked(*record, true);
}

void PendingWantManager::CancelWantSenderLocked(PendingWantRecord &record, bool cleanAbility)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::WANTAGENT, "begin");
    std::lock_guard<ffrt::mutex> locker(mutex_);
    MakeWantSenderCanceledLocked(record);
    if (cleanAbility) {
        ReduceWantAgentNumber(record.GetKey());
        wantRecords_.erase(record.GetKey());
    }
}

int32_t PendingWantManager::DeviceIdDetermine(const Want &want, const sptr<StartOptions> &startOptions,
    const sptr<IRemoteObject> &callerToken, int32_t requestCode, const int32_t callerUid, int32_t callerTokenId,
    int32_t userId)
{
    int32_t result = ERR_OK;
    std::string localDeviceId;
    DelayedSingleton<AbilityManagerService>::GetInstance()->GetLocalDeviceId(localDeviceId);
    if (want.GetElement().GetDeviceID() == "" || want.GetElement().GetDeviceID() == localDeviceId) {
        if (!startOptions) {
            result = DelayedSingleton<AbilityManagerService>::GetInstance()->StartAbilityWithSpecifyTokenIdInner(
                want, callerToken, callerTokenId, true, userId, requestCode);
        } else {
            TAG_LOGD(AAFwkTag::WANTAGENT, "StartOptions windowMode:%{public}d displayId:%{public}d \
                withAnimation:%{public}d windowLeft:%{public}d windowTop:%{public}d windowWidth:%{public}d \
                windowHeight:%{public}d",
                startOptions->GetWindowMode(), startOptions->GetDisplayID(), startOptions->GetWithAnimation(),
                startOptions->GetWindowLeft(), startOptions->GetWindowTop(), startOptions->GetWindowWidth(),
                startOptions->GetWindowHeight());
            result = DelayedSingleton<AbilityManagerService>::GetInstance()->StartAbilityWithSpecifyTokenIdInner(
                want, *startOptions, callerToken, true, userId, requestCode, callerTokenId);
        }

        if (result != ERR_OK && result != START_ABILITY_WAITING) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "startAbility failed");
        }
        return result;
    }

    sptr<IRemoteObject> remoteObject =
        OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->GetSystemAbility(DISTRIBUTED_SCHED_SA_ID);
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "GetSystemAbility failed");
        result = ERR_INVALID_VALUE;
        return result;
    }
    DistributedClient dmsClient;
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    result = dmsClient.StartRemoteAbility(want, callingUid, requestCode, accessToken, callerTokenId);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "StartRemoteAbility failed result = %{public}d", result);
    }

    return result;
}

int32_t PendingWantManager::PendingWantStartAbility(const Want &want, const sptr<StartOptions> &startOptions,
    const sptr<IRemoteObject> &callerToken, int32_t requestCode, const int32_t callerUid, int32_t callerTokenId,
    int32_t userId)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "start ability");
    int32_t result = DeviceIdDetermine(want, startOptions, callerToken, requestCode, callerUid, callerTokenId, userId);
    return result;
}

int32_t PendingWantManager::PendingWantStartServiceExtension(Want &want, const sptr<IRemoteObject> &callerToken)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "start service extension");
    if (!PermissionVerification::GetInstance()->IsSystemAppCall() &&
        !PermissionVerification::GetInstance()->IsSACall()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "non-system app called");
        return ERR_INVALID_VALUE;
    }
    //reset flags
    want.SetFlags(0);
    return DelayedSingleton<AbilityManagerService>::GetInstance()->StartExtensionAbility(want, callerToken);
}

int32_t PendingWantManager::PendingWantStartAbilitys(const std::vector<WantsInfo> &wantsInfo,
    const sptr<StartOptions> &startOptions, const sptr<IRemoteObject> &callerToken, int32_t requestCode,
    const int32_t callerUid, int32_t callerTokenId, int32_t userId)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "start abilitys");
    int32_t result = ERR_OK;
    for (const auto &item : wantsInfo) {
        auto res = DeviceIdDetermine(item.want, startOptions, callerToken, requestCode, callerUid, callerTokenId,
            userId);
        if (res != ERR_OK && res != START_ABILITY_WAITING) {
            result = res;
        }
    }
    return result;
}

int32_t PendingWantManager::PendingWantPublishCommonEvent(
    const Want &want, const SenderInfo &senderInfo, int32_t callerUid, int32_t callerTokenId)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "publish common event");

    CommonEventData eventData;
    eventData.SetWant(want);
    eventData.SetCode(senderInfo.code);

    CommonEventPublishInfo eventPublishData;

    if (!want.GetBundle().empty()) {
        TAG_LOGI(AAFwkTag::WANTAGENT, "eventPublishData set bundleName: %{public}s", want.GetBundle().c_str());
        eventPublishData.SetBundleName(want.GetBundle());
    }

    if (!senderInfo.requiredPermission.empty()) {
        std::vector<std::string> permissions;
        permissions.emplace_back(senderInfo.requiredPermission);
        eventPublishData.SetSubscriberPermissions(permissions);
    }

    bool result = IN_PROCESS_CALL(DelayedSingleton<EventFwk::CommonEvent>::GetInstance()->PublishCommonEvent(
        eventData, eventPublishData, nullptr, callerUid, callerTokenId));
    return ((result == true) ? ERR_OK : (-1));
}

int32_t PendingWantManager::PendingRecordIdCreate()
{
    TAG_LOGD(AAFwkTag::WANTAGENT, "begin");

    static std::atomic_int id(0);
    return ++id;
}

sptr<PendingWantRecord> PendingWantManager::GetPendingWantRecordByCode(int32_t code)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::WANTAGENT, "resuest code:%{public}d", code);

    std::lock_guard<ffrt::mutex> locker(mutex_);
    auto iter = std::find_if(wantRecords_.begin(), wantRecords_.end(), [&code](const auto &pair) {
        return pair.second->GetKey()->GetCode() == code;
    });
    return ((iter == wantRecords_.end()) ? nullptr : iter->second);
}

int32_t PendingWantManager::GetPendingWantUid(const sptr<IWantSender> &target)
{
    if (target == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null target");
        return -1;
    }
    sptr<IRemoteObject> obj = target->AsObject();
    if (obj == nullptr || obj->IsProxyObject()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "target obj null or a proxy object");
        return -1;
    }

    sptr<PendingWantRecord> targetRecord = static_cast<PendingWantRecord*>(target.GetRefPtr());
    auto record = GetPendingWantRecordByCode(targetRecord->GetKey()->GetCode());
    return ((record != nullptr) ? (record->GetUid()) : (-1));
}

int32_t PendingWantManager::GetPendingWantUserId(const sptr<IWantSender> &target)
{
    TAG_LOGD(AAFwkTag::WANTAGENT, "begin");

    if (target == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null target");
        return -1;
    }
    sptr<IRemoteObject> obj = target->AsObject();
    if (obj == nullptr || obj->IsProxyObject()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "target obj null or a proxy object");
        return -1;
    }
    sptr<PendingWantRecord> targetRecord = static_cast<PendingWantRecord*>(target.GetRefPtr());
    auto record = GetPendingWantRecordByCode(targetRecord->GetKey()->GetCode());
    return ((record != nullptr) ? (record->GetKey()->GetUserId()) : (-1));
}

std::string PendingWantManager::GetPendingWantBundleName(const sptr<IWantSender> &target)
{
    TAG_LOGD(AAFwkTag::WANTAGENT, "begin");

    if (target == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null target");
        return "";
    }
    sptr<IRemoteObject> obj = target->AsObject();
    if (obj == nullptr || obj->IsProxyObject()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "target obj null or a proxy object");
        return "";
    }

    sptr<PendingWantRecord> targetRecord = static_cast<PendingWantRecord*>(target.GetRefPtr());
    auto record = GetPendingWantRecordByCode(targetRecord->GetKey()->GetCode());
    if (record != nullptr) {
        return record->GetKey()->GetBundleName();
    }
    return "";
}

int32_t PendingWantManager::GetPendingWantCode(const sptr<IWantSender> &target)
{
    TAG_LOGD(AAFwkTag::WANTAGENT, "begin");

    if (target == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null target");
        return -1;
    }
    sptr<IRemoteObject> obj = target->AsObject();
    if (obj == nullptr || obj->IsProxyObject()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "target obj null or a proxy object");
        return -1;
    }

    sptr<PendingWantRecord> targetRecord = static_cast<PendingWantRecord*>(target.GetRefPtr());
    auto record = GetPendingWantRecordByCode(targetRecord->GetKey()->GetCode());
    return ((record != nullptr) ? (record->GetKey()->GetCode()) : (-1));
}

int32_t PendingWantManager::GetPendingWantType(const sptr<IWantSender> &target)
{
    TAG_LOGD(AAFwkTag::WANTAGENT, "begin");

    if (target == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null target");
        return -1;
    }
    sptr<IRemoteObject> obj = target->AsObject();
    if (obj == nullptr || obj->IsProxyObject()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "target obj null or a proxy object");
        return -1;
    }

    sptr<PendingWantRecord> targetRecord = static_cast<PendingWantRecord*>(target.GetRefPtr());
    auto record = GetPendingWantRecordByCode(targetRecord->GetKey()->GetCode());
    return ((record != nullptr) ? (record->GetKey()->GetType()) : (-1));
}

void PendingWantManager::RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &recevier)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::WANTAGENT, "begin");

    if ((sender == nullptr) || (recevier == nullptr)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "sender or recevier null");
        return;
    }
    sptr<IRemoteObject> obj = sender->AsObject();
    if (obj == nullptr || obj->IsProxyObject()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "target obj null or a proxy object");
        return;
    }

    sptr<PendingWantRecord> targetRecord = static_cast<PendingWantRecord*>(sender.GetRefPtr());
    auto record = GetPendingWantRecordByCode(targetRecord->GetKey()->GetCode());
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null record. code = %{public}d",
            targetRecord->GetKey()->GetCode());
        return;
    }
    bool cancel = record->GetCanceled();
    std::lock_guard<ffrt::mutex> locker(mutex_);
    if (!cancel) {
        record->RegisterCancelListener(recevier);
    }
}

void PendingWantManager::UnregisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &recevier)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sender == nullptr || recevier == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "sender or recevier null");
        return;
    }
    sptr<IRemoteObject> obj = sender->AsObject();
    if (obj == nullptr || obj->IsProxyObject()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "target obj null or a proxy object");
        return;
    }

    sptr<PendingWantRecord> targetRecord = static_cast<PendingWantRecord*>(sender.GetRefPtr());
    auto record = GetPendingWantRecordByCode(targetRecord->GetKey()->GetCode());
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null record");
        return;
    }
    std::lock_guard<ffrt::mutex> locker(mutex_);
    record->UnregisterCancelListener(recevier);
}

int32_t PendingWantManager::GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::WANTAGENT, "begin");
    if (target == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null target");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> obj = target->AsObject();
    if (obj == nullptr || obj->IsProxyObject()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "target obj null or a proxy object");
        return ERR_INVALID_VALUE;
    }

    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null want");
        return ERR_INVALID_VALUE;
    }
    sptr<PendingWantRecord> targetRecord = static_cast<PendingWantRecord*>(target.GetRefPtr());

    if (targetRecord == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null targetRecord");
        return ERR_INVALID_VALUE;
    }

    auto record = GetPendingWantRecordByCode(targetRecord->GetKey()->GetCode());
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null record, request code=%{public}d", targetRecord->GetKey()->GetCode());
        return ERR_INVALID_VALUE;
    }
    want.reset(new (std::nothrow) Want(record->GetKey()->GetRequestWant()));
    return NO_ERROR;
}

int32_t PendingWantManager::GetPendingRequestWantFromProxy(const sptr<IWantSender> &target, std::shared_ptr<Want> &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::WANTAGENT, "begin");
    if (target == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null target");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> obj = target->AsObject();
    if (obj == nullptr || obj->IsProxyObject()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "target obj null or a proxy object");
        return ERR_INVALID_VALUE;
    }

    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null want");
        return ERR_INVALID_VALUE;
    }
    sptr<PendingWantRecord> targetRecord = static_cast<PendingWantRecord*>(target.GetRefPtr());

    if (targetRecord == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null targetRecord");
        return ERR_INVALID_VALUE;
    }

    want.reset(new (std::nothrow) Want(targetRecord->GetKey()->GetRequestWant()));
    return NO_ERROR;
}

int32_t PendingWantManager::GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info)
{
    TAG_LOGD(AAFwkTag::WANTAGENT, "begin");
    if (target == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null target");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> obj = target->AsObject();
    if (obj == nullptr || obj->IsProxyObject()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "target obj null or a proxy object");
        return ERR_INVALID_VALUE;
    }
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null info");
        return ERR_INVALID_VALUE;
    }
    sptr<PendingWantRecord> targetRecord = static_cast<PendingWantRecord*>(target.GetRefPtr());
    auto record = GetPendingWantRecordByCode(targetRecord->GetKey()->GetCode());
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null record");
        return ERR_INVALID_VALUE;
    }
    WantSenderInfo wantSenderInfo;
    wantSenderInfo.requestCode = record->GetKey()->GetRequestCode();
    wantSenderInfo.type = record->GetKey()->GetType();
    wantSenderInfo.flags = (uint32_t)(record->GetKey()->GetFlags());
    wantSenderInfo.allWants = record->GetKey()->GetAllWantsInfos();
    wantSenderInfo.appIndex = record->GetKey()->GetAppIndex();
    wantSenderInfo.userId = record->GetKey()->GetUserId();
    info.reset(new (std::nothrow) WantSenderInfo(wantSenderInfo));
    return NO_ERROR;
}

void PendingWantManager::ClearPendingWantRecord(const std::string &bundleName, int32_t uid)
{
    CHECK_POINTER(taskHandler_);
    TAG_LOGI(AAFwkTag::WANTAGENT, "begin");
    auto task = [bundleName, uid, thisWeakPtr = weak_from_this()]() {
        auto wantManager = thisWeakPtr.lock();
        if (wantManager) {
            wantManager->ClearPendingWantRecordTask(bundleName, uid);
        }
    };
    taskHandler_->SubmitTask(task);
}

void PendingWantManager::ClearPendingWantRecordTask(const std::string &bundleName, int32_t uid)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::WANTAGENT, "begin");
    std::lock_guard<ffrt::mutex> locker(mutex_);
    auto iter = wantRecords_.begin();
    while (iter != wantRecords_.end()) {
        bool hasBundle = false;
        const auto &pendingRecord = iter->second;
        if ((pendingRecord != nullptr)) {
            std::vector<std::string> bundleNameVec;
            pendingRecord->GetKey()->GetAllBundleNames(bundleNameVec);
            for (const auto &bundleItem: bundleNameVec) {
                if (bundleItem == bundleName && uid == pendingRecord->GetUid()) {
                    hasBundle = true;
                    break;
                }
            }
            if (hasBundle) {
                ReduceWantAgentNumber(iter->first);
                iter = wantRecords_.erase(iter);
                TAG_LOGI(AAFwkTag::WANTAGENT, "wantRecords_ size %{public}zu", wantRecords_.size());
            } else {
                ++iter;
            }
        } else {
            ++iter;
        }
    }
}

bool PendingWantManager::CheckPermission(sptr<PendingWantRecord> record)
{
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "record is nullptr");
        return false;
    }
    int32_t type = static_cast<int32_t>(OperationType::UNKNOWN_TYPE);
    if (record->GetCanceled() != true) {
        type = record->GetKey()->GetType();
    }

    if (type == static_cast<int32_t>(OperationType::START_ABILITY) ||
        type == static_cast<int32_t>(OperationType::START_ABILITIES) ||
        type == static_cast<int32_t>(OperationType::START_SERVICE) ||
        type == static_cast<int32_t>(OperationType::START_FOREGROUND_SERVICE)) {
        return CheckCallerPermission();
    }
    return true;
}

bool PendingWantManager::CheckCallerPermission()
{
    auto callerPid = IPCSkeleton::GetCallingPid();
    AppExecFwk::RunningProcessInfo processInfo;
    DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByPid(callerPid, processInfo);
    auto permission = DelayedSingleton<PermissionVerification>::GetInstance();
    if (permission == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null permission");
        return false;
    }
    if ((!processInfo.isFocused && !processInfo.isAbilityForegrounding) ||
        (!permission->IsSystemAppCall() && !CheckWindowState(callerPid))) {
        TAG_LOGW(AAFwkTag::WANTAGENT, "caller unfocused");
        if (!permission->VerifyCallingPermission(PermissionConstants::PERMISSION_START_ABILITIES_FROM_BACKGROUND) &&
            !permission->VerifyCallingPermission(PermissionConstants::PERMISSION_START_ABILIIES_FROM_BACKGROUND) &&
            !permission->IsSACall()) {
            TAG_LOGW(AAFwkTag::WANTAGENT, "caller PERMISSION_DENIED");
            return false;
        }
    }
    return true;
}

bool PendingWantManager::CheckWindowState(int32_t pid)
{
    auto sceneSessionManager = Rosen::SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
    if (sceneSessionManager == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null manager");
        return false;
    }
    std::vector<Rosen::MainWindowState> windowStates;
    Rosen::WSError ret = sceneSessionManager->GetMainWindowStatesByPid(pid, windowStates);
    if (ret != Rosen::WSError::WS_OK || windowStates.empty()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "fail GetWindow");
        return false;
    }
    for (auto &windowState : windowStates) {
        if (!windowState.isPcOrPadEnableActivation_ && !windowState.isForegroundInteractive_) {
            TAG_LOGI(AAFwkTag::WANTAGENT, "window interactive");
            return false;
        }
    }
    return true;
}

void PendingWantManager::Dump(std::vector<std::string> &info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::WANTAGENT, "dump begin");
    std::string dumpInfo = "    PendingWantRecords:";
    info.push_back(dumpInfo);

    std::lock_guard<ffrt::mutex> locker(mutex_);
    for (const auto &item : wantRecords_) {
        const auto &pendingKey = item.first;
        if (!pendingKey) {
            continue;
        }
        dumpInfo = "        PendWantRecord ID #" + std::to_string(pendingKey->GetCode()) +
            "  type #" + std::to_string(pendingKey->GetType());
        info.push_back(dumpInfo);
        dumpInfo = "        bundle name [" + pendingKey->GetBundleName() + "]";
        info.push_back(dumpInfo);
        dumpInfo = "        result who [" + pendingKey->GetRequestWho() + "]";
        info.push_back(dumpInfo);
        dumpInfo = "        request code #" + std::to_string(pendingKey->GetRequestCode()) +
            "  flags #" + std::to_string(pendingKey->GetFlags());
        info.push_back(dumpInfo);
        dumpInfo = "        resolved type [" + pendingKey->GetRequestResolvedType() + "]";
        info.push_back(dumpInfo);
        dumpInfo = "        Wants:";
        info.push_back(dumpInfo);
        auto Wants = pendingKey->GetAllWantsInfos();
        for (const auto &Want : Wants) {
            dumpInfo = "          uri [" + Want.want.GetElement().GetDeviceID() + "//" +
                Want.want.GetElement().GetBundleName() + "/" + Want.want.GetElement().GetAbilityName() + "]";
            info.push_back(dumpInfo);
            dumpInfo = "          resolved types [" + Want.resolvedTypes + "]";
            info.push_back(dumpInfo);
        }
    }
}
void PendingWantManager::DumpByRecordId(std::vector<std::string> &info, const std::string &args)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::WANTAGENT, "dump by id begin");
    std::string dumpInfo = "    PendingWantRecords:";
    info.push_back(dumpInfo);

    std::lock_guard<ffrt::mutex> locker(mutex_);
    for (const auto &item : wantRecords_) {
        const auto &pendingKey = item.first;
        if (pendingKey && (args == std::to_string(pendingKey->GetCode()))) {
            dumpInfo = "        PendWantRecord ID #" + std::to_string(pendingKey->GetCode()) +
                "  type #" + std::to_string(pendingKey->GetType());
                info.push_back(dumpInfo);
                dumpInfo = "        bundle name [" + pendingKey->GetBundleName() + "]";
                info.push_back(dumpInfo);
                dumpInfo = "        result who [" + pendingKey->GetRequestWho() + "]";
                info.push_back(dumpInfo);
                dumpInfo = "        request code #" + std::to_string(pendingKey->GetRequestCode()) +
                "  flags #" + std::to_string(pendingKey->GetFlags());
                info.push_back(dumpInfo);
            dumpInfo = "        resolved type [" + pendingKey->GetRequestResolvedType() + "]";
            info.push_back(dumpInfo);
            dumpInfo = "        Wants:";
            info.push_back(dumpInfo);
            auto Wants = pendingKey->GetAllWantsInfos();
            for (const auto& Want : Wants) {
                dumpInfo = "          uri [" + Want.want.GetElement().GetDeviceID() + "//" +
                    Want.want.GetElement().GetBundleName() + "/" + Want.want.GetElement().GetAbilityName() + "]";
                info.push_back(dumpInfo);
                dumpInfo = "          resolved types [" + Want.resolvedTypes + "]";
                info.push_back(dumpInfo);
            }
        }
    }
}

int32_t PendingWantManager::GetAllRunningInstanceKeysByBundleName(
    const std::string &bundleName, std::vector<std::string> &appKeyVec)
{
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "bundle name is empty");
        return ERR_INVALID_VALUE;
    }

    auto appMgr = AppMgrUtil::GetAppMgr();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "app mgr is null");
        return OBJECT_NULL;
    }

    return IN_PROCESS_CALL(appMgr->GetAllRunningInstanceKeysByBundleName(bundleName, appKeyVec));
}
}  // namespace AAFwk
}  // namespace OHOS
