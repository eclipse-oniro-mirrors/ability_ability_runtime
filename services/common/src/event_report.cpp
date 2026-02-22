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

#include "event_report.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "record_cost_time_util.h"

namespace OHOS {
namespace AAFwk {
namespace {
// event params
constexpr const char *EVENT_KEY_APP_PID = "APP_PID";
constexpr const char *EVENT_KEY_USERID = "USER_ID";
constexpr const char *EVENT_KEY_ERROR_CODE = "ERROR_CODE";
constexpr const char *EVENT_KEY_BUNDLE_NAME = "BUNDLE_NAME";
constexpr const char *EVENT_KEY_MODULE_NAME = "MODULE_NAME";
constexpr const char *EVENT_KEY_ABILITY_NAME = "ABILITY_NAME";
constexpr const char *EVENT_KEY_ABILITY_NUMBER = "ABILITY_NUMBER";
constexpr const char *EVENT_KEY_ABILITY_TYPE = "ABILITY_TYPE";
constexpr const char *EVENT_KEY_VERSION_NAME = "VERSION_NAME";
constexpr const char *EVENT_KEY_VERSION_CODE = "VERSION_CODE";
constexpr const char *EVENT_KEY_PROCESS_NAME = "PROCESS_NAME";
constexpr const char *EVENT_KEY_EXTENSION_TYPE = "EXTENSION_TYPE";
constexpr const char *EVENT_KEY_STARTUP_TIME = "STARTUP_TIME";
constexpr const char *EVENT_KEY_STARTUP_ABILITY_TYPE = "STARTUP_ABILITY_TYPE";
constexpr const char *EVENT_KEY_STARTUP_EXTENSION_TYPE = "STARTUP_EXTENSION_TYPE";
constexpr const char *EVENT_KEY_CALLER_BUNDLE_NAME = "CALLER_BUNDLE_NAME";
constexpr const char *EVENT_KEY_WANTAGENT_NUMBER = "GET_WANTAGENT_NUMBER";
constexpr const char *EVENT_KEY_CALLER_UID = "CALLER_UID";
constexpr const char *EVENT_KEY_CALLER_PROCESS_NAME = "CALLER_PROCESS_NAME";
constexpr const char *EVENT_KEY_CALLER_PROCESS_ID = "CALLER_PROCESS_ID";
constexpr const char *EVENT_KEY_EXIT_TIME = "EXIT_TIME";
constexpr const char *EVENT_KEY_EXIT_RESULT = "EXIT_RESULT";
constexpr const char *EVENT_KEY_EXIT_PID = "EXIT_PID";
constexpr const char *EVENT_KEY_EXIT_REASON = "EXIT_REASON";
constexpr const char *EVENT_KEY_BUNDLE_TYPE = "BUNDLE_TYPE";
constexpr const char *EVENT_KEY_START_TYPE = "START_TYPE";
constexpr const char *EVENT_KEY_START_REASON = "START_REASON";
constexpr const char *EVENT_KEY_CALLER_STATE = "CALLER_STATE";
constexpr const char *EVENT_KEY_CALLER_VERSION_NAME = "CALLER_VERSION_NAME";
constexpr const char *EVENT_KEY_CALLER_VERSION_CODE = "CALLER_VERSION_CODE";
constexpr const char *EVENT_KEY_URI = "URI";
constexpr const char *EVENT_KEY_RESTART_TIME = "RESTART_TIME";
constexpr const char *EVENT_KEY_APP_UID = "APP_UID";
constexpr const char *EVENT_KEY_PROCESS_TYPE = "PROCESS_TYPE";
constexpr const char *EVENT_KEY_TIME = "TIME";
constexpr const char *EVENT_KEY_PID = "PID";
constexpr const char *EVENT_KEY_REASON = "REASON";
constexpr const char *EVENT_KEY_SUB_REASON = "SUB_REASON";
constexpr const char *INVALID_EVENT_NAME = "INVALIDEVENTNAME";
constexpr const char *EVENT_KEY_APP_INDEX = "APP_INDEX";
constexpr const char *EVENT_KEY_ERR_REASON = "ERR_REASON";
constexpr const char *EVENT_KEY_LIFE_CYCLE = "LIFE_CYCLE";
constexpr const char *EVENT_KEY_PERSISTENT_ID = "PERSISTENT_ID";
constexpr const char *EVENT_KEY_INTENT_NAME = "INTENT_NAME";
constexpr const char *EVENT_KEY_ERROR_MESSAGE = "ERROR_MESSAGE";
constexpr const char *EVENT_KEY_CALLER_PID = "CALLER_PID";
constexpr const char *EVENT_KEY_EXIT_MESSAGE = "EXIT_MESSAGE";
constexpr const char *EVENT_KEY_SHOULD_KILL_FOREGROUND = "SHOULD_KILL_FOREGROUND";
constexpr const char *EVENT_FILE_OR_FOLDER_PATH = "FILE_OR_FOLDER_PATH";
constexpr const char *EVENT_FILE_OR_FOLDER_SIZE = "FILE_OR_FOLDER_SIZE";
constexpr const char *EVENT_COMPONENT_NAME_KEY = "COMPONENT_NAME";
constexpr const char *EVENT_PARTITION_NAME_KEY = "PARTITION_NAME";
constexpr const char *EVENT_REMAIN_PARTITION_SIZE_KEY = "REMAIN_PARTITION_SIZE";

constexpr const int32_t DEFAULT_EXTENSION_TYPE = -1;
}

void EventReport::SendAppEvent(const EventName &eventName, HiSysEventEventType type, const EventInfo &eventInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    RecordCostTimeUtil timeRecord("SendAppEvent");
    std::string name = ConvertEventName(eventName);
    auto hisyseventReport = std::make_shared<HisyseventReport>(8);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    switch (eventName) {
        case EventName::APP_STARTUP_TYPE:
            hisyseventReport->InsertParam(EVENT_KEY_APP_PID, eventInfo.pid);
            hisyseventReport->InsertParam(EVENT_KEY_VERSION_CODE, eventInfo.versionCode);
            hisyseventReport->InsertParam(EVENT_KEY_VERSION_NAME, eventInfo.versionName);
            hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
            hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
            hisyseventReport->InsertParam(EVENT_KEY_START_TYPE, eventInfo.startType);
            hisyseventReport->InsertParam(EVENT_KEY_START_REASON, eventInfo.startReason);
            hisyseventReport->Report("AAFWK", name.c_str(), type);
            break;
        case EventName::DRAWN_COMPLETED:
            TAG_LOGI(AAFwkTag::DEFAULT,
                "DRAWN_COMPLETED, bundle: %{public}s, ability: %{public}s",
                eventInfo.bundleName.c_str(), eventInfo.abilityName.c_str());
            hisyseventReport->InsertParam(EVENT_KEY_USERID, eventInfo.userId);
            hisyseventReport->InsertParam(EVENT_KEY_APP_PID, eventInfo.pid);
            hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
            hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
            hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
            hisyseventReport->Report("AAFWK", name.c_str(), type);
            break;
        default:
            hisyseventReport->InsertParam(EVENT_KEY_APP_PID, eventInfo.pid);
            hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
            hisyseventReport->InsertParam(EVENT_KEY_VERSION_NAME, eventInfo.versionName);
            hisyseventReport->InsertParam(EVENT_KEY_VERSION_CODE, eventInfo.versionCode);
            hisyseventReport->InsertParam(EVENT_KEY_PROCESS_NAME, eventInfo.processName);
            hisyseventReport->Report("AAFWK", name.c_str(), type);
            break;
    }
}

void EventReport::LogErrorEvent(const std::string &name, HiSysEventEventType type, const EventInfo &eventInfo)
{
    auto hisyseventReport = std::make_shared<HisyseventReport>(8);
    hisyseventReport->InsertParam(EVENT_KEY_USERID, eventInfo.userId);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
    hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    hisyseventReport->InsertParam(EVENT_KEY_ERROR_CODE, eventInfo.errCode);
    hisyseventReport->InsertParam(EVENT_KEY_ERROR_MESSAGE, eventInfo.errMsg);
    hisyseventReport->Report("AAFWK", name.c_str(), type);
}

void EventReport::LogStartErrorEvent(const std::string &name, HiSysEventEventType type, const EventInfo &eventInfo)
{
    auto hisyseventReport = std::make_shared<HisyseventReport>(8);
    hisyseventReport->InsertParam(EVENT_KEY_USERID, eventInfo.userId);
    hisyseventReport->InsertParam(EVENT_KEY_APP_INDEX, eventInfo.appIndex);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
    hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    hisyseventReport->InsertParam(EVENT_KEY_ERROR_CODE, eventInfo.errCode);
    hisyseventReport->InsertParam(EVENT_KEY_ERROR_MESSAGE, eventInfo.errMsg);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName);
    hisyseventReport->Report("AAFWK", name.c_str(), type);
}

void EventReport::LogWantAgentNumberEvent(const std::string &name, HiSysEventEventType type, const EventInfo &eventInfo)
{
    auto hisyseventReport = std::make_shared<HisyseventReport>(3);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_UID, eventInfo.callerUid);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName);
    hisyseventReport->InsertParam(EVENT_KEY_WANTAGENT_NUMBER, eventInfo.wantAgentNumber);
    hisyseventReport->Report("AAFWK", name.c_str(), type);
}

