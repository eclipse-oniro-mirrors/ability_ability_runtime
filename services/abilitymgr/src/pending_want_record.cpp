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

#include "pending_want_record.h"

#include "hilog_tag_wrapper.h"
#include "pending_want_manager.h"
#include "int_wrapper.h"
#include "multi_app_utils.h"
#include "event_report.h"
#include "permission_verification.h"
#include "ability_util.h"

namespace OHOS {
namespace AAFwk {
std::string APP_MULTI_INSTANCE{ "ohos.extra.param.key.appInstance" };
constexpr int32_t INVALID_UID = -1;

void SendTriggerFailedEvent(const Want &want, int32_t appIndex, int32_t userId,
    int32_t errCode, const std::string errMsg)
{
    EventInfo eventInfo;
    eventInfo.bundleName = want.GetElement().GetBundleName();
    eventInfo.moduleName = want.GetElement().GetModuleName();
    eventInfo.abilityName = want.GetElement().GetAbilityName();
    eventInfo.appIndex = appIndex;
    eventInfo.errCode = errCode;
    eventInfo.errMsg = errMsg;
    eventInfo.userId = userId;
    eventInfo.startType = TRIGGER_FAILED;

    EventReport::SendTriggerEvent(EventName::START_ABILITY_ERROR, HISYSEVENT_FAULT, eventInfo);
}

PendingWantRecord::PendingWantRecord()
{}

PendingWantRecord::PendingWantRecord(const std::shared_ptr<PendingWantManager> &pendingWantManager, int32_t uid,
    int32_t callerTokenId, const sptr<IRemoteObject> &callerToken, std::shared_ptr<PendingWantKey> key)
    : uid_(uid), callerTokenId_(callerTokenId), callerToken_(callerToken), pendingWantManager_(pendingWantManager),
    key_(key)
{}

PendingWantRecord::~PendingWantRecord()
{}

void PendingWantRecord::Send(SenderInfo &senderInfo)
{
    SenderInner(senderInfo);
}

void PendingWantRecord::RegisterCancelListener(const sptr<IWantReceiver> &receiver)
{
    std::lock_guard guard(mCancelCallbacksMutex_);
    if (receiver == nullptr) {
        return;
    }
    mCancelCallbacks_.emplace_back(receiver);
}

void PendingWantRecord::UnregisterCancelListener(const sptr<IWantReceiver> &receiver)
{
    std::lock_guard guard(mCancelCallbacksMutex_);
    if (receiver == nullptr) {
        return;
    }
    if (mCancelCallbacks_.size()) {
        auto it = std::find(mCancelCallbacks_.cbegin(), mCancelCallbacks_.cend(), receiver);
        if (it != mCancelCallbacks_.cend()) {
            mCancelCallbacks_.erase(it);
        }
    }
}

int32_t PendingWantRecord::SenderInner(SenderInfo &senderInfo)
{
    std::lock_guard<ffrt::mutex> locker(lock_);
    if (canceled_) {
        if (senderInfo.finishedReceiver != nullptr) {
            Want want;
            WantParams wantParams = {};
            senderInfo.finishedReceiver->PerformReceive(want, senderInfo.code, "canceled", wantParams, false, false, 0);
        }
        TAG_LOGE(AAFwkTag::WANTAGENT, "wantagent has been canceled");
        return ERR_WANTAGENT_CANCELED;
    }

    auto pendingWantManager = pendingWantManager_.lock();
    if (pendingWantManager == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null pendingWantManager");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGI(AAFwkTag::WANTAGENT, "before CancelWantSenderLocked");
    if (((uint32_t)key_->GetFlags() & (uint32_t)Flags::ONE_TIME_FLAG) != 0) {
        pendingWantManager->CancelWantSenderLocked(*this, true);
    }

    Want want;
    BuildSendWant(senderInfo, want);

    bool sendFinish = (senderInfo.finishedReceiver != nullptr);
    int32_t res = ExecuteOperation(pendingWantManager, senderInfo, want);
    TAG_LOGI(AAFwkTag::WANTAGENT, "ExecuteOperation return %{public}d, sendFinish %{public}d", res, sendFinish);
    if (sendFinish && res != ERR_WANTAGENT_CANCELED) {
        WantParams wantParams = {};
        TAG_LOGI(AAFwkTag::WANTAGENT, "start callback");
        senderInfo.finishedReceiver->PerformReceive(want, senderInfo.code, "", wantParams, false, false, 0);
    }
    if (res != NO_ERROR) {
        return ERR_INVALID_VALUE;
    }
    return res;
}

bool PendingWantRecord::IsBundleNotExistForUser0(const Want &want)
{
    if (key_ == nullptr || key_->GetUserId() != 0) {
        return false;
    }
    auto bundleName = want.GetElement().GetBundleName();
    if (bundleName.empty()) {
        bundleName = want.GetBundle();
    }
    if (bundleName.empty()) {
        return false;
    }
    auto bms = AbilityUtil::GetBundleManagerHelper();
    if (bms == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "bms is nullptr");
        return false;
    }
    AppExecFwk::BundleInfo bundleInfo;
    bool exist = IN_PROCESS_CALL(bms->GetBundleInfo(
        bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, 0));
    if (!exist) {
        TAG_LOGI(AAFwkTag::WANTAGENT,
            "bundle %{public}s not installed under user 0, will retry with userId -1",
            bundleName.c_str());
    }
    return !exist;
}

int32_t PendingWantRecord::ExecuteOperation(
    std::shared_ptr<PendingWantManager> pendingWantManager, SenderInfo &senderInfo, Want &want)
{
    TAG_LOGI(AAFwkTag::WANTAGENT, "start ability type:%{public}d, bundleName: %{public}s, ability: %{public}s",
        key_->GetType(), want.GetBundle().c_str(), want.GetElement().GetAbilityName().c_str());
    int32_t res = NO_ERROR;
    switch (key_->GetType()) {
        case static_cast<int32_t>(OperationType::START_ABILITY):
            res = pendingWantManager->PendingWantStartAbility(want, senderInfo.startOptions,
                senderInfo.callerToken, -1, callerUid_, callerTokenId_, key_->GetUserId());
            if (res != NO_ERROR && IsBundleNotExistForUser0(want)) {
                res = pendingWantManager->PendingWantStartAbility(want, senderInfo.startOptions,
                    senderInfo.callerToken, -1, callerUid_, callerTokenId_, -1);
            }
            if (res != NO_ERROR) {
                SendTriggerFailedEvent(want, key_->GetAppIndex(), callerUid_, res, "Trigger Failed");
            }
            break;
        case static_cast<int32_t>(OperationType::START_ABILITIES): {
            std::vector<WantsInfo> allWantsInfos = key_->GetAllWantsInfos();
            if (allWantsInfos.empty()) {
                TAG_LOGE(AAFwkTag::WANTAGENT, "allWantsInfos is empty for START_ABILITIES operation");
                SendTriggerFailedEvent(want, key_->GetAppIndex(), callerUid_, ERR_INVALID_VALUE,
                    "Empty wants infos");
                res = ERR_INVALID_VALUE;
            } else {
                allWantsInfos.back().want = want;
                res = pendingWantManager->PendingWantStartAbilitys(allWantsInfos, senderInfo.startOptions,
                    senderInfo.callerToken, -1, callerUid_, callerTokenId_, key_->GetUserId());
                if (res != NO_ERROR) {
                    SendTriggerFailedEvent(want, key_->GetAppIndex(), callerUid_, res, "Trigger Failed");
                }
            }
            break;
        }
        case static_cast<int32_t>(OperationType::START_SERVICE):
        case static_cast<int32_t>(OperationType::START_FOREGROUND_SERVICE):
            res = pendingWantManager->PendingWantStartAbility(want, nullptr, senderInfo.callerToken,
                -1, callerUid_, callerTokenId_, key_->GetUserId());
            if (res != NO_ERROR && IsBundleNotExistForUser0(want)) {
                res = pendingWantManager->PendingWantStartAbility(want, nullptr, senderInfo.callerToken,
                    -1, callerUid_, callerTokenId_, -1);
            }
            if (res != NO_ERROR) {
                SendTriggerFailedEvent(want, key_->GetAppIndex(), callerUid_, res, "Trigger Failed");
            }
            break;
        case static_cast<int32_t>(OperationType::SEND_COMMON_EVENT):
            res = pendingWantManager->PendingWantPublishCommonEvent(want, senderInfo, callerUid_, callerTokenId_);
            break;
        case static_cast<int32_t>(OperationType::START_SERVICE_EXTENSION):
            res = pendingWantManager->PendingWantStartServiceExtension(want, senderInfo.callerToken);
            break;
        default:
            break;
    }
    return res;
}

int32_t PendingWantRecord::GetAppIndexbyUid(int32_t uid, const std::string bundleName, int32_t &appIndex)
{
    auto bms = AbilityUtil::GetBundleManagerHelper();
    std::string getBundleName;

    if (IN_PROCESS_CALL(bms->GetNameAndIndexForUid(uid, getBundleName, appIndex)) != ERR_OK) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "failed get appIndex for bundleName: %{public}s, uid: %{public}d",
            bundleName.c_str(), uid);
        return ERR_INVALID_VALUE;
    }
    if (getBundleName != bundleName) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "getBundleName: %{public}s not match src BundleName: %{public}s",
            getBundleName.c_str(), bundleName.c_str());
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::WANTAGENT, "getBundleName: %{public}s, srcBundleName: %{public}s, uid: %{public}d, "
        "appIndex: %{public}d", getBundleName.c_str(), bundleName.c_str(), uid, appIndex);
    return ERR_OK;
}

