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
#include "ffrt.h"
#include "freeze_util.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "hisysevent.h"
#include "parameter.h"
#include "xcollie/watchdog.h"
#include "time_util.h"

namespace OHOS {
using AbilityRuntime::FreezeUtil;
namespace AppExecFwk {
namespace {
constexpr char EVENT_UID[] = "UID";
constexpr char EVENT_PID[] = "PID";
constexpr char EVENT_MESSAGE[] = "MSG";
constexpr char EVENT_PACKAGE_NAME[] = "PACKAGE_NAME";
constexpr char EVENT_PROCESS_NAME[] = "PROCESS_NAME";
constexpr char EVENT_STACK[] = "STACK";
constexpr int32_t HALF_DURATION = 3000;
constexpr int32_t HALF_INTERVAL = 300;
}
std::weak_ptr<EventHandler> AppfreezeInner::appMainHandler_;
std::shared_ptr<AppfreezeInner> AppfreezeInner::instance_ = nullptr;
std::mutex AppfreezeInner::singletonMutex_;

AppfreezeInner::AppfreezeInner()
{}

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

void AppfreezeInner::ChangeFaultDateInfo(FaultData& faultData, const std::string& msgContent)
{
    faultData.errorObject.message += msgContent;
    faultData.faultType = FaultDataType::APP_FREEZE;
    faultData.notifyApp = false;
    faultData.waitSaveState = false;
    faultData.forceExit = false;
    int32_t pid = IPCSkeleton::GetCallingPid();
    faultData.errorObject.stack = "\nDump tid stack start time: " +
        AbilityRuntime::TimeUtil::DefaultCurrentTimeStr() + "\n";
    std::string stack = "";
    if (!HiviewDFX::GetBacktraceStringByTidWithMix(stack, pid, 0, true)) {
        stack = "Failed to dump stacktrace for " + std::to_string(pid) + "\n" + stack;
    }
    faultData.errorObject.stack += stack + "\nDump tid stack end time: " +
        AbilityRuntime::TimeUtil::DefaultCurrentTimeStr() + "\n";
    bool isExit = IsExitApp(faultData.errorObject.name) && faultData.needKillProcess;
    if (isExit) {
        faultData.forceExit = true;
        faultData.waitSaveState = AppRecovery::GetInstance().IsEnabled();
        AAFwk::ExitReason exitReason = {REASON_APP_FREEZE, "Kill Reason:" + faultData.errorObject.name};
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
    faultData.errorObject.message =
        "\nFault time:" + AbilityRuntime::TimeUtil::FormatTime("%Y/%m/%d-%H:%M:%S") + "\n";
    faultData.errorObject.message += "App main thread is not response!";
    faultData.faultType = FaultDataType::APP_FREEZE;
    faultData.timeoutMarkers = "";
    if (isSixSecondEvent) {
        faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_6S;
    } else {
        faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_3S;
    }
    if (!IsHandleAppfreeze()) {
        NotifyANR(faultData);
        return;
    }
    std::string msgContent;
    GetMainHandlerDump(msgContent);
    ChangeFaultDateInfo(faultData, msgContent);
    return;
}

int AppfreezeInner::AppfreezeHandle(const FaultData& faultData, bool onlyMainThread)
{
    if (!IsHandleAppfreeze()) {
        NotifyANR(faultData);
        return -1;
    }
    auto reportFreeze = [faultData, onlyMainThread]() {
        if (faultData.errorObject.name == "") {
            TAG_LOGE(AAFwkTag::APPDFR, "null name");
            return;
        }
        AppExecFwk::AppfreezeInner::GetInstance()->AcquireStack(faultData, onlyMainThread);
    };

    {
        std::lock_guard<std::mutex> lock(handlingMutex_);
        handlinglist_.emplace_back(faultData);
        constexpr int HANDLING_MIN_SIZE = 1;
        if (handlinglist_.size() <= HANDLING_MIN_SIZE) {
            TAG_LOGW(AAFwkTag::APPDFR, "submit reportAppFreeze, eventName:%{public}s, startTime:%{public}s\n",
                faultData.errorObject.name.c_str(), AbilityRuntime::TimeUtil::DefaultCurrentTimeStr().c_str());
            ffrt::submit(reportFreeze, {}, {}, ffrt::task_attr().name("reportAppFreeze"));
        }
    }
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

int AppfreezeInner::AcquireStack(const FaultData& info, bool onlyMainThread)
{
    HITRACE_METER_FMT(HITRACE_TAG_APP, "AppfreezeInner::AcquireStack name:%s", info.errorObject.name.c_str());
    std::string msgContent;
    int64_t startTime = AbilityRuntime::TimeUtil::CurrentTimeMillis();
    GetMainHandlerDump(msgContent);
    TAG_LOGW(AAFwkTag::APPDFR, "get mainhandler dump, eventName:%{public}s, endTime:%{public}s, "
        "interval:%{public}" PRId64 " ms", info.errorObject.name.c_str(),
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
        ChangeFaultDateInfo(faultData, msgContent);
    }
    return 0;
}

void AppfreezeInner::ThreadBlock(std::atomic_bool& isSixSecondEvent)
{
    FaultData faultData;
    faultData.errorObject.message =
        "\nFault time:" + AbilityRuntime::TimeUtil::FormatTime("%Y/%m/%d-%H:%M:%S") + "\n";
    faultData.errorObject.message += "App main thread is not response!";
    faultData.faultType = FaultDataType::APP_FREEZE;
    bool onlyMainThread = false;

    if (isSixSecondEvent) {
        faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_6S;
        onlyMainThread = true;
#ifdef APP_NO_RESPONSE_DIALOG
        isSixSecondEvent.store(false);
#endif
    } else {
        faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_3S;
        isSixSecondEvent.store(true);
        std::string outFile;
        OHOS::HiviewDFX::Watchdog::GetInstance().StartSample(HALF_DURATION, HALF_INTERVAL, outFile);
        faultData.appfreezeInfo = outFile;
    }
    faultData.timeoutMarkers = "";

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
        "currentTime:%{public}s, processExit:%{public}d\n", faultData.errorObject.name.c_str(), pid,
        applicationInfo->bundleName.c_str(), AbilityRuntime::TimeUtil::DefaultCurrentTimeStr().c_str(),
        faultData.needKillProcess);

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