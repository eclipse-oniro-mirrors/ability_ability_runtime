/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "res_sched_util.h"

#include <string>

#include "ability_info.h"
#include "ui_extension_wrapper.h"
#include "hilog_tag_wrapper.h"
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
#include "res_sched_client.h"
#include "res_type.h"
#endif

namespace OHOS {
namespace AAFwk {
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
using AssociatedStartType = ResourceSchedule::ResType::AssociatedStartType;
static constexpr int PERF_NUM = 10202;
#endif
ResSchedUtil &ResSchedUtil::GetInstance()
{
    static ResSchedUtil instance;
    return instance;
}

int64_t ResSchedUtil::convertType(int64_t resSchedType)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    if (resSchedType == RES_TYPE_SCB_START_ABILITY) {
        return static_cast<int64_t>(AssociatedStartType::SCB_START_ABILITY);
    } else if (resSchedType == RES_TYPE_EXTENSION_START_ABILITY) {
        return static_cast<int64_t>(AssociatedStartType::EXTENSION_START_ABILITY);
    } else if (resSchedType == RES_TYPE_MISSION_LIST_START_ABILITY) {
        return static_cast<int64_t>(AssociatedStartType::MISSION_LIST_START_ABILITY);
    }
#endif
    TAG_LOGE(AAFwkTag::DEFAULT, "sched invalid");
    return -1;
}

void ResSchedUtil::ReportSubHealtyPerfInfoToRSS()
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    uint32_t resType = ResourceSchedule::ResType::RES_TYPE_ANCO_CUST;
    std::unordered_map<std::string, std::string> eventParams{{"name", "soc_perf"}};
    TAG_LOGI(AAFwkTag::DEFAULT, "soc_perf start");
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(resType, PERF_NUM, eventParams);
#endif
}

void ResSchedUtil::ReportForkAllEventToRSS(int32_t imagePid, int32_t orginalPid,
    std::shared_ptr<AbilityInfo> abilityInfo, int32_t forkAllState)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "nullptr");
        return;
    }
    uint32_t resType = ResourceSchedule::ResType::RES_TYPE_CREATE_LIVING_APP_PROCESS;
    std::unordered_map<std::string, std::string> eventParams {
        { "uid", std::to_string(abilityInfo->applicationInfo.uid) },
        { "bundleName", abilityInfo->applicationInfo.bundleName },
        { "imagePid", std::to_string(imagePid) },
        { "originalPid", std::to_string(orginalPid) }
    };
    TAG_LOGI(AAFwkTag::DEFAULT, "call");
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(resType, forkAllState, eventParams);
#endif
}

void ResSchedUtil::ReportAbilityStartInfoToRSS(const AbilityInfo &abilityInfo, int32_t pid, bool isColdStart,
    bool supportWarmSmartGC, int32_t preloadMode)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    uint32_t resType = ResourceSchedule::ResType::RES_TYPE_APP_ABILITY_START;
    std::unordered_map<std::string, std::string> eventParams {
        { "name", "ability_start" },
        { "uid", std::to_string(abilityInfo.applicationInfo.uid) },
        { "bundleName", abilityInfo.applicationInfo.bundleName },
        { "abilityName", abilityInfo.name },
        { "pid", std::to_string(pid) },
        { "supportWarmSmartGC", supportWarmSmartGC ? "1": "0" },
        { "preloadMode", std::to_string(preloadMode) }
    };
    TAG_LOGD(AAFwkTag::DEFAULT, "call");
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(resType, isColdStart ? 1 : 0, eventParams);
#endif
}

void ResSchedUtil::ReportAbilityAssociatedStartInfoToRSS(
    const AbilityInfo &abilityInfo, int64_t resSchedType, int32_t callerUid, int32_t callerPid)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    uint32_t resType = ResourceSchedule::ResType::RES_TYPE_APP_ASSOCIATED_START;
    std::unordered_map<std::string, std::string> eventParams {
        { "name", "associated_start" },
        { "caller_uid", std::to_string(callerUid) },
        { "caller_pid", std::to_string(callerPid) },
        { "callee_uid", std::to_string(abilityInfo.applicationInfo.uid) },
        { "callee_bundle_name", abilityInfo.applicationInfo.bundleName }
    };
    int64_t type = convertType(resSchedType);
    TAG_LOGD(AAFwkTag::DEFAULT, "call");
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(resType, type, eventParams);
#endif
}

