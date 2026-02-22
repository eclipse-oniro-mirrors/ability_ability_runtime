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
#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
constexpr int32_t CHARACTER_WIDTH = 2;
#endif

constexpr int32_t BACKGROUND_REPORT_COUNT_MAX = 5;
constexpr int32_t WATCHDOG_REPORT_COUNT_MAX = 5;
#ifdef SUPPORT_ASAN
constexpr uint32_t CHECK_INTERVAL_TIME = 45000;
#else
constexpr uint32_t CHECK_INTERVAL_TIME = 3000;
#endif

#ifdef APP_NO_RESPONSE_DIALOG_WEARABLE
constexpr uint32_t WEARABLE_CHECK_INTERVAL_TIME = 5000;
#endif

constexpr uint32_t BACKGROUND_INTERVAL_TIME = 20000;
}
std::shared_ptr<EventHandler> Watchdog::appMainHandler_ = nullptr;

#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
OHOS::HiviewDFX::HiTraceId* Watchdog::hitraceId_ = nullptr;
#endif

static constexpr const char* const CHECK_BACKGROUND_THREAD[] = {
    "com.ohos.sceneboard"
};

Watchdog::Watchdog()
{}

Watchdog::~Watchdog()
{
    if (!stopWatchdog_) {
        TAG_LOGD(AAFwkTag::APPDFR, "Stop watchdog");
        OHOS::HiviewDFX::Watchdog::GetInstance().StopWatchdog();
    }
}
void Watchdog::Init(const std::shared_ptr<EventHandler> mainHandler)
{
    std::unique_lock<std::mutex> lock(cvMutex_);
    Watchdog::appMainHandler_ = mainHandler;
    if (appMainHandler_ != nullptr) {
        TAG_LOGD(AAFwkTag::APPDFR, "Watchdog init send event");
        appMainHandler_->SendEvent(CHECK_MAIN_THREAD_IS_ALIVE, 0, EventQueue::Priority::VIP);
    }
    lastWatchTime_ = 0;
#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
    hitraceId_ = OHOS::HiviewDFX::HiTraceChain::GetIdAddress();
#endif
    auto watchdogTask = [this] { this->Timer(); };
#ifdef APP_NO_RESPONSE_DIALOG_WEARABLE
    OHOS::HiviewDFX::Watchdog::GetInstance().RunPeriodicalTask("AppkitWatchdog", watchdogTask,
        WEARABLE_CHECK_INTERVAL_TIME, INI_TIMER_FIRST_SECOND);
#else
    OHOS::HiviewDFX::Watchdog::GetInstance().RunPeriodicalTask("AppkitWatchdog", watchdogTask,
        CHECK_INTERVAL_TIME, INI_TIMER_FIRST_SECOND);
#endif
    SetMainThreadSample();
}

void Watchdog::SetMainThreadSample()
{
    char* env = getenv("DFX_APPFREEZE_LOG_OPTIONS");
    if (env == nullptr) {
        return;
    }
    AppExecFwk::AppfreezeInner::GetInstance()->SetMainThreadSample(
        strstr(env, "mainthread_sampling:enable") != nullptr);
}

