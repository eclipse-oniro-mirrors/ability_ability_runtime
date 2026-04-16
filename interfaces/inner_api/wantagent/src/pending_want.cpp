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

#include "pending_want.h"

#include "ability_runtime_error_util.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "want_agent_client.h"
#include "want_sender_info.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;
namespace OHOS::AbilityRuntime::WantAgent {
namespace {
    constexpr int32_t NOTEQ = -1;
}

PendingWant::PendingWant(const sptr<AAFwk::IWantSender> &target)
    : target_(target), cancelReceiver_(nullptr), allowListToken_(nullptr)
{}

PendingWant::PendingWant(const sptr<AAFwk::IWantSender> &target, const sptr<IRemoteObject> allowListToken)
    : target_(target), cancelReceiver_(nullptr), allowListToken_(allowListToken)
{}

WantAgentConstant::OperationType PendingWant::GetType(sptr<AAFwk::IWantSender> target)
{
    int32_t operationType = 0;
    WantAgentClient::GetInstance().GetPendingWantType(target, operationType);
    return static_cast<WantAgentConstant::OperationType>(operationType);
}

std::shared_ptr<PendingWant> PendingWant::GetAbility(
    const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context,
    int requestCode, const std::shared_ptr<Want> &want, unsigned int flags)
{
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    GetAbility(context, requestCode, want, flags, nullptr, pendingWant);
    return pendingWant;
}

ErrCode PendingWant::GetAbility(
    const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context, int requestCode,
    const std::shared_ptr<AAFwk::Want> &want, unsigned int flags,
    const std::shared_ptr<AAFwk::WantParams> &options,
    std::shared_ptr<PendingWant> &pendingWant, int userId)
{
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "input param want is nullptr");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }
    WantsInfo wantsInfo;
    wantsInfo.want = *want;
    wantsInfo.resolvedTypes = want != nullptr ? want->GetType() : "";
    if (options != nullptr && !options->IsEmpty()) {
        wantsInfo.want.SetParams(*options);
    }

    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = static_cast<int32_t>(WantAgentConstant::OperationType::START_ABILITY);
    wantSenderInfo.allWants.push_back(wantsInfo);
    if (userId >= 0) {
        wantSenderInfo.bundleName = want->GetOperation().GetBundleName();
    } else {
        wantSenderInfo.bundleName = context->GetBundleName();
    }
    wantSenderInfo.flags = flags;
    wantSenderInfo.userId = userId;
    wantSenderInfo.requestCode = requestCode;
    sptr<IWantSender> target = nullptr;
    ErrCode result = WantAgentClient::GetInstance().GetWantSender(wantSenderInfo, nullptr, target);
    if (result != ERR_OK) {
        return result;
    }
    pendingWant = std::make_shared<PendingWant>(target);
    return ERR_OK;
}

std::shared_ptr<PendingWant> PendingWant::GetAbilities(
    const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context, int requestCode,
    std::vector<std::shared_ptr<Want>> &wants, unsigned int flags)
{
    std::shared_ptr<PendingWant> pendingWant = nullptr;
    GetAbilities(context, requestCode, wants, flags, nullptr, pendingWant);
    return pendingWant;
}

ErrCode PendingWant::GetAbilities(
    const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context, int requestCode,
    std::vector<std::shared_ptr<Want>> &wants, unsigned int flags, const std::shared_ptr<WantParams> &options,
    std::shared_ptr<PendingWant> &pendingWant)
{
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = static_cast<int32_t>(WantAgentConstant::OperationType::START_ABILITIES);
    wantSenderInfo.bundleName = context->GetBundleName();
    wantSenderInfo.flags = flags;
    wantSenderInfo.userId = -1; // -1 : invalid user id
    wantSenderInfo.requestCode = requestCode;
    for (auto want : wants) {
        WantsInfo wantsInfo;
        if (want != nullptr) {
            wantsInfo.want = *want;
        }
        wantsInfo.resolvedTypes = want != nullptr ? want->GetType() : "";
        if (options != nullptr && !options->IsEmpty()) {
            wantsInfo.want.SetParams(*options);
        }
        wantSenderInfo.allWants.push_back(wantsInfo);
    }
    sptr<IWantSender> target = nullptr;
    ErrCode result = WantAgentClient::GetInstance().GetWantSender(wantSenderInfo, nullptr, target);
    if (result != ERR_OK) {
        return result;
    }
    pendingWant = std::make_shared<PendingWant>(target);
    return ERR_OK;
}

