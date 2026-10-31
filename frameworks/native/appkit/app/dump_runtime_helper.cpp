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

#include "dump_runtime_helper.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cinttypes>
#include <dfx_signal_handler.h>
#include <fcntl.h>
#include <malloc.h>
#include <pthread.h>
#include <securec.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/xattr.h>
#include <unistd.h>
#include <dirent.h>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "app_mgr_client.h"
#include "faultloggerd_client.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "ets_runtime.h"
#include "singleton.h"
#include "dfx_jsnapi.h"
#include "dfx_frame.h"
#include "dfx_frame_formatter.h"
#include "unwinder.h"
#include "parameters.h"
#include "ffrt.h"
#include "directory_ex.h"
#include "storage_acl.h"
#include "hidebug_dump.h"
#include "dump_fd_guard.h"
#ifdef CJ_FRONTEND
#include "cj_runtime.h"
#endif
#if defined(NWEB)
#include "dump_arkweb_helper.h"
#endif

namespace OHOS {
namespace AppExecFwk {
const char *MODULE_NAME = "hiviewdfx.jsLeakWatcher";
const char *CHECK = "check";
const char *REQUIRE_NAPI = "requireNapi";

static bool g_betaVersion = OHOS::system::GetParameter("const.logsystem.versiontype", "unknown") == "beta";
static bool g_developMode = (OHOS::system::GetParameter("persist.hiview.leak_detector", "unknown") == "enable") ||
                            (OHOS::system::GetParameter("persist.hiview.leak_detector", "unknown") == "true");
static int g_oomDumpProcessQuota = 0;
static uint64_t g_lastOOMDumpTime = 0;
static constexpr const char* const EVENT_XATTR_NAME = "user.appevent";
static constexpr const char* const OOM_QUOTA_XATTR_NAME = "user.oomdump.quota";
static constexpr const char* const RUNNING_ID_NAME = "user.runningId";
static constexpr const char* const PROPERTY2C = "user.oomdumptelemetry.quota";
static constexpr const char* const TELEMETRY_ENABLE_PROPERTY = "user.oomdumptelemetry.enable";
static constexpr const char* const HIAPPEVENT_PATH = "/data/storage/el2/base/cache/hiappevent";
static constexpr const char* const OOM_QUOTA_PATH = "/data/storage/el2/base/cache/rawheap";
static constexpr const char* const JS_HEAP_LOGTYPE = "user.event_config.js_heap_logtype";
static constexpr const char* const EVENT_RAWHEAP = "event_rawheap";
#ifdef __LP64__
static constexpr const char* const SO_NAME = "liborphanmem.z.so";
#else
static constexpr const char* const SO_NAME = "";
#endif
static constexpr const char* const GET_MEM_LEAK_STRING_SYMBOL = "OHOS_GetMemLeakString";
static constexpr uint64_t OOM_DUMP_INTERVAL = 7 * 24 * 60 * 60;
static constexpr uint64_t OOM_DUMP_SPACE_LIMIT = 30uLL * 1024 * 1024 * 1024;
static constexpr uint32_t EVENT_RESOURCE_OVERLIMIT_MASK = 6;
static constexpr uint64_t BIT_MASK = 1;
static constexpr uint32_t BUF_SIZE_256 = 256;
static constexpr int32_t DECIMAL_BASE = 10;
static constexpr int32_t HEX_BASE = 16;
static constexpr int KB_PER_MB = 1024;
static constexpr size_t MEM_LEAK_MAX_SIZE = 100;
static constexpr int JSVM_SNAPSHOT = 0;
static constexpr int JSVM_RAW = 1;
static constexpr const char* const THREAD_STACK_MAP_PREFIX = "[anon:stack:";
static constexpr const char* const SELF_SMAPS_PATH = "/proc/self/smaps";
static constexpr const char* const SELF_TASK_PATH = "/proc/self/task";
static constexpr size_t THREAD_STACK_LEAK_MAX_FRAMES = 64;
static constexpr size_t THREAD_STACK_LEAK_MAX_STACKS = 10;
static constexpr size_t THREAD_STACK_SCAN_UNIT = sizeof(uintptr_t);
static constexpr size_t THREAD_STACK_READ_CHUNK = 4096; // 4KB batch read size

// Scan a buffer of stack memory for candidate PC values that fall inside
// executable maps. Called by CollectPcsFromStack after a chunk is read.
static void ScanChunkForPcs(const uint8_t *buf, uint64_t bytesValid,
    const std::shared_ptr<OHOS::HiviewDFX::DfxMaps> &maps, std::vector<uintptr_t> &pcs)
{
    // Defensive null check: callers currently guard maps before calling, but
    // keep this function self-consistent so future callers cannot deref null.
    if (maps == nullptr) {
        return;
    }
    for (uint64_t offset = bytesValid; offset >= sizeof(uintptr_t) &&
        pcs.size() < THREAD_STACK_LEAK_MAX_FRAMES; offset -= THREAD_STACK_SCAN_UNIT) {
        uintptr_t value = 0;
        if (memcpy_s(&value, sizeof(value), buf + offset - sizeof(uintptr_t), sizeof(value)) != EOK) {
            continue;
        }
        if (value == 0) {
            continue;
        }
        std::shared_ptr<OHOS::HiviewDFX::DfxMap> map = nullptr;
        if (maps->FindMapByAddr(static_cast<uintptr_t>(value), map) && map && map->IsMapExec()) {
            pcs.push_back(value);
        }
    }
}

enum {
    INDEX_DELIVERY_TS = 0,
    INDEX_COMPRESSED_TS_0,
    INDEX_COMPRESSED_TS_1,
    INDEX_COMPRESSED_TS_2,
    INDEX_COMPRESSED_TS_3,
    INDEX_COMPRESSED_TS_4,
    INDEX_APP_QUOTA,
    INDEX_HAS_SENT,
    INDEX_ROM_RSV_SIZE,
    PROPERTY2C_SIZE
};

enum {
    TYPE_TXT = 0,
    TYPE_VECTOR,
    TYPE_SNAPSHOT
};

DumpRuntimeHelper::DumpRuntimeHelper(const std::shared_ptr<OHOSApplication> &application)
    : application_(application)
{}

DumpRuntimeHelper::DumpRuntimeHelper(const std::shared_ptr<OHOSApplication> &application,
    const std::shared_ptr<ApplicationInfo> &appInfo)
    : application_(application), appInfo_(appInfo)
{}

bool DumpRuntimeHelper::Check2DOOMDumpOpt()
{
    char* env = getenv("DFX_RESOURCE_OVERLIMIT_OPTIONS");
    if (env == nullptr) {
        return false;
    }
    return strstr(env, "oomdump:enable") != nullptr;
}

void DumpRuntimeHelper::SetAppFreezeFilterCallback()
{
    CreateDirDelay(OOM_QUOTA_PATH);
    if (application_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null application");
        return;
    }
    auto &runtime = application_->GetRuntime();
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null runtime");
        return;
    }
    auto appfreezeFilterCallback =
        [this] (const int32_t pid, const bool needDecreaseQuota, std::string &eventConfig) -> bool {
        eventConfig = GetEventConfig(JS_HEAP_LOGTYPE);
        auto client = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
        if (client == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null client");
            return false;
        }
        if (!CheckOomdumpSwitch()) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed to check oomdump switch");
            return false;
        }
        if (CheckDumpQuota(eventConfig, needDecreaseQuota)) {
            client->SetAppFreezeFilter(pid);
            return true;
        }
        return false;
    };
    auto vm = (static_cast<AbilityRuntime::JsRuntime&>(*runtime)).GetEcmaVm();
    panda::DFXJSNApi::SetAppFreezeFilterCallback(vm, appfreezeFilterCallback);
}

