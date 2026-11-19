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

#include "mission_snapshot.h"
#ifdef SUPPORT_SCREEN
#include "pixel_map_bridge.h"
#endif //SUPPORT_SCREEN

namespace OHOS {
namespace AAFwk {
bool MissionSnapshot::ReadFromParcel(Parcel &parcel)
{
#ifdef SUPPORT_SCREEN
    std::unique_ptr<AppExecFwk::ElementName> ability(parcel.ReadParcelable<AppExecFwk::ElementName>());
    if (ability == nullptr) {
        return false;
    }
    topAbility = *ability;
    auto &bridge = PixelMapBridge::GetInstance();
    Media::PixelMap *rawPtr = bridge.ReadPixelMapFromParcel(&parcel);
    if (rawPtr == nullptr) {
        return false;
    }
    // Route the delete back through the bridge so it runs inside the wrap
    // SO (where the PixelMap was allocated), keeping allocation/release
    // matched and CFI friendly.
    snapshot = std::shared_ptr<Media::PixelMap>(rawPtr,
        [&bridge](Media::PixelMap *p) { bridge.DestroyPixelMap(p); });
#endif
    return true;
}

MissionSnapshot *MissionSnapshot::Unmarshalling(Parcel &parcel)
{
    MissionSnapshot *info = new (std::nothrow) MissionSnapshot();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool MissionSnapshot::Marshalling(Parcel &parcel) const
{
#ifdef SUPPORT_SCREEN
    if (!parcel.WriteParcelable(&topAbility)) {
        return false;
    }
    if (!PixelMapBridge::GetInstance().WritePixelMapToParcel(snapshot.get(), &parcel)) {
        return false;
    }
#endif
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
