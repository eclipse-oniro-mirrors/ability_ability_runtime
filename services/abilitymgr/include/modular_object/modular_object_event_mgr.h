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

#ifndef OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_EVENT_MGR_H
#define OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_EVENT_MGR_H

#include <memory>

#include "modular_object_event_receiver.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * @class ModularObjectExtensionEventMgr
 * @brief Manages the lifecycle of the system event receiver for modular object extensions.
 */
class ModularObjectExtensionEventMgr : public std::enable_shared_from_this<ModularObjectExtensionEventMgr> {
public:
    ModularObjectExtensionEventMgr() = default;
    ~ModularObjectExtensionEventMgr() = default;

    /**
     * @brief Subscribes to required common events.
     */
    void SubscribeSysEventReceiver();

private:
    std::shared_ptr<ModularObjectEventReceiver> sysEventReceiver_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_EVENT_MGR_H