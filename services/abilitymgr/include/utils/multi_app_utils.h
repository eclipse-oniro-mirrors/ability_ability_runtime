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

#ifndef OHOS_ABILITY_RUNTIME_MULTI_APP_UTILS_H
#define OHOS_ABILITY_RUNTIME_MULTI_APP_UTILS_H

#include <string>

namespace OHOS {
namespace AAFwk {
/**
 * @class MultiAppUtils
 * provides multi-app utilities.
 */
class MultiAppUtils {
public:
    /**
     * GetRunningMultiAppIndex, get multi-app running application index.
     *
     * @param bundleName The bundle name of the app.
     * @param uid The calling uid.
     * @param appIndex The resultant application index.
     */
    static void GetRunningMultiAppIndex(const std::string &bundleName, int32_t uid, int32_t &appIndex);

    /**
     * GetPreferredAppCloneIndex, get the preferred app index configured by bundle manager.
     *
     * @param bundleName The bundle name of the app.
     * @param userId The user ID.
     * @param appIndex The preferred application index, 0 for main app.
     * @return Returns true if a valid main app or clone app preference is found.
     */
    static bool GetPreferredAppCloneIndex(const std::string &bundleName, int32_t userId, int32_t &appIndex);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MULTI_APP_UTILS_H
