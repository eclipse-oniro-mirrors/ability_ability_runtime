/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include <sstream>
#include <sys/ioctl.h>

#include "faultloggerd_client.h"
#include "file_ex.h"
#include "ffrt.h"
#include "dfx_dump_catcher.h"
#include "directory_ex.h"
#include "hisysevent_report.h"
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
#include "appfreeze_event_report.h"
#include "appfreeze_util.h"
#include "xcollie/process_kill_reason.h"

#undef FREEZE_DOMAIN
#define FREEZE_DOMAIN 0xD001310
namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr char KILL_EVENT_NAME[] = "APP_KILL";
constexpr int MAX_LAYER = 8;
constexpr int FREEZEMAP_SIZE_MAX = 20;
constexpr int FREEZE_TIME_LIMIT = 60000;
constexpr int FREEZE_EVENT_MAX_SIZE = 200;
constexpr int64_t FREEZE_KILL_LIMIT = 60000;
static constexpr uint8_t ARR_SIZE = 7;
static constexpr uint8_t DECIMAL = 10;
static constexpr uint8_t FREE_ASYNC_INDEX = 6;
static constexpr uint16_t FREE_ASYNC_MAX = 1000;
static constexpr int64_t NANOSECONDS = 1000000000;  // NANOSECONDS mean 10^9 nano second
static constexpr int64_t MICROSECONDS = 1000000;    // MICROSECONDS mean 10^6 millias second
static constexpr int DUMP_STACK_FAILED = -1;
static constexpr int DUMP_KERNEL_STACK_SUCCESS = 1;
static constexpr int MIN_APP_UID = 20000;
static constexpr int MAX_REPORT_STACK_SIZE = 256 * 1024; // 256KB
const std::string LOG_FILE_PATH = "/data/log/eventlog";
static bool g_betaVersion = OHOS::system::GetParameter("const.logsystem.versiontype", "unknown") == "beta";
static bool g_overseaVersion = OHOS::system::GetParameter("const.global.region", "CN") != "CN";
static bool g_developMode = (OHOS::system::GetParameter("persist.hiview.leak_detector", "unknown") == "enable") ||
                            (OHOS::system::GetParameter("persist.hiview.leak_detector", "unknown") == "true");

static constexpr const char *const HITRACE_ID = "hitrace_id: ";
static constexpr const char *const SPAN_ID = "span_id: ";
static constexpr const char *const PARENT_SPAN_ID = "parent_span_id: ";
static constexpr const char *const TRACE_FLAG = "trace_flag: ";
static constexpr const char *const DEV_SYSLOAD = "/dev/sysload";
static constexpr int SYSLOAD_GET_KILL_INFO_MAGIC = 0xE5AC02;

#define KILL_LOG_BASE 'S'
#define GET_KILL_INFO _IOWR(KILL_LOG_BASE, 0x06, int32_t)

#define SYSLOAD_GET_KILL_INFO_MAGIC 0xE5AC02

struct KillEventInfo {
    enum HiviewDFX::ProcessKillReason::KillEventId id;
    int adj;
    bool processed;
    bool foreground;
    pid_t pid;
    int uid;
    int64_t timestamp;
    int64_t eventParamFirst;
    int64_t eventParamSecond;
    int64_t eventParamThird;
    int64_t eventParamFouth;
};

struct KillInfo {
    unsigned int magic;
    pid_t pid;
    struct KillEventInfo data;
    unsigned int structSize;
};
}
static constexpr const char *const TWELVE_BIG_CPU_CUR_FREQ = "/sys/devices/system/cpu/cpufreq/policy2/scaling_cur_freq";
static constexpr const char *const TWELVE_BIG_CPU_MAX_FREQ = "/sys/devices/system/cpu/cpufreq/policy2/scaling_max_freq";
static constexpr const char *const TWELVE_MID_CPU_CUR_FREQ = "/sys/devices/system/cpu/cpufreq/policy1/scaling_cur_freq";
static constexpr const char *const TWELVE_MID_CPU_MAX_FREQ = "/sys/devices/system/cpu/cpufreq/policy1/scaling_max_freq";
const static std::set<std::string> HALF_EVENT_CONFIGS = {"UI_BLOCK_3S", "THREAD_BLOCK_3S", "BUSSNESS_THREAD_BLOCK_3S",
                                                         "LIFECYCLE_HALF_TIMEOUT", "LIFECYCLE_HALF_TIMEOUT_WARNING"};
static constexpr int PERF_TIME = 60000;
static constexpr int FREEZE_EVENT_ZERO = 0;
static constexpr int FREEZE_EVENT_FIRST = 1;
static constexpr int FREEZE_EVENT_SED = 2;
static constexpr int FREEZE_EVENT_THIRD = 3;
static constexpr int FREEZE_EVENT_FORTH = 4;
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

std::string AppfreezeManager::GetCatcherStack(const std::string& fileName, const std::string& catcherStack)
{
    std::string stackFile = WriteToFile(fileName, catcherStack);
    if (!stackFile.empty()) {
        return stackFile;
    }
    if (catcherStack.size() > MAX_REPORT_STACK_SIZE) {
        stackFile = "";
        TAG_LOGW(AAFwkTag::APPDFR, "create stack failed, catcherStack over size: %{public}zu",
            catcherStack.size());
    } else {
        stackFile = catcherStack;
        TAG_LOGW(AAFwkTag::APPDFR, "create stack failed, catcherStack size: %{public}zu", catcherStack.size());
    }
    return stackFile;
}

