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

#include "agent_event_handler.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AgentRuntime {
AgentEventHandler::AgentEventHandler(const std::shared_ptr<AAFwk::TaskHandlerWrap> &taskHandler)
    : AAFwk::EventHandlerWrap(taskHandler)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "init AgentEventHandler");
}

void AgentEventHandler::ProcessEvent(const AAFwk::EventWrap &event)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "Event obtained: %{public}s.", event.GetEventString().c_str());
}
}  // namespace AgentRuntime
}  // namespace OHOS