void PendingWantRecord::BuildSendWant(SenderInfo &senderInfo, Want &want)
{
    if (key_->GetAllWantsInfos().size() != 0) {
        want = key_->GetRequestWant();
    }
    uint32_t flags = static_cast<uint32_t>(key_->GetFlags());
    bool immutable = false;
    if (((flags & static_cast<uint32_t>(Flags::CONSTANT_FLAG)) != 0) ||
        ((flags & static_cast<uint32_t>(Flags::ALLOW_CANCEL_FLAG)) != 0)) {
        immutable = true;
    }
    senderInfo.resolvedType = key_->GetRequestResolvedType();
    if (!immutable) {
        want.AddFlags(key_->GetFlags());
    }
    WantParams wantParams = want.GetParams();
    auto sendInfoWantParams = senderInfo.want.GetParams().GetParams();
    for (auto mapIter = sendInfoWantParams.begin(); mapIter != sendInfoWantParams.end(); mapIter++) {
        std::string sendInfoWantParamKey = mapIter->first;
        if (want.GetParams().GetParam(sendInfoWantParamKey) == nullptr) {
            wantParams.SetParam(sendInfoWantParamKey, mapIter->second);
        }
    }

    if (!wantParams.HasParam(Want::PARAM_APP_CLONE_INDEX_KEY)) {
        int32_t appIndex = key_->GetAppIndex();
        if (GetUid() != INVALID_UID && !want.GetBundle().empty()) {
            if (GetAppIndexbyUid(GetUid(), want.GetBundle(), appIndex) != ERR_OK) {
                TAG_LOGE(AAFwkTag::WANTAGENT, "getAppIndex failed, srcBundleName: %{public}s, uid: %{public}d",
                    want.GetBundle().c_str(), GetUid());
            }
        }
        wantParams.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, Integer::Box(appIndex));
    }
    CheckAppInstanceKey(want.GetBundle(), wantParams);
    want.SetParams(wantParams);
}

