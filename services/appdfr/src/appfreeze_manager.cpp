/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "appfreeze_manager.h"

#include <fcntl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <fstream>

#include "backtrace_local.h"
#include "faultloggerd_client.h"
#include "file_ex.h"
#include "ffrt.h"
#include "dfx_dump_catcher.h"
#include "directory_ex.h"
#include "hisysevent.h"
#include "hitrace_meter.h"
#include "parameter.h"
#include "parameters.h"
#include "singleton.h"
#include "res_sched_util.h"
#include "app_mgr_client.h"
#include "hilog_tag_wrapper.h"
#include "time_util.h"
#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
#include "hitrace/hitracechain.h"
#endif
#include "appfreeze_cpu_freq_manager.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr char EVENT_UID[] = "UID";
constexpr char EVENT_PID[] = "PID";
constexpr char EVENT_TID[] = "TID";
constexpr char EVENT_INPUT_ID[] = "INPUT_ID";
constexpr char EVENT_MESSAGE[] = "MSG";
constexpr char EVENT_PACKAGE_NAME[] = "PACKAGE_NAME";
constexpr char EVENT_PROCESS_NAME[] = "PROCESS_NAME";
constexpr char EVENT_STACK[] = "STACK";
constexpr char BINDER_INFO[] = "BINDER_INFO";
constexpr char APP_RUNNING_UNIQUE_ID[] = "APP_RUNNING_UNIQUE_ID";
constexpr char FREEZE_MEMORY[] = "FREEZE_MEMORY";
constexpr char FREEZE_INFO_PATH[] = "FREEZE_INFO_PATH";
#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
constexpr char EVENT_TRACE_ID[] = "HITRACE_ID";
constexpr char EVENT_SPAN_ID[] = "SPAN_ID";
constexpr char EVENT_PARENT_SPAN_ID[] = "PARENT_SPAN_ID";
constexpr char EVENT_TRACE_FLAG[] = "TRACE_FLAG";
constexpr int32_t CHARACTER_WIDTH = 2;
#endif

constexpr int MAX_LAYER = 8;
constexpr int FREEZEMAP_SIZE_MAX = 20;
constexpr int FREEZE_TIME_LIMIT = 60000;
static constexpr uint8_t ARR_SIZE = 7;
static constexpr uint8_t DECIMAL = 10;
static constexpr uint8_t FREE_ASYNC_INDEX = 6;
static constexpr uint16_t FREE_ASYNC_MAX = 1000;
static constexpr int64_t NANOSECONDS = 1000000000;  // NANOSECONDS mean 10^9 nano second
static constexpr int64_t MICROSECONDS = 1000000;    // MICROSECONDS mean 10^6 millias second
static constexpr int DUMP_STACK_FAILED = -1;
static constexpr int DUMP_KERNEL_STACK_SUCCESS = 1;
constexpr uint64_t SEC_TO_MILLISEC = 1000;
constexpr uint32_t BUFFER_SIZE = 1024;
const std::string LOG_FILE_PATH = "data/log/eventlog";
static bool g_betaVersion = OHOS::system::GetParameter("const.logsystem.versiontype", "unknown") == "beta";
static bool g_developMode = (OHOS::system::GetParameter("persist.hiview.leak_detector", "unknown") == "enable") ||
                            (OHOS::system::GetParameter("persist.hiview.leak_detector", "unknown") == "true");
static constexpr const char *const APPFREEZE_LOG_PREFIX = "/data/app/el2/100/log/";
static constexpr const char *const APPFREEZE_LOG_SUFFIX = "/watchdog/freeze/";
}
static constexpr const char *const TWELVE_BIG_CPU_CUR_FREQ = "/sys/devices/system/cpu/cpufreq/policy2/scaling_cur_freq";
static constexpr const char *const TWELVE_BIG_CPU_MAX_FREQ = "/sys/devices/system/cpu/cpufreq/policy2/scaling_max_freq";
static constexpr const char *const TWELVE_MID_CPU_CUR_FREQ = "/sys/devices/system/cpu/cpufreq/policy1/scaling_cur_freq";
static constexpr const char *const TWELVE_MID_CPU_MAX_FREQ = "/sys/devices/system/cpu/cpufreq/policy1/scaling_max_freq";
const static std::set<std::string> HALF_EVENT_CONFIGS = {"UI_BLOCK_3S", "THREAD_BLOCK_3S", "BUSSNESS_THREAD_BLOCK_3S",
                                                         "LIFECYCLE_HALF_TIMEOUT", "LIFECYCLE_HALF_TIMEOUT_WARNING"};
static constexpr int PERF_TIME = 60000;
std::shared_ptr<AppfreezeManager> AppfreezeManager::instance_ = nullptr;
ffrt::mutex AppfreezeManager::singletonMutex_;
ffrt::mutex AppfreezeManager::freezeMutex_;
ffrt::mutex AppfreezeManager::catchStackMutex_;
std::map<int, std::string> AppfreezeManager::catchStackMap_;
ffrt::mutex AppfreezeManager::freezeFilterMutex_;
ffrt::mutex AppfreezeManager::freezeInfoMutex_;
std::string AppfreezeManager::appfreezeInfoPath_;

AppfreezeManager::AppfreezeManager()
{
    name_ = "AppfreezeManager" + std::to_string(AbilityRuntime::TimeUtil::CurrentTimeMillis());
}

AppfreezeManager::~AppfreezeManager()
{
}

std::shared_ptr<AppfreezeManager> AppfreezeManager::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<ffrt::mutex> lock(singletonMutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<AppfreezeManager>();
        }
    }
    return instance_;
}

void AppfreezeManager::DestroyInstance()
{
    std::lock_guard<ffrt::mutex> lock(singletonMutex_);
    if (instance_ != nullptr) {
        instance_.reset();
        instance_ = nullptr;
    }
}

