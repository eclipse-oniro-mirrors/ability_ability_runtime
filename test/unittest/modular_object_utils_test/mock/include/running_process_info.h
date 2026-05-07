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

#ifndef MOCK_RUNNING_PROCESS_INFO_H
#define MOCK_RUNNING_PROCESS_INFO_H

#include <cstdint>

namespace OHOS {
namespace AppExecFwk {

enum class AppProcessState {
    APP_STATE_BEGIN = 0,
    APP_STATE_READY = 1,
    APP_STATE_FOREGROUND = 2,
    APP_STATE_BACKGROUND = 4,
    APP_STATE_END
};

struct RunningProcessInfo {
    AppProcessState state_ = AppProcessState::APP_STATE_FOREGROUND;
    bool isPreForeground = false;
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // MOCK_RUNNING_PROCESS_INFO_H
