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

#include "mock_flag.h"
#include "ability_record.h"
#include "base_extension_record.h"
#include "ability_record/ability_record_utils.h"

bool MockFlag::isSupportModularObjectExtension = true;
int32_t MockFlag::callingUid = 1000;
pid_t MockFlag::callingPid = 1234;
int32_t MockFlag::getRunningProcessInfoRet = 0;
int32_t MockFlag::processState = 2; // APP_STATE_FOREGROUND
bool MockFlag::isPreForeground = false;
bool MockFlag::isDeveloperMode = false;
int32_t MockFlag::queryDataRet = 0;
bool MockFlag::extensionFound = true;
bool MockFlag::extensionDisabled = false;
bool MockFlag::bundleMgrHelperNull = false;
int32_t MockFlag::getNameAndIndexRet = 0;
int32_t MockFlag::getOsAccountRet = 0;
bool MockFlag::getApplicationInfoRet = true;
bool MockFlag::amsNull = false;
bool MockFlag::isSceneBoardEnabled = true;
bool MockFlag::hasRunningUIAbility = true;
bool MockFlag::hasRunningUIExtension = false;
bool MockFlag::missionListMgrNull = false;
bool MockFlag::uiAbilityMgrNull = false;
bool MockFlag::uiExtMgrNull = false;
bool MockFlag::isSACall = false;
int32_t MockFlag::checkCallModularObjectExtensionPermissionRet = 0;
bool MockFlag::modularObjectLimited = false;

// Token mock
std::shared_ptr<OHOS::AAFwk::AbilityRecord> OHOS::AAFwk::Token::abilityRecord_ = nullptr;

// AbilityRecord mock
OHOS::AppExecFwk::AbilityInfo OHOS::AAFwk::AbilityRecord::abilityInfo;
pid_t OHOS::AAFwk::BaseExtensionRecord::clientPid = -1;
