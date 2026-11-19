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

#ifndef MOCK_IPC_SKELETON_H
#define MOCK_IPC_SKELETON_H

#include <string>
#include <sys/types.h>
#include "mock_flag.h"

namespace OHOS {
class IPCSkeleton {
public:
    static int32_t GetCallingUid()
    {
        return MockFlag::callingUid;
    }
    static pid_t GetCallingPid()
    {
        return MockFlag::callingPid;
    }
    static std::string ResetCallingIdentity()
    {
        return "";
    }
    static void SetCallingIdentity(const std::string &identity)
    {
        (void)identity;
    }
};
} // namespace OHOS

#endif // MOCK_IPC_SKELETON_H
