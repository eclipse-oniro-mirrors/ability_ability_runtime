/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "utils/oe_extension_utils.h"

#include "ability_manager_errors.h"
#include "ability_record.h"
#include "app_scheduler.h"
#include "ffrt.h"
#include "hilog_tag_wrapper.h"
#include "startup_util.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t OESA_UID = 7061;
}

OEExtensionUtils &OEExtensionUtils::GetInstance()
{
    static OEExtensionUtils instance;
    return instance;
}

int32_t OEExtensionUtils::ValidateCaller(
    int32_t callingUid,
    const Want &want,
    const sptr<IRemoteObject> &callerToken,
    int32_t hostPid,
    std::string &hostBundleName,
    int32_t &userId)
{
    if (callingUid != OESA_UID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid calling uid: %{public}d, expected: %{public}d", callingUid, OESA_UID);
        return CHECK_PERMISSION_FAILED;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid caller token");
        return ERR_INVALID_CALLER;
    }

    const auto &abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo.type != AppExecFwk::AbilityType::EXTENSION ||
        abilityInfo.extensionAbilityType != AppExecFwk::ExtensionAbilityType::CONTENT_EMBED) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "caller is not oe extension");
        return ERR_INVALID_CALLER;
    }

    if (want.GetBundle() != abilityInfo.bundleName) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want bundleName %{public}s does not match caller bundleName %{public}s",
            want.GetBundle().c_str(), abilityInfo.bundleName.c_str());
        return INVALID_PARAMETERS_ERR;
    }

    if (want.GetElement().GetAbilityName().empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want abilityName is empty");
        return INVALID_PARAMETERS_ERR;
    }

    AppExecFwk::RunningProcessInfo processInfo;
    DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByPid(hostPid, processInfo);
    if (processInfo.state_ != AppExecFwk::AppProcessState::APP_STATE_FOREGROUND) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "hostPid=%{public}d not foreground: %{public}d", hostPid, processInfo.state_);
        return NOT_TOP_ABILITY;
    }

    int32_t appIndex = 0;
    if (!AbilityRuntime::StartupUtil::GetAppIndex(want, appIndex) ||
        (appIndex != 0 && appIndex != abilityRecord->GetAppIndex())) {
        return ERR_APP_CLONE_INDEX_INVALID;
    }
    const_cast<Want &>(want).SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, appIndex);

    hostBundleName = processInfo.bundleNames.empty() ? "" : processInfo.bundleNames[0];
    userId = abilityRecord->GetOwnerMissionUserId();
    return ERR_OK;
}

void OEExtensionUtils::AddOEExtRequest(int32_t requestId)
{
    ScheduleDelayedCleanup(requestId);
    std::lock_guard<std::mutex> lock(oeExtRequestsMutex_);
    oeExtRequests_.insert(requestId);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Add OE extension request, requestId=%{public}d", requestId);
}

bool OEExtensionUtils::RemoveOEExtRequest(int32_t requestId)
{
    std::lock_guard<std::mutex> lock(oeExtRequestsMutex_);
    return oeExtRequests_.erase(requestId) > 0;
}

void OEExtensionUtils::ScheduleDelayedCleanup(int32_t requestId)
{
    auto cleanupTask = [requestId]() {
        if (OEExtensionUtils::GetInstance().RemoveOEExtRequest(requestId)) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "Remove OE extension request, requestId=%{public}d", requestId);
        }
    };
    constexpr int32_t cleanupDelay = 10 * 1000 * 1000;
    ffrt::submit(std::move(cleanupTask), ffrt::task_attr().delay(cleanupDelay).name("OERequestCleanup"));
}

} // namespace AAFwk
} // namespace OHOS
