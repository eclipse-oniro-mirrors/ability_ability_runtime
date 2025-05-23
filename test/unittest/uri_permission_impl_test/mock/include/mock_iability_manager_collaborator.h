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

#ifndef OHOS_ABILITY_RUNTIME_IABILITY_MANAGER_COLLABORATOR_H
#define OHOS_ABILITY_RUNTIME_IABILITY_MANAGER_COLLABORATOR_H

#include <vector>
#include <string>

namespace OHOS {
namespace AAFwk {

class IAbilityManagerCollaborator {
public:
    virtual ~IAbilityManagerCollaborator();
    IAbilityManagerCollaborator();
    int32_t GrantUriPermission(const std::vector<std::string> &uriVec, uint32_t flag, uint32_t targetTokenId,
        const std::string &targetBundleName);
    int32_t RevokeUriPermission(uint32_t tokenId);
public:
    static int32_t grantResult;
    static int32_t revokeResult;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_IABILITY_MANAGER_COLLABORATOR_H