void DumpRuntimeHelper::WriteRunningId()
{
    std::string curRunningId = DFX_GetAppRunningUniqueId();
    if (curRunningId.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "curRunningId is empty");
        return;
    }

    SetDirXattr(OOM_QUOTA_PATH, RUNNING_ID_NAME, curRunningId);
}

bool DumpRuntimeHelper::CheckOomdumpSwitch()
{
    // if value is disable, then no dump whatever
    std::string switchValue = OHOS::system::GetParameter("hiview.oomdump.switch", "unknown");
    TAG_LOGI(AAFwkTag::APPKIT, "oomdump switch: %{public}s", switchValue.c_str());
    return switchValue != "disable";
}

bool DumpRuntimeHelper::Check2CQuota()
{
    std::vector<int64_t> quota2C;
    if (!GetQuota(OOM_QUOTA_PATH, PROPERTY2C, quota2C, PROPERTY2C_SIZE)) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to GetQuota for PROPERTY2C");
        return false;
    }

    int compressQuota = GetCompressQuota(quota2C);
    int appQuota = static_cast<int>(quota2C[INDEX_APP_QUOTA]);
    int leftQuota = MIN(compressQuota, appQuota) - static_cast<int>(quota2C[INDEX_HAS_SENT]);
    if (leftQuota <= 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid quota, compress: %{public}d, app: %{public}d, sent: %{public}" PRId64,
            compressQuota, appQuota, quota2C[INDEX_HAS_SENT]);
        return false;
    }

    uint64_t now = GetCurrentTimestamp();
    if (quota2C[INDEX_DELIVERY_TS] < 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid quota2C[%{public}d]: %{public}" PRId64,
            INDEX_DELIVERY_TS, quota2C[INDEX_DELIVERY_TS]);
        return false;
    }
    uint64_t deliveryTs = static_cast<uint64_t>(quota2C[INDEX_DELIVERY_TS]);
    if (now < deliveryTs || now - deliveryTs > OOM_DUMP_INTERVAL) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid deliveryTs: %{public}" PRIu64, deliveryTs);
        return false;
    }

    uint64_t spaceQuota = static_cast<uint64_t>(quota2C[INDEX_ROM_RSV_SIZE]) * KB_PER_MB;
    if (!CheckOOMFreeSpace(spaceQuota)) {
        TAG_LOGE(AAFwkTag::APPKIT, "rom free space is less than spaceQuota: %{public}" PRIu64, spaceQuota);
        return false;
    }

    TAG_LOGE(AAFwkTag::APPKIT, "success check 2C Quota");
    return true;
}

bool DumpRuntimeHelper::Check2DQuota(bool needDecreaseQuota)
{
    if (!Check2DOOMDumpOpt()) {
        TAG_LOGE(AAFwkTag::APPKIT, "Check2DOOMDumpOpt failed");
        return false;
    }
    uint64_t time = 0ull;
    int quota = 0;
    int value = OHOS::system::GetIntParameter("persist.hiview.oomdump.process.maxcount", 0);
    if (value < 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid persist.hiview.oomdump.process.maxcount: %{public}d", value);
        return false;
    }
    uint32_t oomDumpProcessMaxQuota = static_cast<uint32_t>(value);
    if (!Get2DQuota(OOM_QUOTA_PATH, OOM_QUOTA_XATTR_NAME, time, quota)) {
        TAG_LOGI(AAFwkTag::APPKIT, "failed to Get2DQuota, need to Init2DOOMDumpQuota");
        if (!Init2DOOMDumpQuota(OOM_QUOTA_PATH, oomDumpProcessMaxQuota)) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed Init2DOOMDumpQuota");
            return false;
        }
    }
    value = OHOS::system::GetIntParameter("persist.hiview.oomdump.maxcount", 0);
    if (value < 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid persist.hiview.oomdump.maxcount: %{public}d", value);
        return false;
    }
    uint32_t oomDumpMaxQuota = static_cast<uint32_t>(value);
    if (!Check2DOOMDumpQuota(oomDumpMaxQuota, oomDumpProcessMaxQuota)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Check2DOOMDumpQuota failed");
        return false;
    }
    if (!Set2DQuota(OOM_QUOTA_PATH, GetCurrentTimestamp(), g_oomDumpProcessQuota - 1)) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to Set2DQuota");
        return false;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "succeed to Check2DQuota");
    return true;
}

void DumpRuntimeHelper::DumpJsHeap(const OHOS::AppExecFwk::JsHeapDumpInfo &info)
{
    if (application_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null application");
        return;
    }
    auto &runtime = application_->GetRuntime();
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null runtime");
        return;
    }
    auto language = runtime->GetLanguage();
    if (language != OHOS::AbilityRuntime::Runtime::Language::JS &&
        language != OHOS::AbilityRuntime::Runtime::Language::ETS) {
        TAG_LOGE(AAFwkTag::APPKIT, "runtime language is not JS or ETS");
        return;
    }
    if (info.needLeakobj && language == OHOS::AbilityRuntime::Runtime::Language::JS) {
        std::string checkList = "";
        GetCheckList(runtime, checkList);
        WriteCheckList(checkList);
    }

    if (info.needSnapshot) {
        OHOS::AbilityRuntime::Runtime::JsHeapDumpParam param;
        param.isFullGC = info.needGc;
        param.isBinary = info.needBinary;
        param.isClearNodeIdCache = info.needClearNodeIdCache;
        param.isProcDump = info.needProcDump;
        if (appInfo_ != nullptr) {
            param.languageEnv = appInfo_->arkTSMode;
        } else {
            param.languageEnv = OHOS::AbilityRuntime::CODE_LANGUAGE_ARKTS_1_0;
        }
        TAG_LOGI(AAFwkTag::APPKIT, "DumpJsHeap snapshot, language: %{public}d, languageEnv: %{public}s",
            static_cast<int>(language), param.languageEnv.c_str());
        runtime->DumpHeapSnapshot(info.tid, param);
        return;
    }
    if (info.needGc) {
        DumpJsHeapGc(runtime, info);
    }
}

void DumpRuntimeHelper::DumpJsHandleMap(const OHOS::AppExecFwk::JsHandleMapInfo &info)
{
    (void)info;
    if (application_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null application");
        return;
    }
    auto &runtime = application_->GetRuntime();
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null runtime");
        return;
    }
    auto language = runtime->GetLanguage();
    if (language != OHOS::AbilityRuntime::Runtime::Language::JS &&
        language != OHOS::AbilityRuntime::Runtime::Language::ETS) {
        TAG_LOGE(AAFwkTag::APPKIT, "runtime language is not JS or ETS");
        return;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "DumpJsHandleMap, language: %{public}d", static_cast<int>(language));
    runtime->DumpJsHandleMap();
}

