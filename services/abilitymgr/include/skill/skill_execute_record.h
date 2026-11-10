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

#ifndef OHOS_ABILITY_RUNTIME_SKILL_EXECUTE_RECORD_H
#define OHOS_ABILITY_RUNTIME_SKILL_EXECUTE_RECORD_H

#include <string>
#include <sys/types.h>

#include "iremote_object.h"
#include "skill/skill_execute_callback_interface.h"

namespace OHOS {
namespace AAFwk {

enum class SkillExecuteState {
    UNKNOWN = 0,
    EXECUTING,
    EXECUTE_DONE,
    REMOTE_DIED,
    TIMED_OUT,
};

struct SkillExecuteRecord {
    std::string requestCode;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient = nullptr;
    std::string targetBundleName;
    std::string callerBundleName;
    uint32_t callerTokenId = 0;
    uint64_t requestCodeSeq = 0;
    pid_t targetPid = 0;
    SkillExecuteState state = SkillExecuteState::UNKNOWN;
    sptr<ISkillExecuteCallback> callback = nullptr;
    // Attribution: taskId is the task key (empty = not attributed); callerUid is the
    // ownership anchor (IPC-forwarded by the reporter); attributionId is the unregister
    // handle (0 = not registered). Registered at the early anchors (cold UIAbility attach /
    // warm dispatch before ExecuteSkill), not at launch completion; every terminal path
    // funnels into RemoveRecord. This phase attributes UIAbility targets only; extensions
    // run unattributed by design (taskId/callerUid stay for audit).
    std::string taskId = "";
    int32_t callerUid = -1;
    int64_t attributionId = 0;
};

} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SKILL_EXECUTE_RECORD_H
