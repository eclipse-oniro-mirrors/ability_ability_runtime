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

#ifndef MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_CLIENT_H
#define MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_CLIENT_H

#include <gmock/gmock.h>
#include "app_mgr_client.h"
#include "param.h"

namespace OHOS {
namespace AppExecFwk {
class AppMgrClientMock : public AppMgrClient {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"AppMgrClientMock");
    AppMgrClientMock()
    {}
    virtual ~AppMgrClientMock()
    {}
    MOCK_METHOD0(ConnectAppMgrService, AppMgrResultCode());
    MOCK_METHOD1(RegisterAppStateCallback, AppMgrResultCode(const sptr<IAppStateCallback> &callback));
    MOCK_METHOD4(LoadAbility, AppMgrResultCode(const AbilityInfo&, const ApplicationInfo&,
        const AAFwk::Want&, AbilityRuntime::LoadParam));
    MOCK_METHOD2(TerminateAbility, AppMgrResultCode(const sptr<IRemoteObject>&, bool));
    MOCK_METHOD2(UpdateExtensionState, AppMgrResultCode(const sptr<IRemoteObject> &token, const ExtensionState state));
    MOCK_METHOD4(UpdateApplicationInfoInstalled, AppMgrResultCode(const std::string &bundleName, const int uid,
        const std::string&, bool));
    MOCK_METHOD0(UpdateApplicationInfoInstalledDone, AppMgrResultCode());
    MOCK_METHOD1(VerifyKillProcessPermission, int32_t(const std::string &bundleName));
    MOCK_METHOD4(KillApplication, AppMgrResultCode(const std::string&, const bool clearPageStack, int32_t appIndex,
        const std::string&));
    MOCK_METHOD3(KillApplicationByUid,
        AppMgrResultCode(const std::string &bundleName, const int uid, const std::string&));
    MOCK_METHOD3(ClearUpApplicationData, AppMgrResultCode(const std::string&, int32_t appCloneIndex, int32_t userId));
    MOCK_METHOD1(StartupResidentProcess, void(const std::vector<AppExecFwk::BundleInfo> &bundleInfos));
    MOCK_METHOD5(StartSpecifiedAbility, void(const AAFwk::Want&, const AppExecFwk::AbilityInfo&, int32_t,
        const std::string&, bool));
    MOCK_METHOD1(GetAllRunningProcesses, AppMgrResultCode(std::vector<RunningProcessInfo> &info));
    MOCK_METHOD1(GetAllRenderProcesses, AppMgrResultCode(std::vector<RenderProcessInfo> &info));
    MOCK_METHOD1(GetAllChildrenProcesses, AppMgrResultCode(std::vector<ChildProcessInfo> &info));
    MOCK_METHOD2(GetProcessRunningInfosByUserId, AppMgrResultCode(
        std::vector<RunningProcessInfo> &info, int32_t userId));
    MOCK_METHOD4(StartUserTestProcess, int(
        const AAFwk::Want &want, const sptr<IRemoteObject> &observer, const BundleInfo &bundleInfo, int32_t userId));
    MOCK_METHOD3(FinishUserTest, int(
        const std::string &msg, const int64_t &resultCode, const std::string &bundleName));
    MOCK_METHOD2(UpdateConfiguration, AppMgrResultCode(const Configuration &config, const int32_t userId));
    MOCK_METHOD1(GetConfiguration, AppMgrResultCode(Configuration& config));
    MOCK_METHOD2(GetAbilityRecordsByProcessID, int(
        const int pid, std::vector<sptr<IRemoteObject>> &tokens));
    MOCK_METHOD2(SetGameSAPrelaunch, AppMgrResultCode(const sptr<IRemoteObject> &token, bool isGameSAPrelaunch));
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_CLIENT_H
