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

#ifndef OHOS_ABILITY_RUNTIME_SA_INTERCEPTOR_MANAGER_H
#define OHOS_ABILITY_RUNTIME_SA_INTERCEPTOR_MANAGER_H

#include <map>
#include <mutex>
#include <vector>

#include "ability_info.h"
#include "ability_record.h"
#include "sa_interceptor_interface.h"
#include "singleton.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
class SAInterceptorManager {
public:
    static SAInterceptorManager &GetInstance();

    int32_t AddSAInterceptor(sptr<ISAInterceptor> interceptor);
    int32_t RemoveSAInterceptor(sptr<ISAInterceptor> interceptor);
    int32_t ExecuteSAInterceptor(const std::string &params, Rule &rule);

    void OnObserverDied(const wptr<IRemoteObject> &remote);
    bool SAInterceptorListIsEmpty();
    std::string GenerateSAInterceptorParams(const AAFwk::Want &want, sptr<IRemoteObject> callerToken,
        const AppExecFwk::AbilityInfo &abilityInfo, const std::string &dialogSessionId);

private:
    SAInterceptorManager();
    ~SAInterceptorManager();
    bool ObserverExist(sptr<IRemoteBroker> observer);
private:
    std::mutex saInterceptorLock_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
    std::vector<sptr<ISAInterceptor>> saInterceptors_;
    DISALLOW_COPY_AND_MOVE(SAInterceptorManager);
};

class SAInterceptorRecipient : public IRemoteObject::DeathRecipient {
public:
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
    explicit SAInterceptorRecipient(RemoteDiedHandler handler);
    virtual ~SAInterceptorRecipient();
    virtual void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

private:
    RemoteDiedHandler handler_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SA_INTERCEPTOR_MANAGER_H