int AppfreezeManager::MergeNotifyInfo(FaultData& faultNotifyData, const AppfreezeManager::AppInfo& appInfo)
{
    std::string memoryContent;
    CollectFreezeSysMemory(memoryContent);
    std::string fileName = faultNotifyData.errorObject.name + "_" +
        AbilityRuntime::TimeUtil::FormatTime("%Y%m%d%H%M%S") + "_" + std::to_string(appInfo.pid) + "_stack";
    std::string catcherStack;
    faultNotifyData.errorObject.message += "\nCatche stack trace start time: " +
        AbilityRuntime::TimeUtil::DefaultCurrentTimeStr() + "\n";
    uint64_t dumpStartTime = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    std::string resultMsg;
    if (faultNotifyData.errorObject.name == AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING ||
        faultNotifyData.errorObject.name == AppFreezeType::LIFECYCLE_TIMEOUT_WARNING) {
        catcherStack += CatcherStacktrace(appInfo.pid, faultNotifyData.errorObject.stack);
    } else {
        std::pair<std::string, std::string> catchResult =
            CatchJsonStacktrace(appInfo.pid, faultNotifyData.errorObject.name, faultNotifyData.errorObject.stack);
        catcherStack += catchResult.first;
        resultMsg += catchResult.second;
    }
    uint64_t dumpFinishTime = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    std::string timeStamp = "Catche stack trace end time: " + AbilityRuntime::TimeUtil::DefaultCurrentTimeStr();
    catcherStack = GetCatcherStack(fileName, catcherStack);
    faultNotifyData.errorObject.stack = catcherStack.empty() ? faultNotifyData.errorObject.stack : catcherStack;
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
    RecordAppFreezeBehavior(faultNotifyData, dumpStartTime, dumpFinishTime, resultMsg);
    return 0;
}

void AppfreezeManager::RecordAppFreezeBehavior(FaultData& faultData, uint64_t dumpStartTime,
    uint64_t dumpFinishTime, const std::string& resultMsg)
{
    std::lock_guard<std::mutex> mapLock(freezeMapMutex_);
    std::string eventName = faultData.errorObject.name;
    if (freezeEventMap_.size() > FREEZE_EVENT_MAX_SIZE) {
        freezeEventMap_.clear();
    }
    auto it = std::find(APP_FREEZE_EVENT_NAME.begin(), APP_FREEZE_EVENT_NAME.end(), eventName);
    if (it != APP_FREEZE_EVENT_NAME.end()) {
        freezeEventMap_[faultData.pid][eventName].schedTime = faultData.schedTime;
        freezeEventMap_[faultData.pid][eventName].detectTime = faultData.detectTime;
        freezeEventMap_[faultData.pid][eventName].dumpStartTime = dumpStartTime;
        freezeEventMap_[faultData.pid][eventName].dumpFinishTime = dumpFinishTime;
        freezeEventMap_[faultData.pid][eventName].dumpResult = resultMsg;
        freezeEventMap_[faultData.pid][eventName].appStatus = faultData.appStatus;
        freezeEventMap_[faultData.pid][KILL_EVENT_NAME].dumpStartTime = faultData.samplerStartTime;
        freezeEventMap_[faultData.pid][KILL_EVENT_NAME].dumpFinishTime = faultData.samplerFinishTime;
        freezeEventMap_[faultData.pid][KILL_EVENT_NAME].dumpResult = std::to_string(faultData.samplerCount);
        freezeEventMap_[faultData.pid][KILL_EVENT_NAME].isRepeatKilledThread = false;
    }
}

int AppfreezeManager::AppfreezeHandleWithStack(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo)
{
    TAG_LOGW(AAFwkTag::APPDFR, "NotifyAppFaultTask called, eventName:%{public}s, bundleName:%{public}s, "
        "name_:%{public}s, currentTime:%{public}s, isInForeground:%{public}d",
        faultData.errorObject.name.c_str(), appInfo.bundleName.c_str(),
        name_.c_str(), AbilityRuntime::TimeUtil::DefaultCurrentTimeStr().c_str(), faultData.isInForeground);
    if (!IsHandleAppfreeze(appInfo.bundleName)) {
        return -1;
    }
    FaultData faultNotifyData;
    faultNotifyData.errorObject.name = faultData.errorObject.name;
    faultNotifyData.errorObject.message = faultData.errorObject.message;
    faultNotifyData.errorObject.stack = faultData.errorObject.stack;
    faultNotifyData.faultType = FaultDataType::APP_FREEZE;
    faultNotifyData.eventId = faultData.eventId;
    faultNotifyData.schedTime = faultData.schedTime;
    faultNotifyData.detectTime = faultData.detectTime;
    faultNotifyData.appStatus = faultData.appStatus;
    faultNotifyData.samplerStartTime = faultData.samplerStartTime;
    faultNotifyData.samplerFinishTime = faultData.samplerFinishTime;
    faultNotifyData.samplerCount = faultData.samplerCount;
    faultNotifyData.pid = faultData.pid;
    faultNotifyData.tid = faultData.tid;
    faultNotifyData.appfreezeInfo = faultData.appfreezeInfo;
    faultNotifyData.appRunningUniqueId = faultData.appRunningUniqueId;
    faultNotifyData.procStatm = faultData.procStatm;
    faultNotifyData.isInForeground = faultData.isInForeground;
    faultNotifyData.isEnableMainThreadSample = faultData.isEnableMainThreadSample;
    faultNotifyData.applicationHeapInfo = faultData.applicationHeapInfo;
    faultNotifyData.processLifeTime = faultData.processLifeTime;
    faultNotifyData.markedId = faultData.markedId;
    faultNotifyData.processedId = faultData.processedId;
    faultNotifyData.dispatchedEventId = faultData.dispatchedEventId;
    HITRACE_METER_FMT(HITRACE_TAG_APP, "AppfreezeHandleWithStack pid:%{public}d-name:%{public}s",
        appInfo.pid, faultData.errorObject.name.c_str());
    return MergeNotifyInfo(faultNotifyData, appInfo);
}

