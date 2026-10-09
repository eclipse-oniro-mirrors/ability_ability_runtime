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

#ifndef OHOS_APP_EXECFWK_TASK_MGR_MANAGED_PROCESS_H
#define OHOS_APP_EXECFWK_TASK_MGR_MANAGED_PROCESS_H

#include <cstdint>
#include <sys/types.h>

#include "task_mgr/common.h"

namespace OHOS {
namespace AppExecFwk {
// A process attributed into a task by a tool chain. The tool knows its own type when it
// calls RegisterManagedProcess, so the type is recorded per process.
struct ManagedProcess {
    pid_t pid = -1;
    AttributionTool tool = AttributionTool::UNKNOWN;
    int64_t attributedTimeMillis = 0;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_APP_EXECFWK_TASK_MGR_MANAGED_PROCESS_H
