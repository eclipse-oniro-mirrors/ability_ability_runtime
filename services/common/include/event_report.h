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

#ifndef OHOS_ABILITY_RUNTIME_EVENT_REPORT_H
#define OHOS_ABILITY_RUNTIME_EVENT_REPORT_H

#include <string>

#include "hisysevent.h"

using HiSysEventType = OHOS::HiviewDFX::HiSysEvent::EventType;
using HiSysEvent = OHOS::HiviewDFX::HiSysEvent;

namespace OHOS {
namespace AAFwk {
struct EventInfo {
    bool isPreload = false;
    int32_t pid = -1;
    int32_t userId = -1;
    int32_t extensionType = -1;
    int32_t abilityNumber = 0;
    int32_t abilityType = -1;
    int32_t callerUid = -1;
    int32_t exitResult = -1;
    int32_t bundleType = -1;
    int32_t startType = 0;
    int32_t startReason = 0;
    int32_t appUid = -1;
    int32_t errCode = -1;
    int32_t callerState = -1;
    int32_t processType = -1;
    int32_t callerPid = -1;
    int64_t duration = 0;
    int32_t reason = -1;
    int32_t subReason = -1;
    int32_t exitReason = -1;
    int32_t preloadMode = 0;
    int32_t appIndex = -1;
    int32_t persistentId = 0;
    std::string lifeCycleState;
    uint32_t versionCode = 0;
    uint32_t callerVersionCode = -1;
    int64_t time = 0;
    std::string versionName;
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    std::string processName;
    std::string callerProcessName;
    std::string callerBundleName;
    std::string callerVersionName;
    std::string uri;
    std::string errReason;
    std::string lifeCycle;
    std::string intentName;
    std::string errMsg;
    std::string componentName;
    std::string partitionName;
    uint64_t remainPartitionSize;
    std::vector<std::string> fileOfFolderPath;
    std::vector<uint64_t> fileOfFolderSize;
};

enum class EventName {
    // fault event
    START_ABILITY_ERROR = 0,
    TERMINATE_ABILITY_ERROR,
    START_EXTENSION_ERROR,
    STOP_EXTENSION_ERROR,
    CONNECT_SERVICE_ERROR,
    DISCONNECT_SERVICE_ERROR,
    UI_EXTENSION_ERROR,
    UI_SERVICE_EXTENSION_ERROR,
    EXECUTE_INSIGHT_INTENT_ERROR,
    STARTUP_TASK_ERROR,
    START_ABILITY_SYSTEM_ERROR,

    // ability behavior event
    START_ABILITY,
    TERMINATE_ABILITY,
    CLOSE_ABILITY,
    ABILITY_ONFOREGROUND,
    ABILITY_ONBACKGROUND,
    ABILITY_ONACTIVE,
    ABILITY_ONINACTIVE,
    START_ABILITY_BY_APP_LINKING,

    // serviceExtensionAbility behavior event
    START_SERVICE,
    STOP_SERVICE,
    CONNECT_SERVICE,
    DISCONNECT_SERVICE,
    START_ABILITY_OTHER_EXTENSION,

    // app behavior event
    APP_ATTACH,
    APP_LAUNCH,
    APP_FOREGROUND,
    APP_BACKGROUND,
    APP_TERMINATE,
    PROCESS_START,
    PROCESS_EXIT,
    DRAWN_COMPLETED,
    APP_STARTUP_TYPE,
    PROCESS_START_FAILED,

    // key behavior event
    GRANT_URI_PERMISSION,
    FA_SHOW_ON_LOCK,
    START_PRIVATE_ABILITY,
    RESTART_PROCESS_BY_SAME_APP,
    START_STANDARD_ABILITIES,

    // atomic service event
    CREATE_ATOMIC_SERVICE_PROCESS,
    ATOMIC_SERVICE_DRAWN_COMPLETE,
    
    // uri permission
    SHARE_UNPRIVILEGED_FILE_URI,

    // report data
    USER_DATA_SIZE
};

class EventReport {
public:
    static void SendAppEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo);
    static void SendAbilityEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo);
    static void SendAtomicServiceEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo);
    static void SendExtensionEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo);
    static void SendKeyEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo);
    static void SendAppLaunchEvent(const EventName &eventName, const EventInfo &eventInfo);
    static void SendAppForegroundEvent(const EventName &eventName, const EventInfo &eventInfo);
    static void SendAppBackgroundEvent(const EventName &eventName, const EventInfo &eventInfo);
    static void SendProcessStartEvent(const EventName &eventName, const EventInfo &eventInfo);
    static void SendProcessStartFailedEvent(const EventName &eventName, const EventInfo &eventInfo);
    static void SendProcessExitEvent(const EventName &eventName, const EventInfo &eventInfo);
    static void SendStartServiceEvent(const EventName &eventName, const EventInfo &eventInfo);
    static void SendStopServiceEvent(const EventName &eventName, const EventInfo &eventInfo);
    static void SendConnectServiceEvent(const EventName &eventName, const EventInfo &eventInfo);
    static void SendDisconnectServiceEvent(const EventName &eventName, const EventInfo &eventInfo);
    static void SendStartAbilityOtherExtensionEvent(const EventName &eventName, const EventInfo &eventInfo);
    static void SendGrantUriPermissionEvent(const EventName &eventName, const EventInfo &eventInfo);
    static void SendExecuteIntentEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo);
    static void SendLaunchFrameworkEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo);
    static void SendReportDataPartitionUsageEvent(const EventName &eventName, HiSysEventType type,
        const EventInfo &eventInfo);

private:
    static std::string ConvertEventName(const EventName &eventName);
    static void LogErrorEvent(const std::string &name, HiSysEventType type, const EventInfo &eventInfo);
    static void LogStartAbilityEvent(const std::string &name, HiSysEventType type, const EventInfo &eventInfo);
    static void LogTerminateAbilityEvent(const std::string &name, HiSysEventType type, const EventInfo &eventInfo);
    static void LogAbilityOnForegroundEvent(const std::string &name, HiSysEventType type, const EventInfo &eventInfo);
    static void LogAbilityOnBackgroundEvent(const std::string &name, HiSysEventType type, const EventInfo &eventInfo);
    static void LogAbilityOnActiveEvent(const std::string &name, HiSysEventType type, const EventInfo &eventInfo);
    static void LogAbilityOnInactiveEvent(const std::string &name, HiSysEventType type, const EventInfo &eventInfo);
    static void LogStartStandardEvent(const std::string &name, HiSysEventType type, const EventInfo &eventInfo);
    static void LogStartAbilityByAppLinking(const std::string &name, HiSysEventType type, const EventInfo &eventInfo);
    static void LogUIExtensionErrorEvent(const std::string &name, HiSysEventType type, const EventInfo &eventInfo);
    static void LogUIServiceExtErrorEvent(const std::string &name, HiSysEventType type, const EventInfo &eventInfo);
    static void LogSystemErrorEvent(const std::string &name, HiSysEventType type, const EventInfo &eventInfo);
};
}  // namespace AAFWK
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_EVENT_REPORT_H
