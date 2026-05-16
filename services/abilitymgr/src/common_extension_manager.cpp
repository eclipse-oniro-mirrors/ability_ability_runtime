/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "common_extension_manager.h"

#include "ability_cache_manager.h"
#include "ability_util.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "multi_instance_utils.h"
#include "ui_extension_utils.h"
#include "appfreeze_manager.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string FRS_BUNDLE_NAME = "com.ohos.formrenderservice";
const std::string FRS_APP_INDEX = "ohos.extra.param.key.frs_index";
}

CommonExtensionManager::CommonExtensionManager(int userId) : AbilityConnectManager(userId)
{}

CommonExtensionManager::~CommonExtensionManager()
{}

int CommonExtensionManager::AttachAbilityThreadInner(const sptr<IAbilityScheduler> &scheduler,
    const sptr<IRemoteObject> &token)
{
    auto abilityRecord = GetExtensionByTokenFromServiceMap(token);
    if (abilityRecord == nullptr) {
        auto terminatingRecord = GetExtensionByTokenFromTerminatingMap(token);
        if (terminatingRecord != nullptr) {
            TAG_LOGW(AAFwkTag::EXT, "Ability:%{public}s/%{public}s, user:%{public}d",
                terminatingRecord->GetInfoBundleName().c_str(),
                terminatingRecord->GetInfoAbilityName().c_str(), userId_);
        }
        auto tmpRecord = Token::GetAbilityRecordByToken(token);
        if (tmpRecord && tmpRecord != terminatingRecord) {
            TAG_LOGW(AAFwkTag::EXT, "Token:%{public}s/%{public}s, user:%{public}d",
                tmpRecord->GetInfoBundleName().c_str(),
                tmpRecord->GetInfoAbilityName().c_str(), userId_);
        }
    }
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    std::string element = abilityRecord->GetURI();
    TAG_LOGD(AAFwkTag::EXT, "ability:%{public}s", element.c_str());
    abilityRecord->RemoveLoadTimeoutTask();
    AbilityRuntime::FreezeUtil::GetInstance().DeleteLifecycleEvent(token);
    abilityRecord->SetScheduler(scheduler);
    TAG_LOGD(AAFwkTag::EXT, "Inactivate");
    abilityRecord->Inactivate();
    return ERR_OK;
}
}  // namespace AAFwk
}  // namespace OHOS