/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "start_window_option.h"
#include "hilog_tag_wrapper.h"

#ifdef START_WINDOW_OPTIONS_WITH_PIXELMAP
#include "pixel_map_bridge.h"
#endif

namespace OHOS {
namespace AAFwk {
bool StartWindowOption::ReadFromParcel(Parcel &parcel)
{
    hasStartWindow = parcel.ReadBool();
    startWindowBackgroundColor = parcel.ReadString();
#ifdef START_WINDOW_OPTIONS_WITH_PIXELMAP
    auto &bridge = PixelMapBridge::GetInstance();
    Media::PixelMap *rawPtr = bridge.ReadPixelMapFromParcel(&parcel);
    if (rawPtr != nullptr) {
        // Route the delete back through the bridge so it runs inside the wrap
        // SO (where the PixelMap was allocated), keeping allocation/release
        // matched and CFI friendly.
        startWindowIcon = std::shared_ptr<Media::PixelMap>(rawPtr,
            [&bridge](Media::PixelMap *p) { bridge.DestroyPixelMap(p); });
    }
#endif
    return true;
}

StartWindowOption *StartWindowOption::Unmarshalling(Parcel &parcel)
{
    StartWindowOption *option = new (std::nothrow) StartWindowOption();
    if (option == nullptr) {
        return nullptr;
    }

    if (!option->ReadFromParcel(parcel)) {
        delete option;
        option = nullptr;
    }

    return option;
}

bool StartWindowOption::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(hasStartWindow)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "hasStartWindow write failed");
        return false;
    }
    if (!parcel.WriteString(startWindowBackgroundColor)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "startWindowBackgroundColor write failed");
        return false;
    }
#ifdef START_WINDOW_OPTIONS_WITH_PIXELMAP
    if (!PixelMapBridge::GetInstance().WritePixelMapToParcel(startWindowIcon.get(), &parcel)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "startWindowIcon write failed");
        return false;
    }
#endif
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
