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

#ifndef OHOS_ABILITY_RUNTIME_PIXEL_MAP_BRIDGE_H
#define OHOS_ABILITY_RUNTIME_PIXEL_MAP_BRIDGE_H

#include <mutex>

namespace OHOS {
class Parcel;
namespace Media {
class PixelMap;
}
namespace AAFwk {

// Function pointer types matching the C exports of libimage_native_wrap.z.so.
// Parcel and PixelMap are only forward-declared here, so the bridge SO never
// needs the full image_native/parcel headers and stays free of that dependency.
using ReadPixelMapFunc = Media::PixelMap *(*)(Parcel *parcelPtr);
using WritePixelMapFunc = bool (*)(Media::PixelMap *pixelMapPtr, Parcel *parcelPtr);
using DestroyPixelMapFunc = void (*)(Media::PixelMap *pixelMapPtr);

// Bridge singleton that dlopen()'s libimage_native_wrap.z.so and caches the
// resolved function pointers.
//
// All methods are thread safe. The wrapped library is loaded lazily on first
// use; if it is unavailable every accessor degrades to a no-op / failure.
class PixelMapBridge {
public:
    static PixelMapBridge &GetInstance();
    ~PixelMapBridge();

    // Reads a PixelMap from the given Parcel.
    Media::PixelMap *ReadPixelMapFromParcel(Parcel *parcelPtr);
    // Writes a PixelMap into the given Parcel. Returns false on failure.
    bool WritePixelMapToParcel(Media::PixelMap *pixelMapPtr, Parcel *parcelPtr);
    // Destroys a PixelMap.
    void DestroyPixelMap(Media::PixelMap *pixelMapPtr);

private:
    PixelMapBridge();
    bool LoadLibrary();
    void UnloadLibrary();

    std::mutex mutex_;
    void *handle_ = nullptr;
    ReadPixelMapFunc readFunc_ = nullptr;
    WritePixelMapFunc writeFunc_ = nullptr;
    DestroyPixelMapFunc destroyFunc_ = nullptr;
    bool loadAttempted_ = false;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_PIXEL_MAP_BRIDGE_H