std::string AppfreezeManager::WriteToFile(const std::string& fileName, const std::string& content)
{
    std::string path = LOG_FILE_PATH + "/freeze";
    constexpr mode_t defaultLogDirMode = 0770;
    if (!OHOS::FileExists(path)) {
        OHOS::ForceCreateDirectory(path);
        OHOS::ChangeModeDirectory(path, defaultLogDirMode);
    }
    std::string realPath = AppfreezeUtil::FreezePathToRealPath(path);
    if (realPath.empty()) {
        realPath = path;
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
    if (info.typeId != AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT || !IsHandleAppfreeze(info.bundleName)) {
        return -1;
    }
    if (info.eventName != AppFreezeType::LIFECYCLE_TIMEOUT && info.eventName != AppFreezeType::LIFECYCLE_HALF_TIMEOUT
        && info.eventName != AppFreezeType::LIFECYCLE_TIMEOUT_WARNING
        && info.eventName != AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING) {
        return -1;
    }

    std::string faultTimeStr = "\nFault time:" + AbilityRuntime::TimeUtil::FormatTime("%Y/%m/%d-%H:%M:%S") + "\n";
    if (!g_betaVersion && info.eventName == AppFreezeType::LIFECYCLE_HALF_TIMEOUT) {
        auto hisyseventReport = std::make_shared<AAFwk::HisyseventReport>(2);
        hisyseventReport->InsertParam("PID", info.pid);
        hisyseventReport->InsertParam("PACKAGE_NAME", info.bundleName);
        int32_t ret = hisyseventReport->Report("AAFWK", "FREEZE_HALF_HIVIEW_LOG", HISYSEVENT_FAULT);
        TAG_LOGW(AAFwkTag::APPDFR, "hisysevent write FREEZE_HALF_HIVIEW_LOG, pid:%{public}d, packageName:%{public}s,"
            " ret:%{public}d", info.pid, info.bundleName.c_str(), ret);
    }

    std::string message;
    if (g_betaVersion && info.eventName == AppFreezeType::LIFECYCLE_TIMEOUT) {
        auto hisyseventReport = std::make_shared<AAFwk::HisyseventReport>(3);
        hisyseventReport->InsertParam("PID", info.pid);
        hisyseventReport->InsertParam("PACKAGE_NAME", info.bundleName);
        hisyseventReport->InsertParam("FAULT_TIME", faultTimeStr);
        int32_t ret = hisyseventReport->Report("AAFWK", "FREEZE_HALF_HIVIEW_LOG", HISYSEVENT_FAULT);
        message = (ret == 0) ? "FREEZE_HALF_HIVIEW_LOG write success" : "";
    }

    TAG_LOGD(AAFwkTag::APPDFR, "called %{public}s, name_ %{public}s", info.bundleName.c_str(), name_.c_str());
    HITRACE_METER_FMT(HITRACE_TAG_APP, "LifecycleTimeoutHandle:%{public}s bundleName:%{public}s",
        info.eventName.c_str(), info.bundleName.c_str());

    AppFaultDataBySA faultDataSA = GenerateFaultDataBySA(info, flow);
    faultDataSA.errorObject.message = faultTimeStr + faultDataSA.errorObject.message + message;
    DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->NotifyAppFaultBySA(faultDataSA);
    return 0;
}

AppFaultDataBySA AppfreezeManager::GenerateFaultDataBySA(const ParamInfo& info, const FreezeUtil::LifecycleFlow& flow)
{
    AppFaultDataBySA faultDataSA;
    if (info.eventName == AppFreezeType::LIFECYCLE_TIMEOUT) {
        std::string realPath = AppfreezeUtil::FreezePathToRealPath("/proc/" + std::to_string(info.pid) + "/statm");
        std::ifstream statmStream(realPath);
        if (statmStream) {
            std::string procStatm;
            std::getline(statmStream, procStatm);
            statmStream.close();
            faultDataSA.procStatm = procStatm;
        }
    }
    faultDataSA.errorObject.name = info.eventName;
    faultDataSA.errorObject.message = info.msg;
    faultDataSA.faultType = FaultDataType::APP_FREEZE;
    faultDataSA.timeoutMarkers = "notifyFault" + std::to_string(info.pid) +
                                 "-" + std::to_string(AbilityRuntime::TimeUtil::CurrentTimeMillis());
    faultDataSA.pid = info.pid;
    faultDataSA.needKillProcess = info.needKillProcess;
    if (flow.state != AbilityRuntime::FreezeUtil::TimeoutState::UNKNOWN) {
        faultDataSA.token = flow.token;
        faultDataSA.state = static_cast<uint32_t>(flow.state);
    }
    faultDataSA.detectTime = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    return faultDataSA;
}

FaultData AppfreezeManager::GetFaultNotifyData(const FaultData& faultData, int pid)
{
    FaultData faultNotifyData;
    faultNotifyData.errorObject.name = faultData.errorObject.name;
    faultNotifyData.errorObject.message = faultData.errorObject.message;
    faultNotifyData.errorObject.stack = faultData.errorObject.stack;
    faultNotifyData.faultType = FaultDataType::APP_FREEZE;
    faultNotifyData.eventId = faultData.eventId;
    if (faultData.errorObject.name == AppFreezeType::APP_INPUT_BLOCK || faultData.tid <= 0) {
        faultNotifyData.tid = pid;
    } else {
        faultNotifyData.tid = faultData.tid;
    }
    faultNotifyData.appfreezeInfo = faultData.appfreezeInfo;
    faultNotifyData.appRunningUniqueId = faultData.appRunningUniqueId;
    faultNotifyData.procStatm = faultData.procStatm;
    faultNotifyData.isInForeground = faultData.isInForeground;
    faultNotifyData.isEnableMainThreadSample = faultData.isEnableMainThreadSample;
    faultNotifyData.applicationHeapInfo = faultData.applicationHeapInfo;
    faultNotifyData.processLifeTime = faultData.processLifeTime;
    faultNotifyData.markedId = faultData.markedId;
    faultNotifyData.processedId = faultData.processedId;
    faultNotifyData.dispatchedEventId = faultData.dispatchedEventId;
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
        binderInfo += "PeerBinder pids is empty\n";
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
    binderInfo += CatchASyncByPid(asyncPids, syncPids, pid);
    std::string fileName = faultData.errorObject.name + "_" +
        AbilityRuntime::TimeUtil::FormatTime("%Y%m%d%H%M%S") + "_" + std::to_string(pid) + "_binder";
    std::string fullStackPath = WriteToFile(fileName, binderInfo);
    binderInfo = fullStackPath + "," + binderPidsStr + "," + terminalBinderTid;

    int ret = NotifyANR(faultNotifyData, appInfo, binderInfo, memoryContent);
    return ret;
}

std::string AppfreezeManager::CatchASyncByPid(const std::set<int> &asyncPids, const std::set<int> &syncPids, int pid)
{
    std::string binderInfo;
    for (auto& pidTemp : asyncPids) {
        int uid = AppfreezeUtil::GetUidByPid(pidTemp);
        if (uid < 0 || uid >= MIN_APP_UID) {
            TAG_LOGI(AAFwkTag::APPDFR, "Async stack, skip current pid:%{public}d, uid:%{public}d", pidTemp, uid);
            continue;
        }
        TAG_LOGI(AAFwkTag::APPDFR, "AsyncBinder pidTemp pids:%{public}d", pidTemp);
        if (pidTemp != pid && syncPids.find(pidTemp) == syncPids.end()) {
            std::string content = "Binder catcher stacktrace, type is async, pid : " + std::to_string(pidTemp) + "\n";
            content += CatcherStacktrace(pidTemp, "");
            binderInfo += content;
        }
    }
    return binderInfo;
}

std::string AppfreezeManager::ParseDecToHex(uint64_t id)
{
    std::stringstream ss;
    ss << std::hex << id;
    return ss.str();
}

std::string AppfreezeManager::GetHitraceInfo()
{
#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
    OHOS::HiviewDFX::HiTraceId hitraceId = OHOS::HiviewDFX::HiTraceChain::GetId();
    if (hitraceId.IsValid() == 0) {
        TAG_LOGW(AAFwkTag::APPDFR, "get hitrace id is invalid.");
        return "";
    }
    std::ostringstream hitraceIdStr;
    hitraceIdStr << "hitrace_id: " << ParseDecToHex(hitraceId.GetChainId()) <<
        ", span_id: " << ParseDecToHex(hitraceId.GetSpanId()) <<
        ", parent_span_id: " << ParseDecToHex(hitraceId.GetParentSpanId()) <<
        ", trace_flag: " << ParseDecToHex(hitraceId.GetFlags());
    std::string hiTraceIdInfo = hitraceIdStr.str();
    TAG_LOGW(AAFwkTag::APPDFR, "hitraceIdStr: %{public}s", hiTraceIdInfo.c_str());
    return hitraceIdStr.str();
#endif
    return "";
}

void AppfreezeManager::InitWarningCpuInfo(const FaultData& faultData,
    const AppfreezeManager::AppInfo& appInfo)
{
    std::string eventName = faultData.errorObject.name;
    if (eventName != AppFreezeType::THREAD_BLOCK_3S &&
        eventName != AppFreezeType::LIFECYCLE_HALF_TIMEOUT) {
        return;
    }
    std::string type = std::to_string(appInfo.pid) + "-" + std::to_string(appInfo.uid) + "-" +
        appInfo.bundleName;
    bool ret = AppExecFwk::AppfreezeCpuFreqManager::GetInstance().InsertCpuDetailInfo(type, appInfo.pid);
    TAG_LOGI(AAFwkTag::APPDFR, "Insert cpuInfo ret:%{public}d, pid:%{public}d, name:%{public}s, "
        "appfreezeInfo:%{public}s, type:%{public}s", ret, appInfo.pid,
        eventName.c_str(), faultData.appfreezeInfo.c_str(), type.c_str());
}

std::string AppfreezeManager::GetAppfreezeInfoPath(const FaultData& faultData,
    const AppfreezeManager::AppInfo& appInfo)
{
    std::string eventName = faultData.errorObject.name;
    std::string cpuInfoFile = faultData.appfreezeInfo;
    if (eventName == AppFreezeType::THREAD_BLOCK_6S || eventName == AppFreezeType::LIFECYCLE_TIMEOUT ||
        eventName == AppFreezeType::APP_INPUT_BLOCK) {
        std::string type = std::to_string(appInfo.pid) + "-" + std::to_string(appInfo.uid) + "-" +
            appInfo.bundleName;
        cpuInfoFile += "," + AppExecFwk::AppfreezeCpuFreqManager::GetInstance().GetCpuInfoPath(
            type, appInfo.bundleName, appInfo.uid, appInfo.pid);
        TAG_LOGI(AAFwkTag::APPDFR, "name:%{public}s, cpuInfoFile:%{public}s, type:%{public}s",
            faultData.errorObject.name.c_str(), cpuInfoFile.c_str(), type.c_str());
    }
    return cpuInfoFile;
}

int AppfreezeManager::NotifyANR(const FaultData& faultData, const AppfreezeManager::AppInfo& appInfo,
    const std::string& binderInfo, const std::string& memoryContent)
{
    std::string eventName = faultData.errorObject.name;
    this->PerfStart(eventName);
    int64_t startTime = AbilityRuntime::TimeUtil::CurrentTimeMillis();
    int tid = faultData.tid;
    std::string appRunningUniqueId = faultData.appRunningUniqueId;
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid > 0 ? tid : 0;
    eventInfo.pid = appInfo.pid;
    eventInfo.uid = appInfo.uid;
    eventInfo.eventId = faultData.eventId;
    eventInfo.bundleName = appInfo.bundleName;
    eventInfo.processName = appInfo.processName;
    eventInfo.binderInfo = binderInfo;
    eventInfo.freezeMemory = memoryContent + "\n" + faultData.procStatm;
    eventInfo.appRunningUniqueId = appRunningUniqueId;
    eventInfo.errorStack = faultData.errorObject.stack;
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = faultData.errorObject.message;
    eventInfo.freezeInfoFile = GetAppfreezeInfoPath(faultData, appInfo);
    eventInfo.hitraceInfo = GetHitraceInfo();
    eventInfo.foregroundState = faultData.isInForeground;
    eventInfo.enableFreeze = faultData.isEnableMainThreadSample;
    eventInfo.applicationHeapInfo = faultData.applicationHeapInfo;
    eventInfo.processLifeTime = faultData.processLifeTime;
    eventInfo.markedId = faultData.markedId;
    eventInfo.processedId = faultData.processedId;
    eventInfo.dispatchedEventId = faultData.dispatchedEventId;

    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName, HISYSEVENT_FAULT, eventInfo);
    TAG_LOGW(AAFwkTag::APPDFR, "reportEvent:%{public}s, pid:%{public}d, tid:%{public}d, bundleName:%{public}s, "
        "appRunningUniqueId:%{public}s, endTime:%{public}s, interval:%{public}" PRId64 " ms, "
        "eventId:%{public}d freezeInfoFile:%{public}s foreground:%{public}d enableFreeze:%{public}d,"
        "applicationHeapInfo:%{public}s processLifeTime:%{public}s hisysevent write ret: %{public}d",
        faultData.errorObject.name.c_str(), appInfo.pid, faultData.tid, appInfo.bundleName.c_str(),
        appRunningUniqueId.c_str(), AbilityRuntime::TimeUtil::DefaultCurrentTimeStr().c_str(),
        AbilityRuntime::TimeUtil::CurrentTimeMillis() - startTime, faultData.eventId,
        eventInfo.freezeInfoFile.c_str(), eventInfo.foregroundState, eventInfo.enableFreeze,
        eventInfo.applicationHeapInfo.c_str(), eventInfo.processLifeTime.c_str(), ret);
    OHOS::HiviewDFX::HiTraceChain::ClearId();
    return 0;
}

