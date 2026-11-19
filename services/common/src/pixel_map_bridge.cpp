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

#include "pixel_map_bridge.h"

#include <dlfcn.h>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char *IMAGE_NATIVE_WRAP_PATH = "libimage_native_wrap.z.so";
constexpr const char *SYMBOL_READ = "ImageNative_ReadPixelMapFromParcel";
constexpr const char *SYMBOL_WRITE = "ImageNative_WritePixelMapToParcel";
constexpr const char *SYMBOL_DESTROY = "ImageNative_DestroyPixelMap";
}  // namespace

PixelMapBridge &PixelMapBridge::GetInstance()
{
    static PixelMapBridge instance;
    return instance;
}

PixelMapBridge::PixelMapBridge() = default;

PixelMapBridge::~PixelMapBridge()
{
    UnloadLibrary();
}

void PixelMapBridge::UnloadLibrary()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (handle_ != nullptr) {
        readFunc_ = nullptr;
        writeFunc_ = nullptr;
        destroyFunc_ = nullptr;
        dlclose(handle_);
        handle_ = nullptr;
    }
}

bool PixelMapBridge::LoadLibrary()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (loadAttempted_) {
        return handle_ != nullptr;
    }
    loadAttempted_ = true;

    handle_ = dlopen(IMAGE_NATIVE_WRAP_PATH, RTLD_NOW | RTLD_GLOBAL);
    if (handle_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "dlopen %{public}s failed", IMAGE_NATIVE_WRAP_PATH);
        return false;
    }

    readFunc_ = reinterpret_cast<ReadPixelMapFunc>(dlsym(handle_, SYMBOL_READ));
    writeFunc_ = reinterpret_cast<WritePixelMapFunc>(dlsym(handle_, SYMBOL_WRITE));
    destroyFunc_ = reinterpret_cast<DestroyPixelMapFunc>(dlsym(handle_, SYMBOL_DESTROY));
    if (readFunc_ == nullptr || writeFunc_ == nullptr || destroyFunc_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "dlsym pixelmap wrap symbols failed");
        dlclose(handle_);
        handle_ = nullptr;
        return false;
    }
    return true;
}

Media::PixelMap *PixelMapBridge::ReadPixelMapFromParcel(Parcel *parcelPtr)
{
    if (!LoadLibrary()) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    return readFunc_(parcelPtr);
}

bool PixelMapBridge::WritePixelMapToParcel(Media::PixelMap *pixelMapPtr, Parcel *parcelPtr)
{
    if (!LoadLibrary()) {
        return false;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    return writeFunc_(pixelMapPtr, parcelPtr);
}

void PixelMapBridge::DestroyPixelMap(Media::PixelMap *pixelMapPtr)
{
    if (!LoadLibrary()) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    destroyFunc_(pixelMapPtr);
}
}  // namespace AAFwk
}  // namespace OHOS
