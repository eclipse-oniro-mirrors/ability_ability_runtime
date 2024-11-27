/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_IPC_IPC_SKELETON_H
#define OHOS_IPC_IPC_SKELETON_H

#include "iremote_object.h"

namespace OHOS {
class IPCSkeleton {
public:
    IPCSkeleton() = default;
    ~IPCSkeleton() = default;

    static uint32_t GetCallingTokenID();
    static uint32_t GetCallingPid();
    static uint32_t GetCallingUid();

    static void SetCallingTokenId(uint32_t tokenId);
    static void SetCallingPid(uint32_t pId);
    static void SetCallingUid(uint32_t uId);

    static void ResetTokenId();
    static void ResetPId();
    static void Reset();

    static uint32_t callerTokenId;
    static uint32_t callerPId;
    static uint32_t callerUId;
};
} // namespace OHOS
#endif // OHOS_IPC_IPC_SKELETON_H