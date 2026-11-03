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

#include "insight_intent_execution_info.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
bool InsightIntentExecutionInfo::ReadFromParcel(Parcel &parcel)
{
    intentName = parcel.ReadString();
    callerBundleName = parcel.ReadString();
    callerTokenId = parcel.ReadString();
    callerUid = parcel.ReadInt32();
    return true;
}

bool InsightIntentExecutionInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(intentName) ||
        !parcel.WriteString(callerBundleName) ||
        !parcel.WriteString(callerTokenId) ||
        !parcel.WriteInt32(callerUid)) {
        TAG_LOGE(AAFwkTag::INTENT, "write execution info failed");
        return false;
    }
    return true;
}

InsightIntentExecutionInfo *InsightIntentExecutionInfo::Unmarshalling(Parcel &parcel)
{
    auto info = new (std::nothrow) InsightIntentExecutionInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "new execution info failed");
        return nullptr;
    }
    if (!info->ReadFromParcel(parcel)) {
        TAG_LOGE(AAFwkTag::INTENT, "read execution info failed");
        delete info;
        return nullptr;
    }
    return info;
}
} // namespace AppExecFwk
} // namespace OHOS