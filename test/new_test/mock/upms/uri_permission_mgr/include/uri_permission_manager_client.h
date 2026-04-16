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

#ifndef OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_CLIENT_H
#define OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_CLIENT_H

#include <string>
#include <vector>

#include "policy_info.h"
#include "uri.h"

namespace OHOS {
namespace AAFwk {
class UriPermissionManagerClient {
public:
    static UriPermissionManagerClient& GetInstance();
    ~UriPermissionManagerClient() = default;

    int GrantUriPermission(const Uri &uri, uint32_t flag, const std::string targetBundleName, int32_t appIndex = 0,
        uint32_t initiatorTokenId = 0)
    {
        return 0;
    }

    int GrantUriPermission(const std::vector<Uri> &uriVec, uint32_t flag, const std::string targetBundleName,
        int32_t appIndex = 0, uint32_t initiatorTokenId = 0)
    {
        return 0;
    }

    int32_t GrantUriPermissionPrivileged(const std::vector<Uri> &uriVec, uint32_t flag,
        const std::string &targetBundleName, int32_t appIndex = 0, uint32_t initiatorTokenId = 0,
        int32_t hideSensitiveType = 0)
    {
        return 0;
    }

    int RevokeAllUriPermissions(uint32_t tokenId)
    {
        return 0;
    }

    int RevokeUriPermissionManually(const Uri &uri, const std::string bundleName, int32_t appIndex = 0)
    {
        return 0;
    }

    bool VerifyUriPermission(const Uri& uri, uint32_t flag, uint32_t tokenId)
    {
        return false;
    }

    std::vector<bool> CheckUriAuthorization(const std::vector<std::string> &uriVec, uint32_t flag, uint32_t tokenId)
    {
        std::vector<bool> result(uriVec.size(), false);
        return result;
    }

    int32_t ClearPermissionTokenByMap(uint32_t tokenId)
    {
        return 0;
    }

    int32_t GrantUriPermissionByKey(const std::string &key, uint32_t flag, uint32_t targetTokenId)
    {
        return 0;
    }

    int32_t GrantUriPermissionByKeyAsCaller(const std::string &key, uint32_t flag, uint32_t callerTokenId,
        uint32_t targetTokenId)
    {
        return 0;
    }

    int32_t Active(const std::vector<PolicyInfo> &policy, std::vector<uint32_t> &result)
    {
        return 0;
    }

    void SetUriPermServiceStarted() {}

    bool IsUriPermServiceStarted()
    {
        return false;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_CLIENT_H
