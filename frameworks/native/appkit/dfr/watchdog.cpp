/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "watchdog.h"

#include <parameter.h>
#include <unistd.h>

#include "app_mgr_client.h"
#include "app_recovery.h"
#include "appfreeze_inner.h"
#include "hisysevent.h"
#include "hilog_tag_wrapper.h"
#include "xcollie/watchdog.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr uint32_t CHECK_MAIN_THREAD_IS_ALIVE = 1;
constexpr int RESET_RATIO = 2;

constexpr int32_t BACKGROUND_REPORT_COUNT_MAX = 5;
#ifdef SUPPORT_ASAN
constexpr uint32_t CHECK_INTERVAL_TIME = 45000;
#else
constexpr uint32_t CHECK_INTERVAL_TIME = 3000;
#endif
}
std::shared_ptr<EventHandler> Watchdog::appMainHandler_ = nullptr;

Watchdog::Watchdog()
{}

Watchdog::~Watchdog()
{
    if (!stopWatchdog_) {
        TAG_LOGD(AAFwkTag::APPDFR, "Stop watchdog when deconstruct.");
        OHOS::HiviewDFX::Watchdog::GetInstance().StopWatchdog();
    }
}

void Watchdog::Init(const std::shared_ptr<EventHandler> mainHandler)
{
    std::unique_lock<std::mutex> lock(cvMutex_);
    Watchdog::appMainHandler_ = mainHandler;
    if (appMainHandler_ != nullptr) {
        TAG_LOGD(AAFwkTag::APPDFR, "Watchdog init send event");
        appMainHandler_->SendEvent(CHECK_MAIN_THREAD_IS_ALIVE, 0, EventQueue::Priority::HIGH);
    }
    lastWatchTime_ = 0;
    auto watchdogTask = [this] { this->Timer(); };
    OHOS::HiviewDFX::Watchdog::GetInstance().RunPeriodicalTask("AppkitWatchdog", watchdogTask,
        CHECK_INTERVAL_TIME, INI_TIMER_FIRST_SECOND);
}

void Watchdog::Stop()
{
    TAG_LOGD(AAFwkTag::APPDFR, "Watchdog is stop!");
    std::unique_lock<std::mutex> lock(cvMutex_);
    if (stopWatchdog_) {
        TAG_LOGD(AAFwkTag::APPDFR, "Watchdog has stoped.");
        return;
    }
    stopWatchdog_.store(true);
    cvWatchdog_.notify_all();
    OHOS::HiviewDFX::Watchdog::GetInstance().StopWatchdog();

    if (appMainHandler_) {
        appMainHandler_.reset();
        appMainHandler_ = nullptr;
    }
}

void Watchdog::SetAppMainThreadState(const bool appMainThreadState)
{
    std::unique_lock<std::mutex> lock(cvMutex_);
    appMainThreadIsAlive_.store(appMainThreadState);
}

void Watchdog::SetBundleInfo(const std::string& bundleName, const std::string& bundleVersion)
{
    OHOS::HiviewDFX::Watchdog::GetInstance().SetBundleInfo(bundleName, bundleVersion);
}

void Watchdog::SetBackgroundStatus(const bool isInBackground)
{
    std::unique_lock<std::mutex> lock(cvMutex_);
    isInBackground_.store(isInBackground);
    OHOS::HiviewDFX::Watchdog::GetInstance().SetForeground(!isInBackground);
}

void Watchdog::AllowReportEvent()
{
    std::unique_lock<std::mutex> lock(cvMutex_);
    needReport_.store(true);
    isSixSecondEvent_.store(false);
    backgroundReportCount_.store(0);
}

bool Watchdog::IsReportEvent()
{
    if (appMainThreadIsAlive_) {
        appMainThreadIsAlive_.store(false);
        return false;
    }
    TAG_LOGD(AAFwkTag::APPDFR, "AppMainThread is not alive");
    return true;
}

bool Watchdog::IsStopWatchdog()
{
    std::unique_lock<std::mutex> lock(cvMutex_);
    return stopWatchdog_;
}

void Watchdog::SetBgWorkingThreadStatus(const bool isBgWorkingThread)
{
    std::unique_lock<std::mutex> lock(cvMutex_);
    isBgWorkingThread_.store(isBgWorkingThread);
}

void Watchdog::Timer()
{
    std::unique_lock<std::mutex> lock(cvMutex_);
    if (stopWatchdog_) {
        TAG_LOGD(AAFwkTag::APPDFR, "Watchdog has stoped.");
        return;
    }
    if (!needReport_) {
        TAG_LOGE(AAFwkTag::APPDFR, "Watchdog timeout, wait for the handler to recover, and do not send event.");
        return;
    }

    if (IsReportEvent()) {
        const int bufferLen = 128;
        char paramOutBuf[bufferLen] = {0};
        const char *hook_mode = "startup:";
        int ret = GetParameter("libc.hook_mode", "", paramOutBuf, bufferLen);
        if (ret <= 0 || strncmp(paramOutBuf, hook_mode, strlen(hook_mode)) != 0) {
            ReportEvent();
        }
    }
    if (appMainHandler_ != nullptr) {
        appMainHandler_->SendEvent(CHECK_MAIN_THREAD_IS_ALIVE, 0, EventQueue::Priority::HIGH);
    }
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
    if ((now - lastWatchTime_) >= (CHECK_INTERVAL_TIME / RESET_RATIO)) {
        lastWatchTime_ = now;
    }
}

void Watchdog::ReportEvent()
{
    if (isBgWorkingThread_) {
        TAG_LOGD(AAFwkTag::APPDFR, "Thread is working in the background, do not report this time");
        return;
    }
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
    if ((now - lastWatchTime_) > (RESET_RATIO * CHECK_INTERVAL_TIME) ||
        (now - lastWatchTime_) < (CHECK_INTERVAL_TIME / RESET_RATIO)) {
        TAG_LOGI(AAFwkTag::APPDFR,
            "Thread may be blocked, do not report this time. currTime: %{public}llu, lastTime: %{public}llu",
            static_cast<unsigned long long>(now), static_cast<unsigned long long>(lastWatchTime_));
        return;
    }

    if (isInBackground_ && backgroundReportCount_.load() < BACKGROUND_REPORT_COUNT_MAX) {
        TAG_LOGI(AAFwkTag::APPDFR, "In Background, thread may be blocked in, do not report this time. "
            "currTime: %{public}" PRIu64 ", lastTime: %{public}" PRIu64 "",
            static_cast<uint64_t>(now), static_cast<uint64_t>(lastWatchTime_));
        backgroundReportCount_++;
        return;
    }
    backgroundReportCount_++;

    if (!needReport_) {
        return;
    }

#ifndef APP_NO_RESPONSE_DIALOG
    if (isSixSecondEvent_) {
        needReport_.store(false);
    }
#endif
    AppExecFwk::AppfreezeInner::GetInstance()->ThreadBlock(isSixSecondEvent_);
}
}  // namespace AppExecFwk
}  // namespace OHOS
