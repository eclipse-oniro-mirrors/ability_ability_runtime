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

#ifndef OHOS_ABILITY_RUNTIME_TOOL_SUMMARY_H
#define OHOS_ABILITY_RUNTIME_TOOL_SUMMARY_H

#include <iremote_broker.h>
#include <parcel.h>
#include <string>

namespace OHOS {
namespace CliTool {
/**
 * @brief Tool summary information (lightweight for listing)
 */
class ToolSummary : public Parcelable {
public:
    std::string name;
    std::string version;
    std::string description;

    ToolSummary() = default;
    ~ToolSummary() = default;

    bool Marshalling(Parcel &parcel) const override;
    static ToolSummary *Unmarshalling(Parcel &parcel);
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_TOOL_SUMMARY_H