ErrCode PendingWant::GetCommonEvent(
    const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context,
    int requestCode, const std::shared_ptr<Want> &want, unsigned int flags,
    std::shared_ptr<PendingWant> &pendingWant)
{
    return GetCommonEventAsUser(context, requestCode, want, flags, 0, pendingWant);
}

ErrCode PendingWant::GetCommonEventAsUser(
    const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context,
    int requestCode, const std::shared_ptr<Want> &want, unsigned int flags, int uid,
    std::shared_ptr<PendingWant> &pendingWant)
{
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    WantsInfo wantsInfo;
    if (want != nullptr) {
        wantsInfo.want = *want;
    }
    wantsInfo.resolvedTypes = want != nullptr ? want->GetType() : "";

    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = static_cast<int32_t>(WantAgentConstant::OperationType::SEND_COMMON_EVENT);
    wantSenderInfo.allWants.push_back(wantsInfo);
    wantSenderInfo.bundleName = context->GetBundleName();
    wantSenderInfo.flags = flags;
    wantSenderInfo.userId = -1; // -1 : invalid user id
    wantSenderInfo.requestCode = requestCode;
    sptr<IWantSender> target = nullptr;
    ErrCode result = WantAgentClient::GetInstance().GetWantSender(wantSenderInfo, nullptr, target);
    if (result != ERR_OK) {
        return result;
    }
    pendingWant = std::make_shared<PendingWant>(target);
    return ERR_OK;
}

ErrCode PendingWant::GetService(
    const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context,
    int requestCode, const std::shared_ptr<AAFwk::Want> &want, unsigned int flags,
    std::shared_ptr<PendingWant> &pendingWant)
{
    return BuildServicePendingWant(context, requestCode, want, flags,
        WantAgentConstant::OperationType::START_SERVICE, pendingWant);
}

ErrCode PendingWant::GetServiceExtension(
    const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context,
    int requestCode, const std::shared_ptr<AAFwk::Want> &want, unsigned int flags,
    std::shared_ptr<PendingWant> &pendingWant)
{
    return BuildServicePendingWant(context, requestCode, want, flags,
        WantAgentConstant::OperationType::START_SERVICE_EXTENSION, pendingWant);
}

ErrCode PendingWant::GetForegroundService(
    const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context, int requestCode,
    const std::shared_ptr<Want> &want, unsigned int flags,
    std::shared_ptr<PendingWant> &pendingWant)
{
    return BuildServicePendingWant(
        context, requestCode, want, flags, WantAgentConstant::OperationType::START_FOREGROUND_SERVICE,
        pendingWant);
}

ErrCode PendingWant::BuildServicePendingWant(
    const std::shared_ptr<OHOS::AbilityRuntime::ApplicationContext> &context,
    int requestCode, const std::shared_ptr<Want> &want,
    unsigned int flags, WantAgentConstant::OperationType serviceKind,
    std::shared_ptr<PendingWant> &pendingWant)
{
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    WantsInfo wantsInfo;
    if (want != nullptr) {
        wantsInfo.want = *want;
    }
    wantsInfo.resolvedTypes = want != nullptr ? want->GetType() : "";

    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = static_cast<int32_t>(serviceKind);
    wantSenderInfo.allWants.push_back(wantsInfo);
    wantSenderInfo.bundleName = context->GetBundleName();
    wantSenderInfo.flags = flags;
    wantSenderInfo.userId = -1; // -1 : invalid user id
    wantSenderInfo.requestCode = requestCode;
    sptr<IWantSender> target = nullptr;
    ErrCode result = WantAgentClient::GetInstance().GetWantSender(wantSenderInfo, nullptr, target);
    if (result != ERR_OK) {
        return result;
    }
    pendingWant = std::make_shared<PendingWant>(target);
    return ERR_OK;
}

ErrCode PendingWant::Cancel(const sptr<AAFwk::IWantSender> &target, uint32_t flags)
{
    return WantAgentClient::GetInstance().CancelWantSender(target, flags);
}

void PendingWant::Send(
    int resultCode, sptr<CompletedDispatcher> &onCompleted, const sptr<AAFwk::IWantSender> &target)
{
    Send(resultCode, nullptr, onCompleted, "", nullptr, nullptr, target, nullptr);
}

void PendingWant::Send(int resultCode, const std::shared_ptr<Want> &want,
    sptr<CompletedDispatcher> &onCompleted, const sptr<AAFwk::IWantSender> &target)
{
    Send(resultCode, want, onCompleted, "", nullptr, nullptr, target, nullptr);
}

