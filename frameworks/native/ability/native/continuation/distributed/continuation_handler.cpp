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
#include "continuation_handler.h"

#include "ability_manager_client.h"
#include "distributed_errors.h"
#include "element_name.h"
#include "hilog_tag_wrapper.h"

using OHOS::AAFwk::WantParams;
namespace OHOS {
namespace AppExecFwk {
const std::string ContinuationHandler::ORIGINAL_DEVICE_ID("deviceId");
const std::string VERSION_CODE_KEY = "version";
const std::string FEATURE_ABILITY_FLAG_KEY = "ohos.dms.faFlag";
ContinuationHandler::ContinuationHandler(
    std::weak_ptr<ContinuationManager> &continuationManager, std::weak_ptr<Ability> &ability)
{
    ability_ = ability;
    continuationManager_ = continuationManager;
}

bool ContinuationHandler::HandleStartContinuationWithStack(const sptr<IRemoteObject> &token,
    const std::string &deviceId, uint32_t versionCode)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "called");
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null token");
        return false;
    }
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null abilityInfo");
        return false;
    }

    abilityInfo_->deviceId = deviceId;

    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null continuationManagerTmp");
        return false;
    }

    // decided to start continuation. Callback to ability.
    Want want;
    want.SetParam(VERSION_CODE_KEY, static_cast<int32_t>(versionCode));
    want.SetParam("targetDevice", deviceId);
    WantParams wantParams = want.GetParams();
    int32_t status = continuationManagerTmp->OnContinue(wantParams);
    if (status != ERR_OK) {
        TAG_LOGW(AAFwkTag::CONTINUATION,
            "OnContinue failed, BundleName = %{public}s, ClassName= %{public}s, status: %{public}d",
            abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str(),
            status);
    }

    want.SetParams(wantParams);
    want.AddFlags(want.FLAG_ABILITY_CONTINUATION);
    want.SetElementName(deviceId, abilityInfo_->bundleName, abilityInfo_->name, abilityInfo_->moduleName);

    int result = AAFwk::AbilityManagerClient::GetInstance()->StartContinuation(want, token, status);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "startContinuation failed");
        return false;
    }
    return true;
}

bool ContinuationHandler::HandleStartContinuation(const sptr<IRemoteObject> &token, const std::string &deviceId)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "called");
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null token");
        return false;
    }
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null abilityInfo");
        return false;
    }

    abilityInfo_->deviceId = deviceId;

    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null continuationManagerTmp");
        return false;
    }

    // DMS decided to start continuation. Callback to ability.
    if (!continuationManagerTmp->StartContinuation()) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "ID_ABILITY_SHELL_CONTINUE_ABILITY, BundleName = %{public}s, ClassName= %{public}s",
            abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str());
        return false;
    }

    WantParams wantParams;
    if (!continuationManagerTmp->SaveData(wantParams)) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "ID_ABILITY_SHELL_CONTINUE_ABILITY, BundleName = %{public}s, ClassName= %{public}s",
            abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str());
        return false;
    }

    Want want = SetWantParams(wantParams);
    want.SetElementName(deviceId, abilityInfo_->bundleName, abilityInfo_->name, abilityInfo_->moduleName);
    want.SetParam(FEATURE_ABILITY_FLAG_KEY, true);

    int result = AAFwk::AbilityManagerClient::GetInstance()->StartContinuation(want, token, 0);
    if (result != 0) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "startContinuation failed");
        return false;
    }
    return true;
}

void ContinuationHandler::HandleReceiveRemoteScheduler(const sptr<IRemoteObject> &remoteReplica)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "begin");
    if (remoteReplica == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null remoteReplica");
        return;
    }

    if (remoteReplicaProxy_ != nullptr && schedulerDeathRecipient_ != nullptr) {
        auto schedulerObjectTmp = remoteReplicaProxy_->AsObject();
        if (schedulerObjectTmp != nullptr) {
            schedulerObjectTmp->RemoveDeathRecipient(schedulerDeathRecipient_);
        }
    }

    if (schedulerDeathRecipient_ == nullptr) {
        schedulerDeathRecipient_ = new (std::nothrow) ReverseContinuationSchedulerRecipient(
            [this](const wptr<IRemoteObject> &arg) { this->OnReplicaDied(arg); });
    }

    remoteReplicaProxy_ = iface_cast<IReverseContinuationSchedulerReplica>(remoteReplica);
    auto schedulerObject = remoteReplicaProxy_->AsObject();
    if (schedulerObject == nullptr || !schedulerObject->AddDeathRecipient(schedulerDeathRecipient_)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null schedulerObject");
    }

    remoteReplicaProxy_->PassPrimary(remotePrimaryStub_);
}

void ContinuationHandler::HandleCompleteContinuation(int result)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "begin");
    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null continuationManagerTmp");
        return;
    }

    continuationManagerTmp->CompleteContinuation(result);
}

void ContinuationHandler::SetReversible(bool reversible)
{
    reversible_ = reversible;
}

