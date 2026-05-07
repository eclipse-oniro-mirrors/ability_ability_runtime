/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_SERVICE_INNER_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_SERVICE_INNER_H

#include "gmock/gmock.h"
#include "semaphore_ex.h"
#include "app_mgr_service_inner.h"
#include "param.h"

namespace OHOS {
namespace AppExecFwk {
class MockAppMgrServiceInner : public AppMgrServiceInner {
public:
    MockAppMgrServiceInner() : lock_(0)
    {}
    virtual ~MockAppMgrServiceInner()
    {}

    MOCK_METHOD4(LoadAbility, void(std::shared_ptr<AbilityInfo> abilityInfo, std::shared_ptr<ApplicationInfo> appInfo,
            std::shared_ptr<AAFwk::Want> want, std::shared_ptr<AbilityRuntime::LoadParam> loadParam));
    MOCK_METHOD2(AttachApplication, void(const pid_t pid, const sptr<IAppScheduler>& app));
    MOCK_METHOD1(ApplicationForegrounded, void(const int32_t recordId));
    MOCK_METHOD1(ApplicationBackgrounded, void(const int32_t recordId));
    MOCK_METHOD1(ApplicationTerminated, void(const int32_t recordId));
    MOCK_METHOD3(UpdateAbilityState, void(const sptr<IRemoteObject>& token, const AbilityState state,
        bool isFromScreenOffBackground));
    MOCK_METHOD2(TerminateAbility, void(const sptr<IRemoteObject>& token, bool clearMissionFlag));
    MOCK_METHOD4(UpdateApplicationInfoInstalled, int(const std::string&, const int uid, const std::string&, bool));
    MOCK_METHOD4(KillApplication, int32_t(const std::string& bundleName, const bool clearPageStack, int32_t appIndex,
        const std::string& reason));
    MOCK_METHOD3(KillApplicationByUid, int(const std::string&, const int uid, const std::string&));
    MOCK_METHOD1(AbilityTerminated, void(const sptr<IRemoteObject>& token));
    MOCK_METHOD5(ClearUpApplicationData,
        int32_t(const std::string&, const int32_t, const pid_t, int32_t appCloneIndex, int32_t userId));
    MOCK_METHOD1(IsBackgroundRunningRestricted, int32_t(const std::string&));
    MOCK_METHOD1(GetAllRunningProcesses, int32_t(std::vector<RunningProcessInfo>&));
    MOCK_METHOD1(GetAllRunningInstanceKeysBySelf, int32_t(std::vector<std::string> &instanceKeys));
    MOCK_METHOD3(GetAllRunningInstanceKeysByBundleName, int32_t(const std::string &bundleName,
        std::vector<std::string> &instanceKeys, int32_t userId));
    MOCK_METHOD1(GetAllRenderProcesses, int32_t(std::vector<RenderProcessInfo>&));
    MOCK_METHOD3(PreloadExtension, int32_t(const AAFwk::Want&, int32_t, int32_t));
#ifdef SUPPORT_CHILD_PROCESS
    MOCK_METHOD1(GetAllChildrenProcesses, int(std::vector<ChildProcessInfo>&));
#endif // SUPPORT_CHILD_PROCESS
    MOCK_METHOD1(RegisterAppStateCallback, void(const sptr<IAppStateCallback>& callback));
    MOCK_METHOD0(StopAllProcess, void());
    MOCK_CONST_METHOD0(QueryAppSpawnConnectionState, SpawnConnectionState());
    MOCK_CONST_METHOD2(AddAppDeathRecipient, void(const pid_t pid, const sptr<AppDeathRecipient>& appDeathRecipient));
    MOCK_METHOD1(KillProcessByAbilityToken, void(const sptr<IRemoteObject>& token));
    MOCK_METHOD3(KillProcessesByUserId, void(int32_t userId, bool isNeedSendAppSpawnMsg,
        sptr<AAFwk::IUserCallback> callback));
    MOCK_METHOD1(AddAbilityStageDone, void(const int32_t recordId));
    MOCK_METHOD0(GetConfiguration, std::shared_ptr<Configuration>());
    MOCK_METHOD2(IsSharedBundleRunning, bool(const std::string &bundleName, uint32_t versionCode));
    MOCK_METHOD3(GetBundleNameByPid, int32_t(const int pid, std::string &bundleName, int32_t &uid));
#ifdef SUPPORT_CHILD_PROCESS
    MOCK_METHOD3(StartChildProcess, int32_t(const pid_t hostPid, pid_t &childPid, const ChildProcessRequest &request));
    MOCK_METHOD1(GetChildProcessInfoForSelf, int32_t(ChildProcessInfo &info));
#endif // SUPPORT_CHILD_PROCESS
    MOCK_METHOD4(PreloadApplication, int32_t(const std::string&, int32_t, AppExecFwk::PreloadMode, int32_t));
#ifdef SUPPORT_CHILD_PROCESS
    MOCK_METHOD4(CreateNativeChildProcess, int32_t(const pid_t hostPid, const std::string &libName,
        const sptr<IRemoteObject> &callback, const ChildProcessRequest &request));
#endif // SUPPORT_CHILD_PROCESS
    MOCK_METHOD1(DumpIpcAllStart, int(std::string& result));
    MOCK_METHOD1(DumpIpcAllStop, int(std::string& result));
    MOCK_METHOD1(DumpIpcAllStat, int(std::string& result));
    MOCK_METHOD2(DumpIpcStart, int(const int32_t pid, std::string& result));
    MOCK_METHOD2(DumpIpcStop, int(const int32_t pid, std::string& result));
    MOCK_METHOD2(DumpIpcStat, int(const int32_t pid, std::string& result));
    MOCK_METHOD2(DumpFfrt, int(const std::vector<int32_t>& pid, std::string& result));
    MOCK_METHOD3(DumpArkWeb, int32_t(const std::vector<int32_t> &pids, const std::string &customArgs,
        std::string &result));
    MOCK_METHOD1(RegisterKiaInterceptor, int32_t(const sptr<IKiaInterceptor> &interceptor));
    MOCK_METHOD2(CheckIsKiaProcess, int32_t(pid_t pid, bool &isKia));
    MOCK_METHOD1(NotifyProcMemoryLevel, int32_t(const std::map<pid_t, MemoryLevel> &procLevelMap));
    MOCK_METHOD2(DumpHeapMemory, int32_t(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo));
    MOCK_METHOD1(GetProcessRunningInformation, int32_t(RunningProcessInfo &info));
    MOCK_METHOD4(PreloadApplicationByPhase, int32_t(const std::string&, int32_t, int32_t, AppExecFwk::PreloadPhase));
    MOCK_METHOD2(NotifyPreloadAbilityStateChanged, int32_t(sptr<IRemoteObject>, bool));
    MOCK_METHOD3(CheckPreloadAppRecordExist, bool(const std::string&, int32_t, int32_t));
    MOCK_CONST_METHOD0(IsFoundationCall, bool());
    MOCK_METHOD2(QueryRunningSharedBundles, int32_t(pid_t pid, std::map<std::string, uint32_t> &sharedBundles));
    MOCK_METHOD2(NotifyPageShow, int32_t(const sptr<IRemoteObject> &token, const PageStateData &pageStateData));
    MOCK_METHOD1(RegisterRenderStateObserver, int32_t(const sptr<IRenderStateObserver> &observer));
    MOCK_METHOD2(RegisterApplicationStateObserver, int32_t(const sptr<IApplicationStateObserver> &observer,
        const std::vector<std::string> &bundleNameList));
    MOCK_METHOD5(MakeImage, int32_t(const AAFwk::Want &want, int32_t userId, AppExecFwk::PreloadMode preloadMode,
        int32_t appIndex, sptr<IImageErrorHandler>));
    MOCK_METHOD2(DestroyImage, int32_t(uint64_t checkpointId, sptr<IImageErrorHandler>));
    MOCK_METHOD1(RegisterImageProcessStateObserver, int32_t(const sptr<IImageProcessStateObserver> &observer));
    MOCK_METHOD1(UnregisterImageProcessStateObserver, int32_t(const sptr<IImageProcessStateObserver> &observer));
    MOCK_METHOD2(GetAllAbilityInfos, int32_t(const int32_t pid, std::vector<AppExecFwk::AbilityStateData> &infos));

    void StartSpecifiedAbility(const AAFwk::Want&, const AppExecFwk::AbilityInfo&, int32_t, const std::string&, bool)
    {}

    void Post()
    {
        if (currentCount_ > 1) {
            currentCount_--;
        } else {
            lock_.Post();
            currentCount_ = count_;
        }
    }
    // for mock function return int32_t
    int32_t Post4Int()
    {
        if (currentCount_ > 1) {
            currentCount_--;
        } else {
            lock_.Post();
            currentCount_ = count_;
        }
        return 0;
    }

    void Wait()
    {
        lock_.Wait();
    }

    void SetWaitCount(const int32_t waitCount)
    {
        count_ = waitCount;
        currentCount_ = waitCount;
    }

    int32_t OpenAppSpawnConnection() override
    {
        return 0;
    }

private:
    Semaphore lock_;
    int32_t count_ = 1;
    int32_t currentCount_ = 1;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_SERVICE_INNER_H
