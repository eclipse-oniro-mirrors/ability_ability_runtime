/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_EVENT_HANDLER_H
#define OHOS_ABILITY_RUNTIME_ABILITY_EVENT_HANDLER_H

#include <memory>

#include "event_handler_wrap.h"

namespace OHOS {
namespace AAFwk {
class AbilityManagerService;
/**
 * @class AbilityEventHandler
 * AbilityEventHandler handling the ability event.
 */
class AbilityEventHandler : public EventHandlerWrap {
public:
    AbilityEventHandler(
        const std::shared_ptr<TaskHandlerWrap> &taskHandler, const std::weak_ptr<AbilityManagerService> &server);
    virtual ~AbilityEventHandler() = default;

    /**
     * ProcessEvent with request.
     *
     * @param event, inner event loop.
     */
    void ProcessEvent(const EventWrap &event) override;

private:
    void ProcessLoadTimeOut(const EventWrap &event, bool isHalf = false);
    void ProcessActiveTimeOut(int64_t abilityRecordId);
    void ProcessInactiveTimeOut(int64_t abilityRecordId);
    void ProcessForegroundTimeOut(const EventWrap &event, bool isHalf = false);
    void ProcessShareDataTimeOut(int64_t uniqueId);
    void ProcessConnectTimeOut(const EventWrap &event, bool isHalf = false);
    void ProcessSkillExecuteTimeOut(int64_t requestCodeSeq);
private:
    std::weak_ptr<AbilityManagerService> server_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_EVENT_HANDLER_H
