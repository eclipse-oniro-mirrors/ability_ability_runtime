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

#include "insight_intent_execute_lite_param.h"

#include "message_parcel.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
bool InsightIntentExecuteLiteParam::ReadFromParcel(Parcel &parcel)
{
    key = parcel.ReadUint64();
    insightIntentName = Str16ToStr8(parcel.ReadString16());
    std::unique_ptr<WantParams> wp(parcel.ReadParcelable<WantParams>());
    if (wp == nullptr) {
        return false;
    }
    insightIntentParam = *wp;
    insightIntentHostClient = (static_cast<MessageParcel*>(&parcel))->ReadRemoteObject();
    return true;
}

InsightIntentExecuteLiteParam *InsightIntentExecuteLiteParam::Unmarshalling(Parcel &parcel)
{
    InsightIntentExecuteLiteParam *param = new (std::nothrow) InsightIntentExecuteLiteParam();
    if (param == nullptr) {
        return nullptr;
    }
    if (!param->ReadFromParcel(parcel)) {
        delete param;
        param = nullptr;
    }
    return param;
}

bool InsightIntentExecuteLiteParam::Marshalling(Parcel &parcel) const
{
    parcel.WriteUint64(key);
    parcel.WriteString16(Str8ToStr16(insightIntentName));
    parcel.WriteParcelable(&insightIntentParam);
    (static_cast<MessageParcel*>(&parcel))->WriteRemoteObject(insightIntentHostClient);
    return true;
}
} // namespace AppExecFwk
} // namespace OHOS