void DumpRuntimeHelper::DumpJsHeapGc(const std::unique_ptr<AbilityRuntime::Runtime> &runtime,
    const OHOS::AppExecFwk::JsHeapDumpInfo &info)
{
    if (appInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null appInfo");
        return;
    }
    if (appInfo_->arkTSMode == OHOS::AbilityRuntime::CODE_LANGUAGE_ARKTS_1_0) {
        runtime->ForceFullGC(info.tid);
        TAG_LOGI(AAFwkTag::APPKIT, "force full gc for arkts 1.0");
        return;
    }
    auto &etsRuntime = static_cast<OHOS::AbilityRuntime::ETSRuntime&>(*runtime);
    etsRuntime.XGC();
    if (appInfo_->arkTSMode == OHOS::AbilityRuntime::CODE_LANGUAGE_ARKTS_1_2) {
        runtime->ForceFullGC(info.tid);
        TAG_LOGI(AAFwkTag::APPKIT, "force full gc for arkts 1.2");
        return;
    }
    if (appInfo_->arkTSMode == OHOS::AbilityRuntime::CODE_LANGUAGE_ARKTS_HYBRID) {
        auto& jsRuntimePtr = etsRuntime.GetJsRuntime();
        auto jsRuntime = static_cast<OHOS::AbilityRuntime::JsRuntime*>(jsRuntimePtr.get());
        if (jsRuntime != nullptr) {
            jsRuntime->ForceFullGC(info.tid);
        }
        runtime->ForceFullGC(info.tid);
        TAG_LOGI(AAFwkTag::APPKIT, "force full gc for arkts hybrid");
    }
}

void DumpRuntimeHelper::DumpCjHeap(const OHOS::AppExecFwk::CjHeapDumpInfo &info)
{
#ifdef CJ_FRONTEND
    auto cjEnv = OHOS::CJEnv::LoadInstance();
    if (cjEnv == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null cjEnv");
        return;
    }
    if (info.needSnapshot) {
        int32_t fd = RequestFileDescriptor(static_cast<int32_t>(FaultLoggerType::CJ_HEAP_SNAPSHOT));
        if (fd < 0) {
            TAG_LOGE(AAFwkTag::APPKIT, "fd:%{public}d.\n", fd);
            return;
        }
        DumpFdGuard snapshotFdGuard(fd);
        cjEnv->dumpHeapSnapshot(fd);
        return;
    }
    if (info.needGc) {
        cjEnv->forceFullGC();
    }
#endif
}

void DumpRuntimeHelper::DumpMem(const OHOS::AppExecFwk::MemDumpInfo &info, sptr<IMemDumpCallback> callback)
{
    TAG_LOGI(AAFwkTag::APPKIT, "dump type: %{public}d", static_cast<uint32_t>(info.dumpType));
    auto client = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();

    if (info.dumpType == MemDumpType::NATIVE) {
        std::string dumpResult;
        auto task = [self = shared_from_this(), info, callback, client, dumpResult]() mutable {
            self->DumpNativeHeap(info, dumpResult);
            if (client != nullptr && callback != nullptr) {
                client->ReportDumpMemResult(callback, dumpResult);
            }
        };
        ffrt::submit(task, {}, {}, {ffrt::task_attr().name("ffrt_DumpNativeHeap")});
        return;
    }

    if (info.dumpType == MemDumpType::THREAD_STACK_LEAK) {
        SubmitThreadStackLeakTask(callback);
        return;
    }

    std::string dumpResult;
    if (info.dumpType == MemDumpType::KMP_KOTLIN) {
        DumpKmpKotlinHeap(info);
    }
    if (info.dumpType == MemDumpType::JSVM) {
        DumpJsvmHeap(info);
    }
    if (info.dumpType == MemDumpType::ARKWEB_JS) {
        DumpArkwebJsHeap(info);
    }
    if (info.dumpType == MemDumpType::ARKTS_HEAP) {
        DumpArktsHeapSize(info, dumpResult);
    }
    if (client != nullptr && callback != nullptr) {
        client->ReportDumpMemResult(callback, dumpResult);
    }
}

bool DumpRuntimeHelper::ReportThreadStackLeakResult(const sptr<IMemDumpCallback> &callback,
    const std::string &dumpResult)
{
    auto client = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (client == nullptr) {
        return false;
    }
    try {
        auto ret = client->ReportDumpMemResult(callback, dumpResult);
        if (ret == RESULT_OK) {
            return true;
        }
        TAG_LOGE(AAFwkTag::APPKIT, "ReportDumpMemResult failed: %{public}d", static_cast<int32_t>(ret));
    } catch (const std::exception &e) {
        TAG_LOGE(AAFwkTag::APPKIT, "ReportDumpMemResult exception: %{public}s", e.what());
    } catch (...) {
        TAG_LOGE(AAFwkTag::APPKIT, "ReportDumpMemResult unknown exception");
    }
    return false;
}

void DumpRuntimeHelper::SubmitThreadStackLeakTask(sptr<IMemDumpCallback> callback)
{
    // Async branch: result is reported via IPC callback. On success, the service
    // invokes the callback. On failure, we fall back to invoking the callback
    // directly on this ffrt worker thread so the caller does not hang.
    auto task = [callback]() {
        if (callback == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "callback is null, cannot report thread stack leak result");
            return;
        }
        std::string dumpResult;
        try {
            dumpResult = DumpRuntimeHelper::DetectAndUnwindThreadStackLeak();
        } catch (const std::exception &e) {
            TAG_LOGE(AAFwkTag::APPKIT, "DetectAndUnwindThreadStackLeak exception: %{public}s", e.what());
            dumpResult = "DetectAndUnwindThreadStackLeak failed: exception occurred\n";
        } catch (...) {
            TAG_LOGE(AAFwkTag::APPKIT, "DetectAndUnwindThreadStackLeak unknown exception");
            dumpResult = "DetectAndUnwindThreadStackLeak failed: unknown exception\n";
        }
        if (!ReportThreadStackLeakResult(callback, dumpResult)) {
            // Fallback: invoke callback directly on this ffrt worker thread so the
            // caller does not hang waiting for the result. The callback is expected
            // to be a Binder proxy whose OnMemDumpDone marshals across the binder
            // driver (thread-safe); an in-process stub implementation must guarantee
            // its own thread-safety since this call runs off the caller's thread.
            try {
                callback->OnMemDumpDone(dumpResult);
            } catch (const std::exception &e) {
                TAG_LOGE(AAFwkTag::APPKIT, "OnMemDumpDone exception: %{public}s", e.what());
            } catch (...) {
                TAG_LOGE(AAFwkTag::APPKIT, "OnMemDumpDone unknown exception");
            }
        }
    };
    ffrt::submit(task, {}, {}, {ffrt::task_attr().name("ffrt_DumpThreadStackLeak")});
}

void DumpRuntimeHelper::DumpNativeHeap(const OHOS::AppExecFwk::MemDumpInfo &info, std::string &dumpResult)
{
    if (info.needLeakobj) {
        TAG_LOGI(AAFwkTag::APPKIT, "dump native need leakobj");
        if (!GetDumpResult(dumpResult)) {
            TAG_LOGE(AAFwkTag::APPKIT, "GetDumpResult failed");
        }
    } else {
        TAG_LOGI(AAFwkTag::APPKIT, "dump native no need leakobj");
        int32_t fd = RequestFileDescriptor(static_cast<int32_t>(FaultLoggerType::NATIVE_SNAPSHOT));
        if (fd > 0) {
            DumpFdGuard nativeFdGuard(fd);
            if (!GetSnapshot(fd)) {
                TAG_LOGE(AAFwkTag::APPKIT, "GetSnapshot failed");
            }
        } else {
            TAG_LOGE(AAFwkTag::APPKIT, "RequestFileDescriptor failed");
        }
    }
}

GetMemLeakStringFunc DumpRuntimeHelper::LoadMemLeakFunc(void **handle)
{
    if (handle == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "LoadMemLeakFunc handle is null");
        return nullptr;
    }
    if (SO_NAME[0] == '\0') {
        TAG_LOGW(AAFwkTag::APPKIT, "mem leak so is unsupported on non-64-bit");
        return nullptr;
    }
    void* hd = dlopen(SO_NAME, RTLD_LAZY);
    if (!hd) {
        TAG_LOGE(AAFwkTag::APPKIT, "Error loading %{public}s", SO_NAME);
        return nullptr;
    }
    auto func = reinterpret_cast<GetMemLeakStringFunc>(dlsym(hd, GET_MEM_LEAK_STRING_SYMBOL));
    if (!func) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to resolve symbol: %{public}s", GET_MEM_LEAK_STRING_SYMBOL);
        dlclose(hd);
        return nullptr;
    }
    *handle = hd;
    return func;
}

