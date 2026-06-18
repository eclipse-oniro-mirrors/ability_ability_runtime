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
#include <sstream>

#include "ability_manager_client.h"
#include "ability_state.h"
#include "appfreeze_manager.h"
#include "app_recovery.h"
#include "backtrace_local.h"
#include "file_ex.h"
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
#include "exit_reason.h"
#include "dfx_jsnapi.h"

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
static constexpr const char *const HEAP_SHARED_SIZE = "HEAP_SHARED_SIZE";
static constexpr const char *const PROCESS_LIFETIME = "PROCESS_LIFETIME";
static constexpr const char *const COLON_SEPARATOR = ":";
static constexpr const char *const COMMA_SEPARATOR = ",";
static constexpr const char *const SECOND = "s";
static constexpr const char *const GC_COUNT = "count";
static constexpr const char *const GC_MAX_PAUSE = "maxPause";
static constexpr const char *const GC_MIN_PAUSE = "minPause";
static constexpr const char *const GC_AVERAGE_PAUSE = "averagePause";
static constexpr const char *const GC_LAST_START_TIME = "lastStartTime";
static constexpr const char *const GC_LAST_END_TIME = "lastEndTime";
static constexpr const char *const GC_LAST_TYPE = "lastType";
static constexpr const char *const GC_SHARED_GC_TYPE = "Shared GC";
static constexpr const char *const PROC_SELF_IO = "/proc/self/io";
constexpr int THREAD_BLOCK_3S_TYPE = 0;
constexpr int THREAD_BLOCK_6S_TYPE = 1;
constexpr int LIFECYCLE_HALF_TIMEOUT_TYPE = 2;
constexpr int LIFECYCLE_TIMEOUT_TYPE = 3;
constexpr int APP_INPUT_BLOCK_TYPE = 4;
constexpr int BUSSINESS_THREAD_BLOCK_3S_TYPE = 5;
constexpr int BUSSINESS_THREAD_BLOCK_6S_TYPE = 6;
constexpr int BUSINESS_INPUT_BLOCK_TYPE = 7;
constexpr int DUMP_MAIN_STACK_TIMEOUT = 1; // s
constexpr int LAST_SAVE_MAIN_STACK_TIME = 3000; // ms
constexpr uint64_t APP_INPUT_BLOCK_TIME = 8000; // ms
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

