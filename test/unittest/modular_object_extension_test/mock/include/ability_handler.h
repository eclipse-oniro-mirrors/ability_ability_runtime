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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_HANDLER_H
#define OHOS_ABILITY_RUNTIME_ABILITY_HANDLER_H

#include <memory>
#include <string>

namespace OHOS {
namespace AppExecFwk {

class EventRunner {
public:
    static std::shared_ptr<EventRunner> Create(const std::string &name)
    {
        return std::make_shared<EventRunner>();
    }
};

class EventHandler {
public:
    EventHandler() = default;
    explicit EventHandler(const std::shared_ptr<EventRunner> &) {}
};

class AbilityHandler : public EventHandler {
public:
    AbilityHandler() = default;
    explicit AbilityHandler(const std::shared_ptr<EventRunner> &runner) : EventHandler(runner) {}
    std::shared_ptr<EventRunner> GetEventRunner() const { return nullptr; }
};

class InnerEvent {
public:
    class Pointer {};
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_ABILITY_HANDLER_H