bool DumpRuntimeHelper::GetDumpResult(std::string &dumpResult)
{
    void* handle = nullptr;
    auto func = LoadMemLeakFunc(&handle);
    if (!func) {
        return false;
    }
    char* buf = nullptr;
    int outLen = 0;
    bool ret = func(TYPE_TXT, &buf, MEM_LEAK_MAX_SIZE, &outLen);
    if (!ret || buf == nullptr || outLen <= 0 ||
        static_cast<size_t>(outLen) > malloc_usable_size(buf)) {
        TAG_LOGE(AAFwkTag::APPKIT, "TYPE_TXT Error, ret:%{public}d, outLen:%{public}d", ret, outLen);
        free(buf);
        if (handle != nullptr) {
            dlclose(handle);
        }
        return false;
    }
    dumpResult.assign(buf, static_cast<size_t>(outLen));
    TAG_LOGI(AAFwkTag::APPKIT, "TYPE_TXT finish, size:%{public}zu", dumpResult.size());
    free(buf);
    if (handle != nullptr) {
        dlclose(handle);
    }
    return true;
}

bool DumpRuntimeHelper::GetSnapshot(int fd)
{
    void* handle = nullptr;
    auto func = LoadMemLeakFunc(&handle);
    if (!func) {
        return false;
    }
    char* buf = nullptr;
    int outLen = 0;
    bool ret = func(TYPE_SNAPSHOT, &buf, MEM_LEAK_MAX_SIZE, &outLen);
    if (!ret || buf == nullptr || outLen <= 0 ||
        static_cast<size_t>(outLen) > malloc_usable_size(buf)) {
        TAG_LOGE(AAFwkTag::APPKIT, "TYPE_SNAPSHOT Error, ret:%{public}d, outLen:%{public}d", ret, outLen);
        free(buf);
        if (handle != nullptr) {
            dlclose(handle);
        }
        return false;
    }
    lseek(fd, 0, SEEK_END);
    size_t total = 0;
    size_t len = static_cast<size_t>(outLen);
    while (total < len) {
        ssize_t written = write(fd, buf + total, len - total);
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            TAG_LOGE(AAFwkTag::APPKIT, "write snapshot failed, errno:%{public}d", errno);
            free(buf);
            if (handle != nullptr) {
                dlclose(handle);
            }
            return false;
        }
        total += static_cast<size_t>(written);
    }
    TAG_LOGI(AAFwkTag::APPKIT, "TYPE_SNAPSHOT finish, outLen:%{public}d", outLen);
    free(buf);
    if (handle != nullptr) {
        dlclose(handle);
    }
    return true;
}

// Extract tid from a thread stack map name like "[anon:stack:11111]".
// Returns 0 if the tid cannot be parsed.
int32_t DumpRuntimeHelper::ParseTidFromMapName(const std::string &mapName)
{
    auto pos = mapName.find(THREAD_STACK_MAP_PREFIX);
    if (pos == std::string::npos) {
        return 0;
    }
    pos += std::string(THREAD_STACK_MAP_PREFIX).size();
    std::string tidStr;
    for (; pos < mapName.size(); pos++) {
        if (mapName[pos] >= '0' && mapName[pos] <= '9') {
            tidStr.push_back(mapName[pos]);
        } else {
            break;
        }
    }
    if (tidStr.empty()) {
        return 0;
    }
    errno = 0;
    char *end = nullptr;
    int64_t val = strtoll(tidStr.c_str(), &end, DECIMAL_BASE);
    if (errno != 0 || end == tidStr.c_str() || *end != '\0') {
        return 0;
    }
    if (val > INT32_MAX || val < INT32_MIN) {
        return 0;
    }
    return static_cast<int32_t>(val);
}

// Check whether the given tid is still alive in current process by looking up
// /proc/self/task/<tid>. Returns true if the tid directory exists.
bool DumpRuntimeHelper::IsTidAlive(int32_t tid)
{
    if (tid <= 0) {
        return false;
    }
    std::string taskPath = std::string(SELF_TASK_PATH) + "/" + std::to_string(tid);
    struct stat st{};
    if (stat(taskPath.c_str(), &st) != 0) {
        return false;
    }
    return S_ISDIR(st.st_mode);
}

// Count total alive threads by listing /proc/self/task entries.
size_t DumpRuntimeHelper::CountTotalThreads()
{
    DIR *dir = opendir(SELF_TASK_PATH);
    if (dir == nullptr) {
        return 0;
    }
    size_t count = 0;
    struct dirent *entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_name[0] == '.') {
            continue;
        }
        count++;
    }
    closedir(dir);
    return count;
}

// Parse /proc/self/smaps and collect all thread stack map regions whose name
// starts with THREAD_STACK_MAP_PREFIX. Each region's tid is extracted from the
// map name and validated against /proc/self/task.
std::vector<ThreadStackRegion> DumpRuntimeHelper::ParseLeakedThreadStackRegions()
{
    std::vector<ThreadStackRegion> regions;
    std::ifstream smaps(SELF_SMAPS_PATH);
    if (!smaps.is_open()) {
        TAG_LOGE(AAFwkTag::APPKIT, "open smaps failed");
        return regions;
    }
    std::string line;
    while (std::getline(smaps, line)) {
        if (line.find(THREAD_STACK_MAP_PREFIX) == std::string::npos) {
            continue;
        }
        auto headerEnd = line.find(' ');
        auto dashPos = line.find('-');
        if (headerEnd == std::string::npos || dashPos == std::string::npos || dashPos >= headerEnd) {
            continue;
        }
        ThreadStackRegion region;
        errno = 0;
        char *end = nullptr;
        std::string beginStr = line.substr(0, dashPos);
        region.begin = strtoull(beginStr.c_str(), &end, HEX_BASE);
        if (errno != 0 || end == beginStr.c_str() || *end != '\0') {
            continue;
        }
        errno = 0;
        end = nullptr;
        std::string endStr = line.substr(dashPos + 1, headerEnd - dashPos - 1);
        region.end = strtoull(endStr.c_str(), &end, HEX_BASE);
        if (errno != 0 || end == endStr.c_str() || *end != '\0') {
            continue;
        }
        auto namePos = line.find(THREAD_STACK_MAP_PREFIX);
        region.mapName = line.substr(namePos);
        region.tid = ParseTidFromMapName(region.mapName);
        if (region.tid > 0 && region.begin < region.end && !IsTidAlive(region.tid)) {
            regions.emplace_back(std::move(region));
        }
    }
    return regions;
}

