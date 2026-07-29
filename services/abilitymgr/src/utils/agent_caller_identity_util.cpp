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

#include "utils/agent_caller_identity_util.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "utils/agent_ability_util.h"

namespace OHOS {
namespace AAFwk {
AgentCallerIdentityScope::~AgentCallerIdentityScope()
{
    if (active_) {
        IPCSkeleton::SetCallingIdentity(serviceIdentity_);
    }
}

int32_t AgentCallerIdentityScope::ApplyIfNeeded(AppExecFwk::ExtensionAbilityType extensionType,
    const std::string &callerIdentity)
{
    if (!AgentAbilityUtil::IsAgentExtensionType(extensionType)) {
        return ERR_OK;
    }

    if (callerIdentity.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "AGENT request missing original caller identity");
        return ERR_INVALID_VALUE;
    }

    serviceIdentity_ = IPCSkeleton::ResetCallingIdentity();
    std::string identity = callerIdentity;
    IPCSkeleton::SetCallingIdentity(identity);
    active_ = true;
    return ERR_OK;
}
} // namespace AAFwk
} // namespace OHOS
