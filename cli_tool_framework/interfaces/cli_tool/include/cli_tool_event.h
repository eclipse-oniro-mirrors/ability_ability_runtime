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

#ifndef OHOS_ABILITY_RUNTIME_CLI_TOOL_EVENT_H
#define OHOS_ABILITY_RUNTIME_CLI_TOOL_EVENT_H

#include <string>

#include "parcel.h"

namespace OHOS {
namespace CliTool {

/**
 * @brief Tool event (for async mode)
 */
class CliToolEvent : public Parcelable {
public:
    std::string type;              // "stdout", "stderr", "exit", "error"
    std::string eventData;
    int32_t exitCode = 0;
    int64_t timestamp = 0;

    CliToolEvent() = default;
    ~CliToolEvent() = default;

    bool Marshalling(Parcel &parcel) const override;
    static CliToolEvent *Unmarshalling(Parcel &parcel);
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_TOOL_EVENT_H