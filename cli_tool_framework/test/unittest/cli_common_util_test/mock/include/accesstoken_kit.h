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

#ifndef OHOS_ABILITY_RUNTIME_CLI_COMMON_MOCK_ACCESSTOKEN_KIT_H
#define OHOS_ABILITY_RUNTIME_CLI_COMMON_MOCK_ACCESSTOKEN_KIT_H

#include <string>
#include <vector>

#include "access_token.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
class AccessTokenKit {
public:
    static int32_t VerifyAccessToken(AccessTokenID tokenId, const std::vector<std::string> &permissions,
        std::vector<int32_t> &permStateList);
    static int32_t VerifyAccessToken(AccessTokenID tokenId, const std::string &permissionName, bool crossUser);
};
} // namespace AccessToken
} // namespace Security
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_COMMON_MOCK_ACCESSTOKEN_KIT_H