bool AppfreezeInner::IsProcessDebug(int32_t pid, std::string bundleName)
{
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

std::string AppfreezeInner::GetBundleNameByApplication()
{
    auto applicationInfo = applicationInfo_.lock();
    if (applicationInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPDFR, "null applicationInfo_");
        return "";
    }
    return applicationInfo->bundleName;
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

std::string AppfreezeInner::LogFormatHeapSize(size_t totalSize, size_t objectSize, size_t sharedSize)
{
    std::ostringstream oss;
    oss << HEAP_TOTAL_SIZE << COLON_SEPARATOR << totalSize << COMMA_SEPARATOR <<
        HEAP_OBJECT_SIZE << COLON_SEPARATOR << objectSize << COMMA_SEPARATOR <<
        HEAP_SHARED_SIZE << COLON_SEPARATOR << sharedSize;
    return oss.str();
}

std::string AppfreezeInner::LogFormatGC(const panda::GCStatistic& gCStatistic)
{
    std::ostringstream oss;
    oss << GC_COUNT << COLON_SEPARATOR << gCStatistic.count << COMMA_SEPARATOR <<
        GC_MAX_PAUSE << COLON_SEPARATOR << gCStatistic.maxPause << COMMA_SEPARATOR <<
        GC_MIN_PAUSE << COLON_SEPARATOR << gCStatistic.minPause << COMMA_SEPARATOR <<
        GC_AVERAGE_PAUSE << COLON_SEPARATOR << gCStatistic.averagePause << COMMA_SEPARATOR <<
        GC_LAST_START_TIME << COLON_SEPARATOR << gCStatistic.lastStartTime << COMMA_SEPARATOR <<
        GC_LAST_END_TIME << COLON_SEPARATOR << gCStatistic.lastEndTime << COMMA_SEPARATOR <<
        GC_LAST_TYPE << COLON_SEPARATOR << gCStatistic.lastType;
    return oss.str();
}

std::string AppfreezeInner::ParseIOValue(std::string ioStr)
{
    if (ioStr.empty()) {
        return "";
    }
    std::istringstream iss(ioStr);
    std::string line;
    bool first = true;

    std::string key;
    std::string value;
    std::ostringstream oss;
    while (std::getline(iss, line)) {
        if (line.empty()) {
            continue;
        }
        size_t colonPos = line.find(":");
        if (colonPos == std::string::npos) {
            continue;
        }
        key = line.substr(0, colonPos);
        value = line.substr(colonPos + 1);
        value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
        if (key.empty() || value.empty()) {
            continue;
        }
        if (!first) {
            oss << COMMA_SEPARATOR;
        } else {
            first = false;
        }
        oss << key << COLON_SEPARATOR << value;
    }
    TAG_LOGD(AAFwkTag::APPDFR, "read io: %{public}s", oss.str().c_str());
    return oss.str();
}

std::string AppfreezeInner::GetProcessIOStr()
{
    char realPath[PATH_MAX] = {0};
    if (realpath(PROC_SELF_IO, realPath) == nullptr) {
        TAG_LOGE(AAFwkTag::APPDFR, "realpath error, errno: %{public}d, path: %{public}s", errno, PROC_SELF_IO);
        return "";
    }
    UniqueFd ioFd(open(realPath, O_RDONLY | O_CLOEXEC));
    if (ioFd < 0) {
        TAG_LOGE(AAFwkTag::APPDFR, "open %{public}s fail. errno %{public}d", realPath, errno);
        return "";
    }

    std::string ioStr;
    if (!ReadFdToString(ioFd.Get(), ioStr)) {
        TAG_LOGE(AAFwkTag::APPDFR, "read string fail, path: %{public}s", realPath);
        return "";
    }

    return ParseIOValue(ioStr);
}

void AppfreezeInner::GetApplicationInfo(FaultData& faultData)
{
    TAG_LOGD(AAFwkTag::APPDFR, "called");
    if (!IsAppFreeze(faultData.errorObject.name) && !IsAppFreezeWarning(faultData.errorObject.name)) {
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
    size_t sharedSize = jsRuntime->GetSharedHeapSize();
    faultData.applicationHeapInfo = LogFormatHeapSize(heapTotalSize, heapObjectSize, sharedSize);
    faultData.processLifeTime = GetProcessLifeCycle();
    faultData.applicationIOInfo = GetProcessIOStr();
    panda::GCStatistic gCStatistic = jsRuntime->GetGCStatistic();
    faultData.applicationGCInfo = LogFormatGC(gCStatistic);
    faultData.isBlockInGc = CheckSharedGC(gCStatistic.lastType) &&
        CheckBlockInGC(faultData.errorObject.name, gCStatistic.lastStartTime, gCStatistic.lastEndTime);
    TAG_LOGI(AAFwkTag::APPDFR, "heap info: %{public}s, process lifeTime: %{public}s",
        faultData.applicationHeapInfo.c_str(), faultData.processLifeTime.c_str());
}

bool AppfreezeInner::CheckSharedGC(std::string lastType)
{
    return lastType != GC_SHARED_GC_TYPE;
}

int64_t AppfreezeInner::GetFreezeCurrentTime()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
}

bool AppfreezeInner::IsBlockTimeInGCPeriod(uint64_t halfTime, uint64_t blockTime,
    uint64_t lastStartTime, uint64_t lastEndTime)
{
    if (halfTime == 0 || blockTime == 0 || lastStartTime == 0 || lastEndTime == 0) {
        return false;
    }
    if (lastStartTime > lastEndTime || halfTime > blockTime) {
        return false;
    }

    return (halfTime >= lastStartTime && blockTime < lastEndTime);
}

bool AppfreezeInner::CheckBlockInGC(const std::string& faultName,
    uint64_t lastStartTime, uint64_t lastEndTime)
{
    uint64_t now =  static_cast<uint64_t>(GetFreezeCurrentTime());
    if (faultName == AppFreezeType::THREAD_BLOCK_3S) {
        threadBlock3STime_ = now;
    } else if (faultName == AppFreezeType::THREAD_BLOCK_6S) {
        return IsBlockTimeInGCPeriod(threadBlock3STime_, now, lastStartTime, lastEndTime);
    } else if (faultName == AppFreezeType::LIFECYCLE_HALF_TIMEOUT) {
        lifeCycleHalfTime_ = now;
    } else if (faultName == AppFreezeType::LIFECYCLE_TIMEOUT) {
        return IsBlockTimeInGCPeriod(lifeCycleHalfTime_, now, lastStartTime, lastEndTime);
    } else if (faultName == AppFreezeType::APP_INPUT_BLOCK) {
        uint64_t inputBegin = now - APP_INPUT_BLOCK_TIME;
        return IsBlockTimeInGCPeriod(inputBegin, now, lastStartTime, lastEndTime);
    }
    return false;
}

int AppfreezeInner::TransformHicollieFaultNumber(const std::string& faultName)
{
    if (faultName == AppFreezeType::THREAD_BLOCK_3S) {
        return THREAD_BLOCK_3S_TYPE;
    } else if (faultName == AppFreezeType::THREAD_BLOCK_6S) {
        return THREAD_BLOCK_6S_TYPE;
    } else if (faultName == AppFreezeType::LIFECYCLE_HALF_TIMEOUT) {
        return LIFECYCLE_HALF_TIMEOUT_TYPE;
    } else if (faultName == AppFreezeType::LIFECYCLE_TIMEOUT) {
        return LIFECYCLE_TIMEOUT_TYPE;
    } else if (faultName == AppFreezeType::APP_INPUT_BLOCK) {
        return APP_INPUT_BLOCK_TYPE;
    } else if (faultName == AppFreezeType::BUSSINESS_THREAD_BLOCK_3S) {
        return BUSSINESS_THREAD_BLOCK_3S_TYPE;
    } else if (faultName == AppFreezeType::BUSSINESS_THREAD_BLOCK_6S) {
        return BUSSINESS_THREAD_BLOCK_6S_TYPE;
    } else if (faultName == AppFreezeType::BUSINESS_INPUT_BLOCK) {
        return BUSINESS_INPUT_BLOCK_TYPE;
    }
    return -1;
}

std::string AppfreezeInner::GetMainStackDump(int32_t pid)
{
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
    if (!lastMainStack_.empty() && now - lastMainStackTime_ < LAST_SAVE_MAIN_STACK_TIME) {
        {
            std::unique_lock<ffrt::mutex> lock(mainStackMutex_);
            return lastMainStack_;
        }
    }
    auto task = [pid, this]() {
        std::string startTime = "\nDump main thread stack start time: " +
            AbilityRuntime::TimeUtil::DefaultCurrentTimeStr() + "\n";
        std::string mainStack;
        if (HiviewDFX::GetBacktraceStringByTidWithMix(mainStack, pid, 0, true)) {
            mainStack = startTime + mainStack + "\nDump main thread stack end time: " +
                AbilityRuntime::TimeUtil::DefaultCurrentTimeStr() + "\n";
        } else {
            TAG_LOGE(AAFwkTag::APPDFR, "get main stack failed, mainStack=%{public}s", mainStack.c_str());
        }
        lastMainStackTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
            system_clock::now().time_since_epoch()).count();
        std::unique_lock<ffrt::mutex> lock(mainStackMutex_);
        lastMainStack_ = mainStack;
        mainStackCv_.notify_one();
    };
    ffrt::submit_h(task);

    {
        std::unique_lock<ffrt::mutex> lock(mainStackMutex_);
        if (mainStackCv_.wait_for(lock, std::chrono::seconds(DUMP_MAIN_STACK_TIMEOUT)) == ffrt::cv_status::timeout) {
            TAG_LOGW(AAFwkTag::APPDFR, "get main stack has been extecting more than 1s");
        } else {
            TAG_LOGI(AAFwkTag::APPDFR, "get main stack has finished less than 1s");
            return lastMainStack_;
        }
    }
    return "";
}

