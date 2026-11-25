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

#ifndef OHOS_ABILITY_RUNTIME_MOCK_APPLICATION_STATE_OBSERVER_STUB_H
#define OHOS_ABILITY_RUNTIME_MOCK_APPLICATION_STATE_OBSERVER_STUB_H

#include <gmock/gmock.h>
// NOLINTNEXTLINE(g.nam.03-cpp): Redefining C++ keyword 'private' for unit testing
#define private public
// NOLINTNEXTLINE(g.nam.03-cpp): Redefining C++ keyword 'protected' for unit testing
#define protected public
#include "application_state_observer_stub.h"
#undef protected
#undef private

namespace OHOS {
namespace AppExecFwk {
class MockApplicationStateObserverStub : public ApplicationStateObserverStub {
public:
    MockApplicationStateObserverStub() = default;
    virtual ~MockApplicationStateObserverStub() = default;

    MOCK_METHOD1(OnGamePrelaunchForegroundStateChanged, void(const AppStateData &appStateData));
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_MOCK_APPLICATION_STATE_OBSERVER_STUB_H
