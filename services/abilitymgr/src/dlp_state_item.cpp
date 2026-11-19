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

#include "dlp_state_item.h"

#include "global_constant.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string DLP_BUNDLE_NAME = "com.ohos.dlpmanager";
}

DlpStateItem::DlpStateItem(int32_t dlpUid, int32_t dlpPid) : dlpUid_(dlpUid), dlpPid_(dlpPid) {}

DlpStateItem::~DlpStateItem() {}

bool DlpStateItem::AddDlpConnectionState(const std::shared_ptr<AbilityRecord> &record,
    AbilityRuntime::DlpStateData &data)
{
    return HandleDlpConnectionState(record, true, data);
}

bool DlpStateItem::RemoveDlpConnectionState(const std::shared_ptr<AbilityRecord> &record,
    AbilityRuntime::DlpStateData &data)
{
    return HandleDlpConnectionState(record, false, data);
}

int32_t DlpStateItem::GetDlpUid() const
{
    return dlpUid_;
}

int32_t DlpStateItem::GetOpenedAbilitySize() const
{
    return static_cast<int32_t>(dlpAbilities_.size());
}

bool DlpStateItem::HandleDlpConnectionState(const std::shared_ptr<AbilityRecord> &record, bool isAdd,
    AbilityRuntime::DlpStateData &data)
{
    if (!record || !AbilityRuntime::GlobalConstant::IsDlpIndex(record->GetAppIndex())) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "invalid dlp ability");
        return false;
    }

    if (dlpUid_ == 0 || dlpPid_ == 0) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "invalid dlp manager state");
        return false;
    }

    sptr<IRemoteObject> tokenObj = nullptr;
    if (record->GetToken()) {
        tokenObj = record->GetToken()->AsObject();
    }

    if (!tokenObj) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "invalid ability");
        return false;
    }

    const auto &it = std::find_if(dlpAbilities_.begin(), dlpAbilities_.end(), [&tokenObj](const auto &item) {
        return tokenObj == item;
    });

    if (isAdd) {
        if (it != dlpAbilities_.end()) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "dlp ability already reported");
            return false;
        }
        dlpAbilities_.emplace_back(tokenObj);
    } else {
        if (it == dlpAbilities_.end()) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "find failed");
            return false;
        }
        dlpAbilities_.erase(it);
    }

    GenerateDlpStateData(record, data);
    return true;
}

void DlpStateItem::GenerateDlpStateData(
    const std::shared_ptr<AbilityRecord> &dlpAbility, AbilityRuntime::DlpStateData &dlpData)
{
    dlpData.callerUid = dlpUid_;
    dlpData.callerPid = dlpPid_;
    dlpData.callerName = DLP_BUNDLE_NAME;
    dlpData.targetPid = dlpAbility->GetPid();
    dlpData.targetUid = dlpAbility->GetUid();
    dlpData.targetBundleName = dlpAbility->GetAbilityInfo().bundleName;
    dlpData.targetModuleName = dlpAbility->GetAbilityInfo().moduleName;
    dlpData.targetAbilityName = dlpAbility->GetAbilityInfo().name;
}
}  // namespace AAFwk
}  // namespace OHOS
