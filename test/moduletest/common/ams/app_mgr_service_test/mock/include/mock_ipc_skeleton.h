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

#ifndef OHOS_ABILITY_RUNTIME_MOCK_IPC_SKELETON_H
#define OHOS_ABILITY_RUNTIME_MOCK_IPC_SKELETON_H

#include <sys/types.h>

namespace OHOS {
class IPCSkeleton {
public:
    static pid_t GetCallingPid();

    static pid_t GetCallingUid();

    static uint32_t GetCallingTokenID();

    static uint64_t GetCallingFullTokenID();

    // Test-only helpers (absent from the real header); the mock source defines them.
    static void SetCallingUid(pid_t uid);

    static uint32_t SetCallingTokenID(pid_t tokenId);
};
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_MOCK_IPC_SKELETON_H
