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

#ifndef OHOS_ABILITY_RUNTIME_DAEMON_SERVICE_MANAGER_CLIENT_H
#define OHOS_ABILITY_RUNTIME_DAEMON_SERVICE_MANAGER_CLIENT_H

#include <functional>
#include <vector>

#include "launch_service_info.h"
#include "launch_service_observer.h"

namespace OHOS {
namespace AbilityRuntime {
class DaemonServiceManagerClient final {
public:
    static DaemonServiceManagerClient &GetInstance();

    int32_t NotifyResidentStartupCompleted();
    int32_t Register(const LaunchServiceDescriptor &descriptor);
    int32_t Unregister(const LaunchServiceDescriptor &descriptor);
    int32_t Start(const LaunchServiceDescriptor &descriptor);
    int32_t Stop(const LaunchServiceDescriptor &descriptor);
    int32_t GetInformation(const LaunchServiceDescriptor &descriptor, LaunchServiceInformation &information);
    int32_t GetAllRegisteredInformation(int32_t userId, std::vector<LaunchServiceInformation> &informations);
    void SetLaunchServicesEnabled(const std::string &bundleName, int32_t userId, int32_t appIndex,
        const std::vector<uint8_t> &token, bool enable, std::function<void(int32_t)> completion);

    int32_t RegisterObserver(const sptr<ILaunchServiceObserver> &observer);
    int32_t UnregisterObserver(const sptr<ILaunchServiceObserver> &observer);

private:
    DaemonServiceManagerClient() = default;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_DAEMON_SERVICE_MANAGER_CLIENT_H
