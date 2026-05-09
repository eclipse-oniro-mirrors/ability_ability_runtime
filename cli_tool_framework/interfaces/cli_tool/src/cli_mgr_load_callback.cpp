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

#include "cli_mgr_load_callback.h"

#include "cli_tool_mgr_client.h"
#include "hilog_tag_wrapper.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace CliTool {
void CliMgrLoadCallback::OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    if (systemAbilityId != CLI_TOOL_MGR_SERVICE_ID) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "System ability id %{public}d mismatch", systemAbilityId);
        return;
    }

    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Object is nullptr");
        return;
    }

    TAG_LOGD(AAFwkTag::CLI_TOOL, "Load system ability %{public}d succeed", systemAbilityId);
    CliToolMGRClient::GetInstance().OnLoadSystemAbilitySuccess(remoteObject);
}

void CliMgrLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    if (systemAbilityId != CLI_TOOL_MGR_SERVICE_ID) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "System ability id %{public}d mismatch", systemAbilityId);
        return;
    }

    TAG_LOGD(AAFwkTag::CLI_TOOL, "Load system ability %{public}d failed", systemAbilityId);
    CliToolMGRClient::GetInstance().OnLoadSystemAbilityFail();
}
}  // namespace CliTool
}  // namespace OHOS
