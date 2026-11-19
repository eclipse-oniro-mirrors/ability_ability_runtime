/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "ability_event_handler.h"

#include "ability_manager_service.h"
#include "ability_util.h"

namespace OHOS {
namespace AAFwk {
AbilityEventHandler::AbilityEventHandler(
    const std::shared_ptr<TaskHandlerWrap> &taskHandler, const std::weak_ptr<AbilityManagerService> &server)
    : EventHandlerWrap(taskHandler), server_(server)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "constructors");
}

void AbilityEventHandler::ProcessEvent(const EventWrap &event)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Event obtained: %{public}s.", event.GetEventString().c_str());
    // check libc.hook_mode
    const int bufferLen = 128;
    char paramOutBuf[bufferLen] = {0};
    const char *hook_mode = "startup:";
    int ret = GetParameter("libc.hook_mode", "", paramOutBuf, bufferLen);
    if (ret > 0 && strncmp(paramOutBuf, hook_mode, strlen(hook_mode)) == 0) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Hook_mode: no process time out");
        return;
    }
    switch (event.GetEventId()) {
        case AbilityManagerService::LOAD_HALF_TIMEOUT_MSG:
            ProcessLoadTimeOut(event, true);
            break;
        case AbilityManagerService::LOAD_TIMEOUT_MSG:
            ProcessLoadTimeOut(event, false);
            break;
        case AbilityManagerService::ACTIVE_TIMEOUT_MSG:
            ProcessActiveTimeOut(event.GetParam());
            break;
        case AbilityManagerService::INACTIVE_TIMEOUT_MSG:
            TAG_LOGD(AAFwkTag::ABILITYMGR, "Inactive timeout.");
            // inactivate pre ability immediately in case blocking next ability start
            ProcessInactiveTimeOut(event.GetParam());
            break;
        case AbilityManagerService::FOREGROUND_HALF_TIMEOUT_MSG:
            ProcessForegroundTimeOut(event, true);
            break;
        case AbilityManagerService::FOREGROUND_TIMEOUT_MSG:
            ProcessForegroundTimeOut(event, false);
            break;
        case AbilityManagerService::SHAREDATA_TIMEOUT_MSG:
            ProcessShareDataTimeOut(event.GetParam());
            break;
        case AbilityManagerService::CONNECT_TIMEOUT_MSG:
            ProcessConnectTimeOut(event, false);
            break;
        case AbilityManagerService::CONNECT_HALF_TIMEOUT_MSG:
            ProcessConnectTimeOut(event, true);
            break;
        case AbilityManagerService::SKILL_EXECUTE_TIMEOUT_MSG:
            ProcessSkillExecuteTimeOut(event.GetParam());
            break;
        default:
            TAG_LOGW(AAFwkTag::ABILITYMGR, "unsupported timeout message");
            break;
    }
}

void AbilityEventHandler::ProcessLoadTimeOut(const EventWrap &event, bool isHalf)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto server = server_.lock();
    CHECK_POINTER(server);
    server->HandleLoadTimeOut(event.GetParam(), isHalf, event.IsExtension());
}

void AbilityEventHandler::ProcessActiveTimeOut(int64_t abilityRecordId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto server = server_.lock();
    CHECK_POINTER(server);
    server->HandleActiveTimeOut(abilityRecordId);
}

void AbilityEventHandler::ProcessInactiveTimeOut(int64_t abilityRecordId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    auto server = server_.lock();
    CHECK_POINTER(server);
    server->HandleInactiveTimeOut(abilityRecordId);
}

void AbilityEventHandler::ProcessForegroundTimeOut(const EventWrap &event, bool isHalf)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "foreground timeout");
    auto server = server_.lock();
    CHECK_POINTER(server);
    server->HandleForegroundTimeOut(event.GetParam(), isHalf, event.IsExtension());
}

void AbilityEventHandler::ProcessShareDataTimeOut(int64_t uniqueId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "shareData timeout");
    auto server = server_.lock();
    CHECK_POINTER(server);
    server->HandleShareDataTimeOut(uniqueId);
}

void AbilityEventHandler::ProcessConnectTimeOut(const EventWrap &event, bool isHalf)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "connect timeout");
    auto server = server_.lock();
    CHECK_POINTER(server);
    server->HandleConnectTimeOut(event.GetParam(), isHalf);
}

void AbilityEventHandler::ProcessSkillExecuteTimeOut(int64_t requestCodeSeq)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "skill execute timeout");
    auto server = server_.lock();
    CHECK_POINTER(server);
    server->HandleSkillExecuteTimeOut(requestCodeSeq);
}

}  // namespace AAFwk
}  // namespace OHOS
