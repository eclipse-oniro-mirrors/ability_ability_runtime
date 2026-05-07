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
class MockAppMgrClient : public AppMgrClient {
public:
    MockAppMgrClient();
    ~MockAppMgrClient();
    MOCK_METHOD4(LoadAbility, AppMgrResultCode(const AbilityInfo&, const ApplicationInfo&,
        const AAFwk::Want&, AbilityRuntime::LoadParam));
    MOCK_METHOD2(TerminateAbility, AppMgrResultCode(const sptr<IRemoteObject>&, bool));
    MOCK_METHOD3(UpdateAbilityState, AppMgrResultCode(const sptr<IRemoteObject>& token, const AbilityState state,
        bool isFromScreenOffBackground));
    MOCK_METHOD4(KillApplication, AppMgrResultCode(const std::string&, const bool clearPageStack, int32_t,
        const std::string&));
    MOCK_METHOD1(KillProcessByAbilityToken, AppMgrResultCode(const sptr<IRemoteObject>& token));
    MOCK_METHOD3(KillProcessesByUserId, AppMgrResultCode(int32_t userId, bool isNeedSendAppSpawnMsg,
        sptr<AAFwk::IUserCallback> callback));
    MOCK_METHOD1(AbilityAttachTimeOut, void(const sptr<IRemoteObject>& token));
    MOCK_METHOD2(GetRunningProcessInfoByToken, void((const sptr<IRemoteObject>& token,
        AppExecFwk::RunningProcessInfo& info)));
    MOCK_METHOD1(GetAllRunningProcesses, AppMgrResultCode(std::vector<RunningProcessInfo>& info));
    MOCK_METHOD1(GetAllRunningInstanceKeysBySelf, AppMgrResultCode(std::vector<std::string> &instanceKeys));
    MOCK_METHOD3(GetAllRunningInstanceKeysByBundleName, AppMgrResultCode(const std::string &bundleName,
        std::vector<std::string> &instanceKeys, int32_t userId));

    AppMgrResultCode GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo>& info, int32_t userId);

    AppMgrResultCode ConnectAppMgrService() override;
    AppMgrResultCode RegisterAppStateCallback(const sptr<IAppStateCallback>& callback) override;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MODULETEST_OHOS_ABILITY_RUNTIME_MOCK_APP_MGR_CLIENT_H
