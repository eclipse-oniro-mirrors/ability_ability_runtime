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
#include "appfreeze_inner.h"

#include <sys/time.h>

#include "ability_manager_client.h"
#include "ability_state.h"
#include "appfreeze_manager.h"
#include "app_recovery.h"
#include "backtrace_local.h"
#include "exit_reason.h"
#include "file_ex.h"
#include "ffrt.h"
#include "freeze_util.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "hisysevent_report.h"
#include "js_runtime.h"
#include "ohos_application.h"
#include "parameter.h"
#include "xcollie/watchdog.h"
#include "time_util.h"
#include "parameters.h"
#include "unique_fd.h"
#include "input_manager.h"

namespace OHOS {
using AbilityRuntime::FreezeUtil;
namespace AppExecFwk {
namespace {
constexpr int32_t HALF_DURATION = 3000;
constexpr int32_t HALF_INTERVAL = 300;
const bool BETA_VERSION = OHOS::system::GetParameter("const.logsystem.versiontype", "unknown") == "beta";
constexpr int32_t APPFREEZE_INNER_TASKWORKER_NUM = 1;
static constexpr const char *const HEAP_TOTAL_SIZE = "HEAP_TOTAL_SIZE";
static constexpr const char *const HEAP_OBJECT_SIZE = "HEAP_OBJECT_SIZE";
static constexpr const char *const PROCESS_LIFETIME = "PROCESS_LIFETIME";
static constexpr const char *const COLON_SEPARATOR = ":";
static constexpr const char *const COMMA_SEPARATOR = ",";
static constexpr const char *const SECOND = "s";
}
std::weak_ptr<EventHandler> AppfreezeInner::appMainHandler_;
std::shared_ptr<AppfreezeInner> AppfreezeInner::instance_ = nullptr;
std::mutex AppfreezeInner::singletonMutex_;

AppfreezeInner::AppfreezeInner()
{
    appfreezeInnerTaskHandler_ = AAFwk::TaskHandlerWrap::CreateConcurrentQueueHandler(
        "app_freeze_inner_task_queue", APPFREEZE_INNER_TASKWORKER_NUM, AAFwk::TaskQoS::USER_INITIATED);
    appfreezeInnerTaskHandler_->SetPrintTaskLog(true);
}

AppfreezeInner::~AppfreezeInner()
{}

void AppfreezeInner::SetMainHandler(const std::shared_ptr<EventHandler>& eventHandler)
{
    appMainHandler_ = eventHandler;
}

void AppfreezeInner::SetApplicationInfo(const std::shared_ptr<ApplicationInfo>& applicationInfo)
{
    applicationInfo_ = applicationInfo;
}

std::shared_ptr<AppfreezeInner> AppfreezeInner::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(singletonMutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<AppfreezeInner>();
        }
    }
    return instance_;
}

void AppfreezeInner::DestroyInstance()
{
    std::lock_guard<std::mutex> lock(singletonMutex_);
    if (instance_ != nullptr) {
        instance_.reset();
        instance_ = nullptr;
    }
}

bool AppfreezeInner::IsHandleAppfreeze()
{
    return !isAppDebug_;
}

std::string AppfreezeInner::GetProcStatm(int32_t pid)
{
    std::string procStatm;
    std::ifstream statmStream("/proc/" + std::to_string(pid) + "/statm");
    if (statmStream) {
        std::getline(statmStream, procStatm);
        statmStream.close();
    }
    return procStatm;
}

void AppfreezeInner::GetMainHandlerDump(std::string& msgContent)
{
    msgContent = "\nMain handler dump start time: " + AbilityRuntime::TimeUtil::DefaultCurrentTimeStr() + "\n";
    auto mainHandler = appMainHandler_.lock();
    if (mainHandler == nullptr) {
        msgContent += "mainHandler is destructed!\n";
    } else {
        MainHandlerDumper handlerDumper;
        msgContent += "mainHandler dump is:\n";
        mainHandler->Dump(handlerDumper);
        msgContent += handlerDumper.GetDumpInfo();
    }
    msgContent += "Main handler dump end time: " + AbilityRuntime::TimeUtil::DefaultCurrentTimeStr() + "\n";
}

