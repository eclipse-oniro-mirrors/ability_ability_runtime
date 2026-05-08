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

#ifndef MOCK_ABILITY_HANDLER_H
#define MOCK_ABILITY_HANDLER_H

#include <memory>

namespace OHOS {
namespace AppExecFwk {
class EventHandler : public std::enable_shared_from_this<EventHandler> {
public:
    virtual ~EventHandler() = default;
};
} // namespace AppExecFwk

namespace AbilityRuntime {
class AbilityHandler : public AppExecFwk::EventHandler {
public:
    ~AbilityHandler() override = default;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // MOCK_ABILITY_HANDLER_H