std::map<int, std::list<AppfreezeManager::PeerBinderInfo>> AppfreezeManager::BinderParser(std::ifstream& fin,
    std::string& stack, std::set<int>& asyncPids) const
{
    std::map<uint32_t, uint32_t> asyncBinderMap;
    std::vector<std::pair<uint32_t, uint64_t>> freeAsyncSpacePairs;
    std::map<int, std::list<AppfreezeManager::PeerBinderInfo>> binderInfos = BinderLineParser(fin, stack,
        asyncBinderMap, freeAsyncSpacePairs);

    if (!g_overseaVersion) {
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
        if (g_overseaVersion) {
            continue;
        }
        isBinderMatchup = (!isBinderMatchup && line.find("free_async_space") != line.npos) ? true : isBinderMatchup;
        std::vector<std::string> strList = GetFileToList(line);

        if (isBinderMatchup) {
            if (line.find("free_async_space") == line.npos && strList.size() == ARR_SIZE &&
                std::atoll(strList[FREE_ASYNC_INDEX].c_str()) < FREE_ASYNC_MAX) {
                freeAsyncSpacePairs.emplace_back(std::atoi(strList[0].c_str()),
                    std::atoll(strList[FREE_ASYNC_INDEX].c_str()));
            }
        } else if (line.find("async\t") != std::string::npos && strList.size() > ARR_SIZE) {
            std::string serverPid = StrSplit(strList[3], 0);
            std::string serverTid = StrSplit(strList[3], 1);
            if (serverPid != "" && serverTid != "" && std::atoi(serverTid.c_str()) == 0) {
                asyncBinderMap[std::atoi(serverPid.c_str())]++;
            }
        } else if (strList.size() >= ARR_SIZE) { // 7: valid array size
            AppfreezeManager::PeerBinderInfo info = {0};
            // 0: local id,
            std::string clientPid = StrSplit(strList[0], 0);
            std::string clientTid = StrSplit(strList[0], 1);
            // 2: peer id,
            std::string serverPid = StrSplit(strList[2], 0);
            std::string serverTid = StrSplit(strList[2], 1);
             // 5: wait time, s
            std::string wait = StrSplit(strList[5], 1);
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

std::string AppfreezeManager::StrSplit(const std::string& str, uint16_t index) const
{
    std::vector<std::string> strings;
    SplitStr(str, ":", strings);
    return index < strings.size() ? strings[index] : "";
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

    if (g_overseaVersion || binderInfos.size() == 0 || binderInfos.find(params.pid) == binderInfos.end()) {
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

std::pair<std::string, std::string> AppfreezeManager::CatchJsonStacktrace(int pid, const std::string& faultType,
    const std::string& stack) const
{
    HITRACE_METER_FMT(HITRACE_TAG_APP, "CatchJsonStacktrace pid:%{public}d", pid);
    HiviewDFX::DfxDumpCatcher dumplog;
    std::string msg;
    int timeout = 3000;
    int tid = 0;
    std::pair<int, std::string> dumpResult = dumplog.DumpCatchWithTimeout(pid, msg, timeout, tid, true);
    if (dumpResult.first == DUMP_STACK_FAILED) {
        TAG_LOGI(AAFwkTag::APPDFR, "appfreeze catch json stacktrace failed, %{public}s", dumpResult.second.c_str());
        msg = "Failed to dump stacktrace for " + std::to_string(pid) + "\n" + dumpResult.second + msg + stack;
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
    return std::make_pair(msg, dumpResult.second);
}

std::string AppfreezeManager::CatcherStacktrace(int pid, const std::string& stack) const
{
    HITRACE_METER_FMT(HITRACE_TAG_APP, "CatcherStacktrace pid:%{public}d", pid);
    HiviewDFX::DfxDumpCatcher dumplog;
    std::string msg;
    std::pair<int, std::string> dumpResult = dumplog.DumpCatchWithTimeout(pid, msg);
    if (dumpResult.first == DUMP_STACK_FAILED) {
        TAG_LOGI(AAFwkTag::APPDFR, "appfreeze catch stacktrace failed, %{public}s",
            dumpResult.second.c_str());
        msg = "Failed to dump stacktrace for " + std::to_string(pid) + "\n" + dumpResult.second + msg + stack;
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
        bool result = (it->second.state == AppFreezeState::APPFREEZE_STATE_CANCELING);
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

void AppfreezeManager::SetFreezeState(const std::string& key, int state, const std::string& eventName)
{
    if (eventName == AppFreezeType::THREAD_BLOCK_3S ||
        eventName == AppFreezeType::BUSSINESS_THREAD_BLOCK_3S) {
        return;
    }
    AppFreezeInfo info;
    info.state = state;
    info.occurTime = GetFreezeCurrentTime();
    std::lock_guard<ffrt::mutex> lock(freezeMutex_);
    appfreezeInfo_[key] = info;
}

int AppfreezeManager::GetReportTimes(const std::string& key)
{
    int reportTimes = DEFAULT_APPFREEZE_REPORT_TIMES;
    std::lock_guard<ffrt::mutex> lock(freezeMutex_);
    auto it = appfreezeInfo_.find(key);
    if (it != appfreezeInfo_.end()) {
        reportTimes += it->second.reportTimes;
        appfreezeInfo_[key].reportTimes = reportTimes;
    }
    return reportTimes;
}

int64_t AppfreezeManager::GetLastOccurTime(const std::string& key)
{
    std::lock_guard<ffrt::mutex> lock(freezeMutex_);
    auto it = appfreezeInfo_.find(key);
    if (it != appfreezeInfo_.end()) {
        return it->second.occurTime;
    }
    return 0;
}

bool AppfreezeManager::ClearOldInfo(std::map<std::string, AppFreezeInfo>& infoMap,
    size_t maxSize, int64_t maxTimelimit)
{
    if (infoMap.size() < maxSize) {
        return true;
    }
    int removeCount = 0;
    int64_t curTime = GetFreezeCurrentTime();
    for (auto it = infoMap.begin(); it != infoMap.end();) {
        auto interval = curTime - it->second.occurTime;
        if (interval > maxTimelimit || interval < 0) {
            it = infoMap.erase(it);
            removeCount++;
        } else {
            ++it;
        }
    }
    TAG_LOGI(AAFwkTag::APPDFR, "remove old count: %{public}d, "
        "current info count: %{public}zu", removeCount, infoMap.size());
    return removeCount != 0;
}

bool AppfreezeManager::IsNeedIgnoreFreezeEvent(const std::string& key, const std::string& eventName,
    int maxReportTimes)
{
    int state = 0;
    {
        std::lock_guard<ffrt::mutex> lock(freezeMutex_);
        if (!ClearOldInfo(appfreezeInfo_, FREEZEMAP_SIZE_MAX, FREEZE_TIME_LIMIT)) {
            return false;
        }
        auto it = appfreezeInfo_.find(key);
        state = it != appfreezeInfo_.end() ? it->second.state : AppFreezeState::APPFREEZE_STATE_IDLE;
    }
    if (state == AppFreezeState::APPFREEZE_STATE_FREEZE) {
        auto diff = GetFreezeCurrentTime() - GetLastOccurTime(key);
        int reportTimes = GetReportTimes(key);
        TAG_LOGW(AAFwkTag::APPDFR, "durationTime: %{public}" PRId64 " eventName: %{public}s "
            "reportTimes: %{public}d key: %{public}s",
            diff, eventName.c_str(), reportTimes, key.c_str());
        if (diff >= 0 && diff <= FREEZE_TIME_LIMIT) {
            return reportTimes > maxReportTimes;
        }
    }
    SetFreezeState(key, AppFreezeState::APPFREEZE_STATE_FREEZE, eventName);
    TAG_LOGI(AAFwkTag::APPDFR, "Set freeze info, eventName: %{public}s key: %{public}s",
        eventName.c_str(), key.c_str());
    return false;
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
    appfreezeFilterMap_[bundleName] = info;
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
        TAG_LOGD(AAFwkTag::APPDFR, "RemoveAppFreezeFilter:failed, not found bundleName: "
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
    bool ret = appfreezeFilterMap_.find(bundleName) == appfreezeFilterMap_.end();
    TAG_LOGI(AAFwkTag::APPDFR, "SetAppFreezeFilter: %{public}d, bundleName=%{public}s, "
        "pid:%{public}d", ret, bundleName.c_str(), pid);
    return ret;
}

void AppfreezeManager::ReportAppFreezeSysEvents(int32_t pid, const std::string& bundleName)
{
    std::lock_guard<std::mutex> mapLock(freezeMapMutex_);
    if (freezeEventMap_.find(pid) == freezeEventMap_.end()) {
        return;
    }

    uint64_t now = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    auto hisyseventReport = std::make_shared<AAFwk::HisyseventReport>(40);
    hisyseventReport->InsertParam("THREAD_HALF_SCHED",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_ZERO]].schedTime);
    hisyseventReport->InsertParam("THREAD_HALF_DETECT",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_ZERO]].detectTime);
    hisyseventReport->InsertParam("THREAD_HALF_DUMP_START",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_ZERO]].dumpStartTime);
    hisyseventReport->InsertParam("THREAD_HALF_DUMP_FINISH",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_ZERO]].dumpFinishTime);
    hisyseventReport->InsertParam("THREAD_HALF_DUMP_RESULT",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_ZERO]].dumpResult);
    hisyseventReport->InsertParam("THREAD_HALF_APP_STATUS",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_ZERO]].appStatus);
    hisyseventReport->InsertParam("THREAD_TIMEOUT_SCHED",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_FIRST]].schedTime);
    hisyseventReport->InsertParam("THREAD_TIMEOUT_DETECT",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_FIRST]].detectTime);
    hisyseventReport->InsertParam("THREAD_TIMEOUT_DUMP_START",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_FIRST]].dumpStartTime);
    hisyseventReport->InsertParam("THREAD_TIMEOUT_DUMP_FINISH",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_FIRST]].dumpFinishTime);
    hisyseventReport->InsertParam("THREAD_TIMEOUT_DUMP_RESULT",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_FIRST]].dumpResult);
    hisyseventReport->InsertParam("THREAD_TIMEOUT_APP_STATUS",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_FIRST]].appStatus);
    hisyseventReport->InsertParam("INPUT_SCHED",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_SED]].schedTime);
    hisyseventReport->InsertParam("INPUT_DETECT",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_SED]].detectTime);
    hisyseventReport->InsertParam("INPUT_DUMP_START",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_SED]].dumpStartTime);
    hisyseventReport->InsertParam("INPUT_DUMP_FINISH",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_SED]].dumpFinishTime);
    hisyseventReport->InsertParam("INPUT_DUMP_RESULT",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_SED]].dumpResult);
    hisyseventReport->InsertParam("INPUT_APP_STATUS",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_SED]].appStatus);
    hisyseventReport->InsertParam("LIFECYCLE_HALF_SCHED",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_THIRD]].schedTime);
    hisyseventReport->InsertParam("LIFECYCLE_HALF_DETECT",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_THIRD]].detectTime);
    hisyseventReport->InsertParam("LIFECYCLE_HALF_DUMP_START",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_THIRD]].dumpStartTime);
    hisyseventReport->InsertParam("LIFECYCLE_HALF_DUMP_FINISH",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_THIRD]].dumpFinishTime);
    hisyseventReport->InsertParam("LIFECYCLE_HALF_DUMP_RESULT",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_THIRD]].dumpResult);
    hisyseventReport->InsertParam("LIFECYCLE_HALF_APP_STATUS",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_THIRD]].appStatus);
    hisyseventReport->InsertParam("LIFECYCLE_TIMEOUT_SCHED",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_FORTH]].schedTime);
    hisyseventReport->InsertParam("LIFECYCLE_TIMEOUT_DETECT",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_FORTH]].detectTime);
    hisyseventReport->InsertParam("LIFECYCLE_TIMEOUT_DUMP_START",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_FORTH]].dumpStartTime);
    hisyseventReport->InsertParam("LIFECYCLE_TIMEOUT_DUMP_FINISH",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_FORTH]].dumpFinishTime);
    hisyseventReport->InsertParam("LIFECYCLE_TIMEOUT_DUMP_RESULT",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_FORTH]].dumpResult);
    hisyseventReport->InsertParam("LIFECYCLE_TIMEOUT_APP_STATUS",
        freezeEventMap_[pid][APP_FREEZE_EVENT_NAME[FREEZE_EVENT_FORTH]].appStatus);
    hisyseventReport->InsertParam("APP_KILL_TIME",
        freezeEventMap_[pid][KILL_EVENT_NAME].schedTime);
    hisyseventReport->InsertParam("APP_TERMINATED_TIME", now);
    hisyseventReport->InsertParam("SAMPLER_START",
        freezeEventMap_[pid][KILL_EVENT_NAME].dumpStartTime);
    hisyseventReport->InsertParam("SAMPLER_FINISH",
        freezeEventMap_[pid][KILL_EVENT_NAME].dumpFinishTime);
    hisyseventReport->InsertParam("SAMPLER_COUNT",
        freezeEventMap_[pid][KILL_EVENT_NAME].dumpResult);
    hisyseventReport->InsertParam("BUNDLE_NAME", bundleName);
    hisyseventReport->InsertParam("APP_PID", pid);
    hisyseventReport->InsertParam("REPEAT_KILLED_THREAD",
        freezeEventMap_[pid][KILL_EVENT_NAME].isRepeatKilledThread);
    int ret = hisyseventReport->Report("RELIABILITY", "APP_FREEZE_STATISTIC", HISYSEVENT_STATISTIC);
    freezeEventMap_.erase(pid);
}

