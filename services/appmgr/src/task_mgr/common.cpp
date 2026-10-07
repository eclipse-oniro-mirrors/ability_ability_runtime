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

#include "task_mgr/common.h"

namespace OHOS {
namespace AppExecFwk {
const char *TaskFinishReasonToString(TaskFinishReason reason)
{
    switch (reason) {
        case TaskFinishReason::FINISHED_BY_CALLER:
            return "caller";
        case TaskFinishReason::FINISHED_BY_TIMEOUT:
            return "timeout";
        case TaskFinishReason::FINISHED_BY_CREATOR_DIED:
            return "creatorDied";
        default:
            return "unknown";
    }
}

const char *TaskPriorityToString(int32_t priority)
{
    switch (priority) {
        case TASK_PRIORITY_LOW:
            return "LOW(100)";
        case TASK_PRIORITY_MEDIUM:
            return "MEDIUM(200)";
        case TASK_PRIORITY_HIGH:
            return "HIGH(300)";
        default:
            return "INVALID";
    }
}

const char *AttributionToolToString(AttributionTool tool)
{
    switch (tool) {
        case AttributionTool::INSIGHT:
            return "insight";
        case AttributionTool::CLI:
            return "cli";
        case AttributionTool::A2A:
            return "a2a";
        case AttributionTool::ABILITY:
            return "ability";
        default:
            return "unknown";
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
