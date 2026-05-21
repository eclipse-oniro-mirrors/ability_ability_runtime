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

#ifndef MOCK_RES_SCHED_UTIL_H
#define MOCK_RES_SCHED_UTIL_H

#include <gmock/gmock.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>

namespace OHOS {
namespace AppExecFwk {
struct AbilityInfo;
}
namespace AAFwk {
using AbilityInfo = AppExecFwk::AbilityInfo;

enum class LoadingStage : int32_t {
    LOAD_BEGIN = 1,
    LOAD_END,
    FOREGROUND_BEGIN,
    FOREGROUND_END,
    CONNECT_BEGIN,
    CONNECT_END,
    DESTROY_BEGIN = 8,
    DESTROY_END = 9,
    PRELOAD_BEGIN,
    PRELOAD_END,
    PRE_LAUNCH_BEGIN,
};

class ResSchedUtil {
public:
    ResSchedUtil() = default;
    virtual ~ResSchedUtil() = default;
    static ResSchedUtil &GetInstance();

    MOCK_METHOD(void, ReportAbilityStartInfoToRSS,
        (const AbilityInfo &abilityInfo, int32_t pid, bool isColdStart,
        bool supportWarmSmartGC, int32_t preloadMode, bool isSuggestCache), ());
    MOCK_METHOD(void, ReportPreloadApplicationToRSS,
        (const std::shared_ptr<AbilityInfo>& abilityInfo, int32_t preloadMode), ());
    MOCK_METHOD(void, ReportAbilityAssociatedStartInfoToRSS,
        (const AbilityInfo &abilityInfo, int64_t resSchedType, int32_t callerUid, int32_t callerPid), ());
    MOCK_METHOD(void, ReportEventToRSS,
        (const int32_t uid, const std::string &bundleName, const std::string &reason,
        const int32_t pid, const int32_t callerPid, bool isCreateFromImage), ());
    MOCK_METHOD(void, ReportUIExtensionProcColdStartToRss,
        (int32_t extensionAbilityType, int hostPid, const std::string& hostBundleName, const std::string& bundleName,
        const std::string& abilityName, const std::string& moduleName, bool isPreloadUIExtension), ());
    MOCK_METHOD(void, PromotePriorityToRSS,
        (int32_t callerUid, int32_t callerPid, const std::string &targetBundleName,
        int32_t targetUid, int32_t targetPid), ());
    MOCK_METHOD(std::string, GetThawReasonByAbilityType, (const AbilityInfo &abilityInfo), ());
    MOCK_METHOD(void, GetAllFrozenPidsFromRSS, (std::unordered_set<int32_t> &frozenPids), ());
    MOCK_METHOD(bool, CheckShouldForceKillProcess, (int32_t pid, const std::string& bundleName), ());
    inline void ReportLoadingEventToRss(LoadingStage stage, int32_t pid, int32_t uid,
        int64_t timeDuration, int64_t abilityRecordId) {}
    inline void ReportLoadingEventToRss(LoadingStage stage, int32_t pid, int32_t uid, int64_t timeDuration,
        int64_t abilityRecordId, const std::unordered_map<std::string, std::string> &extraParams) {}
    MOCK_METHOD(std::unordered_set<std::string>, GetNWebPreloadSet, (), (const));
    MOCK_METHOD(void, ReportAbilityIntentExemptionInfoToRSS, (int32_t callerUid, int32_t callerPid), ());
    MOCK_METHOD(void, ReportSubHealtyPerfInfoToRSS, (), ());
    MOCK_METHOD(void, ReportForkAllEventToRSS,
        (int32_t imagePid, int32_t orginalPid,
        std::shared_ptr<AbilityInfo> abilityInfo, int32_t forkAllState), ());
};
} // namespace AAFwk
} // namespace OHOS
#endif // MOCK_RES_SCHED_UTIL_H
