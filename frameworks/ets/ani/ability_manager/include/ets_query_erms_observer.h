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

#ifndef OHOS_ABILITY_RUNTIME_ETS_QUERY_ERMS_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_ETS_QUERY_ERMS_OBSERVER_H

#include "ani_common_util.h"
#include "query_erms_observer_stub.h"

namespace OHOS {
namespace AbilityRuntime {
struct EtsQueryERMSObserverObject {
    std::string appId;
    std::string startTime;
    ani_object callback;
};

class EtsQueryERMSObserver : public QueryERMSObserverStub {
public:
    explicit EtsQueryERMSObserver(ani_vm *etsVm);
    ~EtsQueryERMSObserver();
    void OnQueryFinished(const std::string &appId, const std::string &startTime,
        const AtomicServiceStartupRule &rule, int32_t resultCode) override;
    void AddEtsObserverObject(const std::string &appId, const std::string &startTime,
            ani_object callback);
private:
    void CallCallback(ani_object callback, const AtomicServiceStartupRule &rule, int32_t resultCode);
    void HandleOnQueryFinished(const std::string &appId, const std::string &startTime,
        const AtomicServiceStartupRule &rule, int32_t resultCode);
    bool WrapAtomicServiceStartupRule(ani_env *env, const AbilityRuntime::AtomicServiceStartupRule &rule,
        ani_object &ruleObj);
    ani_env *AttachCurrentThread();
    void DetachCurrentThread();

    ani_vm *etsVm_;
    std::mutex etsObserverObjectListLock_;
    std::vector<EtsQueryERMSObserverObject> etsObserverObjectList_;
    bool isAttachThread_ = false;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_ETS_QUERY_ERMS_OBSERVER_H
