/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_DISPOSED_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_DISPOSED_OBSERVER_H

#include "ability_util.h"
#include "application_state_observer_stub.h"
#include "app_mgr_interface.h"
#include "cpp/mutex.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class DisposedRuleInterceptor;
class DisposedObserver : public AppExecFwk::ApplicationStateObserverStub {
public:
    DisposedObserver(const AppExecFwk::DisposedRule &disposedRule,
        const std::shared_ptr<DisposedRuleInterceptor> &interceptor, int32_t uid);
    ~DisposedObserver() = default;

    static std::string GenerateAbilityKey(const std::string &moduleName, const std::string &abilityName);

    void AddAbilityKey(const std::string &moduleName, const std::string &abilityName);

    bool HasAbilityKey(const std::string &moduleName, const std::string &abilityName);

    bool RemoveAbilityKey(const std::string &moduleName, const std::string &abilityName);

    size_t GetAbilityKeyCount();

private:
    void OnAbilityStateChanged(const AppExecFwk::AbilityStateData &abilityStateData) override;
    void OnPageShow(const AppExecFwk::PageStateData &pageStateData) override;

    ErrCode ExecuteUIExtension(const AppExecFwk::PageStateData &pageStateData);
private:
    std::shared_ptr<DisposedRuleInterceptor> interceptor_ = nullptr;
    AppExecFwk::DisposedRule disposedRule_;
    sptr<IRemoteObject> token_ = nullptr;
    ffrt::mutex abilityKeyLock_;
    int32_t uid_ = 0;
    std::vector<std::string> abilityKeys_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_DISPOSED_OBSERVER_H