// Scan a leaked thread stack region for candidate PC values. The PC values are
// read from the stack memory as uintptr_t values that fall inside executable
// maps. Memory is read in 4KB chunks via process_vm_readv to minimize system
// call overhead, then each uintptr_t in the buffer is checked against DfxMaps
// for executable mapping. process_vm_readv returns an error instead of
// triggering SIGSEGV on unmapped pages, so the readv itself is the safety
// boundary — no prior address readability check is needed.
// NOTE: DfxMaps is a snapshot created once in DetectAndUnwindThreadStackLeak.
// Between snapshot creation and the readv call, the process's virtual memory
// layout may change (munmap, page reclaim). process_vm_readv handles this
// safely by returning an error rather than triggering SIGSEGV.
std::vector<uintptr_t> DumpRuntimeHelper::CollectPcsFromStack(
    uint64_t begin, uint64_t end, const std::shared_ptr<OHOS::HiviewDFX::DfxMaps> &maps)
{
    std::vector<uintptr_t> pcs;
    if (maps == nullptr) {
        return pcs;
    }
    if (end < begin || end - begin < THREAD_STACK_SCAN_UNIT) {
        return pcs;
    }
    end &= ~static_cast<uint64_t>(sizeof(uintptr_t) - 1);
    std::vector<uint8_t> buffer;
    buffer.reserve(THREAD_STACK_READ_CHUNK);
    for (uint64_t chunkEnd = end; chunkEnd > begin && pcs.size() < THREAD_STACK_LEAK_MAX_FRAMES;) {
        uint64_t chunkBegin = chunkEnd - std::min(static_cast<uint64_t>(THREAD_STACK_READ_CHUNK), chunkEnd - begin);
        chunkBegin = (chunkBegin + sizeof(uintptr_t) - 1) & ~static_cast<uint64_t>(sizeof(uintptr_t) - 1);
        if (chunkBegin >= chunkEnd) {
            break;
        }
        uint64_t chunkSize = chunkEnd - chunkBegin;
        // Use explicit cast to silence narrowing/portability warnings:
        // chunkSize is uint64_t but resize() takes size_t, and the remote
        // address must go through uintptr_t before reinterpret_cast to void*
        // to avoid truncation on 32-bit targets.
        buffer.resize(static_cast<size_t>(chunkSize));
        size_t readLen = static_cast<size_t>(chunkSize);
        struct iovec localiov = {buffer.data(), readLen};
        struct iovec remoteiov = {reinterpret_cast<void *>(static_cast<uintptr_t>(chunkBegin)), readLen};
        ssize_t nread = process_vm_readv(getpid(), &localiov, 1, &remoteiov, 1, 0);
        if (nread <= 0) {
            TAG_LOGW(AAFwkTag::APPKIT, "process_vm_readv failed, errno:%{public}d", errno);
            chunkEnd = chunkBegin;
            continue;
        }
        uint64_t bytesValid = static_cast<uint64_t>(nread) &
            ~static_cast<uint64_t>(sizeof(uintptr_t) - 1);
        ScanChunkForPcs(buffer.data(), bytesValid, maps, pcs);
        chunkEnd = chunkBegin;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "CollectPcsFromStack: pcs:%{public}zu", pcs.size());
    return pcs;
}

// Build a standardized stack string for one leaked thread stack using
// faultloggerd's Unwinder + DfxFrameFormatter, which produces the canonical
// "#00 pc <pc> <mapName>(<funcName>+<offset>)" format.
// Returns empty string if no frames could be resolved.
std::string DumpRuntimeHelper::FormatLeakedStack(int32_t tid, const std::vector<uintptr_t> &pcs)
{
    if (pcs.empty()) {
        return "";
    }
    std::vector<OHOS::HiviewDFX::DfxFrame> frames;
    OHOS::HiviewDFX::Unwinder::GetLocalFramesByPcs(frames, pcs);
    if (frames.empty()) {
        return "";
    }
    std::ostringstream oss;
    oss << "tid:" << tid << "\n";
    oss << OHOS::HiviewDFX::DfxFrameFormatter::GetFramesStr(frames);
    return oss.str();
}

std::string DumpRuntimeHelper::DetectAndUnwindThreadStackLeak()
{
    size_t totalThreadCount = CountTotalThreads();
    auto regions = ParseLeakedThreadStackRegions();
    size_t leakCount = regions.size();
    std::string statusSuffix;
    std::string body;

    if (regions.empty()) {
        TAG_LOGI(AAFwkTag::APPKIT, "no leaked thread stack found, total thread count:%{public}zu",
            totalThreadCount);
    } else {
        auto maps = OHOS::HiviewDFX::DfxMaps::Create(0, true, false);
        if (maps == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed to create DfxMaps, skip stack unwinding");
            statusSuffix = " (unwind failed)";
        } else {
            body = UnwindLeakedStacks(regions, maps);
        }
    }

    std::ostringstream header;
    header << "total thread count: " << totalThreadCount
           << ", leak thread stack count: " << leakCount << statusSuffix << "\n";
    return header.str() + body;
}

std::string DumpRuntimeHelper::UnwindLeakedStacks(
    const std::vector<ThreadStackRegion> &regions,
    std::shared_ptr<OHOS::HiviewDFX::DfxMaps> &maps)
{
    std::string dumpResult;
    size_t printedStacks = 0;
    for (const auto &region : regions) {
        if (printedStacks >= THREAD_STACK_LEAK_MAX_STACKS) {
            TAG_LOGI(AAFwkTag::APPKIT,
                "max stack limit reached, printedStacks:%{public}zu totalOrphan:%{public}zu",
                printedStacks, regions.size());
            break;
        }
        TAG_LOGW(AAFwkTag::APPKIT, "leaked stack found, tid:%{public}d not alive", region.tid);
        if (IsTidAlive(region.tid)) {
            TAG_LOGI(AAFwkTag::APPKIT, "tid:%{public}d became alive before scan, skipping", region.tid);
            continue;
        }
        auto pcs = CollectPcsFromStack(region.begin, region.end, maps);
        std::string stackStr = FormatLeakedStack(region.tid, pcs);
        if (stackStr.empty()) {
            TAG_LOGW(AAFwkTag::APPKIT, "no frames resolved for tid:%{public}d, skipping", region.tid);
            continue;
        }
        printedStacks++;
        dumpResult.append(stackStr);
    }
    return dumpResult;
}

void DumpRuntimeHelper::DumpKmpKotlinHeap(const OHOS::AppExecFwk::MemDumpInfo &info)
{
    int32_t fd = RequestFileDescriptor(static_cast<int32_t>(FaultLoggerType::KMP_HEAP_SNAPSHOT));
    if (fd < 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "RequestFileDescriptor failed");
        return;
    }
    DumpFdGuard kmpFdGuard(fd);
    if (info.needGc) {
        DumpKmpGc(fd);
    } else {
        DumpKmpSnapshot(fd, info.mayReportToOEM);
    }
}

void DumpRuntimeHelper::DumpKmpGc(int32_t fd)
{
    auto& dumpListener = OHOS::HiviewDFX::HidebugMemDumpListener::GetInstance();
    bool gcRet = dumpListener.TriggerListener("KMP", fd,
        OH_HiDebug_MemListenerType::OH_HIDEBUG_RUNNING_GC, false, nullptr);
    if (!gcRet) {
        TAG_LOGE(AAFwkTag::APPKIT, "TriggerListener gc failed");
    }
}

void DumpRuntimeHelper::DumpKmpSnapshot(int32_t fd, bool mayReportToOEM)
{
    auto& dumpListener = OHOS::HiviewDFX::HidebugMemDumpListener::GetInstance();
    bool ret = dumpListener.TriggerListener("KMP", fd,
        OH_HiDebug_MemListenerType::OH_HIDEBUG_DUMP_SNAPSHOT, mayReportToOEM, nullptr);
    if (!ret) {
        TAG_LOGE(AAFwkTag::APPKIT, "TriggerListener failed");
    }
}

