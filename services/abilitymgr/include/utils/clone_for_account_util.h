/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CLONE_FOR_ACCOUNT_UTIL_H
#define OHOS_ABILITY_RUNTIME_CLONE_FOR_ACCOUNT_UTIL_H

#include <mutex>
#include <unordered_map>
#include <string>
#include "ability_info.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class CloneForAccountUtil {
public:
    static void ProcessAppIndex(Want &want, sptr<IRemoteObject> callerToken, int32_t userId);

    static bool GetCachedAppIndex(const std::string &bundleName, int32_t &appIndex);
    static void CacheAppIndex(const std::string &bundleName, int32_t appIndex);
    static void RemoveCachedAppIndex(const std::string &bundleName);

private:
    static std::mutex mapMutex_;
    static std::unordered_map<std::string, int32_t> appIndexMap_;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_CLONE_FOR_ACCOUNT_UTIL_H
