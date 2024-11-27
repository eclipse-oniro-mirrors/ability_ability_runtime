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

#ifndef OHOS_ABILITY_RUNTIME_PRELOAD_UIEXT_STATE_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_PRELOAD_UIEXT_STATE_OBSERVER_H

#include <memory>

#include "application_state_observer_stub.h"

namespace OHOS {
namespace AbilityRuntime {
class ExtensionRecord;
}
namespace AAFwk {
class PreLoadUIExtStateObserver final : public AppExecFwk::ApplicationStateObserverStub {
public:
    PreLoadUIExtStateObserver(std::weak_ptr<AbilityRuntime::ExtensionRecord> extensionRecord);
    void OnProcessDied(const AppExecFwk::ProcessData &processData) override;
    void OnAppCacheStateChanged(const AppExecFwk::AppStateData &appStateData) override;

private:
    std::weak_ptr<AbilityRuntime::ExtensionRecord> extensionRecord_ = std::weak_ptr<AbilityRuntime::ExtensionRecord>();
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_PRELOAD_UIEXT_STATE_OBSERVER_H