bool AppfreezeInner::ReadFdToString(int fd, std::string& content)
{
    content.clear();
    struct stat sb;
    if (fstat(fd, &sb) != -1 && sb.st_size > 0) {
        content.reserve(sb.st_size);
    }

    char buf[BUFSIZ] = {0};
    ssize_t n;
    while ((n = OHOS_TEMP_FAILURE_RETRY(read(fd, buf, sizeof(buf)))) > 0) {
        content.append(buf, n);
    }
    return (n == 0);
}

bool AppfreezeInner::GetProcessStartTime(pid_t tid, unsigned long long &startTime)
{
    std::string path = "/proc/" +std::to_string(tid);
    UniqueFd dirFd(open(path.c_str(), O_DIRECTORY | O_RDONLY));
    if (dirFd == -1) {
        TAG_LOGE(AAFwkTag::APPDFR, "GetProcessInfo open %{public}s fail. errno %{public}d", path.c_str(), errno);
        return false;
    }

    UniqueFd statFd(openat(dirFd.Get(), "stat", O_RDONLY | O_CLOEXEC));
    if (statFd == -1) {
        TAG_LOGE(AAFwkTag::APPDFR, "GetProcessInfo open %{public}s/stat fail. errno %{public}d", path.c_str(), errno);
        return false;
    }

    std::string statStr;
    if (!ReadFdToString(statFd.Get(), statStr)) {
        TAG_LOGE(AAFwkTag::APPDFR, "GetProcessInfo read string fail.");
        return false;
    }

    auto lastParenPos = statStr.find_last_of(")");
    if (lastParenPos == std::string::npos) {
        return false;
    }
    std::string eoc = statStr.substr(lastParenPos);
    std::istringstream is(eoc);
    constexpr int startTimePos = 21;
    constexpr int base = 10;
    int pos = 0;
    std::string tmp;
    while (is >> tmp && pos <= startTimePos) {
        pos++;
        if (pos == startTimePos) {
            startTime = strtoull(tmp.c_str(), nullptr, base);
            return true;
        }
    }
    TAG_LOGE(AAFwkTag::APPDFR, "GetProcessInfo Get process info fail.");
    return false;
}

std::string AppfreezeInner::GetProcessLifeCycle()
{
    struct timespec ts;
    (void)clock_gettime(CLOCK_BOOTTIME, &ts);
    uint64_t sysUpTime = static_cast<uint64_t>(ts.tv_sec + static_cast<time_t>(ts.tv_nsec != 0 ? 1L : 0L));

    unsigned long long startTime = 0;
    if (GetProcessStartTime(getpid(), startTime)) {
        auto clkTck = sysconf(_SC_CLK_TCK);
        if (clkTck == -1) {
            TAG_LOGE(AAFwkTag::APPDFR, "Get _SC_CLK_TCK fail. errno %{public}d", errno);
            return "";
        }
        uint64_t procUpTime = sysUpTime - startTime / static_cast<uint32_t>(clkTck);
        constexpr uint64_t invalidTimeLimit = 10 * 365 * 24 * 3600; // 10 year
        if (procUpTime > invalidTimeLimit) {
            TAG_LOGE(AAFwkTag::APPDFR, "invalid system upTime %{public}" PRIu64"  proc upTime: %{public}" PRIu64 ",  "
                "startTime: %{public}llu.", sysUpTime, procUpTime, startTime);
            return "";
        }
        std::ostringstream oss;
        oss << PROCESS_LIFETIME << COLON_SEPARATOR << std::to_string(procUpTime) << SECOND;
        return oss.str();
    }
    return "";
}

std::string AppfreezeInner::LogFormat(size_t totalSize, size_t objectSize)
{
    std::ostringstream oss;
    oss << HEAP_TOTAL_SIZE << COLON_SEPARATOR << totalSize << COMMA_SEPARATOR <<
        HEAP_OBJECT_SIZE << COLON_SEPARATOR << objectSize;
    return oss.str();
}

