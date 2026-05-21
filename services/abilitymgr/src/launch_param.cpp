/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "launch_param.h"

#include "hilog_tag_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace AAFwk {
bool LaunchParam::ReadFromParcel(Parcel &parcel)
{
    int32_t reason = 0;
    if (!parcel.ReadInt32(reason)) {
        return false;
    }
    launchReason = static_cast<LaunchReason>(reason);
    launchReasonMessage = Str16ToStr8(parcel.ReadString16());

    if (!parcel.ReadInt32(reason)) {
        return false;
    }
    lastExitReason = static_cast<LastExitReason>(reason);

    lastExitMessage = Str16ToStr8(parcel.ReadString16());

    std::unique_ptr<LastExitDetailInfo> detailInfo(parcel.ReadParcelable<LastExitDetailInfo>());
    if (detailInfo == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "detailInfo null");
        return false;
    }
    lastExitDetailInfo = *detailInfo;
    launchUptime = parcel.ReadInt64();
    launchUTCTime = parcel.ReadInt64();
    specifiedId = Str16ToStr8(parcel.ReadString16());
    return true;
}

LaunchParam *LaunchParam::Unmarshalling(Parcel &parcel)
{
    LaunchParam *param = new (std::nothrow) LaunchParam();
    if (param == nullptr) {
        return nullptr;
    }

    if (!param->ReadFromParcel(parcel)) {
        delete param;
        param = nullptr;
    }
    return param;
}

bool LaunchParam::Marshalling(Parcel &parcel) const
{
    // write launchReason
    if (!parcel.WriteInt32(static_cast<int32_t>(launchReason))) {
        return false;
    }
    if (!parcel.WriteString16(Str8ToStr16(launchReasonMessage))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write launchReasonMessage failed");
        return false;
    }
    // write lastExitReason
    if (!parcel.WriteInt32(static_cast<int32_t>(lastExitReason))) {
        return false;
    }
    if (!parcel.WriteString16(Str8ToStr16(lastExitMessage))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write lastExitMessage failed");
        return false;
    }
    // write lastExitDetailInfo
    if (!parcel.WriteParcelable(&lastExitDetailInfo)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write lastExitDetailInfo failed");
        return false;
    }
    // write launchUptime
    if (!parcel.WriteInt64(launchUptime)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write launchUptime failed");
        return false;
    }
    // write launchUTCTime
    if (!parcel.WriteInt64(launchUTCTime)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write launchUTCTime failed");
        return false;
    }
    // write specifiedId
    if (!parcel.WriteString16(Str8ToStr16(specifiedId))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write specifiedId failed");
        return false;
    }
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
