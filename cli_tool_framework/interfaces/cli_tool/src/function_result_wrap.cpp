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

#include "function_result_wrap.h"

#include <memory>

#include "hilog_tag_wrapper.h"

namespace OHOS::CliTool {
bool FunctionResultWrap::Marshalling(Parcel &parcel) const
{
    if (!result.Marshalling(parcel)) {
        return false;
    }
    if (!parcel.WriteString(toolCallId) || !parcel.WriteString(dmSessionId)) {
        return false;
    }
    return true;
}

FunctionResultWrap *FunctionResultWrap::Unmarshalling(Parcel &parcel)
{
    auto wrap = std::make_unique<FunctionResultWrap>();
    auto res = std::unique_ptr<InvokeFunctionResult>(InvokeFunctionResult::Unmarshalling(parcel));
    if (res == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to unmarshal InvokeFunctionResult.");
        return nullptr;
    }
    wrap->result = std::move(*res);
    // Trace identifiers: tolerant tail reads, default "" (not provided).
    if (!parcel.ReadString(wrap->toolCallId) || !parcel.ReadString(wrap->dmSessionId)) {
        TAG_LOGD(AAFwkTag::CLI_TOOL, "FunctionResultWrap trace ids not present, using default(\"\").");
    }
    return wrap.release();
}
}