bool AppfreezeManager::IsHandleAppfreeze(const std::string& bundleName)
{
    if (bundleName.empty()) {
        return true;
    }
    return !DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->IsAttachDebug(bundleName);
}

int AppfreezeManager::AppfreezeHandle(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo)
{
    TAG_LOGD(AAFwkTag::APPDFR, "called %{public}s, bundleName %{public}s, name_ %{public}s",
        faultData.errorObject.name.c_str(), appInfo.bundleName.c_str(), name_.c_str());
    if (!IsHandleAppfreeze(appInfo.bundleName)) {
        return -1;
    }
    HITRACE_METER_FMT(HITRACE_TAG_APP, "AppfreezeHandler:%{public}s bundleName:%{public}s",
        faultData.errorObject.name.c_str(), appInfo.bundleName.c_str());
    std::string memoryContent = "";
    CollectFreezeSysMemory(memoryContent);
    if (faultData.errorObject.name == AppFreezeType::APP_INPUT_BLOCK ||
        faultData.errorObject.name == AppFreezeType::THREAD_BLOCK_3S ||
        faultData.errorObject.name == AppFreezeType::LIFECYCLE_HALF_TIMEOUT ||
        faultData.errorObject.name == AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING) {
        AcquireStack(faultData, appInfo, memoryContent);
    } else {
        NotifyANR(faultData, appInfo, "", memoryContent);
    }
    return 0;
}

void AppfreezeManager::CollectFreezeSysMemory(std::string& memoryContent)
{
    memoryContent = "\nGet freeze memory start time: " + AbilityRuntime::TimeUtil::DefaultCurrentTimeStr() + "\n";
    std::string tmp = "";
    std::string pressMemInfo = "/proc/pressure/memory";
    OHOS::LoadStringFromFile(pressMemInfo, tmp);
    memoryContent += tmp + "\n";
    std::string memInfoPath = "/proc/memview";
    if (!OHOS::FileExists(memInfoPath)) {
        memInfoPath = "/proc/meminfo";
    }
    OHOS::LoadStringFromFile(memInfoPath, tmp);
    memoryContent += tmp + "\nGet freeze memory end time: " + AbilityRuntime::TimeUtil::DefaultCurrentTimeStr();
}

int AppfreezeManager::MergeNotifyInfo(FaultData& faultNotifyData, const AppfreezeManager::AppInfo& appInfo)
{
    std::string memoryContent = "";
    CollectFreezeSysMemory(memoryContent);
    std::string fileName = faultNotifyData.errorObject.name + "_" +
        AbilityRuntime::TimeUtil::FormatTime("%Y%m%d%H%M%S") + "_" + std::to_string(appInfo.pid) + "_stack";
    std::string catcherStack = "";
    faultNotifyData.errorObject.message += "\nCatche stack trace start time: " +
        AbilityRuntime::TimeUtil::DefaultCurrentTimeStr() + "\n";
    if (faultNotifyData.errorObject.name == AppFreezeType::LIFECYCLE_HALF_TIMEOUT ||
        faultNotifyData.errorObject.name == AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING ||
        faultNotifyData.errorObject.name == AppFreezeType::LIFECYCLE_TIMEOUT ||
        faultNotifyData.errorObject.name == AppFreezeType::LIFECYCLE_TIMEOUT_WARNING) {
        catcherStack += CatcherStacktrace(appInfo.pid, faultNotifyData.errorObject.stack);
    } else {
        catcherStack += CatchJsonStacktrace(appInfo.pid, faultNotifyData.errorObject.name,
            faultNotifyData.errorObject.stack);
    }
    std::string timeStamp = "Catche stack trace end time: " + AbilityRuntime::TimeUtil::DefaultCurrentTimeStr();
    faultNotifyData.errorObject.stack = WriteToFile(fileName, catcherStack);
    if (appInfo.isOccurException) {
        faultNotifyData.errorObject.message += "\nnotifyAppFault exception.\n";
    }
    faultNotifyData.errorObject.message += timeStamp;
    if (faultNotifyData.errorObject.name == AppFreezeType::APP_INPUT_BLOCK ||
        faultNotifyData.errorObject.name == AppFreezeType::THREAD_BLOCK_3S ||
        faultNotifyData.errorObject.name == AppFreezeType::LIFECYCLE_HALF_TIMEOUT ||
        faultNotifyData.errorObject.name == AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING) {
        AcquireStack(faultNotifyData, appInfo, memoryContent);
    } else {
        NotifyANR(faultNotifyData, appInfo, "", memoryContent);
    }
    return 0;
}

int AppfreezeManager::AppfreezeHandleWithStack(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo)
{
    TAG_LOGW(AAFwkTag::APPDFR, "NotifyAppFaultTask called, eventName:%{public}s, bundleName:%{public}s, "
        "name_:%{public}s, currentTime:%{public}s", faultData.errorObject.name.c_str(), appInfo.bundleName.c_str(),
        name_.c_str(), AbilityRuntime::TimeUtil::DefaultCurrentTimeStr().c_str());
    if (!IsHandleAppfreeze(appInfo.bundleName)) {
        return -1;
    }
    FaultData faultNotifyData;
    faultNotifyData.errorObject.name = faultData.errorObject.name;
    faultNotifyData.errorObject.message = faultData.errorObject.message;
    faultNotifyData.errorObject.stack = faultData.errorObject.stack;
    faultNotifyData.faultType = FaultDataType::APP_FREEZE;
    faultNotifyData.eventId = faultData.eventId;
    faultNotifyData.tid = faultData.tid;
    faultNotifyData.appfreezeInfo = faultData.appfreezeInfo;

    HITRACE_METER_FMT(HITRACE_TAG_APP, "AppfreezeHandleWithStack pid:%d-name:%s",
        appInfo.pid, faultData.errorObject.name.c_str());
    return MergeNotifyInfo(faultNotifyData, appInfo);
}

