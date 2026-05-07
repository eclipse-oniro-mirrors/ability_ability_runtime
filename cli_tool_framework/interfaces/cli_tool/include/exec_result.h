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

#ifndef OHOS_ABILITY_RUNTIME_EXEC_RESULT_H
#define OHOS_ABILITY_RUNTIME_EXEC_RESULT_H

#include <string>

#include "parcel.h"

namespace OHOS {
namespace CliTool {
/**
 * @brief Tool execution result
 */
class ExecResult : public Parcelable {
public:
    int32_t exitCode = 1;
    std::string outputText = "";
    std::string errorText = "";
    int32_t signalNumber = 0;
    bool timedOut = false;
    int64_t executionTime = 0;

    bool Marshalling(Parcel &parcel) const;
    static ExecResult *Unmarshalling(Parcel &parcel);
};
} // namespace CliTool
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_EXEC_RESULT_H