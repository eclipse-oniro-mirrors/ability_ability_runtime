/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APPFREEZE_EVENT_REPORT_H
#define OHOS_ABILITY_RUNTIME_APPFREEZE_EVENT_REPORT_H

#include <string>

#include "hisysevent_report.h"

namespace OHOS {
namespace AppExecFwk {
struct AppfreezeEventInfo {
    std::string bundleName;
    std::string processName;
    std::string binderInfo;
    std::string freezeMemory;
    std::string appRunningUniqueId;
    std::string errorStack;
    std::string errorName;
    std::string errorMessage;
    std::string freezeInfoFile;
    std::string hitraceInfo;
    std::string applicationHeapInfo;
    std::string applicationGCInfo;
    std::string applicationIOInfo;
    bool isBlockInGc = false;
    std::string processLifeTime;
    std::string mainStack;
    std::string externalLog;
    int32_t markedId = 0;
    int32_t processedId = 0;
    int32_t dispatchedEventId = 0;
    int tid = 0;
    int pid = 0;
    int uid = 0;
    int eventId = -1;
    bool foregroundState;
    bool enableFreeze;
    bool reportLifecycleToFreeze = false;
    bool isFrozen = false;
};

class AppfreezeEventReport {
public:
    static int SendAppfreezeEvent(const std::string &eventName, HiSysEventEventType type,
        const AppfreezeEventInfo &eventInfo);

private:
    static int LogAppInputBlockEvent(const std::string &name, HiSysEventEventType type,
        const AppfreezeEventInfo &eventInfo);
    static int LogThreadBlockEvent(const std::string &name, HiSysEventEventType type,
        const AppfreezeEventInfo &eventInfo);
    static int LogLifeCycleTimeoutEvent(const std::string &name, HiSysEventEventType type,
        const AppfreezeEventInfo &eventInfo);
    static int LogGeneralEvent(const std::string &name, HiSysEventEventType type,
        const AppfreezeEventInfo &eventInfo);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APPFREEZE_EVENT_REPORT_H