std::string AppfreezeManager::WriteToFile(const std::string& fileName, std::string& content)
{
    std::string dir_path = LOG_FILE_PATH + "/freeze";
    constexpr mode_t defaultLogDirMode = 0770;
    if (!OHOS::FileExists(dir_path)) {
        OHOS::ForceCreateDirectory(dir_path);
        OHOS::ChangeModeDirectory(dir_path, defaultLogDirMode);
    }
    std::string realPath;
    if (!OHOS::PathToRealPath(dir_path, realPath)) {
        TAG_LOGE(AAFwkTag::APPDFR, "pathToRealPath failed:%{public}s", dir_path.c_str());
        return "";
    }
    std::string stackPath = realPath + "/" + fileName;
    constexpr mode_t defaultLogFileMode = 0644;
    FILE* fp = fopen(stackPath.c_str(), "w+");
    chmod(stackPath.c_str(), defaultLogFileMode);
    if (fp == nullptr) {
        TAG_LOGI(AAFwkTag::APPDFR, "stackPath create failed, errno: %{public}d", errno);
        return "";
    } else {
        TAG_LOGI(AAFwkTag::APPDFR, "stackPath: %{public}s", stackPath.c_str());
    }
    OHOS::SaveStringToFile(stackPath, content, true);
    (void)fclose(fp);
    return stackPath;
}

int AppfreezeManager::LifecycleTimeoutHandle(const ParamInfo& info, FreezeUtil::LifecycleFlow flow)
{
    if (info.typeId != AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT) {
        return -1;
    }
    if (!IsHandleAppfreeze(info.bundleName)) {
        return -1;
    }
    if (info.eventName != AppFreezeType::LIFECYCLE_TIMEOUT &&
        info.eventName != AppFreezeType::LIFECYCLE_TIMEOUT_WARNING &&
        info.eventName != AppFreezeType::LIFECYCLE_HALF_TIMEOUT &&
        info.eventName != AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING) {
        return -1;
    }
    TAG_LOGD(AAFwkTag::APPDFR, "called %{public}s, name_ %{public}s", info.bundleName.c_str(),
        name_.c_str());
    HITRACE_METER_FMT(HITRACE_TAG_APP, "LifecycleTimeoutHandle:%{public}s bundleName:%{public}s",
        info.eventName.c_str(), info.bundleName.c_str());
    AppFaultDataBySA faultDataSA;
    faultDataSA.errorObject.name = info.eventName;
    faultDataSA.errorObject.message = info.msg;
    faultDataSA.errorObject.stack = "\nDump tid stack start time:" +
        AbilityRuntime::TimeUtil::DefaultCurrentTimeStr() + "\n";
    std::string stack = "";
    if (!HiviewDFX::GetBacktraceStringByTidWithMix(stack, info.pid, 0, true)) {
        stack = "Failed to dump stacktrace for " + stack;
    }
    faultDataSA.errorObject.stack += stack + "\nDump tid stack end time:" +
        AbilityRuntime::TimeUtil::DefaultCurrentTimeStr() + "\n";
    faultDataSA.faultType = FaultDataType::APP_FREEZE;
    faultDataSA.timeoutMarkers = "notifyFault" +
                                 std::to_string(info.pid) +
                                 "-" + std::to_string(AbilityRuntime::TimeUtil::CurrentTimeMillis());
    faultDataSA.pid = info.pid;
    faultDataSA.needKillProcess = info.needKillProcess;
    if (flow.state != AbilityRuntime::FreezeUtil::TimeoutState::UNKNOWN) {
        faultDataSA.token = flow.token;
        faultDataSA.state = static_cast<uint32_t>(flow.state);
    }
    DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->NotifyAppFaultBySA(faultDataSA);
    return 0;
}

FaultData AppfreezeManager::GetFaultNotifyData(const FaultData& faultData, int pid)
{
    FaultData faultNotifyData;
    faultNotifyData.errorObject.name = faultData.errorObject.name;
    faultNotifyData.errorObject.message = faultData.errorObject.message;
    faultNotifyData.errorObject.stack = faultData.errorObject.stack;
    faultNotifyData.faultType = FaultDataType::APP_FREEZE;
    faultNotifyData.eventId = faultData.eventId;
    faultNotifyData.tid = (faultData.errorObject.name == AppFreezeType::APP_INPUT_BLOCK) ? pid : faultData.tid;
    faultNotifyData.appfreezeInfo = faultData.appfreezeInfo;
    return faultNotifyData;
}

