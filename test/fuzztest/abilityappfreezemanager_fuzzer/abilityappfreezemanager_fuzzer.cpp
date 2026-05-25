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

#include "abilityappfreezemanager_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "appfreeze_manager.h"
#undef private

#include "fault_data.h"

using namespace OHOS::AppExecFwk;

namespace {
// Named constants for switch cases
constexpr int32_t CMD_APPFREEZE_HANDLE = 0;
constexpr int32_t CMD_APPFREEZE_HANDLE_WITH_STACK = 1;
constexpr int32_t CMD_LIFECYCLE_TIMEOUT_HANDLE = 2;
constexpr int32_t CMD_WRITE_TO_FILE = 3;
constexpr int32_t CMD_IS_HANDLE_APPFREEZE = 4;
constexpr int32_t CMD_IS_PROCESS_DEBUG = 5;
constexpr int32_t CMD_DELETE_STACK = 6;
constexpr int32_t CMD_CANCEL_APPFREEZE_DETECT = 7;
constexpr int32_t CMD_REMOVE_DEATH_PROCESS = 8;
constexpr int32_t CMD_RESET_APPFREEZE_STATE = 9;
constexpr int32_t CMD_IS_VALID_FREEZE_FILTER = 10;
constexpr int32_t CMD_REPORT_SYS_EVENTS = 11;
constexpr int32_t CMD_REGISTER_APP_KILL_TIME = 12;
constexpr int32_t CMD_CHECK_IN_BACKGROUND = 13;
constexpr int32_t CMD_CHECK_APPFREEZE_HAPPEND = 14;
constexpr int32_t CMD_CHECK_NEED_RECORD = 15;
constexpr int32_t CMD_IS_BETA_VERSION = 16;
constexpr int32_t CMD_INSERT_KILL_THREAD = 17;
constexpr int32_t CMD_IS_SKIP_DETECT = 18;
constexpr int32_t CMD_GET_PROCESS_KILL_REASON = 19;
constexpr int32_t CMD_GET_FREEZE_EXIT_REASON = 20;
constexpr int32_t CMD_UPDATE_FREEZE_EXCLUDED_PID = 21;
constexpr int32_t CMD_IS_FREEZE_EXCLUDED_PID = 22;
constexpr int32_t CMD_SET_FREEZE_STATE = 23;
constexpr int32_t CMD_GET_REPORT_TIMES = 24;
constexpr int32_t CMD_GET_LAST_OCCUR_TIME = 25;
constexpr int32_t CMD_INIT_WARNING_CPU_INFO = 26;
constexpr int32_t CMD_IS_NEED_IGNORE_FREEZE_EVENT = 27;
constexpr int32_t CMD_MAX = 28;

// Named constants for event name selection
constexpr size_t EVENT_LIFECYCLE_TIMEOUT = 0;
constexpr size_t EVENT_LIFECYCLE_HALF_TIMEOUT = 1;
constexpr size_t EVENT_LIFECYCLE_TIMEOUT_WARNING = 2;
constexpr size_t EVENT_LIFECYCLE_HALF_TIMEOUT_WARNING = 3;
constexpr size_t EVENT_THREAD_BLOCK_3S = 4;
constexpr size_t EVENT_THREAD_BLOCK_6S = 5;
constexpr size_t EVENT_APP_INPUT_BLOCK = 6;
constexpr size_t EVENT_BUSINESS_THREAD_BLOCK_3S = 7;
constexpr size_t EVENT_BUSINESS_THREAD_BLOCK_6S = 8;
constexpr size_t EVENT_BUSINESS_INPUT_BLOCK = 9;
constexpr size_t EVENT_BG_FREEZE_WARNING = 10;
constexpr size_t EVENT_RANDOM_STRING = 11;
constexpr size_t EVENT_MAX = 12;

// Named constants for kill ID scenarios
constexpr int32_t KILL_ID_INVALID = 0;
constexpr int32_t KILL_ID_NEGATIVE = 1;
constexpr int32_t KILL_ID_POSITIVE = 2;
constexpr int32_t KILL_ID_MAX = 3;

// Named constants for kill ID values
constexpr int32_t INVALID_KILL_ID_VALUE = -2;
constexpr int32_t KILL_ID_NEGATIVE_MIN = -1000;
constexpr int32_t KILL_ID_NEGATIVE_MAX = -3;
constexpr int32_t KILL_ID_POSITIVE_MIN = 1;
constexpr int32_t KILL_ID_POSITIVE_MAX = 1000;

// Named constants for operation loop bounds
constexpr size_t MIN_OP_COUNT = 1;
constexpr size_t MAX_OP_COUNT = 20;

constexpr size_t STRING_MAX_LEN = 128;

std::string GetEventName(FuzzedDataProvider &fdp)
{
    size_t choice = fdp.ConsumeIntegralInRange<size_t>(0, EVENT_MAX - 1);
    switch (choice) {
        case EVENT_LIFECYCLE_TIMEOUT:
            return AppFreezeType::LIFECYCLE_TIMEOUT;
        case EVENT_LIFECYCLE_HALF_TIMEOUT:
            return AppFreezeType::LIFECYCLE_HALF_TIMEOUT;
        case EVENT_LIFECYCLE_TIMEOUT_WARNING:
            return AppFreezeType::LIFECYCLE_TIMEOUT_WARNING;
        case EVENT_LIFECYCLE_HALF_TIMEOUT_WARNING:
            return AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING;
        case EVENT_THREAD_BLOCK_3S:
            return AppFreezeType::THREAD_BLOCK_3S;
        case EVENT_THREAD_BLOCK_6S:
            return AppFreezeType::THREAD_BLOCK_6S;
        case EVENT_APP_INPUT_BLOCK:
            return AppFreezeType::APP_INPUT_BLOCK;
        case EVENT_BUSINESS_THREAD_BLOCK_3S:
            return AppFreezeType::BUSSINESS_THREAD_BLOCK_3S;
        case EVENT_BUSINESS_THREAD_BLOCK_6S:
            return AppFreezeType::BUSSINESS_THREAD_BLOCK_6S;
        case EVENT_BUSINESS_INPUT_BLOCK:
            return AppFreezeType::BUSINESS_INPUT_BLOCK;
        case EVENT_BG_FREEZE_WARNING:
            return AppFreezeType::BG_FREEZE_WARNING;
        case EVENT_RANDOM_STRING:
            return fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
        default:
            break;
    }
    return fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
}

std::string GetLifecycleEventName(FuzzedDataProvider &fdp)
{
    size_t choice = fdp.ConsumeIntegralInRange<size_t>(0, EVENT_LIFECYCLE_HALF_TIMEOUT_WARNING);
    switch (choice) {
        case EVENT_LIFECYCLE_TIMEOUT:
            return AppFreezeType::LIFECYCLE_TIMEOUT;
        case EVENT_LIFECYCLE_HALF_TIMEOUT:
            return AppFreezeType::LIFECYCLE_HALF_TIMEOUT;
        case EVENT_LIFECYCLE_TIMEOUT_WARNING:
            return AppFreezeType::LIFECYCLE_TIMEOUT_WARNING;
        case EVENT_LIFECYCLE_HALF_TIMEOUT_WARNING:
            return AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING;
        default:
            break;
    }
    return AppFreezeType::LIFECYCLE_TIMEOUT;
}

AppfreezeManager::AppInfo BuildAppInfo(FuzzedDataProvider &fdp)
{
    AppfreezeManager::AppInfo appInfo;
    appInfo.isOccurException = fdp.ConsumeBool();
    appInfo.pid = fdp.ConsumeIntegral<int32_t>();
    appInfo.uid = fdp.ConsumeIntegral<int32_t>();
    appInfo.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    appInfo.processName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    return appInfo;
}

FaultData BuildFaultData(FuzzedDataProvider &fdp)
{
    FaultData faultData;
    faultData.errorObject.name = GetEventName(fdp);
    faultData.errorObject.message =
        fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    faultData.errorObject.stack =
        fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    faultData.errorObject.mainStack =
        fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    faultData.faultType = FaultDataType::APP_FREEZE;
    faultData.pid = fdp.ConsumeIntegral<int32_t>();
    faultData.tid = fdp.ConsumeIntegral<int32_t>();
    faultData.eventId = fdp.ConsumeIntegral<int32_t>();
    faultData.schedTime = fdp.ConsumeIntegral<uint64_t>();
    faultData.detectTime = fdp.ConsumeIntegral<uint64_t>();
    faultData.appStatus = fdp.ConsumeIntegral<int32_t>();
    faultData.isInForeground = fdp.ConsumeBool();
    faultData.isEnableMainThreadSample = fdp.ConsumeBool();
    faultData.reportLifecycleToFreeze = fdp.ConsumeBool();
    faultData.appfreezeInfo = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    faultData.appRunningUniqueId =
        fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    faultData.procStatm = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    return faultData;
}

void FuzzAppfreezeHandle(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr)
{
    FaultData faultData = BuildFaultData(fdp);
    AppfreezeManager::AppInfo appInfo = BuildAppInfo(fdp);
    mgr->AppfreezeHandle(faultData, appInfo);
}

void FuzzAppfreezeHandleWithStack(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr)
{
    FaultData faultData = BuildFaultData(fdp);
    AppfreezeManager::AppInfo appInfo = BuildAppInfo(fdp);
    mgr->AppfreezeHandleWithStack(faultData, appInfo);
}

void FuzzLifecycleTimeoutHandle(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr)
{
    AppfreezeManager::ParamInfo info;
    info.needKillProcess = fdp.ConsumeBool();
    info.typeId = fdp.ConsumeBool()
        ? AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT
        : AppfreezeManager::TypeAttribute::NORMAL_TIMEOUT;
    info.pid = fdp.ConsumeIntegral<int32_t>();
    info.eventName = fdp.ConsumeBool()
        ? GetLifecycleEventName(fdp)
        : GetEventName(fdp);
    info.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    info.msg = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    mgr->LifecycleTimeoutHandle(info);
}

void FuzzWriteToFile(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr)
{
    std::string fileName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    std::string content = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    mgr->WriteToFile(fileName, content);
}

void FuzzBasicQueryMethods(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr)
{
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    int32_t pid = fdp.ConsumeIntegral<int32_t>();
    mgr->IsHandleAppfreeze(bundleName);
    mgr->IsProcessDebug(pid, bundleName);
    mgr->DeleteStack(pid);
    mgr->IsValidFreezeFilter(pid, bundleName);
}

void FuzzFreezeFilterOperations(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr)
{
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    int32_t pid = fdp.ConsumeIntegral<int32_t>();
    mgr->CancelAppFreezeDetect(pid, bundleName);
    mgr->RemoveDeathProcess(bundleName);
    mgr->ResetAppfreezeState(pid, bundleName);
}

void FuzzReportAndKillTime(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr)
{
    int32_t pid = fdp.ConsumeIntegral<int32_t>();
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    uint64_t killTime = fdp.ConsumeIntegral<uint64_t>();
    mgr->ReportAppFreezeSysEvents(pid, bundleName);
    mgr->RegisterAppKillTime(pid, killTime);
}

void FuzzCheckInBackground(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr)
{
    FaultData faultData;
    faultData.errorObject.name = fdp.ConsumeBool()
        ? AppFreezeType::THREAD_BLOCK_6S
        : GetEventName(fdp);
    faultData.isInForeground = fdp.ConsumeBool();
    mgr->CheckInBackGround(faultData);
}

void FuzzCheckAppfreezeHappend(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr)
{
    std::string key = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    std::string eventName = GetEventName(fdp);
    mgr->CheckAppfreezeHappend(key, eventName);
}

void FuzzCheckNeedRecord(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr)
{
    std::string eventName = GetEventName(fdp);
    mgr->CheckNeedRecordAppRunningUnquieId(eventName);
}

void FuzzIsBetaVersion(const std::shared_ptr<AppfreezeManager> &mgr)
{
    mgr->IsBetaVersion();
}

void FuzzInsertKillThread(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr)
{
    int32_t state = fdp.ConsumeIntegral<int32_t>();
    int32_t pid = fdp.ConsumeIntegral<int32_t>();
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    mgr->InsertKillThread(state, pid, uid, bundleName);
}

void FuzzIsSkipDetect(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr)
{
    int32_t pid = fdp.ConsumeIntegral<int32_t>();
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    std::string eventName = GetEventName(fdp);
    mgr->IsSkipDetect(pid, uid, bundleName, eventName);
}

void FuzzGetProcessKillReason(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr)
{
    int32_t pid = fdp.ConsumeIntegral<int32_t>();
    std::string killMsg = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    size_t killChoice = fdp.ConsumeIntegralInRange<size_t>(0, KILL_ID_MAX - 1);
    int32_t killId = 0;
    switch (killChoice) {
        case KILL_ID_INVALID:
            killId = INVALID_KILL_ID_VALUE;
            break;
        case KILL_ID_NEGATIVE:
            killId = fdp.ConsumeIntegralInRange<int32_t>(
                KILL_ID_NEGATIVE_MIN, KILL_ID_NEGATIVE_MAX);
            break;
        case KILL_ID_POSITIVE:
            killId = fdp.ConsumeIntegralInRange<int32_t>(
                KILL_ID_POSITIVE_MIN, KILL_ID_POSITIVE_MAX);
            break;
        default:
            killId = fdp.ConsumeIntegral<int32_t>();
            break;
    }
    mgr->GetProcessKillReason(killId, pid, killMsg);
}

void FuzzGetFreezeExitReason(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr)
{
    std::string eventName = GetEventName(fdp);
    mgr->GetFreezeExitReason(eventName);
}

void FuzzFreezeExcludedPid(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr)
{
    int32_t targetPid = fdp.ConsumeIntegral<int32_t>();
    int32_t profilerPid = fdp.ConsumeIntegral<int32_t>();
    bool isAdd = fdp.ConsumeBool();
    mgr->UpdateFreezeExcludedPid(isAdd, targetPid, profilerPid);
    mgr->IsFreezeExcludedPid(targetPid);
}

void FuzzFreezeStateOperations(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr)
{
    std::string key = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    std::string eventName = GetEventName(fdp);
    int state = fdp.ConsumeIntegral<int>();
    mgr->SetFreezeState(key, state, eventName);
    mgr->GetReportTimes(key);
    mgr->GetLastOccurTime(key);
}

void FuzzInitWarningCpuInfo(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr)
{
    FaultData faultData = BuildFaultData(fdp);
    AppfreezeManager::AppInfo appInfo = BuildAppInfo(fdp);
    mgr->InitWarningCpuInfo(faultData, appInfo);
}

void FuzzIsNeedIgnoreFreezeEvent(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr)
{
    std::string key = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    std::string eventName = GetEventName(fdp);
    mgr->IsNeedIgnoreFreezeEvent(key, eventName);
}

void DispatchCommandSecond(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr, int32_t cmd);

void DispatchCommand(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr, int32_t cmd)
{
    switch (cmd) {
        case CMD_APPFREEZE_HANDLE:
            FuzzAppfreezeHandle(fdp, mgr);
            break;
        case CMD_APPFREEZE_HANDLE_WITH_STACK:
            FuzzAppfreezeHandleWithStack(fdp, mgr);
            break;
        case CMD_LIFECYCLE_TIMEOUT_HANDLE:
            FuzzLifecycleTimeoutHandle(fdp, mgr);
            break;
        case CMD_WRITE_TO_FILE:
            FuzzWriteToFile(fdp, mgr);
            break;
        case CMD_IS_HANDLE_APPFREEZE:
        case CMD_IS_PROCESS_DEBUG:
        case CMD_DELETE_STACK:
        case CMD_IS_VALID_FREEZE_FILTER:
            FuzzBasicQueryMethods(fdp, mgr);
            break;
        case CMD_CANCEL_APPFREEZE_DETECT:
        case CMD_REMOVE_DEATH_PROCESS:
        case CMD_RESET_APPFREEZE_STATE:
            FuzzFreezeFilterOperations(fdp, mgr);
            break;
        default:
            DispatchCommandSecond(fdp, mgr, cmd);
            break;
    }
}

void DispatchCommandSecond(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr, int32_t cmd)
{
    switch (cmd) {
        case CMD_REPORT_SYS_EVENTS:
        case CMD_REGISTER_APP_KILL_TIME:
            FuzzReportAndKillTime(fdp, mgr);
            break;
        case CMD_CHECK_IN_BACKGROUND:
            FuzzCheckInBackground(fdp, mgr);
            break;
        case CMD_CHECK_APPFREEZE_HAPPEND:
            FuzzCheckAppfreezeHappend(fdp, mgr);
            break;
        case CMD_CHECK_NEED_RECORD:
            FuzzCheckNeedRecord(fdp, mgr);
            break;
        case CMD_IS_BETA_VERSION:
            FuzzIsBetaVersion(mgr);
            break;
        case CMD_INSERT_KILL_THREAD:
            FuzzInsertKillThread(fdp, mgr);
            break;
        case CMD_IS_SKIP_DETECT:
            FuzzIsSkipDetect(fdp, mgr);
            break;
        case CMD_GET_PROCESS_KILL_REASON:
            FuzzGetProcessKillReason(fdp, mgr);
            break;
        case CMD_GET_FREEZE_EXIT_REASON:
            FuzzGetFreezeExitReason(fdp, mgr);
            break;
        case CMD_UPDATE_FREEZE_EXCLUDED_PID:
        case CMD_IS_FREEZE_EXCLUDED_PID:
            FuzzFreezeExcludedPid(fdp, mgr);
            break;
        case CMD_SET_FREEZE_STATE:
        case CMD_GET_REPORT_TIMES:
        case CMD_GET_LAST_OCCUR_TIME:
            FuzzFreezeStateOperations(fdp, mgr);
            break;
        case CMD_INIT_WARNING_CPU_INFO:
            FuzzInitWarningCpuInfo(fdp, mgr);
            break;
        case CMD_IS_NEED_IGNORE_FREEZE_EVENT:
            FuzzIsNeedIgnoreFreezeEvent(fdp, mgr);
            break;
        default:
            break;
    }
}

void FuzzMainHandlers(FuzzedDataProvider &fdp,
    const std::shared_ptr<AppfreezeManager> &mgr)
{
    size_t opCount = fdp.ConsumeIntegralInRange<size_t>(
        MIN_OP_COUNT, MAX_OP_COUNT);
    for (size_t i = 0; i < opCount && fdp.remaining_bytes() > 0; ++i) {
        int32_t cmd = fdp.ConsumeIntegralInRange<int32_t>(0, CMD_MAX - 1);
        DispatchCommand(fdp, mgr, cmd);
    }
}
} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < 1) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    auto mgr = AppfreezeManager::GetInstance();
    if (mgr == nullptr) {
        return 0;
    }
    FuzzMainHandlers(fdp, mgr);
    return 0;
}
