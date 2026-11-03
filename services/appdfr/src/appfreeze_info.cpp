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
#include "appfreeze_info.h"
#include "fault_data.h"

namespace OHOS {
namespace AppExecFwk {
bool AppFreezeInfo::IsHalfEvent(const std::string& name)
{
    return name == AppFreezeType::THREAD_BLOCK_3S || name == AppFreezeType::LIFECYCLE_HALF_TIMEOUT ||
        name == AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING ||
        name == AppFreezeType::BUSSINESS_THREAD_BLOCK_3S;
}

std::string AppFreezeInfo::EventGroup(const std::string& name)
{
    if (name == AppFreezeType::THREAD_BLOCK_3S || name == AppFreezeType::THREAD_BLOCK_6S ||
        name == AppFreezeType::APP_INPUT_BLOCK) {
        return "main";
    }
    if (name == AppFreezeType::LIFECYCLE_HALF_TIMEOUT || name == AppFreezeType::LIFECYCLE_TIMEOUT ||
        name == AppFreezeType::LIFECYCLE_HALF_TIMEOUT_WARNING ||
        name == AppFreezeType::LIFECYCLE_TIMEOUT_WARNING) {
        return "lifecycle";
    }
    return "other";
}

} // namespace AppExecFwk
} // namespace OHOS