int AppfreezeManager::AcquireStack(const FaultData& faultData,
    const AppfreezeManager::AppInfo& appInfo, const std::string& memoryContent)
{
    int pid = appInfo.pid;
    FaultData faultNotifyData = GetFaultNotifyData(faultData, pid);

    std::string binderInfo;
    std::string binderPidsStr;
    std::string terminalBinderTid;
    AppfreezeManager::TerminalBinder terminalBinder = {0, 0};
    AppfreezeManager::ParseBinderParam params = {pid, faultNotifyData.tid, pid, 0};
    std::set<int> asyncPids;
    std::set<int> syncPids = GetBinderPeerPids(binderInfo, params, asyncPids, terminalBinder);
    if (syncPids.empty()) {
        binderInfo +="PeerBinder pids is empty\n";
    }
    for (auto& pidTemp : syncPids) {
        TAG_LOGI(AAFwkTag::APPDFR, "PeerBinder pidTemp pids:%{public}d", pidTemp);
        if (pidTemp == pid) {
            continue;
        }
        std::string content = "Binder catcher stacktrace, type is peer, pid : " + std::to_string(pidTemp) + "\n";
        content += CatcherStacktrace(pidTemp, "");
        binderPidsStr += " " + std::to_string(pidTemp);
        if (terminalBinder.pid > 0 && pidTemp == terminalBinder.pid) {
            terminalBinder.tid  = (terminalBinder.tid > 0) ? terminalBinder.tid : terminalBinder.pid;
            content = "Binder catcher stacktrace, terminal binder tag\n" + content +
                "Binder catcher stacktrace, terminal binder tag\n";
            terminalBinderTid = std::to_string(terminalBinder.tid);
        }
        binderInfo += content;
    }
    for (auto& pidTemp : asyncPids) {
        TAG_LOGI(AAFwkTag::APPDFR, "AsyncBinder pidTemp pids:%{public}d", pidTemp);
        if (pidTemp != pid && syncPids.find(pidTemp) == syncPids.end()) {
            std::string content = "Binder catcher stacktrace, type is async, pid : " + std::to_string(pidTemp) + "\n";
            content += CatcherStacktrace(pidTemp, "");
            binderInfo += content;
        }
    }

    std::string fileName = faultData.errorObject.name + "_" +
        AbilityRuntime::TimeUtil::FormatTime("%Y%m%d%H%M%S") + "_" + std::to_string(pid) + "_binder";
    std::string fullStackPath = WriteToFile(fileName, binderInfo);
    binderInfo = fullStackPath + "," + binderPidsStr + "," + terminalBinderTid;

    int ret = NotifyANR(faultNotifyData, appInfo, binderInfo, memoryContent);
    return ret;
}

std::string AppfreezeManager::ParseDecToHex(uint64_t id)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(CHARACTER_WIDTH) << id;
    return ss.str();
}

bool AppfreezeManager::GetHitraceId(HitraceInfo& info)
{
#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
    OHOS::HiviewDFX::HiTraceId hitraceId = OHOS::HiviewDFX::HiTraceChain::GetId();
    if (hitraceId.IsValid() == 0) {
        TAG_LOGW(AAFwkTag::APPDFR, "get hitrace id is invalid.");
        return false;
    }
    info.hiTraceChainId = ParseDecToHex(hitraceId.GetChainId());
    info.spanId = ParseDecToHex(hitraceId.GetSpanId());
    info.pspanId = ParseDecToHex(hitraceId.GetParentSpanId());
    info.traceFlag = ParseDecToHex(hitraceId.GetFlags());
    TAG_LOGW(AAFwkTag::APPDFR,
        "hiTraceChainId:%{public}s, spanId:%{public}s, pspanId:%{public}s, traceFlag:%{public}s",
        info.hiTraceChainId.c_str(), info.spanId.c_str(), info.pspanId.c_str(), info.traceFlag.c_str());
    return true;
#endif
    return false;
}

std::string AppfreezeManager::GetFreezeInfoFile(const FaultData& faultData,
    const std::string& bundleName)
{
    std::lock_guard<ffrt::mutex> lock(freezeInfoMutex_);
    std::string filePath;
    if (faultData.errorObject.name == AppFreezeType::THREAD_BLOCK_3S) {
        filePath = !faultData.appfreezeInfo.empty() ? (APPFREEZE_LOG_PREFIX + bundleName +
            APPFREEZE_LOG_SUFFIX + faultData.appfreezeInfo) : "";
    }
    TAG_LOGI(AAFwkTag::APPDFR, "appfreezeInfo:%{public}s, filePath:%{public}s",
        faultData.appfreezeInfo.c_str(), filePath.c_str());
    return filePath;
}

std::string AppfreezeManager::ReportAppfreezeCpuInfo(const FaultData& faultData,
    const AppfreezeManager::AppInfo& appInfo)
{
    std::string filePath;
    if (faultData.errorObject.name == AppFreezeType::THREAD_BLOCK_3S) {
        AppExecFwk::AppfreezeCpuFreqManager::GetInstance()->SetHalfStackPath(GetFreezeInfoFile(faultData,
            appInfo.bundleName));
        AppExecFwk::AppfreezeCpuFreqManager::GetInstance()->InitHalfCpuInfo(appInfo.pid);
    } else if (faultData.errorObject.name == AppFreezeType::LIFECYCLE_HALF_TIMEOUT) {
        AppExecFwk::AppfreezeCpuFreqManager::GetInstance()->InitHalfCpuInfo(appInfo.pid);
    } else if (faultData.errorObject.name == AppFreezeType::THREAD_BLOCK_6S ||
        faultData.errorObject.name == AppFreezeType::LIFECYCLE_TIMEOUT) {
        filePath = AppExecFwk::AppfreezeCpuFreqManager::GetInstance()->WriteCpuInfoToFile(appInfo.bundleName,
            appInfo.uid, appInfo.pid);
    }
    TAG_LOGI(AAFwkTag::APPDFR, "appfreezeInfo:%{public}s, filePath:%{public}s",
        faultData.appfreezeInfo.c_str(), filePath.c_str());
    return filePath;
}

