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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_REQUEST_H
#define OHOS_ABILITY_RUNTIME_ABILITY_REQUEST_H

#include <cinttypes>
#include <memory>

#include "ability_connect_callback_interface.h"
#include "ability_info.h"
#include "ability_start_setting.h"
#include "irequest_start_ability_callback.h"
#include "launch_param.h"
#include "process_options.h"
#include "session_info.h"
#include "start_options.h"
#include "start_specified_ability_params.h"
#include "want.h"
#include "ui_extension/ui_extension_ability_connect_info.h"

namespace OHOS {
namespace AAFwk {
using UIExtensionAbilityConnectInfo = AbilityRuntime::UIExtensionAbilityConnectInfo;
/**
 * @class AbilityRequest
 * Wrap parameters of starting ability.
 */
enum AbilityCallType {
    INVALID_TYPE = 0,
    CALL_REQUEST_TYPE,
    START_OPTIONS_TYPE,
    START_SETTINGS_TYPE,
    START_EXTENSION_TYPE,
};

enum CollaboratorType {
    DEFAULT_TYPE = 0,
    RESERVE_TYPE,
    OTHERS_TYPE
};

struct AbilityRequest {
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo appInfo;
    StartOptions startOptions;
    std::vector<AppExecFwk::SupportWindowMode> supportWindowModes;
    std::string specifiedFlag;
    std::string customProcess;
    std::string moduleProcess;
    std::string reservedBundleName;
    std::string appId;
    std::string startTime;
    std::string hostBundleName;

    sptr<IRemoteObject> callerToken = nullptr;          // call ability
    sptr<IRemoteObject> asCallerSourceToken = nullptr;          // call ability
    sptr<IAbilityConnection> connect = nullptr;
    sptr<IRemoteObject> abilityInfoCallback = nullptr;
    sptr<IRequestStartAbilityCallback> requestCallback = nullptr;
    sptr<SessionInfo> sessionInfo;
    std::shared_ptr<AbilityStartSetting> startSetting = nullptr;
    std::shared_ptr<ProcessOptions> processOptions = nullptr;
    std::shared_ptr<StartWindowOption> startWindowOption = nullptr;
    std::shared_ptr<StartSpecifiedAbilityParams> startSpecifiedParams = nullptr;
    sptr<UIExtensionAbilityConnectInfo> uiExtensionAbilityConnectInfo = nullptr;

    int64_t restartTime = 0;
    uint64_t specifiedFullTokenId = 0;

    int32_t primaryWindowId = -1;
    int32_t restartCount = -1;
    int32_t uid = 0;
    int32_t collaboratorType = CollaboratorType::DEFAULT_TYPE;
    int32_t callerTokenRecordId = -1;
    int32_t userId = -1;
    int32_t loadExtensionTimeout = 0; // only for connectAbility
    uint32_t callerAccessTokenId = 0;
    uint32_t specifyTokenId = 0;
    int callerUid = -1;         // call ability
    int requestCode = -1;
    int32_t atomicServiceShortLink = 0;
    AbilityCallType callType = AbilityCallType::INVALID_TYPE;           // call ability
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    AppExecFwk::ExtensionProcessMode extensionProcessMode = AppExecFwk::ExtensionProcessMode::UNDEFINED;

    bool isStartInSplitMode = false;
    bool restart = false;
    bool startRecent = false;
    bool uriReservedFlag = false;
    bool isFromIcon = false;
    bool isShellCall = false;
    bool isTargetPlugin = false;
    // ERMS embedded atomic service
    bool isQueryERMS = false;
    bool isEmbeddedAllowed = false;
    bool callSpecifiedFlagTimeout = false;
    bool hideStartWindow = false;
    bool hideFailureTipDialog = false;
    bool promotePriority = false;
    bool isFromOpenLink = false;
    bool isStartByOEExt = false;
    std::pair<bool, LaunchReason> IsContinuation() const;

    bool IsAcquireShareData() const
    {
        return want.GetBoolParam(Want::PARAM_ABILITY_ACQUIRE_SHARE_DATA, false);
    }

    bool IsAppRecovery() const
    {
        return want.GetBoolParam(Want::PARAM_ABILITY_RECOVERY_RESTART, false);
    }

    bool IsCallType(const AbilityCallType & type) const
    {
        return (callType == type);
    }

    void Dump(std::vector<std::string> &state);

    void Voluation(const Want &srcWant, int srcRequestCode,
        const sptr<IRemoteObject> &srcCallerToken, const std::shared_ptr<AbilityStartSetting> srcStartSetting = nullptr,
        int srcCallerUid = -1);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