void EventReport::LogTriggerFailedEvent(const std::string &name, HiSysEventEventType type, const EventInfo &eventInfo)
{
    auto hisyseventReport = std::make_shared<HisyseventReport>(9);
    hisyseventReport->InsertParam(EVENT_KEY_USERID, eventInfo.userId);
    hisyseventReport->InsertParam(EVENT_KEY_APP_INDEX, eventInfo.appIndex);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
    hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    hisyseventReport->InsertParam(EVENT_KEY_ERROR_CODE, eventInfo.errCode);
    hisyseventReport->InsertParam(EVENT_KEY_ERROR_MESSAGE, eventInfo.errMsg);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName);
    hisyseventReport->InsertParam(EVENT_KEY_START_TYPE, eventInfo.startType);
    hisyseventReport->Report("AAFWK", name.c_str(), type);
}

void EventReport::LogSystemErrorEvent(const std::string &name, HiSysEventEventType type, const EventInfo &eventInfo)
{
    auto hisyseventReport = std::make_shared<HisyseventReport>(6);
    hisyseventReport->InsertParam(EVENT_KEY_APP_INDEX, eventInfo.appIndex);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
    hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    hisyseventReport->InsertParam(EVENT_KEY_ERROR_CODE, eventInfo.errCode);
    hisyseventReport->InsertParam(EVENT_KEY_ERROR_MESSAGE, eventInfo.errMsg);
    hisyseventReport->Report("AAFWK", name.c_str(), type);
}

