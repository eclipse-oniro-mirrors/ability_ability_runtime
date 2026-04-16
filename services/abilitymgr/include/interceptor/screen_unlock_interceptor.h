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

#ifndef OHOS_ABILITY_RUNTIME_SCREEN_UNLOCK_INTERCEPTOR
#define OHOS_ABILITY_RUNTIME_SCREEN_UNLOCK_INTERCEPTOR

#include "ability_interceptor_interface.h"

namespace OHOS {
namespace AAFwk {
class ScreenUnlockInterceptor : public IAbilityInterceptor {
public:
    ScreenUnlockInterceptor() = default;
    ~ScreenUnlockInterceptor() = default;
    ErrCode DoProcess(AbilityInterceptorParam param) override;
    virtual void SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler) override
    {
        return;
    };
private:
    bool GetTargetAbilityInfo(const AbilityInterceptorParam &param, AppExecFwk::AbilityInfo &targetAbilityInfo);
    void QueryTargetAbilityInfo(const AbilityInterceptorParam &param, AppExecFwk::AbilityInfo &targetAbilityInfo);
    ErrCode ProcessSystemApp(const AppExecFwk::AbilityInfo &targetAbilityInfo);
    ErrCode ProcessNonSystemApp(const AppExecFwk::AbilityInfo &targetAbilityInfo);
    void ReportSystemAppUIAbilityEvent(const AppExecFwk::AbilityInfo &targetAbilityInfo);
    ErrCode CheckExtensionInterception(const std::string &extensionTypeName,
        const std::string &bundleName, bool isSystemApp);
    ErrCode CheckSystemAppExtensionInterception(const std::string &extensionTypeName,
        const std::string &bundleName);
    ErrCode CheckThirdPartyExtensionInterception(const std::string &extensionTypeName,
        const std::string &bundleName);
    ErrCode CheckInterceptionByConfig(const std::string &extensionTypeName,
        const std::string &appIdentifier, bool interception, bool isSystemApp);
    std::string GetAppIdentifier(const std::string &bundleName);
};
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_SCREEN_UNLOCK_INTERCEPTOR