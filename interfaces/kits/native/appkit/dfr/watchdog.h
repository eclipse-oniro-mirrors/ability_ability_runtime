/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_WATCHDOG_H
#define OHOS_ABILITY_RUNTIME_WATCHDOG_H

#include <string>
#include <mutex>
#include <thread>
#include "event_handler.h"
#include "inner_event.h"
#include "application_impl.h"
#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
#include "hitrace/hitracechain.h"
#endif

namespace OHOS {
namespace AppExecFwk {
constexpr uint32_t INI_TIMER_FIRST_SECOND = 10000;
constexpr const char* MAIN_THREAD_TIMEOUT_TASK = "MAIN_THREAD_TIMEOUT_TASK";
class Watchdog {
public:
    Watchdog();
    ~Watchdog();

    /**
     *
     * @brief Init the Watchdog.
     *
     * @param mainHandler The handler of main thread.
     */
    void Init(const std::shared_ptr<EventHandler> mainHandler);

    /**
     *
     * @brief Stop the mainthread function of watchdog.
     *
     */
    void Stop();

    /**
     *
     * @brief Set the state of main thread.
     *
     * @param appMainThreadState The state of main thread.
     */
    void SetAppMainThreadState(const bool appMainThreadState);

    /**
     *
     * @brief Set whether app is in the background or not
     *
     */
    void SetBackgroundStatus(const bool isInBackground);

    /**
     *
     * @brief Allow report the main thread timeout event.
     *
     */
    void AllowReportEvent();

    /**
     *
     * @brief Get StopWatchdog flag.
     *
     */
    bool IsStopWatchdog();

    /**
     *
     * @brief Check and reset the main thread state.
     *
     */
    bool IsReportEvent();

      /**
     *
     * @brief Change TimeOut.
     *
     */
    void ChangeTimeOut(const std::string& bundleName);

    /**
     *
     * @brief Set bundle info.
     *
     */
    void SetBundleInfo(const std::string& bundleName, const std::string& bundleVersion, bool isSystemApp);

    /**
     *
     * @brief Set whether thread is working in the background or not.
     *
     */
    void SetBgWorkingThreadStatus(const bool isBgWorkingThread);

private:
    void Timer();
    void ReportEvent();
    void SetMainThreadSample();
#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
    void SetHiTraceChainId();

    static OHOS::HiviewDFX::HiTraceId* hitraceId_;
#endif

    std::atomic_bool appMainThreadIsAlive_ = false;
    std::atomic_bool stopWatchdog_ = false;
    std::atomic_bool needReport_ = true;
    std::atomic_bool isSixSecondEvent_ = false;
    std::atomic_bool isInBackground_ = true;
    std::atomic_bool isBgWorkingThread_ = false;
    std::atomic_int backgroundReportCount_ = 0;
    std::atomic_int watchdogReportCount_ = 0;
    std::mutex cvMutex_;
    std::condition_variable cvWatchdog_;
    static std::shared_ptr<EventHandler> appMainHandler_;
    int64_t lastWatchTime_ = 0;
    bool isSystemApp_ = true;
    int64_t lastBackGroundWatchTime_ = 0;
    std::string bundleName_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_WATCHDOG_H
