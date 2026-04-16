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

#include "bundle_mgr_helper.h"
#include "extension_ability_info.h"
#include "mock_my_flag.h"

namespace OHOS {
bool AgentRuntime::MyFlag::retRegisterBundleEventCallback = false;
bool AgentRuntime::MyFlag::retGetApplicationInfo = false;
bool AgentRuntime::MyFlag::isRegisterBundleEventCallbackCalled = false;
int32_t AgentRuntime::MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
AppExecFwk::AppProcessState AgentRuntime::MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
bool AgentRuntime::MyFlag::retQueryExtensionAbilityInfos = true;
bool AgentRuntime::MyFlag::shouldFillExtensionAbilityInfos = true;
AppExecFwk::ExtensionAbilityType AgentRuntime::MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
int32_t AgentRuntime::MyFlag::extensionAbilityUid = 0;

namespace AppExecFwk {
BundleMgrHelper::BundleMgrHelper()
{}

BundleMgrHelper::~BundleMgrHelper()
{}

bool BundleMgrHelper::RegisterBundleEventCallback(const sptr<IBundleEventCallback> &bundleEventCallback)
{
    AgentRuntime::MyFlag::isRegisterBundleEventCallbackCalled = true;
    return AgentRuntime::MyFlag::retRegisterBundleEventCallback;
}

bool BundleMgrHelper::GetApplicationInfo(const std::string &appName, const ApplicationFlag flag, const int32_t userId,
    ApplicationInfo &appInfo)
{
    return AgentRuntime::MyFlag::retGetApplicationInfo;
}

bool BundleMgrHelper::QueryExtensionAbilityInfos(const AAFwk::Want &want, const int32_t &flag, const int32_t &userId,
    std::vector<ExtensionAbilityInfo> &extensionInfos)
{
    if (AgentRuntime::MyFlag::retQueryExtensionAbilityInfos && AgentRuntime::MyFlag::shouldFillExtensionAbilityInfos) {
        ExtensionAbilityInfo info;
        info.type = AgentRuntime::MyFlag::extensionAbilityType;
        info.applicationInfo.uid = AgentRuntime::MyFlag::extensionAbilityUid;
        extensionInfos.push_back(info);
    }
    return AgentRuntime::MyFlag::retQueryExtensionAbilityInfos;
}
}  // namespace AppExecFwk
}  // namespace OHOS