std::shared_ptr<PendingWantKey> PendingWantRecord::GetKey()
{
    return key_;
}

int32_t PendingWantRecord::GetUid() const
{
    return uid_;
}

void PendingWantRecord::SetCanceled()
{
    canceled_ = true;
}
bool PendingWantRecord::GetCanceled()
{
    return canceled_;
}

void PendingWantRecord::SetCallerUid(const int32_t callerUid)
{
    callerUid_ = callerUid;
}

std::list<sptr<IWantReceiver>> PendingWantRecord::GetCancelCallbacks()
{
    std::lock_guard guard(mCancelCallbacksMutex_);
    return mCancelCallbacks_;
}

void PendingWantRecord::CheckAppInstanceKey(const std::string& bundleName, WantParams &wantParams)
{
    if (key_ == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "pending want key is null");
        return;
    }

    auto currType = key_->GetType();
    if (!(static_cast<int32_t>(OperationType::START_ABILITY) == currType ||
        static_cast<int32_t>(OperationType::START_ABILITIES) == currType)) {
        TAG_LOGD(AAFwkTag::WANTAGENT, "want agent type mismatch");
        return;
    }

    auto appKey = wantParams.GetStringParam(APP_MULTI_INSTANCE);
    if (appKey.empty()) {
        TAG_LOGD(AAFwkTag::WANTAGENT, "want params non-existent app instance value");
        return;
    }

    auto pendingWantManager = pendingWantManager_.lock();
    if (pendingWantManager == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "pending want manager is null");
        return;
    }

    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "bundle name is empty");
        return;
    }

    std::vector<std::string> appKeyVec;
    auto result = pendingWantManager->GetAllRunningInstanceKeysByBundleName(bundleName, appKeyVec);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "get app key error");
        return;
    }

    auto find = std::find(appKeyVec.begin(), appKeyVec.end(), appKey);
    if (find == appKeyVec.end()) {
        TAG_LOGD(AAFwkTag::WANTAGENT, "%{public}s non-existent instance key %{public}s",
            bundleName.c_str(), appKey.c_str());
        wantParams.Remove(APP_MULTI_INSTANCE);
    }
}
}  // namespace AAFwk
}  // namespace OHOS