void EventReport::LogStartAbilityEvent(const std::string &name, HiSysEventEventType type, const EventInfo &eventInfo)
{
    auto hisyseventReport = std::make_shared<HisyseventReport>(5);
    hisyseventReport->InsertParam(EVENT_KEY_USERID, eventInfo.userId);
    hisyseventReport->InsertParam(EVENT_KEY_APP_INDEX, eventInfo.appIndex);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
    hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    hisyseventReport->Report("AAFWK", name.c_str(), type);
}

void EventReport::LogTerminateAbilityEvent(
    const std::string &name, HiSysEventEventType type, const EventInfo &eventInfo)
{
    auto hisyseventReport = std::make_shared<HisyseventReport>(3);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    hisyseventReport->Report("AAFWK", name.c_str(), type);
}

void EventReport::LogAbilityOnForegroundEvent(
    const std::string &name, HiSysEventEventType type, const EventInfo &eventInfo)
{
    auto hisyseventReport = std::make_shared<HisyseventReport>(7);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
    hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_TYPE, eventInfo.bundleType);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName);
    hisyseventReport->Report("AAFWK", name.c_str(), type);
}

void EventReport::LogAbilityOnBackgroundEvent(
    const std::string &name, HiSysEventEventType type, const EventInfo &eventInfo)
{
    auto hisyseventReport = std::make_shared<HisyseventReport>(6);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
    hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_TYPE, eventInfo.bundleType);
    hisyseventReport->Report("AAFWK", name.c_str(), type);
}

void EventReport::LogAbilityOnActiveEvent(const std::string &name, HiSysEventEventType type, const EventInfo &eventInfo)
{
    auto hisyseventReport = std::make_shared<HisyseventReport>(6);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
    hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    hisyseventReport->InsertParam(EVENT_KEY_ABILITY_TYPE, eventInfo.abilityType);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_TYPE, eventInfo.bundleType);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName);
    hisyseventReport->Report("AAFWK", name.c_str(), type);
}

void EventReport::LogStartStandardEvent(const std::string &name, HiSysEventEventType type, const EventInfo &eventInfo)
{
    TAG_LOGD(AAFwkTag::DEFAULT, "EventInfo: [%{public}d, %{public}s, %{public}s, %{public}s]",
        eventInfo.userId, eventInfo.bundleName.c_str(), eventInfo.moduleName.c_str(),
        eventInfo.abilityName.c_str());
    auto hisyseventReport = std::make_shared<HisyseventReport>(5);
    hisyseventReport->InsertParam(EVENT_KEY_USERID, eventInfo.userId);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
    hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NUMBER, eventInfo.abilityNumber);
    hisyseventReport->Report("AAFWK", name.c_str(), type);
}

void EventReport::LogStartAbilityByAppLinking(
    const std::string &name, HiSysEventEventType type, const EventInfo &eventInfo)
{
    TAG_LOGD(AAFwkTag::DEFAULT, "EventInfo, bundleName: %{public}s, callerBundleName: %{public}s, uri: %{public}s",
        eventInfo.bundleName.c_str(), eventInfo.callerBundleName.c_str(), eventInfo.uri.c_str());
    auto hisyseventReport = std::make_shared<HisyseventReport>(3);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName);
    hisyseventReport->InsertParam(EVENT_KEY_URI, eventInfo.uri);
    auto ret = hisyseventReport->Report("AAFWK", name.c_str(), type);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::DEFAULT, "Write event fail: %{public}s, ret %{public}d", name.c_str(), ret);
    }
}

void EventReport::LogKillProcessWithReason(
    const std::string &name, HiSysEventEventType type, const EventInfo &eventInfo)
{
    auto hisyseventReport = std::make_shared<HisyseventReport>(5);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_PID, eventInfo.callerPid);
    hisyseventReport->InsertParam(EVENT_KEY_PID, eventInfo.pid);
    hisyseventReport->InsertParam(EVENT_KEY_EXIT_MESSAGE, eventInfo.exitMsg);
    hisyseventReport->InsertParam(EVENT_KEY_SHOULD_KILL_FOREGROUND, eventInfo.shouldKillForeground);
    hisyseventReport->InsertParam(EVENT_KEY_ERROR_CODE, eventInfo.errCode);
    hisyseventReport->InsertParam(EVENT_KEY_ERROR_MESSAGE, eventInfo.errMsg);
    hisyseventReport->Report("AAFWK", name.c_str(), type);
}

void EventReport::LogUIExtensionErrorEvent(
    const std::string &name, HiSysEventEventType type, const EventInfo &eventInfo)
{
    auto hisyseventReport = std::make_shared<HisyseventReport>(11);
    hisyseventReport->InsertParam(EVENT_KEY_USERID, eventInfo.userId);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
    hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    hisyseventReport->InsertParam(EVENT_KEY_ERROR_CODE, eventInfo.errCode);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName);
    hisyseventReport->InsertParam(EVENT_KEY_APP_INDEX, eventInfo.appIndex);
    hisyseventReport->InsertParam(EVENT_KEY_ERR_REASON, eventInfo.errReason);
    hisyseventReport->InsertParam(EVENT_KEY_LIFE_CYCLE, eventInfo.lifeCycle);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_UID, eventInfo.callerUid);
    hisyseventReport->InsertParam(EVENT_KEY_PERSISTENT_ID, eventInfo.persistentId);
    hisyseventReport->Report("AAFWK", name.c_str(), type);
}