int AppfreezeManager::NotifyANR(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo,
    const std::string& binderInfo, const std::string& memoryContent)
{
    std::string appRunningUniqueId = "";
    DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->GetAppRunningUniqueIdByPid(appInfo.pid,
        appRunningUniqueId);
    int ret = 0;
    this->PerfStart(faultData.errorObject.name);
    int64_t startTime = AbilityRuntime::TimeUtil::CurrentTimeMillis();
    if (faultData.errorObject.name == AppFreezeType::APP_INPUT_BLOCK) {
        ret = HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::AAFWK, faultData.errorObject.name,
            OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, EVENT_UID, appInfo.uid, EVENT_PID, appInfo.pid,
            EVENT_PACKAGE_NAME, appInfo.bundleName, EVENT_PROCESS_NAME, appInfo.processName, EVENT_MESSAGE,
            faultData.errorObject.message, EVENT_STACK, faultData.errorObject.stack, BINDER_INFO, binderInfo,
            APP_RUNNING_UNIQUE_ID, appRunningUniqueId, EVENT_INPUT_ID, faultData.eventId,
            FREEZE_MEMORY, memoryContent);
    } else if (faultData.errorObject.name == AppFreezeType::THREAD_BLOCK_6S) {
        HitraceInfo info;
        bool hitraceIsValid = GetHitraceId(info);
        ret = HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::AAFWK, faultData.errorObject.name,
            OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, EVENT_UID, appInfo.uid, EVENT_PID, appInfo.pid,
            EVENT_TID, faultData.tid,
            EVENT_PACKAGE_NAME, appInfo.bundleName, EVENT_PROCESS_NAME, appInfo.processName, EVENT_MESSAGE,
            faultData.errorObject.message, EVENT_STACK, faultData.errorObject.stack, BINDER_INFO, binderInfo,
            APP_RUNNING_UNIQUE_ID, appRunningUniqueId, FREEZE_MEMORY, memoryContent,
            EVENT_TRACE_ID, hitraceIsValid ? info.hiTraceChainId : "",
            EVENT_SPAN_ID, hitraceIsValid ? info.spanId : "",
            EVENT_PARENT_SPAN_ID, hitraceIsValid ? info.pspanId : "",
            EVENT_TRACE_FLAG, hitraceIsValid ? info.traceFlag : "",
            FREEZE_INFO_PATH, ReportAppfreezeCpuInfo(faultData, appInfo));
    } else {
        ret = HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::AAFWK, faultData.errorObject.name,
            OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, EVENT_UID, appInfo.uid, EVENT_PID, appInfo.pid,
            EVENT_TID, faultData.tid > 0 ? faultData.tid : appInfo.pid,
            EVENT_PACKAGE_NAME, appInfo.bundleName, EVENT_PROCESS_NAME, appInfo.processName, EVENT_MESSAGE,
            faultData.errorObject.message, EVENT_STACK, faultData.errorObject.stack, BINDER_INFO, binderInfo,
            APP_RUNNING_UNIQUE_ID, appRunningUniqueId, FREEZE_MEMORY, memoryContent,
            FREEZE_INFO_PATH, ReportAppfreezeCpuInfo(faultData, appInfo));
    }
    TAG_LOGW(AAFwkTag::APPDFR,
        "reportEvent:%{public}s, pid:%{public}d, tid:%{public}d, bundleName:%{public}s, appRunningUniqueId:%{public}s"
        ", endTime:%{public}s, interval:%{public}" PRId64 " ms, eventId:%{public}d hisysevent write ret: %{public}d",
        faultData.errorObject.name.c_str(), appInfo.pid, faultData.tid, appInfo.bundleName.c_str(),
        appRunningUniqueId.c_str(), AbilityRuntime::TimeUtil::DefaultCurrentTimeStr().c_str(),
        AbilityRuntime::TimeUtil::CurrentTimeMillis() - startTime, faultData.eventId, ret);
#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
    OHOS::HiviewDFX::HiTraceChain::ClearId();
#endif
    return 0;
}

std::map<int, std::list<AppfreezeManager::PeerBinderInfo>> AppfreezeManager::BinderParser(std::ifstream& fin,
    std::string& stack, std::set<int>& asyncPids) const
{
    std::map<uint32_t, uint32_t> asyncBinderMap;
    std::vector<std::pair<uint32_t, uint64_t>> freeAsyncSpacePairs;
    std::map<int, std::list<AppfreezeManager::PeerBinderInfo>> binderInfos = BinderLineParser(fin, stack,
        asyncBinderMap, freeAsyncSpacePairs);

    std::sort(freeAsyncSpacePairs.begin(), freeAsyncSpacePairs.end(),
        [] (const auto& pairOne, const auto& pairTwo) { return pairOne.second < pairTwo.second; });
    std::vector<std::pair<uint32_t, uint32_t>> asyncBinderPairs(asyncBinderMap.begin(), asyncBinderMap.end());
    std::sort(asyncBinderPairs.begin(), asyncBinderPairs.end(),
        [] (const auto& pairOne, const auto& pairTwo) { return pairOne.second > pairTwo.second; });

    size_t freeAsyncSpaceSize = freeAsyncSpacePairs.size();
    size_t asyncBinderSize = asyncBinderPairs.size();
    size_t individualMaxSize = 2;
    for (size_t i = 0; i < individualMaxSize; i++) {
        if (i < freeAsyncSpaceSize) {
            asyncPids.insert(freeAsyncSpacePairs[i].first);
        }
        if (i < asyncBinderSize) {
            asyncPids.insert(asyncBinderPairs[i].first);
        }
    }

    return binderInfos;
}