void AppfreezeInner::GetApplicationInfo(FaultData& faultData)
{
    TAG_LOGD(AAFwkTag::APPDFR, "called");
    if (!IsAppFreeze(faultData.errorObject.name)) {
        TAG_LOGI(AAFwkTag::APPDFR, "not to get application info");
        return;
    }

    if (!application_) {
        TAG_LOGE(AAFwkTag::APPDFR, "null application_");
        return;
    }
    auto &runtime = application_->GetRuntime();
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPDFR, "null runtime");
        return;
    }

    if (runtime->GetLanguage() != AbilityRuntime::Runtime::Language::JS) {
        TAG_LOGE(AAFwkTag::APPDFR, "only support js");
        return;
    }

    AbilityRuntime::JsRuntime* jsRuntime = static_cast<AbilityRuntime::JsRuntime*>(runtime.get());
    if (jsRuntime == nullptr) {
        TAG_LOGE(AAFwkTag::APPDFR, "null runtime");
        return;
    }

    size_t heapTotalSize = jsRuntime->GetHeapTotalSize();
    size_t heapObjectSize = jsRuntime->GetHeapObjectSize();
    faultData.applicationHeapInfo = LogFormat(heapTotalSize, heapObjectSize);
    faultData.processLifeTime = GetProcessLifeCycle();
    TAG_LOGI(AAFwkTag::APPDFR, "heap info: %{public}s, process lifeTime: %{public}s",
        faultData.applicationHeapInfo.c_str(), faultData.processLifeTime.c_str());
}

void AppfreezeInner::ChangeFaultDateInfo(FaultData& faultData, const std::string& msgContent)
{
    faultData.errorObject.message += msgContent;
    faultData.isInForeground = GetAppInForeground();
    bool isInBackGround = AppExecFwk::AppfreezeManager::GetInstance()->CheckInBackGround(faultData);
    faultData.faultType = FaultDataType::APP_FREEZE;
    faultData.notifyApp = false;
    faultData.waitSaveState = false;
    faultData.forceExit = false;
    GetApplicationInfo(faultData);
    if (faultData.errorObject.name == AppFreezeType::APP_INPUT_BLOCK) {
        MMI::InputManager::GetInstance()->GetLastEventIds(faultData.markedId,
            faultData.processedId, faultData.dispatchedEventId);
    }
    int32_t pid = IPCSkeleton::GetCallingPid();
    std::string stack = "";
    std::string startTime = "\nDump main thread stack start time: " +
        AbilityRuntime::TimeUtil::DefaultCurrentTimeStr() + "\n";
    if (HiviewDFX::GetBacktraceStringByTidWithMix(stack, pid, 0, true)) {
        faultData.errorObject.stack = startTime + stack + "\nDump main thread stack end time: " +
            AbilityRuntime::TimeUtil::DefaultCurrentTimeStr() + "\n";
    }
    bool isExit = IsExitApp(faultData.errorObject.name) && faultData.needKillProcess;
    if (isExit) {
        faultData.forceExit = true;
        faultData.waitSaveState = AppRecovery::GetInstance().IsEnabled();
        std::string reason = isInBackGround ? AppFreezeType::BG_FREEZE_WARNING : faultData.errorObject.name;
        AAFwk::ExitReason exitReason = {isInBackGround ? REASON_PERFORMANCE_CONTROL : REASON_APP_FREEZE,
            "Kill Reason:" + reason};
        AbilityManagerClient::GetInstance()->RecordAppExitReason(exitReason);
    }
    NotifyANR(faultData);
    if (isExit) {
        AppFreezeRecovery();
    }
}

void AppfreezeInner::AppfreezeHandleOverReportCount(bool isSixSecondEvent)
{
    FaultData faultData;
    std::string faultTimeStr = "\nFault time:" + AbilityRuntime::TimeUtil::FormatTime("%Y/%m/%d-%H:%M:%S") + "\n";
    faultData.errorObject.message = faultTimeStr;
    faultData.errorObject.message += "App main thread is not response!";
    int32_t pid = static_cast<int32_t>(getpid());
    if (isSixSecondEvent) {
        faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_6S;
        faultData.procStatm = GetProcStatm(pid);
        if (BETA_VERSION) {
            std::string packageName = "";
            auto hisyseventReport = std::make_shared<HisyseventReport>(3);
            hisyseventReport->InsertParam("PID", pid);
            hisyseventReport->InsertParam("PACKAGE_NAME", packageName);
            hisyseventReport->InsertParam("FAULT_TIME", faultTimeStr);
            int32_t ret = hisyseventReport->Report("AAFWK", "FREEZE_HALF_HIVIEW_LOG", HISYSEVENT_FAULT);
            faultData.errorObject.message += (ret == 0) ? "FREEZE_HALF_HIVIEW_LOG write success" : "";
        }
    } else {
        if (!BETA_VERSION) {
            std::string packageName = "";
            auto hisyseventReport = std::make_shared<HisyseventReport>(2);
            hisyseventReport->InsertParam("PID", pid);
            hisyseventReport->InsertParam("PACKAGE_NAME", packageName);
            int32_t ret = hisyseventReport->Report("AAFWK", "FREEZE_HALF_HIVIEW_LOG", HISYSEVENT_FAULT);
            TAG_LOGW(AAFwkTag::APPDFR, "hisysevent write FREEZE_HALF_HIVIEW_LOG, pid:%{public}d, packageName:,"
                " ret:%{public}d", pid, ret);
        }
        faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_3S;
    }
    if (!IsHandleAppfreeze()) {
        NotifyANR(faultData);
        return;
    }
    std::string msgContent;
    EnableFreezeSample(faultData);
    GetMainHandlerDump(msgContent);
    ChangeFaultDateInfo(faultData, msgContent);
    return;
}