void AppfreezeInner::ChangeFaultDateInfo(FaultData& faultData, const std::string& msgContent)
{
    faultData.errorObject.message += msgContent;
    faultData.isInForeground = GetAppInForeground();
    faultData.faultType = FaultDataType::APP_FREEZE;
    faultData.notifyApp = false;
    faultData.waitSaveState = false;
    faultData.forceExit = false;
    int faultNum = TransformHicollieFaultNumber(faultData.errorObject.name);
    faultData.callbackLog = OHOS::HiviewDFX::Watchdog::GetInstance().ReadDataFromBuffer(faultNum);
    GetApplicationInfo(faultData);
    if (faultData.errorObject.name == AppFreezeType::LIFECYCLE_TIMEOUT) {
        faultData.reportLifecycleToFreeze = GetReportLifeCycleAsAppfreeze();
    }
    if (faultData.errorObject.name == AppFreezeType::APP_INPUT_BLOCK) {
        MMI::InputManager::GetInstance()->GetLastEventIds(faultData.markedId,
            faultData.processedId, faultData.dispatchedEventId);
    }
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t uid = IPCSkeleton::GetCallingUid();
    faultData.errorObject.mainStack = GetMainStackDump(pid);
    bool isExit = IsExitApp(faultData.errorObject.name) && faultData.needKillProcess;
    if (isExit) {
        faultData.forceExit = true;
        faultData.waitSaveState = AppRecovery::GetInstance().IsEnabled();
        std::string reason = faultData.errorObject.name;
        AAFwk::ExitReasonCompability exitReason = { REASON_APP_FREEZE, "Reason:" + reason };
        exitReason.killId =
            AppExecFwk::AppfreezeManager::GetInstance()->GetFreezeExitReason(faultData.errorObject.name);
        exitReason.killMsg = reason;
        exitReason.innerMsg = reason;
        auto result = AbilityManagerClient::GetInstance()->RecordAppWithReason(pid, uid, exitReason);
        TAG_LOGI(AAFwkTag::APPDFR, "Record result=%{public}d, pid=%{public}d, uid=%{public}d, "
            "killId=%{public}d", result, pid, uid, exitReason.killId);
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
    std::string bundleName = GetBundleNameByApplication();
    if (!IsHandleAppfreeze() || IsProcessDebug(pid, bundleName)) {
        TAG_LOGW(AAFwkTag::APPDFR, "don't report event and kill, pid:%{public}d, bundleName:%{public}s",
            pid, bundleName.c_str());
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
        newFaultData.appfreezeInfo = OHOS::HiviewDFX::Watchdog::GetInstance().StartSample(HALF_DURATION, HALF_INTERVAL);
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
    int32_t pid = static_cast<int32_t>(getpid());
    std::string bundleName = GetBundleNameByApplication();
    if (!IsHandleAppfreeze() || IsProcessDebug(pid, bundleName)) {
        TAG_LOGW(AAFwkTag::APPDFR, "don't report event and kill, pid:%{public}d, bundleName:%{public}s",
            pid, bundleName.c_str());
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

bool AppfreezeInner::IsAppFreezeWarning(const std::string& name)
{
    if (name == AppFreezeType::THREAD_BLOCK_3S || name == AppFreezeType::LIFECYCLE_HALF_TIMEOUT) {
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
        "interval:[%{public}" PRId64 "] ms", info.errorObject.name.c_str(),
        AbilityRuntime::TimeUtil::DefaultCurrentTimeStr().c_str(),
        AbilityRuntime::TimeUtil::CurrentTimeMillis() - startTime);

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

void AppfreezeInner::SetReportLifeCycleAsAppfreeze(bool reportLifecycleToFreeze)
{
    reportLifecycleToFreeze_ = reportLifecycleToFreeze;
}

bool AppfreezeInner::GetReportLifeCycleAsAppfreeze()
{
    return reportLifecycleToFreeze_;
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