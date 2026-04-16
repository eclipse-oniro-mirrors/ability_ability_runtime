/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "app_spawn_client.h"
#include "mock_my_status.h"
namespace OHOS {
namespace AppExecFwk {
AppSpawnClient::AppSpawnClient(bool isNWebSpawn)
{
}

AppSpawnClient::AppSpawnClient(const char* serviceName)
{
}

AppSpawnClient::~AppSpawnClient()
{
}

ErrCode AppSpawnClient::OpenConnection()
{
    return ERR_OK;
}

void AppSpawnClient::CloseConnection()
{
}

SpawnConnectionState AppSpawnClient::QueryConnectionState() const
{
    return state_;
}

AppSpawnClientHandle AppSpawnClient::GetAppSpawnClientHandle() const
{
    return nullptr;
}

int32_t AppSpawnClient::SetDacInfo(const AppSpawnStartMsg &startMsg, AppSpawnReqMsgHandle reqHandle)
{
    return 0;
}

int32_t AppSpawnClient::SetMountPermission(const AppSpawnStartMsg &startMsg, AppSpawnReqMsgHandle reqHandle)
{
    return 0;
}

int32_t AppSpawnClient::SetStartFlags(const AppSpawnStartMsg &startMsg, AppSpawnReqMsgHandle reqHandle)
{
    return 0;
}

int32_t AppSpawnClient::SetStrictMode(const AppSpawnStartMsg &startMsg, const AppSpawnReqMsgHandle &reqHandle)
{
    return ERR_OK;
}

int32_t AppSpawnClient::AppspawnSetExtMsg(const AppSpawnStartMsg &startMsg, AppSpawnReqMsgHandle reqHandle)
{
    return 0;
}

int32_t AppSpawnClient::AppspawnSetExtMsgMore(const AppSpawnStartMsg &startMsg, AppSpawnReqMsgHandle reqHandle)
{
    return 0;
}

int32_t AppSpawnClient::AppspawnSetExtMsgSec(const AppSpawnStartMsg &startMsg, AppSpawnReqMsgHandle reqHandle)
{
    return 0;
}

int32_t AppSpawnClient::AppspawnCreateDefaultMsg(const AppSpawnStartMsg &startMsg, AppSpawnReqMsgHandle reqHandle)
{
    return 0;
}

bool AppSpawnClient::VerifyMsg(const AppSpawnStartMsg &startMsg)
{
    return false;
}

int32_t AppSpawnClient::PreStartNWebSpawnProcess()
{
    return 0;
}

int32_t AppSpawnClient::StartProcess(const AppSpawnStartMsg &startMsg, pid_t &pid)
{
    return AAFwk::MyStatus::GetInstance().startProcess_;
}

int32_t AppSpawnClient::StartImageProcess(const AppSpawnStartMsg &startMsg, pid_t &pid, uint64_t &checkpointId)
{
    return 0;
}

int32_t AppSpawnClient::SendAppSpawnUninstallDebugHapMsg(int32_t userId)
{
    return 0;
}

int32_t AppSpawnClient::GetRenderProcessTerminationStatus(const AppSpawnStartMsg &startMsg, int &status)
{
    return 0;
}

#ifdef SUPPORT_CHILD_PROCESS
int32_t AppSpawnClient::SetChildProcessTypeStartFlag(const AppSpawnReqMsgHandle &reqHandle,
    int32_t childProcessType)
{
    return ERR_OK;
}
#endif // SUPPORT_CHILD_PROCESS

int32_t AppSpawnClient::SetExtMsgFds(const AppSpawnReqMsgHandle &reqHandle,
    const std::map<std::string, int32_t> &fds)
{
    return ERR_OK;
}

int32_t AppSpawnClient::SetIsolationModeFlag(const AppSpawnStartMsg &startMsg, const AppSpawnReqMsgHandle &reqHandle)
{
    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS
