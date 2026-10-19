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

#ifndef OHOS_ABILITY_RUNTIME_DUMP_RUNTIME_HELPER_H
#define OHOS_ABILITY_RUNTIME_DUMP_RUNTIME_HELPER_H

#include "app_jsheap_mem_info.h"
#include "app_jshandle_map_info.h"
#include "app_cjheap_mem_info.h"
#include "app_mem_dump_info.h"
#include "mem_dump_callback_interface.h"
#include "napi_common_want.h"
#include "ohos_application.h"
#include "runtime.h"
#include <dlfcn.h>
#include <inttypes.h>
#include <memory>
#include <sys/types.h>

namespace OHOS {
namespace HiviewDFX {
class DfxMaps;
} // namespace HiviewDFX

namespace AppExecFwk {
using GetMemLeakStringFunc = bool (*)(int, char **, size_t, int *);

// Represents a leaked thread stack memory region parsed from /proc/self/smaps.
// Each entry corresponds to an [anon:stack:NNN] mapping whose tid no longer
// exists in /proc/self/task.
struct ThreadStackRegion {
    std::string mapName;   // full map name, e.g. "[anon:stack:12345]"
    uint64_t begin = 0;    // start address of the mapping
    uint64_t end = 0;      // end address of the mapping
    int32_t tid = 0;       // tid parsed from the map name
};

class DumpRuntimeHelper : public std::enable_shared_from_this<DumpRuntimeHelper> {
public:
    explicit DumpRuntimeHelper(const std::shared_ptr<OHOSApplication> &application);
    DumpRuntimeHelper(const std::shared_ptr<OHOSApplication> &application,
                      const std::shared_ptr<ApplicationInfo> &appInfo);
    ~DumpRuntimeHelper() = default;
    void SetAppFreezeFilterCallback();
    void DumpJsHeap(const OHOS::AppExecFwk::JsHeapDumpInfo &info);
    void DumpJsHandleMap(const OHOS::AppExecFwk::JsHandleMapInfo &info);
    void DumpCjHeap(const OHOS::AppExecFwk::CjHeapDumpInfo &info);
    void DumpMem(const OHOS::AppExecFwk::MemDumpInfo &info, sptr<IMemDumpCallback> callback);
private:
    std::shared_ptr<OHOSApplication> application_ = nullptr;
    std::shared_ptr<ApplicationInfo> appInfo_ = nullptr;

