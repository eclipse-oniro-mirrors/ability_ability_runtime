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

#ifndef OHOS_ABILITY_RUNTIME_EXEC_TOOL_PARAM_H
#define OHOS_ABILITY_RUNTIME_EXEC_TOOL_PARAM_H

#include <string>

#include "exec_options.h"
#include "parcel.h"
#include "want_params.h"

namespace OHOS {
namespace CliTool {
/**
 * @brief Tool execution result
 */
class ExecToolParam : public Parcelable {
public:
    std::string toolName;
    std::string subcommand;
    std::string challenge;
    ExecOptions options;
    AAFwk::WantParams args;

    bool Marshalling(Parcel &parcel) const;
    static ExecToolParam *Unmarshalling(Parcel &parcel);
};
} // namespace CliTool
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_EXEC_TOOL_PARAM_H
