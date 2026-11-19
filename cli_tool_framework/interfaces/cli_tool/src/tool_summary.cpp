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

#include "tool_summary.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {
bool ToolSummary::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(name)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write name failed");
        return false;
    }
    if (!parcel.WriteString(version)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write version failed");
        return false;
    }
    if (!parcel.WriteString(description)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write description failed");
        return false;
    }
    return true;
}

ToolSummary *ToolSummary::Unmarshalling(Parcel &parcel)
{
    auto *summary = new (std::nothrow) ToolSummary();
    if (summary == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to allocate ToolSummary");
        return nullptr;
    }

    if (!parcel.ReadString(summary->name)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read name failed");
        delete summary;
        return nullptr;
    }
    if (!parcel.ReadString(summary->version)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read version failed");
        delete summary;
        return nullptr;
    }
    if (!parcel.ReadString(summary->description)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read description failed");
        delete summary;
        return nullptr;
    }

    return summary;
}
} // namespace CliTool
} // namespace OHOS
