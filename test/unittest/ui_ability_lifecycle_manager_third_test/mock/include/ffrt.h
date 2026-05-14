/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef FFRT_API_FFRT_H
#define FFRT_API_FFRT_H

#include <cinttypes>
#include <functional>
#include <string>
#include <thread>

#include "cpp/mutex.h"
#include "cpp/condition_variable.h"
#include "mock_my_flag.h"

namespace ffrt {
struct task_attr {
    inline task_attr &qos(int32_t) { return *this;}
    inline task_attr &name(const std::string &) { return *this; }
    inline task_attr &delay(int64_t) { return *this; }
    inline task_attr &timeout(int64_t) { return *this; }
};
struct task_handle {};

inline void submit(std::function<void()> &&task, task_attr attr = {})
{
    OHOS::AAFwk::MyFlag::ffrtSubmitFlag_++;
    if (task) {
        std::thread taskThread(task);
        taskThread.detach();
    }
}

inline task_handle submit_h(std::function<void()> &&task)
{
    return task_handle{};
}

inline void wait(std::vector<task_handle> taskHandles) {}
}  // namespace ffrt
#endif // FFRT_API_FFRT_H