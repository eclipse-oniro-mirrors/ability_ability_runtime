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

#ifndef OHOS_CLI_TOOL_INVOKE_FUNCTION_PARAM_H
#define OHOS_CLI_TOOL_INVOKE_FUNCTION_PARAM_H
#include "parcel.h"
#include "want_params.h"
#include "string"
namespace OHOS::CliTool {
struct InvokeOptions {
    std::string context;
    // Trace identifiers for log correlation; empty means "not provided".
    std::string toolCallId;
    std::string dmSessionId;
};

struct InvokeFunctionParam : public Parcelable {
    std::string functionNamespace;
    std::string functionName;
    AAFwk::WantParams args;
    InvokeOptions invokeOptions;
    bool Marshalling(Parcel &parcel) const override;
    static InvokeFunctionParam *Unmarshalling(Parcel &parcel);
};
}
#endif
