/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef MOCK_APP_MGR_CLIENT_H
#define MOCK_APP_MGR_CLIENT_H

#include "running_process_info.h"
#include "mock_flag.h"

namespace OHOS {
namespace AppExecFwk {

class AppMgrClient {
    DECLARE_DELAYED_SINGLETON(AppMgrClient);
public:
    int32_t GetRunningProcessInfoByPid(const pid_t pid, RunningProcessInfo &info)
    {
        if (MockFlag::getRunningProcessInfoRet != 0) {
            return MockFlag::getRunningProcessInfoRet;
        }
        info.state_ = static_cast<AppProcessState>(MockFlag::processState);
        info.isPreForeground = MockFlag::isPreForeground;
        return 0;
    }
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // MOCK_APP_MGR_CLIENT_H
