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

#ifndef OHOS_ABILITY_RUNTIME_JS_CLI_EVENT_REPLY_MANAGER_H
#define OHOS_ABILITY_RUNTIME_JS_CLI_EVENT_REPLY_MANAGER_H

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "event_handler.h"

namespace OHOS {
namespace CliTool {

class JsCliEventHandlerManager final {
public:
    using EventReplyTask = std::function<void()>;

    static JsCliEventHandlerManager &GetInstance();

    int32_t PostTask(const EventReplyTask &task);

private:
    JsCliEventHandlerManager() = default;
    ~JsCliEventHandlerManager() = default;

    void EnsureInitialized();

    std::mutex mutex_;
    bool initialized_ = false;

    std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_CLI_EVENT_REPLY_MANAGER_H
