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

#include "cli_event_reply_manager.h"

#include "cli_tool_mgr_client.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

namespace {
constexpr int32_t ERROR_CODE = -1;
constexpr int32_t ERR_OK = 0;
} // namespace

CliEventReplyManager &CliEventReplyManager::GetInstance()
{
    static CliEventReplyManager instance;
    return instance;
}

std::string CliEventReplyManager::GenerateReplyEventId(const std::string &prefix)
{
    uint64_t suffix = replyEventCount_.fetch_add(1, std::memory_order_relaxed) + 1;
    return prefix + std::to_string(suffix);
}

void CliEventReplyManager::ClearAllEvent()
{
    std::lock_guard<std::mutex> lock(mutex_);
    eventReplyTasks_.clear();
}

void CliEventReplyManager::RemoveEventReplyCallback(const std::string &eventId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    eventReplyTasks_.erase(eventId);
}

std::string CliEventReplyManager::AddEventReplyCallback(const std::string &name, const EventReplyCallback &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::string eventId = GenerateReplyEventId(name);
    eventReplyTasks_[eventId] = EventReplyCallbackRecord {eventId, false, std::move(callback), std::nullopt};
    return eventId;
}

void CliEventReplyManager::ActivateEventReplyCallback(const std::string &eventId)
{
    CliEventReplyResult result;
    EventReplyCallback replyCallback;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = eventReplyTasks_.find(eventId);
        if (it == eventReplyTasks_.end()) {
            return;
        }
        it->second.isActive = true;
        if (!it->second.result.has_value()) {
            return;
        }
        result = it->second.result.value();
        replyCallback = std::move(it->second.callback);
        eventReplyTasks_.erase(it);
    }
    if (replyCallback == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "replyCallback is null");
        return;
    }
    replyCallback(result);
}

int32_t CliEventReplyManager::HandleEventReply(const std::string &eventId, const CliEventReplyResult &result)
{
    EventReplyCallback replyCallback;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = eventReplyTasks_.find(eventId);
        if (it == eventReplyTasks_.end()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL,
                "HandleEventReply replyCallback not found, eventId:%{public}s", eventId.c_str());
            return ERROR_CODE;
        }
        if (!it->second.isActive) {
            it->second.result = result;
            return ERR_OK;
        }
        replyCallback = std::move(it->second.callback);
        eventReplyTasks_.erase(it);
    }
    if (replyCallback == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "HandleEventReply replyCallback is null, eventId:%{public}s", eventId.c_str());
        return ERROR_CODE;
    }
    replyCallback(result);
    return ERR_OK;
}

} // namespace CliTool
} // namespace OHOS
