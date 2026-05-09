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

#include "js_cli_event_handler_manager.h"

#include "event_runner.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

namespace {
constexpr int32_t ERROR_CODE = -1;
constexpr int32_t ERR_OK = 0;
} // namespace

JsCliEventHandlerManager &JsCliEventHandlerManager::GetInstance()
{
    static JsCliEventHandlerManager instance;
    return instance;
}

int32_t JsCliEventHandlerManager::PostTask(const EventReplyTask &task)
{
    EnsureInitialized();

    if (!handler_) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "PostTask null handler_ object");
        return ERROR_CODE;
    }
    handler_->PostTask([task]() {
            if (task) {
                task();
            }
        }, "CliHandleEventReply");
    return ERR_OK;
}

void JsCliEventHandlerManager::EnsureInitialized()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        return;
    }
    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
    if (handler_) {
        initialized_ = true;
    }
}

} // namespace CliTool
} // namespace OHOS