void Watchdog::Stop()
{
    TAG_LOGD(AAFwkTag::APPDFR, "called");
    std::unique_lock<std::mutex> lock(cvMutex_);
    if (stopWatchdog_) {
        TAG_LOGD(AAFwkTag::APPDFR, "stoped");
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

#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
void Watchdog::SetHiTraceChainId()
{
    OHOS::HiviewDFX::HiTraceId hitraceId = *hitraceId_;
    if (hitraceId.IsValid() == 0) {
        TAG_LOGW(AAFwkTag::APPDFR, "get hitrace id is invalid.");
        return;
    }
    OHOS::HiviewDFX::HiTraceChain::SetId(hitraceId);
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(CHARACTER_WIDTH) << hitraceId.GetChainId();
    TAG_LOGI(AAFwkTag::APPDFR, "Main thread blocked, %{public}s", ss.str().c_str());
}
#endif

#ifdef APP_NO_RESPONSE_DIALOG
bool isDeviceType2in1()
{
    const int bufferLen = 128;
    char paramOutBuf[bufferLen] = {0};
    const char *devicetype2in1 = "2in1";
    int ret = GetParameter("const.product.devicetype", "", paramOutBuf, bufferLen);
    return ret > 0 && strncmp(paramOutBuf, devicetype2in1, strlen(devicetype2in1)) == 0;
}
#endif

#ifdef APP_NO_RESPONSE_DIALOG
void Watchdog::ChangeTimeOut(const std::string& bundleName)
{
    constexpr char SCENEBOARD_SERVICE_ABILITY[] = "com.ohos.sceneboard";
    constexpr int TIMEOUT = 5000;
    if (bundleName == SCENEBOARD_SERVICE_ABILITY) {
        OHOS::HiviewDFX::Watchdog::GetInstance().RemovePeriodicalTask("AppkitWatchdog");
        auto watchdogTask = [this] { this->Timer(); };
        OHOS::HiviewDFX::Watchdog::GetInstance().RunPeriodicalTask("AppkitWatchdog", watchdogTask, TIMEOUT,
            INI_TIMER_FIRST_SECOND);
    }
}
#endif

void Watchdog::SetBundleInfo(const std::string& bundleName, const std::string& bundleVersion, bool isSystemApp)
{
    OHOS::HiviewDFX::Watchdog::GetInstance().SetSystemApp(isSystemApp);
    OHOS::HiviewDFX::Watchdog::GetInstance().SetBundleInfo(bundleName, bundleVersion);
#ifdef APP_NO_RESPONSE_DIALOG
    if (isDeviceType2in1()) {
        ChangeTimeOut(bundleName);
    }
#endif
    {
        std::unique_lock<std::mutex> lock(cvMutex_);
        bundleName_ = bundleName;
    }
}

void Watchdog::SetBackgroundStatus(const bool isInBackground)
{
    std::unique_lock<std::mutex> lock(cvMutex_);
    isInBackground_.store(isInBackground);
    OHOS::HiviewDFX::Watchdog::GetInstance().SetForeground(!isInBackground);
    AppExecFwk::AppfreezeInner::GetInstance()->SetAppInForeground(!isInBackground);
}

void Watchdog::AllowReportEvent()
{
    std::unique_lock<std::mutex> lock(cvMutex_);
    needReport_.store(true);
    isSixSecondEvent_.store(false);
    backgroundReportCount_.store(0);
    watchdogReportCount_.store(0);
}

bool Watchdog::IsReportEvent()
{
    if (appMainThreadIsAlive_) {
        appMainThreadIsAlive_.store(false);
        return false;
    }
    TAG_LOGD(AAFwkTag::APPDFR, "AppMainThread not alive");
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
        TAG_LOGD(AAFwkTag::APPDFR, "stoped");
        return;
    }
    if (!needReport_) {
        watchdogReportCount_++;
        TAG_LOGE(AAFwkTag::APPDFR, "wait count: %{public}d", watchdogReportCount_.load());
        if (watchdogReportCount_.load() >= WATCHDOG_REPORT_COUNT_MAX) {
#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
            SetHiTraceChainId();
#endif
#ifndef APP_NO_RESPONSE_DIALOG
            AppExecFwk::AppfreezeInner::GetInstance()->AppfreezeHandleOverReportCount(true);
#endif
            watchdogReportCount_.store(0);
        } else if (watchdogReportCount_.load() >= (WATCHDOG_REPORT_COUNT_MAX - 1)) {
#ifndef APP_NO_RESPONSE_DIALOG
            AppExecFwk::AppfreezeInner::GetInstance()->AppfreezeHandleOverReportCount(false);
#endif
        }
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
        appMainHandler_->SendEvent(CHECK_MAIN_THREAD_IS_ALIVE, 0, EventQueue::Priority::VIP);
    }
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
#ifdef APP_NO_RESPONSE_DIALOG_WEARABLE
    if ((now - lastWatchTime_) < 0 || (now - lastWatchTime_) >= (WEARABLE_CHECK_INTERVAL_TIME / RESET_RATIO)) {
#else
    if ((now - lastWatchTime_) < 0 || (now - lastWatchTime_) >= (CHECK_INTERVAL_TIME / RESET_RATIO)) {
#endif
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
#ifdef APP_NO_RESPONSE_DIALOG_WEARABLE
    if ((now - lastWatchTime_) > (RESET_RATIO * WEARABLE_CHECK_INTERVAL_TIME) ||
        (now - lastWatchTime_) < (WEARABLE_CHECK_INTERVAL_TIME / RESET_RATIO)) {
#else
    if ((now - lastWatchTime_) > (RESET_RATIO * CHECK_INTERVAL_TIME) ||
        (now - lastWatchTime_) < (CHECK_INTERVAL_TIME / RESET_RATIO)) {
#endif
        TAG_LOGI(AAFwkTag::APPDFR,
            "Thread may be blocked, not report time. currTime: %{public}llu, lastTime: %{public}llu",
            static_cast<unsigned long long>(now), static_cast<unsigned long long>(lastWatchTime_));
        return;
    }

    if (isInBackground_ && backgroundReportCount_.load() < BACKGROUND_REPORT_COUNT_MAX) {
        bool enableCheck = std::find(std::begin(CHECK_BACKGROUND_THREAD), std::end(CHECK_BACKGROUND_THREAD),
            bundleName_) != std::end(CHECK_BACKGROUND_THREAD);
        if (enableCheck) {
            backgroundReportCount_++;
        }
        if ((now - lastBackGroundWatchTime_) < 0 || (now - lastBackGroundWatchTime_) >= (BACKGROUND_INTERVAL_TIME)) {
            TAG_LOGW(AAFwkTag::APPDFR, "In Background, thread may be blocked in, not report time"
                "currTime: %{public}" PRIu64 ", lastTime: %{public}" PRIu64 ", enableCheck: %{public}d",
                static_cast<uint64_t>(now), static_cast<uint64_t>(lastWatchTime_), enableCheck);
            lastBackGroundWatchTime_ = now;
        }
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

#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
    if (isSixSecondEvent_) {
        SetHiTraceChainId();
    }
#endif
    AppExecFwk::AppfreezeInner::GetInstance()->ThreadBlock(isSixSecondEvent_, 0, now, isInBackground_);
}
}  // namespace AppExecFwk
}  // namespace OHOS
