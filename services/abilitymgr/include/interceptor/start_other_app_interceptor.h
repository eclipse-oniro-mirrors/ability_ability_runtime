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

#ifndef OHOS_ABILITY_RUNTIME_START_OTHER_APP_INTERCEPTOR
#define OHOS_ABILITY_RUNTIME_START_OTHER_APP_INTERCEPTOR

#include "ability_interceptor_interface.h"
#include "application_info.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class StartOtherAppInterceptor : public IAbilityInterceptor {
public:
    StartOtherAppInterceptor() = default;
    ~StartOtherAppInterceptor() = default;
    ErrCode DoProcess(const AbilityInterceptorParam &param) override;
    virtual void SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler) override
    {
        return;
    };
private:
    bool CheckNativeCall();
    bool CheckCallerIsSystemApp();
    bool CheckTargetIsSystemApp(const AppExecFwk::ApplicationInfo &applicationInfo);
    bool GetApplicationInfo(const sptr<IRemoteObject> &callerToken,
        AppExecFwk::ApplicationInfo &applicationInfo);
    bool CheckAncoShellCall(const AppExecFwk::ApplicationInfo &applicationInfo,
        const Want want);
    bool CheckStartOtherApp(const Want want);
    bool CheckCallerApiBelow12(const AppExecFwk::ApplicationInfo &applicationInfo);
    bool IsDelegatorCall(const Want want);
};
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_START_OTHER_APP_INTERCEPTOR