void DumpRuntimeHelper::DumpJsvmHeap(const OHOS::AppExecFwk::MemDumpInfo &info)
{
    TAG_LOGI(AAFwkTag::APPKIT, "dump jsvm heaps, tid:%{public}d, needRaw:%{public}d", info.tid, info.needRaw);
    void* jsvmHandle = dlopen("libjsvm.so", RTLD_LAZY);
    if (jsvmHandle == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "jsvm dlopen failed");
        return;
    }
    using jsvmFunc = int (*)(uint32_t, int);
    auto jsvmDump = reinterpret_cast<jsvmFunc>(dlsym(jsvmHandle, "jsvm_dump_heapsnapshot"));
    if (jsvmDump == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "jsvm dlsym failed");
        dlclose(jsvmHandle);
        return;
    }
    int dumpType = info.needRaw ? JSVM_RAW : JSVM_SNAPSHOT;
    int ret = jsvmDump(info.tid, dumpType);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "jsvm dump failed");
        dlclose(jsvmHandle);
        return;
    }
    dlclose(jsvmHandle);
}

void DumpRuntimeHelper::DumpArkwebJsHeap(const OHOS::AppExecFwk::MemDumpInfo &info)
{
#if defined(NWEB)
    TAG_LOGI(AAFwkTag::APPKIT, "dump arkwebjs heaps, renderPid:%{public}u, needDump:%{public}d, "
        "needGc:%{public}d, needRaw:%{public}d", info.renderPid, info.needDump, info.needGc, info.needRaw);
    if (info.pid == 0 || info.renderPid == 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid pid:%{public}u, renderPid:%{public}u", info.pid, info.renderPid);
        return;
    }
    DumpArkWebHelper::DumpArkWebJSHeap(static_cast<int32_t>(info.pid), static_cast<int32_t>(info.renderPid),
        info.needDump, info.needGc, info.needRaw);
#endif
}

void DumpRuntimeHelper::DumpArktsHeapSize(const OHOS::AppExecFwk::MemDumpInfo &info, std::string &dumpResult)
{
    if (application_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null application");
        return;
    }
    auto &runtime = application_->GetRuntime();
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null runtime");
        return;
    }
    auto language = runtime->GetLanguage();
    if (language != AbilityRuntime::Runtime::Language::JS &&
        language != AbilityRuntime::Runtime::Language::ETS) {
        TAG_LOGE(AAFwkTag::APPKIT, "runtime language is not JS or ETS");
        return;
    }
    AbilityRuntime::JsRuntime* jsRuntime = nullptr;
    if (language == AbilityRuntime::Runtime::Language::JS) {
        jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    } else {
        auto &etsRuntime = static_cast<AbilityRuntime::ETSRuntime&>(*runtime);
        auto &jsRuntimePtr = etsRuntime.GetJsRuntime();
        if (jsRuntimePtr == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null jsRuntime in ets");
            return;
        }
        jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(jsRuntimePtr.get());
    }
    if (jsRuntime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null jsRuntime");
        return;
    }
    size_t heapSize = jsRuntime->GetHeapTotalSize();
    char threadName[THREAD_NAME_MAX_LEN] = {0};
    if (pthread_getname_np(pthread_self(), threadName, sizeof(threadName)) != 0) {
        TAG_LOGW(AAFwkTag::APPKIT, "pthread_getname_np failed");
    }
    dumpResult = std::to_string(heapSize) + "|" + threadName;
    TAG_LOGI(AAFwkTag::APPKIT, "arkts heap size: %{public}zu bytes, thread: %{public}s", heapSize, threadName);
}

void DumpRuntimeHelper::GetCheckList(const std::unique_ptr<AbilityRuntime::Runtime> &runtime, std::string &checkList)
{
    if (runtime->GetLanguage() != AbilityRuntime::Runtime::Language::JS) {
        TAG_LOGE(AAFwkTag::APPKIT, "current language not js");
        return;
    }
    AbilityRuntime::JsRuntime &jsruntime = static_cast<AbilityRuntime::JsRuntime&>(*runtime);
    AbilityRuntime::HandleScope handleScope(jsruntime);
    auto env = jsruntime.GetNapiEnv();

    napi_value global = nullptr;
    napi_get_global(env, &global);
    napi_value requireValue = GetJsLeakModule(env, global);
    if (requireValue == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null requireValue");
        return;
    }
    napi_value result = GetMethodCheck(env, requireValue, global);
    if (result == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null result");
        return;
    }

    size_t checkListSize = 0;
    napi_get_value_string_utf8(env, result, nullptr, 0, &checkListSize);
    checkList.resize(checkListSize + 1);
    napi_get_value_string_utf8(env, result, &checkList[0], checkListSize + 1, &checkListSize);
}

napi_value DumpRuntimeHelper::GetJsLeakModule(napi_env env, napi_value global)
{
    napi_value napiFunc = nullptr;
    napi_status status = napi_get_named_property(env, global, REQUIRE_NAPI, &napiFunc);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "fail, %{public}d", status);
        return nullptr;
    }
    napi_value moduleName = nullptr;
    status = napi_create_string_utf8(env, MODULE_NAME, strlen(MODULE_NAME), &moduleName);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "fail, %{public}d", status);
        return nullptr;
    }
    napi_value param[1] = {moduleName};
    napi_value requireValue = nullptr;
    status = napi_call_function(env, global, napiFunc, 1, &param[0], &requireValue);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "fail, %{public}d", status);
        return nullptr;
    }
    return requireValue;
}

napi_value DumpRuntimeHelper::GetMethodCheck(napi_env env, napi_value requireValue, napi_value global)
{
    napi_value methodCheck = nullptr;
    napi_status status = napi_get_named_property(env, requireValue, CHECK, &methodCheck);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "fail, %{public}d", status);
        return nullptr;
    }
    napi_valuetype valuetype = napi_undefined;
    status = napi_typeof(env, methodCheck, &valuetype);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed, %{public}d", status);
        return nullptr;
    }
    napi_value result = nullptr;
    status = napi_call_function(env, global, methodCheck, 0, nullptr, &result);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "fail, %{public}d", status);
        return nullptr;
    }
    return result;
}

void DumpRuntimeHelper::WriteCheckList(const std::string &checkList)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    int32_t fd = RequestFileDescriptor(static_cast<int32_t>(FaultLoggerType::JS_HEAP_LEAK_LIST));
    if (fd < 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "fd:%{public}d.\n", fd);
        return;
    }
    DumpFdGuard checkListFdGuard(fd);
    if (write(fd, checkList.c_str(), strlen(checkList.c_str())) == -1) {
        TAG_LOGE(AAFwkTag::APPKIT, "fd:%{public}d, errno:%{public}d.\n", fd, errno);
        return;
    }
}

void DumpRuntimeHelper::CreateDirDelay(const std::string &path)
{
    ffrt::submit([=] {
        if (!CreateDir(path)) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed to create %{public}s", path.c_str());
        } else {
            TAG_LOGI(AAFwkTag::APPKIT, "success to create %{public}s", path.c_str());
        }
        WriteRunningId();
        }, {}, {}, {ffrt::task_attr().name("ffrt_dfr_CreateDir")});
}

bool DumpRuntimeHelper::Init2DOOMDumpQuota(const std::string &path, uint32_t oomDumpProcessMaxQuota)
{
    uint64_t currentTime = GetCurrentTimestamp();
    if (!Set2DQuota(path, currentTime, oomDumpProcessMaxQuota)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Set2DQuota failed, current: %{public}" PRIu64 ", quota: %{public}d",
            currentTime, oomDumpProcessMaxQuota);
        return false;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "Init dumpTime: %{public}" PRIu64 ", quota: %{public}d.",
        currentTime, oomDumpProcessMaxQuota);
    return true;
}

