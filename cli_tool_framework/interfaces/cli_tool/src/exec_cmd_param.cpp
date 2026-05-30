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

#include "exec_cmd_param.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {
bool ExecCmdParam::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(cmd)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write cmd failed.");
        return false;
    }
    if (!parcel.WriteString(workDir)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write workDir failed.");
        return false;
    }
    if (!parcel.WriteString(env)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write env failed.");
        return false;
    }
    if (!parcel.WriteString(policy)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write policy failed.");
        return false;
    }
    if (!parcel.WriteParcelable(&options)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write options failed.");
        return false;
    }
    return true;
}

ExecCmdParam *ExecCmdParam::Unmarshalling(Parcel &parcel)
{
    auto *result = new (std::nothrow) ExecCmdParam();
    if (result == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Fail to create ExecCmdParam.");
        return nullptr;
    }
    if (!parcel.ReadString(result->cmd)) {
        delete result;
        return nullptr;
    }
    if (!parcel.ReadString(result->workDir)) {
        delete result;
        return nullptr;
    }
    if (!parcel.ReadString(result->env)) {
        delete result;
        return nullptr;
    }
    if (!parcel.ReadString(result->policy)) {
        delete result;
        return nullptr;
    }

    std::unique_ptr<ExecOptions> execOptions(parcel.ReadParcelable<ExecOptions>());
    if (execOptions == nullptr) {
        delete result;
        return nullptr;
    }
    result->options = *execOptions;
    return result;
}
} // namespace CliTool
} // namespace OHOS
