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

#include "exec_result.h"

namespace OHOS {
namespace CliTool {
bool ExecResult::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(exitCode)) {
        return false;
    }
    if (!parcel.WriteString(outputText)) {
        return false;
    }
    if (!parcel.WriteString(errorText)) {
        return false;
    }
    if (!parcel.WriteInt32(signalNumber)) {
        return false;
    }
    if (!parcel.WriteBool(timedOut)) {
        return false;
    }
    if (!parcel.WriteInt64(executionTime)) {
        return false;
    }
    return true;
}

ExecResult *ExecResult::Unmarshalling(Parcel &parcel)
{
    auto *result = new (std::nothrow) ExecResult();
    if (result && !parcel.ReadInt32(result->exitCode)) {
        delete result;
        return nullptr;
    }
    if (!parcel.ReadString(result->outputText)) {
        delete result;
        return nullptr;
    }
    if (!parcel.ReadString(result->errorText)) {
        delete result;
        return nullptr;
    }
    if (!parcel.ReadInt32(result->signalNumber)) {
        delete result;
        return nullptr;
    }
    if (!parcel.ReadBool(result->timedOut)) {
        delete result;
        return nullptr;
    }
    if (!parcel.ReadInt64(result->executionTime)) {
        delete result;
        return nullptr;
    }
    return result;
}
} // namespace CliTool
} // namespace OHOS