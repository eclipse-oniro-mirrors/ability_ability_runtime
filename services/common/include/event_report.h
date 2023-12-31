/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
    int32_t pid = -1;
    int32_t userId = -1;
    int32_t extensionType = -1;
    uint32_t versionCode = 0;
    int32_t errCode = -1;
    std::string versionName;
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    std::string processName;
    std::string callerProcessName;
    std::string callerBundleName;
    int32_t abilityType = -1;
    int64_t time = 0;
    int32_t callerUid = -1;
    int32_t exitResult = -1;
    int32_t bundleType = -1;
};

enum class EventName {
    // fault event
    START_ABILITY_ERROR,
    TERMINATE_ABILITY_ERROR,
    START_EXTENSION_ERROR,
    STOP_EXTENSION_ERROR,
    CONNECT_SERVICE_ERROR,
    DISCONNECT_SERVICE_ERROR,
    CLOSE_UI_ABILITY_BY_SCB_ERROR,

    // ability behavior event
    START_ABILITY,
    TERMINATE_ABILITY,
    CLOSE_ABILITY,
    CLOSE_UI_ABILITY_BY_SCB,
    ABILITY_ONFOREGROUND,
    ABILITY_ONBACKGROUND,
    ABILITY_ONACTIVE,
    ABILITY_ONINACTIVE,

    // serviceExtensionAbility behavior event
    START_SERVICE,
    STOP_SERVICE,
    CONNECT_SERVICE,
    DISCONNECT_SERVICE,

    // app behavior event
    APP_ATTACH,
    APP_LAUNCH,
    APP_FOREGROUND,
    APP_BACKGROUND,
    APP_TERMINATE,
    PROCESS_START,
    PROCESS_EXIT,
    DRAWN_COMPLETED,
};

class EventReport {
public:
    static void SendAppEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo);
    static void SendAbilityEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo);
    static void SendExtensionEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo);

private:
    static std::string ConvertEventName(const EventName &eventName);
};
}  // namespace AAFWK
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_EVENT_REPORT_H
