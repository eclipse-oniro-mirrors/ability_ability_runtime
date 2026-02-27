/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_APPLICATION_PROXY_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_APPLICATION_PROXY_H

#include "gmock/gmock.h"
#include "semaphore_ex.h"
#include "app_scheduler_proxy.h"
#include "app_malloc_info.h"

namespace OHOS {
namespace AppExecFwk {
class MockApplicationProxy : public AppSchedulerProxy {
public:
    MockApplicationProxy(const sptr<IRemoteObject> &impl) : AppSchedulerProxy(impl) {}
    MOCK_METHOD0(ScheduleForegroundApplication, bool());
    MOCK_METHOD0(ScheduleBackgroundApplication, void());
    MOCK_METHOD1(ScheduleTerminateApplication, void(bool));
    MOCK_METHOD1(ScheduleShrinkMemory, void(const int));
    MOCK_METHOD0(ScheduleLowMemory, void());
    MOCK_METHOD2(ScheduleMemoryLevel, void(int32_t level, bool isShellCall));
    MOCK_METHOD2(ScheduleHeapMemory, void(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo));
    MOCK_METHOD2(ScheduleLaunchApplication, void(const AppLaunchData&, const Configuration& config));
    MOCK_METHOD4(ScheduleLaunchAbility, void(const AbilityInfo&, const sptr<IRemoteObject>&,
        const std::shared_ptr<AAFwk::Want>&, int32_t));
    MOCK_METHOD2(ScheduleCleanAbility, void(const sptr<IRemoteObject>&, bool));
    MOCK_METHOD1(ScheduleProfileChanged, void(const Profile&));
    MOCK_METHOD2(ScheduleConfigurationUpdated, void(const Configuration&, ConfigUpdateReason));
    MOCK_METHOD0(ScheduleProcessSecurityExit, void());
    MOCK_METHOD1(ScheduleAbilityStage, void(const HapModuleInfo&));
    MOCK_METHOD2(ScheduleUpdateApplicationInfoInstalled, void(const ApplicationInfo&, const std::string&));
    MOCK_METHOD2(ScheduleAcceptWant, void(const AAFwk::Want& want, const std::string& moduleName));
    MOCK_METHOD1(SchedulePrepareTerminate, void(const std::string &moduleName));
    MOCK_METHOD2(ScheduleNewProcessRequest, void(const AAFwk::Want& want, const std::string& moduleName));
    MOCK_METHOD3(ScheduleNotifyLoadRepairPatch, int32_t(const std::string& bundleName,
        const sptr<IQuickFixCallback>& callback, const int32_t recordId));
    MOCK_METHOD2(ScheduleNotifyHotReloadPage, int32_t(const sptr<IQuickFixCallback>& callback, const int32_t recordId));
    MOCK_METHOD3(ScheduleNotifyUnLoadRepairPatch, int32_t(const std::string& bundleName,
        const sptr<IQuickFixCallback>& callback, const int32_t recordId));
    MOCK_METHOD1(ScheduleNotifyAppFault, int32_t(const FaultData &faultData));
    MOCK_METHOD1(AttachAppDebug, void(bool isDebugFromLocal));
    MOCK_METHOD0(DetachAppDebug, void());
    MOCK_METHOD2(SetAppWaitingDebug, int32_t(const std::string &bundleName, bool isPersist));
    MOCK_METHOD0(CancelAppWaitingDebug, int32_t());
    MOCK_METHOD1(GetWaitingDebugApp, int32_t(std::vector<std::string> &debugInfoList));
    MOCK_METHOD1(IsWaitingDebugApp, bool(const std::string &bundleName));
    MOCK_METHOD0(ClearNonPersistWaitingDebugFlag, void());
    MOCK_METHOD1(ScheduleDumpIpcStart, int32_t(std::string &result));
    MOCK_METHOD1(ScheduleDumpIpcStop, int32_t(std::string &result));
    MOCK_METHOD1(ScheduleDumpIpcStat, int32_t(std::string &result));
    MOCK_METHOD0(IsMemorySizeSufficient, bool());
    MOCK_METHOD1(ScheduleJsHeapMemory, void(OHOS::AppExecFwk::JsHeapDumpInfo &info));
    MOCK_METHOD1(ScheduleDumpFfrt, int32_t(std::string& result));
    MOCK_METHOD2(ScheduleDumpArkWeb, int32_t(const std::string &customArgs, std::string &result));
    MOCK_METHOD0(ScheduleClearPageStack, void());
    MOCK_METHOD0(ScheduleCacheProcess, void());

    void Post()
    {
        lock_.Post();
    }

    void Wait()
    {
        lock_.Wait();
    }

    void ShrinkMemory(const int level)
    {
        shrinkLevel_ = level;
        lock_.Post();
    }

    int GetShrinkLevel() const
    {
        return shrinkLevel_;
    }

    void LaunchApplication(const AppLaunchData& launchData, const Configuration& config)
    {
        launchData_ = launchData;
        lock_.Post();
    }

    bool CompareAppLaunchData(const AppLaunchData& launchData) const
    {
        if (launchData_.GetApplicationInfo().name != launchData.GetApplicationInfo().name) {
            return false;
        }
        if (launchData_.GetProfile().GetName() != launchData.GetProfile().GetName()) {
            return false;
        }
        if (launchData_.GetProcessInfo().GetProcessName() != launchData.GetProcessInfo().GetProcessName()) {
            return false;
        }
        return true;
    }

    void LaunchAbility(const AbilityInfo& info, const sptr<IRemoteObject>&)
    {
        abilityInfo_ = info;
        lock_.Post();
    }

    bool CompareAbilityInfo(const AbilityInfo& info) const
    {
        return (info.name == abilityInfo_.name);
    }

    void ProfileChanged(const Profile& profile)
    {
        profile_ = profile;
        lock_.Post();
    }

    bool CompareProfile(const Profile& profile) const
    {
        return (profile.GetName() == profile_.GetName());
    }

    int32_t ScheduleChangeAppGcState(int32_t state, uint64_t tid = 0) override
    {
        return 0;
    }

private:
    Semaphore lock_;
    volatile int shrinkLevel_ = 0;
    AppLaunchData launchData_;
    AbilityInfo abilityInfo_;
    Profile profile_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_APPLICATION_PROXY_H
