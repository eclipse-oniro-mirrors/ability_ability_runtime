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

#ifndef OHOS_ABILITY_RUNTIME_CLI_SESSION_INFO_H
#define OHOS_ABILITY_RUNTIME_CLI_SESSION_INFO_H

#include <string>

#include "exec_result.h"
#include "parcel.h"

namespace OHOS {
namespace CliTool {
/**
 * @struct CliSessionInfo
 * @brief Information about a CLI tool execution session.
 */
class CliSessionInfo : public Parcelable {
public:
    std::string sessionId;
    std::string toolName;
    std::string status;            // "running", "completed", "failed"
    std::shared_ptr<ExecResult> result = nullptr;  // optional, only when status="completed" and status="failed"

    CliSessionInfo() = default;

    bool Marshalling(Parcel &parcel) const;
    static CliSessionInfo *Unmarshalling(Parcel &parcel);
};
} // namespace CliTool
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_CLI_SESSION_INFO_H