void EventReport::LogUIServiceExtErrorEvent(
    const std::string &name, HiSysEventEventType type, const EventInfo &eventInfo)
{
    auto hisyseventReport = std::make_shared<HisyseventReport>(10);
    hisyseventReport->InsertParam(EVENT_KEY_USERID, eventInfo.userId);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
    hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    hisyseventReport->InsertParam(EVENT_KEY_ERROR_CODE, eventInfo.errCode);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName);
    hisyseventReport->InsertParam(EVENT_KEY_APP_INDEX, eventInfo.appIndex);
    hisyseventReport->InsertParam(EVENT_KEY_ERR_REASON, eventInfo.errReason);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_UID, eventInfo.callerUid);
    hisyseventReport->InsertParam(EVENT_KEY_LIFE_CYCLE, eventInfo.lifeCycle);
    hisyseventReport->Report("AAFWK", name.c_str(), type);
}

void EventReport::SendAbilityEvent(
    const EventName &eventName, HiSysEventEventType type, const EventInfo &eventInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    switch (eventName) {
        case EventName::START_ABILITY_ERROR:
            LogStartErrorEvent(name, type, eventInfo);
            break;
        case EventName::TERMINATE_ABILITY_ERROR:
            LogErrorEvent(name, type, eventInfo);
            break;
        case EventName::START_ABILITY_SYSTEM_ERROR:
            LogSystemErrorEvent(name, type, eventInfo);
            break;
        case EventName::START_ABILITY:
            LogStartAbilityEvent(name, type, eventInfo);
            break;
        case EventName::TERMINATE_ABILITY:
        case EventName::CLOSE_ABILITY:
            LogTerminateAbilityEvent(name, type, eventInfo);
            break;
        case EventName::ABILITY_ONFOREGROUND:
            LogAbilityOnForegroundEvent(name, type, eventInfo);
            break;
        case EventName::ABILITY_ONBACKGROUND:
        case EventName::ABILITY_ONINACTIVE:
            LogAbilityOnBackgroundEvent(name, type, eventInfo);
            break;
        case EventName::ABILITY_ONACTIVE:
            LogAbilityOnActiveEvent(name, type, eventInfo);
            break;
        case EventName::START_STANDARD_ABILITIES:
            LogStartStandardEvent(name, type, eventInfo);
            break;
        case EventName::START_ABILITY_BY_APP_LINKING:
            LogStartAbilityByAppLinking(name, type, eventInfo);
            break;
        case EventName::KILL_PROCESS_WITH_REASON:
            LogKillProcessWithReason(name, type, eventInfo);
            break;
        default:
            break;
    }
}

void EventReport::SendWantAgentEvent(
    const EventName &eventName, HiSysEventEventType type, const EventInfo &eventInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }

    if (eventName == EventName::WANTAGENT_NUMBER) {
        LogWantAgentNumberEvent(name, type, eventInfo);
    }
}

void EventReport::SendTriggerEvent(
    const EventName &eventName, HiSysEventEventType type, const EventInfo &eventInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }

    LogTriggerFailedEvent(name, type, eventInfo);
}

void EventReport::SendAtomicServiceEvent(
    const EventName &eventName, HiSysEventEventType type, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    auto hisyseventReport = std::make_shared<HisyseventReport>(10);
    switch (eventName) {
        case EventName::ATOMIC_SERVICE_DRAWN_COMPLETE:
            hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
            hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
            hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
            hisyseventReport->Report("AAFWK", name.c_str(), type);
            break;
        case EventName::CREATE_ATOMIC_SERVICE_PROCESS:
            hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
            hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
            hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
            hisyseventReport->InsertParam(EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName);
            hisyseventReport->InsertParam(EVENT_KEY_CALLER_PROCESS_NAME, eventInfo.callerProcessName);
            hisyseventReport->InsertParam(EVENT_KEY_CALLER_UID, eventInfo.callerUid);
            hisyseventReport->Report("AAFWK", name.c_str(), type);
            break;
        default:
            break;
    }
}

void EventReport::SendGrantUriPermissionEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName: %{public}s", name.c_str());
        return;
    }
    auto hisyseventReport = std::make_shared<HisyseventReport>(10);
    switch (eventName) {
        case EventName::GRANT_URI_PERMISSION:
            hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
            hisyseventReport->InsertParam(EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName);
            hisyseventReport->InsertParam(EVENT_KEY_URI, eventInfo.uri);
            hisyseventReport->Report("AAFWK", name.c_str(), HISYSEVENT_BEHAVIOR);
            break;
        case EventName::SHARE_UNPRIVILEGED_FILE_URI:
            hisyseventReport->InsertParam(EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName);
            hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
            hisyseventReport->Report("AAFWK", name.c_str(), HISYSEVENT_BEHAVIOR);
            break;
        default:
            break;
    }
}

