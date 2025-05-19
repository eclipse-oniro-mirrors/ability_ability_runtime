/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "startup_config.h"

namespace OHOS {
namespace AbilityRuntime {
StartupConfig::StartupConfig() = default;

StartupConfig::~StartupConfig() = default;

StartupConfig::StartupConfig(int32_t awaitTimeoutMs) : awaitTimeoutMs_(awaitTimeoutMs)
{}

StartupConfig::StartupConfig(const std::shared_ptr<StartupListener> &listener) : listener_(listener)
{}

StartupConfig::StartupConfig(int32_t awaitTimeoutMs, const std::shared_ptr<StartupListener> &listener)
    : awaitTimeoutMs_(awaitTimeoutMs), listener_(listener)
{}

int32_t StartupConfig::GetAwaitTimeoutMs() const
{
    return awaitTimeoutMs_;
}

void StartupConfig::ListenerOnCompleted(const std::shared_ptr<StartupTaskResult> &result)
{
    if (listener_ != nullptr) {
        listener_->OnCompleted(result);
    }
}

const std::string &StartupConfig::GetCustomization() const
{
    return customization_;
}
} // namespace AbilityRuntime
} // namespace OHOS
