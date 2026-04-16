/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "child_process.h"

#include "js_child_process.h"
#include "ets_child_process_instance.h"

namespace OHOS {
namespace AbilityRuntime {
std::shared_ptr<ChildProcess> ChildProcess::Create(const std::unique_ptr<Runtime> &runtime)
{
    if (!runtime) {
        return std::make_shared<ChildProcess>();
    }
    switch (runtime->GetLanguage()) {
        case AbilityRuntime::Runtime::Language::JS:
            return AbilityRuntime::JsChildProcess::Create(runtime);
        case AbilityRuntime::Runtime::Language::ETS: {
            auto* childProcess = AbilityRuntime::CreateETSChildProcess(runtime);
            if (childProcess == nullptr) {
                return nullptr;
            }
            return std::shared_ptr<ChildProcess>(childProcess);
        }
        default:
            return std::make_shared<ChildProcess>();
    }
}

bool ChildProcess::Init(const std::shared_ptr<ChildProcessStartInfo> &info)
{
    processStartInfo_ = info;
    return true;
}

void ChildProcess::OnStart() {}

void ChildProcess::OnStart(std::shared_ptr<AppExecFwk::ChildProcessArgs> args) {}

}  // namespace AbilityRuntime
}  // namespace OHOS