void EventReport::SendExtensionEvent(
    const EventName &eventName, HiSysEventEventType type, const EventInfo &eventInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    auto hisyseventReport = std::make_shared<HisyseventReport>(10);
    switch (eventName) {
        case EventName::START_EXTENSION_ERROR:
        case EventName::STOP_EXTENSION_ERROR:
        case EventName::CONNECT_SERVICE_ERROR:
            hisyseventReport->InsertParam(EVENT_KEY_USERID, eventInfo.userId);
            hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
            hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
            hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
            hisyseventReport->InsertParam(EVENT_KEY_ERROR_CODE, eventInfo.errCode);
            hisyseventReport->Report("AAFWK", name.c_str(), type);
            break;
        case EventName::DISCONNECT_SERVICE_ERROR:
            hisyseventReport->InsertParam(EVENT_KEY_ERROR_CODE, eventInfo.errCode);
            hisyseventReport->Report("AAFWK", name.c_str(), type);
            break;
        case EventName::UI_EXTENSION_ERROR:
            LogUIExtensionErrorEvent(name, type, eventInfo);
            break;
        case EventName::UI_SERVICE_EXTENSION_ERROR:
            LogUIServiceExtErrorEvent(name, type, eventInfo);
            break;
        default:
            break;
    }
}

void EventReport::SendKeyEvent(const EventName &eventName, HiSysEventEventType type, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    TAG_LOGI(AAFwkTag::DEFAULT, "name: %{public}s", name.c_str());
    auto hisyseventReport = std::make_shared<HisyseventReport>(10);
    switch (eventName) {
        case EventName::FA_SHOW_ON_LOCK:
        case EventName::START_PRIVATE_ABILITY:
            hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
            hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
            hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
            hisyseventReport->Report("AAFWK", name.c_str(), type);
            break;
        case EventName::RESTART_PROCESS_BY_SAME_APP:
            hisyseventReport->InsertParam(EVENT_KEY_RESTART_TIME, eventInfo.time);
            hisyseventReport->InsertParam(EVENT_KEY_APP_UID, eventInfo.appUid);
            hisyseventReport->InsertParam(EVENT_KEY_CALLER_PROCESS_NAME, eventInfo.callerProcessName);
            hisyseventReport->InsertParam(EVENT_KEY_PROCESS_NAME, eventInfo.processName);
            hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
            hisyseventReport->Report("AAFWK", name.c_str(), type);
            break;
        default:
            break;
    }
}

void EventReport::SendAppLaunchEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    auto hisyseventReport = std::make_shared<HisyseventReport>(10);
    hisyseventReport->InsertParam(EVENT_KEY_APP_PID, eventInfo.pid);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_VERSION_NAME, eventInfo.versionName);
    hisyseventReport->InsertParam(EVENT_KEY_VERSION_CODE, eventInfo.versionCode);
    hisyseventReport->InsertParam(EVENT_KEY_PROCESS_NAME, eventInfo.processName);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_VERSION_NAME, eventInfo.callerVersionName);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_VERSION_CODE, eventInfo.callerVersionCode);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_UID, eventInfo.callerUid);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_STATE, eventInfo.callerState);
    hisyseventReport->Report("AAFWK", name.c_str(), HISYSEVENT_BEHAVIOR);
}

void EventReport::SendAppForegroundEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    auto hisyseventReport = std::make_shared<HisyseventReport>(10);
    hisyseventReport->InsertParam(EVENT_KEY_APP_PID, eventInfo.pid);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_VERSION_NAME, eventInfo.versionName);
    hisyseventReport->InsertParam(EVENT_KEY_VERSION_CODE, eventInfo.versionCode);
    hisyseventReport->InsertParam(EVENT_KEY_PROCESS_NAME, eventInfo.processName);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_TYPE, eventInfo.bundleType);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName);
    hisyseventReport->InsertParam(EVENT_KEY_PROCESS_TYPE, eventInfo.processType);
    auto ret = hisyseventReport->Report("AAFWK", name.c_str(), HISYSEVENT_BEHAVIOR);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::DEFAULT, "fail: %{public}s, ret %{public}d", name.c_str(), ret);
    }
}

void EventReport::SendAppBackgroundEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    auto hisyseventReport = std::make_shared<HisyseventReport>(10);
    hisyseventReport->InsertParam(EVENT_KEY_APP_PID, eventInfo.pid);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_VERSION_NAME, eventInfo.versionName);
    hisyseventReport->InsertParam(EVENT_KEY_VERSION_CODE, eventInfo.versionCode);
    hisyseventReport->InsertParam(EVENT_KEY_PROCESS_NAME, eventInfo.processName);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_TYPE, eventInfo.bundleType);
    hisyseventReport->InsertParam(EVENT_KEY_PROCESS_TYPE, eventInfo.processType);
    auto ret = hisyseventReport->Report("AAFWK", name.c_str(), HISYSEVENT_BEHAVIOR);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::DEFAULT, "fail: %{public}s, ret %{public}d", name.c_str(), ret);
    }
}