std::map<int, std::list<AppfreezeManager::PeerBinderInfo>> AppfreezeManager::BinderLineParser(std::ifstream& fin,
    std::string& stack, std::map<uint32_t, uint32_t>& asyncBinderMap,
    std::vector<std::pair<uint32_t, uint64_t>>& freeAsyncSpacePairs) const
{
    std::map<int, std::list<AppfreezeManager::PeerBinderInfo>> binderInfos;
    std::string line;
    bool isBinderMatchup = false;
    TAG_LOGI(AAFwkTag::APPDFR, "start");
    stack += "BinderCatcher --\n\n";
    while (getline(fin, line)) {
        stack += line + "\n";
        isBinderMatchup = (!isBinderMatchup && line.find("free_async_space") != line.npos) ? true : isBinderMatchup;
        std::vector<std::string> strList = GetFileToList(line);

        auto strSplit = [](const std::string& str, uint16_t index) -> std::string {
            std::vector<std::string> strings;
            SplitStr(str, ":", strings);
            return index < strings.size() ? strings[index] : "";
        };

        if (isBinderMatchup) {
            if (line.find("free_async_space") == line.npos && strList.size() == ARR_SIZE &&
                std::atoll(strList[FREE_ASYNC_INDEX].c_str()) < FREE_ASYNC_MAX) {
                freeAsyncSpacePairs.emplace_back(
                    std::atoi(strList[0].c_str()),
                    std::atoll(strList[FREE_ASYNC_INDEX].c_str()));
            }
        } else if (line.find("async\t") != std::string::npos && strList.size() > ARR_SIZE) {
            std::string serverPid = strSplit(strList[3], 0);
            std::string serverTid = strSplit(strList[3], 1);
            if (serverPid != "" && serverTid != "" && std::atoi(serverTid.c_str()) == 0) {
                asyncBinderMap[std::atoi(serverPid.c_str())]++;
            }
        } else if (strList.size() >= ARR_SIZE) { // 7: valid array size
            AppfreezeManager::PeerBinderInfo info = {0};
            // 0: local id,
            std::string clientPid = strSplit(strList[0], 0);
            std::string clientTid = strSplit(strList[0], 1);
            // 2: peer id,
            std::string serverPid = strSplit(strList[2], 0);
            std::string serverTid = strSplit(strList[2], 1);
             // 5: wait time, s
            std::string wait = strSplit(strList[5], 1);
            if (clientPid == "" || clientTid == "" || serverPid == "" || serverTid == "" || wait == "") {
                continue;
            }
            info = {std::strtol(clientPid.c_str(), nullptr, DECIMAL), std::strtol(clientTid.c_str(), nullptr, DECIMAL),
                    std::strtol(serverPid.c_str(), nullptr, DECIMAL), strtol(serverTid.c_str(), nullptr, DECIMAL)};
            int waitTime = std::strtol(wait.c_str(), nullptr, DECIMAL);
            TAG_LOGI(AAFwkTag::APPDFR, "server:%{public}d, client:%{public}d, wait:%{public}d", info.serverPid,
                info.clientPid, waitTime);
            binderInfos[info.clientPid].push_back(info);
        }
    }
    TAG_LOGI(AAFwkTag::APPDFR, "binderInfos size: %{public}zu", binderInfos.size());
    return binderInfos;
}

std::vector<std::string> AppfreezeManager::GetFileToList(std::string line) const
{
    std::vector<std::string> strList;
    std::istringstream lineStream(line);
    std::string tmpstr;
    while (lineStream >> tmpstr) {
        strList.push_back(tmpstr);
    }
    TAG_LOGD(AAFwkTag::APPDFR, "strList size: %{public}zu", strList.size());
    return strList;
}

std::set<int> AppfreezeManager::GetBinderPeerPids(std::string& stack, AppfreezeManager::ParseBinderParam params,
    std::set<int>& asyncPids, AppfreezeManager::TerminalBinder& terminalBinder) const
{
    std::set<int> pids;
    std::ifstream fin;
    std::string path = LOGGER_DEBUG_PROC_PATH;
    char resolvePath[PATH_MAX] = {0};
    if (realpath(path.c_str(), resolvePath) == nullptr) {
        TAG_LOGE(AAFwkTag::APPDFR, "invalid realpath");
        return pids;
    }
    fin.open(resolvePath);
    if (!fin.is_open()) {
        TAG_LOGE(AAFwkTag::APPDFR, "open failed, %{public}s", resolvePath);
        stack += "open file failed :" + path + "\r\n";
        return pids;
    }

    stack += "\n\nPeerBinderCatcher -- pid==" + std::to_string(params.pid) + "\n\n";
    std::map<int, std::list<AppfreezeManager::PeerBinderInfo>> binderInfos = BinderParser(fin, stack, asyncPids);
    fin.close();

    if (binderInfos.size() == 0 || binderInfos.find(params.pid) == binderInfos.end()) {
        return pids;
    }

    ParseBinderPids(binderInfos, pids, params, true, terminalBinder);
    for (auto& each : pids) {
        TAG_LOGD(AAFwkTag::APPDFR, "each pids:%{public}d", each);
    }
    return pids;
}

void AppfreezeManager::ParseBinderPids(const std::map<int, std::list<AppfreezeManager::PeerBinderInfo>>& binderInfos,
    std::set<int>& pids, AppfreezeManager::ParseBinderParam params, bool getTerminal,
    AppfreezeManager::TerminalBinder& terminalBinder) const
{
    auto it = binderInfos.find(params.pid);
    params.layer++;
    if (params.layer >= MAX_LAYER || it == binderInfos.end()) {
        return;
    }

    for (auto& each : it->second) {
        pids.insert(each.serverPid);
        params.pid = each.serverPid;
        if (getTerminal && ((each.clientPid == params.eventPid && each.clientTid == params.eventTid) ||
            (each.clientPid == terminalBinder.pid && each.clientTid == terminalBinder.tid))) {
            terminalBinder.pid = each.serverPid;
            terminalBinder.tid = each.serverTid;
            ParseBinderPids(binderInfos, pids, params, true, terminalBinder);
        } else {
            ParseBinderPids(binderInfos, pids, params, false, terminalBinder);
        }
    }
}

void AppfreezeManager::DeleteStack(int pid)
{
    std::lock_guard<ffrt::mutex> lock(catchStackMutex_);
    auto it = catchStackMap_.find(pid);
    if (it != catchStackMap_.end()) {
        catchStackMap_.erase(it);
    }
}

void AppfreezeManager::FindStackByPid(std::string& msg, int pid) const
{
    std::lock_guard<ffrt::mutex> lock(catchStackMutex_);
    auto it = catchStackMap_.find(pid);
    if (it != catchStackMap_.end()) {
        msg = it->second;
    }
}

