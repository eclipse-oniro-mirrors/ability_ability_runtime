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

#ifndef OHOS_AGENT_RUNTIME_AGENT_EVENT_HANDLER_H
#define OHOS_AGENT_RUNTIME_AGENT_EVENT_HANDLER_H

#include <memory>

#include "event_handler_wrap.h"

namespace OHOS {
namespace AgentRuntime {
/**
 * @class AgentEventHandler
 * AgentEventHandler handling the agent event.
 */
class AgentEventHandler : public AAFwk::EventHandlerWrap {
public:
    explicit AgentEventHandler(const std::shared_ptr<AAFwk::TaskHandlerWrap> &taskHandler);
    virtual ~AgentEventHandler() = default;

    /**
     * ProcessEvent with request.
     *
     * @param event, inner event loop.
     */
    void ProcessEvent(const AAFwk::EventWrap &event) override;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_AGENT_EVENT_HANDLER_H
