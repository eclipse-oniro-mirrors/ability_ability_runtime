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

#include "app_mgr_client.h"

namespace OHOS {
namespace AppExecFwk {
int32_t AppMgrClient::isAppRunningReturnCode = ERR_OK;
int32_t AppMgrClient::usedKillAppTimes = 0;
bool AppMgrClient::isAppRunningReturnValue = false;
bool AppMgrClient::isKeepAliveAppservice = true;
int32_t AppMgrClient::ret = -1;
std::vector<RunningProcessInfo> AppMgrClient::infos = {};

AppMgrClient::AppMgrClient() {}

AppMgrClient::~AppMgrClient() {}

int32_t AppMgrClient::KillApplication(const std::string &bundleName, bool clearPageStack, int32_t appIndex,
    const std::string& reason, int32_t userId)
{
    usedKillAppTimes++;
    return ERR_OK;
}

int32_t AppMgrClient::IsAppRunningByBundleNameAndUserId(const std::string &bundleName, int32_t userId,
    bool &isRunning)
{
    isRunning = isAppRunningReturnValue;
    return isAppRunningReturnCode;
}

void AppMgrClient::SetKeepAliveDkv(const std::string &bundleName, bool enable, int32_t uid) {}

void AppMgrClient::SetKeepAliveAppService(const std::string &bundleName, bool enable, int32_t uid)
{
    isKeepAliveAppservice = enable;
}

int32_t AppMgrClient::GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId)
{
    info = infos;
    return ret;
}
}  // namespace AppExecFwk
}  // namespace OHOS
