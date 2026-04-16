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

#ifndef OHOS_ABILITY_RUNTIME_DISPOSED_RULE_INTERCEPTOR_H
#define OHOS_ABILITY_RUNTIME_DISPOSED_RULE_INTERCEPTOR_H

#include "ability_interceptor_interface.h"

#include "disposed_observer.h"
#include "task_utils_wrap.h"

namespace OHOS {
namespace AppExecFwk {
struct DisposedRule;
}
namespace AAFwk {
class DisposedRuleInterceptor : public IAbilityInterceptor,
                                public std::enable_shared_from_this<DisposedRuleInterceptor> {
public:
    DisposedRuleInterceptor() = default;
    ~DisposedRuleInterceptor() = default;
    ErrCode DoProcess(AbilityInterceptorParam param) override;
    void SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler) override
    {
        taskHandler_ = taskHandler;
    };
    void UnregisterObserver(int32_t uid);
private:
    bool CheckControl(const Want &want, int32_t userId, AppExecFwk::DisposedRule &disposedRule, int32_t appIndex);
    bool FindBlockDisposedRule(const Want &want, const std::vector<AppExecFwk::DisposedRule> &disposedRuleList,
        AppExecFwk::DisposedRule &disposedRule);
    void FindNonBlockDisposedRule(const std::vector<AppExecFwk::DisposedRule> &disposedRuleList,
        AppExecFwk::DisposedRule &disposedRule);
    ErrCode StartNonBlockRule(const Want &want, AppExecFwk::DisposedRule &disposedRule, int32_t uid);
    ErrCode CreateModalUIExtension(const Want &want, const sptr<IRemoteObject> &callerToken);
    void SetInterceptInfo(const Want &want, AppExecFwk::DisposedRule &disposedRule);
    bool IsSkipDisposeRule(const std::string &callerName, const AbilityInterceptorParam &param);
private:
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::map<int32_t, sptr<DisposedObserver>> disposedObserverMap_;
    ffrt::mutex observerLock_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_DISPOSED_RULE_INTERCEPTOR_H