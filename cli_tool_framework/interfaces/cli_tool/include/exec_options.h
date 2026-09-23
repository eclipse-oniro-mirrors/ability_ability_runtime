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

#ifndef OHOS_ABILITY_RUNTIME_EXEC_OPTIONS_H
#define OHOS_ABILITY_RUNTIME_EXEC_OPTIONS_H

#include <map>
#include <string>

#include "parcel.h"

namespace OHOS {
namespace CliTool {
/**
 * @struct ExecOptions
 * @brief Options for executing CLI tools.
 */
class ExecOptions : public Parcelable {
public:
    bool background = false;
    int64_t yieldMs = 0;
    int64_t timeout = 0;
    std::string toolCallId;
    std::string dmSessionId;

    bool Marshalling(Parcel &parcel) const;
    static ExecOptions *Unmarshalling(Parcel &parcel);
};
// Shared trace-id rule: max length of toolCallId/dmSessionId ([A-Za-z0-9_-]{1,256}).
constexpr size_t TRACE_ID_MAX_LEN = 256;

/**
 * @brief Validates a trace identifier received over IPC.
 *
 * Empty means "not provided" and is valid. A non-empty value must consist of
 * [A-Za-z0-9_-] only and be at most TRACE_ID_MAX_LEN (256) characters.
 * Server-side entry points (ExecTool/ExecCmd) reject requests whose
 * identifiers fail this check, so untrusted Parcel input can never reach
 * logs or child-process environments.
 */
bool IsValidTraceId(const std::string &value);

} // namespace CliTool
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_EXEC_OPTIONS_H