void EventReport::SendProcessStartEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    constexpr int32_t defaultVal = -1;
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    if (eventInfo.extensionType == defaultVal) {
        auto hisyseventReport = std::make_shared<HisyseventReport>(10);
        hisyseventReport->InsertParam(EVENT_KEY_STARTUP_TIME, eventInfo.time);
        hisyseventReport->InsertParam(EVENT_KEY_STARTUP_ABILITY_TYPE, eventInfo.abilityType);
        hisyseventReport->InsertParam(EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName);
        hisyseventReport->InsertParam(EVENT_KEY_CALLER_UID, eventInfo.callerUid);
        hisyseventReport->InsertParam(EVENT_KEY_CALLER_PROCESS_NAME, eventInfo.callerProcessName);
        hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
        hisyseventReport->InsertParam(EVENT_KEY_PID, eventInfo.pid);
        hisyseventReport->InsertParam(EVENT_KEY_PROCESS_NAME, eventInfo.processName);
        hisyseventReport->InsertParam(EVENT_KEY_CALLER_PROCESS_ID, eventInfo.callerPid);
        hisyseventReport->Report("AAFWK", name.c_str(), HISYSEVENT_BEHAVIOR);
    } else {
        auto hisyseventReport = std::make_shared<HisyseventReport>(10);
        hisyseventReport->InsertParam(EVENT_KEY_STARTUP_TIME, eventInfo.time);
        hisyseventReport->InsertParam(EVENT_KEY_STARTUP_ABILITY_TYPE, eventInfo.abilityType);
        hisyseventReport->InsertParam(EVENT_KEY_STARTUP_EXTENSION_TYPE, eventInfo.extensionType);
        hisyseventReport->InsertParam(EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName);
        hisyseventReport->InsertParam(EVENT_KEY_CALLER_UID, eventInfo.callerUid);
        hisyseventReport->InsertParam(EVENT_KEY_CALLER_PROCESS_NAME, eventInfo.callerProcessName);
        hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
        hisyseventReport->InsertParam(EVENT_KEY_PID, eventInfo.pid);
        hisyseventReport->InsertParam(EVENT_KEY_PROCESS_NAME, eventInfo.processName);
        hisyseventReport->InsertParam(EVENT_KEY_CALLER_PROCESS_ID, eventInfo.callerPid);
        hisyseventReport->Report("AAFWK", name.c_str(), HISYSEVENT_BEHAVIOR);
    }
}

void EventReport::SendProcessStartFailedEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "eventName:%{public}s,processName:%{public}s,reason:%{public}d,subReason:%{public}d",
        name.c_str(), eventInfo.processName.c_str(), eventInfo.reason, eventInfo.subReason);
    if (eventInfo.extensionType == DEFAULT_EXTENSION_TYPE) {
        auto hisyseventReport = std::make_shared<HisyseventReport>(11);
        hisyseventReport->InsertParam(EVENT_KEY_STARTUP_TIME, eventInfo.time);
        hisyseventReport->InsertParam(EVENT_KEY_STARTUP_ABILITY_TYPE, eventInfo.abilityType);
        hisyseventReport->InsertParam(EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName);
        hisyseventReport->InsertParam(EVENT_KEY_CALLER_UID, eventInfo.callerUid);
        hisyseventReport->InsertParam(EVENT_KEY_CALLER_PROCESS_NAME, eventInfo.callerProcessName);
        hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
        hisyseventReport->InsertParam(EVENT_KEY_PROCESS_NAME, eventInfo.processName);
        hisyseventReport->InsertParam(EVENT_KEY_CALLER_PROCESS_ID, eventInfo.callerPid);
        hisyseventReport->InsertParam(EVENT_KEY_PROCESS_TYPE, eventInfo.processType);
        hisyseventReport->InsertParam(EVENT_KEY_REASON, eventInfo.reason);
        hisyseventReport->InsertParam(EVENT_KEY_SUB_REASON, eventInfo.subReason);
        hisyseventReport->Report("AAFWK", name.c_str(), HISYSEVENT_FAULT);
    } else {
        auto hisyseventReport = std::make_shared<HisyseventReport>(12);
        hisyseventReport->InsertParam(EVENT_KEY_STARTUP_TIME, eventInfo.time);
        hisyseventReport->InsertParam(EVENT_KEY_STARTUP_ABILITY_TYPE, eventInfo.abilityType);
        hisyseventReport->InsertParam(EVENT_KEY_STARTUP_EXTENSION_TYPE, eventInfo.extensionType);
        hisyseventReport->InsertParam(EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName);
        hisyseventReport->InsertParam(EVENT_KEY_CALLER_UID, eventInfo.callerUid);
        hisyseventReport->InsertParam(EVENT_KEY_CALLER_PROCESS_NAME, eventInfo.callerProcessName);
        hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
        hisyseventReport->InsertParam(EVENT_KEY_PROCESS_NAME, eventInfo.processName);
        hisyseventReport->InsertParam(EVENT_KEY_CALLER_PROCESS_ID, eventInfo.callerPid);
        hisyseventReport->InsertParam(EVENT_KEY_PROCESS_TYPE, eventInfo.processType);
        hisyseventReport->InsertParam(EVENT_KEY_REASON, eventInfo.reason);
        hisyseventReport->InsertParam(EVENT_KEY_SUB_REASON, eventInfo.subReason);
        hisyseventReport->Report("AAFWK", name.c_str(), HISYSEVENT_FAULT);
    }
}

void EventReport::SendProcessExitEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    auto hisyseventReport = std::make_shared<HisyseventReport>(12);
    hisyseventReport->InsertParam(EVENT_KEY_EXIT_TIME, eventInfo.time);
    hisyseventReport->InsertParam(EVENT_KEY_EXIT_RESULT, eventInfo.exitResult);
    hisyseventReport->InsertParam(EVENT_KEY_EXIT_PID, eventInfo.pid);
    hisyseventReport->InsertParam(EVENT_KEY_PROCESS_NAME, eventInfo.processName);
    hisyseventReport->InsertParam(EVENT_KEY_EXTENSION_TYPE, eventInfo.extensionType);
    hisyseventReport->InsertParam(EVENT_KEY_EXIT_REASON, eventInfo.exitReason);
    hisyseventReport->Report("AAFWK", name.c_str(), HISYSEVENT_BEHAVIOR);
}