    static bool Check2DOOMDumpOpt();
    void GetCheckList(const std::unique_ptr<AbilityRuntime::Runtime> &runtime, std::string &checkList);
    void WriteCheckList(const std::string &checkList);
    void DumpNativeHeap(const OHOS::AppExecFwk::MemDumpInfo &info, std::string &dumpResult);
    napi_value GetJsLeakModule(napi_env env, napi_value global);
    napi_value GetMethodCheck(napi_env env, napi_value requireValue, napi_value global);
    // Asynchronously detect and unwind orphan thread stacks, then report the
    // result via `callback`. This is the THREAD_STACK_LEAK branch of DumpMem.
    // Threading contract (addresses ai_review concern about cross-thread risk):
    // - Detection runs on an ffrt worker thread, NOT the caller's thread.
    // - `callback` is invoked on that ffrt worker thread. Callers MUST pass a
    //   Binder proxy (IPC代理) so OnMemDumpDone marshals across the binder
    //   driver and is thread-safe; if a local in-process stub is passed, its
    //   OnMemDumpDone implementation must be thread-safe on its own.
    // - DumpMem returns immediately for this dump type (async branch); callers
    //   must NOT expect a synchronous dumpResult and must wait for the callback.
    static void SubmitThreadStackLeakTask(sptr<IMemDumpCallback> callback);
    // Report the thread stack leak result via AppMgrClient::ReportDumpMemResult.
    // Returns true on success (RESULT_OK), false on failure (client null,
    // non-OK return, or exception). Extracted from SubmitThreadStackLeakTask to
    // keep that method's nesting depth within the codeCheck limit (<=4).
    static bool ReportThreadStackLeakResult(const sptr<IMemDumpCallback> &callback,
        const std::string &dumpResult);
    static bool Init2DOOMDumpQuota(const std::string &path, uint32_t oomDumpProcessMaxQuota);
    static bool Check2DOOMDumpQuota(uint32_t oomDumpMaxQuota, uint32_t oomDumpProcessMaxQuota);
    static bool Get2DQuota(const std::string &path, const std::string &property, uint64_t &timestamp, int &quota);
    static bool GetQuota(const std::string &path, const std::string &property,
                         std::vector<int64_t> &output, size_t size);
    static bool Set2DQuota(const std::string &path, uint64_t timestamp, int newQuota);
    static uint64_t GetCurrentTimestamp();
    static bool CheckOOMFreeSpace(uint64_t maxSpace);
    static uint64_t GetMaskFromDirXattr(const std::string &path);
    static bool CheckAppListenedEvents(const std::string &path);
    static bool SetDirXattr(const std::string &path, const std::string &name, const std::string &value);
    static bool GetDirXattr(const std::string &path, const std::string &name, std::string &value);
    static bool IsFileExists(const std::string &file);
    static bool CreateDir(const std::string &path);
    static void CreateDirDelay(const std::string &path);
    static void WriteRunningId();
    static bool CheckOomdumpSwitch();
    static bool Check2CQuota();
    static bool Check2DQuota(bool needDecreaseQuota);
    static bool CheckRandomTelemetryQuota();
    static bool IsInFilterList();
    static bool CheckDumpQuota(const std::string &eventConfig, bool needDecreaseQuota);
    static bool SafeStoll(const std::string &str, long long &value);
    static GetMemLeakStringFunc LoadMemLeakFunc(void **handle);
    static bool GetDumpResult(std::string &dumpResult);
    static bool GetSnapshot(int fd);
    static std::string DetectAndUnwindThreadStackLeak();
    static std::string UnwindLeakedStacks(
        const std::vector<ThreadStackRegion> &regions,
        std::shared_ptr<OHOS::HiviewDFX::DfxMaps> &maps);
    // Extract tid from a thread stack map name like "[anon:stack:11111]".
    // Returns 0 if the tid cannot be parsed.
    static int32_t ParseTidFromMapName(const std::string &mapName);
    // Check whether the given tid is still alive in current process by looking
    // up /proc/self/task/<tid>. Returns true if the tid directory exists.
    static bool IsTidAlive(int32_t tid);
    // Count total alive threads by listing /proc/self/task entries.
    static size_t CountTotalThreads();
    // Parse /proc/self/smaps and collect all thread stack map regions whose
    // name starts with "[anon:stack:" and whose tid is no longer alive.
    static std::vector<ThreadStackRegion> ParseLeakedThreadStackRegions();
    // Scan a leaked thread stack region [begin, end) for candidate PC values
    // that fall inside executable maps. Memory is read in chunks via
    // process_vm_readv, which returns an error instead of SIGSEGV on
    // unmapped pages. The maps parameter is used to check whether each
    // candidate value falls in an executable mapping.
    // The maps parameter should be created once by the caller and reused.
    static std::vector<uintptr_t> CollectPcsFromStack(
        uint64_t begin, uint64_t end, const std::shared_ptr<OHOS::HiviewDFX::DfxMaps> &maps);
    // Format a leaked thread stack into a standardized string using
    // faultloggerd's Unwinder + DfxFrameFormatter. Returns empty string if
    // no frames could be resolved.
    static std::string FormatLeakedStack(int32_t tid, const std::vector<uintptr_t> &pcs);
    static int GetCompressQuota (const std::vector<int64_t> &quotas);
    static bool SplitPropertyByComma(const std::string &property, std::string &runningId, std::string &value);
    static std::string GetEventConfig(const std::string &key);
    void DumpJsHeapGc(const std::unique_ptr<AbilityRuntime::Runtime> &runtime,
        const OHOS::AppExecFwk::JsHeapDumpInfo &info);
    void DumpKmpKotlinHeap(const OHOS::AppExecFwk::MemDumpInfo &info);
    void DumpJsvmHeap(const OHOS::AppExecFwk::MemDumpInfo &info);
    void DumpArkwebJsHeap(const OHOS::AppExecFwk::MemDumpInfo &info);
    static constexpr int32_t THREAD_NAME_MAX_LEN = 16;
    void DumpArktsHeapSize(const OHOS::AppExecFwk::MemDumpInfo &info, std::string &dumpResult);
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_DUMP_RUNTIME_HELPER_H
