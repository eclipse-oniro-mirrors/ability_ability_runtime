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

#include "ability_post_event_timeout.h"

#include "ability_handler.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
const int64_t AbilityPostEventTimeout::defalutDelayTime = 5000;

std::atomic<uint32_t> AbilityPostEventTimeout::allocationId_ = 0;

AbilityPostEventTimeout::AbilityPostEventTimeout(std::string str, std::shared_ptr<AbilityHandler> &eventHandler)
{
    uint32_t taskId = allocationId_++;
    std::string strId = std::to_string(taskId);
    task_ = str + strId;
    taskExec_ = false;
    handler_ = eventHandler;
}
AbilityPostEventTimeout::~AbilityPostEventTimeout()
{
    handler_.reset();
}

void AbilityPostEventTimeout::TimingBegin(int64_t delaytime)
{
    TAG_LOGI(AAFwkTag::ABILITY, "call %{public}s", task_.c_str());
    if (handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null %{public}s handler_",
            task_.c_str());
        return;
    }

    auto task = [weak = weak_from_this()]() {
        auto timeoutTask = weak.lock();
        if (timeoutTask == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null timeout");
            return;
        }
        timeoutTask->TimeOutProc();
    };
    handler_->PostTask(task, task_, delaytime);
}
void AbilityPostEventTimeout::TimeEnd()
{
    if (handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null %{public}s handler_", task_.c_str());
        return;
    }

    std::lock_guard<std::mutex> lck(mtx_);
    if (!taskExec_) {
        taskExec_ = true;
        handler_->RemoveTask(task_);
    }
}

void AbilityPostEventTimeout::TimeOutProc()
{
    TAG_LOGI(AAFwkTag::ABILITY, "call %{public}s", task_.c_str());
    if (handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null %{public}s handler_", task_.c_str());
        return;
    }

    std::lock_guard<std::mutex> lck(mtx_);
    if (!taskExec_) {
        taskExec_ = true;
        TAG_LOGW(AAFwkTag::ABILITY, "%{public}s TimeOut", task_.c_str());
        handler_->RemoveTask(task_);
    } else {
        TAG_LOGW(AAFwkTag::ABILITY, "Failed,Event:%{public}s",
            task_.c_str());
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
