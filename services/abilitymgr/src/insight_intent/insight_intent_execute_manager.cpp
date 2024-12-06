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

#include "insight_intent_execute_manager.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_callback_interface.h"
#include "insight_intent_utils.h"
#include "permission_verification.h"
#include "want_params_wrapper.h"
#include "time_util.h"
#include "res_sched_util.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr size_t INSIGHT_INTENT_EXECUTE_RECORDS_MAX_SIZE = 256;
constexpr char EXECUTE_INSIGHT_INTENT_PERMISSION[] = "ohos.permission.EXECUTE_INSIGHT_INTENT";
}
using namespace AppExecFwk;

void InsightIntentExecuteRecipient::OnRemoteDied(const wptr<OHOS::IRemoteObject> &remote)
{
    TAG_LOGD(AAFwkTag::INTENT, "InsightIntentExecuteRecipient OnRemoteDied, %{public}" PRIu64, intentId_);
    auto object = remote.promote();
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null object");
        return;
    }
    DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->RemoteDied(intentId_);
}

InsightIntentExecuteManager::InsightIntentExecuteManager() = default;

InsightIntentExecuteManager::~InsightIntentExecuteManager() = default;

int32_t InsightIntentExecuteManager::CheckAndUpdateParam(uint64_t key, const sptr<IRemoteObject> &callerToken,
    const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param)
{
    int32_t result = CheckCallerPermission();
    if (result != ERR_OK) {
        return result;
    }
    if (callerToken == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null callerToken");
        return ERR_INVALID_VALUE;
    }
    if (param == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null param");
        return ERR_INVALID_VALUE;
    }
    if (param->bundleName_.empty() || param->moduleName_.empty() || param->abilityName_.empty() ||
        param->insightIntentName_.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid param");
        return ERR_INVALID_VALUE;
    }
    uint64_t intentId = 0;
    result = AddRecord(key, callerToken, param->bundleName_, intentId);
    if (result != ERR_OK) {
        return result;
    }

    param->insightIntentId_ = intentId;
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::CheckAndUpdateWant(Want &want, ExecuteMode executeMode)
{
    int32_t result = IsValidCall(want);
    if (result != ERR_OK) {
        return result;
    }
    uint64_t intentId = 0;
    ElementName elementName = want.GetElement();
    result = AddRecord(0, nullptr, want.GetBundle(), intentId);
    if (result != ERR_OK) {
        return result;
    }

    std::string srcEntry;
    auto ret = AbilityRuntime::InsightIntentUtils::GetSrcEntry(elementName,
        want.GetStringParam(INSIGHT_INTENT_EXECUTE_PARAM_NAME), executeMode, srcEntry);
    if (ret != ERR_OK || srcEntry.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "empty srcEntry");
        return ERR_INVALID_VALUE;
    }
    want.SetParam(INSIGHT_INTENT_SRC_ENTRY, srcEntry);
    want.SetParam(INSIGHT_INTENT_EXECUTE_PARAM_ID, std::to_string(intentId));
    want.SetParam(INSIGHT_INTENT_EXECUTE_PARAM_MODE, executeMode);
    TAG_LOGD(AAFwkTag::INTENT, "check done. insightIntentId: %{public}" PRIu64, intentId);
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::AddRecord(uint64_t key, const sptr<IRemoteObject> &callerToken,
    const std::string &bundleName, uint64_t &intentId)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    intentId = ++intentIdCount_;
    auto record = std::make_shared<InsightIntentExecuteRecord>();
    record->key = key;
    record->state = InsightIntentExecuteState::EXECUTING;
    record->callerToken = callerToken;
    record->bundleName = bundleName;
    if (callerToken != nullptr) {
        record->deathRecipient = sptr<InsightIntentExecuteRecipient>::MakeSptr(intentId);
        callerToken->AddDeathRecipient(record->deathRecipient);
    }

    // replace
    records_[intentId] = record;
    if (intentId > INSIGHT_INTENT_EXECUTE_RECORDS_MAX_SIZE) {
        // save the latest INSIGHT_INTENT_EXECUTE_RECORDS_MAX_SIZE records
        records_.erase(intentId - INSIGHT_INTENT_EXECUTE_RECORDS_MAX_SIZE);
    }
    TAG_LOGD(AAFwkTag::INTENT, "init done, records_ size: %{public}zu", records_.size());
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::RemoveExecuteIntent(uint64_t intentId)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    records_.erase(intentId);
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::ExecuteIntentDone(uint64_t intentId, int32_t resultCode,
    const AppExecFwk::InsightIntentExecuteResult &result)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto findResult = records_.find(intentId);
    if (findResult == records_.end()) {
        TAG_LOGE(AAFwkTag::INTENT, "intent not found, id: %{public}" PRIu64, intentId);
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<InsightIntentExecuteRecord> record = findResult->second;
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null record, id: %{public}" PRIu64, intentId);
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::INTENT, "callback start, id:%{public}" PRIu64, intentId);
    if (record->state != InsightIntentExecuteState::EXECUTING) {
        TAG_LOGW(AAFwkTag::INTENT, "insight intent execute state is not EXECUTING, id:%{public}" PRIu64, intentId);
        return ERR_INVALID_OPERATION;
    }
    record->state = InsightIntentExecuteState::EXECUTE_DONE;
    sptr<IInsightIntentExecuteCallback> remoteCallback = iface_cast<IInsightIntentExecuteCallback>(record->callerToken);
    if (remoteCallback == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "intentExecuteCallback empty");
        return ERR_INVALID_VALUE;
    }
    remoteCallback->OnExecuteDone(record->key, resultCode, result);
    if (record->callerToken != nullptr) {
        record->callerToken->RemoveDeathRecipient(record->deathRecipient);
        record->callerToken = nullptr;
    }
    TAG_LOGD(AAFwkTag::INTENT, "execute done, records_ size: %{public}zu", records_.size());
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::RemoteDied(uint64_t intentId)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto result = records_.find(intentId);
    if (result == records_.end()) {
        TAG_LOGE(AAFwkTag::INTENT, "intent not found, id: %{public}" PRIu64, intentId);
        return ERR_INVALID_VALUE;
    }
    if (result->second == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null intent record , id: %{public}" PRIu64, intentId);
        return ERR_INVALID_VALUE;
    }
    result->second->callerToken = nullptr;
    result->second->state = InsightIntentExecuteState::REMOTE_DIED;
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::GetBundleName(uint64_t intentId, std::string &bundleName) const
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto result = records_.find(intentId);
    if (result == records_.end()) {
        TAG_LOGE(AAFwkTag::INTENT, "intent not found, id: %{public}" PRIu64, intentId);
        return ERR_INVALID_VALUE;
    }
    if (result->second == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null intent record,id: %{public}" PRIu64, intentId);
        return ERR_INVALID_VALUE;
    }
    bundleName = result->second->bundleName;
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::GenerateWant(
    const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param, Want &want)
{
    if (param == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null param");
        return ERR_INVALID_VALUE;
    }
    want.SetElementName("", param->bundleName_, param->abilityName_, param->moduleName_);

    if (param->insightIntentParam_ != nullptr) {
        sptr<AAFwk::IWantParams> pExecuteParams = WantParamWrapper::Box(*param->insightIntentParam_);
        if (pExecuteParams != nullptr) {
            WantParams wantParams;
            wantParams.SetParam(INSIGHT_INTENT_EXECUTE_PARAM_PARAM, pExecuteParams);
            want.SetParams(wantParams);
        }
    }

    std::string srcEntry;
    auto ret = AbilityRuntime::InsightIntentUtils::GetSrcEntry(want.GetElement(), param->insightIntentName_,
        static_cast<AppExecFwk::ExecuteMode>(param->executeMode_), srcEntry);
    if (!srcEntry.empty()) {
        want.SetParam(INSIGHT_INTENT_SRC_ENTRY, srcEntry);
    } else if (ret == ERR_INSIGHT_INTENT_GET_PROFILE_FAILED &&
        param->executeMode_ == AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND) {
        TAG_LOGI(AAFwkTag::INTENT, "insight intent srcEntry invalid, try free install ondemand");
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
        want.AddFlags(Want::FLAG_INSTALL_ON_DEMAND);
    } else {
        TAG_LOGE(AAFwkTag::INTENT, "insight intent srcEntry invalid");
        return ERR_INVALID_VALUE;
    }

    want.SetParam(INSIGHT_INTENT_EXECUTE_PARAM_NAME, param->insightIntentName_);
    want.SetParam(INSIGHT_INTENT_EXECUTE_PARAM_MODE, param->executeMode_);
    want.SetParam(INSIGHT_INTENT_EXECUTE_PARAM_ID, std::to_string(param->insightIntentId_));
    if (param->displayId_ != INVALID_DISPLAY_ID) {
        want.SetParam(Want::PARAM_RESV_DISPLAY_ID, param->displayId_);
        TAG_LOGD(AAFwkTag::INTENT, "Generate want with displayId: %{public}d", param->displayId_);
    }
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::IsValidCall(const Want &want)
{
    std::string insightIntentName = want.GetStringParam(INSIGHT_INTENT_EXECUTE_PARAM_NAME);
    if (insightIntentName.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "empty insightIntentName");
        return ERR_INVALID_VALUE;
    }
    TAG_LOGD(AAFwkTag::INTENT, "insightIntentName: %{public}s", insightIntentName.c_str());

    int32_t ret = CheckCallerPermission();
    if (ret != ERR_OK) {
        return ret;
    }
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::CheckCallerPermission()
{
    bool isSystemAppCall = PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI();
    if (!isSystemAppCall) {
        TAG_LOGE(AAFwkTag::INTENT, "system-api cannot use");
        return ERR_NOT_SYSTEM_APP;
    }

    bool isCallingPerm = PermissionVerification::GetInstance()->VerifyCallingPermission(
        EXECUTE_INSIGHT_INTENT_PERMISSION);
    if (!isCallingPerm) {
        TAG_LOGE(AAFwkTag::INTENT, "permission %{public}s verification failed", EXECUTE_INSIGHT_INTENT_PERMISSION);
        return ERR_PERMISSION_DENIED;
    }
    return ERR_OK;
}
void InsightIntentExecuteManager::SetIntentExemptionInfo(int32_t uid)
{
    std::lock_guard<ffrt::mutex> guard(intentExemptionLock_);
    std::map<int32_t, int64_t>::iterator iter = intentExemptionDeadlineTime_.find(uid);
    intentExemptionDeadlineTime_[uid] = AbilityRuntime::TimeUtil::CurrentTimeMillis();
}

bool InsightIntentExecuteManager::CheckIntentIsExemption(int32_t uid)
{
    std::lock_guard<ffrt::mutex> guard(intentExemptionLock_);
    if (intentExemptionDeadlineTime_.find(uid) != intentExemptionDeadlineTime_.end()) {
        if (AbilityRuntime::TimeUtil::CurrentTimeMillis() - INTENT_EXEMPTION_DURATION <=
            intentExemptionDeadlineTime_[uid]) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "exemption check uid:%{public}d", uid);
            return true;
        } else {
            intentExemptionDeadlineTime_.erase(uid);
            return false;
        }
    }
    return false;
}

std::map<int32_t, int64_t> InsightIntentExecuteManager::GetAllIntentExemptionInfo() const
{
    std::lock_guard<ffrt::mutex> guard(intentExemptionLock_);
    return intentExemptionDeadlineTime_;
}
} // namespace AAFwk
} // namespace OHOS
