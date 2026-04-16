/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef MOCK_MY_STATUS_H
#define MOCK_MY_STATUS_H

#include <string>

#include "scene_board/ui_ability_record.h"
#include "base_extension_record.h"

namespace OHOS {
namespace AAFwk {
class MyStatus {
public:
    static MyStatus &GetInstance()
    {
        static MyStatus instance;
        return instance;
    };

    bool auIsLaunchEmbededUIAbility_ = true;
    int atkGetTokenTypeFlag_ = 0;
    bool smhGetUIAbilityManagerByUid_ = true;
    bool smhGetUIAbilityManagerByUserId_ = true;
    bool smhGetMissionListManagerByUserId_ = true;
    bool smhGetCurrentDataAbilityManager_ = true;
    bool smhGetConnectManagerByToken_ = true;
    bool smhVerificationAllToken_ = true;
    std::shared_ptr<UIAbilityRecord> ualmGetAbilityRecordByToken_ = nullptr;
    int ualmGetSessionIdByAbilityToken_ = 0;
    int fimStartFreeInstall_ = ERR_OK;
    int eriQueryAtomicServiceStartupRule_ = ERR_OK;
    std::shared_ptr<AbilityRecord> arGetAbilityRecord_ = nullptr;
    AppExecFwk::AbilityInfo arGetAbilityInfo_ = {};
    bool sbjIsSceneBoardEnabled_ = false;
    int ipcGetCallingUid_ = 1;
    int ipcGetCallingPid_ = 1;
    uint32_t ipcGetCallingTokenID_ = 1;
    uint32_t ipcGetSelfTokenID_ = 1;
    int permPermission_ = 1;
    bool perJudgeCallerIsAllowedToUseSystemAPI_ = true;
    bool paramGetBoolParameter_ = true;
    std::shared_ptr<BaseExtensionRecord> acmGetUIExtensionBySessionInfo_ = nullptr;
    std::shared_ptr<BaseExtensionRecord> acmGetUIExtensionBySessionFromServiceMap_ = nullptr;
    bool isSupportStartAbilities_ = true;
    bool isSupportMultiInstance_ = true;
    bool isSystemAppCall_ = true;
    bool fimConnectFreeInstallCalled_ = false;
    int32_t fimConnectFreeInstall_ = ERR_OK;
    int32_t fimConnectExtensionType_ = -1;
    std::string fimConnectLocalDeviceId_;
    int32_t softbusGetLocalNodeDeviceInfo_ = ERR_OK;
};
} // namespace AAFwk
} // namespace OHOS
#endif // MOCK_MY_STATUS_H