std::string AppfreezeManager::CatchJsonStacktrace(int pid, const std::string& faultType,
    const std::string& stack) const
{
    HITRACE_METER_FMT(HITRACE_TAG_APP, "CatchJsonStacktrace pid:%d", pid);
    HiviewDFX::DfxDumpCatcher dumplog;
    std::string msg;
    int timeout = 3000;
    int tid = 0;
    std::pair<int, std::string> dumpResult = dumplog.DumpCatchWithTimeout(pid, msg, timeout, tid, true);
    if (dumpResult.first == DUMP_STACK_FAILED) {
        TAG_LOGI(AAFwkTag::APPDFR, "appfreeze catch json stacktrace failed, %{public}s", dumpResult.second.c_str());
        msg = "Failed to dump stacktrace for " + std::to_string(pid) + "\n" + dumpResult.second + "\n" + msg +
            "\nMain thread stack:" + stack;
        if (faultType == AppFreezeType::APP_INPUT_BLOCK) {
            FindStackByPid(msg, pid);
        }
    } else {
        if (dumpResult.first == DUMP_KERNEL_STACK_SUCCESS) {
            msg = "Failed to dump normal stacktrace for " + std::to_string(pid) + "\n" + dumpResult.second +
                "Kernel stack is:\n" + msg;
        }
        if (faultType == AppFreezeType::THREAD_BLOCK_3S) {
            std::lock_guard<ffrt::mutex> lock(catchStackMutex_);
            catchStackMap_[pid] = msg;
        }
    }
    return msg;
}

std::string AppfreezeManager::CatcherStacktrace(int pid, const std::string& stack) const
{
    HITRACE_METER_FMT(HITRACE_TAG_APP, "CatcherStacktrace pid:%d", pid);
    HiviewDFX::DfxDumpCatcher dumplog;
    std::string msg;
    std::pair<int, std::string> dumpResult = dumplog.DumpCatchWithTimeout(pid, msg);
    if (dumpResult.first == DUMP_STACK_FAILED) {
        TAG_LOGI(AAFwkTag::APPDFR, "appfreeze catch stacktrace failed, %{public}s",
            dumpResult.second.c_str());
        msg = "Failed to dump stacktrace for " + std::to_string(pid) + "\n" + dumpResult.second + "\n" + msg +
            "\nMain thread stack:" + stack;
    } else if (dumpResult.first == DUMP_KERNEL_STACK_SUCCESS) {
        msg = "Failed to dump normal stacktrace for " + std::to_string(pid) + "\n" + dumpResult.second +
            "Kernel stack is:\n" + msg;
    }
    return msg;
}

bool AppfreezeManager::IsProcessDebug(int32_t pid, std::string bundleName)
{
    std::lock_guard<ffrt::mutex> lock(freezeFilterMutex_);
    auto it = appfreezeFilterMap_.find(bundleName);
    if (it != appfreezeFilterMap_.end() && it->second.pid == pid) {
        bool result = it->second.state == AppFreezeState::APPFREEZE_STATE_CANCELED;
        TAG_LOGW(AAFwkTag::APPDFR, "AppfreezeFilter: %{public}d, "
            "bundleName=%{public}s, pid:%{public}d, state:%{public}d, g_betaVersion:%{public}d,"
            " g_developMode:%{public}d",
            result, bundleName.c_str(), pid, it->second.state, g_betaVersion, g_developMode);
        return result;
    }

    const int buffSize = 128;
    char paramBundle[buffSize] = {0};
    GetParameter("hiviewdfx.appfreeze.filter_bundle_name", "", paramBundle, buffSize - 1);
    std::string debugBundle(paramBundle);

    if (bundleName.compare(debugBundle) == 0) {
        TAG_LOGI(AAFwkTag::APPDFR, "filtration %{public}s_%{public}s not exit",
            debugBundle.c_str(), bundleName.c_str());
        return true;
    }
    return false;
}

int64_t AppfreezeManager::GetFreezeCurrentTime()
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return static_cast<int64_t>(((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS);
}

void AppfreezeManager::SetFreezeState(int32_t pid, int state, const std::string& errorName)
{
    std::lock_guard<ffrt::mutex> lock(freezeMutex_);
    if (appfreezeInfo_.find(pid) != appfreezeInfo_.end()) {
        appfreezeInfo_[pid].state = state;
        appfreezeInfo_[pid].occurTime = GetFreezeCurrentTime();
    } else {
        AppFreezeInfo info;
        info.pid = pid;
        info.state = state;
        info.occurTime = GetFreezeCurrentTime();
        info.errorName = errorName;
        appfreezeInfo_.emplace(pid, info);
    }
}

int AppfreezeManager::GetFreezeState(int32_t pid)
{
    std::lock_guard<ffrt::mutex> lock(freezeMutex_);
    auto it = appfreezeInfo_.find(pid);
    if (it != appfreezeInfo_.end()) {
        return it->second.state;
    }
    return AppFreezeState::APPFREEZE_STATE_IDLE;
}

int64_t AppfreezeManager::GetFreezeTime(int32_t pid)
{
    std::lock_guard<ffrt::mutex> lock(freezeMutex_);
    auto it = appfreezeInfo_.find(pid);
    if (it != appfreezeInfo_.end()) {
        return it->second.occurTime;
    }
    return 0;
}

void AppfreezeManager::ClearOldInfo()
{
    std::lock_guard<ffrt::mutex> lock(freezeMutex_);
    int64_t currentTime = GetFreezeCurrentTime();
    for (auto it = appfreezeInfo_.begin(); it != appfreezeInfo_.end();) {
        auto diff = currentTime - it->second.occurTime;
        if (diff > FREEZE_TIME_LIMIT) {
            it = appfreezeInfo_.erase(it);
        } else {
            ++it;
        }
    }
}

bool AppfreezeManager::IsNeedIgnoreFreezeEvent(int32_t pid, const std::string& errorName)
{
    if (appfreezeInfo_.size() >= FREEZEMAP_SIZE_MAX) {
        ClearOldInfo();
    }
    int state = GetFreezeState(pid);
    int64_t currentTime = GetFreezeCurrentTime();
    int64_t lastTime = GetFreezeTime(pid);
    auto diff = currentTime - lastTime;
    if (state == AppFreezeState::APPFREEZE_STATE_FREEZE) {
        if (diff >= FREEZE_TIME_LIMIT) {
            TAG_LOGI(AAFwkTag::APPDFR, "durationTime: "
                "%{public}" PRId64 "state: %{public}d", diff, state);
            return false;
        }
        return true;
    } else {
        if (errorName == "THREAD_BLOCK_3S") {
            return false;
        }
        SetFreezeState(pid, AppFreezeState::APPFREEZE_STATE_FREEZE, errorName);
        TAG_LOGI(AAFwkTag::APPDFR, "durationTime: %{public}" PRId64 ", SetFreezeState: "
            "%{public}s", diff, errorName.c_str());
        return false;
    }
}

bool AppfreezeManager::CancelAppFreezeDetect(int32_t pid, const std::string& bundleName)
{
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPDFR, "SetAppFreezeFilter: failed, bundleName is empty.");
        return false;
    }
    std::lock_guard<ffrt::mutex> lock(freezeFilterMutex_);
    AppFreezeInfo info;
    info.pid = pid;
    info.state = AppFreezeState::APPFREEZE_STATE_CANCELING;
    appfreezeFilterMap_.emplace(bundleName, info);
    TAG_LOGI(AAFwkTag::APPDFR, "SetAppFreezeFilter: success, bundleName=%{public}s, "
        "pid:%{public}d, state:%{public}d", bundleName.c_str(), info.pid, info.state);
    return true;
}