void ResSchedUtil::ReportPreloadApplicationToRSS(const std::shared_ptr<AbilityInfo>& abilityInfo, int32_t preloadMode)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    uint32_t resType = ResourceSchedule::ResType::RES_TYPE_PRELOAD_APPLICATION;
    std::unordered_map<std::string, std::string> eventParams {
        { "name", "preload_application" },
        { "uid", std::to_string(abilityInfo->applicationInfo.uid) },
        { "bundleName", abilityInfo->applicationInfo.bundleName },
        { "preloadMode", std::to_string(preloadMode) }
    };
    TAG_LOGD(AAFwkTag::DEFAULT, "call");
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(resType, preloadMode, eventParams);
#endif
}

void ResSchedUtil::ReportUIExtensionProcColdStartToRss(
    int32_t extensionAbilityType, int hostPid, const std::string& hostBundleName, const std::string& bundleName,
    const std::string& abilityName, const std::string& moduleName, bool isPreloadUIExtension)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    uint32_t resType = ResourceSchedule::ResType::RES_TYPE_START_UIEXTENSION_PROC;
    std::unordered_map<std::string, std::string> payload {
        {"extensionAbilityType", std::to_string(extensionAbilityType)},
        {"hostPid", std::to_string(hostPid)},
        {"hostBundleName", hostBundleName},
        {"bundleName", bundleName},
        {"abilityName", abilityName},
        {"moduleName", moduleName},
        {"isPreloadUIExtension", std::to_string(isPreloadUIExtension)},
    };
    TAG_LOGD(AAFwkTag::DEFAULT, "resType:%{public}u, call", resType);
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(resType, 0, payload);
#endif
}

std::string ResSchedUtil::GetThawReasonByAbilityType(const AbilityInfo &abilityInfo)
{
    std::string reason;
    if (abilityInfo.type == AppExecFwk::AbilityType::PAGE) {
        reason = "THAW_BY_START_PAGE_ABILITY";
    } else if (abilityInfo.type == AppExecFwk::AbilityType::EXTENSION &&
               abilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::SERVICE) {
        reason = "THAW_BY_START_SERVICE_EXTENSION";
    } else if (abilityInfo.type == AppExecFwk::AbilityType::EXTENSION &&
               AAFwk::UIExtensionWrapper::IsUIExtension(abilityInfo.extensionAbilityType)) {
        reason = "THAW_BY_START_UI_EXTENSION";
    } else {
        reason = "THAW_BY_START_NOT_PAGE_ABILITY";
    }
    return reason;
}

void ResSchedUtil::ReportAbilityIntentExemptionInfoToRSS(int32_t callerUid, int32_t callerPid)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    std::unordered_map<std::string, std::string> eventParams {
        {"uid", std::to_string(callerUid)},
        {"duration", std::to_string(INTENT_EXEMPTION_DURATION)}
    };

    TAG_LOGD(AAFwkTag::DEFAULT, "report Intent ExemptionInfo Uid:%{public}d", callerUid);
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(
        ResourceSchedule::ResType::RES_TYPE_INTENT_CTRL_APP, 0, eventParams);
#endif
}

void ResSchedUtil::ReportEventToRSS(const int32_t uid, const std::string &bundleName, const std::string &reason,
    const int32_t pid, const int32_t callerPid, bool isCreateFromImage)
{
    TAG_LOGD(AAFwkTag::DEFAULT, "ReportEventToRSS---%{public}d_%{public}s_%{public}d callerPid=%{public}d",
        uid, bundleName.c_str(), pid, callerPid);
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    uint32_t resType = ResourceSchedule::ResType::SYNC_RES_TYPE_THAW_ONE_APP;
    nlohmann::json payload;
    payload.emplace("uid", uid);
    payload.emplace("pid", pid);
    payload.emplace("bundleName", bundleName);
    payload.emplace("reason", reason);
    payload.emplace("callerPid", callerPid);
    payload.emplace("isCreateFromImage", isCreateFromImage);
    nlohmann::json reply;
    TAG_LOGD(AAFwkTag::DEFAULT, "call");
    ResourceSchedule::ResSchedClient::GetInstance().ReportSyncEvent(resType, 0, payload, reply);
#endif
}

void ResSchedUtil::PromotePriorityToRSS(int32_t callerUid, int32_t callerPid, const std::string &targetBundleName,
    int32_t targetUid, int32_t targetPid)
{
    TAG_LOGI(AAFwkTag::DEFAULT, "PromotePriorityToRSS---%{public}d_%{public}d_%{public}s_%{public}d_%{public}d",
        callerUid, callerPid, targetBundleName.c_str(), targetUid, targetPid);
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    uint32_t resType = ResourceSchedule::ResType::RES_TYPE_SA_PULL_APP_IDENTIFIER;
    std::unordered_map<std::string, std::string> eventParams {
        { "callerUid", std::to_string(callerUid) },
        { "callerPid", std::to_string(callerPid) },
        { "targetBundleName", targetBundleName },
        { "targetUid", std::to_string(targetUid) },
        { "targetPid", std::to_string(targetPid) },
    };
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(resType, 0, eventParams);
#endif
}

