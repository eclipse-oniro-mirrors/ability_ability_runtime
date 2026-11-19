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

#include "abilityappmgrapprunningmanager_fuzzer.h"
#include "ability_record.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "app_running_manager.h"
#ifdef SUPPORT_CHILD_PROCESS
#include "child_process_record.h"
#endif // SUPPORT_CHILD_PROCESS
#include "app_running_record.h"
#undef private
#include "securec.h"
#include "ability_record.h"
#include "kill_process_config.h"


using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_TWO = 2;
constexpr int INPUT_THREE = 3;
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
constexpr uint8_t ENABLE = 2;
}
uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[INPUT_TWO] << OFFSET_TWO) |
        ptr[INPUT_THREE];
}

sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

void DoSomethingInterestingWithMyAPIadda(const char* data, size_t size)
{
    std::shared_ptr<AppRunningManager> manager = std::make_shared<AppRunningManager>();
    pid_t pidApps = static_cast<pid_t>(GetU32Data(data));
    std::string jsonStr(data, size);
    int uid = static_cast<int>(GetU32Data(data));
    manager->SetAbilityForegroundingFlagToAppRecord(pidApps);
    int64_t eventId = static_cast<int64_t>(GetU32Data(data));
    manager->HandleTerminateTimeOut(eventId);
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner;
    manager->HandleAbilityAttachTimeOut(token, appMgrServiceInner);
    bool clearMissionFlag = *data % ENABLE;
    manager->TerminateAbility(token, clearMissionFlag, appMgrServiceInner);
    ApplicationInfo appInfos;
    std::string moduleName;
    manager->ProcessUpdateApplicationInfoInstalled(appInfos, moduleName);
    std::list<pid_t> pids;
    KillProcessConfig config;
    config.clearPageStack = *data % ENABLE;
    manager->ProcessExitByBundleNameAndUid(jsonStr, uid, pids, config);
    int32_t userId = static_cast<int32_t>(GetU32Data(data));
    manager->GetPidsByUserId(userId, pids);
    manager->PrepareTerminate(token, clearMissionFlag);
    sptr<IRemoteObject> abilityToken = GetFuzzAbilityToken();
    manager->GetTerminatingAppRunningRecord(abilityToken);
    AppExecFwk::RunningProcessInfo processInfo;
    manager->GetRunningProcessInfoByToken(token, processInfo);
    OHOS::AppExecFwk::RunningProcessInfo infoByPid;
    manager->GetRunningProcessInfoByPid(pidApps, infoByPid);
    std::regex re;
    manager->ClipStringContent(re, jsonStr, jsonStr);
    manager->GetAppRunningRecordByRenderPid(pidApps);
    manager->OnRenderProcessExitedByPid(pidApps);
    manager->GetAppRunningStateByBundleName(jsonStr);
    sptr<IQuickFixCallback> callback;
    manager->NotifyLoadRepairPatch(jsonStr, callback);
    manager->NotifyHotReloadPage(jsonStr, callback);
    manager->NotifyUnLoadRepairPatch(jsonStr, callback);
    std::shared_ptr<ApplicationInfo> infoAPP;
    int32_t recordId = static_cast<int32_t>(GetU32Data(data));
    OHOS::AppExecFwk::AppRunningRecord foregroundingRecord(infoAPP, recordId, jsonStr);
}

void DoSomethingInterestingWithMyAPIaddb(const char* data, size_t size)
{
    std::shared_ptr<AppRunningManager> manager = std::make_shared<AppRunningManager>();
    std::string jsonStr(data, size);
    int32_t recordId = static_cast<int32_t>(GetU32Data(data));
    std::shared_ptr<ApplicationInfo> infoAPP;
    OHOS::AppExecFwk::AppRunningRecord foregroundingRecord(infoAPP, recordId, jsonStr);
    manager->IsApplicationFirstForeground(foregroundingRecord);
    manager->IsApplicationBackground(foregroundingRecord);
    manager->IsApplicationFirstFocused(foregroundingRecord);
    manager->IsApplicationUnfocused(foregroundingRecord.GetUid());
    bool isAttachDebug = *data % ENABLE;
    manager->SetAttachAppDebug(jsonStr, isAttachDebug, false);
    bool isDetachDebug = *data % ENABLE;
    manager->GetAppDebugInfosByBundleName(jsonStr, isDetachDebug);
    std::vector<sptr<IRemoteObject>> abilityTokens;
    manager->GetAbilityTokensByBundleName(jsonStr, abilityTokens);
    pid_t pidApps = static_cast<pid_t>(GetU32Data(data));
#ifdef SUPPORT_CHILD_PROCESS
    manager->GetAppRunningRecordByChildProcessPid(pidApps);
#endif // SUPPORT_CHILD_PROCESS
#ifdef SUPPORT_CHILD_PROCESS
    manager->OnChildProcessExitedByPid(pidApps);
#endif // SUPPORT_CHILD_PROCESS
    manager->GetAllAppRunningRecordCountByBundleName(jsonStr);
    auto uid = static_cast<int32_t>(GetU32Data(data));
    manager->SignRestartAppFlag(uid, jsonStr);
    manager->GetAppRunningUniqueIdByPid(pidApps, jsonStr);
    std::vector<pid_t> hostPids;
    manager->GetAllUIExtensionRootHostPid(pidApps, hostPids);
    std::vector<pid_t> providerPids;
    pid_t hostPid = static_cast<pid_t>(GetU32Data(data));
    manager->GetAllUIExtensionProviderPid(hostPid, providerPids);
    int32_t uiExtensionAbilityId = static_cast<int32_t>(GetU32Data(data));
    pid_t providerPid = static_cast<pid_t>(GetU32Data(data));
    manager->AddUIExtensionLauncherItem(uiExtensionAbilityId, hostPid, providerPid);
    manager->RemoveUIExtensionLauncherItem(pidApps);
    manager->RemoveUIExtensionLauncherItemById(uiExtensionAbilityId);
    manager->DumpIpcAllStart(jsonStr);
    manager->DumpIpcAllStop(jsonStr);
    manager->DumpIpcAllStat(jsonStr);
}

