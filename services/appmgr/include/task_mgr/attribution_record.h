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

#ifndef OHOS_APP_EXECFWK_TASK_MGR_ATTRIBUTION_RECORD_H
#define OHOS_APP_EXECFWK_TASK_MGR_ATTRIBUTION_RECORD_H

#include <cstdint>
#include <string>
#include <sys/types.h>

namespace OHOS {
namespace AppExecFwk {

enum class AttributionTool : int32_t {
    UNKNOWN = 0,
    INSIGHT = 1,
    CLI = 2,
    A2A = 3,
    ABILITY = 4,
};

struct AttributionRecord {
    pid_t pid = -1;
    AttributionTool tool = AttributionTool::UNKNOWN;
    int64_t attributedTimeMillis = 0;  // server-authoritative, set at registration
};

const char *AttributionToolToString(AttributionTool tool);
// Inverse of AttributionToolToString (dump command texts); returns UNKNOWN on no match.
AttributionTool AttributionToolFromString(const std::string &text);
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_APP_EXECFWK_TASK_MGR_ATTRIBUTION_RECORD_H
