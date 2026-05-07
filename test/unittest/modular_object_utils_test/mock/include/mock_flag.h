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

#ifndef MOCK_FLAG_H
#define MOCK_FLAG_H

#include <stdint.h>
#include <sys/types.h>

class MockFlag {
public:
    // AppUtils
    static bool isSupportModularObjectExtension;

    // IPCSkeleton
    static int32_t callingUid;
    static pid_t callingPid;

    // AppMgrClient
    static int32_t getRunningProcessInfoRet;
    static int32_t processState;
    static bool isPreForeground;

    // system::GetBoolParameter
    static bool isDeveloperMode;

    // ModularObjectExtensionRdbStorageMgr
    static int32_t queryDataRet;
    static bool extensionFound;
    static bool extensionDisabled;

    // BundleMgrHelper
    static bool bundleMgrHelperNull;
    static int32_t getNameAndIndexRet;
    static int32_t getOsAccountRet;
    static bool getApplicationInfoRet;

    // AbilityManagerService
    static bool amsNull;
    static bool isSceneBoardEnabled;
    static bool hasRunningUIAbility;
    static bool hasRunningUIExtension;

    // MissionListManager
    static bool missionListMgrNull;
    // UIAbilityManager
    static bool uiAbilityMgrNull;
    // UIExtensionAbilityManager
    static bool uiExtMgrNull;

    // PermissionVerification
    static bool isSACall;
    static int32_t checkCallModularObjectExtensionPermissionRet;

    // RateLimiter
    static bool modularObjectLimited;
};

#endif // MOCK_FLAG_H