void AppfreezeInner::EnableFreezeSample(FaultData& newFaultData)
{
    std::string eventName = newFaultData.errorObject.name;
    newFaultData.isInForeground = GetAppInForeground();
    if (eventName == AppFreezeType::THREAD_BLOCK_3S || eventName == AppFreezeType::LIFECYCLE_HALF_TIMEOUT) {
        OHOS::HiviewDFX::Watchdog::GetInstance().StartSample(HALF_DURATION, HALF_INTERVAL);
        TAG_LOGI(AAFwkTag::APPDFR, "start to sample freeze stack, eventName:%{public}s", eventName.c_str());
        return;
    }
    if (IsAppFreeze(eventName)) {
        newFaultData.appfreezeInfo = OHOS::HiviewDFX::Watchdog::GetInstance().StopSample(HALF_DURATION / HALF_INTERVAL);
        newFaultData.isEnableMainThreadSample = GetMainThreadSample();
        OHOS::HiviewDFX::Watchdog::GetInstance().GetSamplerResult(newFaultData.samplerStartTime,
            newFaultData.samplerFinishTime, newFaultData.samplerCount);
        TAG_LOGI(AAFwkTag::APPDFR, "stop to sample freeze stack, eventName:%{public}s freezeFile:%{public}s "
            "enbleMainThreadSample:%{public}d.", eventName.c_str(), newFaultData.appfreezeInfo.c_str(),
            newFaultData.isEnableMainThreadSample);
    }
}

void AppfreezeInner::ReportAppfreezeTask(const FaultData& faultData, bool onlyMainThread)
{
    FaultData newFaultData = faultData;
    EnableFreezeSample(newFaultData);
    auto reportFreeze = [newFaultData, onlyMainThread]() {
        if (newFaultData.errorObject.name == "") {
            TAG_LOGE(AAFwkTag::APPDFR, "null name");
            return;
        }
        AppExecFwk::AppfreezeInner::GetInstance()->AcquireStack(newFaultData, onlyMainThread);
    };

    {
        std::lock_guard<std::mutex> lock(handlingMutex_);
        handlinglist_.emplace_back(newFaultData);
        constexpr int HANDLING_MIN_SIZE = 1;
        if (handlinglist_.size() <= HANDLING_MIN_SIZE) {
            TAG_LOGW(AAFwkTag::APPDFR, "submit reportAppFreeze, name:%{public}s, startTime:%{public}s\n",
                newFaultData.errorObject.name.c_str(), AbilityRuntime::TimeUtil::DefaultCurrentTimeStr().c_str());
            appfreezeInnerTaskHandler_->SubmitTask(reportFreeze, "reportFreeze");
        }
    }
}

int AppfreezeInner::AppfreezeHandle(const FaultData& faultData, bool onlyMainThread)
{
    if (!IsHandleAppfreeze()) {
        NotifyANR(faultData);
        return -1;
    }
    auto watchdogTask = [faultData, onlyMainThread, this] { this->ReportAppfreezeTask(faultData, onlyMainThread); };
    OHOS::HiviewDFX::Watchdog::GetInstance().RunOneShotTask("ReportAppfreezeInnerTask", watchdogTask);
    return 0;
}

bool AppfreezeInner::IsExitApp(const std::string& name)
{
    if (name == AppFreezeType::THREAD_BLOCK_6S || name == AppFreezeType::APP_INPUT_BLOCK ||
        name == AppFreezeType::LIFECYCLE_TIMEOUT || name == AppFreezeType::BUSSINESS_THREAD_BLOCK_6S ||
        name == AppFreezeType::LIFECYCLE_TIMEOUT_WARNING) {
        return true;
    }
    return false;
}

