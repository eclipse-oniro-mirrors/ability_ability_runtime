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

#include "insight_intent_query_param.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
using WantParams = OHOS::AAFwk::WantParams;

bool InsightIntentQueryParam::ReadFromParcel(Parcel &parcel)
{
    bundleName_ = Str16ToStr8(parcel.ReadString16());
    moduleName_ = Str16ToStr8(parcel.ReadString16());
    intentName_ = Str16ToStr8(parcel.ReadString16());
    className_ = Str16ToStr8(parcel.ReadString16());
    queryEntityParam_.queryType_ = Str16ToStr8(parcel.ReadString16());
    std::shared_ptr<WantParams> wantParams(parcel.ReadParcelable<WantParams>());
    if (wantParams != nullptr) {
        queryEntityParam_.parameters_ = wantParams;
    }
    userId_ = parcel.ReadInt32();
    intentId_ = parcel.ReadUint64();
    return true;
}

InsightIntentQueryParam *InsightIntentQueryParam::Unmarshalling(Parcel &parcel)
{
    InsightIntentQueryParam *param = new (std::nothrow) InsightIntentQueryParam();
    if (param == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "new InsightIntentQueryParam failed");
        return nullptr;
    }

    if (!param->ReadFromParcel(parcel)) {
        TAG_LOGE(AAFwkTag::INTENT, "ReadFromParcel failed");
        delete param;
        param = nullptr;
    }
    return param;
}

bool InsightIntentQueryParam::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString16(Str8ToStr16(bundleName_))) {
        TAG_LOGE(AAFwkTag::INTENT, "write bundleName error");
        return false;
    }
    if (!parcel.WriteString16(Str8ToStr16(moduleName_))) {
        TAG_LOGE(AAFwkTag::INTENT, "write moduleName error");
        return false;
    }
    if (!parcel.WriteString16(Str8ToStr16(intentName_))) {
        TAG_LOGE(AAFwkTag::INTENT, "write intentName error");
        return false;
    }
    if (!parcel.WriteString16(Str8ToStr16(className_))) {
        TAG_LOGE(AAFwkTag::INTENT, "write className error");
        return false;
    }
    if (!parcel.WriteString16(Str8ToStr16(queryEntityParam_.queryType_))) {
        TAG_LOGE(AAFwkTag::INTENT, "write queryType error");
        return false;
    }
    if (!parcel.WriteParcelable(queryEntityParam_.parameters_.get())) {
        TAG_LOGE(AAFwkTag::INTENT, "write parameters error");
        return false;
    }
    if (!parcel.WriteInt32(userId_)) {
        TAG_LOGE(AAFwkTag::INTENT, "write userId error");
        return false;
    }
    if (!parcel.WriteUint64(intentId_)) {
        TAG_LOGE(AAFwkTag::INTENT, "write intentId error");
        return false;
    }
    return true;
}

} // namespace AppExecFwk
} // namespace OHOS
