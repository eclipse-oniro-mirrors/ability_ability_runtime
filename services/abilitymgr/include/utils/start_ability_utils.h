/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_START_ABILITY_UTILS_H
#define OHOS_ABILITY_RUNTIME_START_ABILITY_UTILS_H

#include <memory>
#include <string>

#include "ability_info.h"
#include "extension_ability_info.h"
#include "start_specified_ability_params.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {

struct StartAbilityInfo {
    static std::shared_ptr<StartAbilityInfo> CreateStartAbilityInfo(const Want &want, int32_t userId,
        int32_t appIndex, sptr<IRemoteObject> callerToken);
    static std::shared_ptr<StartAbilityInfo> CreateCallerAbilityInfo(const sptr<IRemoteObject> &callerToken);

    static std::shared_ptr<StartAbilityInfo> CreateStartExtensionInfo(const Want &want, int32_t userId,
        int32_t appIndex, const std::string &hostBundleName = "");

    static void FindExtensionInfo(const Want &want, int32_t flags, int32_t userId,
        int32_t appIndex, std::shared_ptr<StartAbilityInfo> abilityInfo, const std::string &hostBundleName = "");

    std::string GetAppBundleName() const
    {
        return abilityInfo.applicationInfo.bundleName;
    }

    AppExecFwk::AbilityInfo abilityInfo;
    std::string customProcess;
    int32_t status = ERR_OK;
    AppExecFwk::ExtensionProcessMode extensionProcessMode = AppExecFwk::ExtensionProcessMode::UNDEFINED;
    bool isTargetPlugin = false;
    bool skipAbilityStageLifecycle = false;
};

struct StartAbilityUtils {
    static bool GetAppIndex(const Want &want, sptr<IRemoteObject> callerToken, int32_t &appIndex);
    static bool GetApplicationInfo(const std::string &bundleName, int32_t userId,
        AppExecFwk::ApplicationInfo &appInfo);
    static bool GetCallerAbilityInfo(const sptr<IRemoteObject> &callerToken,
        AppExecFwk::AbilityInfo &abilityInfo);
    static int32_t CheckAppProvisionMode(const Want& want, int32_t userId, sptr<IRemoteObject> callerToken);
    static std::vector<int32_t> GetCloneAppIndexes(const std::string &bundleName, int32_t userId);

    static bool IsCallFromAncoShellOrBroker(const sptr<IRemoteObject> &callerToken);
    static void SetTargetCloneIndexInSameBundle(const Want &want, sptr<IRemoteObject> callerToken);
    static int32_t StartUIAbilitiesProcessAppIndex(Want &want,
        sptr<IRemoteObject> callerToken, int32_t &appIndex);
    static int32_t HandleSelfRedirection(bool isFromOpenLink,
        const std::vector<AppExecFwk::AbilityInfo> &abilityInfos);
    static std::string GenerateAsCallerForAncoSessionId();
    static void RemoveAtomicServiceShareRouterIfNeeded(Want &want, const AppExecFwk::AbilityInfo &targetAbilityInfo);

    static thread_local std::shared_ptr<StartAbilityInfo> startAbilityInfo;
    static thread_local std::shared_ptr<StartAbilityInfo> callerAbilityInfo;
    static thread_local bool skipCrowTest;
    static thread_local bool skipStartOther;
    static thread_local bool skipErms;
    static thread_local int32_t ermsResultCode;
    static thread_local bool isWantWithAppCloneIndex;
    static thread_local bool ermsSupportBackToCallerFlag;
    static thread_local bool startSpecifiedBySCB;
};

struct StartAbilityInfoWrap {
    StartAbilityInfoWrap(const Want &want, int32_t validUserId, int32_t appIndex,
        const sptr<IRemoteObject> &callerToken, bool isExtension = false);
    StartAbilityInfoWrap();
    ~StartAbilityInfoWrap();
    void SetStartAbilityInfo(const AppExecFwk::AbilityInfo& abilityInfo);
};

struct StartAbilityWrapParam {
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    int requestCode = -1;
    bool isPendingWantCaller = false;
    int32_t userId = -1;
    bool isStartAsCaller = false;
    uint32_t specifyTokenId = 0;
    bool isForegroundToRestartApp = false;
    bool isImplicit = false;
    bool isUIAbilityOnly = false;
    bool isAppCloneSelector = false;
    bool hideFailureTipDialog = false;
    bool isBySCB = false;
    bool isFreeInstallFromService = false;
    bool removeInsightIntentFlag = false;
    bool isFromOpenLink = false;
    bool isServiceMatch = false;
    uint64_t specifiedFullTokenId = 0;
    std::shared_ptr<StartSpecifiedAbilityParams> startSpecifiedParams = nullptr;
    std::string hostBundleName;
    bool isStartByOEExt = false;
    std::string specifiedFlag;
    bool isGamePrelaunch = false;
};
}
}
#endif // OHOS_ABILITY_RUNTIME_START_ABILITY_UTILS_H