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

#ifndef OHOS_ABILITY_RUNTIME_ETS_CHILD_PROCESS_H
#define OHOS_ABILITY_RUNTIME_ETS_CHILD_PROCESS_H

#include "child_process.h"
#include "ets_runtime.h"
#include "ets_native_reference.h"

namespace OHOS {
namespace AbilityRuntime {
class EtsChildProcess : public ChildProcess {
public:
    explicit EtsChildProcess(ETSRuntime &etsRuntime);
    ~EtsChildProcess() override;

    static EtsChildProcess* Create(const std::unique_ptr<Runtime> &runtime);

    bool Init(const std::shared_ptr<ChildProcessStartInfo> &info) override;
    void OnStart() override;
    void OnStart(std::shared_ptr<AppExecFwk::ChildProcessArgs> args) override;

private:
    ani_ref CallObjectMethod(bool withResult, const char *name, const char *signature, ...);

    ETSRuntime &etsRuntime_;
    std::unique_ptr<AppExecFwk::ETSNativeReference> etsChildProcessObj_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ETS_CHILD_PROCESS_H