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

int AppfreezeInner::AppfreezeHandle(const FaultData& faultData, bool onlyMainThread)
{
    if (!IsHandleAppfreeze()) {
        NotifyANR(faultData);
        return -1;
    }
    auto reportFreeze = [faultData, onlyMainThread]() {
        if (faultData.errorObject.name == "") {
            TAG_LOGE(AAFwkTag::APPDFR, "name is nullptr, AppfreezeHandle failed.");
            return;
        }
        AppExecFwk::AppfreezeInner::GetInstance()->AcquireStack(faultData, onlyMainThread);
    };

    {
        std::lock_guard<std::mutex> lock(handlingMutex_);
        handlinglist_.emplace_back(faultData);
        constexpr int HANDLING_MIN_SIZE = 1;
        if (handlinglist_.size() <= HANDLING_MIN_SIZE) {
            ffrt::submit(reportFreeze, {}, {}, ffrt::task_attr().name("reportAppFreeze"));
        }
    }
    return 0;
}

bool AppfreezeInner::IsExitApp(const std::string& name)
{
    if (name == AppFreezeType::THREAD_BLOCK_6S || name == AppFreezeType::APP_INPUT_BLOCK ||
        name == AppFreezeType::LIFECYCLE_TIMEOUT || name == AppFreezeType::BUSSINESS_THREAD_BLOCK_6S) {
        return true;
    }
    return false;
}

void AppfreezeInner::SendProcessKillEvent(const std::string& killReason)
{
    auto applicationInfo = applicationInfo_.lock();
    if (applicationInfo != nullptr) {
        int32_t pid = static_cast<int32_t>(getpid());
        std::string processName = applicationInfo->process;
        int result = HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::FRAMEWORK, "PROCESS_KILL",
            HiviewDFX::HiSysEvent::EventType::FAULT, EVENT_PID, pid,
            EVENT_PROCESS_NAME, processName, EVENT_MESSAGE, killReason);
        TAG_LOGI(AAFwkTag::APPDFR, "hisysevent write result=%{public}d, send event [FRAMEWORK,PROCESS_KILL],"
            " pid=%{public}d, processName=%{public}s, msg=%{public}s", result, pid, processName.c_str(),
            killReason.c_str());
    }
}

int AppfreezeInner::AcquireStack(const FaultData& info, bool onlyMainThread)
{
    HITRACE_METER_FMT(HITRACE_TAG_APP, "AppfreezeInner::AcquireStack name:%s", info.errorObject.name.c_str());
    std::string stack = "";
    std::string msgContent;

    auto mainHandler = appMainHandler_.lock();
    if (mainHandler == nullptr) {
        msgContent += "mainHandler is destructed!";
    } else {
        MainHandlerDumper handlerDumper;
        msgContent += "mainHandler dump is:\n";
        mainHandler->Dump(handlerDumper);
        msgContent += handlerDumper.GetDumpInfo();
    }

    std::lock_guard<std::mutex> lock(handlingMutex_);
    for (auto it = handlinglist_.begin(); it != handlinglist_.end(); it = handlinglist_.erase(it)) {
        HITRACE_METER_FMT(HITRACE_TAG_APP, "send appfreeze name:%s", it->errorObject.name.c_str());
        FaultData faultData;
        faultData.errorObject.message = it->errorObject.message + "\n";
        if (it->state != 0) {
            FreezeUtil::LifecycleFlow flow = { it->token, static_cast<FreezeUtil::TimeoutState>(it->state) };
            faultData.errorObject.message += "client:\n" +
                FreezeUtil::GetInstance().GetLifecycleEvent(flow) + "\n";
        }
        faultData.errorObject.message += msgContent;
        faultData.errorObject.stack = stack;
        faultData.errorObject.name = it->errorObject.name;
        faultData.faultType = FaultDataType::APP_FREEZE;
        faultData.timeoutMarkers = it->timeoutMarkers;
        faultData.notifyApp = false;
        faultData.waitSaveState = false;
        faultData.forceExit = false;
        faultData.eventId = it->eventId;
        bool isExit = IsExitApp(it->errorObject.name);
        if (isExit) {
            faultData.forceExit = true;
            faultData.waitSaveState = AppRecovery::GetInstance().IsEnabled();
            AAFwk::ExitReason exitReason = {REASON_APP_FREEZE, "Kill Reason:" + faultData.errorObject.name};
            AbilityManagerClient::GetInstance()->RecordAppExitReason(exitReason);
            SendProcessKillEvent("Kill Reason:" + faultData.errorObject.name);
        }
        NotifyANR(faultData);
        if (isExit) {
            AppFreezeRecovery();
        }
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
        TAG_LOGE(AAFwkTag::APPDFR, "NotifyANR fail, applicationInfo_ is nullptr.");
        return -1;
    }

    int32_t pid = static_cast<int32_t>(getpid());
    TAG_LOGI(AAFwkTag::APPDFR, "Start NotifyAppFault:%{public}s, pid:%{public}d, bundleName:%{public}s.",
        faultData.errorObject.name.c_str(), pid, applicationInfo->bundleName.c_str());

    int ret = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->NotifyAppFault(faultData);
    if (ret != 0) {
        TAG_LOGW(AAFwkTag::APPDFR, "NotifyAppFault ret :%{public}d", ret);
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