void ContinuationHandler::SetAbilityInfo(std::shared_ptr<AbilityInfo> &abilityInfo)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "begin");
    abilityInfo_ = std::make_shared<AbilityInfo>(*(abilityInfo.get()));
    ClearDeviceInfo(abilityInfo_);
}

void ContinuationHandler::SetPrimaryStub(const sptr<IRemoteObject> &Primary)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "called");
    remotePrimaryStub_ = Primary;
}

void ContinuationHandler::ClearDeviceInfo(std::shared_ptr<AbilityInfo> &abilityInfo)
{
    abilityInfo->deviceId = "";
    abilityInfo->deviceTypes.clear();
}

void ContinuationHandler::OnReplicaDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "begin");
    if (remoteReplicaProxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null remoteReplicaProxy_");
        return;
    }

    auto object = remote.promote();
    if (!object) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null object");
        return;
    }

    if (object != remoteReplicaProxy_->AsObject()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "not matches with remote");
        return;
    }

    if (remoteReplicaProxy_ != nullptr && schedulerDeathRecipient_ != nullptr) {
        auto schedulerObject = remoteReplicaProxy_->AsObject();
        if (schedulerObject != nullptr) {
            schedulerObject->RemoveDeathRecipient(schedulerDeathRecipient_);
        }
    }
    remoteReplicaProxy_.clear();

    NotifyReplicaTerminated();
}

void ContinuationHandler::NotifyReplicaTerminated()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "begin");

    CleanUpAfterReverse();

    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null continuationManagerTmp");
        return;
    }
    continuationManagerTmp->NotifyRemoteTerminated();
}

Want ContinuationHandler::SetWantParams(const WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "begin");
    Want want;
    want.SetParams(wantParams);
    want.AddFlags(want.FLAG_ABILITY_CONTINUATION);
    if (abilityInfo_->launchMode != LaunchMode::STANDARD) {
        TAG_LOGD(AAFwkTag::CONTINUATION, "Clear task");
    }
    if (reversible_) {
        TAG_LOGD(AAFwkTag::CONTINUATION, "SetWantParams: Reversible");
        want.AddFlags(Want::FLAG_ABILITY_CONTINUATION_REVERSIBLE);
    }
    ElementName element("", abilityInfo_->bundleName, abilityInfo_->name, abilityInfo_->moduleName);
    want.SetElement(element);
    return want;
}

void ContinuationHandler::CleanUpAfterReverse()
{
    remoteReplicaProxy_ = nullptr;
}

void ContinuationHandler::PassPrimary(const sptr<IRemoteObject> &Primary)
{
    remotePrimaryProxy_ = iface_cast<IReverseContinuationSchedulerPrimary>(Primary);
}

bool ContinuationHandler::ReverseContinuation()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "begin");

    if (remotePrimaryProxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null remotePrimaryProxy_");
        return false;
    }

    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null abilityInfo");
        return false;
    }

    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null continuationManagerTmp");
        return false;
    }

    if (!continuationManagerTmp->StartContinuation()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "BundleName = %{public}s, ClassName= %{public}s",
            abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str());
        return false;
    }

    WantParams wantParams;
    if (!continuationManagerTmp->SaveData(wantParams)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "bundleName = %{public}s, className= %{public}s",
            abilityInfo_->bundleName.c_str(), abilityInfo_->name.c_str());
        return false;
    }

    Want want;
    want.SetParams(wantParams);
    if (remotePrimaryProxy_->ContinuationBack(want)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationBack send failed");
        return false;
    }
    return true;
}

void ContinuationHandler::NotifyReverseResult(int reverseResult)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "result = %{public}d", reverseResult);
    if (reverseResult == 0) {
        std::shared_ptr<Ability> ability = nullptr;
        ability = ability_.lock();
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::CONTINUATION, "null ability");
            return;
        }
        ability->TerminateAbility();
    }
}

bool ContinuationHandler::ContinuationBack(const Want &want)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "begin");
    std::shared_ptr<ContinuationManager> continuationManagerTmp = nullptr;
    continuationManagerTmp = continuationManager_.lock();
    if (continuationManagerTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null continuationManagerTmp");
        return false;
    }

    int result = 0;
    if (!continuationManagerTmp->RestoreFromRemote(want.GetParams())) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "RestoreFromRemote failed");
        result = ABILITY_FAILED_RESTORE_DATA;
    }

    remoteReplicaProxy_->NotifyReverseResult(result);
    if (result == 0) {
        CleanUpAfterReverse();
    }
    return true;
}

void ContinuationHandler::NotifyTerminationToPrimary()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "begin");
    if (remotePrimaryProxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null remotePrimaryProxy_");
        return;
    }

    remotePrimaryProxy_->NotifyReplicaTerminated();
}

bool ContinuationHandler::ReverseContinueAbility()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "begin");
    if (remoteReplicaProxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null remotePrimaryProxy_");
        return false;
    }

    bool requestSendSuccess = remoteReplicaProxy_->ReverseContinuation();
    return requestSendSuccess;
}
}  // namespace AppExecFwk
}  // namespace OHOS
