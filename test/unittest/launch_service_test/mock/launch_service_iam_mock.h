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

#ifndef LAUNCH_SERVICE_IAM_MOCK_H
#define LAUNCH_SERVICE_IAM_MOCK_H

#include "user_launch_service_client.h"
#include "iam_common_defines.h"

namespace OHOS::UserIam::UserAuth {
class LaunchServiceIAMMock : public UserLaunchServiceClient {
public:
    static LaunchServiceIAMMock &Instance();

    void Reset()
    {
        result = SUCCESS;
        listCalls = 0;
        unregisterCalls = 0;
        lastUid = -1;
        lastState = LaunchServiceState::DEACTIVATE;
        lastToken.clear();
        records.clear();
        pending.reset();
    }

    void RegisterLaunchService(const std::vector<LaunchServiceRegisterInfo> &,
        const std::shared_ptr<LaunchServiceListCallback> &callback) override
    {
        callback->OnResult(result, records);
    }

    void UnRegisterLaunchService(int32_t, const std::optional<LaunchServiceKey> &,
        const std::shared_ptr<LaunchServiceCallback> &callback) override
    {
        ++unregisterCalls;
        callback->OnResult(result);
    }

    void GetLaunchServiceInfo(int32_t, const std::optional<LaunchServiceKey> &,
        const std::shared_ptr<LaunchServiceListCallback> &callback) override
    {
        callback->OnResult(result, records);
    }

    void GetLaunchServiceList(const std::optional<int32_t> &,
        const std::shared_ptr<LaunchServiceListCallback> &callback) override
    {
        ++listCalls;
        callback->OnResult(result, records);
    }

    void SetLaunchServiceState(int32_t uid, const std::vector<uint8_t> &token, LaunchServiceState state,
        const std::shared_ptr<LaunchServiceCallback> &callback) override
    {
        lastUid = uid;
        lastToken = token;
        lastState = state;
        pending = callback;
    }

    int32_t SetDaemonAllowedlist(int32_t, const std::vector<std::string> &, bool) override
    {
        return result;
    }

    int32_t SetDaemonForbiddenlist(int32_t, const std::vector<std::string> &, bool) override
    {
        return result;
    }

    int32_t result = SUCCESS;
    int32_t listCalls = 0;
    int32_t unregisterCalls = 0;
    int32_t lastUid = -1;
    std::vector<uint8_t> lastToken;
    LaunchServiceState lastState = LaunchServiceState::DEACTIVATE;
    std::vector<LaunchServiceFlatInfo> records;
    std::shared_ptr<LaunchServiceCallback> pending;
};
} // namespace OHOS::UserIam::UserAuth
#endif
