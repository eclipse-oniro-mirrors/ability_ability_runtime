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

#ifndef OHOS_ABILITY_RUNTIME_WANT_AGENT_APP_STATE_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_WANT_AGENT_APP_STATE_OBSERVER_H

#include <functional>
#include <string>
#include <sys/types.h>

#include "application_state_observer_stub.h"

namespace OHOS {
namespace AAFwk {
class WantAgentAppStateObserver final : public AppExecFwk::ApplicationStateObserverStub {
public:
    using ProcessDiedHandler = std::function<void(const std::string &, pid_t)>;
    explicit WantAgentAppStateObserver(ProcessDiedHandler handler);
    ~WantAgentAppStateObserver() = default;
    void OnProcessDied(const AppExecFwk::ProcessData &processData) override;

private:
    ProcessDiedHandler handler_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_WANT_AGENT_APP_STATE_OBSERVER_H
