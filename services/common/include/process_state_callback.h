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

#ifndef OHOS_ABILITY_RUNTIME_PROCESS_STATE_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_PROCESS_STATE_CALLBACK_H

#include "cpp/condition_variable.h"
#include "hilog_tag_wrapper.h"
#include "memory_observer_stub.h"
#include <chrono>

namespace OHOS {
namespace AppExecFwk {
class ProcessStateCallback : public Memory::MemoryObserverStub {
public:
    ProcessStateCallback() = default;
    ~ProcessStateCallback() override = default;

    void OnProcessAvailable(bool isAvailable) override
    {
        TAG_LOGD(AAFwkTag::APPMGR, "OnProcessAvailable, isAvailable:%{public}d", isAvailable);
        {
            std::lock_guard<ffrt::mutex> lock(mtx_);
            isAvailable_ = isAvailable;
            isDone_ = true;
        }
        cv_.notify_all();
    }

    bool WaitForResult(int32_t timeoutMs)
    {
        std::unique_lock<ffrt::mutex> lock(mtx_);
        return cv_.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this] { return isDone_; });
    }

    void Reset()
    {
        std::lock_guard<ffrt::mutex> lock(mtx_);
        isAvailable_ = true;
        isDone_ = false;
    }

    bool IsAvailable() const
    {
        return isAvailable_;
    }

private:
    mutable ffrt::mutex mtx_;
    ffrt::condition_variable cv_;
    bool isAvailable_ = true;
    bool isDone_ = false;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_PROCESS_STATE_CALLBACK_H
