/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "extension_context.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t ExtensionContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("ExtensionContext"));

void ExtensionContext::SetAbilityInfo(const std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> &abilityInfo)
{
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null abilityInfo");
        return;
    }
    abilityInfo_ = abilityInfo;
}

std::shared_ptr<AppExecFwk::AbilityInfo> ExtensionContext::GetAbilityInfo() const
{
    return abilityInfo_;
}

ErrCode ExtensionContext::AddCompletionHandler(const std::string &requestId, OnRequestResult onRequestSucc,
    OnRequestResult onRequestFail)
{
    return ERR_OK;
}

void ExtensionContext::OnRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element,
    const std::string &message)
{
}

void ExtensionContext::OnRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
    const std::string &message)
{
}
}  // namespace AbilityRuntime
}  // namespace OHOS