bool AppfreezeInner::IsAppFreeze(const std::string& name)
{
    if (name == AppFreezeType::THREAD_BLOCK_6S || name == AppFreezeType::APP_INPUT_BLOCK ||
        name == AppFreezeType::LIFECYCLE_TIMEOUT) {
        return true;
    }
    return false;
}

int AppfreezeInner::AcquireStack(const FaultData& info, bool onlyMainThread)
{
    HITRACE_METER_FMT(HITRACE_TAG_APP, "AppfreezeInner::AcquireStack name:%s", info.errorObject.name.c_str());
    std::string msgContent;
    int64_t startTime = AbilityRuntime::TimeUtil::CurrentTimeMillis();
    GetMainHandlerDump(msgContent);
    TAG_LOGW(AAFwkTag::APPDFR, "get mainhandler dump, eventName:%{public}s, endTime:%{public}s, "
        "interval:%{public}lld ms", info.errorObject.name.c_str(),
        AbilityRuntime::TimeUtil::DefaultCurrentTimeStr().c_str(),
        static_cast<long long>(AbilityRuntime::TimeUtil::CurrentTimeMillis() - startTime));

    std::lock_guard<std::mutex> lock(handlingMutex_);
    for (auto it = handlinglist_.begin(); it != handlinglist_.end(); it = handlinglist_.erase(it)) {
        HITRACE_METER_FMT(HITRACE_TAG_APP, "send appfreeze name:%s", it->errorObject.name.c_str());
        FaultData faultData;
        faultData.errorObject.message = it->errorObject.message + "\n";
        if (it->state != 0) {
            faultData.errorObject.message += "client actions for ability:\n" +
                FreezeUtil::GetInstance().GetLifecycleEvent(it->token) + "\nclient actions for app:\n" +
                FreezeUtil::GetInstance().GetAppLifecycleEvent(0) + "\n";
            if (it->errorObject.name == AppFreezeType::LIFECYCLE_TIMEOUT ||
                it->errorObject.name == AppFreezeType::LIFECYCLE_TIMEOUT_WARNING) {
                FreezeUtil::GetInstance().DeleteLifecycleEvent(it->token);
                FreezeUtil::GetInstance().DeleteAppLifecycleEvent(0);
            }
        }
        faultData.errorObject.name = it->errorObject.name;
        faultData.timeoutMarkers = it->timeoutMarkers;
        faultData.eventId = it->eventId;
        faultData.needKillProcess = it->needKillProcess;
        faultData.appfreezeInfo = it->appfreezeInfo;
        faultData.appRunningUniqueId = it->appRunningUniqueId;
        faultData.procStatm = it->procStatm;
        faultData.isEnableMainThreadSample = it->isEnableMainThreadSample;
        faultData.schedTime = it->schedTime;
        faultData.detectTime = it->detectTime;
        faultData.appStatus = it->appStatus;
        faultData.samplerStartTime = it->samplerStartTime;
        faultData.samplerFinishTime = it->samplerFinishTime;
        faultData.samplerCount = it->samplerCount;
        faultData.pid = it->pid;
        faultData.applicationHeapInfo = it->applicationHeapInfo;
        faultData.processLifeTime = it->processLifeTime;
        ChangeFaultDateInfo(faultData, msgContent);
    }
    return 0;
}

