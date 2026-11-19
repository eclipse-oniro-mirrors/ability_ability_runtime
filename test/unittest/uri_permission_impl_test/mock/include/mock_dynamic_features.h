/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MOCK_DYNAMIC_FEATURES_H
#define MOCK_DYNAMIC_FEATURES_H

#include <string>
#include <vector>

#include "ability_manager_errors.h"
#include "feature/imedia_perm_feature.h"
#include "feature/istorage_share_feature.h"

namespace OHOS {
namespace AAFwk {
class MockStorageShareFeature : public IStorageShareFeature {
public:
    inline static bool isZero = true;
    void CreateShareFile(const std::vector<std::string> &uris, uint32_t targetTokenId, uint32_t flag,
        std::vector<int32_t> &resVec) override
    {
        int32_t size = static_cast<int32_t>(uris.size());
        if (size <= 0) {
            return; // resVec stays empty; caller detects failure
        }
        if (isZero) {
            resVec.assign(size, ERR_OK);
        } else {
            resVec.assign(size, -1);
        }
    }

    int32_t DeleteShareFile(uint32_t targetTokenId, const std::vector<std::string> &uris) override
    {
        return ERR_OK;
    }
};

class MockMediaPermFeature : public IMediaPermFeature {
public:
    inline static int32_t grantRet = ERR_OK;  // GrantUriPermission return
    inline static int32_t revokeRet = ERR_OK; // RevokeUriPermission return
    inline static bool checkRet = false;      // CheckUriPermission per-uri result

    // Last-call parameter capture (for forwarder pass-through assertions).
    inline static std::vector<std::string> lastCheckUris;
    inline static uint32_t lastCheckCallerTokenId = 0;
    inline static uint32_t lastCheckFlag = 0;
    inline static std::vector<std::string> lastGrantUris;
    inline static uint32_t lastGrantFlag = 0;
    inline static uint32_t lastGrantCallerTokenId = 0;
    inline static uint32_t lastGrantTargetTokenId = 0;
    inline static int32_t lastGrantHideSensitiveType = 0;
    inline static uint32_t lastRevokeCallerTokenId = 0;
    inline static uint32_t lastRevokeTargetTokenId = 0;
    inline static std::string lastRevokeUri;

    static void Reset()
    {
        grantRet = ERR_OK;
        revokeRet = ERR_OK;
        checkRet = false;
        lastCheckUris.clear();
        lastCheckCallerTokenId = 0;
        lastCheckFlag = 0;
        lastGrantUris.clear();
        lastGrantFlag = 0;
        lastGrantCallerTokenId = 0;
        lastGrantTargetTokenId = 0;
        lastGrantHideSensitiveType = 0;
        lastRevokeCallerTokenId = 0;
        lastRevokeTargetTokenId = 0;
        lastRevokeUri.clear();
    }

    std::vector<bool> CheckUriPermission(const std::vector<std::string> &uriVec, uint32_t callerTokenId,
        uint32_t flag) override
    {
        lastCheckUris = uriVec;
        lastCheckCallerTokenId = callerTokenId;
        lastCheckFlag = flag;
        return std::vector<bool>(uriVec.size(), checkRet);
    }

    int32_t GrantUriPermission(const std::vector<std::string> &uris, uint32_t flag, uint32_t callerTokenId,
        uint32_t targetTokenId, int32_t hideSensitiveType) override
    {
        lastGrantUris = uris;
        lastGrantFlag = flag;
        lastGrantCallerTokenId = callerTokenId;
        lastGrantTargetTokenId = targetTokenId;
        lastGrantHideSensitiveType = hideSensitiveType;
        return grantRet;
    }

    int32_t RevokeUriPermission(uint32_t callerTokenId, uint32_t targetTokenId, const std::string &uri) override
    {
        lastRevokeCallerTokenId = callerTokenId;
        lastRevokeTargetTokenId = targetTokenId;
        lastRevokeUri = uri;
        return revokeRet;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // MOCK_DYNAMIC_FEATURES_H
