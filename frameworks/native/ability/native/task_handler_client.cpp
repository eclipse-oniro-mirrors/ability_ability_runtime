/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "task_handler_client.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
std::shared_ptr<TaskHandlerClient> TaskHandlerClient::GetInstance()
{
    static auto instance = std::make_shared<TaskHandlerClient>();
    return instance;
}

TaskHandlerClient::TaskHandlerClient()
{}

TaskHandlerClient::~TaskHandlerClient()
{}

bool TaskHandlerClient::InitHandlerLocked()
{
    if (taskHandler_ != nullptr) {
        return true;
    }
    std::shared_ptr<EventRunner> runner = EventRunner::Create("TaskRunner");
    if (runner == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null runner");
        return false;
    }
    taskHandler_ = std::make_shared<TaskHandler>(runner);
    if (taskHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null taskHandler_");
        return false;
    }
    return true;
}

bool TaskHandlerClient::PostTask(std::function<void()> task, long delayTime)
{
    TAG_LOGI(AAFwkTag::ABILITY, "called");
    std::shared_ptr<TaskHandler> handler;
    {
        std::lock_guard<std::mutex> lock(taskHandlerMutex_);
        if (!InitHandlerLocked()) {
            return false;
        }
        handler = taskHandler_;
    }
    bool ret = handler->PostTask(task, delayTime, EventQueue::Priority::LOW);
    if (!ret) {
        TAG_LOGE(AAFwkTag::ABILITY, "taskHandler_ PostTask failed");
    }
    return ret;
}

bool TaskHandlerClient::CreateRunner()
{
    std::lock_guard<std::mutex> lock(taskHandlerMutex_);
    return InitHandlerLocked();
}
}  // namespace AppExecFwk
}  // namespace OHOS
