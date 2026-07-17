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

#ifndef OHOS_ABILITY_RUNTIME_TOKENID_PERMISSION_H
#define OHOS_ABILITY_RUNTIME_TOKENID_PERMISSION_H

#include <string>
#include <sys/types.h>

namespace OHOS {
namespace AAFwk {

class TokenIdPermission {
public:
    TokenIdPermission(uint32_t tokenId)
    {
        tokenId_ = tokenId;
    }

    uint32_t GetTokenId()
    {
        return tokenId_;
    }

    bool VerifyProxyAuthorizationUriPermission();

    bool VerifyFileAccessManagerPermission();

    bool VerifyReadImageVideoPermission();

    bool VerifyWriteImageVideoPermission();

    bool VerifyReadAudioPermission();

    bool VerifyWriteAudioPermission();

    bool VerifyRWDownloadPermission();

    bool VerifyRWDeskTopPermission();

    bool VerifyRWDocumentsPermission();

    bool VerifySandboxAccessPermission();

    bool CheckIsPermissionRequestedByTokenId(uint32_t tokenId, const std::string &permissionName,
        bool isSystemGrant);

private:
    uint32_t tokenId_ = 0;

    bool haveFileAccessManagerPermission_ = false;
    bool haveReadImageVideoPermission_ = false;
    bool haveWriteImageVideoPermission_ = false;
    bool haveReadAudioPermission_ = false;
    bool haveWriteAudioPermission_ = false;
    bool haveProxyAuthorizationUriPermission_ = false;
    bool haveRWDownloadPermission_ = false;
    bool haveRWDeskTopPermission_ = false;
    bool haveRWDocumentsPermission_ = false;
    bool haveSandboxAccessPermission_ = false;

    bool initFileAccessManagerPermission_ = false;
    bool initReadImageVideoPermission_ = false;
    bool initWriteImageVideoPermission_ = false;
    bool initReadAudioPermission_ = false;
    bool initWriteAudioPermission_ = false;
    bool initProxyAuthorizationUriPermission_ = false;
    bool initRWDownloadPermission_ = false;
    bool initRWDeskTopPermission_ = false;
    bool initRWDocumentsPermission_ = false;
    bool initSandboxAccessPermission_ = false;
};
}  // OHOS
}  // AAFwk
#endif  // OHOS_ABILITY_RUNTIME_TOKENID_PERMISSION_H