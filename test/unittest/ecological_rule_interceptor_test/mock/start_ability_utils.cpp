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

#include "start_ability_utils.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t ERMS_ISALLOW_RESULTCODE = 10;
}
std::shared_ptr<StartAbilityInfo> StartAbilityUtils::startAbilityInfo;
std::shared_ptr<StartAbilityInfo> StartAbilityUtils::callerAbilityInfo;
bool StartAbilityUtils::skipCrowTest = false;
bool StartAbilityUtils::skipErms = false;
int32_t StartAbilityUtils::ermsResultCode = ERMS_ISALLOW_RESULTCODE;
bool StartAbilityUtils::isWantWithAppCloneIndex = false;
bool StartAbilityUtils::ermsSupportBackToCallerFlag = false;
bool StartAbilityUtils::retGetCallerAbilityInfo;
AppExecFwk::AbilityInfo StartAbilityUtils::callerAbiltyInfo;

std::shared_ptr<StartAbilityInfo> StartAbilityInfo::createStartAbilityInfo = nullptr;

std::shared_ptr<StartAbilityInfo> StartAbilityInfo::CreateStartAbilityInfo(const Want &want, int32_t userId,
    int32_t appIndex, sptr<IRemoteObject> callerToken)
{
    return createStartAbilityInfo;
}

bool StartAbilityUtils::GetCallerAbilityInfo(const sptr<IRemoteObject> &callerToken,
    AppExecFwk::AbilityInfo &abilityInfo)
{
    abilityInfo = callerAbiltyInfo;
    return retGetCallerAbilityInfo;
}
}
}