/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "appfreeze_event_report.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "fault_data.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
// appfreeze event params
constexpr char EVENT_UID[] = "UID";
constexpr char EVENT_PID[] = "PID";
constexpr char EVENT_TID[] = "TID";
constexpr char EVENT_IS_FROZEN[] = "IS_FROZEN";
constexpr char EVENT_INPUT_ID[] = "INPUT_ID";
constexpr char EVENT_MESSAGE[] = "MSG";
constexpr char EVENT_PACKAGE_NAME[] = "PACKAGE_NAME";
constexpr char EVENT_PROCESS_NAME[] = "PROCESS_NAME";
constexpr char EVENT_STACK[] = "STACK";
constexpr char EVENT_BINDER_INFO[] = "BINDER_INFO";
constexpr char EVENT_APP_RUNNING_UNIQUE_ID[] = "APP_RUNNING_UNIQUE_ID";
constexpr char EVENT_FREEZE_MEMORY[] = "FREEZE_MEMORY";
constexpr char EVENT_FREEZE_INFO_PATH[] = "FREEZE_INFO_PATH";
constexpr char EVENT_TRACE_ID[] = "HITRACE_ID";
constexpr char EVENT_FOREGROUND[] = "FOREGROUND";
constexpr char EVENT_ENABLE_MAINTHREAD_SAMPLE[] = "ENABLE_MAINTHREAD_SAMPLE";
constexpr char EVENT_APPLICATION_HEAP_INFO[] = "APPLICATION_HEAP_INFO";
constexpr char EVENT_APPLICATION_GC_INFO[] = "APPLICATION_GC_INFO";
constexpr char EVENT_APPLICATION_IO_INFO[] = "APPLICATION_IO_INFO";
constexpr char EVENT_IS_BLOCK_IN_GC[] = "IS_BLOCK_IN_GC";
constexpr char EVENT_PROCESS_LIFECYCLE_INFO[] = "PROCESS_LIFECYCLE_INFO";
constexpr char EVENT_MAIN_STACK[] = "MAIN_STACK";
constexpr char EVENT_LAST_DISPATCH_EVENTID[] = "LAST_DISPATCH_EVENTID";
constexpr char EVENT_LAST_MARKED_EVENTID[] = "LAST_MARKED_EVENTID";
constexpr char EVENT_LAST_PROCESS_EVENTID[] = "LAST_PROCESS_EVENTID";
constexpr char EVENT_EXTERNAL_LOG[] = "EXTERNAL_LOG";
constexpr char EVENT_REPORT_LIFECYCLE_AS_APPFREEZE [] = "report_lifecycle_as_appfreeze";
}

int AppfreezeEventReport::SendAppfreezeEvent(const std::string &eventName, HiSysEventEventType type,
    const AppfreezeEventInfo &eventInfo)
{
    HITRACE_METER_FMT(HITRACE_TAG_APP, "SendAppfreezeEvent eventName:%{public}s", eventName.c_str());
    int ret = -1;
    if (eventName == AppFreezeType::APP_INPUT_BLOCK) {
        return LogAppInputBlockEvent(eventName, type, eventInfo);
    }
    if (eventName == AppFreezeType::THREAD_BLOCK_3S || eventName == AppFreezeType::THREAD_BLOCK_6S) {
        return LogThreadBlockEvent(eventName, type, eventInfo);
    }
    if (eventName == AppFreezeType::LIFECYCLE_TIMEOUT || eventName == AppFreezeType::LIFECYCLE_HALF_TIMEOUT) {
        return LogLifeCycleTimeoutEvent(eventName, type, eventInfo);
    }
    return LogGeneralEvent(eventName, type, eventInfo);
}

