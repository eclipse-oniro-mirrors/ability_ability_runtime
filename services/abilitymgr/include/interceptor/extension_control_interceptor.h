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

#ifndef OHOS_ABILITY_RUNTIME_EXTENSION_CONTROL_INTERCEPTOR_H
#define OHOS_ABILITY_RUNTIME_EXTENSION_CONTROL_INTERCEPTOR_H

#include "ability_interceptor_interface.h"
#include "ability_info.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class ExtensionControlInterceptor : public IAbilityInterceptor {
public:
    ExtensionControlInterceptor() = default;
    ~ExtensionControlInterceptor() = default;
    ErrCode DoProcess(const AbilityInterceptorParam &param) override;
private:
    bool IsExtensionStartThirdPartyAppEnable(std::string extensionTypeName, std::string targetBundleName);
    bool IsExtensionStartServiceEnable(std::string extensionTypeName, std::string targetUri);
    bool GetCallerAbilityInfo(const AbilityInterceptorParam& param,
        AppExecFwk::AbilityInfo& callerAbilityInfo);
    bool GetTargetAbilityInfo(const AbilityInterceptorParam& param,
        AppExecFwk::AbilityInfo& callerAbilityInfo);
    int32_t ProcessInterceptOld(const AbilityInterceptorParam& param,
        const AppExecFwk::AbilityInfo &targetAbilityInfo, const AppExecFwk::AbilityInfo &callerAbilityInfo);
    int32_t ProcessInterceptNew(const AbilityInterceptorParam& param,
        const AppExecFwk::AbilityInfo &targetAbilityInfo, const AppExecFwk::AbilityInfo &callerAbilityInfo);
};
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_START_OTHER_APP_INTERCEPTOR