void DoSomethingInterestingWithMyAPIaddc(const char* data, size_t size)
{
    std::shared_ptr<AppRunningManager> manager = std::make_shared<AppRunningManager>();
    std::string jsonStr(data, size);
    int32_t pidDump = static_cast<int32_t>(GetU32Data(data));
    manager->DumpIpcStart(pidDump, jsonStr);
    manager->DumpIpcStop(pidDump, jsonStr);
    manager->DumpIpcStat(pidDump, jsonStr);
    std::vector<int32_t> pidFrt;
    manager->DumpFfrt(pidFrt, jsonStr);
    int32_t uids = static_cast<int32_t>(GetU32Data(data));
    std::shared_ptr<ApplicationInfo> appInfosd = std::make_shared<ApplicationInfo>();
    std::set<std::shared_ptr<AppRunningRecord>> cachedSet;
    manager->IsAppProcessesAllCached(jsonStr, uids, cachedSet);
    int64_t eventId = static_cast<int64_t>(GetU32Data(data));
    manager->GetAbilityRunningRecord(eventId);
    std::shared_ptr<AppRunningRecord> appRecord;
    AppExecFwk::RunningProcessInfo infoRecord;
    manager->AssignRunningProcessInfoByAppRecord(appRecord, infoRecord);
}

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    std::shared_ptr<AppRunningManager> manager = std::make_shared<AppRunningManager>();
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string jsonStr(data, size);
    BundleInfo bundleInfo;
    manager->CreateAppRunningRecord(appInfo, jsonStr, bundleInfo, "");
    int uid = static_cast<int>(GetU32Data(data));
    manager->CheckAppRunningRecordIsExist(jsonStr, jsonStr, uid, bundleInfo, jsonStr);
    auto accessTokenId = static_cast<uint32_t>(GetU32Data(data));
    manager->IsAppExist(accessTokenId);
    int32_t appCloneIndex = static_cast<int32_t>(GetU32Data(data));
    bool isRunning = *data % ENABLE;
    manager->CheckAppCloneRunningRecordIsExistByBundleName(jsonStr, appCloneIndex, isRunning);
    pid_t pidApps = static_cast<pid_t>(GetU32Data(data));
    manager->GetAppRunningRecordByPid(pidApps);
    sptr<IRemoteObject> abilityToken = GetFuzzAbilityToken();
    manager->GetAppRunningRecordByAbilityToken(abilityToken);
    wptr<IRemoteObject> remote;
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner;
    manager->OnRemoteDied(remote, appMgrServiceInner);
    manager->GetAppRunningRecordMap();
    int32_t recordId = static_cast<int32_t>(GetU32Data(data));
    manager->RemoveAppRunningRecordById(recordId);
    manager->ClearAppRunningRecordMap();
    std::list<pid_t> pids;
    bool clearPageStack = *data % ENABLE;
    manager->ProcessExitByBundleName(jsonStr, pids, clearPageStack);
    std::vector<AppStateData> list;
    manager->GetForegroundApplications(list);
    Configuration config;
    manager->UpdateConfiguration(config);
    manager->UpdateConfigurationByBundleName(config, jsonStr, appCloneIndex);
    int32_t level = static_cast<int32_t>(GetU32Data(data));
    manager->NotifyMemoryLevel(level);
    std::map<pid_t, MemoryLevel> procLevelMap;
    manager->NotifyProcMemoryLevel(procLevelMap);
    int32_t pidDump = static_cast<int32_t>(GetU32Data(data));
    OHOS::AppExecFwk::MallocInfo mallocInfo;
    manager->DumpHeapMemory(pidDump, mallocInfo);
    OHOS::AppExecFwk::JsHeapDumpInfo info;
    manager->DumpJsHeapMemory(info);
    DoSomethingInterestingWithMyAPIadda(data, size);
    DoSomethingInterestingWithMyAPIaddb(data, size);
    DoSomethingInterestingWithMyAPIaddc(data, size);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    char* ch = static_cast<char*>(malloc(size + 1));
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size + 1, data, size) != EOK) {
        std::cout << "copy failed." << std::endl;
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}

