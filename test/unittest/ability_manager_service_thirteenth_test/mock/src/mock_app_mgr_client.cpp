/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "appmgr/app_mgr_client.h"

#include "mock_my_status.h"

namespace OHOS {
namespace AppExecFwk {
int32_t AppMgrClient::GetRunningProcessInfoByPid(const pid_t pid,
    OHOS::AppExecFwk::RunningProcessInfo &info) const
{
    auto &status = AAFwk::MyStatus::GetInstance();
    info.state_ = status.amcProcessState_;
    return status.amcGetProcessInfoRet_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