bool DumpRuntimeHelper::CreateDir(const std::string &path)
{
    if (IsFileExists(path)) {
        TAG_LOGI(AAFwkTag::APPKIT, "File existed. dir: %{public}s", path.c_str());
        return true;
    }
    if (mkdir(path.c_str(), S_IRWXU) != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to create dir: %{public}s", path.c_str());
        return false;
    }
    constexpr mode_t defaultLogDirMode = 0770;
    if (!OHOS::ChangeModeDirectory(path, defaultLogDirMode)) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to changeMode %{public}s", path.c_str());
        rmdir(path.c_str());
        return false;
    }
    if (OHOS::StorageDaemon::AclSetAccess(path, "g:1201:rwx") != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to AclSetAccess, path: %{public}s", path.c_str());
        rmdir(path.c_str());
        return false;
    }
    return true;
}

bool DumpRuntimeHelper::Check2DOOMDumpQuota(uint32_t oomDumpMaxQuota, uint32_t oomDumpProcessMaxQuota)
{
    if (!CheckAppListenedEvents(HIAPPEVENT_PATH)) {
        TAG_LOGI(AAFwkTag::APPKIT, "Not subscribe oom event.");
        return false;
    }
    if (oomDumpMaxQuota <= 0) {
        TAG_LOGI(AAFwkTag::APPKIT, "The whole machine quota has been exhausted, quota=%{public}u", oomDumpMaxQuota);
        return false;
    }
    if (!CheckOOMFreeSpace(OOM_DUMP_SPACE_LIMIT)) {
        TAG_LOGI(AAFwkTag::APPKIT, "Device space is not enough.");
        return false;
    }
    if (!Get2DQuota(OOM_QUOTA_PATH, OOM_QUOTA_XATTR_NAME, g_lastOOMDumpTime, g_oomDumpProcessQuota)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Check2DOOMDumpQuota failed to get quota.");
        return false;
    }
    uint64_t currentTime = GetCurrentTimestamp();
    if (currentTime - g_lastOOMDumpTime <= OOM_DUMP_INTERVAL) {
        if (g_oomDumpProcessQuota <= 0) {
            TAG_LOGI(AAFwkTag::APPKIT, "Weekly quota has been exhausted, quota=%{public}d", g_oomDumpProcessQuota);
            return false;
        }
    } else {
        TAG_LOGI(AAFwkTag::APPKIT, "Over one week, reset process quota=%{public}u", oomDumpProcessMaxQuota);
        g_oomDumpProcessQuota = static_cast<int>(oomDumpProcessMaxQuota);
        if (!Set2DQuota(OOM_QUOTA_PATH, currentTime, oomDumpProcessMaxQuota)) {
            TAG_LOGE(AAFwkTag::APPKIT, "Failed to reset quota.");
            return false;
        }
    }
    TAG_LOGI(AAFwkTag::APPKIT, "Last oom time=%{public}" PRIu64 ", currentTime=%{public}" PRIu64 ", quota=%{public}d",
        g_lastOOMDumpTime, currentTime, g_oomDumpProcessQuota);
    return true;
}

bool DumpRuntimeHelper::GetQuota(const std::string &path, const std::string &property,
                                 std::vector<int64_t> &output, size_t size)
{
    std::string value;
    if (!GetDirXattr(path, property, value)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get xattr. preperty: %{public}s", property.c_str());
        return false;
    }
    size_t commaCount = static_cast<size_t>(std::count(value.begin(), value.end(), ','));
    if (commaCount != size - 1) {
        TAG_LOGE(AAFwkTag::APPKIT, "Invalid number of commas: %{public}zu", commaCount);
        return false;
    }

    std::vector<int64_t> rtn;
    std::stringstream ss(value);
    std::string segment;
    while (std::getline(ss, segment, ',')) {
        if (segment.empty()) {
            TAG_LOGE(AAFwkTag::APPKIT, "Invalid segment: empty value found, index: %{public}zu", rtn.size());
            return false;
        }
        long long tmp;
        if (!SafeStoll(segment, tmp)) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed to SafeStoll, segment: %{public}s", segment.c_str());
            return false;
        }
        rtn.push_back(static_cast<int64_t>(tmp));
    }

    if (rtn.size() != size) {
        TAG_LOGE(AAFwkTag::APPKIT, "Invalid number of entries: %{public}zu", rtn.size());
        return false;
    }

    output = rtn;
    return true;
}

bool DumpRuntimeHelper::Get2DQuota(const std::string &path, const std::string &property,
                                   uint64_t &timestamp, int &quota)
{
    std::string value;
    if (!GetDirXattr(path, property, value)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get xattr. path: %{public}s, preperty: %{public}s",
            path.c_str(), property.c_str());
        return false;
    }
    std::istringstream ss(value);
    char delimiter;
    if (!(ss >> timestamp >> delimiter >> quota) || delimiter != ',') {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to decode value: %{public}s", value.c_str());
        return false;
    }
    return true;
}

bool DumpRuntimeHelper::Set2DQuota(const std::string &path, uint64_t timestamp, int newQuota)
{
    std::ostringstream value;
    value << timestamp << "," << newQuota;
    if (newQuota < 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "Invalid quota.");
        return false;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "Set quota=%{public}d", newQuota);
    return SetDirXattr(path, OOM_QUOTA_XATTR_NAME, value.str());
}

uint64_t DumpRuntimeHelper::GetCurrentTimestamp()
{
    return static_cast<uint64_t>(time(nullptr));
}

bool DumpRuntimeHelper::CheckOOMFreeSpace(uint64_t maxSpace)
{
    struct statvfs st;
    if (statvfs("/data/storage/el2/base/", &st) != 0) {
        return false;
    }

    unsigned long freeSize = st.f_bsize * st.f_bfree;
    if (freeSize <= maxSpace) {
        return false;
    }
    return true;
}

uint64_t DumpRuntimeHelper::GetMaskFromDirXattr(const std::string &path)
{
    std::string value;
    if (!GetDirXattr(path, EVENT_XATTR_NAME, value)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get subscirbe xattr.");
        return 0;
    }
    return static_cast<uint64_t>(std::strtoull(value.c_str(), nullptr, 0));
}

bool DumpRuntimeHelper::CheckAppListenedEvents(const std::string &path)
{
    uint64_t eventsMask = GetMaskFromDirXattr(path);
    if (eventsMask == 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get events mask for path:%{public}s", path.c_str());
        return false;
    }
    if (!(eventsMask & (BIT_MASK << EVENT_RESOURCE_OVERLIMIT_MASK))) {
        TAG_LOGI(AAFwkTag::APPKIT, "Unlistened event for path:%{public}s", path.c_str());
        return false;
    }
    return true;
}

bool DumpRuntimeHelper::IsFileExists(const std::string &file)
{
    return access(file.c_str(), F_OK) == 0;
}

bool DumpRuntimeHelper::SetDirXattr(const std::string &path, const std::string &name, const std::string &value)
{
    return setxattr(path.c_str(), name.c_str(), value.c_str(), strlen(value.c_str()), 0) == 0;
}

bool DumpRuntimeHelper::GetDirXattr(const std::string &path, const std::string &name, std::string &value)
{
    char buf[BUF_SIZE_256] = {0};
    if (getxattr(path.c_str(), name.c_str(), buf, sizeof(buf) - 1) == -1) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed getxattr, name: %{public}s, err: %{public}d:%{public}s",
                 name.c_str(), errno, strerror(errno));
        return false;
    }
    value = buf;
    return true;
}

