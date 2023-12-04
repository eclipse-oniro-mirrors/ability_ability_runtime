/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "extension_record.h"
#include "ability_util.h"
#include "errors.h"

namespace OHOS {
namespace AbilityRuntime {
ExtensionRecord::ExtensionRecord(const std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord,
    const std::string &hostBundleName, int32_t extensionRecordId)
    : abilityRecord_(abilityRecord), hostBundleName_(hostBundleName), extensionRecordId_(extensionRecordId)
{}

ExtensionRecord::~ExtensionRecord() = default;

sptr<IRemoteObject> ExtensionRecord::GetCallToken() const
{
    CHECK_POINTER_AND_RETURN(abilityRecord_, nullptr);
    auto sessionInfo = abilityRecord_->GetSessionInfo();
    CHECK_POINTER_AND_RETURN(sessionInfo, nullptr);
    return sessionInfo->callerToken;
}

sptr<IRemoteObject> ExtensionRecord::GetRootCallerToken() const
{
    return rootCallerToken_;
}

void ExtensionRecord::SetRootCallerToken(sptr<IRemoteObject> &rootCallerToken)
{
    rootCallerToken_ = rootCallerToken;
}

bool ExtensionRecord::ContinueToGetCallerToken()
{
    return false;
}
} // namespace AbilityRuntime
} // namespace OHOS