int AppfreezeEventReport::LogAppInputBlockEvent(const std::string &name, HiSysEventEventType type,
    const AppfreezeEventInfo &eventInfo)
{
    int ret = -1;
    auto hisyseventReport = std::make_shared<AAFwk::HisyseventReport>(25);
    hisyseventReport->InsertParam(EVENT_UID, eventInfo.uid);
    hisyseventReport->InsertParam(EVENT_PID, eventInfo.pid);
    hisyseventReport->InsertParam(EVENT_IS_FROZEN, eventInfo.isFrozen);
    hisyseventReport->InsertParam(EVENT_PACKAGE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_PROCESS_NAME, eventInfo.processName);
    hisyseventReport->InsertParam(EVENT_MESSAGE, eventInfo.errorMessage);
    hisyseventReport->InsertParam(EVENT_STACK, eventInfo.errorStack);
    hisyseventReport->InsertParam(EVENT_BINDER_INFO, eventInfo.binderInfo);
    hisyseventReport->InsertParam(EVENT_APP_RUNNING_UNIQUE_ID, eventInfo.appRunningUniqueId);
    hisyseventReport->InsertParam(EVENT_INPUT_ID, eventInfo.eventId);
    hisyseventReport->InsertParam(EVENT_FREEZE_MEMORY, eventInfo.freezeMemory);
    hisyseventReport->InsertParam(EVENT_MAIN_STACK, eventInfo.mainStack);
    hisyseventReport->InsertParam(EVENT_ENABLE_MAINTHREAD_SAMPLE, eventInfo.enableFreeze);
    hisyseventReport->InsertParam(EVENT_FOREGROUND, eventInfo.foregroundState);
    hisyseventReport->InsertParam(EVENT_FREEZE_INFO_PATH, eventInfo.freezeInfoFile);
    hisyseventReport->InsertParam(EVENT_APPLICATION_HEAP_INFO, eventInfo.applicationHeapInfo);
    hisyseventReport->InsertParam(EVENT_APPLICATION_GC_INFO, eventInfo.applicationGCInfo);
    hisyseventReport->InsertParam(EVENT_APPLICATION_IO_INFO, eventInfo.applicationIOInfo);
    hisyseventReport->InsertParam(EVENT_IS_BLOCK_IN_GC, eventInfo.isBlockInGc);
    hisyseventReport->InsertParam(EVENT_PROCESS_LIFECYCLE_INFO, eventInfo.processLifeTime);
    hisyseventReport->InsertParam(EVENT_LAST_DISPATCH_EVENTID, std::to_string(eventInfo.dispatchedEventId));
    hisyseventReport->InsertParam(EVENT_LAST_PROCESS_EVENTID, std::to_string(eventInfo.processedId));
    hisyseventReport->InsertParam(EVENT_LAST_MARKED_EVENTID, std::to_string(eventInfo.markedId));
    hisyseventReport->InsertParam(EVENT_EXTERNAL_LOG, eventInfo.externalLog);
    ret = hisyseventReport->Report("AAFWK", name.c_str(), type);
    return ret;
}

int AppfreezeEventReport::LogThreadBlockEvent(const std::string &name, HiSysEventEventType type,
    const AppfreezeEventInfo &eventInfo)
{
    int ret = -1;
    auto hisyseventReport = std::make_shared<AAFwk::HisyseventReport>(25);
    hisyseventReport->InsertParam(EVENT_UID, eventInfo.uid);
    hisyseventReport->InsertParam(EVENT_PID, eventInfo.pid);
    hisyseventReport->InsertParam(EVENT_TID, eventInfo.tid);
    hisyseventReport->InsertParam(EVENT_IS_FROZEN, eventInfo.isFrozen);
    hisyseventReport->InsertParam(EVENT_PACKAGE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_PROCESS_NAME, eventInfo.processName);
    hisyseventReport->InsertParam(EVENT_MESSAGE, eventInfo.errorMessage);
    hisyseventReport->InsertParam(EVENT_STACK, eventInfo.errorStack);
    hisyseventReport->InsertParam(EVENT_BINDER_INFO, eventInfo.binderInfo);
    hisyseventReport->InsertParam(EVENT_APP_RUNNING_UNIQUE_ID, eventInfo.appRunningUniqueId);
    hisyseventReport->InsertParam(EVENT_FREEZE_MEMORY, eventInfo.freezeMemory);
    hisyseventReport->InsertParam(EVENT_MAIN_STACK, eventInfo.mainStack);
    hisyseventReport->InsertParam(EVENT_FREEZE_INFO_PATH, eventInfo.freezeInfoFile);
    hisyseventReport->InsertParam(EVENT_ENABLE_MAINTHREAD_SAMPLE, eventInfo.enableFreeze);
    hisyseventReport->InsertParam(EVENT_FOREGROUND, eventInfo.foregroundState);
    hisyseventReport->InsertParam(EVENT_APPLICATION_HEAP_INFO, eventInfo.applicationHeapInfo);
    hisyseventReport->InsertParam(EVENT_APPLICATION_GC_INFO, eventInfo.applicationGCInfo);
    hisyseventReport->InsertParam(EVENT_APPLICATION_IO_INFO, eventInfo.applicationIOInfo);
    hisyseventReport->InsertParam(EVENT_IS_BLOCK_IN_GC, eventInfo.isBlockInGc);
    hisyseventReport->InsertParam(EVENT_PROCESS_LIFECYCLE_INFO, eventInfo.processLifeTime);
    hisyseventReport->InsertParam(EVENT_EXTERNAL_LOG, eventInfo.externalLog);
    if (name == AppFreezeType::THREAD_BLOCK_6S) {
        hisyseventReport->InsertParam(EVENT_TRACE_ID, eventInfo.hitraceInfo);
    }
    ret = hisyseventReport->Report("AAFWK", name.c_str(), type);
    return ret;
}

