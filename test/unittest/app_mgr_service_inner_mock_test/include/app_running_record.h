/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_RUNNING_RECORD_H
#define OHOS_ABILITY_RUNTIME_APP_RUNNING_RECORD_H

#include <memory>

namespace OHOS {
namespace AppExecFwk {
/**
 * @class AppRunningRecord
 * Record app process info.
 */
class AppRunningRecord : public std::enable_shared_from_this<AppRunningRecord> {
public:
    AppRunningRecord() = default;
    virtual ~AppRunningRecord() = default;

    inline int GetPid() const
    {
        return pid_;
    }

    inline int32_t GetUserId() const
    {
        return userId_;
    }

    int pid_ = -1;
    int32_t userId_ = 0;
};

}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_RUNNING_RECORD_H