void PendingWant::Send(int resultCode, const std::shared_ptr<Want> &want,
    sptr<CompletedDispatcher> &onCompleted, const std::string &requiredPermission,
    const sptr<AAFwk::IWantSender> &target)
{
    Send(resultCode, want, onCompleted, requiredPermission, nullptr, nullptr, target, nullptr);
}

ErrCode PendingWant::Send(int resultCode, const std::shared_ptr<Want> &want,
    sptr<CompletedDispatcher> &onCompleted, const std::string &requiredPermission,
    const std::shared_ptr<WantParams> &options, const std::shared_ptr<StartOptions> &startOptions,
    const sptr<AAFwk::IWantSender> &target, sptr<IRemoteObject> callerToken)
{
    int result =
        SendAndReturnResult(resultCode, want, onCompleted, requiredPermission, options,
            startOptions, target, callerToken);
    return result;
}

int PendingWant::SendAndReturnResult(int resultCode, const std::shared_ptr<Want> &want,
    sptr<CompletedDispatcher> &onCompleted, const std::string &requiredPermission,
    const std::shared_ptr<WantParams> &options, const std::shared_ptr<StartOptions> &startOptions,
    const sptr<AAFwk::IWantSender> &target, sptr<IRemoteObject> callerToken)
{
    TAG_LOGD(AAFwkTag::WANTAGENT, "call");
    SenderInfo senderInfo;
    senderInfo.resolvedType = want != nullptr ? want->GetType() : "";
    if (want != nullptr) {
        senderInfo.want = *want;
    }
    if (options != nullptr) {
        senderInfo.want.SetParams(*options);
    }
    if (startOptions != nullptr) {
        senderInfo.startOptions = new (std::nothrow) StartOptions(*startOptions);
    }
    senderInfo.requiredPermission = requiredPermission;
    senderInfo.code = resultCode;
    senderInfo.finishedReceiver = onCompleted;
    senderInfo.callerToken = callerToken;
    int res = WantAgentClient::GetInstance().SendWantSender(target, senderInfo);
    if (senderInfo.finishedReceiver == nullptr) {
        return res;
    }
    sptr<IRemoteObject> obj = senderInfo.finishedReceiver->AsObject();
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "finishedReceiver obj null");
        return res;
    }
    if (onCompleted != nullptr) {
        onCompleted = static_cast<CompletedDispatcher*>(senderInfo.finishedReceiver.GetRefPtr());
    }
    return res;
}

ErrCode PendingWant::IsEquals(
    const std::shared_ptr<PendingWant> &targetPendingWant, const std::shared_ptr<PendingWant> &otherPendingWant)
{
    if ((targetPendingWant == nullptr) && (otherPendingWant == nullptr)) {
        return ERR_OK;
    }
    if ((targetPendingWant == nullptr) || (otherPendingWant == nullptr)) {
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }
    int targetCode = -1;
    int otherCode = -1;
    ErrCode targetErrCode = targetPendingWant->GetHashCode(targetPendingWant->GetTarget(), targetCode);
    ErrCode otherErrCode = otherPendingWant->GetHashCode(otherPendingWant->GetTarget(), otherCode);
    if (targetErrCode != ERR_OK) {
        return targetErrCode;
    }
    if (otherErrCode != ERR_OK) {
        return otherErrCode;
    }
    return targetCode == otherCode ? ERR_OK : NOTEQ;
}

sptr<IWantSender> PendingWant::GetTarget()
{
    return target_;
}

void PendingWant::SetTarget(const sptr<AAFwk::IWantSender> &target)
{
    target_ = target;
}

PendingWant::CancelReceiver::CancelReceiver(const std::weak_ptr<PendingWant> &outerInstance)
    : outerInstance_(outerInstance)
{}

void PendingWant::CancelReceiver::PerformReceive(const AAFwk::Want &want, int resultCode, const std::string &data,
    const AAFwk::WantParams &extras, bool serialized, bool sticky, int sendingUser)
{}

void PendingWant::CancelReceiver::Send(const int32_t resultCode)
{
    if (outerInstance_.lock() != nullptr) {
        outerInstance_.lock()->NotifyCancelListeners(resultCode);
    }
}

void PendingWant::RegisterCancelListener(
    const std::shared_ptr<CancelListener> &cancelListener, const sptr<AAFwk::IWantSender> &target)
{
    if (cancelListener == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return;
    }
    std::scoped_lock<std::mutex> lock(lock_object);
    if (cancelReceiver_ == nullptr) {
        cancelReceiver_ = new (std::nothrow) CancelReceiver(weak_from_this());
    }
    bool isEmpty = cancelListeners_.empty();
    cancelListeners_.push_back(cancelListener);
    if (isEmpty) {
        WantAgentClient::GetInstance().RegisterCancelListener(target, cancelReceiver_);
    }
}