bool DumpRuntimeHelper::SafeStoll(const std::string &str, long long &value)
{
    value = 0;
    size_t start = 0;
    bool isNegative = false;

    if (str.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "str is empty");
        return false;
    }

    if (str[0] == '-') {
        isNegative = true;
        start = 1;
    } else if (str[0] == '+') {
        start = 1;
    }

    for (size_t i = start; i < str.size(); ++i) {
        if (!isdigit(str[i])) {
            TAG_LOGE(AAFwkTag::APPKIT, "digit check failed. str: %{public}s, index: %{public}zu", str.c_str(), i);
            return false;
        }
        if (value > (LLONG_MAX - (str[i] - '0')) / DECIMAL_BASE) {
            TAG_LOGE(AAFwkTag::APPKIT, "out of range, str: %{public}s", str.c_str());
            return false;
        }
        value = value * DECIMAL_BASE + (str[i] - '0');
    }

    if (isNegative) {
        value = -value;
    }

    TAG_LOGD(AAFwkTag::APPKIT, "success, str: %{public}s, result: %{public}lld", str.c_str(), value);
    return true;
}

int DumpRuntimeHelper::GetCompressQuota(const std::vector<int64_t> &quotas)
{
    int ret = 0;
    uint64_t now = static_cast<uint64_t>(time(nullptr));
    for (int i = INDEX_COMPRESSED_TS_0; i <= INDEX_COMPRESSED_TS_4; i++) {
        uint64_t compressTs = static_cast<uint64_t>(quotas[i]);
        if (now > compressTs && now - compressTs > OOM_DUMP_INTERVAL) {
            ret++;
        }
    }

    return ret;
}

bool DumpRuntimeHelper::SplitPropertyByComma(const std::string &property, std::string &runningId, std::string &value)
{
    size_t commaPos = property.find(',');
    if (commaPos == std::string::npos) {
        TAG_LOGE(AAFwkTag::APPKIT, "no comma in property");
        return false;
    }

    runningId = property.substr(0, commaPos);
    value = property.substr(commaPos + 1);
    return true;
}

std::string DumpRuntimeHelper::GetEventConfig(const std::string &key)
{
    std::string property;
    if (!GetDirXattr(OOM_QUOTA_PATH, key, property)) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to GetDirXattr, key: %{public}s", key.c_str());
        return "";
    }
    std::string customRunningId;
    std::string value;
    if (!SplitPropertyByComma(property, customRunningId, value)) {
        TAG_LOGW(AAFwkTag::APPKIT, "failed to SplitPropertyByComma, property: %{public}s", property.c_str());
        return "";
    }
    std::string curRunningId = DFX_GetAppRunningUniqueId();
    if (curRunningId.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "curRunningId is empty");
        return "";
    }
    if (customRunningId != curRunningId) {
        TAG_LOGW(AAFwkTag::APPKIT, "runningId not match, customRunningId: %{public}s, curRunningId: %{public}s",
            customRunningId.c_str(), curRunningId.c_str());
        return "";
    }

    TAG_LOGI(AAFwkTag::APPKIT, "succeed to getEventConfig, value: %{public}s", value.c_str());
    return value;
}

bool DumpRuntimeHelper::IsInFilterList()
{
    std::string value;
    if (GetDirXattr(OOM_QUOTA_PATH, TELEMETRY_ENABLE_PROPERTY, value) && value == "disable") {
        TAG_LOGI(AAFwkTag::APPKIT, "app is in filter list, xattr value: %{public}s", value.c_str());
        return true;
    }
    return false;
}

bool DumpRuntimeHelper::CheckRandomTelemetryQuota()
{
    // 1. Check if random telemetry is enabled (system property exists)
    std::string value = OHOS::system::GetParameter("persist.hiview.oomdump.random.telemetry", "");
    if (value.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "no random telemetry property");
        return false;
    }

    // 2. Check if current app is in filter list
    if (IsInFilterList()) {
        return false;
    }

    // 3. Parse property values
    std::vector<int64_t> quotaValues;
    std::stringstream ss(value);
    std::string segment;
    while (std::getline(ss, segment, ',')) {
        long long tmp;
        if (!SafeStoll(segment, tmp)) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed to SafeStoll segment: %{public}s", segment.c_str());
            return false;
        }
        quotaValues.push_back(static_cast<int64_t>(tmp));
    }

    if (quotaValues.size() != PROPERTY2C_SIZE) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid random telemetry property size: %zu", quotaValues.size());
        return false;
    }

    int compressQuota = GetCompressQuota(quotaValues);
    int appQuota = static_cast<int>(quotaValues[INDEX_APP_QUOTA]);
    int leftQuota = MIN(compressQuota, appQuota) - static_cast<int>(quotaValues[INDEX_HAS_SENT]);
    if (leftQuota <= 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "compress: %{public}d, app: %{public}d, sent: %{public}" PRId64,
                 compressQuota, appQuota, quotaValues[INDEX_HAS_SENT]);
        return false;
    }

    uint64_t now = GetCurrentTimestamp();
    if (quotaValues[INDEX_DELIVERY_TS] < 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid deliveryTs");
        return false;
    }
    uint64_t deliveryTs = static_cast<uint64_t>(quotaValues[INDEX_DELIVERY_TS]);
    if (now < deliveryTs || now - deliveryTs > OOM_DUMP_INTERVAL) {
        TAG_LOGE(AAFwkTag::APPKIT, "random telemetry deliveryTs expired, deliveryTs: %{public}" PRIu64, deliveryTs);
        return false;
    }

    uint64_t spaceQuota = static_cast<uint64_t>(quotaValues[INDEX_ROM_RSV_SIZE]) * KB_PER_MB;
    if (!CheckOOMFreeSpace(spaceQuota)) {
        TAG_LOGE(AAFwkTag::APPKIT, "random telemetry rom space insufficient, spaceQuota: %{public}" PRIu64, spaceQuota);
        return false;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "random telemetry check success");
    return true;
}

bool DumpRuntimeHelper::CheckDumpQuota(const std::string &eventConfig, bool needDecreaseQuota)
{
    // Step 1: 2custom — highest priority
    if (eventConfig == EVENT_RAWHEAP) {
        TAG_LOGI(AAFwkTag::APPKIT, "2custom matched, must dump.");
        return true;
    }

    // Step 2: DFX_ARKTS_SNAPSHOT env check
    char* snapshotEnv = getenv("DFX_ARKTS_SNAPSHOT");
    if (snapshotEnv != nullptr && (strcmp(snapshotEnv, "disable") == 0 || strcmp(snapshotEnv, "false") == 0)) {
        TAG_LOGI(AAFwkTag::APPKIT, "DFX_ARKTS_SNAPSHOT is %{public}s, no dump.", snapshotEnv);
        return false;
    }

    // Step 3: beta / developer mode
    if (g_betaVersion || g_developMode || !needDecreaseQuota) {
        TAG_LOGI(AAFwkTag::APPKIT, "beta or developMode, dump directly."
            " beta: %{public}d, develop: %{public}d, hidumper: %{public}d",
            g_betaVersion, g_developMode, !needDecreaseQuota);
        return true;
    }

    // Step 4: 2C / 2D / random telemetry
    if (Check2CQuota()) {
        TAG_LOGI(AAFwkTag::APPKIT, "2C quota check passed, will dump.");
        return true;
    }

    if (Check2DQuota(needDecreaseQuota)) {
        TAG_LOGI(AAFwkTag::APPKIT, "2D quota check passed, will dump.");
        return true;
    }

    if (CheckRandomTelemetryQuota()) {
        TAG_LOGI(AAFwkTag::APPKIT, "random telemetry check passed, will dump.");
        return true;
    }

    // Step 5: all checks failed
    TAG_LOGI(AAFwkTag::APPKIT, "all quota checks failed, no dump.");
    return false;
}
} // namespace AppExecFwk
} // namespace OHOS