void AppfreezeManager::RemoveDeathProcess(std::string bundleName)
{
    std::lock_guard<ffrt::mutex> lock(freezeFilterMutex_);
    auto it = appfreezeFilterMap_.find(bundleName);
    if (it != appfreezeFilterMap_.end()) {
        TAG_LOGI(AAFwkTag::APPDFR, "RemoveAppFreezeFilter:success, bundleName: %{public}s",
            bundleName.c_str());
        appfreezeFilterMap_.erase(it);
    } else {
        TAG_LOGI(AAFwkTag::APPDFR, "RemoveAppFreezeFilter:failed, not found bundleName: "
            "%{public}s", bundleName.c_str());
    }
}

void AppfreezeManager::ResetAppfreezeState(int32_t pid, const std::string& bundleName)
{
    std::lock_guard<ffrt::mutex> lock(freezeFilterMutex_);
    if (appfreezeFilterMap_.find(bundleName) != appfreezeFilterMap_.end()) {
        appfreezeFilterMap_[bundleName].state = AppFreezeState::APPFREEZE_STATE_CANCELED;
    }
    TAG_LOGI(AAFwkTag::APPDFR, "SetAppFreezeFilter: reset state, "
        "bundleName=%{public}s, pid:%{public}d, state:%{public}d",
        bundleName.c_str(), pid, appfreezeFilterMap_[bundleName].state);
}

bool AppfreezeManager::IsValidFreezeFilter(int32_t pid, const std::string& bundleName)
{
    if (g_betaVersion || g_developMode) {
        TAG_LOGI(AAFwkTag::APPDFR, "SetAppFreezeFilter: "
            "current device is beta or development");
        return true;
    }
    std::lock_guard<ffrt::mutex> lock(freezeFilterMutex_);
    bool ret = appfreezeFilterMap_.find(bundleName) != appfreezeFilterMap_.end();
    TAG_LOGI(AAFwkTag::APPDFR, "SetAppFreezeFilter: %{public}d, bundleName=%{public}s, "
        "pid:%{public}d", ret, bundleName.c_str(), pid);
    return ret;
}
void AppfreezeManager::PerfStart(std::string eventName)
{
    if (OHOS::system::GetParameter("const.dfx.sub_health_recovery.enable", "") != "true") {
        TAG_LOGI(AAFwkTag::APPDFR, "sub_health_recovery is not enable");
        return;
    }
    auto it = HALF_EVENT_CONFIGS.find(eventName);
    if (it == HALF_EVENT_CONFIGS.end()) {
        return;
    }
    auto curTime = AbilityRuntime::TimeUtil::SystemTimeMillisecond();
    if (curTime - perfTime < PERF_TIME) {
        TAG_LOGE(AAFwkTag::APPDFR, "perf time is less than 60s");
        return;
    }
    std::string bigCpuCurFreq = this->GetFirstLine(TWELVE_BIG_CPU_CUR_FREQ);
    std::string bigCpuMaxFreq = this->GetFirstLine(TWELVE_BIG_CPU_MAX_FREQ);
    std::string midCpuCurFreq = this->GetFirstLine(TWELVE_MID_CPU_CUR_FREQ);
    std::string midCpuMaxFreq = this->GetFirstLine(TWELVE_MID_CPU_MAX_FREQ);
    if (bigCpuCurFreq == bigCpuMaxFreq || midCpuCurFreq == midCpuMaxFreq) {
        perfTime = curTime;
        TAG_LOGI(AAFwkTag::APPDFR, "perf start");
        AAFwk::ResSchedUtil::GetInstance().ReportSubHealtyPerfInfoToRSS();
        TAG_LOGI(AAFwkTag::APPDFR, "perf end");
    }
}
std::string AppfreezeManager::GetFirstLine(const std::string &path)
{
    std::ifstream inFile(path.c_str());
    if (!inFile) {
        return "";
    }
    std::string firstLine;
    getline(inFile, firstLine);
    inFile.close();
    return firstLine;
}
}  // namespace AAFwk
}  // namespace OHOS