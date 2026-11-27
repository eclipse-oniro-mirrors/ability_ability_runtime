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

#ifndef OHOS_ABILITY_RUNTIME_MOCK_PROCESS_LIFECYCLE_OBSERVER_STUB_H
#define OHOS_ABILITY_RUNTIME_MOCK_PROCESS_LIFECYCLE_OBSERVER_STUB_H

#include <gmock/gmock.h>

#include "process_lifecycle_observer_stub.h"

namespace OHOS {
namespace AppExecFwk {
class MockProcessLifecycleObserverStub : public ProcessLifecycleObserverStub {
public:
    MockProcessLifecycleObserverStub() = default;
    virtual ~MockProcessLifecycleObserverStub() = default;

    MOCK_METHOD1(OnProcessCreated, void(const ProcessData &processData));
    MOCK_METHOD1(OnProcessDied, void(const ProcessData &processData));
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_MOCK_PROCESS_LIFECYCLE_OBSERVER_STUB_H
