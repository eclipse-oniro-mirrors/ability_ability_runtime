/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "mock_iability_manager_collaborator.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {

IAbilityManagerCollaborator::IAbilityManagerCollaborator()
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "IAbilityManagerCollaborator init");
}

IAbilityManagerCollaborator::~IAbilityManagerCollaborator()
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "IAbilityManagerCollaborator release");
}

int32_t IAbilityManagerCollaborator::GrantUriPermission(const std::vector<std::string> &uriVec, uint32_t flag,
    uint32_t targetTokenId, const std::string &targetBundleName)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission: %{public}d", grantResult);
    return grantResult;
}

int32_t IAbilityManagerCollaborator::RevokeUriPermission(uint32_t tokenId)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "RevokeUriPermission: %{public}d", revokeResult);
    return revokeResult;
}

int32_t IAbilityManagerCollaborator::VerifyUriPermission(const std::vector<std::string> &uriVec, uint32_t flag,
    uint32_t tokenId, std::vector<bool> &checkResults)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermission: %{public}d", verifyResult);
    checkResults.resize(uriVec.size(), true);
    return verifyResult;
}

int32_t IAbilityManagerCollaborator::NotifyGrantUriPermissionStart(const std::vector<std::string> &uris, uint32_t flag,
    int32_t userId)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "NotifyGrantUriPermissionStart: %{public}d", grantResult);
    return grantResult;
}

int32_t IAbilityManagerCollaborator::NotifyGrantUriPermissionEnd(const std::vector<std::string> &uris, uint32_t flag,
    int32_t userId, const std::vector<bool> &checkResults)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "NotifyGrantUriPermissionEnd: %{public}d", grantResult);
    return grantResult;
}

int32_t IAbilityManagerCollaborator::grantResult = 0;
int32_t IAbilityManagerCollaborator::revokeResult = 0;
int32_t IAbilityManagerCollaborator::verifyResult = 0;
}  // namespace AAFwk
}  // namespace OHOS