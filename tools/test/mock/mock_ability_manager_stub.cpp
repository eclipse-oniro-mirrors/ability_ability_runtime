/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "mock_ability_manager_stub.h"
#include "hilog_tag_wrapper.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

Want MockAbilityManagerStub::capturedWant_;
bool MockAbilityManagerStub::captured_ = false;

const Want& MockAbilityManagerStub::GetCapturedWant()
{
    return capturedWant_;
}

bool MockAbilityManagerStub::HasCapturedWant()
{
    return captured_;
}

void MockAbilityManagerStub::ResetCapturedWant()
{
    captured_ = false;
}

namespace {
const std::string STRING_ABILITY_NAME_INVALID = "invalid_ability";
const std::string STRING_BUNDLE_NAME_INVALID = "invalid_bundle";
}  // namespace

int MockAbilityManagerStub::StartAbility(
    const Want& want, int32_t userId, int requestCode, uint64_t specifiedFullTokenId)
{
    TAG_LOGI(AAFwkTag::TEST, "[%{public}s(%{public}s)] enter", __FILE__, __FUNCTION__);

    // Capture the want so unit tests can assert on the parameters that reached StartAbility.
    capturedWant_ = want;
    captured_ = true;

    AppExecFwk::ElementName element = want.GetElement();

    std::string abilityName = element.GetAbilityName();
    TAG_LOGI(AAFwkTag::TEST, "abilityName: %{public}s", abilityName.c_str());
    if (abilityName == STRING_ABILITY_NAME_INVALID) {
        return RESOLVE_ABILITY_ERR;
    }

    std::string bundleName = element.GetBundleName();
    TAG_LOGI(AAFwkTag::TEST, "bundleName: %{public}s", bundleName.c_str());
    if (bundleName == STRING_BUNDLE_NAME_INVALID) {
        return RESOLVE_APP_ERR;
    }

    auto isDebugApp = want.GetBoolParam("debugApp", false);
    TAG_LOGI(AAFwkTag::TEST, "isDebugApp: %{public}d", isDebugApp);

    return ERR_OK;
}

void MockAbilityManagerStub::DumpState(const std::string& args, std::vector<std::string>& state)
{
    TAG_LOGI(AAFwkTag::TEST, "[%{public}s(%{public}s)] enter", __FILE__, __FUNCTION__);

    std::vector<std::string> argList;
    SplitStr(args, " ", argList);

    std::string command = argList[0];
    if (command == "--all" || command == "-a") {
        // do nothing
    } else if (command == "--stack-list" || command == "-l") {
        // do nothing
    } else if (command == "--stack" || command == "-s") {
        state.push_back(argList[1]);
    } else if (command == "--mission" || command == "-m") {
        state.push_back(argList[1]);
    } else {
        // do nothing
    }
}

int MockAbilityManagerStub::StopServiceAbility(const Want& want, int32_t userId,
    const sptr<IRemoteObject> &token)
{
    TAG_LOGI(AAFwkTag::TEST, "[%{public}s(%{public}s)] enter", __FILE__, __FUNCTION__);

    AppExecFwk::ElementName element = want.GetElement();

    std::string abilityName = element.GetAbilityName();
    TAG_LOGI(AAFwkTag::TEST, "abilityName: %{public}s", abilityName.c_str());
    if (abilityName == STRING_ABILITY_NAME_INVALID) {
        return RESOLVE_ABILITY_ERR;
    }

    std::string bundleName = element.GetBundleName();
    TAG_LOGI(AAFwkTag::TEST, "bundleName: %{public}s", bundleName.c_str());
    if (bundleName == STRING_BUNDLE_NAME_INVALID) {
        return RESOLVE_APP_ERR;
    }

    return ERR_OK;
}