int AppfreezeEventReport::LogLifeCycleTimeoutEvent(const std::string &name, HiSysEventEventType type,
    const AppfreezeEventInfo &eventInfo)
{
    int ret = -1;
    auto hisyseventReport = std::make_shared<AAFwk::HisyseventReport>(25);
    hisyseventReport->InsertParam(EVENT_UID, eventInfo.uid);
    hisyseventReport->InsertParam(EVENT_PID, eventInfo.pid);
    hisyseventReport->InsertParam(EVENT_TID, eventInfo.tid);
    hisyseventReport->InsertParam(EVENT_IS_FROZEN, eventInfo.isFrozen);
    hisyseventReport->InsertParam(EVENT_PACKAGE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_PROCESS_NAME, eventInfo.processName);
    hisyseventReport->InsertParam(EVENT_MESSAGE, eventInfo.errorMessage);
    hisyseventReport->InsertParam(EVENT_STACK, eventInfo.errorStack);
    hisyseventReport->InsertParam(EVENT_BINDER_INFO, eventInfo.binderInfo);
    hisyseventReport->InsertParam(EVENT_APP_RUNNING_UNIQUE_ID, eventInfo.appRunningUniqueId);
    hisyseventReport->InsertParam(EVENT_FREEZE_MEMORY, eventInfo.freezeMemory);
    hisyseventReport->InsertParam(EVENT_MAIN_STACK, eventInfo.mainStack);
    hisyseventReport->InsertParam(EVENT_FREEZE_INFO_PATH, eventInfo.freezeInfoFile);
    hisyseventReport->InsertParam(EVENT_ENABLE_MAINTHREAD_SAMPLE, eventInfo.enableFreeze);
    hisyseventReport->InsertParam(EVENT_EXTERNAL_LOG, eventInfo.externalLog);
    hisyseventReport->InsertParam(EVENT_REPORT_LIFECYCLE_AS_APPFREEZE, eventInfo.reportLifecycleToFreeze);
    hisyseventReport->InsertParam(EVENT_FOREGROUND, eventInfo.foregroundState);
    hisyseventReport->InsertParam(EVENT_APPLICATION_HEAP_INFO, eventInfo.applicationHeapInfo);
    hisyseventReport->InsertParam(EVENT_APPLICATION_GC_INFO, eventInfo.applicationGCInfo);
    hisyseventReport->InsertParam(EVENT_APPLICATION_IO_INFO, eventInfo.applicationIOInfo);
    hisyseventReport->InsertParam(EVENT_IS_BLOCK_IN_GC, eventInfo.isBlockInGc);
    hisyseventReport->InsertParam(EVENT_PROCESS_LIFECYCLE_INFO, eventInfo.processLifeTime);
    ret = hisyseventReport->Report("AAFWK", name.c_str(), type);
    return ret;
}

int AppfreezeEventReport::LogGeneralEvent(const std::string &name, HiSysEventEventType type,
    const AppfreezeEventInfo &eventInfo)
{
    int ret = -1;
    auto hisyseventReport = std::make_shared<AAFwk::HisyseventReport>(20);
    hisyseventReport->InsertParam(EVENT_UID, eventInfo.uid);
    hisyseventReport->InsertParam(EVENT_PID, eventInfo.pid);
    hisyseventReport->InsertParam(EVENT_TID, eventInfo.tid);
    hisyseventReport->InsertParam(EVENT_IS_FROZEN, eventInfo.isFrozen);
    hisyseventReport->InsertParam(EVENT_PACKAGE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_PROCESS_NAME, eventInfo.processName);
    hisyseventReport->InsertParam(EVENT_MESSAGE, eventInfo.errorMessage);
    hisyseventReport->InsertParam(EVENT_STACK, eventInfo.errorStack);
    hisyseventReport->InsertParam(EVENT_BINDER_INFO, eventInfo.binderInfo);
    hisyseventReport->InsertParam(EVENT_APP_RUNNING_UNIQUE_ID, eventInfo.appRunningUniqueId);
    hisyseventReport->InsertParam(EVENT_FREEZE_MEMORY, eventInfo.freezeMemory);
    hisyseventReport->InsertParam(EVENT_FREEZE_INFO_PATH, eventInfo.freezeInfoFile);
    hisyseventReport->InsertParam(EVENT_ENABLE_MAINTHREAD_SAMPLE, eventInfo.enableFreeze);
    hisyseventReport->InsertParam(EVENT_EXTERNAL_LOG, eventInfo.externalLog);
    ret = hisyseventReport->Report("AAFWK", name.c_str(), type);
    return ret;
}
}  // namespace AppExecFwk
}  // namespace OHOS
