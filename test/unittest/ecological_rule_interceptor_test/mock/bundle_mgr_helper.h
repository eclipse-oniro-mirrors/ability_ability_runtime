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

#ifndef OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H
#define OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H

#include "bundle_mgr_interface.h"

namespace OHOS {
namespace AppExecFwk {
using Want = OHOS::AAFwk::Want;

class BundleMgrHelper {
public:
    ~BundleMgrHelper();

    static std::shared_ptr<BundleMgrHelper> GetInstance();

    int32_t GetLaunchWantForBundle(const std::string &bundleName, Want &want, int32_t userId);

    int32_t GetNameForUid(int32_t uid, std::string &name);
    bool GetApplicationInfo(
        const std::string &appName, ApplicationFlag flag, int32_t userId, ApplicationInfo &appInfo);

    int32_t GetCloneAppIndexes(const std::string &bundleName, std::vector<int32_t> &appIndexes, int32_t userId)
    {
        return 0;
    }

    int32_t QueryCloneAbilityInfo(const ElementName &element, int32_t flags, int32_t appCloneIndex,
        AbilityInfo &abilityInfo, int32_t userId)
    {
        return 0;
    }

    int32_t GetPluginAbilityInfo(const std::string &hostBundleName, const std::string &pluginBundleName,
        const std::string &pluginModuleName, const std::string &pluginAbilityName, int32_t userId,
        AbilityInfo &pluginAbilityInfo)
    {
        return 0;
    }

    bool QueryAbilityInfo(const Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo)
    {
        return true;
    }

    int32_t GetSandboxAbilityInfo(const Want &want, int32_t appIndex, int32_t flags, int32_t userId,
        AbilityInfo &abilityInfo)
    {
        return 0;
    }

    bool QueryExtensionAbilityInfos(const Want &want, const int32_t &flag, int32_t &userId,
        std::vector<ExtensionAbilityInfo> &extensionInfos)
    {
        return true;
    }

    int32_t GetSandboxExtAbilityInfos(const Want &want, int32_t appIndex, int32_t flags, int32_t userId,
        std::vector<ExtensionAbilityInfo> &extensionInfos)
    {
        return 0;
    }

    int32_t QueryCloneExtensionAbilityInfoWithAppIndex(const ElementName &element, int32_t flags, int32_t appCloneIndex,
        ExtensionAbilityInfo &extensionInfo, int32_t userId)
    {
        return 0;
    }

public:
    static int32_t retGetLaunchWantForBundle;
    static Want launchWant;
    static int32_t retGetNameForUid;
    static std::string nameForUid;
    static bool retGetApplicationInfo;
    static ApplicationInfo applicationInfo;
    static bool isBundleManagerHelperNull;

private:
    BundleMgrHelper();
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H