void AppfreezeManager::RegisterAppKillTime(int32_t pid, uint64_t time)
{
    std::lock_guard<std::mutex> mapLock(freezeMapMutex_);
    if (freezeEventMap_.find(pid) == freezeEventMap_.end()) {
        return;
    }
    freezeEventMap_[pid][KILL_EVENT_NAME].schedTime = time;
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
    std::string realPath = AppfreezeUtil::FreezePathToRealPath(path);
    if (realPath.empty()) {
        return "";
    }
    std::ifstream inFile(realPath.c_str());
    if (!inFile) {
        return "";
    }
    std::string firstLine;
    getline(inFile, firstLine);
    inFile.close();
    return firstLine;
}

bool AppfreezeManager::CheckInBackGround(const FaultData &faultData)
{
    return faultData.errorObject.name == AppFreezeType::THREAD_BLOCK_6S &&
        !faultData.isInForeground;
}

bool AppfreezeManager::CheckAppfreezeHappend(const std::string& key, const std::string& eventName)
{
    bool result = false;
    if (eventName == AppFreezeType::LIFECYCLE_TIMEOUT || eventName == AppFreezeType::APP_INPUT_BLOCK ||
        eventName == AppFreezeType::THREAD_BLOCK_6S || eventName == AppFreezeType::THREAD_BLOCK_3S ||
        eventName == AppFreezeType::BUSSINESS_THREAD_BLOCK_3S ||
        eventName == AppFreezeType::BUSSINESS_THREAD_BLOCK_6S) {
        result = IsNeedIgnoreFreezeEvent(key, eventName);
    } else if (eventName == AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING ||
        eventName == AppFreezeType::LIFECYCLE_TIMEOUT_WARNING) {
        std::string warningKey = eventName;
        result = IsNeedIgnoreFreezeEvent(warningKey, eventName);
    }
    if (result) {
        TAG_LOGE(AAFwkTag::APPDFR, "appFreeze happend, key:%{public}s, eventName:%{public}s",
            key.c_str(), eventName.c_str());
    }
    return result;
}

