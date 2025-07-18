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

#include "ext_native_startup_task.h"

namespace OHOS {
namespace AbilityRuntime {
ExtNativeStartupTask::ExtNativeStartupTask(const std::string &name) : name_(name)
{}

ExtNativeStartupTask::~ExtNativeStartupTask() = default;

const std::string& ExtNativeStartupTask::GetName() const
{
    return name_;
}
} // namespace AbilityRuntime
} // namespace OHOS