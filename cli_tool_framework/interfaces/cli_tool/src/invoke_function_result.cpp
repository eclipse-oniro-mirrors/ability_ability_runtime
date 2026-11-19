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

#include "invoke_function_result.h"

#include <memory>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {
bool InvokeFunctionResult::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(success)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write success.");
        return false;
    }
    if (!parcel.WriteInt32(errorCode)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write errorCode.");
        return false;
    }
    if (!parcel.WriteString(errorMsg)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write errorMsg.");
        return false;
    }
    if (data != nullptr) {
        if (!parcel.WriteBool(true)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write data flag.");
            return false;
        }
        if (!parcel.WriteParcelable(data.get())) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write data.");
            return false;
        }
    } else {
        if (!parcel.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write data flag.");
            return false;
        }
    }
    return true;
}

InvokeFunctionResult *InvokeFunctionResult::Unmarshalling(Parcel &parcel)
{
    auto result = std::make_unique<InvokeFunctionResult>();
    if (!parcel.ReadBool(result->success)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read success.");
        return nullptr;
    }
    if (!parcel.ReadInt32(result->errorCode)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read errorCode.");
        return nullptr;
    }
    if (!parcel.ReadString(result->errorMsg)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read errorMsg.");
        return nullptr;
    }
    bool hasData = false;
    if (!parcel.ReadBool(hasData)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read data flag.");
        return nullptr;
    }
    if (hasData) {
        auto wantParams = std::unique_ptr<AAFwk::WantParams>(parcel.ReadParcelable<AAFwk::WantParams>());
        if (wantParams == nullptr) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read data.");
            return nullptr;
        }
        result->data = std::shared_ptr<AAFwk::WantParams>(wantParams.release());
    }
    return result.release();
}
} // namespace CliTool
} // namespace OHOS