bool AppfreezeManager::CheckNeedRecordAppRunningUnquieId(const std::string& eventName)
{
    if (eventName == AppFreezeType::THREAD_BLOCK_3S || eventName == AppFreezeType::THREAD_BLOCK_6S ||
        eventName == AppFreezeType::BUSSINESS_THREAD_BLOCK_3S ||
        eventName == AppFreezeType::BUSSINESS_THREAD_BLOCK_6S ||
        eventName == AppFreezeType::BUSINESS_INPUT_BLOCK) {
        return true;
    }
    return false;
}

bool AppfreezeManager::IsBetaVersion()
{
    return g_betaVersion;
}

void AppfreezeManager::InsertKillThread(int32_t state, int32_t pid, int32_t uid, const std::string& bundleName)
{
    std::string key = bundleName + AppfreezeUtil::KEY_SEPARATOR + std::to_string(pid) +
        AppfreezeUtil::KEY_SEPARATOR + std::to_string(uid);
    std::lock_guard<std::mutex> mapLock(freezeKillThreadMutex_);
    if (!ClearOldInfo(freezeKillThreadMap_, FREEZEMAP_SIZE_MAX, FREEZE_TIME_LIMIT)) {
        return;
    }
    if (freezeKillThreadMap_.find(key) != freezeKillThreadMap_.end()) {
        freezeKillThreadMap_[key].state = state;
        freezeKillThreadMap_[key].occurTime = GetFreezeCurrentTime();
    } else {
        AppFreezeInfo info;
        info.state = state;
        info.occurTime = GetFreezeCurrentTime();
        freezeKillThreadMap_[key] = info;
    }
    TAG_LOGI(AAFwkTag::APPDFR, "insert or update key: %{public}s", key.c_str());
}

