/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CJ_ENVIRONMENT_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_CJ_ENVIRONMENT_CALLBACK_H

#include <map>
#include <memory>

#include "cj_utils_ffi.h"
#include "configuration.h"
#include "environment_callback.h"

namespace OHOS {
namespace AbilityRuntime {

class CjEnvironmentCallback : public EnvironmentCallback,
    public std::enable_shared_from_this<CjEnvironmentCallback> {
public:
    explicit CjEnvironmentCallback();
    void OnConfigurationUpdated(const AppExecFwk::Configuration &config) override;
    void OnMemoryLevel(const int level) override;
    int32_t Register(std::function<void(CConfiguration)> cfgCallback,
        std::function<void(int32_t)> memCallback, bool isSync);
    int32_t RegisterV2(std::function<void(CConfigurationV2)> cfgCallback,
        std::function<void(int32_t)> memCallback, bool isSync);
    bool UnRegister(int32_t callbackId, bool isSync = false);
    bool IsEmpty() const;
    static int32_t serialNumber_;

private:
    std::map<int32_t, std::function<void(CConfiguration)>> onConfigurationUpdatedCallbacks_;
    std::map<int32_t, std::function<void(CConfigurationV2)>> onConfigurationUpdatedCallbacksV2_;
    std::map<int32_t, std::function<void(int32_t)>> onMemoryLevelCallbacks_;
    void CallConfigurationUpdatedInner(const AppExecFwk::Configuration &config,
        const std::map<int32_t, std::function<void(CConfiguration)>> &callbacks);
    void CallConfigurationUpdatedInnerV2(const AppExecFwk::Configuration &config,
        const std::map<int32_t, std::function<void(CConfigurationV2)>> &callbacks);
    void CallMemoryLevelInner(const int level,
        const std::map<int32_t, std::function<void(int32_t)>> &callbacks);
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_CJ_ENVIRONMENT_CALLBACK_H
