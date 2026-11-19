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

#ifndef OHOS_ABILITY_RUNTIME_CLI_TOOL_APP_STATE_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_CLI_TOOL_APP_STATE_OBSERVER_H

#include <functional>
#include <string>

#include "application_state_observer_stub.h"

namespace OHOS {
namespace CliTool {

class CliToolAppStateObserver : public AppExecFwk::ApplicationStateObserverStub {
public:
    using ProcessDiedCallback = std::function<void(const std::string&, pid_t)>;

    explicit CliToolAppStateObserver(const std::string &bundleName, ProcessDiedCallback callback);
    ~CliToolAppStateObserver() override = default;

    void OnProcessDied(const AppExecFwk::ProcessData &processData) override;

private:
    std::string bundleName_;
    ProcessDiedCallback processDiedCallback_;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_TOOL_APP_STATE_OBSERVER_H