bool AppfreezeManager::CheckThreadKilled(int32_t pid, int32_t uid, const std::string& bundleName)
{
    std::string key = bundleName + AppfreezeUtil::KEY_SEPARATOR + std::to_string(pid) +
        AppfreezeUtil::KEY_SEPARATOR + std::to_string(uid);
    bool result = false;
    bool isRepeatKilledThread = false;
    {
        std::lock_guard<std::mutex> mapLock(freezeKillThreadMutex_);
        if (freezeKillThreadMap_.empty()) {
            return result;
        }
        auto it = freezeKillThreadMap_.find(key);
        if (it != freezeKillThreadMap_.end()) {
            result = it->second.state >= 0;
            if (result && !it->second.isRepeatKilledThread) {
                it->second.isRepeatKilledThread = true;
                isRepeatKilledThread = true;
            }
        }
    }

    if (isRepeatKilledThread) {
        {
            std::lock_guard<std::mutex> mapLock(freezeMapMutex_);
            if (freezeEventMap_.find(pid) != freezeEventMap_.end()) {
                freezeEventMap_[pid][KILL_EVENT_NAME].isRepeatKilledThread = true;
            }
        }
        ReportAppFreezeSysEvents(pid, bundleName);
    }
    return result;
}

bool AppfreezeManager::IsSkipDetect(int32_t pid, int32_t uid, const std::string& bundleName,
    const std::string& eventName)
{
    if (CheckThreadKilled(pid, uid, bundleName)) {
        TAG_LOGW(AAFwkTag::APPDFR, "bundleName: %{public}s has been killed, pid: %{public}d",
            bundleName.c_str(), pid);
        return true;
    }
    if (IsProcessDebug(pid, bundleName)) {
        TAG_LOGW(AAFwkTag::APPDFR, "don't report event and kill:%{public}s, pid:%{public}d, bundleName:%{public}s",
            eventName.c_str(), pid, bundleName.c_str());
        return true;
    }
    return false;
}