void AppfreezeInner::ThreadBlock(std::atomic_bool& isSixSecondEvent, uint64_t schedTime,
    uint64_t now, bool isInBackground)
{
    FaultData faultData;
    std::string faultTimeStr = "\nFault time:" + AbilityRuntime::TimeUtil::FormatTime("%Y/%m/%d-%H:%M:%S") + "\n";
    faultData.errorObject.message = faultTimeStr;
    faultData.errorObject.message += "App main thread is not response!";
    bool onlyMainThread = false;
    int32_t pid = static_cast<int32_t>(getpid());
    faultData.pid = pid;
    faultData.schedTime = schedTime;
    faultData.detectTime = now;
    faultData.appStatus = isInBackground ? AppStatus::APP_STATUS_BACKGROUND : AppStatus::APP_STATUS_FOREGROUND;

    if (isSixSecondEvent) {
        faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_6S;
        onlyMainThread = true;
#ifdef APP_NO_RESPONSE_DIALOG
        isSixSecondEvent.store(false);
#endif
        faultData.procStatm = GetProcStatm(pid);
        if (BETA_VERSION) {
            std::string packageName = "";
            auto hisyseventReport = std::make_shared<HisyseventReport>(3);
            hisyseventReport->InsertParam("PID", pid);
            hisyseventReport->InsertParam("PACKAGE_NAME", packageName);
            hisyseventReport->InsertParam("FAULT_TIME", faultTimeStr);
            int32_t ret = hisyseventReport->Report("AAFWK", "FREEZE_HALF_HIVIEW_LOG", HISYSEVENT_FAULT);
            faultData.errorObject.message += (ret == 0) ? "FREEZE_HALF_HIVIEW_LOG write success" : "";
        }
    } else {
        if (!BETA_VERSION) {
            std::string packageName = "";
            auto hisyseventReport = std::make_shared<HisyseventReport>(2);
            hisyseventReport->InsertParam("PID", pid);
            hisyseventReport->InsertParam("PACKAGE_NAME", packageName);
            int32_t ret = hisyseventReport->Report("AAFWK", "FREEZE_HALF_HIVIEW_LOG", HISYSEVENT_FAULT);
            TAG_LOGW(AAFwkTag::APPDFR, "hisysevent write FREEZE_HALF_HIVIEW_LOG, pid:%{public}d, packageName:,"
                " ret:%{public}d", pid, ret);
        }
        faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_3S;
        isSixSecondEvent.store(true);
    }

    if (!IsHandleAppfreeze()) {
        return;
    }

    AppfreezeHandle(faultData, onlyMainThread);
}

int AppfreezeInner::NotifyANR(const FaultData& faultData)
{
    HITRACE_METER_FMT(HITRACE_TAG_APP, "AppfreezeInner::NotifyANR name:%s",
        faultData.errorObject.name.c_str());
    auto applicationInfo = applicationInfo_.lock();
    if (applicationInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPDFR, "null applicationInfo_");
        return -1;
    }

    int32_t pid = static_cast<int32_t>(getpid());
    TAG_LOGW(AAFwkTag::APPDFR, "NotifyAppFault:%{public}s, pid:%{public}d, bundleName:%{public}s "
        "currentTime:%{public}s, processExit:%{public}d appfreezeInfo:%{public}s enbleMainThreadSample:%{public}d.",
        faultData.errorObject.name.c_str(), pid,
        applicationInfo->bundleName.c_str(), AbilityRuntime::TimeUtil::DefaultCurrentTimeStr().c_str(),
        faultData.needKillProcess, faultData.appfreezeInfo.c_str(), faultData.isEnableMainThreadSample);

    int ret = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->NotifyAppFault(faultData);
    if (ret != 0) {
        TAG_LOGW(AAFwkTag::APPDFR, "NotifyAppFault ret:%{public}d", ret);
    }
    return ret;
}

void AppfreezeInner::AppFreezeRecovery()
{
    AppRecovery::GetInstance().ScheduleSaveAppState(StateReason::APP_FREEZE);
    AppRecovery::GetInstance().ScheduleRecoverApp(StateReason::APP_FREEZE);
}

void AppfreezeInner::SetAppDebug(bool isAppDebug)
{
    isAppDebug_ = isAppDebug;
}

void AppfreezeInner::SetAppInForeground(bool isInForeground)
{
    isInForeground_ = isInForeground;
}

bool AppfreezeInner::GetAppInForeground()
{
    return isInForeground_;
}

void AppfreezeInner::SetMainThreadSample(bool isEnableMainThreadSample)
{
    isEnableMainThreadSample_ = isEnableMainThreadSample;
}

bool AppfreezeInner::GetMainThreadSample()
{
    return isEnableMainThreadSample_;
}

void AppfreezeInner::SetAppfreezeApplication(const std::shared_ptr<OHOSApplication> &application)
{
    if (application == nullptr) {
        TAG_LOGE(AAFwkTag::APPDFR, "null application");
        return;
    }
    application_ = application;
}

void MainHandlerDumper::Dump(const std::string &message)
{
    dumpInfo += message;
}

std::string MainHandlerDumper::GetTag()
{
    return "";
}

std::string MainHandlerDumper::GetDumpInfo()
{
    return dumpInfo;
}
}  // namespace AAFwk
}  // namespace OHOS