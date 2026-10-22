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

#ifndef MOCK_MY_FLAG_H
#define MOCK_MY_FLAG_H

#include <cstdint>
#include <memory>
#include <mutex>
#include "ability_manager_errors.h"

namespace OHOS {
namespace AppExecFwk {
class MyFlag {
public:
    static std::shared_ptr<MyFlag> GetInstance();
    MyFlag();

    ErrCode GetStartAbility();
    void SetStartAbility(ErrCode startAbility);

    ErrCode GetStartAbilityAsCaller();
    void SetStartAbilityAsCaller(ErrCode startAbilityAsCaller);

    ErrCode GetMinimizeAbility();
    void SetMinimizeAbility(ErrCode minimizeAbility);

    ErrCode GetMoveAbilityToBackground();
    void SetMoveAbilityToBackground(ErrCode moveAbilityToBackground);

    ErrCode GetTerminateAbility();
    void SetTerminateAbility(ErrCode terminateAbility);

    ErrCode GetCloseAbility();
    void SetCloseAbility(ErrCode closeAbility);

    ErrCode GetMissionId();
    void SetMissionId(const int32_t& missionId);

    ErrCode GetMissionIdByToken();
    void SetMissionIdByToken(ErrCode missionIdByToken);

    ErrCode GetMissionLabel();
    void SetMissionLabel(ErrCode missionLabel);

    ErrCode GetMissionIcon();
    void SetMissionIcon(ErrCode missionIcon);

    ErrCode GetChangeAbilityVisibility();
    void SetChangeAbilityVisibility(ErrCode changeAbilityVisibility);

    ErrCode GetRequestModalUIExtension();
    void SetRequestModalUIExtension(ErrCode requestModalUIExtension);

    ErrCode GetAddFreeInstallObserver();
    void SetAddFreeInstallObserver(ErrCode addFreeInstallObserver);

    int32_t GetOpenAtomicService();
    void SetOpenAtomicService(int32_t openAtomicService);

    bool GetBackToCallerApp();
    void SetBackToCallerApp(bool isBackToCallerApp);

private:
    static std::shared_ptr<MyFlag> flagInstance_;
    static std::once_flag flagOnce_;

    static ErrCode startAbility_;
    static ErrCode startAbilityAsCaller_;
    static ErrCode minimizeAbility_;
    static ErrCode moveAbilityToBackground_;
    static ErrCode terminateAbility_;
    static ErrCode closeAbility_;
    static int32_t missionId_;
    static ErrCode missionIdByToken_;
    static ErrCode missionLabel_;
    static ErrCode missionIcon_;
    static ErrCode changeAbilityVisibility_;
    static ErrCode requestModalUIExtension_;
    static ErrCode addFreeInstallObserver_;
    static int32_t openAtomicService_;
    static bool backToCallerApp_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif // MOCK_MY_FLAG_H