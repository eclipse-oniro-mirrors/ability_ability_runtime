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

#include "agent_load_callback.h"

#include "agent_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AgentRuntime {
void AgentLoadCallback::OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    if (systemAbilityId != AGENT_MGR_SERVICE_ID) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "System ability id %{public}d mismatch", systemAbilityId);
        return;
    }

    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Object is nullptr");
        return;
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "Load system ability %{public}d succeed", systemAbilityId);
    AgentManagerClient::GetInstance().OnLoadSystemAbilitySuccess(remoteObject);
}

void AgentLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    if (systemAbilityId != AGENT_MGR_SERVICE_ID) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "System ability id %{public}d mismatch", systemAbilityId);
        return;
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "Load system ability %{public}d failed", systemAbilityId);
    AgentManagerClient::GetInstance().OnLoadSystemAbilityFail();
}
}  // namespace AgentRuntime
}  // namespace OHOS
