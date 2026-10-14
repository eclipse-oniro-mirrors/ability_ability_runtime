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

#include "task_mgr/attribution_record.h"

namespace OHOS {
namespace AppExecFwk {
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

AttributionTool AttributionToolFromString(const std::string &text)
{
    if (text == "insight") {
        return AttributionTool::INSIGHT;
    }
    if (text == "cli") {
        return AttributionTool::CLI;
    }
    if (text == "a2a") {
        return AttributionTool::A2A;
    }
    if (text == "ability") {
        return AttributionTool::ABILITY;
    }
    return AttributionTool::UNKNOWN;
}
}  // namespace AppExecFwk
}  // namespace OHOS
