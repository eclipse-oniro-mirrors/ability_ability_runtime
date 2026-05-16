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

#include "cli_tool_app_state_observer.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

CliToolAppStateObserver::CliToolAppStateObserver(const std::string &bundleName, ProcessDiedCallback callback)
    : bundleName_(bundleName), processDiedCallback_(callback)
{}

void CliToolAppStateObserver::OnProcessDied(const AppExecFwk::ProcessData &processData)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Process died: bundleName=%{public}s, pid=%{public}d",
        bundleName_.c_str(), processData.pid);

    if (processDiedCallback_) {
        processDiedCallback_(bundleName_, processData.pid);
    }
}

} // namespace CliTool
} // namespace OHOS
