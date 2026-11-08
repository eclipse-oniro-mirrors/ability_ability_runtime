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

#include "exec_options.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {
bool ExecOptions::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(background)) {
        return false;
    }
    if (!parcel.WriteInt64(yieldMs)) {
        return false;
    }
    if (!parcel.WriteInt64(timeout)) {
        return false;
    }
    if (!parcel.WriteString(toolCallId)) {
        return false;
    }
    if (!parcel.WriteString(dmSessionId)) {
        return false;
    }
    if (!parcel.WriteString(policy)) {
        return false;
    }
    if (!parcel.WriteString(taskId)) {
        return false;
    }
    return true;
}

ExecOptions *ExecOptions::Unmarshalling(Parcel &parcel)
{
    auto *options = new (std::nothrow) ExecOptions();
    if (options == nullptr) {
        return nullptr;
    }
    if (!parcel.ReadBool(options->background)) {
        delete options;
        return nullptr;
    }
    if (!parcel.ReadInt64(options->yieldMs)) {
        delete options;
        return nullptr;
    }
    if (!parcel.ReadInt64(options->timeout)) {
        delete options;
        return nullptr;
    }
    if (!parcel.ReadString(options->toolCallId)) {
        delete options;
        return nullptr;
    }
    if (!parcel.ReadString(options->dmSessionId)) {
        delete options;
        return nullptr;
    }
    if (!parcel.ReadString(options->policy)) {
        delete options;
        return nullptr;
    }
    // Options tail, not message tail: valid for same-image upgrades only (mixed deploy mis-aligns).
    options->taskId = "";
    if (!parcel.ReadString(options->taskId)) {
        TAG_LOGD(AAFwkTag::CLI_TOOL, "taskId not present, using default(\"\").");
    }
    return options;
}

bool IsValidTraceId(const std::string &value)
{
    if (value.empty()) {
        return true;
    }
    if (value.length() > TRACE_ID_MAX_LEN) {
        return false;
    }
    for (char c : value) {
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '_' || c == '-')) {
            return false;
        }
    }
    return true;
}
} // namespace CliTool
} // namespace OHOS