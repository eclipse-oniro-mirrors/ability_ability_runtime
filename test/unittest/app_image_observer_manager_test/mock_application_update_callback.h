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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_APPLICATION_UPDATE_CALLBACK_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_APPLICATION_UPDATE_CALLBACK_H

#include "application_update_callback.h"
#include "gmock/gmock.h"

namespace OHOS {
namespace AbilityRuntime {
class MockApplicationUpdateCallback : public ApplicationUpdateCallback {
public:
    MockApplicationUpdateCallback() = default;
    virtual ~MockApplicationUpdateCallback() = default;

    MOCK_METHOD0(NotifyApplicationUpdate, void());

    MOCK_METHOD0(NotifyApplicationPreAbilityCreate, void());
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // MOCK_OHOS_ABILITY_RUNTIME_MOCK_APPLICATION_UPDATE_CALLBACK_H
