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

#ifndef OHOS_ABILITY_RUNTIME_ETS_APP_PROCESS_STATE_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_ETS_APP_PROCESS_STATE_OBSERVER_H

#include <memory>
#include <mutex>
#include <set>

#include "ani.h"
#include "ani_task.h"
#include "process_lifecycle_observer_stub.h"

namespace OHOS {
namespace AbilityRuntime {
class EtsAppProcessStateObserver : public OHOS::AppExecFwk::ProcessLifecycleObserverStub {
public:
    explicit EtsAppProcessStateObserver(ani_vm *etsVm);
    ~EtsAppProcessStateObserver() override;
    void OnProcessCreated(const OHOS::AppExecFwk::ProcessData &processData) override;
    void OnProcessDied(const OHOS::AppExecFwk::ProcessData &processData) override;
    void HandleOnProcessCreated(const OHOS::AppExecFwk::ProcessData &processData);
    void HandleOnProcessDied(const OHOS::AppExecFwk::ProcessData &processData);
    void Register(ani_object aniCallback);
    bool UnRegister(ani_object aniCallback = nullptr);
    bool IsEmpty() const;

private:
    void CallEtsFunction(ani_env *env, ani_object etsObserverObject, const char *methodName,
        const char *signature, ani_object processDataObj);
    ani_vm *etsVm_ = nullptr;
    std::set<ani_ref> observers_;
    mutable std::mutex mutex_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_APP_PROCESS_STATE_OBSERVER_H
