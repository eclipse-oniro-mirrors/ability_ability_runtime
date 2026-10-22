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

#include "mock_my_flag.h"

namespace OHOS {
namespace AppExecFwk {
std::shared_ptr<MyFlag> MyFlag::flagInstance_ = nullptr;
std::once_flag MyFlag::flagOnce_;
ErrCode MyFlag::startAbility_ = 0;
ErrCode MyFlag::startAbilityAsCaller_ = 0;
ErrCode MyFlag::minimizeAbility_ = 0;
ErrCode MyFlag::moveAbilityToBackground_ = 0;
ErrCode MyFlag::terminateAbility_ = 0;
ErrCode MyFlag::closeAbility_ = 0;
int32_t MyFlag::missionId_ = 0;
ErrCode MyFlag::missionIdByToken_ = 0;
ErrCode MyFlag::missionLabel_ = 0;
ErrCode MyFlag::missionIcon_ = 0;
ErrCode MyFlag::changeAbilityVisibility_ = 0;
ErrCode MyFlag::requestModalUIExtension_ = 0;
ErrCode MyFlag::addFreeInstallObserver_ = 0;
int32_t MyFlag::openAtomicService_ = 0;
bool MyFlag::backToCallerApp_ = false;

MyFlag::MyFlag() {}

std::shared_ptr<MyFlag> MyFlag::GetInstance()
{
    std::call_once(flagOnce_, [] () {
        flagInstance_ = std::make_shared<MyFlag>();
    });
    return flagInstance_;
}

ErrCode MyFlag::GetStartAbility()
{
    return startAbility_;
}

void MyFlag::SetStartAbility(ErrCode startAbility)
{
    startAbility_ = startAbility;
}

ErrCode MyFlag::GetStartAbilityAsCaller()
{
    return startAbilityAsCaller_;
}

void MyFlag::SetStartAbilityAsCaller(ErrCode startAbilityAsCaller)
{
    startAbilityAsCaller_ = startAbilityAsCaller;
}

ErrCode MyFlag::GetMinimizeAbility()
{
    return minimizeAbility_;
}

void MyFlag::SetMinimizeAbility(ErrCode minimizeAbility)
{
    minimizeAbility_ = minimizeAbility;
}

ErrCode MyFlag::GetMoveAbilityToBackground()
{
    return moveAbilityToBackground_;
}

void MyFlag::SetMoveAbilityToBackground(ErrCode moveAbilityToBackground)
{
    moveAbilityToBackground_ = moveAbilityToBackground;
}

ErrCode MyFlag::GetTerminateAbility()
{
    return terminateAbility_;
}

void MyFlag::SetTerminateAbility(ErrCode terminateAbility)
{
    terminateAbility_ = terminateAbility;
}

ErrCode MyFlag::GetCloseAbility()
{
    return closeAbility_;
}

void MyFlag::SetCloseAbility(ErrCode closeAbility)
{
    closeAbility_ = closeAbility;
}

int32_t MyFlag::GetMissionId()
{
    return missionId_;
}

void MyFlag::SetMissionId(const int32_t& missionId)
{
    missionId_ = missionId;
}

ErrCode MyFlag::GetMissionIdByToken()
{
    return missionIdByToken_;
}
    
void MyFlag::SetMissionIdByToken(ErrCode missionIdByToken)
{
    missionIdByToken_ = missionIdByToken;
}

ErrCode MyFlag::GetMissionLabel()
{
    return missionLabel_;
}
void MyFlag::SetMissionLabel(ErrCode missionLabel)
{
    missionLabel_ = missionLabel;
}

ErrCode MyFlag::GetMissionIcon()
{
    return missionIcon_;
}

void MyFlag::SetMissionIcon(ErrCode missionIcon)
{
    missionIcon_ = missionIcon;
}

ErrCode MyFlag::GetChangeAbilityVisibility()
{
    return changeAbilityVisibility_;
}

void MyFlag::SetChangeAbilityVisibility(ErrCode changeAbilityVisibility)
{
    changeAbilityVisibility_ = changeAbilityVisibility;
}

ErrCode MyFlag::GetRequestModalUIExtension()
{
    return requestModalUIExtension_;
}

void MyFlag::SetRequestModalUIExtension(ErrCode requestModalUIExtension)
{
    requestModalUIExtension_ = requestModalUIExtension;
}

ErrCode MyFlag::GetAddFreeInstallObserver()
{
    return addFreeInstallObserver_;
}

void MyFlag::SetAddFreeInstallObserver(ErrCode addFreeInstallObserver)
{
    addFreeInstallObserver_ = addFreeInstallObserver;
}

int32_t MyFlag::GetOpenAtomicService()
{
    return openAtomicService_;
}

void MyFlag::SetOpenAtomicService(int32_t openAtomicService)
{
    openAtomicService_ = openAtomicService;
}

bool MyFlag::GetBackToCallerApp()
{
    return backToCallerApp_;
}

void MyFlag::SetBackToCallerApp(bool isBackToCallerApp)
{
    backToCallerApp_ = isBackToCallerApp;
}
} // namespace AppExecFwk
} // namespace OHOS