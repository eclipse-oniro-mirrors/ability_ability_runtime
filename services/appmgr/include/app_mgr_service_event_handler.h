/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_MGR_SERVICE_EVENT_HANDLER_H
#define OHOS_ABILITY_RUNTIME_APP_MGR_SERVICE_EVENT_HANDLER_H

#include <list>
#include <mutex>

#include "event_handler_wrap.h"

namespace OHOS {
namespace AppExecFwk {
class AppMgrServiceInner;

class AMSEventHandler : public AAFwk::EventHandlerWrap {
public:
    AMSEventHandler(const std::shared_ptr<AAFwk::TaskHandlerWrap> &taskHandler,
        const std::weak_ptr<AppMgrServiceInner> &appMgr);
    virtual ~AMSEventHandler() override;

    virtual void ProcessEvent(const AAFwk::EventWrap &event) override;

    static constexpr uint32_t TERMINATE_ABILITY_TIMEOUT_MSG = 0;
    static constexpr uint32_t TERMINATE_APPLICATION_TIMEOUT_MSG = 1;
    static constexpr uint32_t ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG = 2;
    static constexpr uint32_t START_SPECIFIED_ABILITY_TIMEOUT_MSG = 3;
    static constexpr uint32_t START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG = 4;
    static constexpr uint32_t START_SPECIFIED_PROCESS_TIMEOUT_MSG = 5;

    static constexpr uint32_t TERMINATE_ABILITY_HALF_TIMEOUT_MSG = 6;
    static constexpr uint32_t TERMINATE_APPLICATION_HALF_TIMEOUT_MSG = 7;
    static constexpr uint32_t ADD_ABILITY_STAGE_INFO_HALF_TIMEOUT_MSG = 8;
    static constexpr uint32_t START_SPECIFIED_ABILITY_HALF_TIMEOUT_MSG = 9;
    static constexpr uint32_t START_PROCESS_SPECIFIED_ABILITY_HALF_TIMEOUT_MSG = 10;
    static constexpr uint32_t START_SPECIFIED_PROCESS_HALF_TIMEOUT_MSG = 11;
    static constexpr uint32_t RENDER_ATTACH_TIMEOUT_MSG = 12;
    static constexpr uint32_t CHILD_PROCESS_ATTACH_TIMEOUT_MSG = 13;
    static constexpr uint32_t PRELOAD_ATTACH_TIMEOUT_MSG = 14;
#ifdef SUPPORT_ASAN
    static constexpr int32_t TERMINATE_ABILITY_TIMEOUT = 45000; // ms
    static constexpr int32_t TERMINATE_APPLICATION_TIMEOUT = 150000; // ms
    static constexpr int32_t BACKGROUND_APPLICATION_TIMEOUT = 45000; // ms
    static constexpr int32_t ADD_ABILITY_STAGE_INFO_TIMEOUT = 45000; // ms
    static constexpr int32_t ADD_ABILITY_STAGE_EMPTY_RESIDENT_TIMEOUT = 100000; // ms
    static constexpr int32_t START_SPECIFIED_ABILITY_TIMEOUT = 45000; // ms
    static constexpr int32_t START_PROCESS_SPECIFIED_ABILITY_TIMEOUT = 75000; // ms
    static constexpr int32_t START_SPECIFIED_PROCESS_TIMEOUT = 45000; // ms
    static constexpr int32_t KILL_PROCESS_TIMEOUT = 45000; // ms
    static constexpr int32_t KILL_PROCESS_TIMEOUT_DELAY = 45000; // ms
#else
    static constexpr int32_t TERMINATE_ABILITY_TIMEOUT = 3000; // ms
    static constexpr int32_t TERMINATE_APPLICATION_TIMEOUT = 10000; // ms
    static constexpr int32_t BACKGROUND_APPLICATION_TIMEOUT = 3000; // ms
    static constexpr int32_t ADD_ABILITY_STAGE_INFO_TIMEOUT = 3000; // ms
    static constexpr int32_t ADD_ABILITY_STAGE_EMPTY_RESIDENT_TIMEOUT = 20000; // ms
    static constexpr int32_t START_SPECIFIED_ABILITY_TIMEOUT = 3000; // ms
    static constexpr int32_t START_PROCESS_SPECIFIED_ABILITY_TIMEOUT = 5000; // ms
    static constexpr int32_t START_SPECIFIED_PROCESS_TIMEOUT = 2000; // ms
    static constexpr int32_t KILL_PROCESS_TIMEOUT = 3000; // ms
    static constexpr int32_t KILL_PROCESS_TIMEOUT_DELAY = 6000; // ms
#endif
    static constexpr int32_t DELAY_KILL_PROCESS_TIMEOUT = 3000; // ms
    static constexpr int32_t DELAY_KILL_EXTENSION_PROCESS_TIMEOUT = 500; // ms
    static constexpr int32_t DELAY_NOTIFY_PROCESS_CACHED_STATE = 2000; // ms
    static constexpr int32_t DELAY_CHECK_ALL_PROCESSES_EXITED = 200; // ms
private:
    std::weak_ptr<AppMgrServiceInner> appMgr_;
};

class AppRunningRecord;
struct AppEventData {
    uint32_t eventId = 0;
    int64_t param = 0;
    std::weak_ptr<AppRunningRecord> appRecord;

    AppEventData(uint32_t eventId, int64_t param, std::shared_ptr<AppRunningRecord> appRecord)
        : eventId(eventId), param(param), appRecord(appRecord) {}
};

class AppEventUtil {
public:
    static AppEventUtil &GetInstance();

    AppEventUtil() = default;
    AppEventUtil(AppEventUtil &) = delete;
    void operator=(AppEventUtil &) = delete;

    void AddEvent(std::shared_ptr<AppRunningRecord> appRecord, uint32_t eventId, int64_t param);
    bool HasEvent(std::shared_ptr<AppRunningRecord> appRecord, uint32_t eventId);
    std::shared_ptr<AppRunningRecord> RemoveEvent(uint32_t eventId, int64_t param);
    std::list<AppEventData> RemoveEvent(std::shared_ptr<AppRunningRecord> appRecord, uint32_t eventId);
private:
    std::mutex appEventListMutex_;
    std::list<AppEventData> appEventList_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_MGR_SERVICE_EVENT_HANDLER_H
