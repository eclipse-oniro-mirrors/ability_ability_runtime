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

#include "app_mgr_client.h"

#include <cstdio>
#include <string>
#include <unistd.h>

#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"

#include "app_mgr_interface.h"
#include "app_service_manager.h"

namespace OHOS {
namespace AppExecFwk {
AppMgrClient::AppMgrClient()
{}

AppMgrClient::~AppMgrClient()
{}

void AppMgrClient::GetRunningProcessInfoByToken(const sptr<IRemoteObject>& token, RunningProcessInfo& info)
{}

void AppMgrClient::GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo& info) const {}

AppMgrResultCode AppMgrClient::LoadAbility(sptr<IRemoteObject> token, sptr<IRemoteObject> preToken,
    const AbilityInfo& abilityInfo, const ApplicationInfo& appInfo, const AAFwk::Want& want,
    int32_t abilityRecordId)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::TerminateAbility(const sptr<IRemoteObject>& token, bool clearMissionFlag)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::UpdateAbilityState(const sptr<IRemoteObject>& token, const AbilityState state,
    bool isFromScreenOffBackground)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::RegisterAppStateCallback(const sptr<IAppStateCallback>& callback)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::KillProcessByAbilityToken(const sptr<IRemoteObject>& token)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::KillProcessesByUserId(int32_t userId, bool isNeedSendAppSpawnMsg,
    sptr<AAFwk::IUserCallback> callback)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::KillProcessesByPids(const std::vector<int32_t> &pids,
    const std::string &reason, bool subProcess, bool isKillPrecedeStart)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::KillApplication(
    const std::string& bundleName, bool clearPageStack = false, int32_t appIndex = 0,
    const std::string& reason = "KillApplication")
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::ClearUpApplicationData(const std::string& bundleName, const int32_t userId)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::GetAllRunningProcesses(std::vector<RunningProcessInfo>& info)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::GetAllRenderProcesses(std::vector<RenderProcessInfo>& info)
{
    return AppMgrResultCode::RESULT_OK;
}

AppMgrResultCode AppMgrClient::ConnectAppMgrService()
{
    return AppMgrResultCode::RESULT_OK;
}

// void AppMgrClient::SetServiceManager(std::unique_ptr<AppServiceManager> serviceMgr)
// {}

void AppMgrClient::AbilityAttachTimeOut(const sptr<IRemoteObject>& token)
{}

void AppMgrClient::PrepareTerminate(const sptr<IRemoteObject>& token, bool clearMissionFlag)
{}
}  // namespace AppExecFwk
}  // namespace OHOS
