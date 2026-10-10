/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "appmgrservicefirst_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "app_mgr_service.h"
#undef private

#include "ability_record.h"
#include "parcel.h"
#include "securec.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_THREE = 3;
constexpr size_t U32_AT_SIZE = 4;
constexpr uint8_t ENABLE = 2;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
}
uint32_t GetU32Data(const char* ptr)
{
    // convert fuzz input data to an integer
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[ENABLE] << OFFSET_TWO) |
        ptr[INPUT_THREE];
}
sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    AppMgrService* appMgrService = new AppMgrService();
    std::shared_ptr<AppMgrServiceInner> innerService = std::make_shared<AppMgrServiceInner>();
    appMgrService->appMgrServiceState_.serviceRunningState = ServiceRunningState::STATE_NOT_START;
    appMgrService->SetInnerService(innerService);
    appMgrService->OnStart();
    sptr<IConfigurationObserver> configurationObserver;
    appMgrService->RegisterConfigurationObserver(configurationObserver);
    appMgrService->UnregisterConfigurationObserver(configurationObserver);
    sptr<IApplicationStateObserver> applicationStateObserver;
    appMgrService->RegisterApplicationStateObserver(applicationStateObserver);
    appMgrService->UnregisterApplicationStateObserver(applicationStateObserver);
    sptr<INativeChildNotify> nativeChildNotify;
    appMgrService->RegisterNativeChildExitNotify(nativeChildNotify);
    appMgrService->UnregisterNativeChildExitNotify(nativeChildNotify);
    pid_t pid = static_cast<pid_t>(GetU32Data(data));
    // appMgrService->AddAppDeathRecipient(pid);
    appMgrService->QueryServiceState();
    sptr<IRemoteObject> app = nullptr;
    appMgrService->AttachApplication(app);
    std::vector<BundleInfo> bundleInfos;
    appMgrService->StartupResidentProcess(bundleInfos);
    Parcel wantParcel;
    Want* want = nullptr;
    if (wantParcel.WriteBuffer(data, size)) {
        want = Want::Unmarshalling(wantParcel);
    }
    std::string renderParam(data, size);
    int32_t ipcFd = static_cast<int32_t>(GetU32Data(data));
    int32_t sharedFd = static_cast<int32_t>(GetU32Data(data));
    int32_t crashFd = static_cast<int32_t>(GetU32Data(data));
    pid_t renderPid = static_cast<pid_t>(GetU32Data(data));
    appMgrService->StartRenderProcess(renderParam, ipcFd, sharedFd, crashFd, renderPid);
    appMgrService->PreStartNWebSpawnProcess();
    sptr<IRemoteObject> scheduler = nullptr;
    appMgrService->AttachRenderProcess(scheduler);
    bool isContinuousTask = *data % ENABLE;
    appMgrService->SetContinuousTaskProcess(static_cast<int32_t>(pid), isContinuousTask);
    int32_t recordId = static_cast<int32_t>(GetU32Data(data));
    appMgrService->ApplicationForegrounded(recordId);
    appMgrService->AddAbilityStageDone(recordId);
    int fd = static_cast<int>(GetU32Data(data));
    std::vector<std::u16string> args;
    appMgrService->Dump(fd, args);
    std::string result(data, size);
    appMgrService->Dump(args, result);
    appMgrService->ShowHelp(args, result);
    std::string flag(data, size);
    appMgrService->ScheduleAcceptWantDone(recordId, *want, flag);
    Configuration config;
    appMgrService->UpdateConfiguration(config);
    appMgrService->GetAmsMgr();
    std::vector<RunningProcessInfo> info;
    appMgrService->GetAllRunningProcesses(info);
    int32_t userId = static_cast<int32_t>(GetU32Data(data));
    appMgrService->GetProcessRunningInfosByUserId(info, userId);
    int32_t level = static_cast<int32_t>(GetU32Data(data));
    appMgrService->NotifyMemoryLevel(level);
    std::vector<AppStateData> list;
    appMgrService->GetForegroundApplications(list);
    std::vector<sptr<IRemoteObject>> tokens;
    appMgrService->GetAbilityRecordsByProcessID(static_cast<int>(pid), tokens);
    int status = static_cast<int>(GetU32Data(data));
    appMgrService->GetRenderProcessTerminationStatus(renderPid, status);
    appMgrService->GetConfiguration(config);
    std::string bundleName(data, size);
    appMgrService->GetAppRunningStateByBundleName(bundleName);
    sptr<IQuickFixCallback> callback;
    appMgrService->NotifyLoadRepairPatch(bundleName, callback);
    appMgrService->NotifyHotReloadPage(bundleName, callback);
    appMgrService->NotifyUnLoadRepairPatch(bundleName, callback);
    appMgrService->ApplicationBackgrounded(recordId);
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    appMgrService->AbilityCleaned(token);
    int32_t appCloneIndex = static_cast<int32_t>(GetU32Data(data));
    appMgrService->ClearUpApplicationData(bundleName, appCloneIndex);
    appMgrService->ApplicationTerminated(recordId);
    std::string msg(data, size);
    int64_t resultCode = static_cast<int64_t>(GetU32Data(data));
    appMgrService->FinishUserTest(msg, resultCode, bundleName);
    appMgrService->OnStop();
    return appMgrService->IsReady();
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
    if (memcpy_s(ch, size, data, size) != EOK) {
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
