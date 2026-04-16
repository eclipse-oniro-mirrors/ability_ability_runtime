/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_WANT_SENDER_INFO_H
#define OHOS_ABILITY_RUNTIME_WANT_SENDER_INFO_H

#include <string>
#include <vector>

#include "parcel.h"

#include "wants_info.h"

namespace OHOS {
namespace AAFwk {
struct WantSenderInfo : public Parcelable {
    int32_t type;
    int32_t requestCode;
    uint32_t flags;
    int32_t userId = -1;
    int32_t appIndex;
    std::string bundleName;
    std::string resultWho;
    std::vector<WantsInfo> allWants;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static WantSenderInfo *Unmarshalling(Parcel &parcel);
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_WANT_SENDER_INFO_H
