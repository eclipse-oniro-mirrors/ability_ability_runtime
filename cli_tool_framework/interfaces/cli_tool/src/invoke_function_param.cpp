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

#include "invoke_function_param.h"

#include <memory>

#include "hilog_tag_wrapper.h"

namespace OHOS::CliTool {
bool InvokeFunctionParam::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(functionNamespace)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write functionNamespace.");
        return false;
    }
    if (!parcel.WriteString(functionName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write functionName.");
        return false;
    }
    if (!parcel.WriteParcelable(&args)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write args.");
        return false;
    }
    if (!parcel.WriteString(invokeOptions.context)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write invokeOptions.context.");
        return false;
    }
    return true;
}

InvokeFunctionParam *InvokeFunctionParam::Unmarshalling(Parcel &parcel)
{
    auto param = std::make_unique<InvokeFunctionParam>();
    if (!parcel.ReadString(param->functionNamespace)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read functionNamespace.");
        return nullptr;
    }
    if (!parcel.ReadString(param->functionName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read functionName.");
        return nullptr;
    }
    std::unique_ptr<AAFwk::WantParams> args(parcel.ReadParcelable<AAFwk::WantParams>());
    if (args == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read args.");
        return nullptr;
    }
    param->args = *args;
    if (!parcel.ReadString(param->invokeOptions.context)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read invokeOptions.context.");
        return nullptr;
    }
    return param.release();
}
}