std::string AppfreezeManager::GetExitReasonByKillId(int32_t killId)
{
    return HiviewDFX::ProcessKillReason::GetKillReason(killId);
}

std::string AppfreezeManager::GetExitKernelReason(int32_t pid)
{
    int sysloadFd = open(DEV_SYSLOAD, O_RDWR);
    if (sysloadFd < 0) {
        TAG_LOGW(AAFwkTag::APPDFR, "open failed, errno:%{public}d", errno);
        return "";
    }
    fdsan_exchange_owner_tag(sysloadFd, 0, FREEZE_DOMAIN);
    KillInfo info = {0};
    info.magic = SYSLOAD_GET_KILL_INFO_MAGIC;
    info.pid = pid;
    info.structSize = sizeof(struct KillInfo);

    int res = ioctl(sysloadFd, GET_KILL_INFO, &info);
    if (fdsan_close_with_tag(sysloadFd, FREEZE_DOMAIN) != 0) {
        TAG_LOGW(AAFwkTag::APPDFR, "GetExitKernelReason fdsan close failed, errno:%{public}d", errno);
    }
    int killId = -1;
    if (res == 0) {
        killId = static_cast<int>(info.data.id);
    } else {
        TAG_LOGW(AAFwkTag::APPDFR, "ioctl failed, errno:%{public}d", errno);
    }
    return GetExitReasonByKillId(killId);
}
}  // namespace AAFwk
}  // namespace OHOS