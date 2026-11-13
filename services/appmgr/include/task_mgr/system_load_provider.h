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

#ifndef OHOS_APP_EXECFWK_TASK_MGR_SYSTEM_LOAD_PROVIDER_H
#define OHOS_APP_EXECFWK_TASK_MGR_SYSTEM_LOAD_PROVIDER_H

#include <cstdint>
#include <functional>
#include <memory>

namespace OHOS {
namespace AppExecFwk {

/**
 * @brief Abstraction of the system load source. The default implementation is backed by the
 *        resource-schedule fusion level (performance-power-thermal); unit tests inject a stub
 *        returning a fixed level. Isolating the source behind this interface keeps the judge
 *        free of resource-schedule headers.
 */
class ISystemLoadProvider {
public:
    virtual ~ISystemLoadProvider() = default;

    /**
     * @brief Registers the listener notified on every fusion level change.
     * @param callback Indicates the level callback (0~7); invoked once with the current
     *        level right after a successful registration when the level is available.
     * @return Returns true if the listener will receive level changes; false if the
     *         registration failed and the judge must fall back to the default level.
     */
    virtual bool RegisterListener(const std::function<void(int32_t)> &callback) = 0;

    /**
     * @brief Unregisters the listener; a no-op when nothing is registered.
     */
    virtual void UnregisterListener() = 0;

    /**
     * @brief Queries the current fusion level.
     * @return Returns the fusion level (0~7); a negative value means unavailable.
     */
    virtual int32_t GetCurrentLevel() const = 0;
};

/**
 * @brief Creates the default provider backed by ressched_client (ResSchedClient
 *        systemload notifier / query). Defined in system_load_provider.cpp so that the
 *        judge and its unit tests do not depend on resource-schedule headers.
 * @return Returns the default provider instance.
 */
std::shared_ptr<ISystemLoadProvider> CreateDefaultSystemLoadProvider();

}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_APP_EXECFWK_TASK_MGR_SYSTEM_LOAD_PROVIDER_H
