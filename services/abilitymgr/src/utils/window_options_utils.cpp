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

#include "utils/window_options_utils.h"

#include "ability_record.h"
#include "app_utils.h"

namespace OHOS {
namespace AAFwk {
void WindowOptionsUtils::SetWindowPositionAndSize(Want& want,
    const sptr<IRemoteObject>& callerToken, const StartOptions& startOptions)
{
    if (!AppUtils::GetInstance().IsStartOptionsWithAnimation()) {
        return;
    }
    if (startOptions.windowLeftUsed_) {
        want.SetParam(Want::PARAM_RESV_WINDOW_LEFT, startOptions.GetWindowLeft());
    }
    if (startOptions.windowTopUsed_) {
        want.SetParam(Want::PARAM_RESV_WINDOW_TOP, startOptions.GetWindowTop());
    }
    if (startOptions.windowWidthUsed_) {
        want.SetParam(Want::PARAM_RESV_WINDOW_WIDTH, startOptions.GetWindowWidth());
    }
    if (startOptions.windowHeightUsed_) {
        want.SetParam(Want::PARAM_RESV_WINDOW_HEIGHT, startOptions.GetWindowHeight());
    }
    if (startOptions.minWindowWidthUsed_) {
        want.SetParam(Want::PARAM_RESV_MIN_WINDOW_WIDTH, startOptions.GetMinWindowWidth());
    }
    if (startOptions.minWindowHeightUsed_) {
        want.SetParam(Want::PARAM_RESV_MIN_WINDOW_HEIGHT, startOptions.GetMinWindowHeight());
    }
    if (startOptions.maxWindowWidthUsed_) {
        want.SetParam(Want::PARAM_RESV_MAX_WINDOW_WIDTH, startOptions.GetMaxWindowWidth());
    }
    if (startOptions.maxWindowHeightUsed_) {
        want.SetParam(Want::PARAM_RESV_MAX_WINDOW_HEIGHT, startOptions.GetMaxWindowHeight());
    }
    bool withAnimation = startOptions.GetWithAnimation();
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (!withAnimation && abilityRecord != nullptr &&
        abilityRecord->GetAbilityInfo().bundleName == want.GetBundle()) {
        want.SetParam(Want::PARAM_RESV_WITH_ANIMATION, withAnimation);
    }
}

std::pair<bool, AppExecFwk::SupportWindowMode> WindowOptionsUtils::WindowModeMap(int32_t windowMode)
{
    std::pair<bool, AppExecFwk::SupportWindowMode> result(false, AppExecFwk::SupportWindowMode::FULLSCREEN);

    if (windowMode == MULTI_WINDOW_DISPLAY_FULLSCREEN) {
        result.first = true;
        result.second = AppExecFwk::SupportWindowMode::FULLSCREEN;
    } else if (windowMode == MULTI_WINDOW_DISPLAY_PRIMARY) {
        result.first = true;
        result.second = AppExecFwk::SupportWindowMode::SPLIT;
    } else if (windowMode == MULTI_WINDOW_DISPLAY_SECONDARY) {
        result.first = true;
        result.second = AppExecFwk::SupportWindowMode::SPLIT;
    } else if (windowMode == MULTI_WINDOW_DISPLAY_SPLIT) {
        result.first = true;
        result.second = AppExecFwk::SupportWindowMode::SPLIT;
    } else if (windowMode == MULTI_WINDOW_DISPLAY_FLOATING) {
        result.first = true;
        result.second = AppExecFwk::SupportWindowMode::FLOATING;
    }
    return result;
}
}  // namespace AAFwk
}  // namespace OHOS