void ResSchedUtil::GetAllFrozenPidsFromRSS(std::unordered_set<int32_t> &frozenPids)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    uint32_t resType = ResourceSchedule::ResType::SYNC_RES_TYPE_GET_ALL_SUSPEND_STATE;
    nlohmann::json payload;
    nlohmann::json reply;
    TAG_LOGD(AAFwkTag::DEFAULT, "call");
    int32_t ret = ResourceSchedule::ResSchedClient::GetInstance().ReportSyncEvent(resType, 0, payload, reply);
    if (ret != 0 || !reply.contains("allSuspendState") || !reply["allSuspendState"].is_array()) {
        TAG_LOGE(AAFwkTag::DEFAULT, "ReportSyncEvent fail");
        return;
    }

    for (nlohmann::json &appObj : reply["allSuspendState"]) {
        // Here can get uid if needed
        if (!appObj.contains("pidsState") || !appObj["pidsState"].is_array()) {
            continue;
        }

        for (nlohmann::json &pidObj : appObj["pidsState"]) {
            if (!pidObj.contains("pid") || !pidObj["pid"].is_number() ||
                !pidObj.contains("isFrozen") || !pidObj["isFrozen"].is_boolean()) {
                break;
            }
            int32_t pid = pidObj["pid"].get<int32_t>();
            bool isFrozen = pidObj["isFrozen"].get<bool>();
            if (isFrozen) {
                frozenPids.insert(pid);
            }
        }
    }
    if (frozenPids.empty()) {
        TAG_LOGW(AAFwkTag::DEFAULT, "empty frozen pids");
    }
#endif
}

bool ResSchedUtil::CheckShouldForceKillProcess(int32_t pid, const std::string& bundleName)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    uint32_t resType = ResourceSchedule::ResType::SYNC_RES_TYPE_SHOULD_FORCE_KILL_PROCESS;
    nlohmann::json payload;
    nlohmann::json reply;
    payload.emplace("pid", pid);
    ResourceSchedule::ResSchedClient::GetInstance().ReportSyncEvent(resType, 0, payload, reply);
    if (!reply.contains("ShouldForceKillProcess") || !reply["ShouldForceKillProcess"].is_number_integer()) {
        return true;
    }
    return reply["ShouldForceKillProcess"].get<int32_t>() == 1;
#else
    return true;
#endif
}

void ResSchedUtil::ReportLoadingEventToRss(LoadingStage stage, int32_t pid, int32_t uid,
    int64_t timeDuration, int64_t abilityRecordId)
{
    ReportLoadingEventToRss(stage, pid, uid, timeDuration, abilityRecordId, {});
}

void ResSchedUtil::ReportLoadingEventToRss(LoadingStage stage, int32_t pid, int32_t uid,
    int64_t timeDuration, int64_t abilityRecordId,
    const std::unordered_map<std::string, std::string> &extraParams)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    uint32_t resType = ResourceSchedule::ResType::RES_TYPE_KEY_PERF_SCENE;
    std::unordered_map<std::string, std::string> eventParams {
        { "extType", "10015"},
        { "pid", std::to_string(pid) },
        { "uid", std::to_string(uid) },
    };
    if (timeDuration > 0) { // millisecond
        eventParams.emplace("timeoutDuration", std::to_string(timeDuration));
    }
    if (abilityRecordId > 0) {
        eventParams.emplace("abilityRecordId", std::to_string(abilityRecordId));
    }
    eventParams.insert(extraParams.begin(), extraParams.end());

    int64_t type = static_cast<int64_t>(stage);
    TAG_LOGD(AAFwkTag::DEFAULT, "call");
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(resType, type, eventParams);
#endif
}

std::unordered_set<std::string> ResSchedUtil::GetNWebPreloadSet() const
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    uint32_t resType = ResourceSchedule::ResType::SYNC_RES_TYPE_GET_NWEB_PRELOAD_SET;
    nlohmann::json payload;
    nlohmann::json reply;
    ResourceSchedule::ResSchedClient::GetInstance().ReportSyncEvent(resType, 0, payload, reply);
    if (!reply.contains("NWebPreloadSet")) {
        TAG_LOGW(AAFwkTag::DEFAULT, "does not get preload process set");
        return {};
    }
    auto jsonObj = reply["NWebPreloadSet"];
    return { jsonObj.begin(), jsonObj.end() };
#else
    return {};
#endif
}
} // namespace AAFwk
} // namespace OHOS
