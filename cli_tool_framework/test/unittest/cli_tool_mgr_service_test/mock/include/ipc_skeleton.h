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

#ifndef OHOS_ABILITY_RUNTIME_CLI_TOOL_MGR_SERVICE_TEST_IPC_SKELETON_H
#define OHOS_ABILITY_RUNTIME_CLI_TOOL_MGR_SERVICE_TEST_IPC_SKELETON_H

#include <cstdint>
#include <sys/types.h>

namespace OHOS {
// Constants for testing
constexpr uint64_t TOKEN_NATIVE = 1;
constexpr uint64_t TOKEN_HAP = 2;
constexpr int32_t FOUNDATION_UID = 5523;

class IPCSkeleton {
public:
    static pid_t GetCallingUid();
    static pid_t GetCallingPid();
    static uint64_t GetCallingFullTokenID();
    static uint64_t GetCallingTokenID();
    static void Reset();
    static void SetCallingTokenID(uint64_t tokenID);
    static void SetCallingUid(pid_t uid);

    static pid_t callingUid;
    static pid_t callingPid;
    static uint64_t callingFullTokenId;
    static uint64_t callingTokenId;
};
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_TOOL_MGR_SERVICE_TEST_IPC_SKELETON_H
