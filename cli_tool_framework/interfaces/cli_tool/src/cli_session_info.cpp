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

#include "cli_session_info.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {
bool CliSessionInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(sessionId)) {
        return false;
    }
    if (!parcel.WriteString(toolName)) {
        return false;
    }
    if (!parcel.WriteString(status)) {
        return false;
    }

    bool hasResult = (result != nullptr);
    if (!parcel.WriteBool(hasResult)) {
        return false;
    }
    if (hasResult && !parcel.WriteParcelable(result.get())) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write result failed.");
        return false;
    }
    return true;
}

CliSessionInfo *CliSessionInfo::Unmarshalling(Parcel &parcel)
{
    auto *info = new (std::nothrow) CliSessionInfo();
    if (info && !parcel.ReadString(info->sessionId)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadString(info->toolName)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadString(info->status)) {
        delete info;
        return nullptr;
    }

    bool hasResult = false;
    if (!parcel.ReadBool(hasResult)) {
        delete info;
        return nullptr;
    }
    if (hasResult) {
        std::shared_ptr<ExecResult> execResult(parcel.ReadParcelable<ExecResult>());
        if (execResult == nullptr) {
            delete info;
            return nullptr;
        }
        info->result = execResult;
    }
    return info;
}
} // namespace CliTool
} // namespace OHOS