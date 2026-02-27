/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_APP_SCHEDULER_H
#define MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_APP_SCHEDULER_H

#include "gmock/gmock.h"
#include "refbase.h"
#include "iremote_object.h"
#include "app_scheduler.h"
#include "app_launch_data.h"
#include "configuration.h"
#include "app_malloc_info.h"

namespace OHOS {
namespace AAFwk {
class MockAppScheduler : public AppScheduler {
public:
    MockAppScheduler() = default;
    virtual ~MockAppScheduler() = default;

    MOCK_METHOD0(ScheduleForegroundApplication, bool());
    MOCK_METHOD0(ScheduleBackgroundApplication, void());
    MOCK_METHOD1(ScheduleTerminateApplication, void(bool));
    MOCK_METHOD2(ScheduleLaunchApplication, void(const AppExecFwk::AppLaunchData&, const Configuration& config));
    MOCK_METHOD4(ScheduleLaunchAbility, void(const AppExecFwk::AbilityInfo&, const sptr<IRemoteObject>&,
        const std::shared_ptr<AAFwk::Want>&, int32_t));
    MOCK_METHOD2(ScheduleCleanAbility, void(const sptr<IRemoteObject>&, bool));
    MOCK_METHOD1(ScheduleProfileChanged, void(const AppExecFwk::Profile&));
    MOCK_METHOD1(ScheduleConfigurationUpdated, void(const AppExecFwk::Configuration& config));
    MOCK_METHOD1(ScheduleShrinkMemory, void(const int));
    MOCK_METHOD2(ScheduleMemoryLevel, void(int32_t level, bool isShellCall));
    MOCK_METHOD2(ScheduleHeapMemory, void(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo));
    MOCK_METHOD0(ScheduleLowMemory, void());
    MOCK_METHOD1(ScheduleNotifyAppFault, int32_t(const FaultData &));
    MOCK_METHOD2(ScheduleChangeAppGcState, int32_t(int32_t state, int32_t tid));
    MOCK_METHOD1(RegisterAppDebugListener, int32_t(const sptr<AppExecFwk::IAppDebugListener> &listener));
    MOCK_METHOD1(UnregisterAppDebugListener, int32_t(const sptr<AppExecFwk::IAppDebugListener> &listener));
    MOCK_METHOD2(AttachAppDebug, int32_t(const std::string &bundleName, bool isDebugFromLocal));
    MOCK_METHOD1(DetachAppDebug, int32_t(const std::string &bundleName));
    MOCK_METHOD1(RegisterAbilityDebugResponse, int32_t(const sptr<AppExecFwk::IAbilityDebugResponse> &response));
    MOCK_METHOD1(ScheduleJsHeapMemory, void(OHOS::AppExecFwk::JsHeapDumpInfo &info));
    MOCK_METHOD1(ScheduleCjHeapMemory, void(OHOS::AppExecFwk::CjHeapDumpInfo &info));
    MOCK_METHOD1(SetWatchdogBackgroundStatus, void(bool status));
    MOCK_METHOD2(OnLoadAbilityFinished, void(uint64_t, int32_t));
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_APP_SCHEDULER_H
