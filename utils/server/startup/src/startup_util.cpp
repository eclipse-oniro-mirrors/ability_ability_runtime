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

#include "startup_util.h"

#include "ability_info.h"
#include "extension_ability_info.h"
#include "global_constant.h"
#include "server_constant.h"
#include "want.h"

namespace OHOS::AbilityRuntime {
namespace {
constexpr int32_t REQUEST_CODE_LENGTH = 32;
constexpr int32_t PID_LENGTH = 16;
constexpr int32_t REQUEST_CODE_PID_LENGTH = 48;
constexpr int32_t VALID_REQUEST_CODE_LENGTH = 49;
}
bool StartupUtil::GetAppIndex(const AAFwk::Want &want, int32_t &appIndex)
{
    if (want.HasParameter(ServerConstant::DLP_INDEX)) {
        appIndex = want.GetIntParam(ServerConstant::DLP_INDEX, 0);
        return appIndex > GlobalConstant::MAX_APP_CLONE_INDEX;
    }
    if (want.HasParameter(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY)) {
        appIndex = want.GetIntParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, 0);
        return (appIndex >= 0 && appIndex <= GlobalConstant::MAX_APP_CLONE_INDEX);
    }
    appIndex = 0;
    return true;
}

int32_t StartupUtil::BuildAbilityInfoFlag()
{
    return AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_PERMISSION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_METADATA;
}

bool StartupUtil::IsSupportAppClone(AppExecFwk::ExtensionAbilityType type)
{
    bool result = type == AppExecFwk::ExtensionAbilityType::WORK_SCHEDULER ||
        type == AppExecFwk::ExtensionAbilityType::BACKUP ||
        type == AppExecFwk::ExtensionAbilityType::SHARE ||
        type == AppExecFwk::ExtensionAbilityType::PUSH ||
        type == AppExecFwk::ExtensionAbilityType::REMOTE_LOCATION ||
        type == AppExecFwk::ExtensionAbilityType::REMOTE_NOTIFICATION ||
        type == AppExecFwk::ExtensionAbilityType::VOIP ||
        type == AppExecFwk::ExtensionAbilityType::FAULT_LOG ||
        type == AppExecFwk::ExtensionAbilityType::EMBEDDED_UI ||
        type == AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
#ifdef ENABLE_CLONE_FOR_ACCOUNT
    result = true;
#endif
    return result;
}

void StartupUtil::InitAbilityInfoFromExtension(AppExecFwk::ExtensionAbilityInfo &extensionInfo,
    AppExecFwk::AbilityInfo &abilityInfo)
{
    abilityInfo.applicationName = extensionInfo.applicationInfo.name;
    abilityInfo.applicationInfo = extensionInfo.applicationInfo;
    abilityInfo.bundleName = extensionInfo.bundleName;
    abilityInfo.package = extensionInfo.moduleName;
    abilityInfo.moduleName = extensionInfo.moduleName;
    abilityInfo.name = extensionInfo.name;
    abilityInfo.srcEntrance = extensionInfo.srcEntrance;
    abilityInfo.srcPath = extensionInfo.srcEntrance;
    abilityInfo.iconPath = extensionInfo.icon;
    abilityInfo.iconId = extensionInfo.iconId;
    abilityInfo.label = extensionInfo.label;
    abilityInfo.labelId = extensionInfo.labelId;
    abilityInfo.description = extensionInfo.description;
    abilityInfo.descriptionId = extensionInfo.descriptionId;
    abilityInfo.priority = extensionInfo.priority;
    abilityInfo.permissions = extensionInfo.permissions;
    abilityInfo.readPermission = extensionInfo.readPermission;
    abilityInfo.writePermission = extensionInfo.writePermission;
    abilityInfo.uri = extensionInfo.uri;
    abilityInfo.extensionAbilityType = extensionInfo.type;
    abilityInfo.visible = extensionInfo.visible;
    abilityInfo.resourcePath = extensionInfo.resourcePath;
    abilityInfo.enabled = extensionInfo.enabled;
    abilityInfo.isModuleJson = true;
    abilityInfo.isStageBasedModel = true;
    abilityInfo.process = extensionInfo.process;
    abilityInfo.metadata = extensionInfo.metadata;
    abilityInfo.compileMode = extensionInfo.compileMode;
    abilityInfo.arkTSMode = extensionInfo.arkTSMode;
    abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityInfo.extensionTypeName = extensionInfo.extensionTypeName;
    abilityInfo.isolationProcess = extensionInfo.isolationProcess;
    abilityInfo.uid = extensionInfo.uid;
    abilityInfo.appIndex = extensionInfo.appIndex;
    if (!extensionInfo.hapPath.empty()) {
        abilityInfo.hapPath = extensionInfo.hapPath;
    }
}

int64_t StartupUtil::GenerateFullRequestCode(int32_t pid, bool backFlag, int32_t requestCode)
{
    if (requestCode <= 0 || pid <= 0) {
        return 0;
    }
    uint64_t fullRequestCode = static_cast<uint64_t>(requestCode);
    uint64_t tempNum = static_cast<uint64_t>(pid);
    fullRequestCode |= (tempNum << REQUEST_CODE_LENGTH);
    if (backFlag) {
        tempNum = 1;
        fullRequestCode |= (tempNum << REQUEST_CODE_PID_LENGTH);
    }
    return fullRequestCode;
}

CallerRequestInfo StartupUtil::ParseFullRequestCode(int64_t fullRequestCode)
{
    CallerRequestInfo requestInfo;
    if (fullRequestCode <= 0) {
        return requestInfo;
    }
    uint64_t tempFullRequestCode = static_cast<uint64_t>(fullRequestCode);
    if ((tempFullRequestCode >> VALID_REQUEST_CODE_LENGTH) > 0) {
        return requestInfo;
    }
    uint64_t tempNum = 1;
    requestInfo.requestCode = (tempFullRequestCode & ((tempNum << REQUEST_CODE_LENGTH) - 1));
    tempFullRequestCode >>= REQUEST_CODE_LENGTH;
    requestInfo.pid = (tempFullRequestCode & ((tempNum << PID_LENGTH) - 1));
    tempFullRequestCode >>= PID_LENGTH;
    requestInfo.backFlag = (tempFullRequestCode == 1);
    return requestInfo;
}
}  // namespace OHOS::AbilityRuntime
