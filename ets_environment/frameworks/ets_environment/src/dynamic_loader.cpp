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

#include "dynamic_loader.h"

#include <cstdio>
#include <dlfcn.h>
#include <mutex>
#include <securec.h>
#include <string>
#include <unordered_set>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace EtsEnv {
namespace {
constexpr int32_t ERROR_BUF_SIZE = 255;
static char g_dlError[ERROR_BUF_SIZE];
static std::unordered_set<std::string> g_hasInited;
static std::string g_sharedLibsSonames = "";
static std::mutex g_dynLoaderMutex;
constexpr int32_t OUT_OF_MEMORY = 12;
constexpr int32_t FILE_EXISTS = 17;
constexpr int32_t INVALID_ARGUMENT = 22;

static void ReadDlError()
{
    char *errMsg = dlerror();
    if (!errMsg) {
        return;
    }
    auto ends = sprintf_s(g_dlError, sizeof(g_dlError), "%s", errMsg);
    if (ends < 0) {
        return;
    } else if (ends >= ERROR_BUF_SIZE) {
        g_dlError[ERROR_BUF_SIZE - 1] = '\0';
    } else {
        g_dlError[ends] = '\0';
    }
}

static void InitSharedLibsSonames()
{
    if (!g_sharedLibsSonames.empty()) {
        return;
    }
    g_sharedLibsSonames =
        // bionic library
        "libc.so:"
        "libdl.so:"
        "libm.so:"
        "libz.so:"
        "libclang_rt.asan.so:"
        "libclang_rt.tsan.so:"
        // z library
        "libace_napi.z.so:"
        "libace_ndk.z.so:"
        "libbundle_ndk.z.so:"
        "libdeviceinfo_ndk.z.so:"
        "libEGL.so:"
        "libGLESv3.so:"
        "libhiappevent_ndk.z.so:"
        "libhuks_ndk.z.so:"
        "libhukssdk.z.so:"
        "libnative_drawing.so:"
        "libnative_window.so:"
        "libnative_buffer.so:"
        "libnative_vsync.so:"
        "libOpenSLES.so:"
        "libpixelmap_ndk.z.so:"
        "libimage_ndk.z.so:"
        "libimage_receiver_ndk.z.so:"
        "libimage_source_ndk.z.so:"
        "librawfile.z.so:"
        "libuv.so:"
        "libhilog.so:"
        "libnative_image.so:"
        "libnative_media_adec.so:"
        "libnative_media_aenc.so:"
        "libnative_media_codecbase.so:"
        "libnative_media_core.so:"
        "libnative_media_vdec.so:"
        "libnative_media_venc.so:"
        "libnative_media_avmuxer.so:"
        "libnative_media_avdemuxer.so:"
        "libnative_media_avsource.so:"
        "libnative_avscreen_capture.so:"
        "libavplayer.so:"
        // adaptor library
        "libohosadaptor.so:"
        "libusb_ndk.z.so:"
        "libvulkan.so:"
        // runtime library
        "libarkaotmanager.so:"
        "libarktarget_options.so:"
        "libhmicui18n.z.so:"
        "libes2panda-public.so:"
        "libes2panda-lib.so:"
        "libhmicuuc.z.so:"
        "libarkcompiler.so:"
        "libarkassembler.so:"
        "libarkfile.so:"
        "libarkziparchive.so:"
        "libarkbase.so:"
        "libc_secshared.so:"
        "libhilog_ndk.z.so:"
        "libarkplatform.so";
}
} // namespace

void DynamicInitNamespace(Dl_namespace *ns, void *parent, const char *entries, const char *name)
{
    if (ns == nullptr || entries == nullptr || name == nullptr) {
        TAG_LOGE(AAFwkTag::ETSRUNTIME, "Invaild args for init namespace.");
        return;
    }
    std::lock_guard<std::mutex> lock(g_dynLoaderMutex);
    if (g_hasInited.count(std::string(name))) {
        return;
    }
    dlns_init(ns, name);
    auto status = dlns_create2(ns, entries, 0);
    std::string errMsg;
    if (status != 0) {
        switch (status) {
            case FILE_EXISTS:
                errMsg = "dlns_create failed: File exists";
                break;
            case INVALID_ARGUMENT:
                errMsg = "dlns_create failed: Invalid argument";
                break;
            case OUT_OF_MEMORY:
                errMsg = "dlns_create failed: Out of memory";
                break;
            default:
                errMsg = "dlns_create failed, status: " + std::to_string(status);
        }
        if (sprintf_s(g_dlError, sizeof(g_dlError), errMsg.c_str()) == -1) {
            TAG_LOGE(AAFwkTag::ETSRUNTIME, "Fail to generate error msg.");
            return;
        }
        return;
    }
    if (parent) {
        dlns_inherit((Dl_namespace *)parent, ns, "allow_all_shared_libs");
    }
    Dl_namespace current;
    dlns_get(nullptr, &current);
    if (strcmp(name, "ets_app") != 0) {
        dlns_inherit(ns, &current, "allow_all_shared_libs");
    } else {
        InitSharedLibsSonames();
        dlns_inherit(ns, &current, g_sharedLibsSonames.c_str());
    }
    g_hasInited.insert(std::string(name));
}

void *DynamicLoadLibrary(Dl_namespace *ns, const char *dlPath, uint32_t mode)
{
    if (ns == nullptr) {
        dlns_get("ets_app", ns);
    }

    auto result = dlopen_ns(ns, dlPath, mode | RTLD_GLOBAL | RTLD_NOW);
    if (!result) {
        ReadDlError();
    }
    return result;
}

void *DynamicFindSymbol(void *so, const char *symbol)
{
    return dlsym(so, symbol);
}

void DynamicFreeLibrary(void *so)
{
    (void)dlclose(so);
}

const char *DynamicGetError()
{
    return g_dlError;
}
} // namespace EtsEnv
} // namespace OHOS