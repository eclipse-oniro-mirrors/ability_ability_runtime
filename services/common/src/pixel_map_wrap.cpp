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

#include "parcel.h"
#include "pixel_map.h"

// This is the "fat" wrap SO that depends on image_native. It exposes plain C
// entry points for Media::PixelMap Parcel operations; libimage_native_bridge
// resolves them via dlsym so its own callers never link image_native.

// Deserializes a PixelMap from the parcel. The returned pointer is owned by the
// caller and must be released via ImageNative_DestroyPixelMap.
extern "C" __attribute__((visibility("default"))) OHOS::Media::PixelMap *ImageNative_ReadPixelMapFromParcel(
    OHOS::Parcel *parcelPtr)
{
    if (parcelPtr == nullptr) {
        return nullptr;
    }
    return parcelPtr->ReadParcelable<OHOS::Media::PixelMap>();
}

// Serializes the PixelMap into the parcel. Callers guarantee pixelMapPtr is
// non-null (they null-check the owning shared_ptr before invoking); a null
// parcelPtr is still guarded here.
extern "C" __attribute__((visibility("default"))) bool ImageNative_WritePixelMapToParcel(
    OHOS::Media::PixelMap *pixelMapPtr, OHOS::Parcel *parcelPtr)
{
    if (parcelPtr == nullptr) {
        return false;
    }
    return parcelPtr->WriteParcelable(pixelMapPtr);
}

// Destroys the PixelMap. The delete executes here, in the same SO that allocated
// it, keeping memory allocation/release matched and CFI friendly.
extern "C" __attribute__((visibility("default"))) void ImageNative_DestroyPixelMap(OHOS::Media::PixelMap *pixelMapPtr)
{
    if (pixelMapPtr == nullptr) {
        return;
    }
    delete pixelMapPtr;
}
