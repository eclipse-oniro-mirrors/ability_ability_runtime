/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_UPDATE_CALLER_INFO_UTIL_H
#define OHOS_ABILITY_RUNTIME_UPDATE_CALLER_INFO_UTIL_H

#include <string>
#include "caller_info.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class UpdateCallerInfoUtil {
public:
    static UpdateCallerInfoUtil &GetInstance();
    ~UpdateCallerInfoUtil() = default;

    void UpdateAsCallerSourceInfo(Want& want, sptr<IRemoteObject> asCallerSourceToken, sptr<IRemoteObject> callerToken);
    void UpdateByIndirectCallerInfo(std::shared_ptr<IndirectCallerInfo> indirectCallerInfo, int32_t &tokenId,
        int32_t &callerUid, int32_t &callerPid);
    void UpdateCallerInfo(Want& want, const sptr<IRemoteObject> &callerToken,
        std::shared_ptr<IndirectCallerInfo> indirectCallerInfo = nullptr);
    void UpdateBackToCallerFlag(const sptr<IRemoteObject> &callerToken, Want &want, int32_t requestCode, bool backFlag);
    void UpdateCallerInfoFromToken(Want& want, const sptr<IRemoteObject> &token);
    void UpdateDmsCallerInfo(Want& want, const sptr<IRemoteObject> &callerToken);

private:
    UpdateCallerInfoUtil() = default;
    void UpdateSignatureInfo(std::string bundleName, Want& want, bool isRemote = false);
    void UpdateAsCallerInfoFromToken(Want& want, sptr<IRemoteObject> asCallerSourceToken);
    void UpdateAsCallerInfoFromCallerRecord(Want& want, sptr<IRemoteObject> callerToken);
    bool UpdateAsCallerInfoFromDialog(Want& want);
    void UpdateCallerBundleName(Want& want, const std::string &bundleName);
    void UpdateCallerAbilityName(Want& want, const std::string &abilityName);
    void UpdateCallerAppCloneIndex(Want& want, int32_t appIndex);
    void ClearProtectedWantParam(Want &want);

    DISALLOW_COPY_AND_MOVE(UpdateCallerInfoUtil);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_DIALOG_SESSION_MANAGEER_H