void EventReport::SendStartServiceEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    auto hisyseventReport = std::make_shared<HisyseventReport>(10);
    hisyseventReport->InsertParam(EVENT_KEY_TIME, eventInfo.time);
    hisyseventReport->InsertParam(EVENT_KEY_USERID, eventInfo.userId);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
    hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    hisyseventReport->InsertParam(EVENT_KEY_EXTENSION_TYPE, eventInfo.extensionType);
    hisyseventReport->InsertParam(EVENT_KEY_PID, eventInfo.pid);
    hisyseventReport->InsertParam(EVENT_KEY_PROCESS_NAME, eventInfo.processName);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_PROCESS_ID, eventInfo.callerPid);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_PROCESS_NAME, eventInfo.callerProcessName);
    hisyseventReport->Report("AAFWK", name.c_str(), HISYSEVENT_BEHAVIOR);
}

void EventReport::SendStopServiceEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    auto hisyseventReport = std::make_shared<HisyseventReport>(10);
    hisyseventReport->InsertParam(EVENT_KEY_USERID, eventInfo.userId);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
    hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    hisyseventReport->InsertParam(EVENT_KEY_EXTENSION_TYPE, eventInfo.extensionType);
    hisyseventReport->InsertParam(EVENT_KEY_TIME, eventInfo.time);
    hisyseventReport->InsertParam(EVENT_KEY_PID, eventInfo.pid);
    hisyseventReport->InsertParam(EVENT_KEY_PROCESS_NAME, eventInfo.processName);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_PROCESS_ID, eventInfo.callerPid);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_PROCESS_NAME, eventInfo.callerProcessName);
    hisyseventReport->Report("AAFWK", name.c_str(), HISYSEVENT_BEHAVIOR);
}

void EventReport::SendConnectServiceEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    auto hisyseventReport = std::make_shared<HisyseventReport>(10);
    hisyseventReport->InsertParam(EVENT_KEY_TIME, eventInfo.time);
    hisyseventReport->InsertParam(EVENT_KEY_USERID, eventInfo.userId);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
    hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    hisyseventReport->InsertParam(EVENT_KEY_PID, eventInfo.pid);
    hisyseventReport->InsertParam(EVENT_KEY_PROCESS_NAME, eventInfo.processName);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_PROCESS_ID, eventInfo.callerPid);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_PROCESS_NAME, eventInfo.callerProcessName);
    hisyseventReport->Report("AAFWK", name.c_str(), HISYSEVENT_BEHAVIOR);
}

void EventReport::SendDisconnectServiceEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    auto hisyseventReport = std::make_shared<HisyseventReport>(5);
    hisyseventReport->InsertParam(EVENT_KEY_TIME, eventInfo.time);
    hisyseventReport->InsertParam(EVENT_KEY_PID, eventInfo.pid);
    hisyseventReport->InsertParam(EVENT_KEY_PROCESS_NAME, eventInfo.processName);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_PROCESS_ID, eventInfo.callerPid);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_PROCESS_NAME, eventInfo.callerProcessName);
    hisyseventReport->Report("AAFWK", name.c_str(), HISYSEVENT_BEHAVIOR);
}

void EventReport::SendStartAbilityOtherExtensionEvent(const EventName &eventName, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    auto hisyseventReport = std::make_shared<HisyseventReport>(5);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
    hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    hisyseventReport->InsertParam(EVENT_KEY_EXTENSION_TYPE, eventInfo.extensionType);
    // Historical reason: Word spelling error during event definition
    hisyseventReport->InsertParam("CALLER_BUNLED_NAME", eventInfo.callerBundleName);
    hisyseventReport->Report("AAFWK", name.c_str(), HISYSEVENT_BEHAVIOR);
}

void EventReport::SendExecuteIntentEvent(
    const EventName &eventName, HiSysEventEventType type, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    auto hisyseventReport = std::make_shared<HisyseventReport>(10);
    hisyseventReport->InsertParam(EVENT_KEY_USERID, eventInfo.userId);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
    hisyseventReport->InsertParam(EVENT_KEY_ABILITY_NAME, eventInfo.abilityName);
    hisyseventReport->InsertParam(EVENT_KEY_ERROR_CODE, eventInfo.errCode);
    hisyseventReport->InsertParam(EVENT_KEY_CALLER_BUNDLE_NAME, eventInfo.callerBundleName);
    hisyseventReport->InsertParam(EVENT_KEY_APP_INDEX, eventInfo.appIndex);
    hisyseventReport->InsertParam(EVENT_KEY_ERR_REASON, eventInfo.errReason);
    hisyseventReport->InsertParam(EVENT_KEY_INTENT_NAME, eventInfo.intentName);
    hisyseventReport->Report("AAFWK", name.c_str(), type);
}

