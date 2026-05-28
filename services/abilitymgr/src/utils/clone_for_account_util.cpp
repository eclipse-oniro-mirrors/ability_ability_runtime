/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "clone_for_account_util.h"

#include "ability_util.h"
#include "ability_record.h"
#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t INVALID_APP_INDEX = -1;
}

std::mutex CloneForAccountUtil::mapMutex_;
std::unordered_map<std::string, int32_t> CloneForAccountUtil::appIndexMap_;

bool CloneForAccountUtil::GetCachedAppIndex(const std::string &bundleName, int32_t &appIndex)
{
    if (bundleName.empty()) {
        return false;
    }
    std::lock_guard lock(mapMutex_);
    auto it = appIndexMap_.find(bundleName);
    if (it != appIndexMap_.end()) {
        appIndex = it->second;
        return true;
    }
    return false;
}

void CloneForAccountUtil::CacheAppIndex(const std::string &bundleName, int32_t appIndex)
{
    if (bundleName.empty()) {
        return;
    }
    std::lock_guard lock(mapMutex_);
    appIndexMap_[bundleName] = appIndex;
}

void CloneForAccountUtil::RemoveCachedAppIndex(const std::string &bundleName)
{
    if (bundleName.empty()) {
        return;
    }
    std::lock_guard lock(mapMutex_);
    appIndexMap_.erase(bundleName);
}

void CloneForAccountUtil::ProcessAppIndex(Want &want, sptr<IRemoteObject> callerToken, int32_t userId)
{
    // Step 1: Read appIndex from want
    int32_t appIndex = INVALID_APP_INDEX;
    if (want.HasParameter(Want::PARAM_APP_CLONE_INDEX_KEY)) {
        appIndex = want.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, INVALID_APP_INDEX);
    }

    // Step 2: Remove appIndex from want
    want.RemoveParam(Want::PARAM_APP_CLONE_INDEX_KEY);

    // Step 3: If want has no appIndex, get from caller
    if (appIndex == INVALID_APP_INDEX && callerToken != nullptr) {
        auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
        if (abilityRecord != nullptr) {
            appIndex = abilityRecord->GetAppIndex();
        }
    }

    // Step 4: Query enabled ability info with appIndex as candidate
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleMgrHelper is nullptr");
        return;
    }
    AppExecFwk::AbilityInfo abilityInfo;
    if (!IN_PROCESS_CALL(bundleMgrHelper->QueryEnabledAbilityInfo(want, userId, appIndex, abilityInfo))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "QueryEnabledAbilityInfo failed");
        return;
    }

    // Step 5: Write resolved appIndex back to want
    want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, abilityInfo.appIndex);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CloneForAccount resolved appIndex: %{public}d", abilityInfo.appIndex);
}
}  // namespace AAFwk
}  // namespace OHOS
