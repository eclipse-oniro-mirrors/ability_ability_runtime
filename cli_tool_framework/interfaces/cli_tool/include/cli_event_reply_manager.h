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

#ifndef OHOS_ABILITY_RUNTIME_CLI_EVENT_REPLY_MANAGER_H
#define OHOS_ABILITY_RUNTIME_CLI_EVENT_REPLY_MANAGER_H

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

#include "cli_session_info.h"

namespace OHOS {
namespace CliTool {

struct CliEventReplyResult {
    int32_t code = 0;
    std::optional<CliSessionInfo> sessionInfo = std::nullopt;
};

class CliEventReplyManager final {
public:
    using EventReplyCallback = std::function<void(const CliEventReplyResult &)>;

    static CliEventReplyManager &GetInstance();

    std::string AddEventReplyCallback(const std::string &name,
                                      const EventReplyCallback &callback);

    void RemoveEventReplyCallback(const std::string &eventId);

    void ActivateEventReplyCallback(const std::string &eventId);

    void ClearAllEvent();

    int32_t HandleEventReply(const std::string &eventId, const CliEventReplyResult &result);

private:
    struct EventReplyCallbackRecord {
        std::string eventId;
        bool isActive = false;
        EventReplyCallback callback;
        std::optional<CliEventReplyResult> result = std::nullopt;
    };

    CliEventReplyManager() = default;
    ~CliEventReplyManager() = default;

    std::string GenerateReplyEventId(const std::string &prefix);

private:
    std::atomic<uint64_t> replyEventCount_ {0};

    std::mutex mutex_;
    std::unordered_map<std::string, EventReplyCallbackRecord> eventReplyTasks_;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_EVENT_REPLY_MANAGER_H