void EventReport::SendLaunchFrameworkEvent(
    const EventName &eventName, HiSysEventEventType type, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    auto hisyseventReport = std::make_shared<HisyseventReport>(10);
    hisyseventReport->InsertParam(EVENT_KEY_USERID, eventInfo.userId);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
    hisyseventReport->InsertParam(EVENT_KEY_ERROR_CODE, eventInfo.errCode);
    hisyseventReport->InsertParam(EVENT_KEY_PROCESS_NAME, eventInfo.processName);
    hisyseventReport->InsertParam(EVENT_KEY_APP_INDEX, eventInfo.appIndex);
    hisyseventReport->InsertParam(EVENT_KEY_ERR_REASON, eventInfo.errReason);
    hisyseventReport->Report("AAFWK", name.c_str(), type);
}

void EventReport::SendReportDataPartitionUsageEvent(const EventName &eventName, HiSysEventEventType type,
    const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }

    auto hisyseventReport = std::make_shared<HisyseventReport>(5);
    hisyseventReport->InsertParam(EVENT_COMPONENT_NAME_KEY, eventInfo.componentName);
    hisyseventReport->InsertParam(EVENT_PARTITION_NAME_KEY, eventInfo.partitionName);
    hisyseventReport->InsertParam(EVENT_REMAIN_PARTITION_SIZE_KEY, eventInfo.remainPartitionSize);
    std::vector<char*> list = {};
    list.reserve(eventInfo.fileOfFolderPath.size());
    for (const auto& s : eventInfo.fileOfFolderPath) {
        list.emplace_back(const_cast<char *>(s.c_str()));
    }
    hisyseventReport->InsertParam(EVENT_FILE_OR_FOLDER_PATH, list);
    hisyseventReport->InsertParam(EVENT_FILE_OR_FOLDER_SIZE, eventInfo.fileOfFolderSize);
#ifdef USE_EXTENSION_DATA
    hisyseventReport->Report("FILEMANAGEMENT", name.c_str(), type);
#else
    hisyseventReport->Report("AAFWK", name.c_str(), type);
#endif
}

void EventReport::SendAppStartupErrorEvent(
    const EventName &eventName, HiSysEventEventType type, const EventInfo &eventInfo)
{
    std::string name = ConvertEventName(eventName);
    if (name == INVALID_EVENT_NAME) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid eventName");
        return;
    }
    auto hisyseventReport = std::make_shared<HisyseventReport>(10);
    hisyseventReport->InsertParam(EVENT_KEY_USERID, eventInfo.userId);
    hisyseventReport->InsertParam(EVENT_KEY_BUNDLE_NAME, eventInfo.bundleName);
    hisyseventReport->InsertParam(EVENT_KEY_MODULE_NAME, eventInfo.moduleName);
    hisyseventReport->InsertParam(EVENT_KEY_APP_INDEX, eventInfo.appIndex);
    hisyseventReport->InsertParam(EVENT_KEY_ERROR_CODE, eventInfo.errCode);
    hisyseventReport->InsertParam(EVENT_KEY_ERROR_MESSAGE, eventInfo.errMsg);
    hisyseventReport->Report("AAFWK", name.c_str(), type);
}

std::string EventReport::ConvertEventName(const EventName &eventName)
{
    const char* eventNames[] = {
        // fault event
        "START_ABILITY_ERROR", "TERMINATE_ABILITY_ERROR", "START_EXTENSION_ERROR",
        "STOP_EXTENSION_ERROR", "CONNECT_SERVICE_ERROR", "DISCONNECT_SERVICE_ERROR",
        "UI_EXTENSION_ERROR", "UI_SERVICE_EXTENSION_ERROR", "EXECUTE_INSIGHT_INTENT_ERROR",
        "STARTUP_TASK_ERROR", "START_ABILITY_SYSTEM_ERROR", "APP_STARTUP_ERROR",

        // ability behavior event
        "START_ABILITY", "TERMINATE_ABILITY", "CLOSE_ABILITY",
        "ABILITY_ONFOREGROUND", "ABILITY_ONBACKGROUND", "ABILITY_ONACTIVE", "ABILITY_ONINACTIVE",
        "START_ABILITY_BY_APP_LINKING", "KILL_PROCESS_WITH_REASON",

        // serviceExtensionAbility behavior event
        "START_SERVICE", "STOP_SERVICE", "CONNECT_SERVICE", "DISCONNECT_SERVICE", "START_ABILITY_OTHER_EXTENSION",

        // app behavior event
        "APP_ATTACH", "APP_LAUNCH", "APP_FOREGROUND", "APP_BACKGROUND", "APP_TERMINATE",
        "PROCESS_START", "PROCESS_EXIT", "DRAWN_COMPLETED", "APP_STARTUP_TYPE", "PROCESS_START_FAILED",

        // key behavior event
        "GRANT_URI_PERMISSION", "FA_SHOW_ON_LOCK", "START_PRIVATE_ABILITY",
        "RESTART_PROCESS_BY_SAME_APP", "START_STANDARD_ABILITIES",

        // atomic service event
        "CREATE_ATOMIC_SERVICE_PROCESS", "ATOMIC_SERVICE_DRAWN_COMPLETE",
        
        // uri permission
        "SHARE_UNPRIVILEGED_FILE_URI",

        // want_agent
        "WANTAGENT_NUMBER",

        // report data
        "USER_DATA_SIZE"
    };
    uint32_t eventIndex = static_cast<uint32_t> (eventName);
    if (eventIndex >= sizeof(eventNames) / sizeof(const char*)) {
        return INVALID_EVENT_NAME;
    }
    return eventNames[eventIndex];
}
}  // namespace AAFwk
}  // namespace OHOS
