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

#include "function_call_execute_param.h"

#include "message_parcel.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
bool FunctionCallExecuteParam::ReadFromParcel(Parcel &parcel)
{
    bundleName = Str16ToStr8(parcel.ReadString16());
    intentName = Str16ToStr8(parcel.ReadString16());
    std::unique_ptr<WantParams> wp(parcel.ReadParcelable<WantParams>());
    if (wp == nullptr) {
        return false;
    }
    wantParam = *wp;
    callerAbilityToken = (static_cast<MessageParcel*>(&parcel))->ReadRemoteObject();
    intentId = parcel.ReadInt32();
    return true;
}

FunctionCallExecuteParam *FunctionCallExecuteParam::Unmarshalling(Parcel &parcel)
{
    FunctionCallExecuteParam *param = new (std::nothrow) FunctionCallExecuteParam();
    if (param == nullptr) {
        return nullptr;
    }
    if (!param->ReadFromParcel(parcel)) {
        delete param;
        param = nullptr;
    }
    return param;
}

bool FunctionCallExecuteParam::Marshalling(Parcel &parcel) const
{
    parcel.WriteString16(Str8ToStr16(bundleName));
    parcel.WriteString16(Str8ToStr16(intentName));
    parcel.WriteParcelable(&wantParam);
    (static_cast<MessageParcel*>(&parcel))->WriteRemoteObject(callerAbilityToken);
    parcel.WriteInt32(intentId);
    return true;
}
} // namespace AppExecFwk
} // namespace OHOS