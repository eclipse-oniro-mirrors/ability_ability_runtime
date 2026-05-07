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

#include "exec_tool_param.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {
bool ExecToolParam::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(toolName)) {
        return false;
    }
    if (!parcel.WriteString(subcommand)) {
        return false;
    }
    if (!parcel.WriteString(challenge)) {
        return false;
    }
    if (!parcel.WriteParcelable(&options)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write options failed.");
        return false;
    }
    if (!parcel.WriteParcelable(&args)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write args failed.");
        return false;
    }
    return true;
}

ExecToolParam *ExecToolParam::Unmarshalling(Parcel &parcel)
{
    auto *result = new (std::nothrow) ExecToolParam();
    if (result == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Fail to create ExecToolParam.");
        return nullptr;
    }
    if (!parcel.ReadString(result->toolName)) {
        delete result;
        return nullptr;
    }
    if (!parcel.ReadString(result->subcommand)) {
        delete result;
        return nullptr;
    }
    if (!parcel.ReadString(result->challenge)) {
        delete result;
        return nullptr;
    }

    std::unique_ptr<ExecOptions> execOptions(parcel.ReadParcelable<ExecOptions>());
    if (execOptions == nullptr) {
        delete result;
        return nullptr;
    }
    result->options = *execOptions;

    std::unique_ptr<AAFwk::WantParams> args(parcel.ReadParcelable<AAFwk::WantParams>());
    if (args == nullptr) {
        delete result;
        return nullptr;
    }
    result->args = *args;
    return result;
}
} // namespace CliTool
} // namespace OHOS