void PendingWant::NotifyCancelListeners(int32_t resultCode)
{
    std::vector<std::shared_ptr<CancelListener>> cancelListeners;
    {
        std::scoped_lock<std::mutex> lock(lock_object);
        cancelListeners = std::vector<std::shared_ptr<CancelListener>>(cancelListeners_);
    }
    for (auto cancelListener : cancelListeners) {
        if (cancelListener != nullptr) {
            cancelListener->OnCancelled(resultCode);
        }
    }
}

void PendingWant::UnregisterCancelListener(
    const std::shared_ptr<CancelListener> &cancelListener, const sptr<AAFwk::IWantSender> &target)
{
    if (cancelListener == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "invalid input param");
        return;
    }

    std::scoped_lock<std::mutex> lock(lock_object);
    bool isEmpty = cancelListeners_.empty();
    cancelListeners_.erase(remove_if(cancelListeners_.begin(),
        cancelListeners_.end(),
        [cancelListener](std::shared_ptr<CancelListener> x) { return x == cancelListener; }),
        cancelListeners_.end());
    if (cancelListeners_.empty() && !isEmpty) {
        WantAgentClient::GetInstance().UnregisterCancelListener(target, cancelReceiver_);
    }
}

ErrCode PendingWant::GetHashCode(const sptr<AAFwk::IWantSender> &target, int &code)
{
    return WantAgentClient::GetInstance().GetPendingWantCode(target, code);
}

ErrCode PendingWant::GetUid(const sptr<AAFwk::IWantSender> &target, int32_t &uid)
{
    return WantAgentClient::GetInstance().GetPendingWantUid(target, uid);
}

ErrCode PendingWant::GetBundleName(const sptr<AAFwk::IWantSender> &target, std::string &bundleName)
{
    return WantAgentClient::GetInstance().GetPendingWantBundleName(target, bundleName);
}

std::shared_ptr<Want> PendingWant::GetWant(const sptr<AAFwk::IWantSender> &target)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::shared_ptr<Want> want = std::make_shared<Want>();
    int ret = WantAgentClient::GetInstance().GetPendingRequestWant(target, want);
    return ret ? nullptr : want;
}

std::shared_ptr<Want> PendingWant::GetWantFromProxy(const sptr<AAFwk::IWantSender> &target)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::shared_ptr<Want> want = std::make_shared<Want>();
    int ret = WantAgentClient::GetInstance().GetPendingRequestWantFromProxy(target, want);
    return ret ? nullptr : want;
}

bool PendingWant::Marshalling(Parcel &parcel) const
{
    if (target_ == nullptr || !(static_cast<MessageParcel*>(&parcel))->WriteRemoteObject(target_->AsObject())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "failed");
        return false;
    }

    return true;
}

PendingWant *PendingWant::Unmarshalling(Parcel &parcel)
{
    PendingWant *pendingWant = new (std::nothrow) PendingWant();
    if (pendingWant == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "read from parcel failed");
        return nullptr;
    }
    sptr<AAFwk::IWantSender> target =
        iface_cast<AAFwk::IWantSender>((static_cast<MessageParcel*>(&parcel))->ReadRemoteObject());
    if (target == nullptr) {
        delete pendingWant;
        return nullptr;
    }
    pendingWant->SetTarget(target);

    return pendingWant;
}

std::shared_ptr<WantSenderInfo> PendingWant::GetWantSenderInfo(const sptr<AAFwk::IWantSender> &target)
{
    std::shared_ptr<WantSenderInfo> info = std::make_shared<WantSenderInfo>();
    int ret = WantAgentClient::GetInstance().GetWantSenderInfo(target, info);
    return ret ? nullptr : info;
}

ErrCode PendingWant::GetType(const sptr<AAFwk::IWantSender> &target, int32_t &operType)
{
    ErrCode result = WantAgentClient::GetInstance().GetPendingWantType(target, operType);
    return result;
}

ErrCode PendingWant::GetWant(const sptr<AAFwk::IWantSender> &target, std::shared_ptr<AAFwk::Want> &want)
{
    ErrCode result = WantAgentClient::GetInstance().GetPendingRequestWant(target, want);
    return result;
}
}  // namespace OHOS::AbilityRuntime::WantAgent
