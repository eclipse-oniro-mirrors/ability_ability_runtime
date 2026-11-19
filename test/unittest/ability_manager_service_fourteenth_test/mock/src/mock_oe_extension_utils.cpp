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

#include "utils/oe_extension_utils.h"

#include "mock_my_status.h"

namespace OHOS {
namespace AAFwk {

OEExtensionUtils &OEExtensionUtils::GetInstance()
{
    static OEExtensionUtils instance;
    return instance;
}

int32_t OEExtensionUtils::ValidateCaller(
    int32_t callingUid,
    const Want &want,
    const sptr<IRemoteObject> &callerToken,
    int32_t hostPid,
    std::string &hostBundleName,
    int32_t &userId)
{
    auto &status = MyStatus::GetInstance();
    if (status.oeuValidateCallerResult_ != ERR_OK) {
        return status.oeuValidateCallerResult_;
    }
    hostBundleName = status.oeuValidateCallerHostBundleName_;
    userId = status.oeuValidateCallerUserId_;
    return ERR_OK;
}

void OEExtensionUtils::AddOEExtRequest(int32_t requestId) {}

bool OEExtensionUtils::RemoveOEExtRequest(int32_t requestId)
{
    return false;
}

void OEExtensionUtils::ScheduleDelayedCleanup(int32_t requestId) {}

} // namespace AAFwk
} // namespace OHOS
