/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_ABILITY_RUNTIME_KIOSK_STATUS_H
#define OHOS_ABILITY_RUNTIME_KIOSK_STATUS_H

#include <string>

#include "iremote_object.h"
#include "parcel.h"

namespace OHOS {
namespace AAFwk {
/**
 * @struct KioskStatus
 * KioskStatus is used to save information about Kiosk mode.
 */
struct KioskStatus : public Parcelable {
    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static KioskStatus *Unmarshalling(Parcel &parcel);
    void Clear();

    bool isKioskMode_ = false;
    std::string kioskBundleName_;
    int32_t kioskBundleUid_{};
    sptr<IRemoteObject> kioskToken_;
    int32_t kioskType_ = 0; // Rosen::KioskType::DEFAULT
    int32_t kioskCallerUid_{};
    bool isProxyEnter_ = false;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_KIOSK_STATUS_H
